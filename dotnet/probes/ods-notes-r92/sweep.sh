#!/usr/bin/env bash
# Prove that X-Ray.Office renders a corpus batch the way LibreOffice does.
#
#   batch-check.sh <corpus-root> <batch-glob> [outdir] [workers]
#
#   batch-check.sh /c/sandbox/workdir/sample-files 'batch-001'          # one batch
#   batch-check.sh /c/sandbox/workdir/sample-files 'batch-0[0-1]*'      # batches 1-19, a regression sweep
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
#
# NOT COMPARABLE TO ANY SCOREBOARD RECORDED BEFORE 2026-09-05. Check 2 has moved twice.
# It counted `pdftotext … | wc -w`; then, from 2026-08-13, only tokens carrying a Unicode
# letter or digit; and it now compares **alphanumeric characters** rather than tokens at all.
# Every earlier count is still emitted — `words/rawwords/glyphs`, in that order, after the
# verdict — so any run reconciles with any older one document by document. A figure quoted
# without saying which metric produced it is ambiguous.
#
# **Why it moved to characters.** A token count asks `pdftotext` a question it cannot answer
# stably. 26.2.4.2 writes each kerned cluster as its own text-showing operator, so it reads
# `2-Way Horizontal Hierarchy` as `2 -W ay H or i z on t a l H ie r a r ch y` — measured on
# `006_2-Way_Horizontal_Hierarchy…pptx`, words 126 / 162 / 124 across 24.2 / 26.2 / ours
# against glyphs 645 / 645 / 645. Under the token rule that put ~35 invented words on each of
# 89 decks and was the single largest obstacle to moving this gate to the version the tree
# targets.
#
# Measured over all 947 documents at one commit, same binary, only the metric changed:
# **850 → 854 match.** Fourteen rows that were failing now pass, three of them with
# *identical* character counts (`053_Flow_Wave_Process` 889/889, `032_BCG_Growth-Share_Matrix`
# 1153/1153, `100_Business_Case_Template` 512/512) — the tokenisation was the whole
# disagreement. Ten rows that were passing now fail, and they are true positives the token
# rule could not see: `042_Business_monthly_budget` has **268 words on both sides and 114
# fewer characters**, `Aircraft_Database.xlsx` is 3 853 characters short at an 0.9 % token
# difference. So the metric is both more robust to grouping and more sensitive to loss.
#
# The band is unchanged in shape — max(2 %, floor) — and the floor converts by measurement:
# this corpus averages **5.12 glyphs per word**, so the old 3-token floor is 15 characters.
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

# Which CLI to measure. `$XRAY_OFFICE_CLI` wins; otherwise the tree this script was invoked
# *from*, not the tree it lives in.
#
# A git worktree shares .claude/ with the main checkout, so `dirname $BASH_SOURCE/../../../..`
# resolves to the MAIN checkout however deep in a worktree you are. An agent sweeping its own
# branch was silently measuring another session's binary — and the first such sweep looked
# entirely normal, because the two checkouts happened to sit near the same commit. Cost two
# sweeps before the pattern was recognised.
#
# $PWD is right because the workflow is always run from the tree under test. The check below
# refuses rather than guesses when it is not.
CLI="${XRAY_OFFICE_CLI:-}"
if [ -z "$CLI" ]; then
  ROOT="$(git -C "$PWD" rev-parse --show-toplevel 2>/dev/null || echo "$PWD")"
  CLI="$ROOT/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
fi
[ -x "$CLI" ] || {
  echo "no CLI at $CLI" >&2
  echo "run this from the tree you want measured, or set XRAY_OFFICE_CLI" >&2
  exit 1
}
echo "measuring $CLI" >&2

# Which soffice is the reference. `$REF_SOFFICE` wins; otherwise whatever is on PATH.
#
# ANNOUNCED, because taking it from PATH silently is a trap this script fell into itself. The
# calibration target is /opt/libreoffice26.2 and the image also carries 24.2.7.2 at /usr/bin/soffice,
# so a sweep launched without PATH pointed at the target measures the wrong reference and produces a
# table that looks entirely normal and is not comparable to any round's figures. One whole-corpus
# sweep -- 1285 documents, about three hours -- was scored against 24.2.7.2 that way and thrown out.
# The version is printed rather than the path alone, because the path does not say which build it is.
REF="${REF_SOFFICE:-soffice}"
command -v "$REF" >/dev/null || { echo "no soffice at $REF" >&2; exit 1; }
echo "reference $(command -v "$REF") -- $("$REF" --version 2>/dev/null | head -1)" >&2


mkdir -p "$OUT/ours" "$OUT/ref"
: > "$OUT/rows.tsv"

# Extractable words, for check 2. A token counts as a word iff it carries at least one
# Unicode letter or digit — categories L* or N*, which is what Python's str.isalnum() is.
# Emits "<words> <rawwords>", the second being the old `pdftotext | wc -w` figure, kept so
# every run can still reproduce the verdict it would have given before this changed.
#
# WHY, because this is the check that decides "is the same text present" and the answer turns
# on what counts as text. Both renderers write list labels and rendering markers into the PDF
# text layer as real text-showing operators with real ToUnicode mappings, and `wc -w` scored
# every one of them as a word:
#
#   * an authored probe holding its body text fixed at 64 words and varying only the list
#     label reads 64 with no list and 76 with twelve U+2022 bullets, twelve U+F0A7
#     Symbol/Wingdings bullets, or twelve U+2013 dashes — a 19% swing on a document whose
#     text did not change by one character;
#   * `ODs-February-2022-Airbus-Commercial-Aircraft.xlsx` — the reference emits `###` **1101
#     times**, the column-too-narrow marker. Those 1101 "words" were cancelling a real
#     895-word surplus in our own output, so the document passed check 2 by luck;
#   * `fy2011-aip-grants.xls` — 11538 standalone `$` and `-` against the reference's 9020,
#     the accounting number format for a zero cell. Raw counts differ by 2518 and fail;
#     letter-or-digit counts are **43201 and 43201**, exact.
#
# A column-overflow marker is not text and neither is a bullet the renderer chose. The gate
# was manufacturing failures out of them — the same shape as the pdf-ops.py stroke pairing
# that once produced 142 phantom `box` notes and cost a dispatched round.
#
# Three things this deliberately does NOT do.
#   * It does not widen the 2% band. That band separates hyphenation drift from missing text;
#     widening it to absorb a systematic term hides the term instead of removing it.
#   * It does not strip an enumerated set of code points, which is the obvious fix and is
#     wrong twice over: the set would be fitted to the documents in hand, and it does not even
#     work — measured on the slides track, the reference writes its Wingdings bullets at
#     U+F06E/U+F06C/U+F0D8/U+F0A7 and X-Ray.Office writes the same glyphs at
#     U+E439/U+E5CD/U+E46F/U+E437. A list built from one side strips nothing on the other.
#   * It does not drop short tokens. A numbering label (`1.`, `iv.`, `a)`) carries a digit or
#     a letter and stays a word on both sides; the probe above reads 76 for a numbered list
#     under both metrics. That is the definition working, not a hole in it.
#
# python3 rather than grep or awk, and this is not a style preference. This image carries only
# the `C` and `C.utf8` locales. Measured on a file of one token per script: `grep -c
# '[[:alnum:]]'` under C.utf8 counts Cyrillic and Greek but **not Han**, and mawk — the default
# awk here — counts neither, scoring ASCII only. Either would silently drop every wholly-CJK
# or wholly-Cyrillic token while looking perfectly correct on the English majority of the
# corpus, which is the `fc-match` trap in a second dimension. str.isalnum() is Unicode by
# construction and cannot be changed by the environment. The split is `str.split()`, which
# reproduces `wc -w` on 1068 of 1068 corpus PDFs — so the tokenisation is untouched and the
# only thing that changed is the filter.
# This function is also the seam for the in-process extractor (`xray-office analyze`): it returns
# exactly the pair that verb should emit, so replacing poppler here is a substitution of this
# body and nothing else in the script. The definition above is the durable part and must not be
# re-decided by the reimplementation — and the `wc -w` reproduction is a *control on poppler's
# tokenisation*, so a different reader has to re-establish it against the `rawwords` column of a
# stored sweep before any verdict it produces is comparable to one here.
words_of() {  # words_of <pdf> -> "<words> <rawwords> <glyphs>"
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
b = sys.stdin.buffer.read().decode("utf-8", "replace")
t = b.split()
# `glyphs` counts alphanumeric *characters*, ignoring how they are grouped, and it is what
# the verdict now uses. A token count asks a question `pdftotext` cannot answer stably:
# 26.2.4.2 writes each kerned cluster as its own text-showing operator, so it reads
# `2-Way Horizontal Hierarchy` as `2 -W ay H or i z on t a l H ie r a r ch y`. Measured on
# `006_2-Way_Horizontal_Hierarchy…pptx`: words 126 / 162 / 124 for 24.2 / 26.2 / ours,
# and glyphs 645 / 645 / 645. Same ink, same characters, 36 invented words.
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
    # Every extension `dotnet/CLAUDE.md` declares in scope. The macro-enabled and template
    # forms are here deliberately: a document this list omits is not reported as skipped, it
    # simply never appears, so the TOTAL line looks correct and the gate is silently blind to
    # it. Two `.xlsm` sat in `sheets/chartset-*` unmeasured until the count was reconciled
    # against the files on disk.
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

one() {  # one <index>
  local idx="$1" i=-1 f base ext stem id o r op rp ow rw of rf un v owraw rwraw og rg
  local prof="$OUT/prof$idx"
  mkdir -p "$prof" "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$OUT/ref/$id.pdf"

    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout -k 30 "${RENDER_TIMEOUT:-240}" "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$o"

    # $REUSE_REF names a directory of reference PDFs from an earlier run of this same script
    # over the same corpus. The reference binary cannot be reached by a diff confined to
    # `dotnet/src`, so re-rendering it to compare two of our own binaries is forty minutes
    # spent reproducing bytes that cannot have moved -- and the check that it really is the
    # same bytes is that the file is copied rather than regenerated. A row whose reference is
    # missing from the bank is still rendered, so a partial bank degrades to a full run.
    if [ -n "${REUSE_REF:-}" ] && [ -f "$REUSE_REF/$id.pdf" ]; then
      cp -f "$REUSE_REF/$id.pdf" "$r"
    else
      rm -rf "$OUT/t$idx"; mkdir -p "$OUT/t$idx"
      timeout -k 30 "${RENDER_TIMEOUT:-240}" "$REF" -env:UserInstallation="file://$prof" \
        --headless --convert-to pdf --outdir "$OUT/t$idx" "$f" >/dev/null 2>&1
      [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$r"
    fi

    op="-"; rp="-"; ow="-"; rw="-"; of="-"; rf="-"; un="-"; owraw="-"; rwraw="-"; og="-"; rg="-"
    if [ -f "$o" ]; then
      op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r ow owraw og < <(words_of "$o")
      of=$(pdffonts "$o" 2>/dev/null | tail -n +3 | grep -c .)
      # The `emb` column, found by its position from the *right*: pdffonts ends every row with
      # emb, sub, uni and a two-field object id, so `emb` is NF-4 and not NF-3. Counting from
      # NF-3 reads `sub` instead, and it happens to give the right answer only for a font whose
      # type name is two or three fields — "Type 1", "Type 1C", "CID Type 0C". Every font
      # X-Ray.Office writes is "TrueType", one field, so this check tested nothing about our own
      # output until it was corrected; measured on a PDF embedding ten faces and naming an
      # eleventh, which it scored as zero unembedded.
      un=$(pdffonts "$o" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)
    fi
    if [ -f "$r" ]; then
      rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r rw rwraw rg < <(words_of "$r")
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
      # The band is unchanged and deliberately so — what changed is `$ow`/`$rw`, above.
      # Replayed over 9552 stored rows from every probe TSV in the tree, this block returns
      # the stored verdict on all 9552 when fed the raw counts, so the rule is untouched and
      # only its input moved.
      #
      # **The input is `glyphs`, not `words`, and that is the 2026-09-05 change.** The band
      # is the same shape -- max(2%, floor) -- and the floor moves from 3 tokens to 15
      # characters, which is three average words on this corpus, so the rule is transferred
      # rather than loosened. The relative half needs no conversion at all: 2% of a count is
      # 2% of it however the text is grouped.
      if [ "$rg" -gt 0 ] 2>/dev/null; then
        awk -v a="$og" -v b="$rg" 'BEGIN{d=(a>b?a-b:b-a); exit !(d > b*0.02 && d > 15)}' \
          && v="${v:+$v,}words"
      elif [ "${og:-0}" -gt 15 ]; then v="${v:+$v,}words"
      fi
      [ "${un:-0}" = "0" ] || v="${v:+$v,}unembedded"
      [ -n "$v" ] || v="match"
    fi

    # `glyphs` is appended last, after `rawwords`, for the same reason `rawwords` was: every
    # reader that reaches for an existing column keeps working, and a new scoreboard stays
    # convertible back to an old one instead of merely incomparable to it.
    # `rawwords` is appended *after* the verdict rather than beside `words`, so that every
    # reader that reaches for `$7` — this script's own tallies, ref-baseline.sh's sibling
    # columns, the replay harness in dotnet/probes/words-rebase-02/verdict.py, and eleven
    # rounds of stored TSVs — keeps working unchanged. It is what makes a new scoreboard
    # convertible back to an old one instead of merely incomparable to it.
    printf "%s\t%s\t%s/%s\t%s/%s\t%s/%s\t%s\t%s\t%s/%s\t%s/%s\n" \
      "${f#"$ROOT_DIR"/}" "${ext,,}" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v" \
      "$owraw" "$rwraw" "$og" "$rg" >> "$OUT/rows.tsv"
  done
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

{
  printf "# words = tokens carrying at least one Unicode letter or digit; rawwords = pdftotext | wc -w\n"
  printf "# NOT comparable to any scoreboard recorded before 2026-08-13 — see dotnet/probes/gate-01/results.md\n"
  printf "path\text\tpages\twords\tfonts\tunemb\tverdict\trawwords\n"
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
