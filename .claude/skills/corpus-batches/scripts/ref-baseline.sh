#!/usr/bin/env bash
# Record the *reference* half of the gate, for a corpus range, at whatever soffice is installed.
#
#   ref-baseline.sh <corpus-root> <batch-glob> <outdir> [workers]
#
#   ref-baseline.sh /c/sandbox/workdir/sample-files 'words/batch-0*' /tmp/refbase-words 6
#
# WHY THIS EXISTS SEPARATELY FROM `batch-check.sh`
# ────────────────────────────────────────────────
# `batch-check.sh` measures ours against the reference in one pass and refuses to start
# without a built CLI. That coupling is right for a round and wrong for two situations:
#
#   * The build is unavailable (no package feed, a broken restore) but soffice works. The
#     expensive half of the gate — 534 soffice conversions — can still be banked.
#   * The reference binary itself changed. Then every stored figure is against a superseded
#     binary and the reference column has to be re-measured *before* any verdict means
#     anything. That is the case this was written for: this container has 26.2.4.2 where
#     every figure in TODO.batches.md was taken against 24.2.7.2.
#
# The conventions are `batch-check.sh`'s, deliberately identical so the two are comparable
# column for column:
#
#   * one soffice profile per worker — two headless instances sharing ~/.config/libreoffice
#     block on the profile lock and one converts nothing, silently, with exit status 0;
#   * per-format identity (`report__docx`, not `report`) — two documents differing only by
#     extension both convert to report.pdf and one overwrites the other;
#   * an absolute outdir, because soffice takes its profile as `file://$OUT/profN` and a
#     relative path there is not a URL, so it starts with an unusable profile and converts
#     nothing while reporting success.
#
# The version is recorded in the output. A baseline whose header does not name the binary it
# was taken against is the thing that caused this whole re-baseline, so it is not optional.
set -uo pipefail

ROOT_DIR="${1:?usage: ref-baseline.sh <corpus-root> <batch-glob> <outdir> [workers]}"
GLOB="${2:?batch glob, e.g. 'words/batch-0*'}"
OUT="${3:?outdir — absolute or it will be made absolute}"
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
WORKERS="${4:-6}"

# Renderings are comparable only with these pinned; a reference PDF carries a /CreationDate
# and a local timezone otherwise, and every later byte comparison then differs on both.
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-1700000000}"
export TZ=UTC

# Which soffice is the reference. `$REF_SOFFICE` wins; otherwise whatever is on PATH.
#
# Hard-coding `soffice` is how a comparison silently scores against the wrong binary. On this
# machine PATH is 24.2.7.2 and the tree is calibrated to 26.2.4.2, and the two genuinely
# disagree -- `f445896e...docx` is 15 pages under 26.2.4.2 and 16 under 24.2.7.2, so a sweep
# that took PATH banked a `pages` failure for a document that matches.
REF="${REF_SOFFICE:-soffice}"
command -v "$REF" >/dev/null 2>&1 || { echo "no soffice at $REF" >&2; exit 3; }
SOFFICE_VERSION="$("$REF" --version 2>/dev/null | head -1)"
[ -n "$SOFFICE_VERSION" ] || { echo "no soffice at $REF" >&2; exit 1; }
echo "reference $(command -v "$REF") -- $SOFFICE_VERSION" >&2
echo "reference binary: $SOFFICE_VERSION" >&2

mkdir -p "$OUT/ref"
: > "$OUT/rows.tsv"

# Check 2's word count: tokens carrying at least one Unicode letter or digit, with the old
# `pdftotext | wc -w` figure alongside. Identical to `batch-check.sh`'s `words_of` on purpose —
# the two scripts are only comparable column for column if they count the same way, and a
# reference baseline taken with the old count against a sweep taken with the new one would
# read as a corpus-wide word failure. The reasoning, the probes and the reason it is python3
# and not grep or awk are all written out there; do not change one of these without the other.
words_of() {  # words_of <pdf> -> "<words> <rawwords>"
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
t = sys.stdin.buffer.read().decode("utf-8", "replace").split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t))'
}

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
echo "${#FILES[@]} documents across ${#DIRS[@]} batches" >&2

one() {  # one <index>
  local idx="$1" i=-1 f base ext stem id r rp rw rf runemb st rwraw
  local prof="$OUT/prof$idx"
  mkdir -p "$prof" "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    r="$OUT/ref/$id.pdf"

    if [ ! -f "$r" ]; then          # resumable: a sweep outlives whatever started it
      rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
      timeout 300 "$REF" -env:UserInstallation="file://$prof" \
        --headless --convert-to pdf --outdir "$OUT/t$idx" "$f" >/dev/null 2>&1
      [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$r"
    fi

    rp="-"; rw="-"; rf="-"; runemb="-"; st="ref-failed"; rwraw="-"
    if [ -f "$r" ]; then
      st="ok"
      rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r rw rwraw < <(words_of "$r")
      rf=$(pdffonts "$r" 2>/dev/null | tail -n +3 | grep -c .)
      # `emb` is NF-4, not NF-3: pdffonts ends every row with emb, sub, uni and a two-field
      # object id, so counting from NF-3 reads `sub` and is right only by accident for a font
      # whose type name is a single field.
      runemb=$(pdffonts "$r" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)
    fi

    # `refrawwords` last, after `status`, so `$7` stays the status column for every existing
    # reader of a stored baseline.
    printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" \
      "${f#"$ROOT_DIR"/}" "${ext,,}" "$rp" "$rw" "$rf" "$runemb" "$st" "$rwraw" >> "$OUT/rows.tsv"
  done
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

{
  printf "# reference binary: %s\n" "$SOFFICE_VERSION"
  printf "# corpus: %s  glob: %s\n" "$ROOT_DIR" "$GLOB"
  printf "# refwords = tokens carrying at least one Unicode letter or digit; refrawwords = pdftotext | wc -w\n"
  printf "# refwords is NOT comparable to a baseline taken before 2026-08-13 — see dotnet/probes/gate-01/results.md\n"
  printf "path\text\trefpages\trefwords\treffonts\trefunemb\tstatus\trefrawwords\n"
  sort "$OUT/rows.tsv"
} > "$OUT/ref-baseline.tsv"

total=$(wc -l < "$OUT/rows.tsv")
ok=$(awk -F'\t' '$7=="ok"' "$OUT/rows.tsv" | wc -l)
echo
echo "BATCHES ${DIRS[*]}"
echo "TOTAL $total  OK $ok  REF-CANNOT-RENDER $((total - ok))"
echo "TSV $OUT/ref-baseline.tsv"
[ "$total" -eq "$ok" ]
