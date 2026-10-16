#!/usr/bin/env bash
# Re-render OUR half of the converted-ODF `.ods` column and score it against banked
# reference PDFs, applying `batch-check.sh`'s own verdict rule byte for byte.
#
#   sweep-ours.sh <corpus-root> <bank-dir> <outdir> [workers]
#
# Sound only while the diff under test cannot reach `soffice`, which a change confined to
# `dotnet/src` cannot. `/home/user/corpus-odf` is not a git checkout, so the file list comes
# from `find` rather than from `git ls-files`; it is deduplicated by inode because the mount
# is case-insensitive and reports one file under two names.
set -uo pipefail
ROOT="${1:?corpus root}"; BANK="${2:?bank dir}"; OUT="${3:?outdir}"; WORKERS="${4:-2}"
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the binary you mean to measure}"
[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
[ -d "$BANK/ref" ] || { echo "no $BANK/ref" >&2; exit 1; }

mkdir -p "$OUT/ours"
: > "$OUT/rows.tsv"

mapfile -t FILES < <(
  find "$ROOT" -type f -iname '*.ods' -printf '%D:%i\t%p\n' \
    | awk -F'\t' '!seen[$1]++ {print $2}' | sort
)
echo "${#FILES[@]} documents" >&2
echo "measuring $CLI" >&2
echo "bank $BANK" >&2

words_of() {
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
b = sys.stdin.buffer.read().decode("utf-8", "replace")
t = b.split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t),
      sum(1 for c in b if c.isalnum()))'
}

one() {
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf og rg un v owraw rwraw
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$BANK/ref/$id.pdf"

    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout -k 30 240 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
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
      "${f#"$ROOT"/}" "${ext,,}" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v" \
      "$owraw" "$rwraw" "$og" "$rg" >> "$OUT/rows.tsv"
  done
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
sort -o "$OUT/rows.tsv" "$OUT/rows.tsv"
awk -F'\t' '{c[$7]++} END {n=0; for (k in c) {print k, c[k]; n+=c[k]} print "TOTAL", n}' "$OUT/rows.tsv"
