#!/usr/bin/env bash

set -x

FUZZER=./clientrequest_fuzzer
SEED=fuzzer/clientrequest-data/corpus
WORK=/tmp/clientrequest_fuzzer
DICT=fuzzer/httpecho-data/http.dict

mkdir -p "$WORK"

DIR=$( dirname -- "$( readlink -f -- "$0"; )"; )

LLVM_VER=18

#export LSAN_OPTIONS=verbosity=1:log_threads=1
export LSAN_OPTIONS=detect_leaks=0
export ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:symbolize=1:allocator_may_return_null=1:detect_leaks=0:suppressions=${DIR}/../test/asan-suppressions.txt:halt_on_error=0
export ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-${LLVM_VER}/bin/llvm-symbolizer
export PATH=/usr/lib/llvm-${LLVM_VER}/bin:$PATH

# 1. Fuzz (Ctrl-C to stop)
$FUZZER -dict=$DICT -jobs=16 -timeout=10 -max_len=4096 "$WORK" "$SEED"

# 2. Compact: merge working findings back into seed corpus
#mv crash-* "$WORK"/.
#mv "$SEED"/* "$WORK"/.
#$FUZZER -merge=1 "$SEED" "$WORK"

# 3. Clean working dir for next run
#rm -rf "$WORK"

