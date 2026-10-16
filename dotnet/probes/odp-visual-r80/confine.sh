#!/usr/bin/env bash
# Render every document of a column with two binaries and report which renderings differ.
#
#   confine.sh <corpus-root> <bank-rows.tsv> <ext> <base-cli> <head-cli> <outdir> [workers]
#
# The reach of a change, measured rather than argued: no reference is rendered, so this
# costs our half twice and nothing else, and a document whose two renderings are
# byte-identical cannot have moved a gate column.
#
# SOURCE_DATE_EPOCH is set on both legs so the PDF's /CreationDate is not the difference --
# `xray-office render` honours the reproducible-builds convention on both sides.
set -uo pipefail
ROOT="${1:?corpus root}"; ROWS="${2:?rows.tsv}"; EXT="${3:?ext}"
BASE="${4:?base cli}"; HEAD="${5:?head cli}"; OUT="${6:?outdir}"; WORKERS="${7:-2}"
export SOURCE_DATE_EPOCH=1700000000
[ -x "$BASE" ] || { echo "no base at $BASE" >&2; exit 1; }
[ -x "$HEAD" ] || { echo "no head at $HEAD" >&2; exit 1; }
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
: > "$OUT/differ.tsv"

mapfile -t FILES < <(awk -F'\t' -v e="$EXT" '$2==e{print $1}' "$ROWS")
echo "${#FILES[@]} $EXT documents" >&2

one() {
  local idx="$1" i=-1 rel f stem a b ha hb
  for rel in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    f="$ROOT/$rel"; stem="$(basename "${f%.*}")"
    rm -rf "${OUT:?}/a$idx" "${OUT:?}/b$idx"; mkdir -p "$OUT/a$idx" "$OUT/b$idx"
    timeout -k 30 240 "$BASE" render "$f" --format pdf --outdir "$OUT/a$idx" >/dev/null 2>&1
    timeout -k 30 240 "$HEAD" render "$f" --format pdf --outdir "$OUT/b$idx" >/dev/null 2>&1
    a="$OUT/a$idx/$stem.pdf"; b="$OUT/b$idx/$stem.pdf"
    ha="-"; hb="-"
    [ -f "$a" ] && ha=$(md5sum "$a" | cut -d' ' -f1)
    [ -f "$b" ] && hb=$(md5sum "$b" | cut -d' ' -f1)
    [ "$ha" = "$hb" ] || printf "%s\t%s\t%s\n" "$rel" "$ha" "$hb" >> "$OUT/differ.tsv"
    printf "." >&2
  done
  rm -rf "${OUT:?}/a$idx" "${OUT:?}/b$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
echo
echo "DIFFER $(wc -l < "$OUT/differ.tsv") of ${#FILES[@]}"
