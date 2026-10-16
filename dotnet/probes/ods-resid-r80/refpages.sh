#!/usr/bin/env bash
# refpages.sh <file.ods> -- render through 26.2.4.2 and print its page count.
set -uo pipefail
f="$1"
REF="${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}"
d=$(printf '%s' "$f" | md5sum | cut -c1-16)
prof="/tmp/plo-$d"
out="/tmp/plo-out-$d"
rm -rf "$out"; mkdir -p "$out" "$prof"
timeout -k 30 300 "$REF" -env:UserInstallation="file://$prof" \
   --headless --convert-to pdf --outdir "$out" "$f" >/dev/null 2>&1
p=$(ls "$out"/*.pdf 2>/dev/null | head -1)
if [ -z "$p" ]; then echo "FAILED"; else
  python3 -c "import pymupdf,sys; d=pymupdf.open(sys.argv[1]); print(len(d))" "$p"
fi
rm -rf "$out" "$prof"
