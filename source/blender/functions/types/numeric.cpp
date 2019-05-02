#include "numeric.hpp"
#include "BLI_lazy_init.hpp"

#include "FN_tuple_call.hpp"
#include "FN_llvm.hpp"

namespace FN {
namespace Types {

LAZY_INIT_REF__NO_ARG(SharedType, GET_TYPE_float)
{
  SharedType type = SharedType::New("Float");
  type->extend<CPPTypeInfoForType<float>>();
  type->extend<PackedLLVMTypeInfo>(
      [](llvm::LLVMContext &context) { return llvm::Type::getFloatTy(context); });
  return type;
}

LAZY_INIT_REF__NO_ARG(SharedType, GET_TYPE_int32)
{
  SharedType type = SharedType::New("Int32");
  type->extend<CPPTypeInfoForType<int32_t>>();
  type->extend<PackedLLVMTypeInfo>(
      [](llvm::LLVMContext &context) { return llvm::Type::getIntNTy(context, 32); });
  return type;
}

class FloatVectorType : public TrivialLLVMTypeInfo {
 private:
  uint m_size;

 public:
  FloatVectorType(uint size) : m_size(size)
  {
  }

  llvm::Type *get_type(llvm::LLVMContext &context) const override
  {
    return llvm::VectorType::get(llvm::Type::getFloatTy(context), m_size);
  }

  void build_store_ir__copy(CodeBuilder &builder,
                            llvm::Value *vector,
                            llvm::Value *address) const override
  {
    address = builder.CastToPointerOf(address, builder.getFloatTy());
    for (uint i = 0; i < m_size; i++) {
      llvm::Value *value = builder.CreateExtractElement(vector, i);
      llvm::Value *value_address = builder.CreateConstGEP1_32(address, i);
      builder.CreateStore(value, value_address);
    }
  }

  llvm::Value *build_load_ir__copy(CodeBuilder &builder, llvm::Value *address) const override
  {
    llvm::Value *vector = builder.getUndef(this->get_type(builder.getContext()));
    address = builder.CastToPointerOf(address, builder.getFloatTy());
    for (uint i = 0; i < m_size; i++) {
      llvm::Value *value_address = builder.CreateConstGEP1_32(address, i);
      llvm::Value *value = builder.CreateLoad(value_address);
      vector = builder.CreateInsertElement(vector, value, i);
    }
    return vector;
  }
};

LAZY_INIT_REF__NO_ARG(SharedType, GET_TYPE_fvec3)
{
  SharedType type = SharedType::New("FVec3");
  type->extend<CPPTypeInfoForType<Vector>>();
  type->extend<FloatVectorType>(3);
  return type;
}

}  // namespace Types
}  // namespace FN
