#!/usr/bin/env bash
# Build the BIFF fixtures: 26.2.4.2's own `--convert-to xls` of the corpus chart workbooks.
#
#   ./make.sh <outdir>
#
# Converting rather than authoring is deliberate. A BIFF chart substream is a tree of nested
# record groups and a hand-built one answers whatever question its author had in mind; these
# are the reference's own encoding of real corpus content, so both renderers read identical
# bytes and every divergence on them is ours.
#
# Name the filter explicitly. A bare `--convert-to xls` writes a file the record scanner in
# `records.py` finds no chart in -- that is what `'xls:MS Excel 97'` is for.
set -uo pipefail
OUT=${1:?outdir}
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
CORPUS=${CORPUS:-/home/user/sample-files}
mkdir -p "$OUT"
for stem in 001_advanced_excel_bar 002_advanced_excel_line 003_advanced_excel_pie \
            005_advanced_excel_area 006_advanced_excel_scatter 007_advanced_excel_bubble \
            016_advanced_excel_radar 028_advanced_excel_doughnut; do
  src=$(find "$CORPUS" -iname "$stem.xlsx" | head -1)
  [ -n "$src" ] || { echo "missing $stem.xlsx" >&2; continue; }
  timeout 180 "$REF" -env:UserInstallation=file:///tmp/lo-mk-$stem \
    --headless --convert-to 'xls:MS Excel 97' --outdir "$OUT" "$src" >/dev/null 2>&1
done
ls -1 "$OUT"
