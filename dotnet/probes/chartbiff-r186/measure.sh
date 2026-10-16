#!/usr/bin/env bash
# Score every BIFF fixture against 26.2.4.2.
#
#   ./measure.sh <fixture-dir> <work-dir>
#
# `worst` is the worst single page's unsigned |ink|% and `sum` is the per-page figure summed.
# Rank on `worst`: a sum is length-weighted and these are all three pages, so here the two
# agree -- which is the only reason both are printed.
set -uo pipefail
IN=${1:?fixture dir}
OUT=${2:?work dir}
CLI=${XRAY_OFFICE_CLI:-/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
DIFF=${DIFF:-/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py}
export SOURCE_DATE_EPOCH=0

echo "reference $REF -- $("$REF" --version 2>/dev/null | head -1)"
echo "measuring $CLI"
mkdir -p "$OUT/ref" "$OUT/cmp"
for f in "$IN"/*.xls; do
  stem=$(basename "$f" .xls)
  [ -f "$OUT/ref/$stem.pdf" ] || {
    timeout 180 "$REF" -env:UserInstallation=file:///tmp/lo-bf-$stem \
      --headless --convert-to pdf --outdir "$OUT/r-$stem" "$f" >/dev/null 2>&1
    mv "$OUT/r-$stem/$stem.pdf" "$OUT/ref/$stem.pdf" 2>/dev/null; rm -rf "$OUT/r-$stem"; }
  rm -rf "$OUT/ours/$stem"
  timeout 180 "$CLI" render --quiet --outdir "$OUT/ours/$stem" "$f" >/dev/null 2>&1
  o=$(ls "$OUT/ours/$stem"/*.pdf 2>/dev/null | head -1)
  [ -f "$OUT/ref/$stem.pdf" ] && [ -n "$o" ] || { printf '%s\tFAILED\n' "$stem"; continue; }
  timeout 300 python3 "$DIFF" "$o" "$OUT/ref/$stem.pdf" --outdir "$OUT/c-$stem" > "$OUT/cmp/$stem.txt" 2>&1
  rm -rf "$OUT/c-$stem"
  awk -F'\t' '$1 ~ /^[0-9]+$/ && $4 ~ /^[0-9.]+$/ {t+=$4; if($4+0>m)m=$4+0; n++}
    END{printf "%-34s worst %6.2f  sum %6.2f  pages %d\n", s, m, t, n}' s="$stem" "$OUT/cmp/$stem.txt"
done
