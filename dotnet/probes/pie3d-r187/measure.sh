#!/usr/bin/env bash
# Render the three corpus pie3DChart documents both ways and score them.
#   ./measure.sh <work-dir>
set -uo pipefail
OUT=${1:?work dir}
CLI=${XRAY_OFFICE_CLI:-/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
DIFF=${DIFF:-/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py}
CORPUS=${CORPUS:-/home/user/sample-files}
export SOURCE_DATE_EPOCH=0
mkdir -p "$OUT/ref" "$OUT/cmp"
echo "reference $REF -- $("$REF" --version 2>/dev/null | head -1)"
echo "measuring $CLI"
for stem in pie-chart-result pie-chart-template 021_Unit_Circle_Chart_3D_Pie_Chart_404247ab; do
  f=$(find "$CORPUS" -iname "$stem.docx" | head -1)
  [ -n "$f" ] || { echo "$stem missing"; continue; }
  [ -f "$OUT/ref/$stem.pdf" ] || {
    timeout 180 "$REF" -env:UserInstallation=file:///tmp/lo-p3m-$stem \
      --headless --convert-to pdf --outdir "$OUT/r-$stem" "$f" >/dev/null 2>&1
    mv "$OUT/r-$stem/$stem.pdf" "$OUT/ref/$stem.pdf" 2>/dev/null; rm -rf "$OUT/r-$stem"; }
  rm -rf "$OUT/ours/$stem"
  timeout 180 "$CLI" render --quiet --outdir "$OUT/ours/$stem" "$f" >/dev/null 2>&1
  o=$(ls "$OUT/ours/$stem"/*.pdf 2>/dev/null | head -1)
  [ -f "$OUT/ref/$stem.pdf" ] && [ -n "$o" ] || { printf '%s\tFAILED\n' "$stem"; continue; }
  timeout 300 python3 "$DIFF" "$o" "$OUT/ref/$stem.pdf" --outdir "$OUT/c-$stem" > "$OUT/cmp/$stem.txt" 2>&1
  rm -rf "$OUT/c-$stem"
  awk -F'\t' '$1 ~ /^[0-9]+$/ {if($2+0>d)d=$2+0; if($4+0>i)i=$4+0}
    END{printf "%-46s diff%% %6.2f   |ink|%% %6.2f\n", s, d, i}' s="$stem" "$OUT/cmp/$stem.txt"
done
