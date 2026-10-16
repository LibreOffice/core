#!/bin/sh
# Render one corpus track with the CLI named by $XRAY_OFFICE_CLI into $2, under the track's
# per-format identity `stem__ext.pdf`.
#   sweep.sh <corpus-subdir> <out-dir> [parallel]
# Reproducible: SOURCE_DATE_EPOCH and TZ are set, so two runs of one build are byte-equal with
# nothing masked. Each job renders into a directory of its own because `render --outdir` names
# its output from the stem alone, and the corpus holds stems that differ only by extension.
# The file count is re-counted from disk by the caller, never taken from this loop.
set -e
src=$1
out=$2
par=${3:-4}
mkdir -p "$out"
export SOURCE_DATE_EPOCH=1700000000
export TZ=UTC
find "$src" -type f | sort > "$out/.jobs"
xargs -a "$out/.jobs" -d '\n' -P "$par" -I{} sh -c '
    f="{}"
    base=$(basename "$f")
    stem=${base%.*}
    ext=$(printf "%s" "${base##*.}" | tr "A-Z" "a-z")
    tmp=$(mktemp -d)
    if "$XRAY_OFFICE_CLI" render --format pdf --quiet --outdir "$tmp" "$f" >/dev/null 2>&1; then
        mv "$tmp"/*.pdf "'"$out"'/${stem}__${ext}.pdf" 2>/dev/null || echo "NOOUT $f"
    else
        echo "FAIL $f"
    fi
    rm -rf "$tmp"
'
