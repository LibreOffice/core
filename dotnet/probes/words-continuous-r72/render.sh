#!/bin/sh
# render.sh <variantdir> <outdir> — one soffice profile per document, keyed on a
# hex digest of the path (never on a worker slot, never on the path itself).
set -e
IN=$1; OUT=$2
CLI=${XRAY_OFFICE_CLI:-/home/user/wt-words69/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
mkdir -p "$OUT/ours" "$OUT/ref26"
for f in "$IN"/*.docx; do
  h=$(printf %s "$f" | md5sum | cut -c1-12)
  SOURCE_DATE_EPOCH=1700000000 "$CLI" render --quiet --outdir "$OUT/ours" "$f"
  "$REF" --headless -env:UserInstallation=file://${TMPDIR:-/tmp}/prof-$h \
      --convert-to pdf --outdir "$OUT/ref26" "$f" >/dev/null 2>&1
done
n=$(ls "$OUT/ours"/*.pdf 2>/dev/null | wc -l); m=$(ls "$OUT/ref26"/*.pdf 2>/dev/null | wc -l)
echo "ours=$n ref26=$m"
[ "$n" -gt 0 ] && [ "$m" -gt 0 ] || { echo "INSTRUMENT PRODUCED NOTHING"; exit 1; }
