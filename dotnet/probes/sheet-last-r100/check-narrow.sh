#!/bin/bash
set -u
DIFF=/home/user/wt-sheetlast/.claude/skills/render-comparison/scripts/pdf-image-diff.py
while IFS= read -r doc; do
  base=$(basename "$doc"); stem="${base%.*}"; ext="${base##*.}"; id="${stem}__${ext}"
  case "$ext" in ods|ODS) R="/home/user/gate-odf-r80/ref/$id.pdf";; *) R="/home/user/gate-orig-r83/ref/$id.pdf";; esac
  [ -f "$R" ] || { echo -e "$id\tno-ref"; continue; }
  rm -rf nw2; mkdir -p nw2
  SOURCE_DATE_EPOCH=1757462400 /home/user/r100-work/cli-narrow/XRay.Office.Cli render "$doc" --outdir nw2 >/dev/null 2>&1
  f=$(ls nw2/*.pdf 2>/dev/null|head -1)
  v=$(python3 "$DIFF" "$f" "$R" 2>/dev/null | awk -F'\t' 'NF>=6&&$1+0>0{n++;s+=$4; if($6~/MAJOR/)m++} END{printf "%.2f\t%d\t%d",s,n,m+0}')
  printf '%s\t%s\n' "$id" "$v"
  rm -rf nw2
done
