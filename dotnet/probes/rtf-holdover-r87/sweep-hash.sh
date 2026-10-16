#!/usr/bin/env bash
# Render one track with the binary under test and print `<name>__<ext>\t<md5>` per
# document, unlinking each PDF as it is hashed. Taken from probes/rtf-htmautsp-r85's
# sweep-orig-hash.sh and generalised to any root and extension set, so the same
# instrument covers the original .doc/.docx track and the converted .odt column.
#
# SOURCE_DATE_EPOCH is what makes a plain md5 mean anything: `xray-office render`
# honours it in both the PDF's /CreationDate and a header's date fields, so two runs
# of one binary are byte-equal with nothing masked.
#
#   XRAY_OFFICE_CLI=… sweep-hash.sh <root> <ext[,ext…]> <out.tsv> [jobs]
set -u
CLI=${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}
ROOT=${1:?root}
EXTS=${2:?exts}
OUT=${3:?out.tsv}
JOBS=${4:-2}
ARGS=()
IFS=, read -ra LIST <<< "$EXTS"
for e in "${LIST[@]}"; do ARGS+=(-o -iname "*.$e"); done
WORK=$(mktemp -d "${TMPDIR:-/tmp}/hash-XXXXXXXX")
find "$ROOT" -type f \( "${ARGS[@]:1}" \) -print0 |
  xargs -0 -P "$JOBS" -I{} bash -c '
    f="$1"; work="$2"; cli="$3"
    b=$(basename "$f"); stem=${b%.*}; ext=${b##*.}; ext=${ext,,}
    d=$(mktemp -d "$work/w-XXXXXXXX")
    SOURCE_DATE_EPOCH=1700000000 timeout -k 30 240 "$cli" render "$f" --format pdf --outdir "$d" >/dev/null 2>&1
    if [ -f "$d/$stem.pdf" ]; then
      printf "%s__%s\t%s\n" "$stem" "$ext" "$(md5sum <"$d/$stem.pdf" | cut -d" " -f1)"
    else
      printf "%s__%s\tFAILED\n" "$stem" "$ext"
    fi
    rm -rf "$d"
  ' _ {} "$WORK" "$CLI" | sort > "$OUT"
rm -rf "$WORK"
wc -l < "$OUT"
