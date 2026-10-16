#!/usr/bin/env bash
# Re-render OUR half of one track of a corpus and score it against a banked reference.
#
#   sweep-track.sh <corpus-root> <path-prefix> <bank-dir> <outdir> [workers]
#
# The sibling of `sweep-ours.sh` for a corpus laid out in `<family>/<batch>/<ext>/` rather than
# by extension: the prefix is matched against the path relative to the root, so `words` is the
# whole words track.  Same soundness condition — the diff under test must not be able to reach
# `soffice` — and the same columns as `batch-check.sh`.
set -uo pipefail
ROOT="${1:?corpus root}"; PREFIX="${2:?path prefix}"; BANK="${3:?bank dir}"
OUT="${4:?outdir}"; WORKERS="${5:-2}"
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the binary you mean to measure}"
[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
[ -d "$BANK/ref" ] || { echo "no $BANK/ref" >&2; exit 1; }

mkdir -p "$OUT/ours"
: > "$OUT/rows.tsv"

mapfile -t FILES < <(find "$ROOT/$PREFIX" -type f | sort)
echo "${#FILES[@]} documents, CLI $CLI" >&2

words_of() {
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
b = sys.stdin.buffer.read().decode("utf-8", "replace")
t = b.split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t),
      sum(1 for c in b if c.isalnum()))'
}

one() {
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf og rg un v owraw rwraw d
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$BANK/ref/$id.pdf"

    d="$OUT/t$idx-$i"; rm -rf "$d"; mkdir -p "$d"
    timeout -k 30 240 "$CLI" render "$f" --format pdf --outdir "$d" >/dev/null 2>&1
    [ -f "$d/$stem.pdf" ] && mv -f "$d/$stem.pdf" "$o"
    rm -rf "$d"

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
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
sort -o "$OUT/rows.tsv" "$OUT/rows.tsv"
awk -F'\t' '{c[$7]++} END {n=0; for (k in c) {print k, c[k]} print "TOTAL", n}' "$OUT/rows.tsv"
awk -F'\t' '{n++} END {print "ROWS", n}' "$OUT/rows.tsv"
