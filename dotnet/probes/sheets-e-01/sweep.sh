#!/bin/sh
# Render the whole sheets track with one CLI into one directory, with the clock pinned.
# $1 = output directory. Waits for every render before returning, and the caller re-counts
# from disk: a file count reaching its target is not the sweep having finished.
set -e
OUT="$1"
mkdir -p "$OUT"
export SOURCE_DATE_EPOCH=1700000000
export TZ=UTC
find /c/sandbox/workdir/sample-files/sheets -type f \
  \( -iname '*.xls' -o -iname '*.xlsx' \) | sort |
while read -r f; do
  "$XRAY_OFFICE_CLI" render --quiet --outdir "$OUT" "$f" >/dev/null 2>&1 || echo "FAILED $f" >&2
done
echo "wrote $(find "$OUT" -name '*.pdf' | wc -l) pdfs"
