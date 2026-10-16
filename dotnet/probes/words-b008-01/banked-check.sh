#!/usr/bin/env bash
# Run batch-check.sh's three checks against the BANKED reference renderings instead of
# re-converting with soffice.
#
#   banked-check.sh <corpus-root> <batch-glob> <outdir> [workers]
#
# Why this exists: `batch-check.sh` renders both halves every run, which is right when the
# question is "does the reference still say that" and wasteful — and, on at least one document,
# actively misleading — when a canonical set already exists at
# /c/sandbox/workdir/refpdfs-26.2.4.2-fonts. Reusing it also removes soffice's own run-to-run
# variation from the comparison.
#
# The checks, the word definition and the 2%+3 band are copied verbatim from batch-check.sh so
# a row here is column-for-column a row there. The reference figures are recomputed from the
# banked PDFs rather than read from ref-baseline-all.tsv, because that file's `refwords` column
# is the raw pdftotext count on at least some rows while the gate wants the letter-or-digit one.
set -uo pipefail

ROOT_DIR="${1:?usage: banked-check.sh <corpus-root> <batch-glob> <outdir> [workers]}"
GLOB="${2:?batch glob}"
OUT="${3:?outdir}"
WORKERS="${4:-4}"
REFS="${REFPDFS:-/c/sandbox/workdir/refpdfs-26.2.4.2-fonts}"

mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"

CLI="${XRAY_OFFICE_CLI:-}"
[ -x "$CLI" ] || { echo "set XRAY_OFFICE_CLI to the binary you mean to measure" >&2; exit 1; }
echo "measuring $CLI against banked refs in $REFS" >&2

# Reproducible output: a document that prints today's date must not differ between two runs.
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-1700000000}"

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
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf un v owraw rwraw track
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"
    track="${f#"$ROOT_DIR"/}"; track="${track%%/*}"
    r="$REFS/$track/$id.pdf"

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
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

{
  printf "# ours: %s\n" "$CLI"
  printf "# reference: banked %s (NOT re-rendered)\n" "$REFS"
  printf "# words = tokens carrying at least one Unicode letter or digit; rawwords = pdftotext | wc -w\n"
  printf "path\text\tpages\twords\tfonts\tunemb\tverdict\trawwords\n"
  sort "$OUT/rows.tsv"
} > "$OUT/report.tsv"

MATCH=$(awk -F'\t' '$7=="match"' "$OUT/rows.tsv" | wc -l)
TOTAL=$(wc -l < "$OUT/rows.tsv")
echo "$MATCH / $TOTAL match"
awk -F'\t' '$7!="match"{printf "  %-70s %s %s %s\n", $1, $3, $4, $7}' "$OUT/rows.tsv" | sort
[ "$MATCH" = "$TOTAL" ]
