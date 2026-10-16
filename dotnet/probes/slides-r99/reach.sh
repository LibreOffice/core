#!/usr/bin/env bash
# For each document: render at base and at head, compare the two PDFs byte for byte, and
# where they differ score each against the banked 26.2.4.2 reference with pdf-image-diff.
# One document at a time, deleted as it goes -- this container has about three gigabytes free.
set -uo pipefail
S=/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/r99
LIST="$1"; OUT="$2"
export SOURCE_DATE_EPOCH=1700000000
: > "$OUT"
T=$(mktemp -d)
while read -r f; do
  [ -n "$f" ] || continue
  base="$(basename "$f")"; stem="${base%.*}"; ext="${base##*.}"; id="${stem}__${ext,,}"
  r="/home/user/gate-orig-r83/ref/$id.pdf"
  rm -rf "$T/b" "$T/h"; mkdir -p "$T/b" "$T/h"
  timeout -k 30 300 "$S/cli-base/XRay.Office.Cli" render "/home/user/sample-files/$f" --format pdf --outdir "$T/b" >/dev/null 2>&1
  timeout -k 30 300 "$S/cli-head/XRay.Office.Cli" render "/home/user/sample-files/$f" --format pdf --outdir "$T/h" >/dev/null 2>&1
  ob="$T/b/$stem.pdf"; oh="$T/h/$stem.pdf"
  if [ ! -f "$ob" ] || [ ! -f "$oh" ]; then echo -e "$base\tFAILED" >> "$OUT"; continue; fi
  if cmp -s "$ob" "$oh"; then echo -e "$base\tidentical" >> "$OUT"; continue; fi
  if [ -f "$r" ]; then
    ib=$(python3 /home/user/wt-slides2/.claude/skills/render-comparison/scripts/pdf-image-diff.py "$ob" "$r" --outdir "$T/d" 2>/dev/null | awk -F'\t' '$1 ~ /^[0-9]+$/ {s+=$4} END{printf "%.2f", s}')
    ih=$(python3 /home/user/wt-slides2/.claude/skills/render-comparison/scripts/pdf-image-diff.py "$oh" "$r" --outdir "$T/d" 2>/dev/null | awk -F'\t' '$1 ~ /^[0-9]+$/ {s+=$4} END{printf "%.2f", s}')
    echo -e "$base\tmoved\t$ib\t$ih" >> "$OUT"
  else
    echo -e "$base\tmoved\tNOREF\tNOREF" >> "$OUT"
  fi
  rm -rf "$T/b" "$T/h" "$T/d"
done < "$LIST"
rm -rf "$T"
