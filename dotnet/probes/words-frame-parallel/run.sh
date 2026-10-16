#!/bin/sh
# Reproduces results.md. Renders `frame-parallel`/`frame-wrap` re-cuts through both references and
# reports where line 4 — the last line of the paragraph above the frame — begins.
#
#   ./run.sh <outdir>
#
# Assert the PDFs exist before reading them: soffice exits 0 when it converts nothing.
set -eu
OUT=${1:-out}
C=$(cd "$(dirname "$0")/../../tests/corpus/features" && pwd)
HERE=$(cd "$(dirname "$0")" && pwd)
REF26=/opt/libreoffice26.2/program/soffice
REF24=/usr/bin/soffice

mkdir -p "$OUT/src"
for y in -0.01 0 0.01; do
  sed "s/svg:y=\"[^\"]*\"/svg:y=\"${y}cm\"/" "$C/frame-parallel.fodt" > "$OUT/src/p_y$y.fodt"
  sed "s/svg:y=\"[^\"]*\"/svg:y=\"${y}cm\"/" "$C/frame-wrap.fodt"     > "$OUT/src/w_y$y.fodt"
done
for w in 2 4 6; do
  for x in 0.5 0.8 0.9 0.95 1 1.1 1.5; do
    sed -e "s/svg:x=\"[^\"]*\"/svg:x=\"${x}cm\"/" -e "s/svg:width=\"[^\"]*\"/svg:width=\"${w}cm\"/" \
        -e "s/svg:y=\"[^\"]*\"/svg:y=\"0cm\"/" \
        "$C/frame-parallel.fodt" > "$OUT/src/w${w}_x${x}.fodt"
  done
done

for pair in "$REF26:26" "$REF24:24"; do
  bin=${pair%%:*}; tag=${pair##*:}
  [ -x "$bin" ] || { echo "no $bin"; continue; }
  prof=$(mktemp -d)
  "$bin" -env:UserInstallation="file://$prof" --headless --norestore \
      --convert-to pdf --outdir "$OUT/$tag" "$OUT"/src/*.fodt >/dev/null 2>&1
  n=$(find "$OUT/$tag" -name '*.pdf' | wc -l)
  [ "$n" -gt 0 ] || { echo "$tag produced no PDF"; exit 1; }
  echo "== LibreOffice $tag =="
  for f in "$OUT/$tag"/*.pdf; do python3 "$HERE/readline4.py" "$f"; done
done
