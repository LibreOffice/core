#!/bin/bash
# score-movers.sh <tsv> -- render each moving document at both binaries and score on ink
set -u
DIFF=/home/user/wt-sheetlast/.claude/skills/render-comparison/scripts/pdf-image-diff.py
printf 'identity\tbase|ink|%%\thead|ink|%%\tpages\tbaseMAJOR\theadMAJOR\n'
awk -F'\t' 'NR>1 && $2!=$3 {print $1}' "$1" | while IFS= read -r doc; do
  base=$(basename "$doc"); stem="${base%.*}"; ext="${base##*.}"
  id="${stem}__${ext}"
  case "$ext" in ods|ODS) R="/home/user/gate-odf-r80/ref/$id.pdf";; *) R="/home/user/gate-orig-r83/ref/$id.pdf";; esac
  if [ ! -f "$R" ]; then printf '%s\tno-ref\n' "$id"; continue; fi
  out=""
  for leg in base head; do
    d=/home/user/r100-work/mv/$leg; rm -rf "$d"; mkdir -p "$d"
    SOURCE_DATE_EPOCH=1757462400 /home/user/r100-work/cli-$leg/XRay.Office.Cli render "$doc" --outdir "$d" >/dev/null 2>&1
    f=$(ls "$d"/*.pdf 2>/dev/null | head -1)
    if [ -z "$f" ] || ! tail -c 2048 "$f" | grep -q '%%EOF'; then out="$out\tFAILED"; continue; fi
    v=$(python3 "$DIFF" "$f" "$R" 2>/dev/null | awk -F'\t' 'NF>=6 && $1+0>0 {n++; s+=$4; if($6~/MAJOR/) m++} END{printf "%.2f\t%d\t%d", s, n, m+0}')
    out="$out|$v"
  done
  b=$(echo "$out" | cut -d'|' -f2); h=$(echo "$out" | cut -d'|' -f3)
  bi=$(echo "$b" | cut -f1); bp=$(echo "$b" | cut -f2); bm=$(echo "$b" | cut -f3)
  hi=$(echo "$h" | cut -f1); hm=$(echo "$h" | cut -f3)
  printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$id" "$bi" "$hi" "$bp" "$bm" "$hm"
  rm -rf /home/user/r100-work/mv
done
