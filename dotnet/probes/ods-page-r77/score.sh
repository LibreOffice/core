#!/usr/bin/env bash
# Score an already-rendered pair of PDF directories with `batch-check.sh`'s verdict rule.
#
#   score.sh <corpus-root> <ours-dir> <ref-dir> <rows.tsv>
#
# Used to re-derive a banked column without rendering anything, and to score a fresh
# `ours/` against the banked `ref/`.
set -uo pipefail
ROOT="${1:?corpus root}"; OURS="${2:?ours dir}"; REF="${3:?ref dir}"; OUTFILE="${4:?rows.tsv}"
: > "$OUTFILE"

words_of() {
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
b = sys.stdin.buffer.read().decode("utf-8", "replace")
t = b.split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t),
      sum(1 for c in b if c.isalnum()))'
}

mapfile -t FILES < <(
  find "$ROOT" -type f -iname '*.ods' -printf '%D:%i\t%p\n' \
    | awk -F'\t' '!seen[$1]++ {print $2}' | sort
)

for f in "${FILES[@]}"; do
  base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
  id="${stem}__${ext,,}"
  o="$OURS/$id.pdf"; r="$REF/$id.pdf"
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
    "$owraw" "$rwraw" "$og" "$rg" >> "$OUTFILE"
done
sort -o "$OUTFILE" "$OUTFILE"
awk -F'\t' '{c[$7]++} END {n=0; for (k in c) {print k, c[k]; n+=c[k]} print "TOTAL", n}' "$OUTFILE"
