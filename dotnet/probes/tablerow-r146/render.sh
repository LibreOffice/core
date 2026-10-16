#!/bin/sh
# Render one document through 26.2.4.2 into its OWN profile and its own outdir.
# Refuses to report success unless the PDF exists and is non-empty.
set -u
REF=/opt/libreoffice26.2/program/soffice
IN="$1"; OUT="$2"
B=$(basename "$IN" .docx)
P=$(mktemp -d /tmp/lo-r146-XXXXXX)
mkdir -p "$OUT"
timeout -k 30 900 "$REF" --headless --norestore \
  -env:UserInstallation=file://"$P" \
  --convert-to pdf --outdir "$OUT" "$IN" >"$OUT/$B.log" 2>&1
rc=$?
if [ ! -s "$OUT/$B.pdf" ]; then
  echo "FAIL $B rc=$rc -- no output or empty; DO NOT COMPARE"; exit 1
fi
echo "OK   $B rc=$rc $(stat -c %s "$OUT/$B.pdf") bytes"
