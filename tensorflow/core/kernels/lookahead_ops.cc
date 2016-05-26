

#include "tensorflow/core/kernels/lookahead_ops.h"

using namespace tensorflow;

template<typename T>
class LookaheadOp<T, 0> : public OpKernel {
 public:
  explicit LookaheadOp(OpKernelConstruction* context) : OpKernel(context) {
    const DataType dt = DataTypeToEnum<T>::v();
    OP_REQUIRES_OK(context, context->MatchSignature({dt, dt}, {dt}));
  }

  void Compute(OpKernelContext* context) override {
    // Grab the input tensor
    const Tensor& input_tensor = context->input(0);
    auto input = input_tensor.tensor<T, 3>();

    const Tensor& filter_tensor = context->input(1);
    auto filter = filter_tensor.matrix<T>();

    // Check that dimension is equal
    OP_REQUIRES(
        context, input_tensor.dim_size(2) == filter_tensor.dim_size(1),
        errors::InvalidArgument("f is not equal in filter and input"));

    // Create output tensor
    Tensor* output_tensor = NULL;
    OP_REQUIRES_OK(context, context->allocate_output(0, input_tensor.shape(),
                                                     &output_tensor));
    auto output = output_tensor->template tensor<T, 3>();

    for (int timestep = 0; timestep < input_tensor.dim_size(0); timestep++) {
      for (int batch = 0; batch < input_tensor.dim_size(1); batch++) {
        for (int frequence = 0; frequence < input_tensor.dim_size(2); frequence++) {
          output(timestep, batch, frequence) = 0;
          for(int tau = 0; tau < filter_tensor.dim_size(0) && timestep + tau < input_tensor.dim_size(0); tau++) {
            output(timestep, batch, frequence) += input(timestep + tau, batch, frequence) * filter(tau, frequence);
          }
        }
      }
    }
  }
};

REGISTER_KERNEL_BUILDER(Name("Lookaheadcpu").Device(DEVICE_CPU), LookaheadOp<float, 0>);
