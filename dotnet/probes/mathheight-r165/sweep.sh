#!/bin/sh
# Convert every fixture to flat ODF with 26.2.4.2 and keep its own resolved view.
# No rasteriser is involved: `svg:height` is the number this probe is after.
set -e
P="$(cd "$(dirname "$0")" && pwd)"
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
echo "reference $REF -- $("$REF" --version 2>/dev/null | head -1)"
mkdir -p "$P/fodt"
"$REF" --headless --convert-to fodt --outdir "$P/fodt" "$P"/fixtures/*.docx \
    -env:UserInstallation=file:///tmp/xray-office-lo-mathheight-r165 >/dev/null
ls "$P/fodt" | wc -l
