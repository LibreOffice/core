# Corpus batch status

Progress driving the [sample-files](https://github.com/theolivenbaum/sample-files) corpus —
534 real-world documents ordered by rendering complexity — to parity with LibreOffice.

The method, the reasoning behind it and the dispatch rules live in the
[`corpus-batches`](../.claude/skills/corpus-batches/SKILL.md) skill. This file is only the
scoreboard.

## The rule

A batch is **done** when it matches *and* every earlier batch in its track still matches.

```sh
S=.claude/skills/corpus-batches/scripts
$S/batch-check.sh /workspace/sample-files 'words/batch-007' out 3      # the batch you are on
$S/batch-check.sh /workspace/sample-files 'words/batch-00[1-6]' out 3  # the gate for moving on
```

Advancing on the first condition alone is how a corpus rots from the front: rendering
errors cascade, so a fix aimed at batch *n* routinely breaks batch *n−4* in a way that looks
nothing like the change.

## How the tracks advance

**The three tracks advance independently, one agent each, and never wait for one another.**
A track's agent takes its lowest still-open batch; when it lands, that batch's predecessor is
re-proved and the next batch is dispatched for that track alone. Slides being three batches
ahead of sheets is not a problem to correct — it is the point, because a track that waits is
an agent idle.

Sheets was originally scheduled last on the grounds that a spreadsheet's value is in its
cells rather than its pagination. That ordering is retired: the track turned out to hold the
corpus's largest systematic defects — one workbook paginating 1170 pages against 220 — and
deferring it was hiding them rather than deprioritising them.

## Levels one and two: done, re-proved on the merged branch

At `28940d76f`, batches 001 and 002 swept together on each track: **words 20/20, slides
19/19, sheets 20/20 — 59 of 59.** Not the agents' own figures from their worktrees, but a
fresh sweep of the merged result, which is the only number that describes what is on the
branch.

Whole-track figures at the same commit: words **137/200** (page error 124), slides
**147/163** (ppt 47/51, pptx 100/112), sheets **108/171** (page error 860).

Words is now **146/200** by addition — 143 whole-track at the eighth round, plus the ninth
round's three, which re-swept 001–007 and 010–017 only. Sheets is now
**125/171** (page error 222) — see "Sheets batches 005–008" below.

## Level one: the image check

`batch-001` on all three tracks passes the word gate — **29 of 29 documents**. Twenty-seven
of the twenty-nine also pass the image check. What remains is three pages across two slide
decks, each with a named cause of feature size rather than a defect: `<a:lum>`/`<a:grayscl>`
blip effects needing the PDF backend to decode and re-encode, PPT auto-numbering across
placeholders, and one undiagnosed measurement difference.

## Status key

| Mark | Means |
|---|---|
| `—` | not started |
| `n/10` | measured, that many documents at parity |
| `WIP` | an agent is working it now |
| `✅` | full parity, **and** every earlier batch in the track re-proved after the last change |
| `· ceiling` | the residue is measured to be the word gate's ceiling, not a defect — closing it would need the output made worse. Switch instruments rather than chase the number |

Record measured numbers, never expected ones. A number here that was not produced by
`batch-check.sh` is worse than a blank.

## Baseline: the whole words track, measured

All 202 documents at `1d13c1e0a`, before any fix: **84 match, 118 do not (42%).**

Sorting the failures by page delta — ours minus LibreOffice — is what turns 118 problems
into three:

| Cluster | Documents | Shape |
|---|---|---|
| **Under-paginate** | 70 | We produce *fewer* pages. Page deltas run 5–43% while word deltas are only 1–6%: `A_320.doc` is 64 pages short and still carries 94% of the text. We are not losing content, we are **fitting too much of it onto each page** — a vertical-budget error (line height, text-area height, or the line-fits rule) manufacturing seventy unrelated-looking failures. |
| **Over-paginate** | 32 | We produce *more* pages, with word counts near-identical. About *where* content breaks, not what it contains. Opposite sign, so probably a different rule. |
| **Text loss** | 16 | Pagination already correct and text still short — 2%–22%, always short, never long. No reflow cascade confusing the picture, so whatever is missing never reached the page at all. |

Failure rates were 42% for `doc` and 43% for `docx`. Near-equal rates say the cause is
**downstream of both readers, in the layout they share** — which is why the work is split by
cluster and not by format.

Cluster lists live in the scratch directory as `c1-underpaginate.tsv`,
`c2-overpaginate.tsv`, `c3-textloss.tsv` (path, delta, pages ours/ref, words ours/ref).

## After the first round: words, measured on the merged branch

Four agents, one per cluster plus the slides track, merged one at a time and re-swept whole
at `fea15726e`:

| | baseline | merged | |
|---|---|---|---|
| full match | 84 | **98** | +14 |
| exactly correct page count | 100 | **110** | +10 |
| total absolute page error | 385 | **306** | −21% |
| failing on pages *and* words | 45 | **26** | −19 |

**The merged result beats every agent's own figure** — they measured 89 and 86 in their
separate worktrees against different bases. The fixes compose: removing a spacing error and
removing a line-height error each expose the other while alone, and land together.

That is the argument for re-sweeping rather than adding up. It is also the argument for not
reverting on a per-agent number: the over-pagination agent's work cost 5 matches *in
isolation* and is part of a +14 here.

### Two batches fell, and the reason matters

`batch-004` 8→7 and `batch-015` 3→0. The second looks alarming and is mostly the fixes
working:

```
                                          before                    after
644730BRI…doc          match   5/5  words 2315/2330    pages  4/5  words 2330/2330
A320SimNotes.doc       pages  36/42 words 13716/14025  pages 37/42 words 13949/14025
…Experience logbook    words   6/6  words  864/1110    words  6/6  words 1180/1110
```

Word counts rose on every document in the batch and one reached the reference **exactly**.
The recovered text — headers and footers that were previously dropped entirely — then
changed what fits on each page, so documents passing on a ±1 page count stopped passing.
Text recovery is upstream of pagination; getting the content right first and the page
boundaries right second is the correct order, not a regression to revert.

Three now *over*-count words (1180 vs 1110, 1108 vs 1081), which is a real defect in the
recovery — a header drawn on pages the reference does not put one on, or drawn twice. That
is the next thing to chase in this batch.

Nineteen defects across the four, the ones with the widest reach being: DOCX `w:spacing` and
`w:ind` inherited element-wise rather than attribute-wise (a paragraph stating only `w:line`
silently lost its style's spacing); `sprmSBkc` read with 0 and 1 swapped, so every
*continuous* section break in a DOC became a page break; a table cell sized from its ink
rather than its advance; headers and footers dropped whenever `w:top` equalled `w:header`;
and, on slides, rendering ignoring the placeholder inheritance chain that extraction had
resolved all along.

## After the second round: words

Two agents, each given a root cause the first round had *named* rather than a cluster to
explore. Merged one at a time and re-swept whole at `d1c401dcd`:

| | baseline | round 1 | round 2 |
|---|---|---|---|
| full match | 84 | 98 | **107** |
| exactly correct page count | 100 | 110 | **119** |
| total absolute page error | 385 | 306 | **297** |

**107 again beats both agents' own figures** (104 and 102 against a shared 98 baseline), and
this time the compounding was predicted: each agent named the *other's* fix as its blocker.
The row-splitting agent's leftovers showed 33.5 pt of accumulated vertical drift it put down
to line height; the line-height agent's leftovers had advances matching to 0.01 pt while
LibreOffice still emitted near-empty pages, which it put down to table rows. Both were right.

### The most valuable finding was a wrong diagnosis

The first round measured line heights 6% short and attributed it to `hhea`-vs-`OS/2`
precedence. The second round **reproduced the measurement to the digit and refuted the
cause**: 12.65 pt is Liberation Serif's line box at 11 pt and 13.45 pt is Carlito's — the
paragraph was being laid out in the wrong font. Runs saying `w:rFonts w:asciiTheme="minorHAnsi"`
reference the theme's font scheme, whose minor Latin face is Calibri; nothing read
`w:asciiTheme` or its seven companions, so they fell back to `w:docDefaults` and Times New
Roman. **112 of the words track's 136 DOCX files name their fonts this way.**

This is the case for telling agents to verify a predecessor's claim rather than build on it.
The measurement was sound and the diagnosis attached to it would have sent a whole round
into font metrics instead of theme resolution.

The precedence rule was corrected anyway — `LineSpacing.cs`, `research/06-rendering.md` B.4
and two tests all had it backwards — and it is exactly neutral on this corpus, the two rules
disagreeing on three installed faces, all CJK.

### What remains: 95 failures

| Shape | Documents |
|---|---|
| under-paginate | 65 |
| over-paginate | 18 |
| pages right, words wrong | 12 |

Failure rates are 30/66 for `doc` and 65/136 for `docx` — still near-equal, so the residue
is still in shared layout rather than in either reader. Named but unfixed leads: unknown
families falling back through fontconfig to DejaVu Sans where LibreOffice uses its own
`VCL.xcu` table and picks Liberation Sans (1.2% of the em apart, and `Aptos` — Microsoft
365's new default — is in the corpus); `A_320.doc`'s in-cell line pitch of 13.0 pt against
ours of 12.65 with font and size agreeing, most likely a `sprmPDyaLine` at-least value not
reaching cell paragraphs; and a header height under-reserved by 27.7 pt on one document.

## Slides: baseline and first round

Baseline at `1d13c1e0a` was **53 of 150** (the sweep was cut short at 150 of 163). After the
first round, measured whole on the merged branch at `fea15726e`: **84 of 163.**

The headline is what the two formats did separately:

| | baseline | merged |
|---|---|---|
| `pptx` | 38/101 | **67/112** |
| `ppt` | 15/49 | **17/51** |

PPTX nearly doubled; **PPT barely moved.** The first round fixed the OOXML placeholder
inheritance chain — resolved for extraction all along, ignored by rendering — and the binary
path never had the corresponding master-shape walk at all. So the tracks have diverged and
the remaining slides work is mostly `.ppt`.

**Every page count in the whole track is correct**, baseline and merged alike. Slides is
entirely about what reaches the page. Of the 79 remaining failures, 53 lose text and 25 emit
*more* than the reference — both real, and different bugs.

## After the second round: slides — 133 of 163

Two agents, one per format, each given a named cause. Merged one at a time, with a sweep
between them to isolate the first:

| | baseline | round 1 | r2, ppt merged | r2, both |
|---|---|---|---|---|
| `ppt` | 15/49 | 17/51 | 36/51 | **42/51** |
| `pptx` | 38/101 | 67/112 | 67/112 | **91/112** |
| total | 53/150 | 84/163 | 103/163 | **133/163** |

**The strongest compounding yet: 133 against individual claims of 103 and 111.** The
intermediate sweep is why that is knowable — it confirmed the PPT agent's *inferred* claim
that pptx could not have moved (all its commits were under `MsBinary`), and it isolates the
+6 that `.ppt` gained from the PPTX agent's shared-layout fix, which that agent had measured
as +3 in its own worktree.

Four slides batches are now at full parity — 002, 004, 006, 011 — the first anywhere in the
corpus besides `words/batch-001`.

The two widest-reaching defects were not about text at all: a chart's data-label flags are
read as `value_or(!bMSO2007Doc)` and the `!bMSO2007Doc` half was hard-coded true, so on a
file Office 2007 wrote a bare `<c:dLbls/>` turned every label on (one deck drew
"Manufacturing" 349 times); and a grouped shape's `a:chExt`/`a:ext` ratio scaled the
placement matrix but not the text rectangle, so 12 pt runs were measured against a box a
thousandth of an inch wide and chopped away — **39 of 112 decks hold a scaled group**, which
is what a deck converted from `.ppt` looks like.

### What remains: 30 failures

Every page count is still correct. The tail is now genuinely small and several of it are not
our defects:

- `010605Vul.ppt` (−100%) — the only document that fails outright; its directory entry
  declares a `PowerPoint Document` stream above 2 GiB in an 84 kB file. A leniency fix was
  written, found to turn a clean error into an OOM, and reverted.
- `Sylva%20introduction%20session.pptx` (−8.6%) — **the reference is wrong**: LibreOffice
  shapes that deck's title per character (`S e p t m b r`), inflating its own word count. We
  render it correctly.
- Two decks at +2.5% are byte-twins of each other; three more sit inside ±3%, at the edge of
  the 2% band.
- `8_P-Pavese` and `16 - UTM - (NASA)` put EMF image text in the PDF text layer where
  LibreOffice does not — arguably better output, and matching would mean preferring EMF+
  records over the downlevel EMF fallback.

## After the third round

Four agents at `f182d6e98`: font substitution, words under-pagination, words
over-pagination and text loss, and the slides tail.

| words | baseline | r1 | r2 | r3 |
|---|---|---|---|---|
| full match | 84 | 98 | 107 | **114** |
| exactly correct page count | 100 | 110 | 119 | **127** |
| total absolute page error | 385 | 306 | 297 | **149** |

| slides | baseline | r1 | r2 | r3 |
|---|---|---|---|---|
| `ppt` | 15/49 | 17/51 | 42/51 | **44/51** |
| `pptx` | 38/101 | 67/112 | 91/112 | **91/112** |
| total | 53/150 | 84/163 | 133/163 | **135/163** |

**Total absolute page error halved, 297 → 149.** That is the number to read this round by:
the match count rose 7, but the corpus moved from "wrong by a lot" to "wrong by one page".
47 of the 88 remaining words failures are now off by exactly ±1 page, and only 12 documents
are off by 3 or more.

### The two premises I got wrong in the briefs

Worth recording because the agents caught both by measuring rather than by reading:

- **The VCL table was already there.** I briefed "Paperless resolves through fontconfig
  alone; LibreOffice consults `VCL.xcu` first". Paperless already had the chains and already
  walked them first. The defect was one layer down — it carried only *half* of each entry,
  dropping `FontType`, so once a chain named nothing installed (the common case on Linux,
  where all fourteen of Tahoma's candidates are absent) the shape was guessed from the family
  name. Face agreement with LibreOffice went 6/55 → 47/55.
- **And the fallback direction was backwards.** Verifying against the installed `soffice`
  rather than its source showed LibreOffice's answer for an unknown family *is* fontconfig's;
  Paperless was the one hard-coding a serif/Liberation list. Reading the source alone would
  have produced a confident, well-cited, wrong fix.

`A_320.doc`'s in-cell pitch was the third measurement-right-cause-wrong case: no
`sprmPDyaLine` exists in that document. The cause is `WW8Dop.fUsePrinterMetrics`, which
formats against a 300 dpi grid instead of the virtual device.

### What remains

**words, 88 failures** — 45 under, 30 over, 13 with correct pages and wrong words. The
histogram is now dominated by ±1 (28 at −1, 19 at +1), so the residue is a boundary rule
rather than a metric. Worst: `A_320.doc` 135/150, `150-5370-10H.docx` 714/721.

**slides, 28 failures** — largest is `WC_Update-Aug03.ppt` at −11.9%, whose missing table is
an *embedded Word document* (`PPT_PST_ExOleObjStg`, zlib-deflated, expanding to a complete
OLE2 file with `WordDocument` and `1Table`); LibreOffice lays it out with Writer in-process.
Rendering embedded OLE by delegating to the owning reader is a feature, not a tail fix.
`Sylva…pptx` at −9.3% is the reference being wrong, confirmed twice.

Named and unfixed: spurious wholly-blank pages (the largest over-pagination cause, four
documents, and the paginator's `!pageIsEmpty` guard already ruled out); `w:caps`/`smallCaps`
parsed and consumed by nothing; and slide autofit shrinking text to fit where we overflow
and drop the tail.

## After the fourth round: the whole corpus, measured

First measurement covering all three tracks, at `317607f2f`:

| Track | documents | match | |
|---|---|---|---|
| `slides` | 163 | **135** | 82% |
| `words` | 200 | **126** | 63% |
| `sheets` | 171 | **82** | 47% |
| **total** | **534** | **343** | **64%** |

Sheets had never been measured before this round. Its baseline was 53/174 and its first fix
took it to 82 — the largest single jump any track has had.

### Sheets fails differently from the other two, as expected

Its dominant defect was not a wrong page but a wrong *number* of them: 40 documents more
than five pages over the reference, 20 of those by more than a hundred, the worst giving
**1170 pages against LibreOffice's 220**. Three of the four causes produce **blank paper**:

- A wrapping cell widened the print area, because the overflow measurement never asked
  whether the cell wraps. A wrapping column is usually prose, so its strings measure to
  thousands of points — and every point became empty columns, every band of empty columns a
  band of blank pages.
- "Is there any cell to the left" stood in for a measurement. That was the right trade only
  while the measurement above was wrong.
- A background fill was treated as a border.

Total absolute page error is still **1120** on sheets against **141** on words, so the
blank-paper class is not exhausted. Failure rates are 28/62 for `xls` and 61/109 for `xlsx`
— near-equal, so the residue is downstream of both readers.

### Words is now a boundary problem, not a metric problem

41 of the 74 remaining failures are off by exactly ±1 page (22 at +1, 19 at −1) and total
absolute page error is 141, down from 385. Metric errors give deltas proportional to length;
these do not.

Two more properties turned out to be parsed and never applied — `w:caps`/`w:smallCaps` this
round, after `TabStop.Leader` last round. That is now a recognised shape worth grepping for
rather than waiting to trip over.

## After the fifth round: what a VM restart costs, and what it does not

The restart killed three agents mid-flight — words batch-004, slides batch-003, sheets
batch-003. Salvage, at `86ce2dc9b`:

| Track | Committed on its branch | Uncommitted | Outcome |
|---|---|---|---|
| `slides` | one fix | — | merged, verified, kept |
| `words` | nothing | 570 lines | saved as a patch, **not** merged |
| `sheets` | nothing | 673 lines | saved as a patch, **not** merged |

The slides fix — painting a PPT's shaded and picture fills instead of falling back to the
shape's `fillColor` — merged clean and holds on the merged tree: build warning-free, ten unit
projects totalling 2505 passing with Presentations up 403 → 416, fidelity 515 of 515 with 0
skipped.

**The lesson is about commit granularity, not about the restart.** An agent that commits each
fix as it lands loses nothing; an agent holding a session's work in the index loses all of it
to a process that was never asked. Both surviving patches went back out as *unverified prior
attempts* rather than as starting points, because a mid-flight diff has no measurement attached
and the one thing this project has learned repeatedly is that an unmeasured claim is usually
wrong in some particular while its measurement, if it had one, would have held.

`words/batch-003` is merged at its agent's 10/10 but has **not** been re-swept on the merged
branch, so it is recorded below as merged-pending rather than `✅`. That sweep is the gate the
words agent runs before advancing, which settles it either way.
## Slides batches 001–007: full parity, swept at `d3cba1703`

Measured, not inferred: `batch-001`–`batch-003` 29 of 29, `batch-004`–`batch-007` 39 of 39.
Every page count in the range is exact.

One document changed state in this round — `slides/batch-003/pptx/NCW-2024-Guide-.pptx`, 216
extractable words against LibreOffice's 224 before and 221 after. The rest of the range was
already passing on the base this was measured against (`86ce2dc9b`), which was checked rather
than assumed: the seven decks in batches 004–007 that state a non-zero `a:rPr/@baseline` were
re-rendered at the base commit and all seven already matched, two of them moving *closer* with
the fix (`DOE Interactive Exercise…pptx` 631→625 against 628, `flying-by-numbers…pptx` 558→560
against 560) without crossing the band. **So the ✅ on 004–007 is a re-proof of merged work, not
a claim about this change.**

### The defect: escapement read by both readers and applied by neither

`a:rPr/@baseline` and a binary PowerPoint's `PPT_CharAttr_Escapement` were each folded into a
`RunEmphasis` flag for extraction, and `SlideTextRun` had nowhere to put the magnitude — so
every ordinal in every deck sat on its baseline at full size. **29 of the track's 112 pptx decks
state a non-zero baseline on a slide.** It is the "read but never used" shape again, the third
found this way after `TabStop.Leader` and `w:caps`.

What made it a corpus failure was not the offset but the *size*: LibreOffice pairs the offset
with `DFLT_ESC_PROP` and sets the run at 58% of its em, which is 42% less advance
(`oox/source/drawingml/textcharacterproperties.cxx:196-199`). On `NCW-2024-Guide-.pptx` an
ordinal drawn full size wrapped one line of a text box already taller than the slide, and the
wrap pushed that box's last paragraph off the bottom edge. The shrink has to reach the line
breaker; applying it in the painter alone would have fixed the picture and not the text.

The percentage is of the em size on a slide and of the font's height in a word processor —
`editeng/source/items/svxfont.cxx:549-558` against `swfont.cxx` — which is why
`Layout/Escapement.cs` could not simply be reused.

## Slides batches 008–010: 27 of 30, and the remaining three are the gate's ceiling

Swept at `068b0eb44`, together with the whole gate and the other two tracks' finished batches,
because both fixes are in the PDF sink all three families share:

| Range | Before | After |
|---|---|---|
| `slides/batch-001`–`007` (the gate) | 68/68 | **68/68** |
| `slides/batch-008` | 9/10 | 9/10 |
| `slides/batch-009` | 9/10 | **10/10** |
| `slides/batch-010` | 8/10 | 8/10 |
| `words/batch-001`–`003` | — | **30/30** |
| `sheets/batch-001`–`002` | — | **20/20** |

148 documents, 145 matching, no document LibreOffice could not render. **Exactly two documents
in slides 001–010 changed state or word count at all**, both of them the intended ones, and no
page count moved anywhere. `words/batch-003` was carrying "10/10 in worktree, awaiting
merged-branch sweep"; this is that sweep.

The predecessor's baseline reproduced to the digit — 9/10, 9/10, 8/10, every page count exact,
all four failures over-counting words — which is what made the two mechanisms below worth
separating instead of treating the whole residue as one thing.

### Invisible-but-extractable text, twice, both in `PdfContentSink`

A clip hides ink; it does not remove glyphs from a content stream. Nor does an em of zero. Both
produce a page that looks right and a text layer `pdftotext` reads in full — the inverse of the
defect the word gate exists to catch, and invisible to every pixel metric. Both are also
disagreements between *our own two backends*: `SkiaDrawingSink` hands the clip to Skia, which
drops the glyphs outright, and draws nothing at zero size.

**The clip.** `slides/batch-008/pptx/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` embeds
`image8.emf`, whose header declares 720 × 250 device units and whose 2832 records draw a whole
journal page — 791 `EXTTEXTOUTW` and not one bitmap. Stretched into a 3433763 × 1190625 EMU
picture frame that `VectorImage.Draw` clips to (`Paperless.Vector/VectorImage.cs:106`), most of
that text lands outside the frame. The sink now tracks the current matrix and the active clip
beside the `q`/`Q` it already wrote and drops a run the clip excludes: **3099 extractable words
against 2108 before, 2240 after.** Every approximation is towards drawing too much — the clip is
kept as a bounding box, the run's box is inflated by an em each side and two above the baseline,
and only a run missing that enlarged box entirely goes.

**The zero em.** `slides/batch-009/pptx/NWD-GLA-Community-Outreach-Day-Oct-2025.pptx` has a
subtitle whose paragraphs carry an *absolute* top margin — LibreOffice's own flat-ODF export of
the deck gives them `fo:margin-top="0.423cm"`, twelve points, which does not shrink with the
font. Sixteen of those are 180 pt of margin in a 90.7 pt box, so **no font scale fits**, and
`SlideTextLayout`'s bisection runs to the bottom of its grid and settles on 0.1/12 of the body's
font height. There a 60 pt run rounds to 0.99 pt and a 52 pt run to nothing: the page held six
`/F1 0 Tf`. The reference reaches the same conclusion and draws no glyph there at all — its page
5 has two `BT` blocks, for the title and the "Official" marking — and across the 98 decks of
batches 001–010 it writes `0 Tf` in no document at all. **658 words against 586 before, 596
after**, so the batch is 10/10.

### The three that remain are the same measured ceiling

All three over-count, none under-counts, and in all three **LibreOffice rasterises an embedded
object and we play it as vectors**. That is settled by measurement rather than by judgement:

| Document | Ours / ref | What the reference's page holds instead |
|---|---|---|
| `batch-008 …AIRBUS-ATB-journee-CRATB.pptx` | 2240 / 2108 | a 692 × 240 JPEG with a soft mask, no text |
| `batch-010 W3_Case_Study_of_a_Tsunami…ppt` | 910 / 817 | an 845 × 572 image with a soft mask, 4 `Tf` on the page |
| `batch-010 Fundamentals_Module_1_basics.ppt` | 1146 / 1099 | the chart as a bitmap — **zero** line segments on the page |

Each puts its whole excess on one or two pages (+474/+473, +93, +50) and the rendered pages are
all but indistinguishable. Matching the numbers means emitting pixels where we emit searchable
text, which is making the output worse to satisfy the instrument. **Do not.** Progress on these
three needs a pixel metric, not `wc -w`.

What was *not* established is which LibreOffice code path rasterises them. The obvious candidate
— `MetaActionType::FLOATTRANSPARENT` with a non-constant gradient, rendered to a bitmap at up to
300 dpi (`vcl/source/pdf/pdfwriter_impl2.cxx:427-470`) — does not fit: `image8.emf` contains no
transparency record at all, and the measured resolutions are 184, 126 and 103 dpi rather than
300. Treat the rasterisation as observed behaviour, not as an explained mechanism.

One residue on that deck is neither: `+46` words on slide 16, which carries three charts with
date category axes. A separate question about axis-label density, and it would not move the
document into band even solved.

### `batch-012` 8/10 and `batch-013` 10/10, and both failures are the reference drawing less

Recorded 6/10 and 9/10; measured at `068b0eb44` they are **8/10 and 10/10**, every page count
exact. Both remaining failures over-count, and in both the reference's page is measurably the
*less* complete one — but by two different mechanisms, which is why they are written down
separately:

- `OnTrac_StarCertificationProgram-3Day.pptx` (1344 against 1045) puts +251 of its +299 on page
  10, where the reference draws the eight-week action-item table as a **644 × 542 JPEG with a
  soft mask** and we play the same object as vectors with real text. The rasterisation ceiling
  again, third instance.
- `NAS-Infrastructure-Roadmaps-v16.0.pptx` (19219 against 15316, 137 pages exact) is a different
  thing and the more interesting one. Twenty-odd of its pages carry four `Excel.Sheet.12` OLE
  objects apiece, **linked rather than embedded** — `<p:oleObj r:id="…"><p:link/>` against a
  SharePoint URL — each with an `mc:Fallback` holding a `<p:pic>` whose blip is a cached
  `image*.emf` sitting in the package. We draw those replacement pictures; the reference draws
  nothing at all, so its page 4 is a title and a footer, 13 words against our 226, with **no
  images and no vector content on it**. That is measured rather than judged: the fallback
  picture demonstrably exists in the file and demonstrably is not in the reference PDF.

  What was *not* established is why. The import path is `OleObjectGraphicDataContext`
  (`oox/source/drawingml/graphicshapecontext.cxx:222-285`), which takes the `mc:Choice` branch
  when the object carries an `spid` — this one does, `_x0000_s1026` — and expects the VML
  drawing to supply the replacement, falling back to `mc:Fallback` only through
  `setMCEState(MCE_STATE::Started)` in `onEndElement`. The deck ships no `vmlDrawing` part.
  That reading is from a 27.2-alpha checkout and the reference came from 24.2.7.2, so treat it
  as a lead, not a conclusion.

  Either way the direction is settled: matching the count means blanking twenty pages of a
  137-page deck. Do not.

### `batch-014` 7/10 and `batch-015` 8/10, and the next real defect is in SmartArt

Measured at the same commit, both matching their recorded figures, every page count exact in
all five failures. Three over-count (`Thailand17.ppt` 2813/2697, `N2_E_Maestroni_Swarm_COP.pptx`
5422/5217, `WiGr_2021W_1_Angebot-Nachfrage-Elastizität…pptx` 2209/1988) and were not taken
apart.

**The other two under-count, which is the direction worth chasing**, and they are near-duplicate
decks: `schematicplay.pptx` 2040 against 2129 and `schematicplaymar21.pptx` 2034 against 2123.
Both put the whole deficit on one page — page 7, −86 — and both were measured **with the two
sink guards disabled**, giving 2040 and 2034 unchanged, so this is not the clip fix. It predates
it.

The page is a SmartArt diagram (`ppt/diagrams/drawing1.xml`; the text appears in no slide part
at all). The shapes come out right — four green chevrons and four rounded bars, in the right
places, indistinguishable from the reference — and **every piece of text in them is drawn
rotated 90° and stacked into a single vertical band down the middle of the slide**, where the
overlapping glyphs collapse under extraction. It is one of the most visible defects seen on this
track and it is invisible to a page-count check.

The suspect, stated as a suspect: all eight `dsp:sp` in that drawing carry
`<a:xfrm rot="5400000">` while their `<a:bodyPr>` says `vert="horz"` and states no `upright`.
The reference draws neither the shapes nor the text rotated. So either the `dsp` drawing's
`rot` is not meant to reach the text at all, or its extent is already stated post-rotation and
we are applying the turn a second time. **Not verified** — nobody has read our `dsp` transform
path against LibreOffice's, and on this project the sentence after a measurement has been wrong
more often than not. The measurement is the rotated band; the explanation above is a lead.

### `batch-011` was already 10/10, and the recorded 7/10 was stale

Swept at the same commit, unchanged by either fix: **10 of 10, every page count exact.** The
7/10 in the table came from a much older base and had survived the merge of the escapement work
and everything alongside it. Worth stating as a habit rather than as a fact about this batch:
**a recorded number describes the commit it was taken at, and after a large merge the cheapest
useful act is to re-measure the next batch before opening it.** Three batches of apparent work
here did not exist.

## Slides 008–017: the whole track measured, and the SmartArt lead was real

Baseline at `db529cfb2`, all 163 documents in one run: **149 match, 14 do not, none the
reference could not render, and every page count in the track exact.** Every one of the 14 is a
word-count difference alone. `batch-015` closed this round; the other 12 failures are now each
attributed, and none of them is a small fix.

Re-swept whole at `3e149fcf6` after the one fix this round: **151 match.**

| Batch | recorded | baseline | after |
|---|---|---|---|
| `batch-001`–`007` (the gate) | ✅ | **68/68** | 68/68 |
| `batch-008` | 9/10 | 9/10 | 9/10 |
| `batch-009` | ✅ | 10/10 | 10/10 |
| `batch-010` | 8/10 | 8/10 | 8/10 |
| `batch-011` | ✅ | 10/10 | 10/10 |
| `batch-012` | 8/10 | 8/10 | 8/10 |
| `batch-013` | ✅ | 10/10 | 10/10 |
| `batch-014` | 7/10 | 7/10 | 7/10 |
| `batch-015` | 8/10 | 8/10 | **10/10** |
| `batch-016` | 7/10 | 7/10 | 7/10 |
| `batch-017` | 3/5 | **4/5** | 4/5 |
| **whole track** | | 149/163 | **151/163** |

The briefed numbers reproduced to the digit everywhere except `batch-017`, whose recorded 3/5 was
stale. **Exactly two rows of the 163 changed between the two sweeps**, both of them the intended
ones, and no page count moved anywhere in the track.

The baseline was also run a second time on the same binary and came back **byte-identical on all
163 rows**, which is worth recording once: the instrument is deterministic, so a difference
between two sweeps is a difference in the program and never in the weather.

### The lead was right about the symptom and wrong about the cause, as usual

`schematicplay.pptx` and `schematicplaymar21.pptx` were handed over as "−86 words on page 7, a
SmartArt diagram whose text is all rotated 90° into a vertical band", with the suspicion that
`<a:xfrm rot="5400000">` on the eight `dsp:sp` should not reach the text at all. The symptom
reproduced exactly. The suspicion is wrong in the way that matters: the shape's rotation *does*
reach its text, and what nothing read was the **`rot` on the same shape's `dsp:txXfrm`**, which
every one of those eight carries and which states `-5400000` — an angle *against* the shape's
rather than a copy of it.

The two add. `Transform2DContext` puts `dsp:txXfrm/@rot` into the same field `a:bodyPr/@rot`
feeds, adding rather than replacing
(`oox/source/drawingml/transform2dcontext.cxx:53-58`), and reads the sum back as "the rotation
beyond compensation of the shape rotation" (`transform2dcontext.cxx:341-344`). So a chevron laid
on its side by its own `a:xfrm` and compensated by its text area keeps its writing horizontal.

**It turns the laid-out box and does not transpose it**, which is the second thing the fix had to
get right and the one that would have been easy to get wrong: LibreOffice scales the text box to
the stated width and height *first* and rotates the result about its centre
(`svx/source/sdr/contact/viewcontactofsdrobjcustomshape.cxx:168-191`), so the lines still break
at the width the file states. Only `TextPreRotateAngle` — what a diagram's `upr` and `grav`
produce — is applied before the scale and so reshapes the box. Transposing would have broken
"Sensorimotor" at 32 pt instead of 75 and overflowed every chevron.

Measured on the whole-track re-sweep: **2040 → 2114 against 2129** and **2034 → 2108 against
2123**, both in band, page counts unmoved at 21/21, and `batch-015` 8/10 → **10/10**. Reach over
the slides corpus: 15 of the 112 pptx decks bake a diagram drawing, 13 of those carry a
`dsp:txXfrm` across 171 shapes, and **3 state a non-zero `rot` on one** — 18 shapes, every one a
quarter turn against a shape turned the opposite quarter. The third,
`batch-006/Course Selection 2025-26 Current Grade 09.pptx`, states it on two `rightArrow`
connectors that carry no text, which is why a batch at full parity was not hiding the same
defect.

A worked comparison is now in the corpus: `slide-diagram-text-turned.pptx` puts the two cases
side by side, and the reference's rendering of it and ours **differ on no pixel at all** —
`pdf-image-diff.py` reports 0.00%. LibreOffice supports `rect` in
`ConstructPresetTextRectangle`, so both honour the same rectangle. On `schematicplay.pptx` itself
it does not: `chevron` is not one of that function's fourteen presets, so LibreOffice keeps the
rotation and *drops the text rectangle*, and its own page 7 wraps the labels as
"Stag e 1: / Sens orimo tor". Ours is the better output there and scores 240 words against its
252, which is where the remaining difference on that page comes from.

### The residue is 13 documents and four named mechanisms, three of them ceilings

Every remaining failure is now attributed. Nothing in the list is a small fix.

**1. LibreOffice rasterises an embedded object where we emit searchable text — 8 documents.**
The recorded ceiling, re-verified and extended. The signature is exact: the reference's page holds
an image *with a soft mask* that ours does not, at 66–265 dpi, and our page holds the same
content as vectors and real glyph runs.

| Document | ours / ref | worst page | what the reference holds there |
|---|---|---|---|
| `008 …AIRBUS-ATB-journee-CRATB.pptx` | 2240 / 2108 | — | a 692 × 240 JPEG with a soft mask |
| `010 W3_Case_Study_of_a_Tsunami….ppt` | 910 / 817 | 10, +93 | an 845 × 572 image with a soft mask |
| `010 Fundamentals_Module_1_basics.ppt` | 1146 / 1099 | 6, +50 | a 529 × 355 image with a soft mask |
| `012 OnTrac_StarCertificationProgram-3Day.pptx` | 1344 / 1045 | 10, +251 | a 644 × 542 JPEG with a soft mask |
| `012 NAS-Infrastructure-Roadmaps-v16.0.pptx` | 19219 / 15316 | 4, +213 | nothing: no image and no vector content |
| `014 Thailand17.ppt` | 2813 / 2697 | 8, +93 | **the same 845 × 572 image** as `W3_Case_Study` |
| `014 N2_E_Maestroni_Swarm_COP.pptx` | 5422 / 5217 | 7, +205 | 113 images against our 5 |
| `016 16 - UTM - (NASA).pptx` | 2459 / 2261 | 29, +103 | a 640 × 480 JPEG with a soft mask |

Two things were established this round that were open before.

- **The raster is not in the file.** Both `.ppt` documents were scanned for a stored replacement
  picture of the reference's exact pixel size — through every zlib stream in the file, not only
  the raw bytes, because Escher blips and `PPT_PST_ExOleObjStg` are deflated. Neither holds one.
  So LibreOffice really does produce the bitmap rather than draw one the author supplied, and
  "our reader is drawing live content where LibreOffice draws a stored thumbnail" is refuted.
- **It is not the PDF writer either.** `implWriteBitmapEx` downsamples only when
  `ReduceImageResolution` is on, which is not the default, and the 300 dpi
  `MetaActionType::FLOATTRANSPARENT` branch (`vcl/source/pdf/pdfwriter_impl2.cxx:452-470`) does
  not fit resolutions of 66, 103, 126 and 265. The bitmap therefore enters `playMetafile`
  **already rasterised**, so whatever does it is upstream of PDF export, in the metafile the
  page hands the writer. That narrows it; it does not name it.

Matching any of these means emitting pixels where we emit text. Do not.

**2. `mc:Choice Requires="a14"`, and this one is finally named — 1 document.**
`014 WiGr_2021W_1_Angebot-Nachfrage-Elastizität….pptx` (2209 / 1988) had never been taken apart.
Twenty of its slides wrap their whole body in an `mc:AlternateContent` whose `mc:Choice` requires
`a14` — DrawingML's 2010 extensions, used here for inline formulas — and whose `mc:Fallback` is
one shape with an `a:blipFill` picture of that same text and a text body of a single space.
PowerPoint takes the Choice and so do we; **LibreOffice takes the Fallback**, because its
supported-namespace list has `a14` commented out with the reason attached —
*"We do not currently support inline formulas and other a14 stuff"*,
`oox/source/core/contexthandler2.cxx:243-249`. Its page 28 is therefore a title, a page number
and a picture: 5 words against our 53.

Measured reach: `Requires` appears in the slides corpus as `p14` 381 times, `v` 45 and `a14`
**22**, the last across only 3 decks. The other two state it on one slide each and both pass the
gate. Matching means drawing a picture of text instead of the text. Do not.

**3. Drop shadows are not drawn at all — 1 document, and the widest-reaching lead on this track.**
`016 pres_ioc_phuket.ppt` **under**-counts, 974 against 1005, with −22 of it on page 5. The
reference's page 5 draws "National" 14 times and ours 7 — and the reference's 14 are *seven
pairs*, each pair 6.01 pt apart in **both** axes, read straight out of `pdftotext -bbox`. Ours are
the odd members of those pairs to within 0.02 pt. So LibreOffice is drawing each shape's shadow,
text and all, and we draw no shadow at all; the rendered pages show it plainly, black offset
shadows behind every coin, box and arrow that we leave flat.

This is a feature rather than a fix — offset, colour, transparency and blur, applied to fill,
outline *and* text — but it is the one lead here with real breadth: **59 of the 112 pptx decks in
this track state an `a:outerShdw` or `a:innerShdw` on a slide shape, 3296 occurrences between
them**, and nothing in `Paperless.Presentations` reads any of them. Its effect on the word gate
is a side effect; its effect on the page is visible at a glance.

**4. Two chart and text-effect gaps — 2 documents.**

- `016 FAAAIandtheArtandScienceofV&Vfinal.pptx` (1201 / 1145) puts +28 on each of pages 13 and 14,
  and it is not extra text: the same four labels, fragmented. Each is a `TextBox` with
  `<a:prstTxWarp prst="textArchUp"/>` and a 45° `a:xfrm rot`, and `prstTxWarp` appears nowhere in
  `dotnet/src`. The reference bends "Automation Autonomy", "Analysis", "Augmentation" and
  "Assistance" around the dial; we lay them straight, they wrap and collide in the middle of the
  slide, and `wc -w` counts the wreckage as more words.
- `017 Demick_JetBlue.pptx` (713 / 617) is the axis-label-density question the last round named
  and did not chase. Its charts have a two-level date category axis; **the reference draws none of
  those labels and we draw every one**, split by `pdftotext` into `20` + `06`. LibreOffice
  suppresses labels that will not fit — it increments a "rhythm" and redraws every *n*-th label
  until they stop overlapping, then staggers, then gives up
  (`chart2/source/view/axes/VCartesianAxis.cxx:810-860`), and caps it again by repeated-label
  count in `estimateMaximumAutoLabelWidth` (`VCartesianAxis.cxx:1595-1620`). That mechanism is
  read out of the source and **not** verified against the running binary on this document; the
  measurement is that the reference draws no category labels there and we draw all of them.

### A defect the word gate cannot see, found while doing the above

`016 16 - UTM - (NASA).pptx` is the only one of the 163 whose PDF poppler complains about, and it
does so 161 times: *"Mismatch between font type and embedded font file"* and then *"No font in
show"* for every text operation using `/F10`. `pdffonts` reports that font — `JAAAAA+Unifont` — as
embedded, and the gate's third check passes.

The bytes settle it. The stream we write for it begins `OTTO` and its table directory is
`CFF `, `OS/2`, `cmap`, `head`, `hhea`, `hmtx`, `maxp`, `name`, `post` — **a CFF-flavoured
OpenType face, embedded as `/FontFile2` under `/Subtype/TrueType`**
(`Paperless.Rendering/Pdf/PdfFontCatalogue.cs:311`, which writes that unconditionally). PDF 1.7
§9.9 wants `/FontFile3` with `/Subtype/OpenType` for those, and a reader that follows it drops
the font, so 161 glyph runs draw nothing while extracting perfectly. It reaches one document here
only because Unifont is a last-resort fallback, but it would reach **any** `.otf` face on any
machine that has one installed.

Left unfixed deliberately: `PdfFontCatalogue` is shared by all three families and
`PdfFontEmbeddingTests` asserts `/FontFile2` on every descriptor it walks, so the change needs
that test taught the OpenType case and a sweep of all three tracks — which is more than this
round had left. It is small and it is precisely located.

### A sweep against a stale binary looks exactly like a sweep that found nothing

Worth adding to the skill's list because it cost a whole slides sweep here and the failure is
silent. The snapshot rule — copy the CLI so a rebuild cannot race the sweep — is necessary and
not sufficient: the copy was taken while `tools/Paperless.Cli/bin` still held the binary from a
*deliberately reverted* build made to prove the new test could fail, and the sweep then measured
the base commit again. Every check in the skill passed: 163 rows, no duplicates, no `ref-failed`,
every page count exact. The tell was that the two documents the fix was written for had not moved
— which is only a tell if you already know what they should read.

Two lines fix it, and `scratchpad/snapshot-cli.sh` now does both: **`md5sum` each of the copied
assemblies against the built tree**, and **render one document whose number the fix is known to
change** before starting. Under a shared machine the whole track takes over an hour, so a wasted
sweep is the most expensive mistake available.

## Sheets batches 001–008: the row height a file only guessed at, swept at `42253c784`

Whole track swept before and after, 171 documents each time.

| | before | after |
| --- | --- | --- |
| documents matching | 113 | **122** |
| documents with an exactly correct page count | 123 | **134** |
| total absolute page error | 843 | **222** |

Eleven documents gained parity and two lost it, both of the latter by one page on a very large
workbook — `FY2023-AIP-grants.xlsx` at 32 against 33 and `ans_mappings_of_eccairs_terms.xlsx` at
193 against 191. Batches 001–004 are 40/40 again, so the gate holds.

The recorded scoreboard was stale low almost everywhere: the baseline sweep measured 005 at 7/10
against a recorded 4, 006 at 7 against 4, 008 at 7 against 5, and eleven of the eighteen batches
one or more above what was written down. Re-measure before believing any of it.

### `ht` without `customHeight` is a hint, and it was the largest lead on the track

The mechanism was in the handover and held exactly. What the handover did not have — and what
made it shippable — is that **the common case measures nothing at all.**
`ScColumn::GetOptimalHeight` (`sc/source/core/data/column2.cxx:898`) asks two questions of every
cell. All of them contribute `lcl_GetAttribHeight` (`column2.cxx:866`), which is arithmetic on the
font's *size*: `trunc(sizeTwips × 1.18) + margins − 23`, floored at the sheet's minimum. Only a
cell that wraps, rotates, stacks or holds a second line also goes through `GetNeededSize`.

The wrapped case turned out to be reproducible too, and the previous round's "5.8% too large"
finding was right about the number and wrong about the conclusion. `GetNeededSize`'s EditEngine
branch quantises to whole device pixels three times — the em size, the ascent, the descent — adds
two truncated one-pixel margins, and divides by `ScSizeDeviceProvider`'s **0.067, not 1/15**
(`sc/source/ui/docshell/sizedev.cxx:48`). That is `Paperless.Text.Fonts.MetricGrid`, which Writer
already had for `fUsePrinterMetrics`. Fitted to thirty probe rows — six font sizes against five
line counts, read out of LibreOffice's own flat-ODF export — it reproduces all thirty.

**A `.xls` written this century is never re-measured at all**, and missing that cost nineteen
documents their page count on the sweep that found it. `ImportExcel8::Read` has its
`AdjustRowHeight()` call `#if 0`-ed out with the reason beside it — "Excel documents look much
better without this call; better in the sense that the row heights are identical to the original
heights in Excel" (`sc/source/filter/excel/read.cxx:1282-1288`). BIFF2–5 still call it
(`read.cxx:780`). Separately, a BIFF sheet can mark every one of its rows manual through
`DEFAULTROWHEIGHT`'s own `fUnsynced` (`colrowst.cxx:212-215`).

### Both unverified claims were wrong in their explanation and right in their measurement

- `Background_Declaration_Template.xls` was recorded as SpreadsheetML's column-band duplication
  arriving in the BIFF path. LibreOffice draws the text **zero** times, not once: it is a cell
  comment, `ftCmo` type 25, whose importer calls `SetInsertSdrObj(false)` and makes a `ScPostIt`
  of it (`sc/source/filter/excel/xiescher.cxx:1852-1883`). 201 words against 181, and 181 now.
- `Foreign_SA`'s residue was recorded as the same box drawn on each column band. It is the
  **page-edge clip**: LibreOffice cuts the notes block mid-word at the band boundary — its own
  text layer holds `Ai` and `r` as separate tokens — and picks the tail up on the next band. We
  draw each line whole on the first band and none of it on the second, 35 words up and 9 down.
  Implementing it needs a clip that drops the glyph runs it excludes, which is what
  `PdfContentSink` is being taught elsewhere.

### A lead for whoever takes 009: a page that draws five words where the reference draws 507

`RegChangeReport.xlsx` now paginates correctly — 12 pages against 12, up from 14 — and is still
2302 extractable words against 3137. The deficit is **almost all on one page**: per-page counts run
378/378, **5/507**, 295/436, 250/345, 422/437 and so on down. Our page 2 holds nothing but the
sheet's `Security Classification: Protected A` band; the reference's holds the whole
"Liability Management Framework" row, whose Description cell is several hundred words of wrapped
text in a row LibreOffice computes at 6480 twips.

The reference's page 3 then *begins mid-sentence* — "immediately. On August 19, 2025, Directive 011
was" — where ours begins at the next whole row. **The mechanism is not established**, and the
obvious explanation does not survive a look at the source: `ScTable::UpdatePageBreaks`
(`sc/source/core/data/table5.cxx:206-240`) never splits a row, it gives an over-tall one a page of
its own. Worth starting from the page-2 measurement rather than from that theory.

### What is left in 005–008

`esurf-12-135-2024-t01.xlsx` draws four-digit years as `201` and `202` — a number clipped to its
column rather than overflowing or becoming `###`, which is the two-way coupling between the
`General` format and the column width. `Hazard Analysis Template.xls` is 2 pages against 3 and 461
words against 682. `Published_Issuances_2024.xlsx`, `SSRO_…_DATA.xlsx` and
`RVSM_Non_approved_list_2025_….xlsx` are word-count failures with correct page counts, two of them
short and one long.

## After the sixth round: words batch-004, and what the salvaged patch was worth

Swept 001–005 together at each step, 50 documents, three workers:

| | baseline | + kerning | + text frames | + table alignment |
|---|---|---|---|---|
| `batch-001` | 10/10 | 10/10 | 10/10 | 10/10 |
| `batch-002` | 10/10 | 10/10 | 10/10 | 10/10 |
| `batch-003` | 10/10 | 10/10 | 10/10 | 10/10 |
| `batch-004` | 9/10 | 9/10 | **10/10** | 10/10 |
| `batch-005` | 7/10 | 7/10 | 7/10 | **8/10** |

The baseline reproduced the briefed 9/10 and 7/10 exactly, and settles `batch-003`,
which was merged at its agent's figure and had never been swept on the merged branch:
it holds.

**batch-004's single failure was one defect and it was not a small one.** Word has two
unrelated ways of positioning a block of text and this tree knew only one. A *text box*
is an Escher shape with an `FSPA`; a *text frame* — WW8's APO — is a run of ordinary body
paragraphs carrying `sprmPPc` and friends, delimited by nothing but those sprms ceasing to
match. Laying them out in the flow cost `07-04.doc` 90 pt at the top of page 1 and spilled
its last three lines onto a fourth page. **28 of the 66 `.doc` files in this track import
with at least one**, so the reach is much wider than the one document it was found on.

### The kerning change is neutral, and that is the finding

Measured on its own, before the text-frame work: **46/50 before and 46/50 after, the same
four documents failing with the same page and word counts to the digit.** It is in anyway,
on its own evidence — the formats state the property, the fidelity suite is 515/515 with
it, and every reader was previously kerning text the reference does not — but nothing on
this corpus can see it, and a later agent should not expect it to have moved anything.

Two things the salvaged patch had wrong, both caught by measuring rather than reading:

- **`GroupState.Clone()` did not carry the flag**, so every RTF nested group lost its
  paragraph's kerning. One unit test, on the fixture written for a different bug.
- **ODF's default is kerning *on*, not off.** The patch reasoned from
  `SwDocShell::Load` → `RemoveAllFormatLanguageDependencies`, which resets the default to
  the pool's `false`, and that reading is what the source says. It cost fourteen fidelity
  comparisons, every one of them `.fodt`. Rendering one flat-ODF document three times
  through the installed `soffice` settled it in a minute: with no attribute the output is
  byte-identical to `style:letter-kerning="true"` and differs from `"false"`. The other
  half of the rule is in `SwDocShell::InitNew`, which sets the document default *true*
  (`docshini.cxx:304`) where the three Microsoft importers each state `false` for
  themselves.

So the salvaged 570 lines were worth roughly what the skill says a mid-flight diff is
worth: the shape of the change survived, one of its two central claims did not, and the
measurement it never had is the only thing that could have told them apart.

### A table's alignment was never read, in either of the two ways a DOCX states it

`batch-005`'s text-loss document was 602 words against 636 with its pagination already
right, and the missing 34 were **30 cells containing the single letter C**. The table is
11022 twips across a 9070-twip text area and centred; placed at the indent instead, its
last four columns started past the page edge and their ink was clipped away. Text that
neither draws nor extracts, with no other symptom — the parity gate could only see a word
count.

Neither attribute was read. `w:tblPr/w:jc` — a different element from the paragraph
alignment of the same name — is stated by **31 of the track's 134 DOCX files, 315 of whose
320 occurrences say `center`**; `w:tblpPr/@tblpXSpec` is stated by 21 more. Only `center`
and `right` move anything: `convertTableJustification` leaves everything else on
`LEFT_AND_WIDTH`, which is the indent. Despite touching a quarter of the track's DOCX
files it cost nothing anywhere — batches 001–004 held at 10/10 and 005 went 7 to 8.

Alignment cannot be folded into the indent at read time, because the answer needs the width
of the area the table sits in, which the reader has not got. `PageTable.LeftWithin` resolves
it at the two places that already know it.

### The largest named-and-unfixed lead: Word 2013's justification

`BID_ACKNOWLEDGEMENT_FORM_FOR_A320.docx` paginates 3 against 2 and the cause is neither a
metric nor a break rule. Every word on the first page is within 0.1 pt of the reference's
width, and the lines still break a word early — because **LibreOffice compresses the spaces
and we only stretch them**. Measured on one justified line: sixteen words summing to 417.63
pt in a 468.0 pt column, our fifteen gaps at 3.358 pt each and the reference's sixteen at
1.894 pt, against a natural Carlito space of 2.26 pt. The reference squeezed a
seventeenth word in by shrinking every space **below** its natural width.

That is `JUSTIFY_LINES_WITH_SHRINKING` (tdf#158776, `sw/source/core/text/portxt.cxx:545`),
and writerfilter turns it on for every DOCX whose `compatibilityMode` is 15 or more —
`sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:10172`, whose comment states the
consequence plainly: *"new paragraph justification has been introduced in version 15,
breaking text layout interoperability: new line shrinking needs less space i.e. it typesets
the same text with less lines and pages."* This document declares mode 15 and holds 44
justified paragraphs.

**Expect this to be worth a lot.** It is a whole-corpus effect on a class the residue is
already known to be made of — documents off by ±1 page with the right words — it applies to
every modern DOCX rather than to a feature some documents use, and its sign is always the
same: we produce more lines than the reference, so more pages. `DocxLayoutSource` already
reads `_compatibilityMode`. What it needs is a line breaker that can retry a full line with
the blanks compressed, which is a change in `Paperless.Text.Layout` and was too large to
start with the time left rather than too hard.

The other `batch-005` failure, `loi_format_letter_of_intent…doc`, is the opposite sign — 9
pages against 10, diverging only from page 5 — so it is *not* this, and it has no diagnosis.

## The sixth round, swept whole at `db529cfb2`: 175 of 178

Three tracks merged in one round, and one of the three changed `PdfContentSink` — a sink all
three families share. Each agent's own cross-track sweep ran on a base predating the other two
merges, so none of them described the tree that actually exists. This is that sweep:

| Track | Batches | | |
|---|---|---|---|
| `words` | 001–005 | **48/50** | `BID_ACKNOWLEDGEMENT…docx` 3 pages vs 2, `loi_format…doc` 9 vs 10 |
| `slides` | 001–009 | **87/88** | `8_P-Pavese_AIRBUS…pptx` 2240 words vs 2108 |
| `sheets` | 001–004 | **40/40** | — |
| **total** | | **175/178** | |

**Every figure is exactly what the three agents reported separately.** That is the result worth
recording: a change to the shared sink, landing in the same round as text frames, table
alignment, kerning and four spreadsheet page-boundary fixes, moved nothing outside its own
track. It is also the first round where the merged total did *not* beat the sum of its parts —
which is what you would expect once the tracks stop sharing root causes, and is the signal that
the remaining defects are per-family rather than in the layers underneath.

The three residual failures are each already named: two with a diagnosis (`compatibilityMode`
space-shrinking; an undiagnosed opposite-sign break at page 5) and one that is the gate's
ceiling rather than a defect.
## After the seventh round: words, and what Word 2013's justification was actually worth

The whole 200-document words track swept at `db529cfb2` and again with the shrinking
justification landed. **139/200 before, 142/200 after**; total absolute page error 124 → 122;
documents with an exactly correct page count 150 → 153.

The baseline reproduced the briefed figures for `batch-005` (8/10), `batch-006` (9/10) and
`batch-007` (8/10) exactly, and corrected two records in the *pessimistic* direction:
**`batch-008` and `batch-009` are 10/10, not the 9/10 recorded.** One baseline row was a
`SIGBUS` in our own CLI — `FMRBullletinB-28.doc` produced no PDF at all — which re-ran
cleanly as 2/2 pages and 460/461 words; a crash and a mismatch look identical in a sweep, so
re-run a zero-page row before believing it.

### Shrinking justification: landed, neutral-or-better, and much narrower than expected

The lead was right about the mechanism and wrong about the reach. `JUSTIFY_LINES_WITH_SHRINKING`
is implemented — a justified line may overrun its column by a quarter of what its blanks are
worth, and the blanks are then squeezed back (`JustificationShrink`, `TextLine.ShrinkAllowance`).
`BID_ACKNOWLEDGEMENT_FORM_FOR_A320.docx` goes 3 pages to 2 against the reference's 2.

What it moved across the track:

| | before | after |
|---|---|---|
| whole track | 139/200 | **142/200** |
| `batch-005` | 8/10 | **9/10** |
| `batch-012` | 7/10 | **8/10** |
| every other batch | — | unchanged |

**No document regressed, in verdict, page count or word count.** Batches 001–004 held 10/10.

The reach, measured rather than assumed: **91 of the track's 134 DOCX declare
`compatibilityMode` 15 or more, and 39 of those also state `w:jc` of `both` or `distribute`
in `document.xml`** — yet only **9 documents' output changed at all**, and only two of those
were failing for it. The rule only moves a break when a line overruns by *less than* a quarter
of its blanks, which is a narrow band; on a 468 pt column with sixteen blanks that is about
nine points. So this is a real feature that applies to nearly half the track and tips very few
documents, rather than the whole-corpus effect the previous round predicted. Predicting reach
from how many files *state* a setting overestimated it by an order of magnitude.

Two documents also moved to an exactly matching word count (`4400-91_Proposal_To_Lease_Space`,
`Company-profile-2022-EN`) and one moved eight words further away while staying inside the 2%
band (`PES-Technical-Report-Template_Jan_2019`, 2682 → 2690 against 2679).

Unverified, and marked so: whether LibreOffice 24.2.7.2 — the binary the references come from
— squeezes a paragraph's *last* line. The tree's own source does (`SwTextAdjuster::FormatBlock`,
"if the last line is longer than the paragraph width, it contains shrinking spaces: don't skip
block format here", which is tdf#162725 and may post-date 24.2). We squeeze it, because the
feature needs it — pulling a paragraph's final words up is how it saves a line — and because
the alternative differs only in whether that one line overflows the margin visually, not in
where any break falls. Nothing in the gate can see the difference.

### `batch-005`'s remaining failure, diagnosed and not fixed

`loi_format_letter_of_intent-a-320-214-a330.doc` is 9 pages against 10 and the cause is *not*
the page it diverges on. Page 4 ends in ten consecutive empty paragraphs, and the reference's
soft page break falls after the eighth where ours falls after the ninth — because by the foot
of page 4 the reference has accumulated **3.8 pt more than we have**, all of it inside one
three-item bulleted list. Measured line pitches through that list: reference 13.80, 12.90,
13.55, 12.90, 12.65, 12.65 pt against our uniform 12.65.

13.80 pt is Liberation Serif's line box (2355/2048) at **12 pt**, where the item's text is
11 pt — so the reference's line is tall because its *list label* is bigger than the paragraph,
and ours is not because **the label contributes nothing to the line's height and is measured at
the paragraph's own size**. `DocReader.Label` passes `paragraph.Size`
(`Ww8/DocReader.cs:474`) and `Ww8Numbering` steps over the level's `grpprlChpx` outright
(`Ww8Numbering.cs:377`, "the character grpprl is still only stepped over"); the label is not a
run, so `MeasuredParagraph.HeightOf` never sees it. Writer's is `SwNumberPortion`, an ordinary
portion that `SwLineLayout::CalcLine` accumulates like any other.

Two changes, then: read the level's character size, and let the label raise its line. Both are
features rather than wirings and both would touch every list in the corpus, which is why they
were not started with the time left. The round-trip test does *not* localise it — the flat-ODF
export renders 11 pages against the reference's 10, diverging the other way, so the `.fodt`
path has separate defects and exonerates nothing.

### `batch-006` and `batch-007`, diagnosed and not fixed

- `f445896eb008d14c1746fc37d412dc22.docx` (006), 15 pages against 16, word counts identical at
  5575. Diverges from page 3. Our page 2 stops at y = 536 pt with a third of the page empty
  while the reference fills it to y = 740 — a table row we move whole to the next page and the
  reference splits.
- `Press release_EUREKA labels ITEA 3 Cluster.docx` (007), 798 words against 823. Two defects.
  The letterhead in `header2.xml` is a `wpg:wgp` **group of nineteen shapes, eighteen of them
  text boxes**, and `DocxFrames.Read` takes `Descendant(placed, "txbxContent")` — the first one
  only, so seventeen boxes' worth of address block never draws (≈39 words). Separately, page 2
  takes section 2's default header where LibreOffice takes section 1's: the file has three
  `w:sectPr`, all with `w:titlePg`, and the last states only an `even` reference.
- `final-technical-report-template.docx` (007), 1108 words against 1135. The header
  "Final Technical Report [Recipient Organization Name] [Award Number]" is drawn by the
  reference on pages 2–6 and by us on pages 1 and 3 only — the same header-slot selection
  defect. Two smaller terms on top: we draw seven list bullets as ASCII `•` where the reference
  draws a symbol-font private-use code point `wc -w` cannot see, and `PAGE`/`NUMPAGES` fields
  print stale cached results.

The `PAGE` field is worth calling out on its own because it is not document-specific:
`FieldInstructions` maps `"PAGE"` to `WritingFieldKind.PageNumber` and **nothing consumes it**
— the read-but-never-used shape again. On `loi_format_letter_of_intent…doc` every one of the
nine footers prints `9`, which is the result Word last cached, against the reference's 1…9.

## The seventh round, swept whole at `e2e0bdee3`: 185 of 188

| Track | Batches | | |
|---|---|---|---|
| `words` | 001–005 | **49/50** | `loi_format…doc` 9 pages vs 10 |
| `slides` | 001–009 | **87/88** | `8_P-Pavese_AIRBUS…pptx` — the ceiling |
| `sheets` | 001–005 | **49/50** | `esurf-12-135-2024-t01.xlsx` 113 words vs 124 |
| **total** | | **185/188** | |

Again exactly the three agents' own figures, so the sum-equals-parts reading from the sixth
round holds for a second round rather than being a one-off. Whole-track figures at this commit:
words **142/200** (page error 122), slides **151/163**, sheets **122/171** (page error 222).

Sheets is the round's result: page error **843 → 222**, the largest single move any track has
had, and it came from three instances of one mistake — believing a number a producer wrote
instead of measuring. `ht` without `customHeight` is a hint Calc recomputes; a BIFF8 `.xls` is
never re-measured at all, because `ImportExcel8::Read` has its `AdjustRowHeight()` call
`#if 0`-ed out beside a comment asserting the heights are already Excel's
(`sc/source/filter/excel/read.cxx:1282-1288`); and a cell comment is not a shape.

### The three tracks now fail in three different ways

Worth stating because it changes how each should be worked:

- **`sheets`** is still a *quantity* problem — 222 pages of error across 49 failing documents.
  Systematic causes remain, and finding one is worth more than fixing ten documents.
- **`words`** is a *boundary* problem — page error 122 over 58 failures, most of them ±1 page.
  The causes are individual and each is worth about one document.
- **`slides`** has largely stopped being a word-count problem at all: 151 of 163, and **all
  twelve remaining failures are attributed**, eight of them to the word gate's ceiling rather
  than to a defect. That track's next real work is judged by the image check, not by `wc`.
## Sheets: what the rich-cell row-height fix actually did — swept at `52f45c51b`

The eleventh round merged **"Measure a rich cell's row instead of believing the writer's guess"**
on its unit tests and one fixture; the agent that wrote it was killed before it could sweep. All
171 documents swept here at `52f45c51b`, against the last whole-track figure at `18b4547ef`:

| | `18b4547ef` | `52f45c51b` | |
| --- | --- | --- | --- |
| documents matching | 125 | **124** | −1 |
| documents with an exactly correct page count | 134 | **133** | −1 |
| total absolute page error | 222 | **195** | **−27** |

**It works, it is the largest page-error move on this track since the seventh round, and it
overshot.** `TK-Syllabus-Comparison-Document-v2.xlsx` was 1314 pages against 1235 and is now
**1190** — the error on the document it was aimed at went 79 → 45 and changed sign, so the rows it
recomputes now come out *short* where they used to come out long. Its sibling
`tk-syllabus-comparison-document-v5.xlsx` is 846 against 855, the same sign. The −27 is that −34
less about seven pages picked up elsewhere.

This is the shape the skill's "fixes that cancel" section describes and the reason to report both
numbers: a match count that falls by one while the continuous quantity falls by 12% is a change
that is right on its own evidence.

### The residue, sorted

Total page error is **195** over 47 failing documents, and the top of the list is where it lives:

| Pages | Document | What is established |
| --- | --- | --- |
| 45 | `TK-Syllabus-Comparison-Document-v2.xlsx` | the rich-cell measurement, now overshooting |
| 42 | `orbus_togaf_tool_csq.xls` | **not ours** — the reference fabricates a `DPCache` sheet |
| 18 | `ODs-February-2022-Airbus-Commercial-Aircraft.xlsx` | undiagnosed |
| 9 | `tk-syllabus-comparison-document-v5.xlsx` | the same signature as the first row |
| 6 | `INDEX_Digital_Transformation_Toolkits.xls` | pictures — see below |

Below those it is a ±1-to-±3 tail: the failing page deltas are `-6:3 -4:1 -3:5 -2:5 -1:10 0:9
+1:6 +2:1 +4:1 +6:2` once the five above are set aside.

## Sheets batches 006–010: a merge has two axes, and a shape has a face — swept at `7049756d9`

Whole track swept at `7049756d9` before anything was changed and again after each fix, 171
documents each time. **The baseline reproduced the briefed 125/171, page error 222 and 134 exact
page counts to the digit** — the second handover on this track to be exactly right, after two that
were stale-low.

| | before | after |
| --- | --- | --- |
| documents matching | 125 | **125** |
| documents with an exactly correct page count | 134 | 134 |
| total absolute page error | 222 | 222 |
| every batch | — | unchanged |

**No document changed verdict, and no page count moved in either direction.** That is the honest
headline: two fixes landed, both measured neutral-or-better across the whole track, and neither
bought a match. What they bought is words on the page — nineteen documents' word counts moved, and
the largest single move on this track since the seventh round.

### A merged block reaches back on two axes, and only one was walked

`ScOutputData::GetMergeOrigin` (`sc/source/ui/view/output2.cxx:953`) walks left while a position is
`bHOverlapped` (`:989`) and then **up** while it is `bVOverlapped` (`:1008`), both gated by the same
`bDoMerge` — `bIsLeft = (nX == mnVisX1)`, `bIsTop = (nY == mnVisY1)`, and both together for a cell
covered on both axes (`:958-983`). `DrawCoveredMerge` implemented the left walk and not the up one,
so a block anchored on the page above vanished from the page its tail falls on.

**The predecessor's salvaged patch was right and its explanation of the previous round's was wrong
in the way this file keeps recording.** The seventh round wrote `RegChangeReport.xlsx` up as a
manual-height row whose overflow was redrawn, and refuted itself with a probe built to that shape.
Row 24 really is 12.75 pt and `customHeight`; what it missed is that the 3 278-character
description *in* that row is `B24:B58`, a merge thirty-five rows tall, with `B59:B84`, `B85:B97`,
`B103:B112` and `B113:B135` behind it. The probe could not reproduce it because the probe had no
merge.

Measured: **2314 words against 3137, and 3060 now**, its pages 2, 3 and 4 going from 5, 294 and 249
to 505, 446 and 343 against 506, 434 and 344. Twelve documents changed across the track — eight
closer, one exact (`Aircraft_Database.xlsx` 16504 → 16520 of 16520), three further by 8, 15 and
1064 words on totals of 2372, 68 113 and 1 293 910, all three still matches.

### A shape's text is set in the face its runs name

`SheetShapeRun` carried a size and no typeface, so every text box on every sheet was shaped in the
furniture face. `+mn-lt` — four of the seven affected documents write it — is not a family name but
a reference into the theme's font scheme, and `DrawingFontScheme` already resolved it for the other
two families.

Measured on `SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.xlsx`: drawn line pitch **12.65 pt
against the reference's 13.43, and 13.43 now**; words 479 → 519 of 550. **Seven of the track's 109
package spreadsheets state a typeface on shape text**, all seven were rendered before and after,
two moved closer, one moved two words further on 3740, four did not move, **none changed verdict**.

### Three failures diagnosed and not fixed, and the first is worth 42 pages of the 222

- **`orbus_togaf_tool_csq.xls` (017), 33 pages against 75: the reference prints a sheet the file
  does not contain.** Its pages 34–75 are headed `DPCache`, and the workbook's BOUNDSHEET records
  name six sheets, none of them that. `XclImpPivotCache::ReadPivotCacheStream` makes it — a pivot
  cache whose source is an external or deleted sheet has no range to point at, so the filter calls
  `rDoc.MakeTable` and names the new sheet `DPCache`
  (`sc/source/filter/excel/xipivot.cxx:717-733`). **19% of the track's whole page error, on one
  document, and it is the reference inventing a sheet.** `DPCache` appears in exactly one of the
  171 reference PDFs, so the reach was measured rather than assumed.
- **`INDEX_Digital_Transformation_Toolkits.xls` (010), 18 against 24 with the words matching
  exactly.** It reads as six blank pages the reference keeps and is not: those six are the narrow
  first column band of the last sheet and they carry 30, 42, 38, 32, 36 and 16 **images** apiece.
  `SheetEmptyPages.TouchedByADrawing` would keep them; it does not, because we draw no images from
  this workbook at all. A census across the track found **twelve documents whose image count
  differs from the reference's**, four of them badly — `apron-area.xls` 0 against 1670,
  `INDEX_…xls` 0 against 414, `SIL_TDB648.xlsx` 2 against 320,
  `Application_Compliance_Checklist_5_Apr_2021.xlsx` 0 against 266 — and `apron-area.xls` is a full
  match, which is why this had never surfaced. The word gate cannot see a picture.
- **`TK-Syllabus-Comparison-Document-v2.xlsx` (017), 1314 against 1235 — 36% of the page error on
  one document.** The words agree; the rows do not. On page 5 both renderings agree to 0.02 pt down
  to the fifth row, both draw the same 25 wrapped lines breaking at the same words at the same
  13.45 pt pitch, and the three rows those lines sit in measure 95.5, 135.8 and 122.3 pt in the
  reference against 105.0, 149.9 and 135.0 here — **1.0995, 1.1038, 1.1038.** The height reserved
  and the height drawn disagree with each other by the same ratio at three different line counts.
  These sheets state `ht` on 634–719 rows *without* `customHeight`, so the number is
  `SheetOptimalRowHeights.WrappedHeight`'s, and its 96 dpi quantisation was fitted to thirty probe
  rows in one face. Read the caveat in the module TODO before trusting the ratio as a per-line
  figure: the rows are 21 columns wide and the tallest cell was not the one whose lines were counted.

  A second document carries the same signature and is in batch-009 rather than 017.
  `airports_6.xlsx` is 18 pages against 17, its pages hold 590, 634, 667 and 644 words against 644,
  654, 727 and 694 — **about 9% fewer per page** — and its dominant drawn row pitch is **8.99 pt on
  both sides**, so again the pitch agrees and the fit does not. It states `ht` without
  `customHeight` on 262 of its 1024 rows. Two documents is not a cluster, but it is the reason to
  measure this one properly before writing it off as one workbook's oddity.

### What this round says about the track

The seventh round's brief called sheets a *quantity* problem — 222 pages of error over 46 failing
documents — and asked for the shared shape behind it. Sorted by page delta, there is no shared
shape: **the top three documents are 139 of the 222 pages**, and they have three unrelated causes,
one of which is the reference and not us. Below them the distribution is a long ±1 to ±3 tail of
individual documents. So the quantity reading has run out with the systematic causes that produced
it, and 42 of the 222 should be struck off as not ours at all.
## Sheets batches 005–008: three ways a cell escapes `DrawStrings`, swept at `28d786009`

Whole track swept at `e2e0bdee3` and again after each fix, 171 documents each time. **The
baseline reproduced the briefed 122/171, page error 222 and 134 exact page counts to the digit**
— the first time this track's handover has been exactly right, and worth recording because the
last two rounds' briefs were both stale-low.

Verified on the merged branch at `b0dee3fac`, after that commit was made: fidelity **520 of 520,
0 skipped**, agreeing with the worktree figure. The merge commit deliberately said the suite was
still running rather than assuming it, so this is the promised result.

| | before | after |
| --- | --- | --- |
| documents matching | 122 | **125** |
| documents with an exactly correct page count | 134 | 134 |
| total absolute page error | 222 | 222 |
| `batch-001`–`004` | 40/40 | 40/40 |
| `batch-005` | 9/10 | **10/10** |
| `batch-006` | 7/10 | **8/10** |
| `batch-008` | 7/10 | **8/10** |
| every other batch | — | unchanged |

**Nothing regressed** — no document changed verdict, page count or word count in the wrong
direction, and the three that changed state all went `words` → `match`. Page error and exact page
counts are unmoved because all three fixes are about *which characters reach the page*, not about
where the page ends.

The three defects are one finding: **`ScOutputData::DrawStrings` is not the only thing that draws
a cell.** A cell that leaves it for `DrawEditStandard` is *clipped* rather than *shortened*, and
the difference is visible in the PDF's text layer. In LibreOffice's own output those cells are
easy to spot — they are drawn in a second, `/P<</MCID n>>BDC`-tagged pass at the end of the
content stream, each behind a clip one row tall.

- **Seven code points send a cell to the EditEngine** — `HasEditCharacters`,
  `sc/source/ui/view/output2.cxx:823-847`, consulted at `:1812`. `esurf-12-135-2024-t01.xlsx`
  writes its dates `28<NBSP>Oct<NBSP>2012`: 113 words against 124, and 123 now.
- **A hyperlink replaces a cell's content with one field, and a field is never wrapped** —
  `insertHyperlink` (`sc/source/filter/oox/worksheethelper.cxx:1062`) and `lclInsertUrl`
  (`sc/source/filter/excel/xicontent.cxx:157`), with the rule stated in `readCellContent`'s own
  comment at `output2.cxx:2560`. A URL is exactly the string a line breaker splits at every
  solidus, so a wrapping column of links measured four or five lines a row instead of one — which
  is a row height, hence a page count. `Published_Issuances_2024.xlsx`: 482 against 458, and 458
  now. **33 of the 171 documents carry cell hyperlinks**; the other 32 were rendered before and
  after and none changed verdict.
- **A clipped string's surviving glyphs do not move.** Dropping the head of a right-aligned string
  leaves the rest where it stood; it was being shifted right by the width dropped.
  `RVSM_Non_approved_list_2025_84c0b3f4ac.xlsx`'s left-clipped dates ran flush into the next
  column, 5.54 pt right of the reference: 419 words against 445, and 445 now.

### What is left in 006–009, with what is established about each

- `Hazard Analysis Template.xls` (006) prints its **cell notes on a page of their own** — Excel's
  "Comments: at end of sheet", `EXC_SETUP_PRINTNOTES` → `ATTR_PAGE_NOTES`
  (`sc/source/filter/excel/xipage.cxx:84`, `:257`). 2 pages against 3. Nothing here reads cell
  comments for layout in any format, so it is a feature rather than a wiring change.
- `SSRO_…_DATA.xlsx` (006) is the **single-face shape text** limitation `SheetShapeText` already
  records: its notes box states `+mn-lt` against a Calibri theme, the reference's line pitch is
  13.5 pt and ours 12.5, which inverts to Carlito against Liberation Sans. Measured reach: **7 of
  the track's 109 XLSX** have shape text naming a typeface.
- `dragon-175066A.xlsx` (007) 14 pages against 13 and `commander-authorisation…xlsx` (008) 22
  against 23 are both cumulative row-height drift with the words already inside the band — no
  single page diverges, every page's word count differs a little.
- `Computer and Software Services_50 State Comparison.xlsx` (008) is 24 against 26 with the words
  matching, and **the two missing pages are blank in the reference too** — footer page number
  only. LibreOffice keeps them because `ScDocument::IsPrintEmpty`'s last branch runs
  `ExtendPrintArea` from column 0 up to the band's first column (`documen9.cxx:486-505`): a
  string in a column to the left reaches into the band, and the reference draws it at x = −348,
  off the paper. We drop the pages.
- `RegChangeReport.xlsx` (009) is measured further below in the module TODO and still not caused.
  The obvious next theory is also refuted: a probe reproducing the file's shape — a 12.75 pt
  manual-height wrapping cell holding 420 words — renders on one page with the text cut after
  four lines, where the corpus document's equivalent cell is drawn on three consecutive pages at
  three different vertical offsets.

## Words batches 010–019, measured at `65c197f15`: 53 of 98

Swept to decide where the next words dispatch should aim. **All ten batch figures reproduced
the scoreboard exactly** — 6/9, 7/10, 8/10, 5/9, 3/10, 5/10, 7/10, 5/10, 3/10, 4/10.

That is worth recording because it contradicts what this file kept finding. Six stale records in
one round, eleven of eighteen in another, and every one stale-low; I swept expecting the same and
got ten exact hits. The difference is that the previous words agent swept the **whole 200-document
track** rather than only the batches it worked, so every row was refreshed at once. Stale records
are a symptom of partial sweeps, not of time passing — which makes a whole-track sweep worth its
cost beyond the number it produces.

### The shape of the 45 failures

| | |
|---|---|
| over-paginate | 23 |
| under-paginate | 15 |
| page-exact, words wrong | 7 |
| **off by exactly one page** | **26** |
| word count short / long / equal | 33 / 9 / 3 |
| absolute page error in this range | 87 of the track's 122 |

Two things follow. Off-by-one dominates, so this range is the boundary problem the track was
already diagnosed as. And **word counts are short far more often than long, in both pagination
directions** — 33 against 9. That is not the classic cascade, where fitting too much on a page
makes text and pages move together; text is missing roughly independently of where the breaks
fall.

### One producer accounts for six failures

Seven documents in this range come from one issuer's form templates (EASA CAO / Part-145
`TE.CAO`, `UG.CAO`, `FO.FCTOA`). **Six of the seven fail**, and the family holds the two largest
word shortfalls in the range, −446 and −225.

**Three hypotheses were tested against it and each explains only a minority. None is the cause.**
Recorded so the next agent does not spend the round re-testing them:

| Hypothesis | Measured | Verdict |
|---|---|---|
| the `TOC` field's cached result is dropped | TOC holds 48 and 145 words against shortfalls of 446 and 225 | explains a fraction at most |
| grouped text boxes drawing only their first `txbxContent` (a known unfixed bug) | 198 and 64 words in text boxes, against 446 and 225 | explains a fraction |
| the same, on the other big shortfalls | `xx_SETIS_PWS…docx` (−541) and `t_TEMPforInvProgs.docx` (−118) contain **zero** text boxes | refuted for those |

The two `B11. TE.CAO.00129` files looked like an ideal A/B pair — our output 1181 and 1183 words
where the reference gives 1278 and 1110, so LibreOffice diverges by 168 where we diverge by 2.
They are **different revisions**, not one document twice: different images, one has
`footer1.xml.rels` and the other `stylesWithEffects.xml`. The pair is still suggestive but it is
not the controlled comparison it appears to be.
## After the eighth round: words, and a label that is bigger than its paragraph

Swept whole-track three times at three commits, each against a checksummed CLI snapshot so the
sweep and the tree could not disagree. **142/200 at the base, 143/200 after**; total absolute
page error 122 → 122; documents with an exactly correct page count 153 → 153.

The base sweep reproduced every one of the twenty-one recorded batch figures exactly — the first
round in a while where the record needed no correction.

| Batch | before | after | |
|---|---|---|---|
| `batch-005` | 9/10 | **10/10** | `loi_format…doc` 9 pages → 10 against 10 |
| `batch-007` | 8/10 | **9/10** | `Press release_EUREKA…docx` 798 words → 838 against 823 |
| `batch-015` | 5/10 | **4/10** | `hdss-bulletin-issue-285…docx` 10 pages → 11 against 10 |
| every other batch | — | unchanged | |

Batches 001–005 are 10/10 and 006 is 9/10; the gate for moving on is green.

### A list label can be bigger than its paragraph, and then it raises the line

Landed, and it is two defects rather than one. A level states character formatting for its label
alone — `w:lvl/w:rPr/w:sz`, `sprmCHps` in a WW8 level's `grpprlChpx`, `text:style-name` on an ODF
level — and none of the three was read, so every label was drawn at its item's size. And the
label is not a run, so it contributed nothing to the height of the line it sits on. Writer's is a
portion (`SwNumberPortion`, `sw/source/core/text/txtfld.cxx:506`) that
`SwLineLayout::CalcLine` (`porlay.cxx:340`) folds into the line's maxima like any other.

The ODF half had a third defect under it: `OdfListLevel.TextStyleName` read `text:style-name` as
`text:text-style-name`, so it was always null. Read, never used, and never even found — and it is
exactly where LibreOffice's own WW8 import writes a level's `grpprlChpx` back out.

Measured on `list-label-taller.fodt/.doc/.docx`, generated by converting one hand-written flat-ODF
fixture: the item's first line advances 22.45 pt and the next 13.13 against LibreOffice's 22.95 and
12.63 — the same 35.58 pt total, which is Liberation Serif's 25.30 pt line box at 22 pt plus its
12.65 at 11. Identical in all three formats.

**A face-only difference raises the line too, and is the commoner case.** Probed against
LibreOffice: an 11 pt Carlito label over 11 pt Liberation Serif text gives a first line of 12.90 pt
against 12.65. That is the 12.90 in the previous round's `loi_format…doc` pitch measurement, and it
is why this fix reaches lists that state no size at all.

Two things it did not fix, both measured and both left:

- **The external leading sits on the wrong side of the baseline.** Inside that exact 35.58 pt the
  split is 22.45/13.13 against 22.95/12.63: `LineMetrics.ScaledAscent` adds a face's external
  leading to the ascent on a device grid and not when scaling exactly, while
  `SwFntObj::GetFontAscent` adds it in both. Invisible on a uniform line — every line moves
  together — and visible only where two sizes share one. Fixing it moves every baseline in every
  document by the leading, so it wants its own round and its own sweep.
- **`hdss-bulletin-issue-285-25-june-2025.docx` regressed 10 pages → 11 against 10.** Its bulleted
  items sit under an exact line spacing that clamps the height, so what the taller label moves is
  the baseline inside the box; the first item of one list went from 20.0 pt below the paragraph
  above to 20.5 against the reference's 19.45. We were 0.55 pt out before and are 1.05 out now, on
  a document that was passing because two errors cancelled.

### A `wpg:wgp` group draws every shape now

`DocxFrames.Read` took `Descendant(placed, "txbxContent")` — the first text box anywhere under the
anchor — so a group drew one member and lost the rest silently. Thirteen of the track's 134 DOCX
carry a `wpg:wgp`. A group is now flattened into an envelope frame plus one frame per leaf, mapped
through `(off − chOff) × ext ÷ chExt`; nested `wpg:grpSp` compose. A member is aligned by the
*group* (`PageFrame.GroupSize`) and only the envelope carries the wrap.

Five documents' word counts moved, four of them towards the reference:

| Document | before | after | reference |
|---|---|---|---|
| `Press release_EUREKA…docx` | 798 | **838** | 823 — now a match |
| `TE.CAO.00125…OJT Logbook.docx` | 2650 | **3066** | 3096 — words now inside the band |
| `docs-quality-MA.IMS.00001….docx` | 12085 | **12176** | 12397 — words now inside the band |
| `FAA-2017-0628-0002_attachment_1.docx` | 638 | **639** | 639 |
| `UG.CAO.00133…Language.docx` | 3475 | 3925 | 3700 — 225 short became 225 over |
| `UG.CAO.00006…User Guide….docx` | 7370 | 8120 | 7458 — newly failing on words |

The last two are the open question this leaves: an overshoot of almost exactly the undershoot on
one of them says we now draw something LibreOffice suppresses — a hidden shape, a placeholder with
`noTextEdit`, or a member drawn once per page where the reference draws it once. Neither document
changed verdict (both were already failing on pages), so it cost nothing, but it should be looked
at before the group reader is trusted further.

### A section break inside a content control was not a section break

`DocxContentReader.SectionProperties` walked `w:body`'s own `w:p` children only, while
`DocxLayoutSource.Walk` — which numbers the blocks by section — descends through `w:sdt`. Word
wraps a cover page or a table of contents in one as a matter of course, so a document whose second
section ends inside the control had that section vanish and every later section shift up by one.
Two of the track's 134 DOCX are affected. On `final-technical-report-template.docx` the running
head drew on pages 1 and 3 against the reference's 2 to 6; it now draws on every page, and the word
count went 1108 → 1117 against 1135, inside the 2% band. **It cost that document its page count**
— 6 → 5 against 6 — so its verdict did not change and total page error rose by one. The change is
right by inspection (a reader must not drop a section) and the page it lost is a separate,
unfixed pagination error on the same document.

### Still diagnosed and not fixed

- **`w:titlePg` with no first-page reference must draw *no* header, not the default.** This is the
  other half of `final-technical-report-template.docx`: we draw the section's default header on
  page 1 where the reference draws nothing. 42 of the 134 DOCX state `w:titlePg` and 6 of those
  declare no `w:type="first"` reference anywhere, so the reach is broad enough to need its own
  before-and-after sweep rather than a spare hour.
- **`WritingFieldKind.PageNumber` still has no consumer**, and `PageFurnitureSet`'s own remark
  ("resolving fields is a later pass than this") still describes a pass that does not exist.
  Unchanged from the seventh round. Note it cannot move the gate: a footer printing `9` on every
  page counts as one word exactly as `1`…`9` does, so this is a correctness item to be justified
  on its own terms rather than by the parity number.
- **`f445896eb008d14c1746fc37d412dc22.docx`** (006), 15 pages against 16 — a table row we move
  whole and the reference splits. Untouched this round.
- **Flat ODF whitespace is not collapsed.** Noticed while building the fixture: a newline and
  indentation inside a `text:p` come through as a line break and leading spaces, where LibreOffice
  collapses them to one space per the ODF whitespace rules. No corpus reach — real ODF writers do
  not pretty-print inside `text:p` — but it makes a hand-written `.fodt` fixture lay out
  differently from the same file through `soffice`, which is a trap for the next agent who writes
  one.

## After the ninth round: words, and three measures of a paragraph that disagreed

Swept batches 001–007 and 010–017 together at `7049756d9` plus the three fixes below — **148
documents, 116 matching, 34 pages of absolute error, 122 documents page-exact.** Batches 001–005
are 10/10, so the gate for moving on is green.

| Batch | before | after | what moved |
|---|---|---|---|
| `batch-010` | 6/9 | **7/9** | `technical report template.docx` 9 pages → 10 against 10 |
| `batch-011` | 7/10 | **8/10** | not attributed — see the caveat below |
| `batch-015` | 4/10 | **5/10** | `hdss-bulletin-issue-285…docx` 11 pages → 10 against 10, undoing the eighth round's regression |
| every other batch | — | unchanged | 001–005 10/10, 006 and 007 9/10, 012 8/10, 013 5/9, 014 3/10, 016 7/10, 017 5/10 |

**The caveat, stated rather than buried.** `batch-010` was swept at my own base before the fixes
and its 6/9 is mine; `batch-011` and `batch-015` are compared against the *recorded* scoreboard
rather than against a base sweep of my own, so "one document changed state" there is a difference
between two measurements taken at different times and I cannot name the document. Treat those two
rows as weaker evidence than the `batch-010` row.

Each of the three fixes below is one sentence of mechanism, and each has a unit test that was
watched to fail with the defect put back.

### A row's part was measured one way and the row another

`TableLayouter.HeightAt` sized the part of a table row that goes on a page from ink — first kept
line's top to last kept line's bottom — while `LayOut` sizes the row from the cell's
`PlacedFlow.Advance`, which carries the first paragraph's space-before and the last one's
space-after. Cutting at the last line therefore looked cheaper than the row, so `SliceRow`
reported "this holds every remaining line", declined the split as pointless, and handed the row
back to a caller that had already found it too tall by the other measure: the row moved whole and
the difference went blank. Measured on `f445896eb008d14c1746fc37d412dc22.docx` (006) as **205.8 pt
of empty page**. Fixed at `Layout/TableLayouter.cs:292` — the row's own first part starts at the
flow's top, and a part holding a cell's last line ends at its advance.

It did **not** fix that document, which is still 15 pages against 16, but its first three pages
now agree with the reference where only the first two did.

### A list label's tab stops looking too early

The follower after a list label is a real tab — Writer's number portion expands to the number plus
`SvxNumberFormat::GetLabelFollowedByAsString`'s `"\t"` (`editeng/source/items/numitem.cxx:504`) —
so past a stop already behind the pen the search carries on through the paragraph's own stops and
then along the default interval (`sw/source/core/text/txttab.cxx:189`). `ListLabel.Advance` stopped
at the level's own stop, so a label wider than its level's room left **no gap at all**. Fixed at
`Layout/ListLabel.cs:214`.

Measured on `final-technical-report-template.docx` (007): an 18 pt `2.0` is 23.0 pt wide against a
stop 18 pt along, and LibreOffice starts the heading at 36.0 pt — the document's
`w:defaultTabStop` — where we drew `2.0Background`. We now start it at 36.00.

### "Contextual" spacing means the same style, not the same properties

`w:contextualSpacing` suppresses the gap only between paragraphs **of the same style**, and Writer
decides that by comparing the two nodes' `SwTextFormatColl` pointers (`lcl_IdenticalStyles`,
`sw/source/core/layout/flowfrm.cxx:1503`). `SharesContextualSpacing` approximated style identity by
line spacing, start indent and alignment — which a heading style based on the body style inherits
wholesale, along with the contextual flag itself. So the space above **every** heading in such a
document disappeared. `ParagraphFormat.StyleKey` now carries the style and all four readers set it
(`Ooxml/WordParagraphFormats.cs:180`, `OpenDocument/OdfParagraphFormats.cs:141`,
`Ww8/Ww8LayoutFormat.cs:288`, `Rtf/RtfDocumentReader.State.cs:1147`); the old comparison remains
as the fallback for a reader that cannot name a style.

Reach is broad — `w:contextualSpacing` on a `Normal` that headings derive from is Word's own
default template — and the three earlier batches held at 68 of 70 across it.

### Two things measured this round and deliberately not fixed

- **We take the larger where LibreOffice adds.** With a `w:after` on the paragraph above and a
  `w:before` on the heading, LibreOffice's gap is their **sum** — 8 + 12 pt measured on a
  hand-written fixture — and `PaginationOptions.CollapsesSpacing` makes us take the larger. The
  mapping is the bug: `DocxReader.cs:270` sets it from `w:doNotUseHTMLParagraphAutoSpacing`, but
  writerfilter never sets `PARA_SPACE_MAX` for DOCX at all — it keeps the *application's* configured
  `AddSpacing`, which is on. So a Word document should always add. This moves every DOCX with both
  spacings set and wants its own round and its own whole-track sweep; the corpus fixture beside it
  sets every `w:after` to zero precisely so the two questions did not have to be answered together.
- **A row LibreOffice refuses to split, and the threshold is unexplained.** On
  `f445896eb008d14c1746fc37d412dc22.docx` the reference leaves 144 pt of page 3 blank rather than
  break row 8. Rewriting that row's `w:trHeight` and re-rendering puts the threshold between
  **4200 and 4300 twips**: at or below it LibreOffice splits the row, above it the row moves whole.
  The room left on the page is 2880 twips, so the rule is not "the declared minimum must fit"; I
  could not find the quantity 4250 twips is. Recorded as a measurement with no explanation attached,
  which is the honest state of it.

### And one open item carried forward unchanged

`WritingFieldKind.PageNumber` still has no consumer. Not attempted this round.

## After the eighth round: slides drop shadows, and two recorded leads that were backwards

Whole slides track swept before and after at `e2e0bdee3`, 163 documents each time. The baseline
reproduced the briefed **151/163** to the digit — same twelve failures, no `ref-failed`, every
page count exact — which is the tell that the instrument and the base are both right.

| | baseline | after |
|---|---|---|
| documents matching | 151 | **152** |
| pages with a correct count | 163/163 | 163/163 |
| documents with an unembedded face | 0 | 0 |

**Exactly four rows of the 163 changed, and all four are the intended ones.** Three of the four
are documents that already matched and now sit closer to the reference:

```
71393_pp7.ppt                            2161/2171  ->  2172/2171   match  -> match
redac-sas-201403-ppt-portfolio-rev-sim   1999/2026  ->  2030/2026   match  -> match
16 - UTM - (NASA).pptx        fonts 10/11 -> 11/11               words  -> words
pres_ioc_phuket.ppt                       974/1005  ->   998/1005   words  -> match
```

### Drop shadows: a feature, and the three things that decide whether it helps or hurts

Nothing drew a shadow at all, on any of the three readers. LibreOffice draws the shape *again* —
offset, every colour replaced by one, behind itself — and that shape of the feature settles three
questions a "grey copy of the outline" reading gets wrong. All three were measured; two of them
would have made the track worse if guessed.

**The colour is replaced and the transparency is not.** Page 34 of
`Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx` is covered by a rectangle whose
gradient runs from zero to 30% alpha and whose shadow states no distance, so the copy sits exactly
underneath it. Cast as a flat opaque fill it tinted the whole slide and that page's unaccounted
ink went **0.18% → 13.52%**; cast with the fill's own alpha it is invisible, as the reference
shows. The document's whole-file ink went 19.73 → 38.85 → **19.51** across the two attempts.

**Blur decides whether the shadow's text is text**, and this is the one that would have cost a
sweep. With a non-zero radius LibreOffice rasterises the shadow, so a themed shadow reaches the
reference PDF as a greyscale image with a soft mask and *no words* — verified on `passiv.pptx`,
whose every page carries a 918 × 272 gray JPEG plus smask and whose count is unchanged at
1256/1256. With a zero radius it stays vector and the text extracts, which is how
`pres_ioc_phuket.ppt` comes to draw "National" fourteen times in seven pairs 6.01 pt apart.
Drawing the shadow's text under blur would have added words to every deck with a themed shadow.

**Most shadows come from the theme.** 1120 slide shapes across the 112 corpus `pptx` decks reach
one through `p:style/a:effectRef` against 352 that state one on their own `p:spPr`.

The reach figure in the handover — "59 of 112 decks state a shadow, 3296 occurrences" — is an
order of magnitude high, in exactly the way the skill warns about. Parsed rather than grepped, and
counting only what a renderer acts on, it is **38 decks and 1472 slide shapes**: most of the 3296
are `a14:hiddenEffects` (1740, which is PowerPoint's record of effects it is *not* drawing) and
unreferenced theme entries.

**One rule was measured rather than reasoned, and the source alone gets it backwards.**
`EffectProperties::assignUsed` replaces the whole effect list when a shape states any effect, which
says `<a:effectLst/>` or a lone `a:glow` drops the theme's shadow. LibreOffice 24.2.7.2 keeps it in
both cases — its flat-ODF export of the new `slide-drop-shadow.pptx` gives the themed 38% black to
the plain shape, the empty-list shape and the glow shape alike. So the first source that states an
*outer shadow* wins, not the first that states anything.

Left undone and stated: the blur itself, which needs offscreen rasterisation this layer has no
access to, so a soft shadow is drawn hard; and a picture's silhouette, so a picture casts a shadow
of its frame and only when its bytes are a JPEG, which has no alpha channel and for which the two
coincide.

### Two recorded leads that the rendered pages contradict

Both were carried for two rounds. Both were reached from a word count, and both are backwards.

**`Demick_JetBlue.pptx` — "the reference draws none of those labels and we draw every one".** The
opposite. The chart states 21 category points; LibreOffice draws about twenty rotated 45° along the
axis and **we draw eleven**, so our rhythm thinning is the more aggressive of the two. What
produced the wrong reading is the word gate's blind spot working in both directions at once: the
reference's rotated labels extract as *nothing at all* under `pdftotext` and ours extract as
*fragments* — `2012-9` comes back as `12`, `20`, `9` — so the page reads +29 words for us while we
put fewer marks on it. Two other differences on that page are real and easier than the density
question: the reference draws a secondary value axis with its own title and we draw neither, and
our legend collides with the category-axis title.

**`a:prstTxWarp` — "appears nowhere in `dotnet/src`", with the implication of a wide gap.** True
and narrow. The attribute appears on 39 of the 112 corpus `pptx` decks, 722 times — but **709 of
those are `textNoShape`**, the identity, and three more are `textPlain`. Ten occurrences across two
decks bend anything, and eight of the ten are `FAAAIandtheArtandScienceofV&Vfinal.pptx`.

### The `/FontFile2` defect is real, and its stated mechanism is only the surface

The measurement reproduced exactly: `16 - UTM - (NASA).pptx` is the one document of 163 whose PDF
poppler complains about, 161 times, and all three gate checks pass while 161 glyph runs draw
nothing. The cause is not the stream key alone. Correcting it to `/FontFile3` with
`/Subtype /OpenType` under a `/Type1` dictionary — what PDF 1.7 §9.9 asks for — makes the file
valid and still draws nothing useful, because a Type1 dictionary selects glyphs by *name* through
the CFF charset while our codes are glyph indices: an 18 pt Loma probe came back as a row of tofu
boxes, and so did the same probe before the change. And Unifont, which is what a last-resort
fallback reaches, is **CID-keyed**, which no simple font dictionary admits at all.

So a CFF-flavoured program is now dropped rather than misdescribed. That buys a valid file and 161
fewer reader errors, not correct outlines; the complete fix is a `/Differences` glyph-name encoding
for name-keyed faces and a composite `/Type0` font with Identity-H codes for CID-keyed ones, and
neither is a line's change. `CompactFontFormat` reads a CFF top dictionary for the `ROS` operator
so the two cases can be told apart when someone does it.

**A hole in the gate found on the way.** `batch-check.sh`'s third check is
`pdffonts … | awk '$(NF-3)=="no"'`, which reads the `sub` column rather than `emb` for a row whose
type is a single word. Every font Paperless writes is `TrueType`, one word — so **the font
embedding check has never tested our own output**. It fires correctly for `Type 1`, `Type 1C` and
`CID Type 0C`, whose names are two or three fields. Worth fixing in the script rather than in a
document.

## The ninth round, swept whole at `18b4547ef`: 187 of 188

| Track | Batches | | |
|---|---|---|---|
| `words` | 001–005 | **50/50** | — |
| `slides` | 001–009 | **87/88** | `8_P-Pavese_AIRBUS…pptx` — the ceiling |
| `sheets` | 001–005 | **50/50** | — |
| **total** | | **187/188** | |

Up from 185/188, and **words 001–005 is now clean for the first time**. The document that closed
it is `loi_format_letter_of_intent…doc`, which had failed since the baseline: 10 pages against 10
and **3365 words against 3365** — exact on both counts, where it had been 9 against 10.

That one is worth following back, because it is the clearest return the salvage discipline has
produced. The list-label diagnosis was written by an agent that never got to act on it; the fix —
letting a label be bigger than its item and raise the line it sits on — was committed by the agent
a VM restart then killed, and survived only because the brief said to commit each fix as it lands.
A later round added the second half, that a label's follower is a real tab and continues past a
stop already behind the pen. Three agents, two of them dead before the result existed.

The single remaining failure in the swept range is the word gate's ceiling rather than a defect:
LibreOffice rasterises an embedded metafile where we emit real searchable text, so closing the
number would mean making the output worse.

### `words` — 200 documents, 21 batches

Measured whole-track at the commit that landed the group reader: **143 of 200**, page error 122.
Batches 001–007 and 010–017 were re-measured in the ninth round — see "After the ninth round"
above — and 010, 011 and 015 are that sweep's figures rather than the whole-track one; the
batches it did not cover (008, 009, 018–021) are unchanged from the whole-track sweep. So the
track total is **146 of 200** by addition and has not been re-swept whole since.

| Batch | Files | Score | Mix | Status |
|---|---|---|---|---|
| `batch-001` | 10 | 43–59 | doc:5 docx:5 | ✅ |
| `batch-002` | 10 | 59–81 | doc:3 docx:7 | ✅ |
| `batch-003` | 10 | 87–102 | doc:5 docx:5 | ✅ |
| `batch-004` | 10 | 102–123 | doc:4 docx:6 | ✅ |
| `batch-005` | 10 | 124–141 | doc:5 docx:5 | ✅ |
| `batch-006` | 10 | 141–158 | doc:4 docx:6 | 9/10 |
| `batch-007` | 10 | 160–185 | doc:4 docx:6 | 9/10 |
| `batch-008` | 10 | 186–204 | doc:4 docx:6 | ✅ |
| `batch-009` | 10 | 208–226 | doc:5 docx:5 | ✅ |
| `batch-010` | 9 | 228–260 | doc:2 docx:8 | 7/9 |
| `batch-011` | 10 | 260–296 | doc:2 docx:8 | 8/10 |
| `batch-012` | 10 | 306–333 | doc:4 docx:6 | 8/10 |
| `batch-013` | 9 | 338–370 | docx:10 | 5/9 |
| `batch-014` | 10 | 372–422 | doc:4 docx:6 | 3/10 |
| `batch-015` | 10 | 424–471 | doc:3 docx:7 | 5/10 |
| `batch-016` | 10 | 473–537 | doc:5 docx:5 | 7/10 |
| `batch-017` | 10 | 537–602 | doc:2 docx:8 | 5/10 |
| `batch-018` | 10 | 620–859 | doc:2 docx:8 | 3/10 |
| `batch-019` | 10 | 956–1521 | doc:1 docx:9 | 4/10 |
| `batch-020` | 10 | 1523–3818 | doc:2 docx:8 | 3/10 |
| `batch-021` | 2 | 4417–4676 | docx:2 | 0/2 |

All twenty-one figures above are the measured sweep at the commit that landed the shrinking
justification, not carried forward. Six of the recorded ones were stale, every one of them
*under*-stating the batch: 008 and 009 at 10/10 rather than 9/10, and 012, 015, 016 and 019
each one or two better than recorded.

### `slides` — 163 documents, 17 batches

Every row below re-proved by a whole-track sweep at `adcbeb2de`: **152 of 163**, the same
eleven failures as at `7049756d9`, all 163 page counts exact.

| Batch | Files | Score | Mix | Status |
|---|---|---|---|---|
| `batch-001` | 9 | 14–282 | ppt:3 pptx:6 | ✅ |
| `batch-002` | 10 | 312–410 | ppt:6 pptx:4 | ✅ |
| `batch-003` | 10 | 411–482 | ppt:5 pptx:5 | ✅ |
| `batch-004` | 10 | 488–560 | ppt:3 pptx:7 | ✅ |
| `batch-005` | 9 | 587–668 | ppt:3 pptx:6 | ✅ |
| `batch-006` | 10 | 671–903 | ppt:4 pptx:6 | ✅ |
| `batch-007` | 10 | 941–1129 | ppt:3 pptx:7 | ✅ |
| `batch-008` | 10 | 1130–1437 | ppt:5 pptx:5 | 9/10 · ceiling |
| `batch-009` | 10 | 1510–1711 | ppt:4 pptx:6 | ✅ |
| `batch-010` | 10 | 1748–1935 | ppt:3 pptx:7 | 8/10 · ceiling |
| `batch-011` | 10 | 1980–2294 | ppt:1 pptx:9 | ✅ |
| `batch-012` | 10 | 2403–3036 | pptx:10 | 8/10 · ceiling |
| `batch-013` | 10 | 3054–3633 | ppt:3 pptx:7 | ✅ |
| `batch-014` | 10 | 3638–4498 | ppt:2 pptx:8 | 7/10 · ceiling ×2, `a14` fallback ×1 |
| `batch-015` | 10 | 4626–7249 | ppt:4 pptx:6 | ✅ |
| `batch-016` | 10 | 7428–13730 | ppt:1 pptx:9 | **8/10** · ceiling ×1, `prstTxWarp` ×1 |
| `batch-017` | 5 | 14810–32582 | ppt:1 pptx:4 | 4/5 · chart labels — see the correction below |

### `sheets` — 171 documents, 18 batches

Measured whole-track at `7049756d9` and again after the eighth round's two fixes: **125 of 171**,
total page error 222, 134 exact page counts — unmoved by either, and the per-batch scores below
were re-proved unchanged. **42 of that 222 is `orbus_togaf_tool_csq.xls`, where the reference
prints a sheet the file does not contain**; see the round's entry above before treating the total
as work outstanding.

| Batch | Files | Score | Mix | Status |
|---|---|---|---|---|
| `batch-001` | 10 | 47–69 | xls:3 xlsx:7 | ✅ |
| `batch-002` | 10 | 69–86 | xls:4 xlsx:6 | ✅ |
| `batch-003` | 10 | 87–116 | xls:5 xlsx:5 | ✅ |
| `batch-004` | 10 | 118–173 | xls:3 xlsx:7 | ✅ |
| `batch-005` | 10 | 173–217 | xls:5 xlsx:5 | ✅ |
| `batch-006` | 10 | 223–249 | xls:3 xlsx:7 | 8/10 |
| `batch-007` | 10 | 253–325 | xls:1 xlsx:9 | 9/10 |
| `batch-008` | 10 | 328–420 | xls:3 xlsx:7 | 8/10 |
| `batch-009` | 9 | 421–540 | xls:2 xlsx:8 | 6/9 |
| `batch-010` | 10 | 560–691 | xls:7 xlsx:3 | 5/10 |
| `batch-011` | 10 | 702–799 | xls:4 xlsx:6 | 6/10 |
| `batch-012` | 10 | 825–995 | xls:1 xlsx:9 | 6/10 |
| `batch-013` | 10 | 1039–1250 | xls:4 xlsx:6 | 7/10 |
| `batch-014` | 10 | 1276–1765 | xls:6 xlsx:4 | 5/10 |
| `batch-015` | 9 | 1773–2264 | xls:4 xlsx:6 | 5/9 |
| `batch-016` | 9 | 2286–4300 | xls:6 xlsx:4 | 4/9 |
| `batch-017` | 10 | 4468–14431 | xls:4 xlsx:6 | 3/10 |
| `batch-018` | 4 | 19384–48127 | xlsx:4 | 3/4 |

## After the ninth round: slides, and two defects the gate is structurally blind to

Whole slides track swept before at `7049756d9` and after at `adcbeb2de`, 163 documents each
time, plus `words/batch-001..005` and `sheets/batch-001..005` because one of the two fixes
lands in `Paperless.Ooxml`, which all three families share. The baseline reproduced the
briefed **152/163** to the digit — the same eleven failures, no `ref-failed`, every page count
exact — which is the tell that the base and the instrument are both right.

| | baseline | after |
|---|---|---|
| slides matching | 152/163 | **152/163** |
| pages with a correct count | 163/163 | 163/163 |
| documents with an unembedded face | 1 | 1 |
| `words/batch-001..005` | — | **50/50** |
| `sheets/batch-001..005` | — | **50/50** |

**The match count did not move, and that is the honest headline.** Every one of the eleven
failures is a word-count failure whose cause is not a colour, and both fixes this round are
colours. What moved is what the gate cannot see.

### The instrument changed, because the gate has run out of things to say

`pdf-image-diff.py` was run over the 45 documents of batches 010, 012, 014, 016 and 017 before
and after. The unit is **unaccounted ink** — the share of a page's ink the two sides cannot
account for between them, summed over a document's pages. Pixel difference is useless at this
point: two renderers that agree about a page still differ on most glyph pixels.

| | before | after |
|---|---|---|
| total unaccounted ink, 45 documents | 2152.23 | **1530.16** |
| pages the tool calls major | 275 | **266** |
| documents whose figure changed | — | **4, all downwards** |

```
466.89 ->  13.95   ws_prod-g-doc-Events-2008-February-5-NATO-activities.ppt   major 12 -> 4
223.97 -> 139.20   HENTZEN_COMPOSITE_MATERIALS_IN_THE_AEROSPACE_INDUSTRY.pptx major 12 -> 12
113.69 ->  65.59   Wildlife for REDAC September 11.pptx                       major 10 -> 9
 79.14 ->  42.88   Demick_JetBlue.pptx                                        major  7 -> 7
```

**All four already matched the word gate**, three of them exactly. Nothing else in the 45
moved in either direction.

### A slide background can be *named* rather than stated, and 60 of 112 decks name theirs

`p:bg` holds one of two children and only one of them is a fill outright. `p:bgPr` carries it;
`p:bgRef` carries an index into `a:bgFillStyleLst` — a fourth style list beside the three
`a:fillRef` uses — plus the colour those entries' `phClr` stands for. The string `bgRef` did
not appear anywhere in `dotnet/src`, so `PptxSlideLayout.Background` walked past it, fell
through slide, layout and master alike, and returned white.

The index is separated from `a:fillRef`'s by magnitude rather than by name: `Theme::getFillStyle`
(`oox/source/drawingml/theme.cxx:49-54`) sends 1000 and up to the background list with 1000
subtracted, and `lclGetStyleElement` beside it *clamps* an index past the end to the last entry
rather than dropping it.

Found on `slides/batch-010/pptx/HENTZEN_COMPOSITE_MATERIALS_IN_THE_AEROSPACE_INDUSTRY.pptx`,
which **matches the reference at 327 words against 326** and draws its white title text on
white paper: its master states `<p:bgRef idx="1001"><a:schemeClr val="bg1"/></p:bgRef>` under a
colour map sending `bg1` to `dk1`, and `dk1` is `#921F07`. Unaccounted ink was 36.49% on page 1
and 21.35% on each of pages 2–12.

Reach, parsed rather than grepped: **60 of the 112 corpus `pptx` decks**, resolving to a
`solidFill` 75 times, a `gradFill` 9 and a `blipFill` 7.

Left undone and stated: `a:duotone`, which those seven `blipFill` entries wrap their blip in.
It is a per-pixel luminance ramp between two colours
(`vcl/source/bitmap/BitmapDuoToneFilter.cxx:15-52`) and the layer that reads the fill has no
image decoder, so six decks now draw the theme's texture untinted rather than nothing at all —
better, not right, and that residue is most of HENTZEN's remaining 139.20.

### A binary deck's colour can name another of the shape's own properties

`MSO_CLR_ToColor`'s `0x10` family holds two different things behind one flag bit, and
`PptColour.Resolve` refused both, with a recorded rationale: "a headless renderer has no desktop
theme, and inventing one would put a colour in the picture that no file states." Indices
**below** `0xF0` do name a desktop colour and that rationale holds for them. From `0xF0` up the
index names **another property of the same shape** (`include/svx/msdffdef.hxx:818-826`) plus a
parameter and a function to put it through, which has nothing to do with a theme and is fully
resolvable headlessly.

The corpus says the distinction is the whole of it. Of the **161** such words across the 51
`ppt` decks, **every one** names a property and **none** names a desktop colour: 31 `fillColor`,
59 `lineOrFillColor`, 71 `shadowColor`, spread over 8 decks. The recorded sentence was right
about the class and wrong about every instance of it in this corpus — the project's standing
pattern, arriving this time inside a source comment rather than inside a handover.

`slides/batch-014/ppt/ws_prod-g-doc-Events-2008-February-5-NATO-activities.ppt` is the clearest:
its master background is a `ShadeScale` whose `fillBackColor` is `0x104301F0` — "take
`fillColor`, darken it by 67/256" — over a `fillColor` of `0x00771531`. That is `#0C051F`, and
LibreOffice's flat-ODF export of the deck states
`draw:start-color="#0c051f" draw:end-color="#311577"`. Falling back to white drew every page as
a pale gradient where the reference draws a near-black one. Its word count was an exact match
throughout, on all thirteen pages.

All eight decks that use the form still match the gate after the change; only the NATO deck's
change was measured with the image diff, because the other seven are in batches with no
before-render kept.

### The eleven that remain

| batch | document | words | what it is |
|---|---|---|---|
| 008 | `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 2240/2108 | ceiling, **verified this round**: page 5's reference is a 692×240 raster with a soft mask and no text |
| 010 | `Fundamentals_Module_1_basics.ppt` | 1146/1099 | ceiling, verified: page 6 is +50 words against a 529×355 raster+smask; the other four differing pages sum to −2 |
| 010 | `W3_Case_Study_of_a_Tsunami_Warning_Simulation_Exercise_Ed.ppt` | 910/817 | ceiling, verified: page 10 is +93, **the entire delta**, against an 845×572 raster+smask; the other nineteen pages agree exactly |
| 012 | `NAS-Infrastructure-Roadmaps-v16.0.pptx` | 19219/15316 | linked `Excel.Sheet.12` OLE with `mc:Fallback` EMFs; carried, not re-checked |
| 012 | `OnTrac_StarCertificationProgram-3Day.pptx` | 1344/1045 | unattributed |
| 014 | `Thailand17.ppt` | 2813/2697 | unattributed |
| 014 | `N2_E_Maestroni_Swarm_COP.pptx` | 5422/5217 | **new**: page 7 alone is +205, the entire delta — a Gantt whose ~50 task labels we draw (left-clipped mid-word) where the reference draws ~15 |
| 014 | `WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211017-171222.pptx` | 2209/1988 | unattributed |
| 016 | `16 - UTM - (NASA).pptx` | 2459/2261 | the CFF-flavoured OpenType, now dropped rather than misdescribed; 1 of 11 faces unembedded |
| 016 | `FAAAIandtheArtandScienceofV&Vfinal.pptx` | 1201/1145 | `a:prstTxWarp`, 8 of the corpus's 10 bending occurrences |
| 017 | `Demick_JetBlue.pptx` | 713/617 | chart label density and a missing secondary value axis |

`batch-016` is **8/10**, as this file already recorded; the ninth round's brief said 7/10. The
file was right.

### The rasterisation ceiling is a *metafile* path, not an OLE path

Worth narrowing, because it is at least three of the eleven. Already established: the raster is
not in the file and is not the PDF writer's downsampling. It is now also established that it is
**not the OLE replacement path**, which was the obvious remaining suspect.

On `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` slide 5 the shape is a bare `p:pic` — no
`p:oleObj`, no `mc:AlternateContent`, no `a:alphaModFix` — whose one relationship besides the
layout is `ppt/media/image8.emf`. That EMF holds 2832 records, 791 of them `EXTTEXTOUTW`, and
**no EMF+ comment records, no `ALPHABLEND`, no `TRANSPARENTBLT`, no raster operation and no
bitmap of any kind**. The reference PDF's page 5 nevertheless carries a single 692×240 RGB
image with a soft mask and no extractable text. So LibreOffice manufactures the raster from a
purely vector metafile, and the search belongs in the metafile-to-PDF path rather than in
`SdrOle2Obj`.

The counter-example rules out the obvious trigger. `2014BSA_Sunday_Killion.pptx` slide 5 embeds
`image10.emf`, which *does* carry 40 EMF+ comment records and a `BITBLT` — and LibreOffice
draws its text as real text, which `pdftotext` extracts. **EMF+ is not the trigger.** The
standout structural difference between the two is that the rasterised one uses `SELECTCLIPPATH`
(9 records) and `FILLPATH` (15) and the other uses neither. That is a correlation on two
documents and is **not verified**.

### A `path="circle"` gradient focused on a corner is drawn linear — and the checked-out source says otherwise

Measured and deliberately **not** implemented. `slides/batch-017/pptx/Wildlife for REDAC September 11.pptx`
matches on words and its page 3 carries 17.56% unaccounted ink: the background is
`<a:gradFill>…<a:path path="circle"><a:fillToRect l="100000" t="100000"/></a:path>`, we draw a
radial blob in the bottom-right corner and the reference draws a broad diagonal ramp.

A ten-slide probe deck, one `a:fillToRect` per slide and nothing else varying, settles what
LibreOffice 24.2.7.2 does. Confirmed twice over — in its flat-ODF export *and* in its rendered
PDF, because the export alone could have been the exporter normalising something:

| `path` | focus (clamped `cx`,`cy`) | LibreOffice 24.2.7.2 |
|---|---|---|
| `circle` | 100, 100 | **linear, 45°** |
| `circle` | 0, 0 | **linear, 225°** |
| `circle` | 100, 0 | **linear, 135°** |
| `circle` | 0, 100 | **linear, 315°** |
| `circle` | 99, 99 | radial, `cx=99% cy=99%` |
| `circle` | 100, 50 | radial, `cx=100% cy=50%` |
| `circle` | 50, 0 (the Office theme's, clamped up from −160) | radial, `cx=50% cy=0%` |
| `rect` | 100, 100 | rectangular, `cx=100% cy=100%` |

A circle path whose focus lands exactly on a corner is drawn as a linear ramp along that
diagonal; one percent away it is an unmistakable circular blob. Rasterising the two
neighbouring cases at 40 dpi shows exactly that, so this is the model rather than an export
artefact.

**Not implemented on purpose.** The checked-out
`oox/source/drawingml/fillproperties.cxx:529-556` maps `XML_circle` to `GradientStyle_RADIAL`
unconditionally and carries a fresh `FIXME tdf#166140` about the size of these gradients, so
what the reference binary does here is very likely a 24.2 defect that upstream has already
changed. Writing bug-compatibility against a release, for a difference no gate check can see,
in code upstream is actively rewriting, is the wrong trade — and it is worth recording as the
one case this round where "the reference is wrong here" probably *is* the right conclusion, and
where the evidence for it is a constructed probe rather than an inability to explain our own
output. Reach if a later round disagrees: 258 corner-focused `circle` path gradients across 85
of the 112 `pptx` decks, though most are unreferenced theme entries and the skill's warning
about grep-estimated reach applies in full.

### What the next agent on this track should take

The word gate is now measuring rendering strategy rather than defects on at least three of the
eleven, and the remaining ink is concentrated rather than spread:

```
473.17  85 pages major  NAS-Infrastructure-Roadmaps-v16.0.pptx      (linked OLE, known)
155.10  16              2014BSA_Sunday_Killion.pptx                 (an EMF chart drawn on a white
                                                                     panel the reference leaves clear)
139.20  12              HENTZEN_...AEROSPACE_INDUSTRY.pptx          (the missing a:duotone)
 80.22   5              N2_E_Maestroni_Swarm_COP.pptx
 67.65  18              Thailand17.ppt
```

`2014BSA_Sunday_Killion.pptx` page 5 is the cheapest of these to look at and matches on words:
the reference shows the slide through the chart's area and we paint it white, so a correctly
drawn chart sits in a white box that should not be there.
