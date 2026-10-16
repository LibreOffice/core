#!/bin/bash
# score.sh <list> <base-cli> <head-cli> <ref-dir> <out.tsv>
#
# Ink at two binaries against the banked 26.2.4.2 reference, for every document of the list.
# Ink is `|ink|%` from pdf-image-diff.py summed unsigned over the document's pages and MAJOR is
# that script's own verdict; a document whose page counts differ is reported by that script as no
# pages at all, and its 0.00 is an absence of measurement rather than a score. Each render is
# deleted as soon as it is scored: a 186-page workbook is a large fraction of what is free here,
# and on a full disk writes fail while deletes still succeed, so a sweep that runs out leaves
# truncated PDFs scoring as real differences.
set -u
LIST=$1; BASE=$2; HEAD=$3; REF=$4; OUT=$5
DIFF=/home/user/wt-pivotres/.claude/skills/render-comparison/scripts/pdf-image-diff.py
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
export SOURCE_DATE_EPOCH=1700000000
score () {   # score <cli-dir> <document> <reference pdf>
  d=$W/pv; rm -rf "$d"; mkdir -p "$d"
  "$1/XRay.Office.Cli" render --format pdf --outdir "$d" --quiet "$2" >/dev/null 2>&1
  f=$(ls "$d"/*.pdf 2>/dev/null | head -1)
  if [ -z "$f" ] || ! tail -c 2048 "$f" | grep -q '%%EOF'; then echo "FAILED 0 0"; rm -rf "$d"; return; fi
  python3 "$DIFF" "$f" "$3" 2>/dev/null \
    | awk -F'\t' 'NF>=6 && $1+0>0 {n++; s+=$4; if($6~/MAJOR/) m++} END{printf "%.2f %d %d\n", s, n, m+0}'
  rm -rf "$d"
}
printf 'document\tpages\tbaseInk\theadInk\tdelta\tbaseMAJOR\theadMAJOR\n' > "$OUT"
while IFS= read -r doc; do
  [ -n "$doc" ] || continue
  b=$(basename "$doc"); stem="${b%.*}"; ext="${b##*.}"
  R="$REF/${stem}__${ext}.pdf"
  [ -f "$R" ] || { printf '%s\tNO-REFERENCE\n' "$stem" >> "$OUT"; continue; }
  read -r bi bp bm <<<"$(score "$BASE" "$doc" "$R")"
  read -r hi hp hm <<<"$(score "$HEAD" "$doc" "$R")"
  p="$bp"; [ "$bp" = "$hp" ] || p="$bp/$hp"
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "$stem" "$p" "$bi" "$hi" "$(python3 -c "print('%+.2f'%($hi-$bi))")" "$bm" "$hm" >> "$OUT"
done < "$LIST"
cat "$OUT"
