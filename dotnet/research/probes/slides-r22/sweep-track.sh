#!/usr/bin/env bash
# sweep.sh <outdir> <cli-dir> [workers]
#
# The slides track swept whole against a *kept* set of reference PDFs. Nothing on this line
# of work touches soffice, so re-rendering 163 documents with it every round is half the cost
# of a sweep for no information. The kept set is verified against a fresh soffice run before
# it is trusted — see refcheck in this directory.
#
# Emits the same three gate columns batch-check.sh does plus the ink metric, so a round can
# be judged on a continuous quantity rather than only on the binary one.
set -uo pipefail

OUT="${1:?usage: sweep.sh <outdir> <cli-dir> [workers]}"
CLIDIR="${2:?}"
WORKERS="${3:-3}"
REF=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/slides-3e093938/sweep-base/ref
ROOT=/workspace/sample-files
DIFF=/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py
CENSUS="$(dirname "$(readlink -f "$0")")/size-census-doc.py"
CLI="$CLIDIR/Paperless.Cli"

[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
echo "measuring $CLI" >&2
mkdir -p "$OUT/ours" "$OUT/cmp"
: > "$OUT/rows.tsv"; : > "$OUT/ink.tsv"; : > "$OUT/census.tsv"

mapfile -t FILES < <(find "$ROOT"/slides -type f \
  \( -iname '*.ppt' -o -iname '*.pptx' -o -iname '*.odp' -o -iname '*.otp' \) | sort)
echo "documents: ${#FILES[@]}" >&2

one() {
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf un v
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$REF/$id.pdf"

    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 300 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$o"

    op="-"; rp="-"; ow="-"; rw="-"; of="-"; rf="-"; un="-"
    if [ -f "$o" ]; then
      op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
      ow=$(pdftotext "$o" - 2>/dev/null | wc -w)
      of=$(pdffonts "$o" 2>/dev/null | tail -n +3 | grep -c .)
      un=$(pdffonts "$o" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)
    fi
    if [ -f "$r" ]; then
      rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
      rw=$(pdftotext "$r" - 2>/dev/null | wc -w)
      rf=$(pdffonts "$r" 2>/dev/null | tail -n +3 | grep -c .)
    fi

    if   [ ! -f "$r" ] && [ ! -f "$o" ]; then v="both-failed"
    elif [ ! -f "$r" ];                  then v="ref-failed"
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

    printf "%s\t%s\t%s/%s\t%s/%s\t%s/%s\t%s\t%s\n" \
      "${f#"$ROOT"/}" "${ext,,}" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v" \
      >> "$OUT/rows.tsv"

    if [ -f "$o" ] && [ -f "$r" ] && [ "$op" = "$rp" ]; then
      python3 "$DIFF" "$o" "$r" --outdir "$OUT/cmp/$id" 2>/dev/null \
        | awk -v p="${f#"$ROOT"/}" -v n="$op" -v vv="$v" '
            /^[0-9]/ { ink += $3; aink += $4; if ($6 == "MAJOR") maj++ }
            END { printf "%s\t%s\t%.2f\t%.2f\t%d\t%s\n", p, n, ink, aink, maj, vv }' >> "$OUT/ink.tsv"
      rm -rf "$OUT/cmp/$id"
    fi

    # The census runs HERE, in the comparison pass, while both renderings are certainly on
    # disk — not as a separate pass afterwards. Round twenty-one's ran afterwards, was starved
    # under load, was killed so the sweep could finish, and then could not be resumed because
    # the sweep's own PDFs had been freed to make room. Inline it costs one more read of files
    # that are already there and parallelises across the sweep's workers.
    #
    # Guarded only on both PDFs existing, not on the page counts agreeing: the census compares
    # the *intersection* of page numbers and has always run over every document with a
    # reference, so gating it the way the image diff is gated would silently change the
    # denominator against every earlier round's figure.
    if [ -f "$o" ] && [ -f "$r" ]; then
      timeout 900 python3 "$CENSUS" "$o" "$r" "$id" >> "$OUT/census.tsv" 2>/dev/null
    fi
  done
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

{ printf "path\text\tpages\twords\tfonts\tunemb\tverdict\n"; sort "$OUT/rows.tsv"; } > "$OUT/parity.tsv"

total=$(wc -l < "$OUT/rows.tsv")
match=$(awk -F'\t' '$7=="match"' "$OUT/rows.tsv" | wc -l)
echo
echo "TOTAL $total  MATCH $match"
awk -F'\t' '{s+=$3; a+=$4; m+=$5} END{printf "INK %.2f  |INK| %.2f  MAJOR PAGES %d  over %d documents\n", s, a, m, NR}' "$OUT/ink.tsv"

# |ink|% is a sum of absolute values and ink% is the signed sum of the same terms, so the
# first can never be the smaller. Round twenty's ink.tsv violated this and the aggregate was
# read for a round before anyone checked.
awk -F'\t' '{s+=$3; a+=$4} END{if (a + 0.005 < (s<0?-s:s)) { print "AGGREGATE INVARIANT VIOLATED: |ink| " a " < |ink%| of " s; exit 1 }}' "$OUT/ink.tsv" \
  || echo "  ^ discard this sweep" >&2

python3 "$(dirname "$(readlink -f "$0")")/size-census-fold.py" "$OUT/census.tsv" 
