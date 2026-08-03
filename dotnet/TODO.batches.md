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

Words is now **142/200** (page error 122) — see "After the seventh round" below.

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

## Sheets batches 001–004: full parity, swept at `194c2dc9b`

`batch-00[1-4]` swept together on the final binary: **40 of 40**. Four documents changed state
— `Air_Boss_Master_List.xlsx` and `Bulletin-37-Appendix-2-…xlsx` in 003, `Part_375_Operators.xlsx`
and `Foreign_SA-CAT-I_and_CAT-II-III_Pub_0.xlsx` in 004. `batch-005` rose from a recorded 4/10 to
a measured **7/10** without being worked, which is the usual sign that the causes were systematic
rather than per-document.

Whole track, first sweep since `28940d76f`: **112/171** against the recorded 108, page error
**843** against 860.

### Four of the five defects are one sentence: a thing stopped belonging to exactly one page

- **A styled-but-empty cell is not content.** `ScTable::IsBlockEmpty` asks the cell store, not the
  attribute array — `sc/source/core/data/table2.cxx:2432-2452`. Bulletin-37's columns I–P carry a
  style and nothing else, and bought a sixth page against LibreOffice's five.
- **An empty merge must still block a neighbour's spill.** `ScOutputData::IsAvailable` stops at a
  merged-or-overlapped cell regardless of content — `sc/source/ui/view/output2.cxx:1178-1191`.
  Every reader was losing empty merges because the anchor looks like trailing padding;
  `SheetLayout.StatedMerges` now carries the ranges.
- **A drawing belongs to the sheet and is drawn on every page it reaches** —
  `ScOutputData::PrePrintDrawingLayer`, `sc/source/ui/view/output3.cxx:40-104`. Air_Boss's note box
  straddles a column break and its right half was on no page of ours.
- **Its bound is the page's cell block, not the paper.** Part_375's slicers sit in band 3 and fit
  on the paper of bands 1 and 2, so the first rule alone drew them three times.

The fourth corrects the third, and the agent shipped the wrong version first, measured Part_375,
and replaced it. That is the salvage patch's `ReachesThePaper` clip — kept as an idea, re-derived,
and found wrong. Worth recording as the concrete answer to what a recovered diff is worth: two of
its ideas survived re-derivation and one of them was a defect.

The fifth is unrelated: **`vertOverflow="clip"` removes lines, not pixels** —
`svx/source/svdraw/svdotextdecomposition.cxx:581-624`, with the rule stated at
`include/svx/svdoutl.hxx:56-59`.

### The largest remaining lead on this track, measured

`ht` without `customHeight` is a *hint* that Calc recomputes —
`sc/source/filter/oox/worksheethelper.cxx:1268-1286`. On `National-Reports.xlsx` our row pitch is
**15.735 pt against LibreOffice's 15.0**, read from both PDFs' glyph boxes, and the flat-ODF export
confirms LibreOffice re-measures every row from its content. The flag is already parsed by every
reader — `SheetGrid.IsOptimalSize` — and **has no callers**: the read-but-never-used shape for the
fourth time. What is missing behind it is `ScColumn::GetNeededSize`, which is a feature rather than
a fix, so it was left rather than approximated.

Marked unverified by the agent, and left that way: that `Background_Declaration_Template.xls`
duplicates words for the same reason SpreadsheetML did (the duplication was measured, the cause was
not), and how LibreOffice clips the two-line residue on `Foreign_SA` across column bands.

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

### `words` — 200 documents, 21 batches

| Batch | Files | Score | Mix | Status |
|---|---|---|---|---|
| `batch-001` | 10 | 43–59 | doc:5 docx:5 | ✅ |
| `batch-002` | 10 | 59–81 | doc:3 docx:7 | ✅ |
| `batch-003` | 10 | 87–102 | doc:5 docx:5 | ✅ |
| `batch-004` | 10 | 102–123 | doc:4 docx:6 | ✅ |
| `batch-005` | 10 | 124–141 | doc:5 docx:5 | 9/10 |
| `batch-006` | 10 | 141–158 | doc:4 docx:6 | 9/10 |
| `batch-007` | 10 | 160–185 | doc:4 docx:6 | 8/10 |
| `batch-008` | 10 | 186–204 | doc:4 docx:6 | ✅ |
| `batch-009` | 10 | 208–226 | doc:5 docx:5 | ✅ |
| `batch-010` | 9 | 228–260 | doc:2 docx:8 | 6/9 |
| `batch-011` | 10 | 260–296 | doc:2 docx:8 | 7/10 |
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
| `batch-014` | 10 | 3638–4498 | ppt:2 pptx:8 | 7/10 |
| `batch-015` | 10 | 4626–7249 | ppt:4 pptx:6 | 8/10 · SmartArt text rotated |
| `batch-016` | 10 | 7428–13730 | ppt:1 pptx:9 | 7/10 |
| `batch-017` | 5 | 14810–32582 | ppt:1 pptx:4 | 3/5 |

### `sheets` — 171 documents, 18 batches

| Batch | Files | Score | Mix | Status |
|---|---|---|---|---|
| `batch-001` | 10 | 47–69 | xls:3 xlsx:7 | ✅ |
| `batch-002` | 10 | 69–86 | xls:4 xlsx:6 | ✅ |
| `batch-003` | 10 | 87–116 | xls:5 xlsx:5 | ✅ |
| `batch-004` | 10 | 118–173 | xls:3 xlsx:7 | ✅ |
| `batch-005` | 10 | 173–217 | xls:5 xlsx:5 | 7/10 · WIP |
| `batch-006` | 10 | 223–249 | xls:3 xlsx:7 | 4/10 |
| `batch-007` | 10 | 253–325 | xls:1 xlsx:9 | 6/10 |
| `batch-008` | 10 | 328–420 | xls:3 xlsx:7 | 5/10 |
| `batch-009` | 9 | 421–540 | xls:2 xlsx:8 | 4/9 |
| `batch-010` | 10 | 560–691 | xls:7 xlsx:3 | 4/10 |
| `batch-011` | 10 | 702–799 | xls:4 xlsx:6 | 4/10 |
| `batch-012` | 10 | 825–995 | xls:1 xlsx:9 | 4/10 |
| `batch-013` | 10 | 1039–1250 | xls:4 xlsx:6 | 6/10 |
| `batch-014` | 10 | 1276–1765 | xls:6 xlsx:4 | 4/10 |
| `batch-015` | 9 | 1773–2264 | xls:4 xlsx:6 | 5/9 |
| `batch-016` | 9 | 2286–4300 | xls:6 xlsx:4 | 2/9 |
| `batch-017` | 10 | 4468–14431 | xls:4 xlsx:6 | 2/10 |
| `batch-018` | 4 | 19384–48127 | xlsx:4 | 2/4 |
