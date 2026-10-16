#!/bin/sh
# Render one document with the 26.2.4.2 reference, in a profile keyed on a hex digest
# of the document name — `soffice` truncates -env:UserInstallation at the first space.
set -e
SOFFICE=/opt/libreoffice26.2/program/soffice
doc="$1"; out="$2"
d=$(printf '%s' "$doc" | md5sum | cut -c1-16)
prof="/tmp/xray-office-lo-$d"
mkdir -p "$out"
timeout -k 30 240 "$SOFFICE" --headless --norestore -env:UserInstallation="file:///tmp/xray-office-lo-$d" \
  --convert-to pdf --outdir "$out" "$doc" >/dev/null 2>&1
rm -rf "$prof"
