#!/usr/bin/env bash
# Render a list of documents with $XRAY_OFFICE_CLI into <outdir>, one PDF per document.
#
#   XRAY_OFFICE_CLI=... confine.sh <list-file> <outdir> [workers]
#
# `SOURCE_DATE_EPOCH` is fixed so two runs of the same binary are byte-equal with nothing masked,
# which is what makes a byte comparison of two legs meaningful (`dotnet/CLAUDE.md`).  Each document
# renders into its own directory, keyed by the document rather than by a worker slot.
set -uo pipefail
LIST="${1:?list file}"; OUT="${2:?outdir}"; WORKERS="${3:-3}"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}"
[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
export SOURCE_DATE_EPOCH=1700000000
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"

mapfile -t FILES < "$LIST"
echo "${#FILES[@]} documents" >&2

one() {
  local idx="$1" i=-1 f base ext stem id t
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    t="$OUT/.t-$idx"
    rm -rf "$t"; mkdir -p "$t"
    timeout -k 30 300 "$CLI" render "$f" --format pdf --outdir "$t" >/dev/null 2>&1
    [ -f "$t/$stem.pdf" ] && mv -f "$t/$stem.pdf" "$OUT/$id.pdf"
    rm -rf "$t"
  done
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
ls "$OUT"/*.pdf 2>/dev/null | wc -l
