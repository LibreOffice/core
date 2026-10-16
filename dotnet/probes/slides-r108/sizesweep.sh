#!/usr/bin/env bash
# Render each .ppt with our CLI, record its per-page dominant drawn text size, delete the PDF.
set -uo pipefail
CLI="$1"; LIST="$2"; OUT="$3"; T="$4"
export SOURCE_DATE_EPOCH=1700000000
: > "$OUT"
mkdir -p "$T"
while IFS= read -r f; do
  [ -n "$f" ] || continue
  base="$(basename "$f")"; stem="${base%.*}"
  rm -rf "$T/w"; mkdir -p "$T/w"
  timeout -k 30 300 "$CLI" render "/home/user/sample-files/$f" --format pdf --outdir "$T/w" >/dev/null 2>&1
  o="$T/w/$stem.pdf"
  if [ -f "$o" ]; then
    python3 /home/user/wt-slidesize/dotnet/probes/slides-r107/sizes.py "$o" "${stem}__ppt" >> "$OUT"
  else
    echo -e "${stem}__ppt\tFAILED\t0\t0" >> "$OUT"
  fi
  rm -rf "$T/w"
done < "$LIST"
rm -rf "$T"
echo SWEEPDONE
