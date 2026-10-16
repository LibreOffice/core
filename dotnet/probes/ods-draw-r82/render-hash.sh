#!/usr/bin/env bash
# Render one track and record a hash per document, deleting each PDF as it goes.
#
#   render-hash.sh <corpus-root> <track-glob> <out.tsv> [workers]
#
# For the "the original track does not move" check: two runs of this over the same documents with
# two binaries are comparable line for line, and the disk cost is one PDF at a time rather than a
# whole track's worth — which matters on a container with a gigabyte free.
#
# The hash is of the PDF with its `/CreationDate` and XMP `dc:date` masked, because `soffice` and
# `xray-office` both stamp the wall clock into a rendering and two runs minutes apart would
# otherwise differ everywhere. `SOURCE_DATE_EPOCH` is set for the same reason and belts the
# braces: a spreadsheet header holding `&D` prints today's date into the ink itself.
set -uo pipefail
ROOT="${1:?corpus root}"; GLOB="${2:?track glob, e.g. sheets}"; OUT="${3:?out.tsv}"
WORKERS="${4:-2}"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the binary you mean to measure}"
[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
export SOURCE_DATE_EPOCH=1700000000

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
: > "$OUT"

mapfile -t FILES < <(
  find "$ROOT/$GLOB" -type f \
    \( -iname '*.xls' -o -iname '*.xlsx' -o -iname '*.xlsm' -o -iname '*.xlsb' \
    -o -iname '*.xlt' -o -iname '*.xltx' -o -iname '*.xltm' -o -iname '*.ods' \
    -o -iname '*.ots' -o -iname '*.fods' -o -iname '*.csv' -o -iname '*.sxc' \) \
    -printf '%D:%i\t%p\n' 2>/dev/null | awk -F'\t' '!seen[$1]++ {print $2}' | sort
)
echo "${#FILES[@]} documents" >&2
echo "measuring $CLI" >&2

one() {
  local idx="$1" i=-1 f base ext stem pdf pages glyphs hash
  mkdir -p "$TMP/w$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    rm -rf "${TMP:?}/w$idx"; mkdir -p "$TMP/w$idx"
    timeout -k 30 240 "$CLI" render "$f" --format pdf --outdir "$TMP/w$idx" >/dev/null 2>&1
    pdf="$TMP/w$idx/$stem.pdf"
    if [ -f "$pdf" ]; then
      pages=$(pdfinfo "$pdf" 2>/dev/null | awk '/^Pages/{print $2}')
      glyphs=$(pdftotext "$pdf" - 2>/dev/null | python3 -c \
        'import sys;b=sys.stdin.buffer.read().decode("utf-8","replace");print(sum(1 for c in b if c.isalnum()))')
      # `xray-office` writes `/CreationDate(D:...)` with no space, so the pattern this inherited
      # -- which required one -- never matched and the mask was inert. What actually made two runs
      # comparable was SOURCE_DATE_EPOCH above; the pattern is corrected here so it is a real belt
      # rather than a decorative one. Round 82 found it by byte-comparing a fresh render against a
      # sweep's own copy and getting five differing bytes, all of them the timestamp.
      hash=$(LC_ALL=C sed -e 's|/CreationDate *([^)]*)|/CreationDate()|g' \
                          -e 's|<dc:date>[^<]*</dc:date>|<dc:date></dc:date>|g' "$pdf" \
             | md5sum | cut -d' ' -f1)
    else
      pages='-'; glyphs='-'; hash='FAILED'
    fi
    printf "%s\t%s\t%s\t%s\t%s\n" "${f#"$ROOT"/}" "${ext,,}" "$pages" "$glyphs" "$hash" >> "$OUT"
  done
  rm -rf "${TMP:?}/w$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
sort -o "$OUT" "$OUT"
wc -l < "$OUT"
