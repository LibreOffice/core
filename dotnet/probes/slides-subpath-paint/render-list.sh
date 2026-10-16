#!/usr/bin/env bash
# Render the documents named on stdin with `xray-office render` into <outdir>, deterministically.
#
#   affected.py <corpus> | render-list.sh <outdir>
#
# `SOURCE_DATE_EPOCH` is what makes two runs byte-comparable: `xray-office render` honours the
# reproducible-builds convention in the PDF's `/CreationDate`, so with it set the only difference
# between a run of the old binary and a run of the new one is the ink.
set -uo pipefail
OUT="${1:?output directory}"
CLI="${XRAY_OFFICE_CLI:?XRAY_OFFICE_CLI must name the binary to measure}"
export SOURCE_DATE_EPOCH=1700000000
mkdir -p "$OUT"
while read -r f; do
    [ -n "$f" ] || continue
    stem="$(basename "${f%.*}")__${f##*.}"
    "$CLI" render "$f" --format pdf --outdir "$OUT/$stem" >/dev/null 2>&1
done
