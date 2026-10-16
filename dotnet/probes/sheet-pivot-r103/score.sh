#!/bin/bash
# score.sh <sweep.tsv> -- re-render each mover at both binaries and score it on ink.
set -u
W=/home/user/r103-sheetpivot
DIFF=/home/user/wt-sheetpivot/.claude/skills/render-comparison/scripts/pdf-image-diff.py
printf 'document\tpages\tbase|ink|%%\thead|ink|%%\tbaseMAJOR\theadMAJOR\n'
awk -F'\t' 'NR>1 && $2!=$3 && $2!="FAILED" && $3!="FAILED" {print $1}' "$1" | while IFS= read -r id; do
  doc=$(grep -F "/$id" $W/xls.list | head -1)
  stem="${id%.*}"; ext="${id##*.}"
  R="/home/user/gate-orig-r83/ref/${stem}__${ext}.pdf"
  [ -f "$R" ] || { printf '%s\tno-ref\n' "$id"; continue; }
  line="$id"
  for leg in base head; do
    d=$W/sc/$leg; rm -rf "$d"; mkdir -p "$d"
    SOURCE_DATE_EPOCH=1700000000 dotnet $W/cli-$leg/XRay.Office.Cli.dll render --format pdf --outdir "$d" --quiet "$doc" >/dev/null 2>&1
    f=$(ls "$d"/*.pdf 2>/dev/null | head -1)
    if [ -z "$f" ] || ! tail -c 2048 "$f" | grep -q '%%EOF'; then line="$line|FAILED"; continue; fi
    v=$(python3 "$DIFF" "$f" "$R" 2>/dev/null | awk -F'\t' 'NF>=6 && $1+0>0 {n++; s+=$4; if($6~/MAJOR/) m++} END{printf "%d %.2f %d", n, s, m+0}')
    line="$line|$v"
  done
  b=$(echo "$line" | cut -d'|' -f2); h=$(echo "$line" | cut -d'|' -f3)
  printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$id" "$(echo $b|cut -d' ' -f1)" "$(echo $b|cut -d' ' -f2)" "$(echo $h|cut -d' ' -f2)" "$(echo $b|cut -d' ' -f3)" "$(echo $h|cut -d' ' -f3)"
  rm -rf $W/sc
done
