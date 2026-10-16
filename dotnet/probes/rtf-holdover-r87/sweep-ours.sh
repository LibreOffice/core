#!/usr/bin/env bash
# Taken unchanged from probes/rtf-resid-r80, so the two rounds' columns are comparable.
# Render our half of one extension of the converted-ODF corpus into <outdir>, named as the
# gate bank names it (<stem>__<ext>.pdf), so it can be scored against the banked reference.
set -u
ROOT=${ROOT:-/home/user/corpus-odf}
CLI=${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}
EXT=${1:?ext}
OUT=${2:?outdir}
JOBS=${3:-2}
mkdir -p "$OUT"
export TMPDIR=${TMPDIR:-/tmp}
find "$ROOT" -type f -iname "*.$EXT" -print0 |
  xargs -0 -P "$JOBS" -I{} bash -c '
    f="$1"; out="$2"; cli="$3"
    stem=$(basename "$f"); stem=${stem%.*}
    ext=$(basename "$f"); ext=${ext##*.}; ext=${ext,,}
    d=$(mktemp -d "$out/.w-XXXXXXXX")
    timeout -k 30 240 "$cli" render "$f" --format pdf --outdir "$d" >/dev/null 2>&1
    if [ -f "$d/$stem.pdf" ]; then mv -f "$d/$stem.pdf" "$out/${stem}__${ext}.pdf"; fi
    rm -rf "$d"
  ' _ {} "$OUT" "$CLI"
