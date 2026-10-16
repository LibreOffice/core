#!/usr/bin/env bash
# Render OUR half of a track, score it against a banked reference, compare it byte for byte
# against a banked *base* rendering, and delete the PDF before moving on.
#
#   sweep-ours.sh <corpus-root> <glob> <ref-dir> <base-dir|-> <outdir> [workers]
#
# Three departures from `probes/ods-draw-r82/sweep-ours.sh`, all forced by this container:
#
#   * **Nothing is kept.** `/` was at 100% when this round started and 473 MB free after a
#     cache clear, against roughly 700 MB for one rendering of the 947-document corpus. Each
#     PDF is scored, hashed and unlinked, so the sweep's footprint is one file per worker.
#   * **A base hash is recorded beside the verdict.** A gate verdict cannot see a fill, so the
#     column that actually says what this round moved is `moved`: the md5 of our PDF with its
#     `/CreationDate` and XMP `dc:date` masked, against the same of the banked base rendering.
#     `same` means the document is byte-identical to the tree this round started from, which is
#     how the untargeted tracks are shown not to have moved.
#   * **The mask is checked rather than assumed.** `xray-office` writes `/CreationDate(D:…)` with
#     no space before the parenthesis and `soffice` writes `/CreationDate (D:…)`; round 82 found
#     its inherited pattern matching neither. Both spellings are masked here.
#
# Sound only while the diff under test cannot reach `soffice`, which a change confined to
# `dotnet/src` cannot.
set -uo pipefail
ROOT="${1:?corpus root}"; GLOB="${2:?glob}"; REF="${3:?ref dir}"
BASE="${4:?base dir or -}"; OUT="${5:?outdir}"; WORKERS="${6:-3}"
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the binary you mean to measure}"
[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
[ -d "$REF" ] || { echo "no reference bank at $REF" >&2; exit 1; }

: > "$OUT/rows.tsv"

mapfile -t FILES < <(
  find "$ROOT" -type f -path "$ROOT/$GLOB" -printf '%D:%i\t%p\n' \
    | awk -F'\t' '!seen[$1]++ {print $2}' | sort
)
echo "${#FILES[@]} documents" >&2
echo "measuring $CLI" >&2
echo "reference bank $REF" >&2
echo "base bank $BASE" >&2

words_of() {
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
b = sys.stdin.buffer.read().decode("utf-8", "replace")
t = b.split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t),
      sum(1 for c in b if c.isalnum()))'
}

# The md5 of a PDF with both spellings of the conversion date masked out.
hash_of() {
  [ -f "$1" ] || { echo "-"; return; }
  perl -0777 -pe 's/\/CreationDate\s*\((?:[^)\\]|\\.)*\)/\/CreationDate(MASKED)/g;
                  s/<xmp:(Create|Modify)Date>[^<]*<\/xmp:\1Date>/<xmp:\1Date>MASKED<\/xmp:\1Date>/g;
                  s/<dc:date>[^<]*<\/dc:date>/<dc:date>MASKED<\/dc:date>/g' "$1" \
    | md5sum | cut -d' ' -f1
}

one() {
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf og rg un v owraw rwraw oh bh moved
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    r="$REF/$id.pdf"

    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout -k 30 240 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    o="$OUT/t$idx/$stem.pdf"

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

    moved="-"
    if [ "$BASE" != "-" ]; then
      oh=$(hash_of "$o"); bh=$(hash_of "$BASE/$id.pdf")
      if   [ "$oh" = "-" ] || [ "$bh" = "-" ]; then moved="unknown"
      elif [ "$oh" = "$bh" ];                  then moved="same"
      else                                          moved="moved"
      fi
    fi

    rm -f "$o"
    printf "%s\t%s\t%s/%s\t%s/%s\t%s/%s\t%s\t%s\t%s/%s\t%s/%s\t%s\n" \
      "${f#"$ROOT"/}" "${ext,,}" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v" \
      "$owraw" "$rwraw" "$og" "$rg" "$moved" >> "$OUT/rows.tsv"
  done
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
sort -o "$OUT/rows.tsv" "$OUT/rows.tsv"
awk -F'\t' '{c[$7]++; m[$10]++} END {
  n=0; for (k in c) {print k, c[k]; n+=c[k]}
  print "TOTAL", n
  for (k in m) print "byte:" k, m[k]}' "$OUT/rows.tsv"
