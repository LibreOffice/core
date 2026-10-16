#!/usr/bin/env bash
# Sweep a track for the word gate *and* for unaccounted ink, in one pass.
#
#   track-ink-sweep.sh <corpus-root> <batch-glob> <outdir> [workers] [cli] [refdir]
#
#   track-ink-sweep.sh /c/sandbox/workdir/sample-files 'slides/batch-0*' out-slides 2
#
# `batch-check.sh` answers "is the right text on the right page". Once a track passes that
# — slides is at 152/163 with every remaining failure attributed — the only instrument left
# is where the ink lands, and running `pdf-image-diff.py` by hand over 163 documents is how
# a round gets spent. This does both from one pair of renderings.
#
# Writes:
#   rows.tsv    the same seven columns batch-check.sh writes
#   parity.tsv  those, sorted, with a header
#   ink.tsv     path, pages, |ink|% (unsigned), ink% (signed), major pages, verdict
#               Two ink columns, deliberately, and both are named in the file's own
#               header row.  For eleven rounds this script wrote ONE column, summed the
#               *signed* figure into it, and labelled the total `INK` -- while
#               `probes/slides-r39/ink-ranking.py`, the other half of the same skill,
#               headlined the *unsigned* one.  Two different measurements circulating
#               under one name is the trap this project has paid for repeatedly, and
#               here it lived inside a single skill.  Rank on unsigned; decide on
#               signed.  A signed sum lets a deficit cancel a surplus, so filling the
#               deficit reads as a regression.
#   cmp/<id>.txt  the full per-page pdf-image-diff report for every document
#
# Three things it does that the obvious version does not:
#
#   * Sums the per-page ink column itself, and therefore does NOT pass --quiet.
#     pdf-image-diff.py totals the major-page count and nothing else, and a page that is
#     not major still carries ink — dropping those understates a document by most of its
#     figure on a deck whose error is spread thin.
#   * Takes the CLI as an argument, so a sweep can run against a snapshot while the tree
#     is rebuilt underneath it. Checksum the snapshot against the tree before starting;
#     a stale copy passes every other check this skill prescribes.
#   * Reuses reference PDFs from an earlier run when given a refdir. The reference cannot
#     change while nothing touches soffice, and it is half the wall clock. Compare the two
#     runs' reference columns row for row afterwards, which is the check that says so.
set -uo pipefail

ROOT_DIR="${1:?usage: track-ink-sweep.sh <corpus-root> <batch-glob> <outdir> [workers] [cli] [refdir]}"
GLOB="${2:?batch glob, e.g. 'slides/batch-0*'}"
OUT="${3:?outdir — name it after the agent or the cluster, never 'out' or 'base'}"
WORKERS="${4:-2}"

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CLI="${5:-$REPO/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}"
REFDIR="${6:-}"

mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
[ -x "$CLI" ] || { echo "no CLI at $CLI — build it first" >&2; exit 1; }

DIFF="$REPO/.claude/skills/render-comparison/scripts/pdf-image-diff.py"
[ -f "$DIFF" ] || { echo "no pdf-image-diff.py at $DIFF" >&2; exit 1; }

# Which soffice is the reference. `$REF_SOFFICE` wins; otherwise whatever is on PATH.
#
# This script hard-coded `soffice` for its whole life while its sibling `batch-check.sh`
# honoured the variable and announced the resolved version -- so an ink sweep run beside a
# gate sweep silently scored against a DIFFERENT BINARY, and said nothing. On this machine
# PATH is 24.2.7.2 and the tree is calibrated to 26.2.4.2: measured on `words/done-005`,
# `f445896e…docx` is 15 pages against 24.2.7.2's 16 and against 26.2.4.2's 15, so the sweep
# banked a `pages` failure for a document that matches. The wrong reference does not fail; it
# answers a different question fluently. Announce it, so the run says which one it is.
REF="${REF_SOFFICE:-soffice}"
command -v "$REF" >/dev/null || { echo "no soffice at $REF" >&2; exit 1; }
echo "measuring $CLI" >&2
echo "reference $(command -v "$REF") -- $("$REF" --version 2>/dev/null | head -1)" >&2

mkdir -p "$OUT/ours" "$OUT/ref" "$OUT/cmp"
: > "$OUT/rows.tsv"
: > "$OUT/ink.tsv"

# Extractable words, for check 2. A token counts as a word iff it carries at least one
# Unicode letter or digit. Verbatim from `batch-check.sh` and `ref-baseline.sh`, and it must
# stay verbatim: this script used a bare `pdftotext | wc -w` for eleven rounds after the gate
# moved off it, so its `verdict` column silently disagreed with `MANIFEST.tsv`'s `status` and
# with every batch-check sweep. A sweep whose verdict column is a different metric from the
# scoreboard's looks exactly like a regression. Emits "<words> <rawwords>"; the raw figure is
# kept as the last TSV column so an old run under this script is still reconcilable.
words_of() {  # words_of <pdf> -> "<words> <rawwords> <glyphs>"
  # Verbatim from `batch-check.sh`, and it has to stay verbatim: for its whole life this
  # script carried its own older copy, returning tokens alone, and its verdict column was
  # therefore the PRE-2026-09-05 gate -- the token rule with a floor of 3 -- while the
  # scoreboard had moved to alphanumeric characters with a floor of 15. Two gates under one
  # column name, which is the trap this file's own header warns about for the ink columns.
  # Measured on the words track: 322 rows `match` under the old rule against 329 under the
  # new one, a seven-row disagreement that is the rule and not the tree.
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
b = sys.stdin.buffer.read().decode("utf-8", "replace")
t = b.split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t),
      sum(1 for c in b if c.isalnum()))'
}

# shellcheck disable=SC2086  # the glob is meant to expand
mapfile -t DIRS < <(cd "$ROOT_DIR" && ls -d $GLOB 2>/dev/null)
[ "${#DIRS[@]}" -gt 0 ] || { echo "no batches matched $GLOB under $ROOT_DIR" >&2; exit 1; }

# One inode, one render. This mount is case-insensitive and carries alias directory entries:
# `Foo.ppt` and `Foo.PPT` are the same file under two names, and how many of them a glob
# enumerates is not stable between runs. Two consequences, and the second is the dangerous one:
#
#   * the TOTAL line over-counts, which is merely misleading; and
#   * the per-format identity lower-cases the extension, so BOTH spellings map to the same id
#     and therefore the same output path. Two workers then render one document to one file
#     while a third step reads it. Slides round 63 caught this as a single document worth
#     94.14 of a 989 abs_ink total, appearing and disappearing between sweeps of an unchanged
#     tree.
#
# `find -printf '%D:%i\t%p'` keys on device and inode with no shell round trip, so it is safe
# for the filenames with spaces, brackets and per-cent signs this corpus contains. It cannot
# drop a genuine document: two genuine documents are never one inode.
#
# WHICH spelling survives matters, and no ordering rule gets it right. Some aliases
# are an upper-cased extension (`Foo.PPT` beside `Foo.ppt`) and some are a wholly
# lower-cased name (`template pilot logbook.xls` beside `Template Pilot Logbook.xls`),
# so "prefer lower case" and "prefer upper case" are each correct for one family and
# wrong for the other. Keeping the wrong one makes every manifest-keyed scorer report
# the document as unswept.
#
# git is the authority: it tracks exactly one spelling per inode, the real one. So the
# dedup prefers a tracked path and falls back to first-seen when the corpus is not a
# checkout. Verified: the deduped enumeration equals MANIFEST.tsv's 946 paths exactly,
# set for set, with nothing on either side.
TRACKED="$(mktemp)"
trap 'rm -f "$TRACKED"' EXIT
# core.quotePath=false is required, not cosmetic: git escapes non-ASCII paths by
# default, so the corpus's CJK filename would not match what find emits and the
# alias would win for that one document.
git -C "$ROOT_DIR" -c core.quotePath=false ls-files 2>/dev/null \
  | sed "s|^|$ROOT_DIR/|" > "$TRACKED" || :

mapfile -t FILES < <(
  for d in "${DIRS[@]}"; do
    # The in-scope extension list, kept identical to batch-check.sh's. It was NOT identical
    # until 2026-08-21: batch-check.sh was widened from thirteen extensions to thirty-four at
    # the start of this session, after two `.xlsm` in sheets/chartset-* turned out to have been
    # silently unmeasured -- and this sibling, written from the same list, kept the narrow one
    # and stayed blind to the same two documents for a dozen rounds. Fixing an instrument does
    # not fix its twin. If this list changes, change it in both.
    find "$ROOT_DIR/$d" -type f \
      \( -iname '*.doc'  -o -iname '*.docx' -o -iname '*.docm' -o -iname '*.dot' \
      -o -iname '*.dotx' -o -iname '*.dotm' -o -iname '*.rtf'  -o -iname '*.odt' \
      -o -iname '*.ott'  -o -iname '*.fodt' -o -iname '*.sxw' \
      -o -iname '*.xls'  -o -iname '*.xlsx' -o -iname '*.xlsm' -o -iname '*.xlsb' \
      -o -iname '*.xlt'  -o -iname '*.xltx' -o -iname '*.xltm' -o -iname '*.ods' \
      -o -iname '*.ots'  -o -iname '*.fods' -o -iname '*.csv'  -o -iname '*.sxc' \
      -o -iname '*.ppt'  -o -iname '*.pptx' -o -iname '*.pptm' -o -iname '*.pot' \
      -o -iname '*.potx' -o -iname '*.potm' -o -iname '*.ppsx' -o -iname '*.ppsm' \
      -o -iname '*.pps'  -o -iname '*.odp'  -o -iname '*.otp'  -o -iname '*.fodp' \
      -o -iname '*.sxi' \) -printf '%D:%i\t%p\n' 2>/dev/null
  done | awk -F'\t' -v T="$TRACKED" '
      BEGIN { while ((getline l < T) > 0) tracked[l] = 1 }
      { if (!($1 in best) || (tracked[$2] && !tracked[best[$1]])) best[$1] = $2 }
      END { for (k in best) print best[k] }
    ' | sort
)
echo "documents: ${#FILES[@]}" >&2

one() {  # one <index>
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf un v ink major pages owraw rwraw
  local prof="$OUT/prof$idx"
  mkdir -p "$prof" "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$OUT/ref/$id.pdf"

    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 300 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$o"

    if [ -n "$REFDIR" ] && [ -f "$REFDIR/$id.pdf" ]; then
      cp -f "$REFDIR/$id.pdf" "$r"
    else
      rm -rf "$OUT/t$idx"; mkdir -p "$OUT/t$idx"
      timeout 300 "$REF" -env:UserInstallation="file://$prof" \
        --headless --convert-to pdf --outdir "$OUT/t$idx" "$f" >/dev/null 2>&1
      [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$r"
    fi

    op="-"; rp="-"; ow="-"; rw="-"; of="-"; rf="-"; un="-"; owraw="-"; rwraw="-"; og="-"; rg="-"
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
      # The gate's own rule, `batch-check.sh`:298-302: alphanumeric CHARACTERS, band
      # max(2%, 15). Not tokens with a floor of 3, which is what stood here.
      if [ "$rg" -gt 0 ] 2>/dev/null; then
        awk -v a="$og" -v b="$rg" 'BEGIN{d=(a>b?a-b:b-a); exit !(d > b*0.02 && d > 15)}' \
          && v="${v:+$v,}words"
      elif [ "${og:-0}" -gt 15 ]; then v="${v:+$v,}words"
      fi
      [ "${un:-0}" = "0" ] || v="${v:+$v,}unembedded"
      [ -n "$v" ] || v="match"
    fi

    # `glyphs` last, after `rawwords`, exactly as `batch-check.sh` appends it: every reader
    # that reaches for an existing column keeps working and the two files stay joinable.
    printf "%s\t%s\t%s/%s\t%s/%s\t%s/%s\t%s\t%s\t%s/%s\t%s/%s\n" \
      "${f#"$ROOT_DIR"/}" "${ext,,}" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v" \
      "$owraw" "$rwraw" "$og" "$rg" >> "$OUT/rows.tsv"

    # Ink, whenever both sides rendered and the page counts agree. The tool refuses a
    # document whose counts differ, and rightly: page 3 against a different page 3 makes
    # every region it reports an artefact.
    ink="-"; sink="-"; major="-"; pages="-"; drift="-"; worst="-"; mean="-"
    if [ -f "$o" ] && [ -f "$r" ] && [ "$op" = "$rp" ]; then
      rm -rf "$OUT/c$idx"
      timeout 900 python3 "$DIFF" "$o" "$r" --outdir "$OUT/c$idx" > "$OUT/cmp/$id.txt" 2>&1
      rm -rf "$OUT/c$idx"          # the PNGs are large and disposable; the report is not
      # pdf-image-diff.py prints: page  diff%  ink%(signed)  |ink|%(unsigned)  regions  verdict
      ink=$(awk -F'\t' '$1 ~ /^[0-9]+$/ && $4 ~ /^[0-9.]+$/ {s+=$4} END{printf "%.2f", s}' \
            "$OUT/cmp/$id.txt")
      sink=$(awk -F'\t' '$1 ~ /^[0-9]+$/ && $3 ~ /^-?[0-9.]+$/ {s+=$3} END{printf "%.2f", s}' \
            "$OUT/cmp/$id.txt")
      # `abs_ink` is a SUM, so it is weighted by length: a 44-page document averaging 0.23 %
      # a page outranks a one-page chart that is 5.18 % wrong. Round 183 spent most of a
      # round on `docs-quality-MA.IMS.00001` for that reason -- it headed the `drift`-free
      # ranking at 10.22 and its mean is 0.23, which is the raster floor. So the worst single
      # page and the mean are written beside the sum, and the worst page is what says whether
      # a document holds a defect worth chasing.
      worst=$(awk -F'\t' '$1 ~ /^[0-9]+$/ && $4 ~ /^[0-9.]+$/ {if ($4+0 > m) m=$4+0}
                           END{printf "%.2f", m}' "$OUT/cmp/$id.txt")
      mean=$(awk -F'\t' '$1 ~ /^[0-9]+$/ && $4 ~ /^[0-9.]+$/ {s+=$4; n++}
                          END{printf "%.2f", (n ? s/n : 0)}' "$OUT/cmp/$id.txt")
      major=$(awk '/pages, .* with major differences/{print $3}' "$OUT/cmp/$id.txt")
      # How many pages hold content the reference puts on a different page. Equal page counts
      # do not prove alignment: a block lost early and made up later leaves every page between
      # compared against its neighbour, and its ink is then measuring the offset. The warning
      # is on stderr, which the redirect above already folds into the report.
      drift=$(awk '/^WARNING: [0-9]+ of [0-9]+ pages hold different content/{print $2; exit}' \
              "$OUT/cmp/$id.txt")
      [ -n "$drift" ] || drift=0
      pages="$op"
      [ -n "$ink" ] || ink="?"
      [ -n "$sink" ] || sink="?"
      [ -n "$major" ] || major="?"
      [ -n "$worst" ] || worst="?"
      [ -n "$mean" ] || mean="?"
    fi
    printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" \
      "${f#"$ROOT_DIR"/}" "$pages" "$worst" "$mean" "$ink" "$sink" "$major" "$drift" "$v" \
      >> "$OUT/ink.tsv"
  done
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

{
  printf "# words = tokens carrying at least one Unicode letter or digit; rawwords = pdftotext | wc -w\n"
  printf "# glyphs = alphanumeric characters, ours/reference -- THIS is what the verdict uses\n"
  printf "path\text\tpages\twords\tfonts\tunemb\tverdict\trawwords\tglyphs\n"
  sort "$OUT/rows.tsv"
} > "$OUT/parity.tsv"
{
  printf "# worst = the WORST single page's unsigned |ink|%% -- RANK THE TRACK ON THIS ONE\n"
  printf "# mean  = the same column averaged over the pages. Below about 0.3 is the raster\n"
  printf "#         floor for a text-heavy document and says the document holds no defect.\n"
  printf "# abs_ink = the same column SUMMED, so it is weighted by length: a 44-page document\n"
  printf "#         at 0.23 a page outranks a one-page chart that is 5.18%% wrong, and it is\n"
  printf "#         the chart that has the defect. Use it for a track total, not for a ranking.\n"
  printf "# signed_ink = sum of the per-page SIGNED ink%% column -- decide direction on this one\n"
  printf "# drift = pages holding content the reference puts elsewhere. NON-ZERO VOIDS THE INK:\n"
  printf "#         those pages are compared against the wrong page, so one lost page reads as\n"
  printf "#         hundreds of defects. Explain the pagination before ranking such a row.\n"
  printf "path\tpages\tworst\tmean\tabs_ink\tsigned_ink\tmajor\tdrift\tverdict\n"
  sort "$OUT/ink.tsv"
} > "$OUT/ink.tsv.tmp" && mv -f "$OUT/ink.tsv.tmp" "$OUT/ink.tsv"

total=$(wc -l < "$OUT/rows.tsv")
match=$(awk -F'\t' '$7=="match"' "$OUT/rows.tsv" | wc -l)
reffail=$(awk -F'\t' '$7=="ref-failed" || $7=="both-failed"' "$OUT/rows.tsv" | wc -l)
echo
echo "BATCHES ${DIRS[*]}"
echo "TOTAL $total  MATCH $match  REF-CANNOT-RENDER $reffail"
# Both figures, both labelled, and the invariant between them checked.  A sum of signed
# page figures can never exceed the sum of the same pages taken unsigned; if it does, the
# two columns were not read off the same pages and no ranking built on them means anything.
awk -F'\t' '/^#/ || $1=="path" {next}
            $5!="-" && $5!="?" {a+=$5; s+=$6; m+=$7; n++; if ($3+0 > w) { w=$3+0; wd=$1 }}
            END{printf "ABS-INK %.2f (unsigned |ink|%%, a track total)  SIGNED-INK %.2f (ink%%, direction)  MAJOR PAGES %d  over %d documents\n", a, s, m, n;
                printf "WORST PAGE %.2f (unsigned |ink|%%, RANKS) on %s\n", w, wd;
                if ((s<0?-s:s) > a + 0.01)
                  printf "INVARIANT VIOLATED: |signed| %.2f > unsigned %.2f\n", (s<0?-s:s), a}' "$OUT/ink.tsv"
echo "TSV $OUT/parity.tsv  $OUT/ink.tsv"
