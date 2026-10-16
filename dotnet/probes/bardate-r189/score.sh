#!/usr/bin/env bash
set -uo pipefail
REF=/opt/libreoffice26.2/program/soffice
DIFF=/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py
OUT=/home/user/r191/sc; mkdir -p "$OUT/ref"
export SOURCE_DATE_EPOCH=0
for f in "$@"; do
  s=$(basename "$f"); s="${s%.*}"; id=$(printf '%s' "$f" | md5sum | cut -c1-12)
  [ -f "$OUT/ref/$id.pdf" ] || { timeout 240 "$REF" -env:UserInstallation=file:///tmp/lo-sc-$id \
      --headless --convert-to pdf --outdir "$OUT/r-$id" "$f" >/dev/null 2>&1
      mv "$OUT/r-$id/$s.pdf" "$OUT/ref/$id.pdf" 2>/dev/null; rm -rf "$OUT/r-$id"; }
  for leg in before after; do
    o=$(ls /home/user/r191/$leg/$id/*.pdf 2>/dev/null | head -1)
    [ -n "$o" ] && [ -f "$OUT/ref/$id.pdf" ] || { echo "$s $leg MISSING"; continue; }
    timeout 300 python3 "$DIFF" "$o" "$OUT/ref/$id.pdf" --outdir "$OUT/c" > "$OUT/$id-$leg.txt" 2>&1
    rm -rf "$OUT/c"
  done
  b=$(awk -F'\t' '$1 ~ /^[0-9]+$/ {if($2+0>d)d=$2+0} END{printf "%.2f", d}' "$OUT/$id-before.txt")
  a=$(awk -F'\t' '$1 ~ /^[0-9]+$/ {if($2+0>d)d=$2+0} END{printf "%.2f", d}' "$OUT/$id-after.txt")
  printf '%-52s worst diff%%  %6s -> %6s\n' "${s:0:50}" "$b" "$a"
done
