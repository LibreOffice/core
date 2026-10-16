#!/usr/bin/env bash
# Render every variant both ways and count the paths each page carries.
#
#   ./measure.sh <variant-dir> <work-dir>
#
# A path count is the instrument here rather than |ink|% because the question is binary --
# *is a chart drawn at all* -- and a bar chart's 57 paths against nothing is not a tolerance
# question. Both counts come out of the PDF's own operators, so no rasteriser is involved.
set -uo pipefail
IN=${1:?variant dir}
OUT=${2:?work dir}
CLI=${XRAY_OFFICE_CLI:-/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
export SOURCE_DATE_EPOCH=0

echo "reference $REF -- $("$REF" --version 2>/dev/null | head -1)"
echo "measuring $CLI"
mkdir -p "$OUT/ref" "$OUT/ours"
printf 'class\trefpaths\tourpaths\n'

for f in "$IN"/k-*.fods; do
  stem=$(basename "$f" .fods)
  [ -f "$OUT/ref/$stem.pdf" ] || {
    timeout 180 "$REF" -env:UserInstallation=file:///tmp/lo-cc-$stem \
      --headless --convert-to pdf --outdir "$OUT/r-$stem" "$f" >/dev/null 2>&1
    mv "$OUT/r-$stem/$stem.pdf" "$OUT/ref/$stem.pdf" 2>/dev/null; rm -rf "$OUT/r-$stem"; }
  rm -rf "$OUT/ours/$stem"
  timeout 180 "$CLI" render --quiet --outdir "$OUT/ours/$stem" "$f" >/dev/null 2>&1
  o=$(ls "$OUT/ours/$stem"/*.pdf 2>/dev/null | head -1)
  printf '%s\t%s\t%s\n' "${stem#k-}" \
    "$(python3 "$(dirname "$0")/countpaths.py" "$OUT/ref/$stem.pdf")" \
    "$(python3 "$(dirname "$0")/countpaths.py" "$o")"
done
