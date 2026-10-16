#!/bin/sh
# The same documents with the ROUND'S BASE binary under quirks, so the arm isolates this
# round's contribution to `XRAY_OFFICE_LIBREOFFICE_QUIRKS` from everything already behind it.
set -e
P="$(cd "$(dirname "$0")" && pwd)"
BASE=${BASE_CLI:-/home/user/wt-math165/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}
export SOURCE_DATE_EPOCH=0
while IFS= read -r f; do
  k=$(printf '%s' "$f" | md5sum | cut -c1-12)
  mkdir -p "$P/sweep/qbase/$k"
  XRAY_OFFICE_LIBREOFFICE_QUIRKS=1 "$BASE" render --outdir "$P/sweep/qbase/$k" "$f" >/dev/null 2>&1 || true
  echo "done $k $(basename "$f")"
done < "$P/quirk-movers.txt"
echo ALLDONE
