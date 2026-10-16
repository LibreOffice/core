#!/usr/bin/env bash
# batch-check.sh's three gate checks, against the BANKED reference PDFs.
#
#   sweep-banked.sh <corpus-root> <track/batch-glob> <refdir> <outdir> [workers]
#
# batch-check.sh re-renders the reference through soffice on every run, which costs minutes
# per batch and — on at least four documents in this corpus — is not even deterministic (see
# TODO.raster-ceiling.md, "the reference is not deterministic"). When the reference binary and
# font set have not changed, the banked renderings at
# /c/sandbox/workdir/refpdfs-26.2.4.2-fonts/ ARE the reference, and re-rendering only adds
# variance.
#
# Columns, verdict rule and word definition are copied from batch-check.sh verbatim so the two
# are comparable row for row. The only substitution is where the reference PDF comes from.
#
# SOURCE_DATE_EPOCH is set so our own half is reproducible run to run.
set -uo pipefail

ROOT_DIR="${1:?usage: sweep-banked.sh <corpus-root> <glob> <refdir> <outdir> [workers]}"
GLOB="${2:?}"
REFDIR="${3:?}"
OUT="${4:?}"
WORKERS="${5:-4}"
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-1755000000}"

CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the binary you mean to measure}"
[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
echo "measuring $CLI" >&2
echo "reference  $REFDIR" >&2

mkdir -p "$OUT/ours"
: > "$OUT/rows.tsv"

words_of() {  # words_of <pdf> -> "<words> <rawwords>"
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
t = sys.stdin.buffer.read().decode("utf-8", "replace").split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t))'
}

# shellcheck disable=SC2086
mapfile -t DIRS < <(cd "$ROOT_DIR" && ls -d $GLOB 2>/dev/null)
[ "${#DIRS[@]}" -gt 0 ] || { echo "no batches matched $GLOB under $ROOT_DIR" >&2; exit 1; }

mapfile -t FILES < <(
  for d in "${DIRS[@]}"; do
    find "$ROOT_DIR/$d" -type f \
      \( -iname '*.doc'  -o -iname '*.docx' -o -iname '*.rtf'  -o -iname '*.odt' -o -iname '*.ott' \
      -o -iname '*.xls'  -o -iname '*.xlsx' -o -iname '*.ods'  -o -iname '*.csv' \
      -o -iname '*.ppt'  -o -iname '*.pptx' -o -iname '*.odp'  -o -iname '*.otp' \) 2>/dev/null
  done | sort
)

one() {
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf un v owraw rwraw
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$REFDIR/$id.pdf"

    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 300 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
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
    elif [ ! -f "$r" ];                  then v="ref-missing"
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
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

{
  printf "# reference half read from banked PDFs at %s (26.2.4.2, correct font set)\n" "$REFDIR"
  printf "# words = tokens carrying at least one Unicode letter or digit; rawwords = pdftotext | wc -w\n"
  printf "path\text\tpages\twords\tfonts\tunemb\tverdict\trawwords\n"
  sort "$OUT/rows.tsv"
} > "$OUT/parity.tsv"

total=$(wc -l < "$OUT/rows.tsv")
match=$(awk -F'\t' '$7=="match"' "$OUT/rows.tsv" | wc -l)
reffail=$(awk -F'\t' '$7=="ref-missing" || $7=="both-failed"' "$OUT/rows.tsv" | wc -l)
bad=$((total - match - reffail))

cat "$OUT/parity.tsv"
echo
echo "BATCHES ${DIRS[*]}"
echo "TOTAL $total  MATCH $match  MISMATCH $bad  REF-MISSING $reffail"
echo "TSV $OUT/parity.tsv"
[ "$bad" -eq 0 ]
