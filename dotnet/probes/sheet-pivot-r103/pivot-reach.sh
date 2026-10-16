#!/bin/bash
# pivot-reach.sh -- the eleven pivot-bearing .xlsx/.xlsm scored whole, and, where a twin
# exists, the same twin with only its pivot-generated borders stripped out.
set -u
W=/home/user/r103-sheetpivot
DIFF=/home/user/wt-sheetpivot/.claude/skills/render-comparison/scripts/pdf-image-diff.py
score () {   # score <document> <reference pdf>
  d=$W/pv; rm -rf "$d"; mkdir -p "$d"
  SOURCE_DATE_EPOCH=1700000000 dotnet $W/cli-head/XRay.Office.Cli.dll render --format pdf --outdir "$d" --quiet "$1" >/dev/null 2>&1
  f=$(ls "$d"/*.pdf 2>/dev/null | head -1)
  if [ -z "$f" ] || ! tail -c 2048 "$f" | grep -q '%%EOF'; then echo "FAILED 0 0"; rm -rf "$d"; return; fi
  python3 "$DIFF" "$f" "$2" 2>/dev/null | awk -F'\t' 'NF>=6 && $1+0>0 {n++; s+=$4; if($6~/MAJOR/) m++} END{printf "%.2f %d %d\n", s, n, m+0}'
  rm -rf "$d"
}
printf 'document\tpages\txlsx|ink|%%\txlsxMAJOR\tods|ink|%%\todsMAJOR\tods-less-pivot-borders\tstrippedMAJOR\tborders\n'
while IFS= read -r doc; do
  [ -z "$doc" ] && continue
  base=$(basename "$doc"); stem="${base%.*}"; ext="${base##*.}"
  R="/home/user/gate-orig-r83/ref/${stem}__${ext}.pdf"
  [ -f "$R" ] || { printf '%s\tno-ref\n' "$stem"; continue; }
  read x xp xm <<<"$(score "$doc" "$R")"
  twin=$(ls "/home/user/corpus-odf/sheets/"*/ods/"$stem.ods" 2>/dev/null | head -1)
  if [ -z "$twin" ]; then
    printf '%s\t%s\t%s\t%s\t-\t-\t-\t-\t-\n' "$stem" "$xp" "$x" "$xm"; continue
  fi
  RO="/home/user/gate-odf-r80/ref/${stem}__ods.pdf"
  [ -f "$RO" ] || { printf '%s\t%s\t%s\t%s\tno-ods-ref\t-\t-\t-\t-\n' "$stem" "$xp" "$x" "$xm"; continue; }
  read o op om <<<"$(score "$twin" "$RO")"
  n=$(python3 $W/pivot-strip.py "$W/stripped.ods" "$twin" | cut -f1)
  read s sp sm <<<"$(score "$W/stripped.ods" "$RO")"
  rm -f "$W/stripped.ods"
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$stem" "$xp" "$x" "$xm" "$o" "$om" "$s" "$sm" "$n"
done < "$1"
