#!/usr/bin/env bash
# The gate's own three checks against the BANKED 26.2.4.2 references, rendering only our half.
#
#   sweep.sh <out-dir> [batch-glob] [workers]
#
# Same columns and the same verdict rule as `batch-check.sh`, so a row here is comparable to a row
# there — verified document for document on batch-004 and batch-006 before this was used for reach.
# What it does not do is re-render the reference: those PDFs are banked at
# /c/sandbox/workdir/refpdfs-26.2.4.2-fonts and re-making them costs an hour and can only reproduce
# what is already there. That is also what makes a whole-track sweep affordable twice in a round,
# which is what a reach figure needs.
#
# SOURCE_DATE_EPOCH is set because reach is measured by diffing two renderings byte for byte and a
# document that prints today's date moves on its own otherwise.
set -uo pipefail

OUT="${1:?usage: sweep.sh <out-dir> [batch-glob] [workers]}"
GLOB="${2:-words/batch-0*}"
WORKERS="${3:-4}"
ROOT_DIR=/c/sandbox/workdir/sample-files
REF_DIR=/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the binary you mean to measure}"

export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-1700000000}"

mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
mkdir -p "$OUT/ours"
: > "$OUT/rows.tsv"

words_of() {
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
t = sys.stdin.buffer.read().decode("utf-8", "replace").split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t))'
}

# shellcheck disable=SC2086
mapfile -t DIRS < <(cd "$ROOT_DIR" && ls -d $GLOB 2>/dev/null)
[ "${#DIRS[@]}" -gt 0 ] || { echo "no batches matched $GLOB" >&2; exit 1; }

mapfile -t FILES < <(
  for d in "${DIRS[@]}"; do
    find "$ROOT_DIR/$d" -type f \( -iname '*.doc' -o -iname '*.docx' -o -iname '*.rtf' \
      -o -iname '*.odt' -o -iname '*.ott' \) 2>/dev/null
  done | sort
)

one() {
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf un v owraw rwraw
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$REF_DIR/$id.pdf"

    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 240 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$o"

    op="-"; rp="-"; ow="-"; rw="-"; of="-"; rf="-"; un="-"; owraw="-"; rwraw="-"
    if [ -f "$o" ]; then
      op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r ow owraw < <(words_of "$o")
      of=$(pdffonts "$o" 2>/dev/null | tail -n +3 | grep -c .)
      un=$(pdffonts "$o" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)
    fi
    if [ -f "$r" ]; then
      rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r rw rwraw < <(words_of "$r")
      rf=$(pdffonts "$r" 2>/dev/null | tail -n +3 | grep -c .)
    fi

    if   [ ! -f "$r" ] && [ ! -f "$o" ]; then v="both-failed"
    elif [ ! -f "$r" ];                  then v="ref-failed"
    elif [ ! -f "$o" ];                  then v="ours-failed"
    else
      v=""
      [ "$op" = "$rp" ] || v="pages"
      if [ "$rw" -gt 0 ] 2>/dev/null; then
        awk -v a="$ow" -v b="$rw" 'BEGIN{d=(a>b?a-b:b-a); exit !(d > b*0.02 && d > 3)}' \
          && v="${v:+$v,}words"
      elif [ "${ow:-0}" -gt 3 ]; then v="${v:+$v,}words"
      fi
      [ "${un:-0}" = "0" ] || v="${v:+$v,}unembedded"
      [ -n "$v" ] || v="match"
    fi

    printf "%s\t%s\t%s/%s\t%s/%s\t%s/%s\t%s\t%s\t%s/%s\n" \
      "${f#"$ROOT_DIR"/}" "${ext,,}" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v" \
      "$owraw" "$rwraw" >> "$OUT/rows.tsv"
  done
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

sort "$OUT/rows.tsv" > "$OUT/parity.tsv"
total=$(wc -l < "$OUT/rows.tsv")
match=$(awk -F'\t' '$7=="match"' "$OUT/rows.tsv" | wc -l)
reffail=$(awk -F'\t' '$7=="ref-failed" || $7=="both-failed"' "$OUT/rows.tsv" | wc -l)
pageerr=$(awk -F'\t' '{split($3,p,"/"); if (p[1] ~ /^[0-9]+$/ && p[2] ~ /^[0-9]+$/) \
  { d=p[1]-p[2]; if (d<0) d=-d; s+=d } } END{print s+0}' "$OUT/rows.tsv")
exact=$(awk -F'\t' '{split($3,p,"/"); if (p[1]==p[2] && p[1] ~ /^[0-9]+$/) n++} END{print n+0}' \
  "$OUT/rows.tsv")

echo "BATCHES ${DIRS[*]}"
echo "TOTAL $total  MATCH $match  MISMATCH $((total - match - reffail))  REF-CANNOT-RENDER $reffail"
echo "PAGE-EXACT $exact  ABS-PAGE-ERROR $pageerr"
echo "TSV $OUT/parity.tsv"
[ "$((total - match - reffail))" -eq 0 ]
