#!/bin/bash
# Run libFuzzer for coolwsd targets, or merge findings back into the corpus.
#
# Usage: fuzzer/run-fuzz.sh <target> [libfuzzer options...]
#        fuzzer/run-fuzz.sh --merge <target>
#   Targets: admin, clientsession, clientrequest, httpresponse, httpecho
#
# Example: fuzzer/run-fuzz.sh admin
# Example: fuzzer/run-fuzz.sh httpecho -jobs=8
# Example: fuzzer/run-fuzz.sh --merge clientrequest

set -e
set -x

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
MODE=fuzz

if test "$1" = "--merge"; then
    MODE=merge
    shift
fi

TARGET=$1
if test -z "$TARGET"; then
    echo "Usage: $0 [--merge] <target> [libfuzzer options...]"
    echo "Targets: admin, clientsession, clientrequest, httpresponse, httpecho"
    exit 1
fi
shift

case "$TARGET" in
    admin)
        BINARY="${SCRIPT_DIR}/admin_fuzzer"
        CORPUS="${SCRIPT_DIR}/admin-data"
        DICT="${SCRIPT_DIR}/kit_fuzzer.dict"
        MAX_LEN=4096
        TIMEOUT=10
        ;;
    clientsession)
        BINARY="${SCRIPT_DIR}/clientsession_fuzzer"
        CORPUS="${SCRIPT_DIR}/data"
        DICT="${SCRIPT_DIR}/kit_fuzzer.dict"
        MAX_LEN=4096
        TIMEOUT=10
        ;;
    clientrequest)
        BINARY="${SCRIPT_DIR}/clientrequest_fuzzer"
        CORPUS="${SCRIPT_DIR}/clientrequest-data/corpus"
        DICT="${SCRIPT_DIR}/httpecho-data/http.dict"
        MAX_LEN=1000
        TIMEOUT=42
        ;;
    httpresponse)
        BINARY="${SCRIPT_DIR}/httpresponse_fuzzer"
        CORPUS="${SCRIPT_DIR}/httpresponse-data"
        DICT="${SCRIPT_DIR}/httpecho-data/http.dict"
        MAX_LEN=4096
        TIMEOUT=10
        ;;
    httpecho)
        BINARY="${SCRIPT_DIR}/httpecho_fuzzer"
        CORPUS="${SCRIPT_DIR}/httpecho-data/corpus"
        DICT="${SCRIPT_DIR}/httpecho-data/http.dict"
        MAX_LEN=16384
        TIMEOUT=1
        ;;
    *)
        echo "Unknown target: $TARGET"
        echo "Available: admin, clientsession, clientrequest, httpresponse, httpecho"
        exit 1
        ;;
esac

OUTPUT="/tmp/libfuzz-${TARGET}-output"
mkdir -p "$OUTPUT"

export LSAN_OPTIONS="detect_leaks=0"
export ASAN_OPTIONS="strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:symbolize=1:allocator_may_return_null=1:detect_leaks=0:halt_on_error=0:suppressions=${SCRIPT_DIR}/../test/asan-suppressions.txt"

if test "$MODE" = "merge"; then
    # Merge findings from the work dir back into the seed corpus.
    mv crash-* "$OUTPUT"/.
    mv slow-* "$OUTPUT"/.
    mv timeout-* "$OUTPUT"/.
    exec "$BINARY" -merge=1 "$CORPUS" "$OUTPUT"
fi

DICT_FLAG=""
if test -f "$DICT"; then
    DICT_FLAG="-dict=$DICT"
fi

NPROC=$(nproc 2>/dev/null || echo 4)
exec "$BINARY" -timeout="$TIMEOUT" -max_len="$MAX_LEN" -jobs="$NPROC" $DICT_FLAG "$@" "$OUTPUT" "$CORPUS"
