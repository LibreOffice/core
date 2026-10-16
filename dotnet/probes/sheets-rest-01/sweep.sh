#!/usr/bin/env bash
# Gate sweep reusing the banked 26.2.4.2 reference PDFs instead of re-rendering them.
# Checks are batch-check.sh's three, in the same order, with the identical words_of definition.
#
#   sweep.sh <corpus-root> <glob> <outdir> [workers]
set -uo pipefail

ROOT_DIR="${1:?corpus root}"
GLOB="${2:?glob}"
OUT="${3:?outdir}"
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
WORKERS="${4:-6}"
BANK="${BANK:-/c/sandbox/workdir/refpdfs-26.2.4.2-fonts}"

CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}"
[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-1700000000}"

mkdir -p "$OUT/ours"
: > "$OUT/rows.tsv"

words_of() {
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
t = sys.stdin.buffer.read().decode("utf-8", "replace").split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t))'
}

mapfile -t DIRS < <(cd "$ROOT_DIR" && ls -d $GLOB 2>/dev/null)
[ "${#DIRS[@]}" -gt 0 ] || { echo "no dirs matched $GLOB" >&2; exit 1; }

mapfile -t FILES < <(
  for d in "${DIRS[@]}"; do
    find "$ROOT_DIR/$d" -type f \
      \( -iname '*.doc' -o -iname '*.docx' -o -iname '*.rtf' -o -iname '*.odt' -o -iname '*.ott' \
      -o -iname '*.xls' -o -iname '*.xlsx' -o -iname '*.ods' -o -iname '*.csv' \
      -o -iname '*.ppt' -o -iname '*.pptx' -o -iname '*.odp' -o -iname '*.otp' \) 2>/dev/null
  done | sort
)

# which track subdir of the bank to look in
track_of() { case "$1" in sheets/*) echo sheets;; words/*) echo words;; slides/*) echo slides;; *) echo "";; esac; }

one() {
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf un v owraw rwraw tr
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    rel="${f#"$ROOT_DIR"/}"
    tr="$(track_of "$rel")"
    o="$OUT/ours/$id.pdf"; r="$BANK/$tr/$id.pdf"

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
      "$rel" "${ext,,}" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v" \
      "$owraw" "$rwraw" >> "$OUT/rows.tsv"
  done
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

{
  printf "path\text\tpages\twords\tfonts\tunemb\tverdict\trawwords\n"
  sort "$OUT/rows.tsv"
} > "$OUT/parity.tsv"

total=$(wc -l < "$OUT/rows.tsv")
match=$(awk -F'\t' '$7=="match"' "$OUT/rows.tsv" | wc -l)
reffail=$(awk -F'\t' '$7=="ref-missing" || $7=="both-failed"' "$OUT/rows.tsv" | wc -l)
bad=$((total - match - reffail))

cat "$OUT/parity.tsv"
echo
echo "DIRS ${DIRS[*]}"
echo "TOTAL $total  MATCH $match  MISMATCH $bad  REF-MISSING $reffail"
echo "TSV $OUT/parity.tsv"
[ "$bad" -eq 0 ]
