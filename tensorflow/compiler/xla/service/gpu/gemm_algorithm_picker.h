/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef TENSORFLOW_COMPILER_XLA_SERVICE_GPU_GEMM_ALGORITHM_PICKER_H_
#define TENSORFLOW_COMPILER_XLA_SERVICE_GPU_GEMM_ALGORITHM_PICKER_H_

#include "absl/types/optional.h"
#include "tensorflow/compiler/xla/service/gpu/backend_configs.pb.h"
#include "tensorflow/compiler/xla/service/gpu/gpu_conv_runner.h"
#include "tensorflow/compiler/xla/service/hlo_instructions.h"
#include "tensorflow/compiler/xla/service/hlo_module.h"
#include "tensorflow/compiler/xla/service/hlo_pass_interface.h"
#include "tensorflow/core/platform/stream_executor_no_cuda.h"
#include "tensorflow/core/protobuf/autotuning.pb.h"
#include "tensorflow/stream_executor/device_memory_allocator.h"

namespace xla {
namespace gpu {

class GemmAutotuneCache {
 public:
  static uint64 GemmAutotuneCacheKeyHasher(
      const se::StreamExecutor* stream_exec, const HloInstruction* instr,
      const HloInstruction* lhs, const HloInstruction* rhs,
      const GemmBackendConfig& gemm_config);
  static GemmAutotuneCacheValue CreateGemmAutotuneCacheValue(
      const se::StreamExecutor* stream_exec, const HloInstruction* instr,
      const HloInstruction* Lhs, const HloInstruction* rhs,
      const GemmBackendConfig& gemm_config,
      absl::optional<se::blas::AlgorithmType> result);
  bool LookupCache(uint64 key, absl::optional<se::blas::AlgorithmType>& result);
  bool AddToCache(uint64 key, const GemmAutotuneCacheValue& cache_value);
  uint64 cache_hits;
  uint64 cache_misses;

 private:
  GemmAutotuneCache();
  ~GemmAutotuneCache();
  friend class GemmAutotuneCacheSingleton;
  std::string autotune_cache_filename_;
  bool in_use_;
  GemmAutotuneCacheProto gemm_autotune_cache_proto_;
};

class GemmAutotuneCacheSingleton {
  public:
   static GemmAutotuneCache& GetInstance();
};

class GemmAlgorithmPicker : public HloModulePass {
 public:
  GemmAlgorithmPicker(se::StreamExecutor* stream_exec,
                      se::DeviceMemoryAllocator* allocator)
      : stream_exec_(stream_exec), allocator_(allocator) {}

  absl::string_view name() const override { return "gemm-algorithm-picker"; }

  StatusOr<bool> Run(HloModule* module) override;

 private:
  se::StreamExecutor* stream_exec_;
  se::DeviceMemoryAllocator* allocator_;
};

}  // namespace gpu
}  // namespace xla

#endif  // TENSORFLOW_COMPILER_XLA_SERVICE_GPU_GEMM_ALGORITHM_PICKER_H_
