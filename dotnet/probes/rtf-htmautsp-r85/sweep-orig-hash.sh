#!/usr/bin/env bash
# Render the original corpus's words track with the binary under test and print one
# `<name>\t<md5>` line per document, unlinking each PDF as it is scored.
#
# `SOURCE_DATE_EPOCH` is what makes a plain md5 mean anything: `xray-office render`
# honours the reproducible-builds convention in both the PDF's /CreationDate and a
# header's date fields, so two runs of one binary are byte-equal with nothing masked.
# Disk on this container is measured in single gigabytes, which is why this hashes
# and unlinks rather than banking 338 PDFs twice.
#
#   XRAY_OFFICE_CLI=… sweep-orig-hash.sh <out.tsv> [jobs]
set -u
ROOT=${ROOT:-/home/user/sample-files}
CLI=${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}
OUT=${1:?out.tsv}
JOBS=${2:-3}
WORK=$(mktemp -d "${TMPDIR:-/tmp}/orig-XXXXXXXX")
find "$ROOT/words" -type f \( -iname '*.doc' -o -iname '*.docx' -o -iname '*.docm' \) -print0 |
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
