#!/bin/sh
# The documents that can move only under `XRAY_OFFICE_LIBREOFFICE_QUIRKS` — those whose field
# instructions carry `\* MERGEFORMAT`, where the default follows Word and keeps the cached
# result's formatting. One output directory per document, keyed on the md5 of the whole path.
set -e
P="$(cd "$(dirname "$0")" && pwd)"
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
HEAD=${HEAD_CLI:-/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}
echo "reference $REF -- $("$REF" --version 2>/dev/null | head -1)"
export SOURCE_DATE_EPOCH=0
while IFS= read -r f; do
  k=$(printf '%s' "$f" | md5sum | cut -c1-12)
  for arm in qref qoff qon; do mkdir -p "$P/sweep/$arm/$k"; done
  [ -f "$P/sweep/qref/$k/done" ] || {
    PATH=$(dirname "$REF"):$PATH timeout -k 30 600 soffice --headless --convert-to pdf \
      --outdir "$P/sweep/qref/$k" "$f" -env:UserInstallation=file:///tmp/xray-office-lo-fieldq-$k >/dev/null 2>&1 || true
    touch "$P/sweep/qref/$k/done"; }
  "$HEAD" render --outdir "$P/sweep/qoff/$k" "$f" >/dev/null 2>&1 || true
  XRAY_OFFICE_LIBREOFFICE_QUIRKS=1 "$HEAD" render --outdir "$P/sweep/qon/$k" "$f" >/dev/null 2>&1 || true
  echo "done $k $(basename "$f")"
done < "$P/quirk-movers.txt"
echo ALLDONE
