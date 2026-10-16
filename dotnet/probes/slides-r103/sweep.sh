#!/usr/bin/env bash
# One .ppt at a time: render, score the gate's own columns (pages, alphanumeric characters)
# and summed |ink|% against the banked 26.2.4.2 reference, delete the rendering.
set -uo pipefail
CLI="$1"; LIST="$2"; OUT="$3"
export SOURCE_DATE_EPOCH=1700000000
: > "$OUT"
T=$(mktemp -d -p /home/user/r103-slidefw)
while read -r f; do
  [ -n "$f" ] || continue
  base="$(basename "$f")"; stem="${base%.*}"
  r="/home/user/gate-orig-r83/ref/${stem}__ppt.pdf"
  [ -f "$r" ] || { echo -e "$stem\tNOREF" >> "$OUT"; continue; }
  rm -rf "$T/w"; mkdir -p "$T/w"
  timeout -k 30 300 "$CLI" render "/home/user/sample-files/$f" --format pdf --outdir "$T/w" >/dev/null 2>&1
  o="$T/w/$stem.pdf"
  if [ -f "$o" ]; then
    op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
    rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
    og=$(pdftotext "$o" - 2>/dev/null | tr -cd '[:alnum:]' | wc -c)
    rg=$(pdftotext "$r" - 2>/dev/null | tr -cd '[:alnum:]' | wc -c)
    ink=$(python3 /home/user/wt-slidefw/.claude/skills/render-comparison/scripts/pdf-image-diff.py \
            "$o" "$r" --outdir "$T/d" 2>/dev/null \
          | awk -F'\t' '$1 ~ /^[0-9]+$/ {s+=$4; n++; if ($6=="MAJOR") m++} END{printf "%.2f\t%d\t%d", s, n, m+0}')
    md5=$(md5sum "$o" | cut -c1-32)
    echo -e "$stem\t$op\t$rp\t$og\t$rg\t$ink\t$md5" >> "$OUT"
  else
    echo -e "$stem\tFAILED" >> "$OUT"
  fi
  rm -rf "$T/w" "$T/d"
done < "$LIST"
rm -rf "$T"
