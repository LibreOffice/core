#!/bin/bash
# Run AFL++ fuzzing for coolwsd targets, or update their corpus.
#
# Usage: fuzzer/run-afl.sh <target> [afl-fuzz options...]
#        fuzzer/run-afl.sh --update-corpus [--minimize [BYTES]] <target>
#   Targets: admin, clientsession, clientrequest, httpresponse, httpecho
#
# Example: fuzzer/run-afl.sh admin
# Example: fuzzer/run-afl.sh httpecho -t 5000
# Example: fuzzer/run-afl.sh --update-corpus clientrequest
# Example: fuzzer/run-afl.sh --update-corpus --minimize clientrequest
# Example: fuzzer/run-afl.sh --update-corpus --minimize 512 clientrequest

set -e
set -x

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
AFL_COMMON_FLAGS="-m none -t 5000"
MODE=fuzz
MINIMIZE=false
MINIMIZE_THRESHOLD=0

if test "$1" = "--update-corpus"; then
    MODE=update
    shift
    if test "$1" = "--minimize"; then
        MINIMIZE=true
        shift
        # Optional size threshold (e.g., --minimize 512 = only files > 512 bytes)
        if test -n "$1" && echo "$1" | grep -qE '^[0-9]+$'; then
            MINIMIZE_THRESHOLD=$1
            shift
        fi
    fi
fi

TARGET=$1
if test -z "$TARGET"; then
    echo "Usage: $0 [--update-corpus] <target> [afl-fuzz options...]"
    echo "Targets: admin, clientsession, clientrequest, httpresponse, httpecho"
    exit 1
fi
shift

case "$TARGET" in
    admin)
        BINARY=./admin_fuzzer
        CORPUS="${SCRIPT_DIR}/admin-data"
        DICT="${SCRIPT_DIR}/kit_fuzzer.dict"
        TIMEOUT=1000
        ;;
    clientsession)
        BINARY=./clientsession_fuzzer
        CORPUS="${SCRIPT_DIR}/data"
        DICT="${SCRIPT_DIR}/kit_fuzzer.dict"
        TIMEOUT=1000
        ;;
    clientrequest)
        BINARY=./clientrequest_fuzzer
        CORPUS="${SCRIPT_DIR}/clientrequest-data/corpus"
        DICT="${SCRIPT_DIR}/httpecho-data/http.dict"
        TIMEOUT=1000
        ;;
    httpresponse)
        BINARY=./httpresponse_fuzzer
        CORPUS="${SCRIPT_DIR}/httpresponse-data"
        DICT="${SCRIPT_DIR}/httpecho-data/http.dict"
        TIMEOUT=1000
        ;;
    httpecho)
        BINARY=./httpecho_fuzzer
        CORPUS="${SCRIPT_DIR}/httpecho-data/corpus"
        DICT="${SCRIPT_DIR}/httpecho-data/http.dict"
        TIMEOUT=3000
        ;;
    *)
        echo "Unknown target: $TARGET"
        echo "Available: admin, clientsession, clientrequest, httpresponse, httpecho"
        exit 1
        ;;
esac

OUTPUT="/tmp/afl-${TARGET}-output"
mkdir -p "$OUTPUT"

export LSAN_OPTIONS="detect_leaks=0:symbolize=0"
export ASAN_OPTIONS="abort_on_error=1:symbolize=0:detect_leaks=0"
# The dlopen warning comes from linked libraries (Poco, OpenSSL), not from
# the code under test. The fuzzed code paths are statically compiled in.
export AFL_IGNORE_PROBLEMS=1
export AFL_AUTORESUME=1

if test "$MODE" = "update"; then
    QUEUES=$(find "$OUTPUT" -mindepth 2 -maxdepth 2 -type d -name queue 2>/dev/null)
    if test -z "$QUEUES"; then
        echo "No AFL++ output found under $OUTPUT"
        echo "Run fuzzing first: $0 $TARGET"
        exit 1
    fi

    fcount() { find "$1" -maxdepth 1 -type f | wc -l; }

    echo "=== Merging AFL++ queues into corpus ==="
    MERGED=$(mktemp -d /tmp/afl-merge-XXXXXX)
    BEFORE=$(fcount "$CORPUS")
    find "$CORPUS" -maxdepth 1 -type f -exec cp -t "$MERGED" {} +
    QUEUE_TOTAL=0
    for q in $QUEUES; do
        count=$(fcount "$q")
        QUEUE_TOTAL=$((QUEUE_TOTAL + count))
        echo "  merging $q ($count files)"
        find "$q" -maxdepth 1 -type f -exec cp -t "$MERGED" {} +
    done
    MERGED_COUNT=$(fcount "$MERGED")
    echo "Corpus: $BEFORE files, queue: $QUEUE_TOTAL files, merged: $MERGED_COUNT files"

    echo "=== Minimizing corpus with afl-cmin ==="
    CMIN_OUT=$(mktemp -d /tmp/afl-cmin-XXXXXX)
    afl-cmin $AFL_COMMON_FLAGS -i "$MERGED" -o "$CMIN_OUT" -- "${BINARY}_native.afl" @@
    AFTER=$(fcount "$CMIN_OUT")
    echo "Minimized: $MERGED_COUNT -> $AFTER files"

    RESULT_DIR="$CMIN_OUT"
    if test "$MINIMIZE" = "true"; then
        echo "=== Shrinking individual test cases with afl-tmin (threshold: ${MINIMIZE_THRESHOLD}B) ==="
        TMIN_OUT=$(mktemp -d /tmp/afl-tmin-XXXXXX)
        NPROC=$(nproc 2>/dev/null || echo 4)
        # Copy small files as-is, only run afl-tmin on files above threshold
        if test "$MINIMIZE_THRESHOLD" -gt 0; then
            find "$CMIN_OUT" -maxdepth 1 -type f ! -size +"${MINIMIZE_THRESHOLD}c" \
                -exec cp -t "$TMIN_OUT" {} +
        fi
        find "$CMIN_OUT" -maxdepth 1 -type f -size +"${MINIMIZE_THRESHOLD}c" | \
            xargs -P "$NPROC" -I{} sh -c \
                'afl-tmin '"$AFL_COMMON_FLAGS"' -i "$1" -o "'"$TMIN_OUT"'/$(basename "$1")" -- "'"${BINARY}_native.afl"'" @@' _ {}
        RESULT_DIR="$TMIN_OUT"
    fi

    echo "=== Updating corpus ==="
    find "$CORPUS" -maxdepth 1 -type f -delete
    find "$RESULT_DIR" -maxdepth 1 -type f -exec cp -t "$CORPUS" {} +

    # AFL++ names files like "id:000000,time:0,execs:0,orig:...". The ':' is
    # forbidden on Windows (NTFS/FAT), which breaks anyone who clones this
    # repo on Windows or unpacks a tarball there. Rewrite ':' to '_'; the
    # remaining filename is still unique because AFL IDs are unique.
    find "$CORPUS" -maxdepth 1 -type f -name '*:*' -print0 | \
        while IFS= read -r -d '' f; do
            dir=$(dirname "$f")
            newbase=$(basename "$f" | tr ':' '_')
            mv -n -- "$f" "$dir/$newbase"
        done

    FINAL=$(fcount "$CORPUS")
    echo "Done: $BEFORE -> $FINAL files in $CORPUS"

    rm -rf "$MERGED" "$CMIN_OUT" ${TMIN_OUT:+"$TMIN_OUT"}
    echo "You can now remove the AFL++ output directory: rm -rf $OUTPUT"
    exit 0
fi

DICT_FLAG=""
if test -f "$DICT"; then
    DICT_FLAG="-x $DICT"
fi

# Check if an AFL++ instance is alive using flock, same as AFL++ itself.
# If we can grab the exclusive lock, no afl-fuzz holds it — instance is dead.
afl_instance_alive() {
    local dir="$OUTPUT/$1"
    test -d "$dir" || return 1
    ! flock -n "$dir" true 2>/dev/null
}

# Variant binaries
BIN_NATIVE="${BINARY}_native.afl"
BIN_SAN="${BINARY}_san.afl"
BIN_CMPLOG="${BINARY}_cmplog.afl"
BIN_CMPCOV="${BINARY}_cmpcov.afl"

# Instance schedule per the AFL++ fuzzing_in_depth.md recommendations:
#   1  main    -> native, AFL_FINAL_SYNC=1
#   2  san01   -> san binary
#   3  cmplog01-> native binary with -c cmplog binary, -l 2AT
#   4  cmpcov01-> cmpcov binary
#   5  cmplog02-> native binary with -c cmplog binary, -l 2AT
#   6  cmpcov02-> cmpcov binary
#   7+ secNN   -> native binary, round-robin through extra settings
#
# Each entry: "instance_name|binary|extra_flags|extra_env"
INSTANCE_SCHEDULE=(
    "main|${BIN_NATIVE}|-M main|AFL_FINAL_SYNC=1"
    "san01|${BIN_NATIVE}|-S san01 -w ${BIN_SAN}|"
    "cmplog01|${BIN_NATIVE}|-S cmplog01 -c ${BIN_CMPLOG} -l 2AT|"
    "cmpcov01|${BIN_CMPCOV}|-S cmpcov01|"
    "cmplog02|${BIN_NATIVE}|-S cmplog02 -c ${BIN_CMPLOG} -l 2AT|"
    "cmpcov02|${BIN_CMPCOV}|-S cmpcov02|"
)

# Extra settings for native secondary instances (7+), round-robin.
# Mix per AFL++ docs: MOpt, old queue, power schedules, trim control, explore/exploit.
NATIVE_EXTRAS=(
    "-L 0"                               # MOpt mutator
    "-Z"                                  # old queue cycling
    "-P explore"                          # explore mode
    "-P exploit"                          # exploit mode
    "-p fast|AFL_DISABLE_TRIM=1"          # fast schedule, no trim
    "-p coe|AFL_DISABLE_TRIM=1"           # coe schedule, no trim
    "-p lin|AFL_DISABLE_TRIM=1"           # lin schedule, no trim
    "-p quad|AFL_DISABLE_TRIM=1"          # quad schedule, no trim
    "-p exploit|AFL_DISABLE_TRIM=1"       # exploit schedule, no trim
    "-p rare|AFL_DISABLE_TRIM=1"          # rare schedule, no trim
)

# Find the first dead or next-new instance slot
SLOT=""
SLOT_IDX=""
for i in "${!INSTANCE_SCHEDULE[@]}"; do
    IFS='|' read -r name bin flags env <<< "${INSTANCE_SCHEDULE[$i]}"
    if ! afl_instance_alive "$name"; then
        SLOT="$i"
        break
    fi
done

if test -z "$SLOT"; then
    # All fixed slots alive — find a native secondary slot (sec07, sec08, ...)
    N=7
    while true; do
        SEC_NAME="sec$(printf '%02d' $N)"
        if ! afl_instance_alive "$SEC_NAME"; then
            # Compute round-robin index into NATIVE_EXTRAS
            EXTRA_IDX=$(( (N - 7) % ${#NATIVE_EXTRAS[@]} ))
            IFS='|' read -r extra_flags extra_env <<< "${NATIVE_EXTRAS[$EXTRA_IDX]}"
            SLOT_NAME="$SEC_NAME"
            SLOT_BIN="$BIN_NATIVE"
            SLOT_FLAGS="-S $SEC_NAME $extra_flags"
            SLOT_ENV="$extra_env"
            break
        fi
        N=$((N + 1))
    done
else
    IFS='|' read -r SLOT_NAME SLOT_BIN SLOT_FLAGS SLOT_ENV <<< "${INSTANCE_SCHEDULE[$SLOT]}"
fi

echo "Starting AFL++ instance: $SLOT_NAME ($SLOT_FLAGS)"
echo "  binary: $SLOT_BIN"
test -n "$SLOT_ENV" && echo "  env: $SLOT_ENV"

# Export any extra env vars
if test -n "$SLOT_ENV"; then
    for var in $SLOT_ENV; do
        export "$var"
    done
fi

exec afl-fuzz $SLOT_FLAGS -i "$CORPUS" -o "$OUTPUT" $DICT_FLAG -t "$TIMEOUT" "$@" -- "$SLOT_BIN" @@
