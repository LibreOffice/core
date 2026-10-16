#!/usr/bin/env bash
# Render every variant through the reference, three at a time.
#   ./render-ref.sh <variant-dir> <outdir> [jobs]
set -uo pipefail
IN=${1:?variants}; OUT=${2:?outdir}; JOBS=${3:-3}
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
export SOURCE_DATE_EPOCH=0
mkdir -p "$OUT"
echo "reference $REF -- $("$REF" --version 2>/dev/null | head -1)"
one() {
  f="$1"; out="$2"; ref="$3"; s=$(basename "$f" .docx)
  [ -f "$out/$s.pdf" ] && return 0
  timeout 180 "$ref" -env:UserInstallation=file:///tmp/lo-p3-$s \
     --headless --convert-to pdf --outdir "$out/w-$s" "$f" >/dev/null 2>&1
  mv "$out/w-$s/$s.pdf" "$out/$s.pdf" 2>/dev/null; rm -rf "$out/w-$s"
}
export -f one
ls "$IN"/*.docx | xargs -d '\n' -P "$JOBS" -I{} bash -c 'one "$@"' _ {} "$OUT" "$REF"
echo DONE
