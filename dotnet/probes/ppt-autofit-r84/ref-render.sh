#!/bin/sh
# One document through the 26.2.4.2 reference, into $2.
#
# Two traps, both recorded in dotnet/CLAUDE.md and both fatal without the guard:
# `soffice` truncates -env:UserInstallation at the first space, so the profile path
# is keyed on a hex digest and never on the document's name; and `timeout N soffice`
# does not bound soffice at all -- it execs oosplash, which ignores SIGTERM -- so the
# kill signal has to be armed with -k.
set -e
DOC="$1"; OUT="$2"
REF="${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}"
KEY=$(printf '%s' "$DOC$OUT" | md5sum | cut -c1-16)
PROF="/tmp/xray-office-lo-$KEY"
mkdir -p "$OUT" "$PROF"
timeout -k 30 240 "$REF" --headless --norestore \
    "-env:UserInstallation=file://$PROF" \
    --convert-to pdf --outdir "$OUT" "$DOC" >/dev/null 2>&1
test -f "$OUT/$(basename "${DOC%.*}").pdf" || { echo "NO OUTPUT for $DOC" >&2; exit 1; }
