#!/usr/bin/env bash
# Re-render OUR half of the converted corpus's `.odp` column and score it against a banked
# reference.
#
#   sweep-ours.sh <corpus-root> <bank-dir> <outdir> [workers] [ext]
#
# `ext` selects the bank's rows by their extension column and defaults to `odp`. Set
# KEEP_PDFS=0 to score each rendering and delete it, which is what a confinement check over a
# second track wants: the verdict columns are the answer and the disk is 95% full.
#
# Sound only when the diff under test cannot reach `soffice`, which a change confined to
# `dotnet/src` cannot. The document list is taken from the bank's own `rows.tsv` rather than
# from a glob, so the denominator and the per-document ids are the bank's by construction --
# a `find` over a case-insensitive mount is what makes a sweep TOTAL drift.
set -uo pipefail
ROOT="${1:?corpus root}"; BANK="${2:?bank dir}"; OUT="${3:?outdir}"; WORKERS="${4:-2}"
EXT="${5:-odp}"; KEEP_PDFS="${KEEP_PDFS:-1}"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the binary you mean to measure}"
[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
[ -d "$BANK/ref" ] || { echo "no $BANK/ref" >&2; exit 1; }
mkdir -p "$OUT/ours" && OUT="$(cd "$OUT" && pwd)"
: > "$OUT/rows.tsv"

mapfile -t FILES < <(awk -F'\t' -v e="$EXT" '$2==e{print $1}' "$BANK/rows.tsv")
echo "${#FILES[@]} documents" >&2

words_of() {
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
b = sys.stdin.buffer.read().decode("utf-8", "replace")
t = b.split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t),
      sum(1 for c in b if c.isalnum()))'
}

one() {
  local idx="$1" i=-1 rel f base ext stem id o r op rp ow rw of rf og rg un v owraw rwraw
  for rel in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    f="$ROOT/$rel"
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$BANK/ref/$id.pdf"

    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 240 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$o"

    op="-"; rp="-"; ow="-"; rw="-"; of="-"; rf="-"; og="-"; rg="-"; un="-"; owraw="-"; rwraw="-"
    if [ -f "$o" ]; then
      op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r ow owraw og < <(words_of "$o")
      of=$(pdffonts "$o" 2>/dev/null | tail -n +3 | grep -c .)
      un=$(pdffonts "$o" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)
    fi
    if [ -f "$r" ]; then
      rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r rw rwraw rg < <(words_of "$r")
      rf=$(pdffonts "$r" 2>/dev/null | tail -n +3 | grep -c .)
    fi

    if   [ ! -f "$r" ] && [ ! -f "$o" ]; then v="both-failed"
    elif [ ! -f "$r" ];                  then v="ref-failed"
    elif [ ! -f "$o" ];                  then v="ours-failed"
    else
      v=""
      [ "$op" = "$rp" ] || v="pages"
      if [ "$rg" -gt 0 ] 2>/dev/null; then
        awk -v a="$og" -v b="$rg" 'BEGIN{d=(a>b?a-b:b-a); exit !(d > b*0.02 && d > 15)}' \
          && v="${v:+$v,}words"
      elif [ "${og:-0}" -gt 15 ]; then v="${v:+$v,}words"
      fi
      [ "${un:-0}" = "0" ] || v="${v:+$v,}unembedded"
      [ -n "$v" ] || v="match"
    fi
    printf "%s\t%s\t%s/%s\t%s/%s\t%s/%s\t%s\t%s\t%s/%s\t%s/%s\n" \
      "$rel" "${ext,,}" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v" \
      "$owraw" "$rwraw" "$og" "$rg" >> "$OUT/rows.tsv"
    [ "$KEEP_PDFS" = "1" ] || rm -f "$o"
  done
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

echo "TOTAL $(wc -l < "$OUT/rows.tsv")"
awk -F'\t' '{print $7}' "$OUT/rows.tsv" | sort | uniq -c | sort -rn
