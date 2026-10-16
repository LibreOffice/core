#!/bin/bash
# render-ours.sh <list-file> <outdir>
#
# Renders our half of a document list under SOURCE_DATE_EPOCH, one output directory per
# DOCUMENT rather than per worker slot, and prints `sha256  identity` for each. Two runs of
# this at two binaries are byte-comparable, which is what a confinement claim needs.
#
# Every PDF is checked for %%EOF before it is hashed: a render truncated by a full disk scores
# as a real difference rather than erroring.
set -u
LIST="$1"; OUT="$2"
: "${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}"
mkdir -p "$OUT"
while IFS= read -r doc; do
  [ -z "$doc" ] && continue
  stem=$(basename "$doc"); stem="${stem%.*}"
  ext="${doc##*.}"
  d="$OUT/${stem}__${ext,,}"
  mkdir -p "$d"
  SOURCE_DATE_EPOCH=1700000000 timeout -k 30 600 "$XRAY_OFFICE_CLI" render "$doc" \
      --format pdf --outdir "$d" >/dev/null 2>&1
  f="$d/$stem.pdf"
  if [ -s "$f" ] && tail -c 32 "$f" | grep -q '%%EOF'; then
    echo "$(sha256sum "$f" | cut -d' ' -f1)  ${stem}__${ext,,}"
  else
    echo "MISSING-OR-TRUNCATED  ${stem}__${ext,,}"
  fi
  rm -rf "$d"
done < "$LIST"
