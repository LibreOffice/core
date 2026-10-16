#!/usr/bin/env bash
# Reference render with 26.2.4.2 ONLY. Absolute -env:UserInstallation, private profile,
# timeout -k so oosplash cannot wedge the run.
set -uo pipefail
SOF=/opt/libreoffice26.2/program/soffice
IN="${1:?input}"
OUT="${2:?outdir}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
PROF="$(mktemp -d /tmp/r154-prof.XXXXXX)"
timeout -k 30 900 "$SOF" --headless --norestore \
  "-env:UserInstallation=file://$PROF" \
  --convert-to pdf --outdir "$OUT" "$IN"
rc=$?
rm -rf "$PROF"
exit $rc
