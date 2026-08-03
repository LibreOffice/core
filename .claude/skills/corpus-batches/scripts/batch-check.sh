#!/usr/bin/env bash
# Prove that Paperless renders a corpus batch the way LibreOffice does.
#
#   batch-check.sh <corpus-root> <batch-glob> [outdir] [workers]
#
#   batch-check.sh /workspace/sample-files 'batch-001'          # one batch
#   batch-check.sh /workspace/sample-files 'batch-0[0-1]*'      # batches 1-19, a regression sweep
#
# Writes a TSV per document and a one-line verdict. Exit status is 0 only when every
# document in range matches, so this can gate a commit.
#
# Two things this script does that the obvious version does not:
#
#   * Parallel workers, each with its own soffice profile. Two headless instances sharing
#     ~/.config/libreoffice block on the profile lock and one of them converts nothing at
#     all — silently, with exit status 0.
#   * Per-format identity (`report__docx`, not `report`). Two documents differing only by
#     extension both convert to report.pdf and one overwrites the other, which reads as a
#     mysterious parity failure on whichever lost.
#
# The checks are the same three, in the same order, as corpus-parity.sh: page count, then
# extractable words, then font embedding. Each is cheap and rules out a whole class, and
# a wrong page count makes everything after it meaningless.
set -uo pipefail

ROOT_DIR="${1:?usage: batch-check.sh <corpus-root> <batch-glob> [outdir] [workers]}"
GLOB="${2:?batch glob, e.g. batch-001 or 'batch-0[0-1]*'}"
OUT="${3:-$(mktemp -d)}"
# Absolute, always. soffice takes its profile as `file://$OUT/profN`, and a relative path
# there is not a URL — it silently starts with an unusable profile and converts nothing, so
# every document is reported as `ref-failed` rather than as an error. Cost one agent a whole
# sweep before the pattern was recognised.
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
WORKERS="${4:-3}"

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CLI="$REPO/dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64/Paperless.Cli"
[ -x "$CLI" ] || { echo "no CLI at $CLI — build it first" >&2; exit 1; }

mkdir -p "$OUT/ours" "$OUT/ref"
: > "$OUT/rows.tsv"

# shellcheck disable=SC2086  # the glob is meant to expand
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

one() {  # one <index>
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf un v
  local prof="$OUT/prof$idx"
  mkdir -p "$prof" "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$OUT/ref/$id.pdf"

    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 240 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$o"

    rm -rf "$OUT/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 240 soffice -env:UserInstallation="file://$prof" \
      --headless --convert-to pdf --outdir "$OUT/t$idx" "$f" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$r"

    op="-"; rp="-"; ow="-"; rw="-"; of="-"; rf="-"; un="-"
    if [ -f "$o" ]; then
      op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
      ow=$(pdftotext "$o" - 2>/dev/null | wc -w)
      of=$(pdffonts "$o" 2>/dev/null | tail -n +3 | grep -c .)
      # The `emb` column, found by its position from the *right*: pdffonts ends every row with
      # emb, sub, uni and a two-field object id, so `emb` is NF-4 and not NF-3. Counting from
      # NF-3 reads `sub` instead, and it happens to give the right answer only for a font whose
      # type name is two or three fields — "Type 1", "Type 1C", "CID Type 0C". Every font
      # Paperless writes is "TrueType", one field, so this check tested nothing about our own
      # output until it was corrected; measured on a PDF embedding ten faces and naming an
      # eleventh, which it scored as zero unembedded.
      un=$(pdffonts "$o" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)
    fi
    if [ -f "$r" ]; then
      rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
      rw=$(pdftotext "$r" - 2>/dev/null | wc -w)
      rf=$(pdffonts "$r" 2>/dev/null | tail -n +3 | grep -c .)
    fi

    # A document LibreOffice itself cannot render is not our failure, and must not be
    # allowed to look like one — it is excluded from the verdict, not counted as a pass.
    if   [ ! -f "$r" ] && [ ! -f "$o" ]; then v="both-failed"
    elif [ ! -f "$r" ];                  then v="ref-failed"
    elif [ ! -f "$o" ];                  then v="ours-failed"
    else
      v=""
      [ "$op" = "$rp" ] || v="pages"
      # Extraction drifts a little on hyphenation and soft breaks; 2% is the band that
      # separates "the same text" from "text is missing", measured across this corpus.
      if [ "$rw" -gt 0 ] 2>/dev/null; then
        awk -v a="$ow" -v b="$rw" 'BEGIN{d=(a>b?a-b:b-a); exit !(d > b*0.02 && d > 3)}' \
          && v="${v:+$v,}words"
      elif [ "${ow:-0}" -gt 3 ]; then v="${v:+$v,}words"
      fi
      [ "${un:-0}" = "0" ] || v="${v:+$v,}unembedded"
      [ -n "$v" ] || v="match"
    fi

    printf "%s\t%s\t%s/%s\t%s/%s\t%s/%s\t%s\t%s\n" \
      "${f#"$ROOT_DIR"/}" "${ext,,}" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v" \
      >> "$OUT/rows.tsv"
  done
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

{
  printf "path\text\tpages\twords\tfonts\tunemb\tverdict\n"
  sort "$OUT/rows.tsv"
} > "$OUT/parity.tsv"

total=$(wc -l < "$OUT/rows.tsv")
match=$(awk -F'\t' '$7=="match"' "$OUT/rows.tsv" | wc -l)
reffail=$(awk -F'\t' '$7=="ref-failed" || $7=="both-failed"' "$OUT/rows.tsv" | wc -l)
bad=$((total - match - reffail))

cat "$OUT/parity.tsv"
echo
echo "BATCHES ${DIRS[*]}"
echo "TOTAL $total  MATCH $match  MISMATCH $bad  REF-CANNOT-RENDER $reffail"
echo "TSV $OUT/parity.tsv"
[ "$bad" -eq 0 ]
