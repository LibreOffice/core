#!/usr/bin/env bash
# Render every wrap deck through both reference binaries and through our CLI.
#   run-wrap.sh <deck dir> <out dir> <tag for our render>
# The two reference legs are skipped when their PDF already exists, so re-running after a code
# change costs only our own leg.
set -uo pipefail
D="${1:?deck dir}"; P="${2:?out dir}"; TAG="${3:-run}"
LO24="${LO24:-soffice}"
LO26="${LO26:-/opt/libreoffice26.2/program/soffice}"
mkdir -p "$P/ref24" "$P/ref26" "$P/$TAG"
for f in "$D"/*.pptx; do
  b=$(basename "$f" .pptx)
  [ -f "$P/ref24/$b.pdf" ] || SOURCE_DATE_EPOCH=1700000000 TZ=UTC timeout 300 "$LO24" --headless \
      -env:UserInstallation=file://$P/prof24 --convert-to pdf --outdir "$P/ref24" "$f" >/dev/null 2>&1
  [ -f "$P/ref26/$b.pdf" ] || SOURCE_DATE_EPOCH=1700000000 TZ=UTC timeout 300 "$LO26" --headless \
      -env:UserInstallation=file://$P/prof26 --convert-to pdf --outdir "$P/ref26" "$f" >/dev/null 2>&1
  SOURCE_DATE_EPOCH=1700000000 TZ=UTC "$XRAY_OFFICE_CLI" render "$f" --format pdf --outdir "$P/$TAG" >/dev/null 2>&1
done
echo "ref24 $(ls "$P/ref24" | grep -c pdf)  ref26 $(ls "$P/ref26" | grep -c pdf)  $TAG $(ls "$P/$TAG" | grep -c pdf)"
