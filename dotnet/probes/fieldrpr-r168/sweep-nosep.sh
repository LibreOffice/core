#!/bin/sh
# Render the 59 documents that can move, at the round's base and at its head, plus the
# reference. One output directory per document, keyed on the md5 of the whole path, so a
# parallel run cannot have two documents in one directory (`dotnet/CLAUDE.md`).
set -e
P="$(cd "$(dirname "$0")" && pwd)"
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
BASE=${BASE_CLI:-/home/user/wt-math165/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}
HEAD=${HEAD_CLI:-/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}
echo "reference $REF -- $("$REF" --version 2>/dev/null | head -1)"
echo "base      $BASE"
echo "head      $HEAD"
export SOURCE_DATE_EPOCH=0
while IFS= read -r f; do
  k=$(printf '%s' "$f" | md5sum | cut -c1-12)
  for arm in ref base head; do mkdir -p "$P/nosep/$arm/$k"; done
  [ -f "$P/nosep/ref/$k/done" ] || {
    PATH=$(dirname "$REF"):$PATH timeout -k 30 600 soffice --headless --convert-to pdf \
      --outdir "$P/nosep/ref/$k" "$f" -env:UserInstallation=file:///tmp/xray-office-lo-fieldrpr-$k >/dev/null 2>&1 || true
    touch "$P/nosep/ref/$k/done"; }
  "$BASE" render --outdir "$P/nosep/base/$k" "$f" >/dev/null 2>&1 || true
  "$HEAD" render --outdir "$P/nosep/head/$k" "$f" >/dev/null 2>&1 || true
  echo "done $k $(basename "$f")"
done < "$P/nosep-movers.txt"
echo ALLDONE
