#!/usr/bin/env bash
# reach.sh <list> <base-cli> <head-cli> <ref-dir> <out.tsv>
#
# The reach measurement `batch-check.sh` makes, taken against the *banked* 26.2.4.2 reference
# rather than re-rendering it: page count first, then alphanumeric characters — column 9's
# `glyphs`, which is the metric that gate scores on and the only one of its three that a
# formatting change can move. A token count is not a reach figure and is not printed.
#
# Each render is deleted as soon as it is counted; on a full disk writes fail while deletes
# still succeed, so the sweep stops itself while there is still room to write.
set -u
LIST=$1; BASE=$2; HEAD=$3; REF=$4; OUT=$5
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
export SOURCE_DATE_EPOCH=1700000000

glyphs () {   # glyphs <pdf>  ->  "<pages> <alphanumeric characters>"
  local p
  p=$(pdfinfo "$1" 2>/dev/null | awk '/^Pages:/{print $2}')
  local g
  g=$(pdftotext -q "$1" - 2>/dev/null | python3 -c '
import sys
print(sum(1 for c in sys.stdin.read() if c.isalnum()))')
  echo "${p:-0} ${g:-0}"
}

render () {   # render <cli-dir> <document>  ->  "<pages> <glyphs>" or "0 0"
  local d=$W/pv; rm -rf "$d"; mkdir -p "$d"
  "$1/XRay.Office.Cli" render --format pdf --outdir "$d" --quiet "$2" >/dev/null 2>&1
  local f; f=$(ls "$d"/*.pdf 2>/dev/null | head -1)
  if [ -z "$f" ] || ! tail -c 2048 "$f" | grep -q '%%EOF'; then echo "0 0"; rm -rf "$d"; return; fi
  glyphs "$f"
  rm -rf "$d"
}

printf 'document\trefPages\trefGlyphs\tbasePages\tbaseGlyphs\theadPages\theadGlyphs\n' > "$OUT"
while IFS= read -r doc; do
  [ -n "$doc" ] || continue
  free=$(df -Pk / | awk 'NR==2{print $4}')
  if [ "$free" -lt 1048576 ]; then printf 'STOPPED\tless than 1 GiB free\n' >> "$OUT"; break; fi
  b=$(basename "$doc"); stem="${b%.*}"; ext="${b##*.}"
  R="$REF/${stem}__${ext}.pdf"
  if [ -f "$R" ]; then read -r rp rg <<<"$(glyphs "$R")"; else rp=-1; rg=-1; fi
  read -r bp bg <<<"$(render "$BASE" "$doc")"
  read -r hp hg <<<"$(render "$HEAD" "$doc")"
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$stem" "$rp" "$rg" "$bp" "$bg" "$hp" "$hg" >> "$OUT"
done < "$LIST"
cat "$OUT"
