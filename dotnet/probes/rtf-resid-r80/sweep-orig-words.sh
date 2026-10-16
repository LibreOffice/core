#!/usr/bin/env bash
# Render the original corpus's words track with the binary under test, named as the gate bank
# names it, so it can be byte-compared against /home/user/gate-orig-r76/ours.
set -u
ROOT=${ROOT:-/home/user/sample-files}
CLI=${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}
OUT=${1:?outdir}
JOBS=${2:-2}
mkdir -p "$OUT"
find "$ROOT/words" -type f \( -iname '*.doc' -o -iname '*.docx' -o -iname '*.docm' \) -print0 |
  xargs -0 -P "$JOBS" -I{} bash -c '
    f="$1"; out="$2"; cli="$3"
    b=$(basename "$f"); stem=${b%.*}; ext=${b##*.}; ext=${ext,,}
    d=$(mktemp -d "$out/.w-XXXXXXXX")
    SOURCE_DATE_EPOCH=1700000000 timeout -k 30 240 "$cli" render "$f" --format pdf --outdir "$d" >/dev/null 2>&1
    [ -f "$d/$stem.pdf" ] && mv -f "$d/$stem.pdf" "$out/${stem}__${ext}.pdf"
    rm -rf "$d"
  ' _ {} "$OUT" "$CLI"
