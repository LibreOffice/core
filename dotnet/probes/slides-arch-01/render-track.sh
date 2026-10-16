#!/usr/bin/env bash
# Render the whole slides track with XRay.Office into $1, reproducibly.
set -uo pipefail
OUT="$1"; CLI="$2"
mkdir -p "$OUT"
export SOURCE_DATE_EPOCH=1700000000
find /c/sandbox/workdir/sample-files/slides -type f -print0 | sort -z | while IFS= read -r -d '' f; do
  base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
  id="${stem}__${ext,,}"
  [ -f "$OUT/$id.pdf" ] && continue
  t="$(mktemp -d)"
  timeout 240 "$CLI" render "$f" --format pdf --outdir "$t" >/dev/null 2>&1
  [ -f "$t/$stem.pdf" ] && mv -f "$t/$stem.pdf" "$OUT/$id.pdf"
  rm -rf "$t"
done
ls "$OUT" | wc -l
