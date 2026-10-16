#!/usr/bin/env bash
# Render one document through a chosen soffice into an output directory.
#
# Two traps this exists to avoid, both recorded in dotnet/CLAUDE.md:
#   * soffice truncates -env:UserInstallation at the first space, so the profile
#     path is keyed on a hex digest and never on the document's name;
#   * `timeout N soffice` does not bound soffice -- it execs oosplash, which
#     ignores SIGTERM -- so the kill-after form is mandatory.
set -uo pipefail
SOF="${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}"
doc="$1"; out="$2"
mkdir -p "$out"
key=$(printf '%s' "$doc$out" | sha256sum | cut -c1-16)
prof="/tmp/xray-office-lo-$key"
rm -rf "$prof"; mkdir -p "$prof"
timeout -k 30 240 "$SOF" --headless --norestore --nolockcheck \
    "-env:UserInstallation=file://$prof" \
    --convert-to pdf --outdir "$out" "$doc" >/dev/null 2>&1
rc=$?
rm -rf "$prof"
exit $rc
