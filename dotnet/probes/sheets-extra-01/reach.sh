#!/usr/bin/env bash
# Reach of the ST_Xstring fix: which of the corpus's 534 documents X-Ray.Office renders differently
# after it.
#
#   reach.sh <before-cli> <after-cli> <outdir> [workers]
#
# Renders every document twice — once with each binary — and compares the two PDFs byte for byte.
# `SOURCE_DATE_EPOCH` is exported here rather than left to the caller because a spreadsheet whose
# header holds `&D` draws today's date, and 17 of the sheets track's 171 documents move between
# two runs a day apart without it. With it set, two runs of the SAME binary are byte-equal with
# nothing masked, so every difference this reports is the change under test and not the clock.
#
# The banked references at refpdfs-26.2.4.2-fonts/ are read, never regenerated: the reference half
# of the gate costs an soffice conversion per document and did not change.
set -uo pipefail

BEFORE="${1:?usage: reach.sh <before-cli> <after-cli> <outdir> [workers]}"
AFTER="${2:?after cli}"
OUT="${3:?outdir}"
WORKERS="${4:-4}"
CORPUS="${CORPUS:-/c/sandbox/workdir/sample-files}"
REFS="${REFS:-/c/sandbox/workdir/refpdfs-26.2.4.2-fonts}"

export SOURCE_DATE_EPOCH=1600000000
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
: > "$OUT/reach.tsv"

mapfile -t FILES < <(find "$CORPUS/words" "$CORPUS/sheets" "$CORPUS/slides" -type f \
  \( -iname '*.doc'  -o -iname '*.docx' -o -iname '*.rtf'  -o -iname '*.odt' -o -iname '*.ott' \
  -o -iname '*.xls'  -o -iname '*.xlsx' -o -iname '*.ods'  -o -iname '*.csv' \
  -o -iname '*.ppt'  -o -iname '*.pptx' -o -iname '*.odp'  -o -iname '*.otp' \) | sort)

echo "${#FILES[@]} documents" >&2

words_of() {  # same definition as batch-check.sh's, so the columns are comparable
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
t = sys.stdin.buffer.read().decode("utf-8","replace").split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t))'
}

one() {
  local idx="$1" i=-1 f base ext stem id track b a state
  mkdir -p "$OUT/b$idx" "$OUT/a$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    track="$(basename "$(dirname "$(dirname "$(dirname "$f")")")")"

    rm -rf "${OUT:?}/b$idx" "${OUT:?}/a$idx"; mkdir -p "$OUT/b$idx" "$OUT/a$idx"
    timeout 300 "$BEFORE" render "$f" --format pdf --outdir "$OUT/b$idx" >/dev/null 2>&1
    timeout 300 "$AFTER"  render "$f" --format pdf --outdir "$OUT/a$idx" >/dev/null 2>&1
    b="$OUT/b$idx/$stem.pdf"; a="$OUT/a$idx/$stem.pdf"

    if [ ! -f "$b" ] && [ ! -f "$a" ]; then state="both-unrendered"
    elif [ ! -f "$b" ];                  then state="now-renders"
    elif [ ! -f "$a" ];                  then state="no-longer-renders"
    elif cmp -s "$b" "$a";               then state="identical"
    else                                      state="CHANGED"
    fi

    local bp="-" ap="-" bw="-" aw="-" rp="-" rw="-" braw araw rraw
    if [ "$state" = "CHANGED" ]; then
      bp=$(pdfinfo "$b" 2>/dev/null | awk '/^Pages/{print $2}')
      ap=$(pdfinfo "$a" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r bw braw < <(words_of "$b")
      read -r aw araw < <(words_of "$a")
      if [ -f "$REFS/$track/$id.pdf" ]; then
        rp=$(pdfinfo "$REFS/$track/$id.pdf" 2>/dev/null | awk '/^Pages/{print $2}')
        read -r rw rraw < <(words_of "$REFS/$track/$id.pdf")
      fi
    fi

    printf "%s\t%s\t%s\t%s/%s/%s\t%s/%s/%s\n" \
      "$state" "$track" "${f#"$CORPUS"/}" "$bp" "$ap" "$rp" "$bw" "$aw" "$rw" >> "$OUT/reach.tsv"
  done
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

{
  printf "state\ttrack\tpath\tpages before/after/ref\twords before/after/ref\n"
  sort "$OUT/reach.tsv"
} > "$OUT/reach-sorted.tsv"

echo
awk -F'\t' '{print $1}' "$OUT/reach.tsv" | sort | uniq -c | sort -rn
echo
grep -E '^(CHANGED|now-renders|no-longer-renders)' "$OUT/reach-sorted.tsv"
echo "TSV $OUT/reach-sorted.tsv"
