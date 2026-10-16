#!/usr/bin/env bash
# Render one document to PDF with the 26.2.4.2 tarball, in a private profile,
# under a hard bound.  soffice execs oosplash, which ignores SIGTERM, so -k is
# not optional.
set -uo pipefail
SOF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
TMO=${RENDER_TIMEOUT:-900}
in=$1; out=$2
mkdir -p "$out"
prof=$(mktemp -d)
timeout -k 30 "$TMO" "$SOF" -env:UserInstallation="file://$prof" \
    --headless --norestore --invisible --nolockcheck \
    --convert-to pdf --outdir "$out" "$in" >/dev/null 2>&1
rc=$?
rm -rf "$prof"
stem=$(basename "$in"); stem=${stem%.*}
[ -s "$out/$stem.pdf" ] || { echo "FAILED $in (rc=$rc)" >&2; exit 1; }
echo "$out/$stem.pdf"
