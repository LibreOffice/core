#!/bin/sh
# Render every probe both ways and report page count plus the geometry each case asks about.
# $1 = probe directory, $2 = output directory, $3 = the XRay.Office.Cli to measure.
set -e
IN=$1; OUT=$2; CLI=$3
mkdir -p "$OUT/ours" "$OUT/ref26" "$OUT/prof"
for f in "$IN"/*.docx; do
  SOURCE_DATE_EPOCH=1700000000 "$CLI" render --quiet --outdir "$OUT/ours" "$f"
  /opt/libreoffice26.2/program/soffice -env:UserInstallation=file://$OUT/prof \
      --headless --convert-to pdf --outdir "$OUT/ref26" "$f" >/dev/null 2>&1
done
for f in "$IN"/*.docx; do
  b=$(basename "$f" .docx)
  for side in ours ref26; do
    p="$OUT/$side/$b.pdf"
    [ -f "$p" ] || { echo "$b $side MISSING"; continue; }
    echo "$b $side pages=$(pdfinfo "$p" | awk '/^Pages/{print $2}')"
  done
done
