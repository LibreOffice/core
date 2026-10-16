#!/usr/bin/env bash
# Render one document through 26.2.4.2 into $2, with a profile keyed on a hex digest of the
# document's own name — `soffice` truncates -env:UserInstallation at the first space, so a
# path carrying the document name is silently unusable.
set -euo pipefail
SRC="$1"; OUT="$2"
mkdir -p "$OUT"
H=$(printf '%s' "$SRC" | md5sum | cut -d' ' -f1)
P="/tmp/lo-prof-$H"
mkdir -p "$P"
timeout 900 /opt/libreoffice26.2/program/soffice -env:UserInstallation="file://$P" \
  --headless --convert-to pdf --outdir "$OUT" "$SRC" >/dev/null 2>&1
ls -1 "$OUT"
