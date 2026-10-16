#!/bin/sh
# ink.sh <listfile> <outdir> — render the named corpus documents through 26.2.4.2 and
# rasterise the reference and both of our halves for compare-images.py.
# One soffice profile per document, keyed on a hex digest of the document's own path.
set -e
LIST=$1; OUT=$2
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
mkdir -p "$OUT/ref26"
while IFS= read -r doc; do
  [ -n "$doc" ] || continue
  h=$(printf %s "$doc" | md5sum | cut -c1-12)
  "$REF" --headless -env:UserInstallation=file://${TMPDIR:-/tmp}/prof-$h \
      --convert-to pdf --outdir "$OUT/ref26" "$doc" >/dev/null 2>&1
done < "$LIST"
n=$(ls "$OUT/ref26"/*.pdf 2>/dev/null | wc -l)
echo "ref26 pdfs: $n"
[ "$n" -gt 0 ] || { echo "INSTRUMENT PRODUCED NOTHING"; exit 1; }
