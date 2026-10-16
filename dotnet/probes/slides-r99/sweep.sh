#!/usr/bin/env bash
# Render each .ppt, extract every text show's (font, size, baseline) from the content
# stream, pair it against the banked 26.2.4.2 reference, delete the rendering.
set -uo pipefail
CLI=/home/user/wt-slides2/dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli
export SOURCE_DATE_EPOCH=1700000000
OUT=/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/r99/shows
mkdir -p "$OUT" "$OUT/tmp"
while read -r f; do
  [ -n "$f" ] || continue
  base="$(basename "$f")"; stem="${base%.*}"
  [ -s "$OUT/$stem.ours.tsv" ] && continue
  rm -rf "$OUT/tmp/w"; mkdir -p "$OUT/tmp/w"
  timeout -k 30 300 "$CLI" render "/home/user/sample-files/$f" --format pdf --outdir "$OUT/tmp/w" >/dev/null 2>&1
  if [ -f "$OUT/tmp/w/$stem.pdf" ]; then
    python3 tfy.py "$OUT/tmp/w/$stem.pdf" > "$OUT/$stem.ours.tsv" 2>/dev/null
  else
    echo FAILED > "$OUT/$stem.ours.tsv"
  fi
  r="/home/user/gate-orig-r83/ref/${stem}__ppt.pdf"
  [ -f "$r" ] && python3 tfy.py "$r" > "$OUT/$stem.ref.tsv" 2>/dev/null
  rm -rf "$OUT/tmp/w"
done
rm -rf "$OUT/tmp"
