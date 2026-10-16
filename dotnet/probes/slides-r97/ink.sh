#!/usr/bin/env bash
# Summed |ink|% against the banked 26.2.4.2 reference, one document at a time, deleting as it goes.
set -uo pipefail
CLI="$1"; LIST="$2"; OUT="$3"
export SOURCE_DATE_EPOCH=1700000000
: > "$OUT"
T=$(mktemp -d)
while read -r f; do
  [ -n "$f" ] || continue
  base="$(basename "$f")"; stem="${base%.*}"; ext="${base##*.}"; id="${stem}__${ext,,}"
  r="/home/user/gate-orig-r83/ref/$id.pdf"
  [ -f "$r" ] || { echo -e "$base\tNOREF" >> "$OUT"; continue; }
  rm -rf "$T/w"; mkdir -p "$T/w"
  timeout -k 30 300 "$CLI" render "/home/user/sample-files/$f" --format pdf --outdir "$T/w" >/dev/null 2>&1
  o="$T/w/$stem.pdf"
  if [ -f "$o" ]; then
    python3 /home/user/wt-slidefont/.claude/skills/render-comparison/scripts/pdf-image-diff.py \
      "$o" "$r" --outdir "$T/d" 2>/dev/null \
      | awk -v id="$base" -F'\t' '$1 ~ /^[0-9]+$/ {s+=$4; n++; if ($6=="MAJOR") m++} END{printf "%s\t%.2f\t%d\t%d\n", id, s, n, m}' >> "$OUT"
  else
    echo -e "$base\tFAILED" >> "$OUT"
  fi
  rm -rf "$T/w" "$T/d"
done < "$LIST"
rm -rf "$T"
