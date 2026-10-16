#!/bin/sh
# Render our half of a list of documents, one output directory per document.
#
#   render-ours.sh <list-file> <outdir> [jobs]
#
# `SOURCE_DATE_EPOCH` is fixed so two runs of the same tree are byte-equal with
# nothing masked; see CLAUDE.md, "Set SOURCE_DATE_EPOCH when comparing two
# renderings byte for byte".
set -eu
LIST=$1
OUT=$2
JOBS=${3:-3}
CLI=${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the XRay.Office.Cli you mean to measure}

export SOURCE_DATE_EPOCH=1700000000
mkdir -p "$OUT"

render() {
    doc=$1
    out=$2
    key=$(printf '%s' "$doc" | md5sum | cut -c1-12)
    dir="$out/$key"
    mkdir -p "$dir"
    if timeout -k 30 300 "$CLI" render "$doc" --outdir "$dir" >"$dir/log" 2>&1; then
        printf '%s\t%s\tok\n' "$key" "$doc"
    else
        printf '%s\t%s\tFAILED\n' "$key" "$doc"
    fi
}

i=0
while IFS= read -r doc; do
    [ -n "$doc" ] || continue
    render "$doc" "$OUT" &
    i=$((i + 1))
    [ $((i % JOBS)) -eq 0 ] && wait
done < "$LIST"
wait
