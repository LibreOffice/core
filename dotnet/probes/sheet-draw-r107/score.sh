#!/bin/bash
# score.sh <sweep.tsv> <list> -- re-render each mover at both binaries and score it on ink
# against its banked 26.2.4.2 reference.
set -u
W=${W:-/home/user/r107-sheetdraw}
DIFF=/home/user/wt-sheetdraw/.claude/skills/render-comparison/scripts/pdf-image-diff.py
BANK=/home/user/gate-orig-r83/ref
printf 'document\tpages\tbase|ink|%%\thead|ink|%%\tbaseMAJOR\theadMAJOR\n'
awk -F'\t' 'NR>1 && $2!=$3 && $2!="FAILED" && $3!="FAILED" {print $1}' "$1" | while IFS= read -r id; do
  doc=$(grep -F "/$id" "$2" | head -1)
  stem="${id%.*}"; ext="${id##*.}"
  R="$BANK/${stem}__${ext}.pdf"
  [ -f "$R" ] || { printf '%s\tno-ref\n' "$id"; continue; }
  line="$id"
  for leg in base head; do
    d=$(mktemp -d "$W/sc-XXXXXX")
    SOURCE_DATE_EPOCH=1700000000 dotnet "$W/cli-$leg/XRay.Office.Cli.dll" \
        render --format pdf --outdir "$d" --quiet "$doc" >/dev/null 2>&1
    f=$(ls "$d"/*.pdf 2>/dev/null | head -1)
    if [ -z "$f" ] || ! tail -c 2048 "$f" | grep -q '%%EOF'; then
      line="$line|FAILED"
    else
      v=$(python3 "$DIFF" "$f" "$R" 2>/dev/null \
          | awk -F'\t' 'NF>=6 && $1+0>0 {n++; s+=$4; if($6~/MAJOR/) m++} END{printf "%d %.2f %d", n, s, m+0}')
      line="$line|$v"
    fi
    rm -rf "$d"
  done
  b=$(echo "$line" | cut -d'|' -f2); h=$(echo "$line" | cut -d'|' -f3)
  printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$id" \
      "$(echo "$b" | cut -d' ' -f1)" "$(echo "$b" | cut -d' ' -f2)" \
      "$(echo "$h" | cut -d' ' -f2)" "$(echo "$b" | cut -d' ' -f3)" "$(echo "$h" | cut -d' ' -f3)"
done
