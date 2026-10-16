#!/usr/bin/env bash
# The gate's three checks against the BANKED 26.2.4.2 references, on any track, plus the face set.
#
#   sweep.sh <track> <out-dir> [batch-glob] [workers]
#
# `words-pages-01/sweep.sh` with two changes, both needed by this round. It takes the track, because
# a font change reaches all three and that script was words-only. And it records the *names* of the
# faces each PDF embeds, not just how many — because a font fix barely moves a word count, and the
# honest measure of one is the symmetric difference between our face set and the reference's.
#
# Columns are otherwise `batch-check.sh`'s, verdict rule included, so a row here is comparable to a
# row there. The reference half is never re-rendered: it is banked at
# /c/sandbox/workdir/refpdfs-26.2.4.2-fonts.
#
# SOURCE_DATE_EPOCH is set because reach is measured by diffing two renderings and a document that
# prints today's date moves on its own otherwise.
set -uo pipefail

TRACK="${1:?usage: sweep.sh <track> <out-dir> [batch-glob] [workers]}"
OUT="${2:?usage: sweep.sh <track> <out-dir> [batch-glob] [workers]}"
GLOB="${3:-$TRACK/batch-0*}"
WORKERS="${4:-4}"
ROOT_DIR=/c/sandbox/workdir/sample-files
REF_DIR="/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/$TRACK"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the binary you mean to measure}"

export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-1700000000}"

mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
mkdir -p "$OUT/ours"
: > "$OUT/rows.tsv"
: > "$OUT/faces.tsv"

words_of() {
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
t = sys.stdin.buffer.read().decode("utf-8", "replace").split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t))'
}

# The embedded face names, subset prefix stripped, sorted and deduplicated. `BAAAAA+DejaVuSans` and
# `CAAAAA+DejaVuSans` are the same face given two subsets, and counting them apart would report a
# difference where there is none.
faces_of() {
  pdffonts "$1" 2>/dev/null | tail -n +3 | awk 'NF{print $1}' \
    | sed -E 's/^[A-Z]{6}\+//' | sort -u | paste -sd, -
}

# shellcheck disable=SC2086
mapfile -t DIRS < <(cd "$ROOT_DIR" && ls -d $GLOB 2>/dev/null)
[ "${#DIRS[@]}" -gt 0 ] || { echo "no batches matched $GLOB" >&2; exit 1; }

# No extension filter: the corpus holds four upper-case extensions and mislabelled files besides,
# and the banked reference is keyed on stem__ext for every file that is there at all.
mapfile -t FILES < <(
  for d in "${DIRS[@]}"; do find "$ROOT_DIR/$d" -type f 2>/dev/null; done | sort
)

one() {
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf un v owraw rwraw ofaces rfaces
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$REF_DIR/$id.pdf"

    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 300 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$o"

    op="-"; rp="-"; ow="-"; rw="-"; of="-"; rf="-"; un="-"; owraw="-"; rwraw="-"
    ofaces=""; rfaces=""
    if [ -f "$o" ]; then
      op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r ow owraw < <(words_of "$o")
      of=$(pdffonts "$o" 2>/dev/null | tail -n +3 | grep -c .)
      un=$(pdffonts "$o" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)
      ofaces=$(faces_of "$o")
    fi
    if [ -f "$r" ]; then
      rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r rw rwraw < <(words_of "$r")
      rf=$(pdffonts "$r" 2>/dev/null | tail -n +3 | grep -c .)
      rfaces=$(faces_of "$r")
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
    printf "%s\t%s\t%s\n" "${f#"$ROOT_DIR"/}" "$ofaces" "$rfaces" >> "$OUT/faces.tsv"
  done
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

sort "$OUT/rows.tsv" > "$OUT/parity.tsv"
sort "$OUT/faces.tsv" -o "$OUT/faces.tsv"
total=$(wc -l < "$OUT/rows.tsv")
match=$(awk -F'\t' '$7=="match"' "$OUT/rows.tsv" | wc -l)
reffail=$(awk -F'\t' '$7=="ref-failed" || $7=="both-failed"' "$OUT/rows.tsv" | wc -l)
pageerr=$(awk -F'\t' '{split($3,p,"/"); if (p[1] ~ /^[0-9]+$/ && p[2] ~ /^[0-9]+$/) \
  { d=p[1]-p[2]; if (d<0) d=-d; s+=d } } END{print s+0}' "$OUT/rows.tsv")
exact=$(awk -F'\t' '{split($3,p,"/"); if (p[1]==p[2] && p[1] ~ /^[0-9]+$/) n++} END{print n+0}' \
  "$OUT/rows.tsv")

echo "TRACK $TRACK  BATCHES ${DIRS[*]}"
echo "TOTAL $total  MATCH $match  MISMATCH $((total - match - reffail))  REF-CANNOT-RENDER $reffail"
echo "PAGE-EXACT $exact  ABS-PAGE-ERROR $pageerr"
echo "TSV $OUT/parity.tsv   FACES $OUT/faces.tsv"
[ "$((total - match - reffail))" -eq 0 ]
