#!/bin/sh
# Render one document through the reference 26.2.4.2 into $2, each run in its own profile.
# Never trust the exit status alone: check the output exists and is non-empty.
set -u
REF=/opt/libreoffice26.2/program/soffice
IN="$1"; OUT="$2"
mkdir -p "$OUT"
P=$(mktemp -d /tmp/lo-vline-XXXXXX)
timeout -k 30 900 "$REF" --headless --norestore -env:UserInstallation=file://"$P" \
   --convert-to pdf --outdir "$OUT" "$IN" >"$OUT/.log" 2>&1
rc=$?
base=$(basename "$IN"); stem=${base%.*}
if [ ! -s "$OUT/$stem.pdf" ]; then
  echo "FAILED rc=$rc $IN"; cat "$OUT/.log"; exit 1
fi
echo "ok rc=$rc $(stat -c %s "$OUT/$stem.pdf") bytes  $OUT/$stem.pdf"
