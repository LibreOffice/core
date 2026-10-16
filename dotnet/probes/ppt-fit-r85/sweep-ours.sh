#!/usr/bin/env bash
# Render OUR half of a track, score it against a banked 26.2.4.2 reference, digest it, delete it.
#
#   XRAY_OFFICE_CLI=... sweep-ours.sh <corpus-root> <ls-files-prefix> <ext-regex> <bank> <out> [workers]
#
# Sound only because the diff under test is confined to `dotnet/src`, which cannot reach `soffice`
# — so the banked reference bytes are exactly the ones the scoreboard was built from.
#
# The PDF is unlinked once its columns and its digest are taken: this container has about four
# gigabytes free and a slides sweep does not fit otherwise. `SOURCE_DATE_EPOCH` is pinned so two
# runs of the same tree are byte-equal with nothing masked.
set -uo pipefail
ROOT="${1:?corpus root}"; PREFIX="${2:?prefix}"; EXTS="${3:?ext regex}"
BANK="${4:?bank dir}"; OUT="${5:?outdir}"; WORKERS="${6:-2}"
export SOURCE_DATE_EPOCH=1700000000
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}"
[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
[ -d "$BANK/ref" ] || { echo "no $BANK/ref" >&2; exit 1; }
: > "$OUT/rows.tsv"

# `git ls-files` where the corpus is a checkout -- it cannot see the case-alias entries the
# mount manufactures -- and `find` where it is not, which is the converted-ODF tree.
if git -C "$ROOT" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  mapfile -t FILES < <(
    git -C "$ROOT" -c core.quotePath=false ls-files "$PREFIX" \
      | grep -Ei "$EXTS" | sed "s|^|$ROOT/|")
else
  mapfile -t FILES < <(find "$ROOT/$PREFIX" -type f | grep -Ei "$EXTS" | sort)
fi
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
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf og rg un v owraw rwraw md5
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    o="$OUT/t$idx/$stem.pdf"; r="$BANK/ref/$id.pdf"
    timeout -k 30 240 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1

    op="-"; rp="-"; ow="-"; rw="-"; of="-"; rf="-"; og="-"; rg="-"; un="-"; owraw="-"; rwraw="-"; md5="-"
    if [ -f "$o" ]; then
      op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r ow owraw og < <(words_of "$o")
      of=$(pdffonts "$o" 2>/dev/null | tail -n +3 | grep -c .)
      un=$(pdffonts "$o" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)
      md5=$(md5sum "$o" | cut -d' ' -f1)
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
    printf "%s\t%s\t%s/%s\t%s/%s\t%s/%s\t%s\t%s\t%s/%s\t%s/%s\t%s\n" \
      "${f#"$ROOT"/}" "${ext,,}" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v" \
      "$owraw" "$rwraw" "$og" "$rg" "$md5" >> "$OUT/rows.tsv"
    rm -rf "${OUT:?}/t$idx"
  done
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
echo "TOTAL $(wc -l < "$OUT/rows.tsv")  MATCH $(awk -F'\t' '$7=="match"' "$OUT/rows.tsv" | wc -l)"
