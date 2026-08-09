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

Words is **146/200** at `52f45c51b`, and that figure is now measured rather than added up:
a whole-track sweep there gives 146 matches, **154 documents with an exactly correct page
count, total absolute page error 120**, and total absolute word error 7110 — 3200 of it text
we miss against 3910 we draw and the reference does not. The addition it replaces (143 at the
eighth round plus the ninth round's three, which re-swept 001–007 and 010–017 only) happened
to be right. Sheets is **125/171** (page error 222) — see "Sheets batches 005–008" below.

Per batch at `52f45c51b`: 001–005 10/10, 006 9/10, 007 9/10, 008 10/10, 009 10/10, 010 7/9,
011 8/10, 012 8/10, 013 5/9, 014 3/10, 015 5/10, 016 7/10, 017 5/10, 018 4/10, 019 3/10,
020 3/10, 021 0/2.

### Words 001–019 re-measured at `22ed440e0`, and three of those figures were wrong

The line above is carried from `52f45c51b`. Swept again at `22ed440e0` before any change,
batches 001–019 give **143 of 188, total absolute page error 87**, and three per-batch numbers
do not reproduce: **007 is 10/10** rather than 9/10, **015 is 4/10** rather than 5/10, and
**010 is 7/9** as stated. `018 4/10` and `019 3/10` are right here and were quoted as 3 and 4
in the round's brief, which is the same error one step further on.

That matters more than the arithmetic. Two of them looked exactly like a regression from the
table-border fix below — a batch that "fell" from 5 to 4 and one that "fell" from 4 to 3 —
and both were the inherited number being wrong. **Re-measure the baseline of any batch a
change appears to have broken before believing it broke.** The whole cost here was two short
sweeps.

Per batch at `22ed440e0`: 001–005 10/10, 006 9/10, 007 10/10, 008 10/10, 009 10/10, 010 7/9,
011 8/10, 012 8/10, 013 5/9, 014 3/10, 015 4/10, 016 7/10, 017 5/10, 018 4/10, 019 3/10.

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
| `· artefact` | **retired — it excused three real defects.** It marked a miss as the PDF sink writing a long run as several `Tj` capped at 28 glyphs where LibreOffice writes one `TJ` array. The mechanism is real, but a cell whose hard break was dropped and then wrapped mid-token gives an identical token histogram, and all three marked documents turned out to be that. Never mark a document *not ours* on a symptom that two causes share |

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

### Then: a legacy workbook's pictures, and the image check on what already passes

Swept whole again after the picture fix, 171 documents:

| | before | after |
| --- | --- | --- |
| documents matching | 124 | **125** |
| documents with an exactly correct page count | 133 | **134** |
| total absolute page error | 195 | **192** |
| every batch | — | unchanged |

**Three documents changed and nothing regressed.** `TICAPCapability_Final.xls` went
`pages,words` → **match** (16/17 pages and 4096 words, now 17/17 and 4839 of 4930);
`INDEX_Digital_Transformation_Toolkits.xls` 18 → 20 pages against 24; and
`PBN Matrix NAAs (V01).xlsx` stayed a match while the *reference's* word count moved 5557 → 5555,
which is worth noting only as a reminder that the reference is not perfectly repeatable either.

`XlsDrawingCollector` produced a drawing only for a shape carrying `TXO` text, so no `.xls` in any
corpus had ever had a picture on its page, and `MSODRAWINGGROUP` — where a workbook's blip store
lives — was declared and read nowhere. **8 of the track's 61 `.xls` carry one**, 27 `msofbtBSE`
between them.

#### The image check, run over the 125 documents that pass the word gate

The brief called this the track's blind spot and it is: **54 of the 125 have at least one page the
image diff calls MAJOR, 348 pages in all.** Classified by the hint on the worst eight:

| Documents | Class | Reading |
| --- | --- | --- |
| the plurality | *marks displaced or reshaped* | a reflow the word gate cannot see. The extraction comparison is the instrument, not more pixels |
| `alle einzeln.xlsx`, `AFS-400_Contacts.xlsx` | *a fill or background shading the reference has and we do not* | 48 and 30 regions |
| `Aircraft_Database.xlsx`, `links-2026.xlsx`, `environment-edb-…xls` | *ink we draw that the reference does not* | 56, 5 and 3 regions |

`alle einzeln.xlsx` is the sharpest and is **a feature rather than a defect**: its first sheet is a
**pivot table** (`xl/pivotTables/pivotTable1.xml`), and LibreOffice lays the pivot out itself —
its own column widths and its own grid of borders — where we draw the cached cell values into the
file's stated columns with the file's one empty border style. The module TODO's "pivot caches and
defined names, still not reached" is what this looks like on paper, on 36 of its pages.

So the sheets track's image residue is **not** the slides track's shape: it is one unimplemented
feature and a long tail of reflow, rather than missing fills.

## Sheets: a hard break inside a cell is a line — swept whole at `1aefcdfdb`

Whole track swept twice, 171 documents each time, before anything was changed and after the fix.
The baseline reproduced the brief to the digit, which is the tell that both the base and the
instrument are right; the worktree had to be fast-forwarded 325 commits first, the fourth agent
in a row to find that.

| | before | after |
| --- | --- | --- |
| documents matching | 129 | **134** |
| documents with an exactly correct page count | 143 | 143 |
| total absolute page error | 116 | 116 |
| **total absolute word error** | 107 780 | **44 496** |
| `batch-001`–`006` | 60/60 | **60/60** |
| `batch-007` | 8/10 | **9/10** |
| `batch-008` | 8/10 | **9/10** |

`batch-007` and `batch-008` are one document short each and both of those are page-count
failures; `005`, `014` and `018` each gained one.

### The handover's headline lead named the wrong layer, and counting settled it in one command

The record said a hard break was being lost in the BIFF reader, on the strength of LibreOffice's
`.ods` of `CSA_CCM_v1.2.xls` holding "1403 multi-paragraph cells" against zero newlines in our
extraction. Counted on those same two exports: the `.ods` holds **578** multi-paragraph
`table:table-cell` elements out of 1505 cells and our own XHTML extraction of it holds **578**
`<br/>`. The reader keeps every break, `<p>ME 2.1<br />ME 2.2<br />PO 9.5<br />PO 9.6</p>` is line
158 of it, and 1403 is not a count of anything in that file.

That is render-comparison's rule 7 — extraction right, rendering wrong, so the defect is in a
value only rendering resolves. `SheetTextLayout.Wrap` shaped the cell's whole text and returned it
as one line whenever that fitted the column, so a break reached the line breaker only when the
concatenation happened to overrun. `LineCount` beside it split on the break first, which is why
the row heights were already right and no page count moved.

### The `· artefact` mark cost this track three matches by excusing them

Three of the four documents marked `· artefact` — the sink writing a long run as several `Tj`
capped at 28 glyphs — were this defect instead, and the fix moved all three without touching the
sink:

| | before | after | reference |
|---|---|---|---|
| `Praktikastellen_…xls` (005) | 2019 | **1828** | 1828 |
| `FY2021-AIP-grants.xlsx` (014) | 161 982 | **156 679** | 156 662 |
| `STC_WebList.xlsx` (018) | 1 354 164 | **1 297 910** | 1 293 910 |

The token-length histogram that identified them **reproduces exactly** — our text layer really
does have a ceiling the reference's does not. It was reading a symptom: a concatenated cell is
broken mid-token by the wrap, and mid-token fragments are also what an operator-granularity split
produces. Two agents ran that histogram and neither asked whether the long token should have been
there at all.

So the artefact is still real and now costs this track **nothing**, and the lesson is the general
one this file keeps recording: **a measurement is evidence and the sentence after it is a
hypothesis.** Compare the two renderings' *lines* before believing a token-length histogram.

### The next lead, cited but not yet measured

`Computer and Software Services_50 State Comparison.xlsx` (008) is 24 pages against 26, words
2816 against 2819, and the reference's two extra pages hold nothing but their footer's page
number. The sheet is `pageOrder="downThenOver"` over two row bands, so those two are a third
*column* band: LibreOffice's print area reaches column O and ours stops at H, and columns I to O
carry 129 rows of visibly-filled cells with no data at all.

`SheetDecorationArea.Extend` starts its attribute scan below the **sheet's** last data row, 42
here, and then applies `SC_VISATTR_STOP`. Calc asks per column — `ScColumn::GetLastVisibleAttr`
(`sc/inc/column.hxx:892-897`) passes that column's own `GetLastDataPos()`, "0 if none", into
`ScAttrArray::GetLastVisibleAttr` (`attarray.cxx:1922`) — so for a column holding no data the scan
starts at the top of the sheet and the run arithmetic is a different sum. Whether that is enough
to keep these columns is **not measured**; the module TODO says what is missing from it.

## Sheets batch-006: a paragraph is not one size, and a note is not on its cell — swept at `5ec407cf3`

Whole track swept twice, 171 documents each time, before anything was changed and after both
fixes. **`batch-006` is 10/10** and the gate `001`–`005` holds at 49/50, the miss being the `Tj`
artefact on `Praktikastellen_…xls` that the brief names.

| | before | after |
| --- | --- | --- |
| documents matching | 127 | **129** |
| documents with an exactly correct page count | 142 | **143** |
| total absolute page error | 117 | **116** |
| `batch-006` | 8/10 | **10/10** |
| every other batch | — | unchanged |

**Exactly five rows changed and none in the wrong direction.** Two went to `match`;
`Foreign_SA-CAT-I_and_CAT-II-III_Pub_0.xlsx` went from 1530 words to 1505 of 1504 and stayed a
match; `RMP 2011-2014 and Inventory.xls` gained the two note pages the reference draws and stayed
a `pages` failure at the same page error of one; and `FAA-2019-0995-0002_attachment_2.xlsx` moved
because the *reference's* word count moved by one between the two sweeps.

### The baseline is five below the brief, and two of the five are the artefact

The brief said 132/171 with page error 119 and 141 exact, measured at `6b6d54d37` plus its fix. At
`5ec407cf3` the two continuous quantities reproduce within two — **117 and 142** — and the match
count is **127**. Ten of the fourteen per-batch figures reproduce exactly; 007, 008, 014 and 018
are each one lower, and every one of the four extra failures is a word-gate verdict on a
page-exact document.

**Two of the four are the `Tj`-splitting artefact and are not regressions**, measured with a
token-length histogram of both text layers rather than assumed — the signature is a hard ceiling
on our side that the reference does not have:

| | ours | reference | tokens over 28 characters |
|---|---|---|---|
| `Praktikastellen_…xls` (005) | max 33 | max 55 | 1 against 19 |
| `FY2021-AIP-grants.xlsx` (014) | max 37 | max 49 | 15 against 92 |
| `STC_WebList.xlsx` (018) | max 89 | max 107 | 734 against 1348 |

So that artefact costs this track **three** matches, not one. The other two —
`CSA_CCM_v1.2.xls` (007) and `SLSA_Directory_031423.xlsx` (008) — are something else: the first
over-counts by 1227 with the same 36-character maximum on both sides, and the second is 187 words
*short*, which splitting cannot cause.

### An unsized DrawingML run is twelve point, and the paragraph around it keeps its own

`SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.xlsx` was page-exact and 31 words short. Its
methodology box has two paragraphs that end with a run stating no `sz`, and two bugs met on that
one character: the default for such a run was 18 pt where LibreOffice gives 12, and the paragraph
took the largest size any run stated, so one stray space re-measured 440 characters of body text.

**Measured against the binary rather than derived**, because both candidates are in the file: a
probe with three text boxes, round-tripped through the flat-ODS export, comes back with 12 pt for
both of its unsized runs, 11 pt for a body followed by an unsized space, and 18 pt for every
shape's *default paragraph style* and no run. 519 words to **548 of 550**, and the probe's three
boxes now break where LibreOffice's own PDF breaks them.

Eleven documents on the track have shape text with runs at all; all eleven were rendered before
and after and **two moved**, the second being
`Foreign_SA-CAT-I_and_CAT-II-III_Pub_0.xlsx` at 1530 → 1505 of 1504. No page count moved.

### A sheet's notes are printed on pages of their own

`Hazard Analysis Template.xls` was 2 pages against 3 with 460 words against 682, and the missing
page is a list of its cell comments — Excel's "Comments: at end of sheet", which the previous
round diagnosed and left as a feature. It is `ScPrintFunc::DoNotes` (`printfun.cxx:1930-2001`) and
short enough to port whole; the order is **column-major**, which the reference shows plainly
(D1, F2, H2, J2, L1, N2, P2, R2).

Reading it needed a join neither record holds: BIFF8's `NOTE` carries the cell and *names* an
object, and the characters are in that object's `TXO`. `ftCmo`'s identifier was being skipped.

**Two of the 171 documents ask for this**, and both are now right where they can be:
`Hazard Analysis Template.xls` is exact at 3 pages and 682 words, and `RMP 2011-2014 and
Inventory.xls` draws both of the reference's note pages with the same marks in the same order.

One instrument note, because this track leans on the trick: **LibreOffice's flat-ODS export drops
cell annotations entirely** — zero `office:annotation` in the `.fods` of a workbook whose notes
plainly print, twenty-four in the `.ods` of the same file. Use `--convert-to ods` for anything
about notes.

### The lead worth taking next: a hard break inside a cell never reaches us

`CSA_CCM_v1.2.xls` is page-exact and 1227 words over, spread evenly across all thirteen pages. The
reference puts `ME 2.1`, `ME 2.2`, `PO 9.5` and `PO 9.6` on four lines of one cell; we draw them
concatenated, and the resulting long string is then broken **mid-token** by the wrap, which is
where the extra tokens come from.

LibreOffice's own `.ods` conversion of that workbook writes the cell as one `table:table-cell`
holding **four `<text:p>` elements**, and holds **1403 multi-paragraph cells** in all; our
extraction of it yields **zero** strings containing a newline. So the break is lost in the BIFF
reader. `SheetTextLayout.Wrap` would drop it anyway — it returns a single line whenever the whole
text fits the column, while `LineCount` beside it splits on `\n` first, so the reserved row height
and the drawn lines are computed by two rules that disagree. The module TODO has the detail and
the warning about estimating its reach by grepping.

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

## The cross-track sweep at `7038ae8c1`: 186 of 188, and one new word-gate artefact

Owed because the last words merge cut the anchor character in the **shaper**, which all three
families share, and nothing had measured that against the other two tracks on one tree.

| Track | Batches | | |
|---|---|---|---|
| `words` | 001–005 | **50/50** | — |
| `slides` | 001–009 | **87/88** | `8_P-Pavese_AIRBUS…pptx` — the known ceiling |
| `sheets` | 001–005 | **49/50** | `Praktikastellen_…xls` — see below |

The shared shaper change is clean: words and sheets hold, and slides' single failure is the
ceiling entry already on the list.

### A URL that draws correctly and extracts as two words

`sheets/batch-005/xls/Praktikastellen_-_chinesischsprachiger_Kulturraum.xls` is **34 pages
against 34** and **2019 words against 1828**. The whole +191 is URL fragmentation: `http://www.`
appears as its own token 48 times in our text layer and never in the reference's.

> **This whole section's conclusion was wrong, and the correction is below the round that made
> it.** It *was* a wrapping defect. `SheetTextLayout.Wrap` dropped a hard break inside the cell,
> the concatenated string then overran its column, and the wrap broke it mid-token — which
> produces a token histogram indistinguishable from an operator-granularity split. The document
> now matches at **1828 of 1828** with nothing about the PDF sink changed. The `Tj` mechanism
> described below is real and was measured correctly; **its attribution to this document was
> not**, and the `· artefact` mark it earned went on to excuse two further genuine defects.
> Left in place rather than deleted, because the reasoning is the instructive part.

**It is not a wrapping defect, and the first two hypotheses were wrong.** The cells state
`no-wrap` (27 of them do, against 7 that wrap) so a wrap would have been a real bug — but
`pdftotext -bbox` settles it: our `http://www.` is reported spanning **51.39–225.70**, which is
the width of the *entire* URL, against the reference's whole string at 52.38–226.41. We draw the
full URL in the right place. The page is right; only the text layer disagrees.

The mechanism is how the two writers emit a long run:

| | text-showing operators | mean glyphs each | longest |
|---|---|---|---|
| ours | 54 `Tj` | 12.8 | **28** |
| reference | 44 `TJ` arrays + 14 `Tj` | 6.0 | 7 |

A `TJ` array carries its kerning *inside* one operator, so a string of any length stays one
show. We emit `Tj` with a `Td`/`Tm` reposition between segments, and **28 glyphs is our
ceiling** — `http://www.europeanchamber.com.cn` is 33 characters, so it spans two operators and
poppler reads the reposition as a word boundary.

This is the word gate's blind spot pointing the other way. The raster ceiling is *our output
being better and scoring worse*; this is our output being **worse** in a way no geometric check
can see — a URL nobody can select as one string. The reference does it better and the corpus
caught it only because a page-exact document's word count moved.

Unfixed. It belongs in the PDF sink every family shares, so it wants its own round and a
cross-track sweep; and since the 28-glyph ceiling is ours, any document with a long unbroken
run is affected, not only this one.

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
| `batch-007` | 10 | 160–185 | doc:4 docx:6 | ✅ |
| `batch-008` | 10 | 186–204 | doc:4 docx:6 | 9/10 |
| `batch-009` | 10 | 208–226 | doc:5 docx:5 | ✅ |
| `batch-010` | 9 | 228–260 | doc:2 docx:8 | 8/9 |
| `batch-011` | 10 | 260–296 | doc:2 docx:8 | 9/10 |
| `batch-012` | 10 | 306–333 | doc:4 docx:6 | 9/10 |
| `batch-013` | 9 | 338–370 | docx:10 | 5/9 |
| `batch-014` | 10 | 372–422 | doc:4 docx:6 | 3/10 |
| `batch-015` | 10 | 424–471 | doc:3 docx:7 | 5/10 |
| `batch-016` | 10 | 473–537 | doc:5 docx:5 | 8/10 |
| `batch-017` | 10 | 537–602 | doc:2 docx:8 | 5/10 |
| `batch-018` | 10 | 620–859 | doc:2 docx:8 | 3/10 |
| `batch-019` | 10 | 956–1521 | doc:1 docx:9 | 3/10 |
| `batch-020` | 10 | 1523–3818 | doc:2 docx:8 | 3/10 |
| `batch-021` | 2 | 4417–4676 | docx:2 | 0/2 |

All twenty-one figures above are the measured sweep at the commit that landed the shrinking
justification, not carried forward. Six of the recorded ones were stale, every one of them
*under*-stating the batch: 008 and 009 at 10/10 rather than 9/10, and 012, 015, 016 and 019
each one or two better than recorded.

### `slides` — 163 documents, 17 batches

Every row below re-proved by a whole-track sweep at `b7950ffd5` **and again after round
eighteen's fix**: **151 of 163** both times, all 163 page counts exact, 0 `ref-failed`.

**Three rows below were stale and are corrected here.** The header said 152 of 163 and marked
`batch-009` and `batch-016` green; rounds seventeen and eighteen both measure 151, with 009 at
9/10 and 016 at 8/10. Round seventeen recorded the correct split in prose and did not carry it
into this table, which is how a scoreboard rots — the prose is read once and the table is read
every round.

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
| `batch-009` | 10 | 1510–1711 | ppt:4 pptx:6 | **9/10** · `NWD-GLA-Community-Outreach-Day-Oct-2025.pptx`, 537 words against 586 — page-exact and the only slides failure that *under*-draws |
| `batch-010` | 10 | 1748–1935 | ppt:3 pptx:7 | 8/10 · ceiling |
| `batch-011` | 10 | 1980–2294 | ppt:1 pptx:9 | ✅ |
| `batch-012` | 10 | 2403–3036 | pptx:10 | 8/10 · ceiling |
| `batch-013` | 10 | 3054–3633 | ppt:3 pptx:7 | ✅ |
| `batch-014` | 10 | 3638–4498 | ppt:2 pptx:8 | 7/10 · ceiling ×2, `a14` fallback ×1 |
| `batch-015` | 10 | 4626–7249 | ppt:4 pptx:6 | ✅ |
| `batch-016` | 10 | 7428–13730 | ppt:1 pptx:9 | **8/10** · ceiling ×1, `prstTxWarp` ×1 |
| `batch-017` | 5 | 14810–32582 | ppt:1 pptx:4 | 4/5 · chart labels — see the correction below |

### `sheets` — 171 documents, 18 batches

Measured whole-track at `22ed440e0`, four times, 171 documents each time. The baseline reproduced
the brief's three headline figures to the digit — **134 of 171, page error 116, 143 exact** — and
after the sixteenth round's three changes: **136 of 171, page error 113, 145 exact.** Batches 001
to 008 are now full parity.

**Seven documents moved and every one improved.** The eighth row that differs between the baseline
and the final sweep is `PBN Matrix NAAs (V01).xlsx`, whose *reference* word count went 5554, then
5557, then 5556 across three runs of the same `soffice` on the same file — a match throughout, and
a reminder that the reference column is not perfectly reproducible either.

| Batch | before | after |
| --- | --- | --- |
| `batch-007` | 9/10 | **10/10** |
| `batch-008` | 9/10 | **10/10** |
| `batch-017` | 4/10 | 4/10 (9/10 → 4/10 mid-round, restored) |
| every other batch | — | unchanged |

**Nothing outside `Paperless.Spreadsheets` was touched**, so the words and slides tracks cannot
have moved and were not swept.

Measured whole-track at `1aefcdfdb`, twice, 171 documents each time. The baseline reproduced the
brief's three headline figures to the digit — **129 of 171, page error 116, 143 exact** — and after
the fifteenth round's one fix: **134 of 171, page error 116, 143 exact, total word error 107 780 →
44 496.** Five documents went to `match`, none left it, and **no page count moved anywhere on the
track**, which is what says the diagnosis was right: `LineCount` had always split on the in-cell
break, so the row heights were already correct and only the drawing was not. `dragon-175066A.xlsx`
went from failing pages *and* words to failing pages alone at 8143 against 8142.

Fourteen documents' word error rose, twelve of them by five or fewer. The two that are not:
`CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` 1227 short became 1316 short, still a `words` failure,
and `afn-afn-20250801-fy25-jan25-mar25.xlsx` 407 over became 449 over on 73 542, still a match.

The paragraph below and the `· artefact` marks it explains are superseded — see "Sheets: a hard
break inside a cell is a line" further down for what those four documents actually were.

Measured whole-track at `5ec407cf3`, twice, 171 documents each time: **127 of 171** before, page
error **117**, **142** exact page counts, and **129 of 171, page error 116, 143 exact** after the
fourteenth round's two fixes. **Exactly five rows changed and none in the wrong direction** — two
documents went to `match`, one went from 1530 words to 1505 of 1504, `RMP 2011-2014 and
Inventory.xls` gained the two note pages the reference draws and keeps its own one-page shortfall,
and one row moved because the *reference's* word count moved by one.

The baseline is five matches below the brief's 132 while its two continuous quantities reproduce
within two of 119 and 141. Ten of the fourteen per-batch figures reproduce exactly; 007, 008, 014
and 018 are each one lower, every one of the four a word-gate verdict on a page-exact document.
**Two of the four are the `Tj`-splitting artefact**, measured with a token-length histogram —
`FY2021-AIP-grants.xlsx` has 15 tokens over 28 characters against the reference's 92, and
`STC_WebList.xlsx` 734 against 1348, the same signature as `Praktikastellen_…xls`. So that artefact
costs this track three matches rather than one, and none of the three is a regression.

Measured whole-track at `6b6d54d37`: **125 of 171**, page error **192**, **134** exact page
counts — the twelfth round's figures to the digit. After the thirteenth round's device fix, at the
same commit plus it: **132 of 171, page error 119, 141 exact.** Thirteen documents improved and
none regressed.

**42 of that 119 is `orbus_togaf_tool_csq.xls`, where the reference prints a sheet the file does
not contain**; see the rounds' entries above before treating the total as work outstanding. What
is left after that is a long tail — the next largest is
`ODs-February-2022-Airbus-Commercial-Aircraft.xlsx` at 18 and still undiagnosed, then two
documents at 6 and the rest at 4 or fewer.

`114339-PROP-P127508-PUBLIC-PPCRNARCPPforPublication.xlsx`, previously flagged as the
row-height overshoot in miniature and worth probing, was exactly that and now matches at 21/21.

| Batch | Files | Score | Mix | Status |
|---|---|---|---|---|
| `batch-001` | 10 | 47–69 | xls:3 xlsx:7 | ✅ |
| `batch-002` | 10 | 69–86 | xls:4 xlsx:6 | ✅ |
| `batch-003` | 10 | 87–116 | xls:5 xlsx:5 | ✅ |
| `batch-004` | 10 | 118–173 | xls:3 xlsx:7 | ✅ |
| `batch-005` | 10 | 173–217 | xls:5 xlsx:5 | ✅ |
| `batch-006` | 10 | 223–249 | xls:3 xlsx:7 | ✅ |
| `batch-007` | 10 | 253–325 | xls:1 xlsx:9 | ✅ |
| `batch-008` | 10 | 328–420 | xls:3 xlsx:7 | ✅ |
| `batch-009` | 9 | 421–540 | xls:2 xlsx:8 | **8/9** |
| `batch-010` | 10 | 560–691 | xls:7 xlsx:3 | **6/10** |
| `batch-011` | 10 | 702–799 | xls:4 xlsx:6 | 6/10 |
| `batch-012` | 10 | 825–995 | xls:1 xlsx:9 | 8/10 |
| `batch-013` | 10 | 1039–1250 | xls:4 xlsx:6 | 7/10 |
| `batch-014` | 10 | 1276–1765 | xls:6 xlsx:4 | **8/10** |
| `batch-015` | 9 | 1773–2264 | xls:4 xlsx:6 | 5/9 |
| `batch-016` | 9 | 2286–4300 | xls:6 xlsx:4 | 4/9 |
| `batch-017` | 10 | 4468–14431 | xls:4 xlsx:6 | 4/10 |
| `batch-018` | 4 | 19384–48127 | xlsx:4 | **3/4** |

**The `· artefact` marks are gone from this track and were never earned.** All four documents
carrying one were failing on an in-cell hard break the drawing dropped, not on the sink's `Tj`
granularity; the fifteenth round's entry below has the numbers. Nothing about the sink changed
and all four now match.

#### The image residue, re-measured on the fixed instrument

Every figure taken before `6b6d54d37` was on a `pdf-image-diff.py` that skipped pages a pixel
apart and counted them as major, so the predecessor's "54 of 125 documents, 348 pages" is not
comparable. On the fixed tool, over the 132 documents that now pass the word gate:
**58 have at least one MAJOR page, 687 pages in all.**

263 of those 687 are `TK-Syllabus-Comparison-Document-v2.xlsx`, which entered the comparison for
the first time this round because its page count only just started matching. Its signature is
uniform across all 263 — *ink missing from ours* spanning the whole table area — which is what the
105 rows out of 7909 that are still a line out look like once they accumulate down a page. So it
is the tail of the row-height work rather than a separate defect.

Of the rest, the plurality hint is still *marks displaced or reshaped* (959 regions), and the
next two are *a solid area drawn differently* (487) and *a fill of the wrong colour* (452). The
first wants the extraction comparison rather than more pixels. The sharpest single case is a
feature and not a defect: `alle einzeln.xlsx`, 36 MAJOR pages of a fill the reference has and we
do not, is a **pivot table**, which LibreOffice lays out itself with its own column widths and
border grid.

## Sheets, round nineteen: a button is on the screen and not on the paper

Whole track swept before anything was changed, 171 documents, two workers, 171 rows, no path twice
and no `ref-failed`. **The brief's two headline figures reproduced and its per-batch figures did
not**, in the way this file now records every round: it gave the track as 138 with page error ~111,
which is right, alongside `batch-009` at 6/9 with `airports_6.xlsx` and
`Company_Seniority_Date_Calculator.xlsx` open — those are round *sixteen*'s numbers, both documents
have matched since rounds seventeen and eighteen, and the base measures `batch-009` at **8/9**. It
also repeated the "fixed 75% scale, so the shortfall is cumulative row height" account of
`airports_6.xlsx` that round seventeen refuted (the fault was horizontal, in font substitution).
Three consecutive rounds have now been described by a brief written before their merge.

| | before | after |
| --- | --- | --- |
| documents matching | 138 | **139** |
| documents with an exactly correct page count | 147 | 147 |
| total absolute page error | 111 | 111 |
| `batch-001`–`008` | 80/80 | **80/80** |
| `batch-009` | 8/9 | 8/9 |
| `batch-010` | 5/10 | **6/10** |
| every other batch | — | unchanged |

**Two rows changed in 171 and neither regressed.** `PC1000.xls` 957 words against 873 → 863/873 and
into parity; `PBN Matrix NAAs (V01).xlsx`'s *reference* word count drifted 5557 → 5555 with ours
unchanged at 5559, verdict `match` on both sides.

That second row is the known LibreOffice non-determinism and it is worth one more line, because it
moved differently in two sweeps of the same change. An intermediate sweep — of a variant that
dropped the object instead of flagging it — had `PBN Matrix` steady and
`TK-Syllabus-Comparison-Document-v2.xlsx` drifting 258368 → 258369 instead. **In both cases only
the reference column moved and ours did not**, on two documents whose verdict was `match` before
and after. So the noise is real, it is not confined to one file, and the way to tell it from a
regression is that our own number is unchanged.

### The flag was read for every object and acted on for eleven types

`ftCmo`'s third field is a flag word and the reader skipped it, so `fPrint`
(`EXC_OBJCMO_PRINTABLE`, `sc/source/filter/inc/xlescher.hxx:228`) never reached the page. Excel
leaves "Print object" off for a button by default, which makes this common rather than exotic.

The rule is narrower than the flag. Calc reads the bit for every object and acts on it only in
`XclImpControlHelper::ProcessControl`, which writes it to the control model's `Printable` property
(`sc/source/filter/excel/xiescher.cxx:1998`); a plain shape with the bit clear is merely traced by
`DoPreProcessSdrObj` (`xiescher.cxx:843-845`) and printed anyway. `PC1000.xls` states both cases in
one file — six Buttons at `0x4001`, a Rectangle at `0x6001` and a Picture at `0x6001`, every one
with the bit clear — and the reference draws the rectangle and the picture and none of the buttons.

Its signature is as clean as this track has produced: **exactly +9 words on each of pages 2 to 9**
of a document whose 13 pages already matched, the nine being two button captions.

**The object is flagged rather than dropped**, because `ScDrawLayer::GetPrintArea` counts every
object and excludes only the hidden-comment layer (`drwlayer.cxx:1395-1424`) — so an unprintable
button anchored past the last cell still widens the printed block. Dropping it would have drawn the
right page and paginated the wrong number of them, which is the failure mode this file already
records under "a change that only ever widens a print area".

### Reach, and the honest second number beside it

**16 of the corpus's 62 binary workbooks carry an unprintable form control**, across batches 005,
008, 010, 011, 012, 013, 014, 016 and 017 — the rule is general. **One of the sixteen moved**: the
others' controls carry no `TXO` text and were already drawing nothing. Both figures belong in the
same sentence, because the first alone overstates the change by an order of magnitude and the
second alone reads like a special case.

**Zero of the 109 corpus SpreadsheetML documents state `fPrintsWithSheet="0"` or a VML
`<x:PrintObject>False</x:PrintObject>`**, so the OOXML side is left unimplemented rather than
written blind.

### `RegChangeReport.xlsx`: round eighteen's measurements hold, two of its conclusions do not

Both figures reproduce exactly — 3060 words against 3137 with page 6 alone at −72, and the
reference embedding Carlito on all twelve pages where we embed none.

**It is a page break, not a missing clip-and-redraw.** `DrawCoveredMerge` already redraws a
straddling merge from its true origin and its own remark cites this workbook; the band at the top
of the reference's page 6 is **row 135 alone**, which is 95.25 pt tall. LibreOffice ends page 5 at
row 134 and we fit row 135 onto it, so our page 6 has no covered cell to reach back from. Off
LibreOffice's own flat-ODF export: page 5 starts at row 100 on both sides, rows 100–134 sum to
586.90 pt and 100–135 to 682.14 pt, and a greedy pagination over those heights reproduces every one
of LibreOffice's breaks for a body height in **[681.62, 682.14)**. Ours admits 682.14. The exported
page geometry does **not** give that number — margins 0.75 in and 0.3 in with a 0.45 in footer and
0.311 in of footer spacing come to a 661.6 pt body, and 661.6 breaks page 5 at row 133, which is not
what the reference draws. That contradiction is the thing to measure first next round.

**The steady −1 a page is the `&<size>` code, not the unhonoured `&"Calibri"`.** Both sides draw
the footer's `_x000D_`, its `#` and its sentence; `&1` puts the reference's `#` on its own baseline
(y 766.70 against 758.10 on page 5) so poppler counts it separately, and we draw the part at one
size so `wc -w` reads `_x000D_#` as one word. Honouring the size is worth +12 across the document,
taking −77 to −65 against a 2% band of 62.7 — still short. Only page 6's band closes it.

## Sheets, round eighteen: an empty row band is not a page the zoom search can count

Whole track swept before anything was changed, 171 documents, two workers, no path twice and no
`ref-failed`. **The brief did not reproduce, and the reason is worth recording**: it handed over
round *sixteen*'s figures — 136 of 171, page error 113, 145 exact, `batch-009` 6/9 — while round
seventeen was already merged at the briefed commit `54729fdc7`. The base measured
**137 of 171, page error 112, 146 exact, `batch-009` 7/9**, which is round seventeen's result to
the digit. It also meant `airports_6.xlsx` already matched and the brief's account of it (a fixed
75% scale, so a cumulative row-height shortfall) had already been refuted upstream as a horizontal
fault in font substitution.

| | before | after |
| --- | --- | --- |
| documents matching | 137 | **138** |
| documents with an exactly correct page count | 146 | **147** |
| total absolute page error | 112 | **111** |
| `batch-001`–`008` | 80/80 | **80/80** |
| `batch-009` | 7/9 | **8/9** |
| every other batch | — | unchanged |

**Two rows changed in 171 and neither regressed.** `Company_Seniority_Date_Calculator.xlsx` 13
pages against 12 → 12/12 and into parity; `SIL_TDB648.xlsx`'s *reference* word count drifted
7678 → 7679 with ours unchanged at 7680 and its verdict `pages` on both sides.

### The count the fit-to-page search bisects on excludes empty bands

`PrintPageRanges::calculate` increments `m_nPagesY` only for a row band `IsPrintEmpty` is false
across (`printfun.cxx:3176`, `:3220`), so it is not the number of bands the geometry produces, and
`ScPrintFunc::CalcZoom` compares the smaller number everywhere — the tdf#103516 nudge included.

`Company_Seniority_Date_Calculator.xlsx` states a print area of `A1:Y49` on a sheet whose last
`<row>` element is 48. Fitting to width gives zoom 80; the nudge tries 78, where the rows split
into 1–48 and the empty row 49 — one page to Calc, two to us — so the nudge saw an unchanged count,
was abandoned, and the sheet printed at 80, spilling a thirteenth page holding one row. The
seventeenth round had measured this to the nudge and distrusted its own arithmetic, suspecting the
row sum, the printable height or the band's end row. **All three were right**; the missing input was
the empty-band rule.

### `RegChangeReport.xlsx` narrowed to one visible band, and a second defect found beside it

The long-standing words-only failure is **3060 against 3137**, and **page 6 alone is −72** of that.
The reference's page 6 opens with a yellow-filled, bordered continuation band holding the tail of
the row above; we omit it and draw that text at the foot of page 5, where the reference clips it
away and re-emits it. A tighter line pitch would explain the same symptom and is **refuted**: 46
lines on page 5 on both sides, dominant pitch 10.1 pt against 10.0. So the cause is that we do not
clip a cell at the page boundary and redraw the remainder on the next page — a feature, and the
same family as the reverted `CRFlags::ManualSize` work.

Separately, the reference embeds **Carlito on all twelve pages** and we embed none. Every font in
the workbook's `styles.xml` is Arial, so Calibri can only come from the footer code
`&"Calibri"&10` — the footer's stated font is not honoured. That is the steady −1 a page beside
page 6's −72.

## Sheets, round seventeen: the substitution chain is not what the binary follows

Whole track swept before anything was changed, 171 documents, two workers. **The brief reproduced
to the digit** — 136 of 171, total absolute page error 113, 145 documents with an exactly correct
page count, `batch-001`–`008` at 80/80, and every per-batch figure as handed over: 009 6/9,
010 5/10, 011 6/10, 012 8/10, 013 7/10, 014 8/10, 015 5/9, 016 4/9, 017 4/10, 018 3/4. 171 rows,
no path twice, no `ref-failed`.

| | before | after |
| --- | --- | --- |
| documents matching | 136 | **137** |
| documents with an exactly correct page count | 145 | **146** |
| total absolute page error | 113 | **112** |
| `batch-001`–`008` | 80/80 | **80/80** |
| `batch-009` | 6/9 | **7/9** |
| every other batch | — | unchanged |

**Two rows changed in 171 and neither regressed.** `airports_6.xlsx` 18 pages against 17 → 17/17
and into parity; `ans_mappings_of_eccairs_terms.xlsx` one word closer on 28 183.

### `airports_6.xlsx`: `Helv` is not Liberation Sans, whatever VCL.xcu says

The handover called this one "a fixed 75% scale, so the shortfall is cumulative row height, not
zoom — a cleaner test of the row model". **The measurement reproduces and the explanation was
wrong, in the way this file keeps recording.** The row model is not involved at all: our dominant
row pitch is 8.99 pt on both sides, and the fault is horizontal.

`pdftotext -bbox` on both PDFs, page 1: the reference draws column B's text from x = 130.26 and
column C's from 149.97; we draw them from 120.73 and 137.98, with the same 0.73 pt cell margin on
both sides. That makes our column A 1760 twips against the reference's 2014 and our column B 460
against 526 — **the same 0.874 ratio on every column**, which is 111 twips a character unit
against 127. LibreOffice's own flat-ODF export gives **127.0 twips** for all eight stated columns.
111 is Liberation Sans at 10 pt and 127 is DejaVu Sans at 10 pt, and the workbook's `fonts[0]` —
which is what `StylesBuffer::getDefaultFont` hands the unit converter — is **`Helv` at 10 pt**.
Being 12.6% narrow, column C wrapped to two lines where the reference draws one, and the extra
line height accumulated into an eighteenth page. The reference PDF embeds **DejaVu Sans alone**;
ours embedded DejaVu Sans and Liberation Sans.

### The chain is dead code on Linux, and that is in the source

`PhysicalFontCollection::FindFontFamily` asks the pre-match hook at
`vcl/source/font/PhysicalFontCollection.cxx:1142` and returns its answer at `:1151`.
`ImplFontSubstitute` — the `SubstFonts` list from `officecfg/registry/data/org/openoffice/VCL.xcu`,
which is what `FontSubstitutions.Tables.cs` is generated from — is only reached in the *second*
loop at `:1180`. On Linux the hook is `FcPreMatchSubstitution::FindFontSubstitute`
(`vcl/unx/generic/font/fontsubst.cxx:98`), which asks fontconfig about every request that is not
symbol-encoded, and fontconfig always answers — with its own default family for a name it has no
rule for. So **for an uninstalled, non-symbol family the chain never runs.**

### The probe, because the source is not the binary

A flat-ODS probe naming **all 296 families the whole 534-document corpus mentions**, each row
drawing `Hamburgefonstiv` and `0123456789` in that family, rendered by LibreOffice 24.2.7.2 and
read back with `pdftotext -bbox`. The two drawn widths name the face outright — the eight
installed faces are more than a point apart on both — so this is a gold table of what the binary
actually does, and it is worth keeping rather than re-deriving. It is
`scratchpad/sheets-r17/lo-faces.tsv`, built by `mkprobe2.py` and `facewidth2.py` beside it.

**Our resolver agrees with it on 270 of the 293 families it could name.** Two of the 23 are acted
on this round — `Helv` and `SansSerif`, where the chain reaches an installed face fontconfig would
not choose and the family is Latin and not symbol-encoded. The other 21 are recorded and
deliberately left:

| Family | ours | LibreOffice | why it is not acted on |
|---|---|---|---|
| `Wingdings`, `Wingdings 2`, `Wingdings 3`, `Webdings` | OpenSymbol | DejaVu Sans | **the probe is the wrong instrument.** ODF states no charset, so the request was not symbol-encoded and the hook did not bail at `fontsubst.cxx:101` as it does for a DOCX or XLSX font carrying `charset="2"`. Our OpenSymbol answer is probably right |
| `MS Gothic`, `MS PGothic` | IPAGothic, IPAPGothic | DejaVu Sans | fontconfig answers by *character*, and the probe asked in Latin |
| `Book Antiqua`, `Bookman Old Style`, `Century`, `Century Schoolbook`, `NewCenturySchlbk`, `CG Times`, `Times-Roman` | DejaVu Serif / Liberation Serif | DejaVu Sans | needs `ClassOf` replaced by fontconfig's own classification — `45-latin.conf` files these as sans where VCL.xcu's `FontType` says roman |
| `Lucida Console` | DejaVu Sans Mono | DejaVu Sans | same |
| `Palatino Linotype`, `SimSun`, `ＭＳ 明朝` | DejaVu Sans | DejaVu Serif | same, in the other direction |
| `Nimbus Sans L` | DejaVu Sans | Liberation Sans | a metric alias our chain does not name |
| `Times New Roman CE`, `Times New Roman CYR`, `TimesNewRoman,Bold` | DejaVu Sans | Liberation Serif | name canonicalisation — LibreOffice's `GetEnglishSearchFontName` strips the suffix and `Normalise` does not |

**That is the next round's work on this, and it is not a sheets change**: 91 corpus documents name
at least one of these families, 63 of them slide decks and 24 word processing. Whoever takes it
should sweep all three tracks, and should not take the four symbol rows from this probe.

### What it costs the other two tracks: three documents in 534, and one of them by a word

The change is in a shared layer, so words and slides are owed. **A whole-track baseline is not the
sharpest instrument here and was not the only one used**, because the change is exactly one thing:
`ChainFor` returns nothing for two keys. A document that never asks for `Helv` or `SansSerif`
cannot move, and that is a property of the file rather than a guess about the code. So the 40
corpus documents naming either family — 16 sheets, 17 words, 7 slides — were rendered on both
binaries and joined to the reference column of the sweeps:

```
    pages b/a/ref        words b/a/ref
   18/17  /17       11077/11043 /11053   better   sheets/batch-009/xlsx/airports_6.xlsx
  192/192 /191      28196/28195 /28183   better   sheets/batch-015/xlsx/ans_mappings_of_eccairs_terms.xlsx
   21/21  /23        4124/4123  /4142    a word   words/batch-017/doc/762.doc
```

**Three of the 40 moved; the other 37 have an identical page count and word count** — which is
what was measured, and is weaker than "identical output". The words document moves by one word
inside a 2% band it already passed, on a document failing on pagination in both binaries — its
verdict is `pages` before and after. Nothing regressed anywhere.

Whole tracks were then swept anyway, at the changed binary. The figures are absolutes rather than
deltas, since neither track had a baseline at this commit and neither was re-swept without the
change: **words 147 of 200, page error 119, 156 page-exact**; **slides 152 of 163, total absolute
page error 0, all 163 page counts exact.** Both are 163 and 200 rows with no path twice and no
`ref-failed`.

Test counts, each project run on its own and the whole output kept: Core 243, Text **234**
(was 217; the seventeen new ones are this round's three positive and fourteen negative resolution
theories), Containers 109, Rendering 104, Markup 259, OpenDocument 125, WordProcessing 565,
Spreadsheets 426, Presentations 505, Vector 291, Fidelity 538. **Zero failed and zero skipped
throughout** — every count is the known-good figure except Text, which is the only project the
change touches.

## Sheets, round sixteen: the attribute scan is asked per column, and it stops twice

Baseline swept whole at `22ed440e0`, 171 documents, and it reproduced the brief exactly —
**134 of 171, page error 116, 143 exact**, batches 001 to 006 at full parity, `batch-007` and
`batch-008` each 9/10 with their single misses being the two documents the handover named. Both
named leads were then taken; one is fixed and the other is measured to its cause and deliberately
not fixed.

| | before | after |
| --- | --- | --- |
| documents matching | 134 | **136** |
| documents with an exactly correct page count | 143 | **145** |
| total absolute page error | 116 | **113** |
| `batch-001`–`batch-008` | 78/80 | **80/80** |

### `batch-008`: an empty column's scan starts at its own last data row

`SheetDecorationArea.Extend` started every column's attribute scan at the *sheet's* last data row.
Calc asks per column and answers per column: `ScColumn::GetLastVisibleAttr` passes that column's
own `GetLastDataPos()`, "0 if none" (`sc/inc/column.hxx:892-897`).

On `Computer and Software Services_50 State Comparison.xlsx` columns I to O carry a solid fill on
all 129 rows and hold no data, and the sheet's data stops at row 42. Below row 42 the fill is one
run of 112 equal rows, past `SC_VISATTR_STOP`, so the old start found nothing at all. What Calc
takes is the **header row above the data** — `nAttrStartRow` is clamped to `nLastData + 1`
(`attarray.cxx:1961`), so a run that is nothing but row 1 measures **zero** rows. The print area
ended at column H where Calc's reaches O, a whole third column band. **26 pages now against 26.**

The handover cited this mechanism and said in as many words that it was not measured to be the
cause. It was, and the citation was right — which is worth recording, because on this project the
usual outcome is the opposite.

### The same change cost nine pages elsewhere, and that is why the round has three sweeps

An intermediate whole-track sweep moved **exactly four rows**, two of them the reference's own word
count drifting. The other two were the target above and
`environment-edb-docs-edb-emissions-databank.xls`, **195 pages to 204** and out of `match`.

Probed, not guessed: its `ICAO databank` sheet holds data to column 104 and per-cell formatting to
column 228, and the printed range came back `0..228` where it had been `0..104` — 124 empty ruled
columns the old start had never been able to reach. Calc drops them by `SC_COLUMNS_STOP` = 30
(`table1.cxx:655`, `737-757`), the sideways twin of `SC_VISATTR_STOP`, which was not implemented at
all: the walk groups columns visually equal over every row, and the first group of thirty or more
ends the block before it. On an ordinary sheet the run past the last formatted column is unbounded
and equal to itself, so the walk stops exactly where it did before — which is why nothing had ever
needed it, and why implementing it changed nothing else on the track.

The lesson is procedural. **A change that only ever widens a print area cannot be checked on the
documents it was aimed at**; the cost is entirely on documents whose print area was already right,
and it took a whole-track sweep to see one.

### `batch-007`: `dragon-175066A.xlsx` was one twip of digit width

14 pages against 13 with the words exact. Measured off both PDFs with `pdftotext -bbox`: column B
starts at 80.528 pt in the reference and 81.153 in ours, column C at 357.0 and 363.1, so column B
is drawn 276.47 pt wide against 281.95. LibreOffice's own flat-ODF export gives column B as
`10.3783in` = 747.24 pt, which makes the scales **0.3700 and 0.3773** — zoom **37** against **38**.

The sheet is `fitToPage` with `fitToWidth` 1 and `fitToHeight` 0, and `ScPrintFunc::CalcZoom`
bisects on **integer** percentages, so the exact fit ratio only has to cross a whole percent for
the answer to move by 2.7% and take a page with it.

It crosses because our columns are 0.7% narrower. The workbook's default font is 宋体 at 11 pt,
which fontconfig resolves to DejaVu Sans — LibreOffice's too, since its export gives exactly 140.0
twips per character unit for all three columns. Our exact metric is **139.9701** and
`SheetFonts.DigitWidthTwips` truncates it to 139. **Forcing 140 for this workbook renders it 13 of
13 at 8143 words against 8142.**

The truncation is not a new defect: it is a rule that section already documents and justifies,
because Carlito 11 pt measures 111.50 where LibreOffice writes 111 and Carlito is the default of
65 of the 171 corpus spreadsheets. What is new is the price. A one-twip column width is normally
invisible; a fit-to-width sheet's integer zoom turns it into a page.

**Changed, and on the corpus rather than on the faces.** All nine faces the record measures are
satisfied by truncating unless the fraction carries past a threshold somewhere in (0.64, 0.70] —
Carlito 12 pt is 121.64 → 121, DejaVu Sans 12 pt is 152.70 → 153 — but only two of the nine
discriminate and there is no mechanism behind the constant, so on its own that is a fudge fitted
to two points. The corpus is what settles it, and the measurement is cheap because **the reference
side does not move for a Paperless-only change**: render our side alone over all 171 and join the
existing reference column. The harness was checked first by reproducing the previous sweep's
135/114/144 with *zero* rows differing, and the change then gave **136, 113, 145** with six
documents moved and every one improved — `dragon-175066A.xlsx` 14 pages to 13 and into parity, and
five word counts closer to the reference, four of them exact. A confirming full sweep against a
freshly generated reference gave the same three numbers.

The reach is every spreadsheet in the corpus and six of the 171 felt it, which is also the answer
to how far a grep would have overstated it.

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

## Words, the paragraph-spacing rule: four formats measured, zero corpus movement

The brief for this round said LibreOffice adds a paragraph's `w:after` to the `w:before` below
where we take the larger, gave a fixture measuring 20 pt against our 12, and named this a
vertical-budget rule that moves every page at once — "plausibly the largest single win left on
this track *or* a large regression".

It is neither. Swept whole before and after, on 200 documents:

| | before | after |
|---|---|---|
| match | 146/200 | **146/200** |
| exactly correct page count | 154 | **154** |
| total absolute page error | 120 | **120** |
| total absolute word error | 7110 (3200 short, 3910 over) | **7110 (3200 / 3910)** |
| documents whose page or word count moved | — | **0** |

Not "no net change": **no document changed at all**, in either number. That is the right result
and it was predictable before the sweep, which is the part worth keeping. Three sweeps, not two:
the second half of the fix landed after the "after" sweep started, so a third was run against the
final binary and reproduces the same four figures and the same zero.

The change is confined to `Paperless.WordProcessing`; nothing in `Paperless.Presentations` or
`Paperless.Spreadsheets` references `PaginationOptions` or `WordProcessing.Layout`, and
`Paperless.Text/Layout/ParagraphLayouter.cs` is untouched, so the other two tracks cannot have
moved.

### The measurement reproduced and the sentence attached to it was wrong, again

The fixture reproduces to the digit. Eight paragraphs each carrying 12 pt of space-before and
8 pt of space-after on 12 pt exact lines gives boundaries of 24 pt when the spacings collapse and
32 pt when they add, and LibreOffice 24.2.7.2 puts them at 32 where we put 24.

The claim attached to it — that `writerfilter` never sets `PARA_SPACE_MAX` for DOCX and keeps the
application's `AddSpacing`, which is on — is wrong twice over.
`DomainMapper_Impl::ApplySettingsTable` **does** set it, unconditionally within the method, from
`w:doNotUseHTMLParagraphAutoSpacing` (`DomainMapper_Impl.cxx`:10179). What it cannot do is run at
all when the package has no settings part, because the method returns at its first line on a null
settings table (:10124). **The fixture had no `word/settings.xml`** — the exact trap
`PaginationOptions.KeepsSpacingAtTopOfPage`'s own remarks already warn about, one file away from
where the claim was written.

Thirteen probes, every combination the four formats offer, ours against LibreOffice's:

| probe | ext | LibreOffice | before | after |
|---|---|---|---|---|
| no `word/settings.xml` | docx | 32.0 | 24.0 | 32.0 |
| empty `w:settings` | docx | 24.0 | 24.0 | 24.0 |
| `compatibilityMode=15` only | docx | 24.0 | 24.0 | 24.0 |
| `w:doNotUseHTMLParagraphAutoSpacing` | docx | 32.0 | 32.0 | 32.0 |
| `AddParaTableSpacing=false` | fodt | 24.0 | 32.0 | 24.0 |
| `AddParaTableSpacing=true` | fodt | 32.0 | 32.0 | 32.0 |
| `AddParaTableSpacing=false` | odt | 24.0 | 32.0 | 24.0 |
| `AddParaTableSpacing=true` | odt | 32.0 | 32.0 | 32.0 |
| no `\htmautsp` | rtf | 32.0 | 32.0 | 32.0 |
| `\htmautsp` | rtf | 24.0 | 32.0 | 24.0 |
| `\htmautsp0` | rtf | 24.0 | 32.0 | 24.0 |
| converted from the collapsing DOCX | doc | 24.0 | 24.0 | 24.0 |
| `fDontUseHTMLAutoSpacing` set | doc | 32.0 | 32.0 | 32.0 |

Four real defects, none of them the one predicted, and **DOC was already right in both
directions**. `\htmautsp0` is worth singling out: LibreOffice declares `htmautsp` a
`RTFControlType::FLAG` (`rtftokenizer.cxx`:701), so the parameter is ignored and `\htmautsp0` asks
for collapsing exactly as `\htmautsp` does. A reader that wrote `Parameter != 0` — the obvious
thing, and what an unmeasured prior attempt at this had written — gets that row backwards.

### Why the reach is zero, and how that was known before the sweep

Every one of the 200 documents was classified by what it says about this rule:

```
 130  docx, settings part, no flag      collapses — already correct
   4  docx, settings part, flag on      adds      — already correct
  66  doc                               from its Dop — already correct
   0  docx with no settings part
   0  rtf stating \htmautsp
   0  odf of any kind
```

The skill's rule is that counting files which *state* a setting overestimates reach by an order of
magnitude. This is the other end of the same instrument and it is much cheaper: when the count of
files that could possibly be affected is **zero**, no rendering is needed to know the answer. Half
an hour of scanning replaced two sweeps' worth of suspense, and the sweeps then confirmed it
exactly.

Keep the fix anyway. It is five fixtures and six tests, the four defects are real, and the
no-settings-part case is specifically the one that makes a synthetic fixture answer this question
backwards — which has now cost two rounds. It also has reach where it was written down: **three of
the 56 DOCX in our own committed corpus have no settings part** — `contextual-spacing-styles.docx`,
`paragraph-shading.docx` and `alt-chunk.docx` — and all three were being laid out under the wrong
rule. None of their assertions depended on it, which is why the suite went from green to green, but
the sentence that made this an "open defect" was written against the first of them.

### A second defect, found by the test rather than by the probe

The probe compares *pitches*, and a pitch comparison is blind to an error the whole page shares.
Adding the fixtures to `ParagraphLeadingComparisonTests`, which compares *absolute* baselines
against LibreOffice, failed both ODF cases on line 1 alone: 81.600 pt against 93.601 pt, exactly
the 12 pt of space-before that the first paragraph on a page is entitled to.

`PARA_SPACE_MAX_AT_PAGES`, which `ww8par.cxx` sets on the line after `PARA_SPACE_MAX` (:1946 and
:1947) and ODF spells `AddParaTableSpacingAtStart`. `PaginationOptions` already models it and the
Word preset already sets it; the ODF path took the bare default, which is `false`, and the ODF
answer is `true`. Measured, first baseline down an A4 page: `true` 93.60, `false` 81.60, **item
removed entirely 93.60**. Absent means true — the reverse of what an absent DOCX settings part
means, so the two formats really do disagree about what silence is.

That is the general lesson from this round: **a probe that compares differences cannot find an
error in the offset**, and the two instruments were half an hour apart in cost.

### Where the four formats now stand, complete

| | spacings add | space kept at top of page |
|---|---|---|
| DOCX | `w:doNotUseHTMLParagraphAutoSpacing`, or **true with no settings part** | true (writerfilter never sets it; the application default) |
| DOC | `fDontUseHTMLAutoSpacing`, defaulting true on a short `Dop` | true (`ww8par.cxx`:1947, unconditional) |
| RTF | true unless `\htmautsp`, whatever its parameter | true (the application default) |
| ODF | `AddParaTableSpacing`, defaulting **true** | `AddParaTableSpacingAtStart`, defaulting **true** |

### What the 54 remaining failures look like at `52f45c51b`

```
by format     doc   54/66 match, 18% fail
              docx  92/134 match, 31% fail

page delta    -31 -13 -12  -4  -2  -1  ±0  +1  +2  +3  +4  +5
(failures)      1   1   1   1   3  14   8  16   6   1   1   1
```

Two things in that worth acting on.

**The format split has stopped being equal.** An earlier round measured 42% `doc` against 43%
`docx` and concluded, correctly then, that the failures were downstream of the readers and that
splitting two agents by format would put them in one file. That is no longer true: `doc` now
fails at 18% and `docx` at 31%, so the shared layout has improved out from under that finding and
what is left leans DOCX-side. **Re-run the one-line check before reusing the conclusion.**

**Thirty of the 54 are off by exactly one page**, 14 under and 16 over — the same near-symmetry
earlier rounds found, and the same argument that the two signs are one quantity rather than two
clusters.

**Eight fail on words with the page count already right**, and they now lean the other way from
the record: `xx_SETIS_PWS_template_10.19.22.docx` at −541 and two small shortfalls against five
documents drawing more than the reference, the largest being `UG.CAO.00133` at +245 and
`ABCD-FE-01-00 Flight Envelope` at +227.

That last one matters beyond its own row, and it is corrected in `TODO.raster-ceiling.md`:
`UG.CAO.00133` is the document that file uses to show that a flagged page does not excuse its
document, on the strength of it running **225 words short overall**. It now runs **245 words
over**, with its four flagged pages accounting for only +105. The point stands and the example has
inverted, which says a document must be re-measured before its flagged pages are subtracted from
it.
## After the tenth round: slides, four fills the gate cannot see

Whole slides track swept before at `52f45c51b` and after at the head of this branch, 163
documents each time, rendering **our** side twice and reusing the reference PDFs — the
reference cannot change when nothing touches `soffice`, and the two sweeps' reference
columns are identical row for row, which is the check that says so.

| | baseline | after |
|---|---|---|
| slides matching the gate | 152/163 | **152/163** |
| pages with a correct count | 163/163 | 163/163 |
| documents whose page count, word count or verdict moved | — | **0** |
| total unaccounted ink, 163 documents | 2564.46 | **1857.62** |
| pages the image tool calls major | 662 | **582** |
| documents whose ink moved | — | **21: 17 down, 4 up by 2.16 between them** |

The baseline reproduced the briefed 152/163 exactly — same eleven failures, no `ref-failed`,
every page count exact — which is the tell that the base and the instrument are both right.
**Nothing the word gate can see moved, in either direction.** That is the expected shape:
every one of the eleven remaining failures is attributed, and all four fixes are fills.

```
247.12 ->  11.10   Course Selection 2025-26 Current Grade 09.pptx   major  9 ->  4
198.24 ->  45.05   171128IPAP.pptx                                  major 32 -> 18
139.10 ->  16.84   2014BSA_Sunday_Killion.pptx                      major 16 ->  7
123.54 ->   8.34   section_1_our_rights_presentation.pptx           major 19 ->  2
 27.75 ->   2.09   5b_upasana_dasgupta_-_liability_and_registration  major 15 ->  1
388.17 -> 368.41   NAS-Infrastructure-Roadmaps-v16.0.pptx           major 85 -> 77
 75.22 ->  66.12   N2_E_Maestroni_Swarm_COP.pptx                    major  5 ->  1
 34.26 ->  25.99   ghgp-supply-chain-initiative_20100323_wri.pptx    major 14 -> 11
```

The four that rose are `OnTrac_StarCertificationProgram-3Day.pptx` (+1.82) and three under
0.2. OnTrac's page 9 is **visually closer** after the change and scores worse: a spurious
black drop shadow behind its form panel is gone, which is what the reference shows, and the
512-pixel region metric moved the other way. Small figures on this instrument are noise.

### Nine documents this instrument cannot measure at all

`pdf-image-diff.py` refuses a page whose two renderings differ in size, and nine of the 163
render to 512×288 on one side and 512×289 on the other — a rounding difference on a 16:9
deck, not a page-size defect. Every page of those nine is skipped, so they contribute
nothing to either total and a change to them would be invisible here. Worth fixing in the
tool before the next round leans on it further.

### `p:sp/@useBgFill` — a shape filled with the slide's own background

The string appeared nowhere in `dotnet/src`. PowerPoint's Designer writes a full-slide
`<p:sp useBgFill="1">` that states no fill and carries a `p:style` whose
`<a:fillRef idx="1">` names `accent1`, so a reader that walks past the attribute paints the
whole slide in the accent colour. On `Course Selection 2025-26 Current Grade 09.pptx` that is
nine of ten pages solid orange against a white reference — 247.12 of unaccounted ink on a
ten-page deck, the second largest figure on the track.

The precedence is `oox/source/ppt/pptshapegroupcontext.cxx:109-113`, which sets the shape's
own fill type to `XML_noFill` **before** parsing its children: a fill the shape states for
itself still wins, and the theme's `a:fillRef` no longer reaches it. `fillproperties.cxx:439`
then hangs `FillUseSlideBackground` on exactly the `noFill` branch, so an explicit
`a:noFill` beside the attribute shows the background too rather than nothing.

Reach, parsed: 19 shapes across 4 of the 112 corpus `pptx` decks.

### Moving a placeholder does not cut it off from what it inherits

`PlaceholderProperties` was consulted only when the slide's shape stated no `a:xfrm`. That
reads "a placeholder that overrides its position inherits nothing else", and LibreOffice does
the opposite: it applies the shape reference on `p:nvSpPr`, before `p:spPr` is parsed at all
(`oox/source/ppt/pptshapecontext.cxx:157-162`), and merges the referenced shape's fill, line
and geometry underneath the shape's own (`shape.cxx:2816-2843`).

`171128IPAP.pptx` is the clean case: its slide titles state their own `a:xfrm` and take a
`C00000` plate from the layout, so most of its forty pages drew **white title text on white
paper** — text that extracts perfectly and cannot be read. 198.24 → 45.05, major 32 → 18.

### A table naming no style gets no style, not the list's `def`

`tableStyles.xml`'s `def` attribute reads exactly like the deck's default table look, and
PowerPoint applies it. LibreOffice does not: `TableProperties::getUsedTableStyle`
(`oox/source/drawingml/table/tableproperties.cxx:89-124`) searches the list only when
`a:tableStyleId` is non-empty and otherwise returns a `static TableStyle` with nothing in it.

Measured rather than read off, because the source alone does not say which renderer to
follow. Page 8 of `section_1_our_rights_presentation.pptx` is a three-column table with
`firstRow`, `firstCol` and `bandRow` set and no `a:tableStyleId`: the reference leaves its
first column white and we filled it `accent1` with white text on it. **Putting the id of the
style the package declares as `def` into that `a:tblPr` makes the reference draw exactly what
we drew** — the fallback is the whole of the difference. 123.54 → 8.34, major 19 → 2.

Reach, parsed: 130 such tables across 24 decks, including 44 in
`NAS-Infrastructure-Roadmaps-v16.0.pptx`. `DrawingTableStyle` lives in `Paperless.Ooxml`,
which all three families share, but `DrawingTableStyle.Read` has exactly one caller —
`PptxSlideLayout` — so no other family's rendering can change.

### A picture frame's fill, and the one thing about it that is still unresolved

Impress adds a picture frame's fill to the shape's decomposition only when the graphic is
transparent (`svx/source/sdr/primitive2d/sdrgrafprimitive2d.cxx:41-42`). The salvaged patch
from the killed agent said a metafile does not count; that reproduced, and the sweep then
found where it does not hold.

Measured against the binary, four renderings of `2014BSA_Sunday_Killion.pptx`. Its slide 5 is
a `p:pic` over `image10.emf` stating `<a:solidFill><a:schemeClr val="tx1"/>` — white, under a
colour map sending `tx1` to a `dk1` of `#FFFFFF`. Page 5 comes back **byte-identical** with
that fill white, red, or replaced by `<a:noFill/>`, showing the slide's own background inside
the frame. A fourth, with the EMF swapped for a PNG whose right half is clear, draws the red
through that half: the rule is about the graphic's kind, not about coverage. 139.10 → 16.84.

**The checked-out source predicts the opposite** — `ImpGraphic::isTransparent` returns true
for everything but an opaque bitmap — so this is measured, not ported.

**Where it does not hold, and why that is not settled.** The first sweep of this rule improved
nine documents and worsened two, both `.ppt` and both on `TODO.raster-ceiling.md`:
`Thailand17.ppt` page 8 and `W3_Case_Study…Ed.ppt` page 10 draw a table on a white plate under
an Escher metafile blip, and suppressing it cost 8.44 and 9.33. Across five measured cases the
line falls on **how the metafile is stored**: the same 892-byte EMF inlined as
`office:binary-data` in a flat ODP keeps its fill, the same deck zipped with it under
`Pictures/` loses it, and a `.ppt` blip is inline by construction. That fits LibreOffice
building a `GDIMetaFile` for one and a `VectorGraphicData` for the other.

A second explanation fits the same five and could not be ruled out: those two `.ppt` pages are
pages where the reference *rasterises* the metafile, and a raster with a soft mask is
transparent by `isTransparent`'s own rule. Separating them needs a rasterised metafile that is
**not** inline and whose frame states a fill; the corpus has none — the four candidates on
`8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` all state `a:noFill`. Recorded as unresolved.

### What the next agent on this track should take

```
368.41  77 pages major  NAS-Infrastructure-Roadmaps-v16.0.pptx   (linked Excel.Sheet.12 OLE, known)
127.20  12              HENTZEN_...AEROSPACE_INDUSTRY.pptx       (the missing a:duotone, known)
 66.45  24 of 24        Framing Europe.ppt                       (unlooked at — every page major)
 66.12   1 of 30        N2_E_Maestroni_Swarm_COP.pptx            (one page, and it is the Gantt)
 56.67  10              Wildlife for REDAC September 11.pptx     (the circle-gradient case, known)
 49.65  18              Thailand17.ppt                           (raster ceiling on p8; 17 others)
 48.41   6 of 268       Reporting_responsibilities_matrix.pptx
 45.05  18 of 40        171128IPAP.pptx                          (residue after this round's fix)
 44.78   8 of 8         redac-fullComm-201705-EE-FRs-briefing.pptx (unlooked at)
 40.64  19 of 20        joint_user_outcomes_michael_fullerton_29.06.12.ppt (unlooked at)
```

`Framing Europe.ppt`, `redac-fullComm-201705-EE-FRs-briefing.pptx` and
`joint_user_outcomes_michael_fullerton_29.06.12.ppt` have never been looked at and are all
"every page major", which on this track has three times now meant one wrong fill repeated
rather than ten separate problems.

## Slides, round eleven: the ink baseline re-measured on the fixed tool

`pdf-image-diff.py` used to refuse a page whose two renderings rounded to 512x288 and
512x289 and skip it; nine of the 163 slides documents were unmeasurable for that reason
alone. The tenth round's headline — 1857.62 unaccounted ink, 582 major pages — was taken
with them in it.

Swept whole at `6b6d54d37` on the fixed tool, 163 documents, two workers:

| | round 10 | this baseline |
|---|---|---|
| documents matching the word gate | 152/163 | **152/163** |
| pages with a correct count | 163/163 | **163/163** |
| documents the reference could not render | 0 | **0** |
| total unaccounted ink | 1857.62 | **2810.88** |
| pages the image tool calls major | 582 | **624** |

**The difference is the nine documents and nothing else, to the digit.** They come to
953.26 ink and 42 major pages between them, and 1857.62 + 953.26 = 2810.88 while
582 + 42 = 624. Every figure the tenth round published for a measurable document reproduces
exactly — `NAS-Infrastructure-Roadmaps-v16.0` 368.41, `HENTZEN` 127.20, `Framing Europe`
66.45, `N2_E_Maestroni` 66.12, `Wildlife for REDAC` 56.67, `Thailand17` 49.65,
`Reporting_responsibilities_matrix` 48.41, `171128IPAP` 45.05, `redac-fullComm` 44.78,
`joint_user_outcomes` 40.64, `Course Selection` 11.10.

So the tenth round's totals **excluded** the nine rather than counting each skipped page as
a major, which is what the handover said the tool did. The tool's own per-document line does
count a skip as major — that part is right, and it is why `3492.pptx` was reported at 22
majors and measures 7 — but whatever produced the track totals did not use that line. Worth
recording because the two statements cannot both describe the same number.

```
766.96  28 of  28 major  order-of-worship-ppt-revised-2018.pptx   (invisible until now)
368.41  77 of 137        NAS-Infrastructure-Roadmaps-v16.0.pptx   (linked OLE, known)
159.50   7 of  22        3492.pptx                                (invisible until now)
127.20  12 of  12        HENTZEN_...AEROSPACE_INDUSTRY.pptx       (the missing a:duotone)
 66.45  24 of  24        Framing Europe.ppt
 66.12   1 of  30        N2_E_Maestroni_Swarm_COP.pptx            (the Gantt)
 56.67  10 of  41        Wildlife for REDAC September 11.pptx     (circle gradient, known)
 49.65  18 of  54        Thailand17.ppt
 48.41   6 of 268        Reporting_responsibilities_matrix.pptx
 45.05  18 of  40        171128IPAP.pptx
 44.78   8 of   8        redac-fullComm-201705-EE-FRs-briefing.pptx
 40.64  19 of  20        joint_user_outcomes_michael_fullerton_29.06.12.ppt
```

**`order-of-worship-ppt-revised-2018.pptx` is 27% of the whole track's unaccounted ink** and
had never been seen, because it is one of the nine.

### Round eleven, measured: 2810.88 -> 1635.52 unaccounted ink

Swept whole before and after, 163 documents each time, reusing the reference PDFs — nothing
this round touches `soffice`, and the after sweep's reference column is the base sweep's file
by construction.

| | baseline | after |
|---|---|---|
| slides matching the word gate | 152/163 | **152/163** |
| pages with a correct count | 163/163 | 163/163 |
| documents whose page count, word count or verdict moved | — | **0** |
| total unaccounted ink | 2810.88 | **1635.52** |
| pages the image tool calls major | 624 | **538** |
| documents whose ink moved | — | **14: 12 down, 2 up by 5.08 between them** |

```
766.96 ->    8.03   order-of-worship-ppt-revised-2018.pptx       major 28 ->  3
159.50 ->   15.70   3492.pptx                                    major  7 ->  3
127.20 ->    5.78   HENTZEN_...AEROSPACE_INDUSTRY.pptx           major 12 ->  3
 66.45 ->   23.99   Framing Europe.ppt                           major 24 -> 10
 44.78 ->    2.95   redac-fullComm-201705-EE-FRs-briefing.pptx   major  8 ->  1
 40.64 ->    4.41   joint_user_outcomes_michael_fullerton.ppt    major 19 ->  1
 18.59 ->    3.48   PRM_training.pptx                            major 10 ->  2
 14.10 ->    3.68   SRDMG(16)024_60 GHz onboard airplanes.pptx   major  2 ->  3
 35.88 ->   26.67   Demick_JetBlue.pptx                          major  7 ->  6
```

The two that rose are `chapter_4_0.pptx` (+5.04) and `social-media-app-bulletin-january.pptx`
(+0.04). **The first is visually closer and scores worse**, which this instrument has done
before: its ink rises from about 0.10 to about 0.19 on nearly every page while `diff%` does
not move at all, and the footer band that carries the difference reads 143 grey before, 119
after, against the reference's **115**. Its six layouts carry `overrideClrMapping`, four of
them differing from the master, so the change is the intended one and it moved the colour
towards the reference. Small figures on a 512-pixel region metric are noise.

### The four fills

**`a:duotone` — 894.16 of ink between two documents.** An Office theme paints one grey
texture in a deck's own colours by putting a duotone on the blip: every pixel becomes
`dark + (light − dark) × its own luminance`. The string appeared nowhere in `dotnet/src`.
`order-of-worship-ppt-revised-2018.pptx` takes its whole background from
`a:bgFillStyleLst`'s third entry that way and drew as a dark vignette against a pale
reference — **27% of the entire track's unaccounted ink in one document**, and it had never
been looked at because it is one of the nine the image tool could not measure.
`HENTZEN_…AEROSPACE_INDUSTRY.pptx` is the same mechanism, its dark red banner coming out
grey. 17 of the 112 corpus decks state one.

The port has three details that are not guessable. Luminance is
`(B×29 + G×151 + R×76) >> 8`, integers summing to 256 rather than the Rec. 601 coefficients
they approximate. Both divisions in `lcl_getDuotoneColorComponent` truncate, so it is not a
rounded lerp. And the encoded bytes have to be dropped when the transform runs — they are
kept so a JPEG reaches a PDF as `DCTDecode` untouched, and that pass-through would have
emitted the original picture and lost the recolouring on one backend only.

Verified against the binary rather than against the formula: `slide-duotone.pptx` draws four
rows — black, mid grey, saturated blue, white — and LibreOffice puts them at `112255`,
`127 126 127`, `41 54 93` and `EEDDAA`. Ours matches all four, and all four of the
alphaModFix page's, exactly. The blue row is what separates VCL's weights from Rec. 601, at
luminance 28 against 29; the grey row is what separates truncation from rounding.

**`a:alphaModFix` on a fill — 143.80 on one document.** `DrawingBlipFill.Opacity` was parsed
and reached only `PlacedPicture`, the `p:pic` path. A picture used as a *fill* went through
`BitmapPaint`, which had nowhere to put it. `3492.pptx` lays black text over a runway
photograph its layout states at `amt="16000"`; we drew the photograph at full strength and
the text was unreadable on it. The "read but never used" shape for the fifth time, and the
tell was the same as ever — every reader parsed the value and grepping it found only the
readers and the model. 5 decks state one on a fill.

**`p:clrMapOvr/a:overrideClrMapping` — 41.83 on one document.** A layout may amend the
master's colour map, and `PptxFile` said so and left it unread with the note "nothing
measured carries one". 10 of the 112 corpus decks carry one and 7 differ from their master's
map. `redac-fullComm-201705-EE-FRs-briefing.pptx`'s title layout sends `bg2` to `dk2` where
its master sends it to `lt2`, and its whole title page is the theme's circle gradient over
that colour: a deep teal radial in the reference and a near-white wash in ours. Settled by
arithmetic before any code was touched — the `tint`/`satMod` chain over `dk2` gives
`(76,160,212)` and `(0,44,56)`, and LibreOffice's flat-ODF export states
`draw:start-color="#002b36" draw:end-color="#4aa2d6"`.

The override *patches* the map in force rather than replacing it, which is the part the
schema does not say: `SlideFragmentHandler` copies the current map for an
`overrideClrMapping` and starts from an empty one for a `clrMap`
(`slidefragmenthandler.cxx:194-203`). A **slide's** own override is deliberately not applied:
measured on a purpose-built fixture, the reference does not repaint a slide's *inherited*
background with it, because Impress resolves a master page's fill once as it imports the
layout. Exactly one slide in the corpus states one, and it restates the master's map.

**`DFF_Prop_fillOpacity` — 92.69 across three `.ppt`.** Escher's fill opacity, absent from
`dotnet/src`. `Framing Europe.ppt` draws four `#0066ff` bands over a blue-to-black shade on
its master, and LibreOffice's flat-ODF export states `draw:opacity="50%"` on each; we drew
them opaque, so twenty-four pages had bright bands through the body text. The 50% was fitted
from the reference's own pixels before any source was read — with base *b* and stripe *s*,
`ref = a·s + (1−a)·b` solves to a = 0.50 at three different heights down the gradient.

Two roundings, deliberately not shared. A non-shaded fill goes through
`XFillTransparenceItem`, a whole percent (`msdffimp.cxx:1367-1376`); a shade goes through
`ImportGradientColor`, which truncates `(1−dTrans)×255` into a luminosity mask
(`msdffimp.cxx:2930-2941`) and never sees a percent. A shade's two ends swap their opacities
with their colours, inside the same `nChgColors` branch. Absent is not zero: `dTrans` starts
at 1.0 and is only overwritten inside `IsProperty`, so reading the property with a zero
fallback would empty every fill in every deck. Reach, parsed from all 51 corpus `.ppt`: 6
documents.

### What the next agent on this track should take

```
368.41  77 of 137 major  NAS-Infrastructure-Roadmaps-v16.0.pptx   (linked Excel OLE, known)
 66.12   1 of  30        N2_E_Maestroni_Swarm_COP.pptx            (the Gantt)
 56.67  10 of  41        Wildlife for REDAC September 11.pptx     (circle gradient, known)
 49.65  18 of  54        Thailand17.ppt
 48.41   6 of 268        Reporting_responsibilities_matrix.pptx   (unlooked at)
 45.05  18 of  40        171128IPAP.pptx                          (residue of round ten's fix)
 37.89  11 of  24        Sector_Skills_Insights_Advanced_Manufacturing…pptx  (unlooked at)
 31.84  28 of  94        2015-Civil-Rights-Website-training.ppt   (unlooked at)
 28.32  13 of  94        8.16_AOD_FINAL_Provider_Training…ppt     (unlooked at)
 26.67   6 of  10        Demick_JetBlue.pptx
 25.99  11 of  52        ghgp-supply-chain-initiative_20100323_wri.pptx
 25.95  12 of  47        ITE106-Chapter 4.ppt                     (unlooked at)
 23.99  10 of  24        Framing Europe.ppt                       (residue: bullet glyphs)
```

`Framing Europe.ppt`'s residue is a different defect and a small one, named but not chased:
the reference draws its bullets as a Wingdings filled square 13.7 pt wide and we draw a
6.3 pt `•`. Its text layer shows the reference's bullet as a private-use code point and ours
as an ASCII bullet, which is the encoding difference the skill's word-gate note describes,
seen here from the pixel side instead.

### The regression guards, and the tests

Four of the six files changed are shared below the presentation family —
`Paperless.Core/Graphics` (a `RasterImage` field and a `BitmapPaint` field),
`Paperless.Rendering` (the decoder and both sinks) and
`Paperless.Ooxml/DrawingML/DrawingFill.cs` — so the other two tracks were swept as well.

| | |
|---|---|
| `slides/batch-001`–`017`, whole track | **152/163**, every page count exact, 0 `ref-failed` |
| `words/batch-001`–`005` | **50/50** |
| `sheets/batch-001`–`005` | **50/50** |

The two slides sweeps are **identical row for row on all 163 documents** — not one page count,
word count, font count or verdict moved — which is both the regression check and the evidence
that all four fixes are invisible to the word gate.

Per project on the final tree, each run redirected to a file: Core 238, Text 182,
Containers 109, Rendering **104** (was 97), Markup 259, OpenDocument 125, WordProcessing 541,
Spreadsheets 374, Presentations **485** (was 473), Vector 291, Fidelity 538 with **0 skipped**.
The two that moved are the twelve tests added and nothing else.

Three fixtures, each verified by putting the bug back and watching the named tests fail:
`ppt-fill-opacity.ppt` (LibreOffice's own conversion of `ppt-fill-opacity.fodp`),
`slide-colour-map-override.pptx` and `slide-duotone.pptx`.
## After the eleventh round: words, and the 54 failures characterised at last

Swept whole four times, 200 documents each: a baseline, the wide form of the one fix, its
narrowed form, and a second fix that was measured and reverted. The baseline reproduced the
brief **to the digit** — 146/200, 154 page-exact, 120 of absolute page error, 7110 of absolute
word error split 3200 short against 3910 over, `doc` 54/66 and `docx` 92/134, and all
twenty-one per-batch figures.

| | baseline | shipped | wide form | + `w:tabs` merge |
|---|---|---|---|---|
| match | 146/200 | **146/200** | 145 | 145 |
| exactly correct page count | 154 | **155** | 154 | 153 |
| total absolute page error | 120 | **119** | 123 | 145 |
| total absolute word error | 7110 | 7195 | 6979 | 7818 |
| documents whose output moved | — | **22** | 28 | 30 |

Batches 001–005 are 10/10 and 006 is 9/10, so the gate is green. Per batch, unchanged from the
baseline everywhere: 001–005 10/10, 006 9/10, 007 9/10, 008 10/10, 009 10/10, 010 7/9, 011
8/10, 012 8/10, 013 5/9, 014 3/10, 015 5/10, 016 7/10, 017 5/10, 018 4/10, 019 3/10, 020 3/10,
021 0/2.

The word error rising by 85 is one document: `TE.CAO.00125 … OJT Logbook.docx` goes 16 pages
against 15 to **15 against 15** and loses that page's furniture with it, 3066 words to 2974.
It fails either way; the page count is the number that got right.

The whole slides track was swept on the same binary — **152/163, every page count exact, page
error 0** — which is the recorded figure exactly, so the shared shaping change is neutral there.

### The residue, sorted: 23 documents are pagination alone

Nobody had clustered the failures since the fixes of the last four rounds landed. Sorted by
whether the *text* is right:

| Cluster | Documents | Shape |
|---|---|---|
| **pagination only** | 23 | page count wrong, word count inside 0.5%. The text is right and the boundaries are not. 16 of the 23 are off by exactly one page and their total page error is 56 |
| **text only** | 8 | page count exact, words wrong: five over by 4.3% to 6.6%, three short |
| **both** | 23 | the hard tail, and it holds every large deviation |

**Whole families fail together, and that is the shape worth dispatching on** — not the batch:

| Family | | |
|---|---|---|
| `*_mcar_part-*` | 0/5 | all over-paginate, +1 +1 +1 +4 +5, every word count inside ±0.6% |
| `150_5300_13_chg*` | 0/3 | all `.doc`, all exactly +2 |
| `150-5370-10*` | 0/2 | −13 and −12 on 700-page documents whose words agree to 0.04% |
| `ABCD-*` | 0/3 | all over-draw, +4.6% to +6.0% |
| `ESPN-R *` | 0/2 | both −1 |
| `TE.CAO`/`UG.CAO`/`FO.FCTOA`/EASA forms | 2/9 | re-measured after the grouped-text-box fix; **not one cluster** — see below |
| tpr/report templates | 1/5 | three at −1, one at +1 |

### The format split: the earlier conclusion no longer holds, and the new one is weaker than it looks

`doc` fails at 18% (12/66) and `docx` at 31% (42/134), reproducing the brief exactly. The
earlier round's 42%/43% is genuinely gone, so "split two agents by format" is no longer refuted
by near-equal rates. **It is still the wrong split**, and the families above are why: the two
largest are `docx` (`mcar`) and `doc` (`150_5300_13`), each five and three documents failing
the same way as each other. Splitting by format would hand one agent both halves of one cause
and the other agent none of it.

### The issuer's form templates are not one cluster

Six of the eight `TE.CAO`/`UG.CAO`/`FO.FCTOA` documents fail, which was the reason to look, and
taking two of them apart per page shows two unrelated defects:

- `UG.CAO.00133 … Language.docx` (18/18 pages, +245 words) is **+18 to +22 words on 13 of its
  18 pages** — a near-constant, which is page furniture. It is section 1's header, drawn by us on
  every page and by LibreOffice on pages 1 and 14–17 only. See below.
- `approvals-and-standardisation … TE.CAO.00129 … logbook.docx` (6/6, +73) is exactly 0 on four
  pages and **+37 on each of the last two**, where we draw a table's numbered heading row and the
  reference has already moved on to the data rows.

Two documents, two mechanisms, no common cause. The family shares a template, not a bug.

### `w:headerReference` and the header nobody inherits — measured, not implemented

`UG.CAO.00133` has five sections. Sections 1 and 4 name a `w:headerReference w:type="default"`
whose part has content; sections 2, 3 and 5 name only `"even"` and `"first"`, and every one of
those parts is empty. The document sets neither `w:titlePg` nor `w:evenAndOddHeaders`. We apply
ECMA-376 §17.10.1 per slot (`DocxReader.cs:299-301`): the Default slot is unnamed there, so it
is inherited from section 1, and with both switches off that inherited header is what every page
draws.

Four probes on the real file, rendered through `soffice`, with the section boundaries confirmed
by pointing a section at a header that has content:

| Probe | Header pages |
|---|---|
| as found | 1, 14–17 |
| section 3 given a `default` header with content | 1, **3–13**, 14–17 (so section 3 *is* pages 3–13) |
| section 3 with every `headerReference` removed | 1, 14–17 — **no inheritance** |
| section 2 given `even`+`first` **footer** references, both parts empty | footer on all 18 — **inert** |
| section 2 given a `default` footer, part empty | footer on page 1 only — **settles the slot** |

So an `even`/`first` reference is inert for the footer and suppressing for the header, on the
same file, with equally empty parts. **I could not name the mechanism.**
`sw/source/writerfilter/dmapper/PropertyMap.cxx:594-598` forces `HeaderIsOn` false for a section
that pushed no applicable header, while `copyHeaderFooter` (:1117-1125) says in its own comment
that it means to link to the previous section; which of the two wins here is not established.

Left unimplemented deliberately. Our reading is the one the standard states, the rule that would
match the reference costs this same document its footer on sixteen pages, and neither is worth
guessing at across the 42 corpus DOCX that state `w:titlePg`. It is worth about 250 words on
this document and it is the largest single named item in the "text only" cluster.

### Four of `UG.CAO.00133`'s raster-ceiling pages are false positives

`TODO.raster-ceiling.md` flags its pages 3, 7, 8 and 18 for +20, +45, +20 and +20 words.
`pdfimages -list` says the raster on each is a 162×109 JPEG that **both** sides draw — the EU
flag in the footer, same object, every flagged page. Condition 1 of the flag is met by a logo
rather than by a rasterised object, and the excess is the header defect above: page 3 is +18 and
the whole of it is that block.

The signature therefore misfires on any document that puts a small logo in its page furniture
*and* has a furniture defect elsewhere. That is not an argument against the flag; it is the
second worked example of why the file says to re-measure a document before subtracting its
flagged pages.

### The anchor character was drawn as a missing-glyph box

Every word-processing reader writes `U+0001` where a picture, a floating frame, an embedded
object or a comment mark stands — `DocxLayoutSource.cs:998` and `:1002`,
`OdtLayoutSource.cs:1045`, `Ww8DocumentReader.Layout.cs:693`, `WritingNodes.cs:221`. No text
face maps it, so HarfBuzz returned `.notdef` at the face's glyph-zero advance — 0.78 em in
Liberation Serif, 0.75 in Liberation Sans — which is ink on the page and room on the line, *on
top of* the picture's own width, which `PageContent.InlineObjects` already adds at the same
offset. 88 of the track's 134 DOCX carry a `w:drawing` and 48 carry a `w:pict`.

`HarfBuzzShaper.Shape` now splits its input at the characters LibreOffice removes before shaping
— `ImplLayoutArgs::AddRun` on `IsControlChar`, `vcl/source/text/ImplLayoutArgs.cxx:111` — and
shapes the stretches between them, so the kern inside a word next to one survives. The tab is
the one member of the C0 range deliberately kept, because `TabRuler` resolves it against the
paragraph's stops the way Writer's tab portion does. Our own two shapers already disagreed about
this: `MetricsShaper.cs:45` gives glyph zero an advance of nought.

Measured on `picture-anchor.docx`, whose sentence has text on both sides of a 1 cm inline
picture: every word of that line now lands where LibreOffice's own PDF of the `.doc` and the
`.fodt` puts it, to 0.10 pt — which is the constant offset already present at the line's first
character. `.notdef` had been adding 9.33 pt.

**The first form of this fix cut the same list in `TextItemiser` as well, and that was wrong.** A
cut there removes the character from every sub-run, so a paragraph whose whole text is one
control character produces no run, no line, and no anchor for the frame it stands for — which is
the ordinary shape of a logo, a footer paragraph holding one `w:drawing`.
`1603642410-MoM-CASCOM-06-2020-draft04.docx` lost "An agency of the European Union" from all
nine of its footers, 4527 words to 4473 with the page count unmoved, and two more documents lost
a page and its furniture. On the whole track that form read 145/200 with 123 of page error, and
part of what looked like an improvement in its word column was text going missing. The C0 range
is dropped one layer down instead, where it costs the character its glyph and leaves the run
intact.

### `w:tabs` merging: right by the specification, and it costs 26 pages

`WordParagraphFormats` takes the innermost layer stating a `w:tabs` whole, so a paragraph with
any direct tab loses every stop its style set. `DomainMapper` does the opposite for
`LN_CT_PPrBase_tabs`: it seeds the working vector from the paragraph style sheet and folds each
stated stop in through `IncorporateTabStop`, which replaces by equal position and appends
otherwise (`DomainMapper.cxx:2604-2620`, `DomainMapper_Impl.cxx:1485-1498`); `w:val="clear"`
removes the inherited stop at its position. RTF has the opposite rule, spelled out on the same
case as an `IsRTFImport` branch citing fdo#81033.

The symptom is real and precisely located. `SPA-11_mcar_part-11_v2.9.docx` has a `TOC3` style
carrying `<w:tab w:val="right" w:leader="dot" w:pos="9350"/>` and entries that each add
`<w:tab w:val="left" w:pos="2520"/>`; those entries lost the leader and the right margin
together, so no dots were drawn, the page number did not align right, and the entry wrapped onto
a second line. Its contents pages 8–10 are the first three in the document to differ from the
reference at all, and they go **+27/+26/+15 words → +22/+20/+12** with the merge.

It is still a net regression and it was reverted. Eight documents moved and none improved:
`02_mcar_part-2_and_IS_v2.10.docx` 317 → **329** pages against 312,
`SPA-02_mcar_part-2_and_IS_v2.9.docx` 270 → **282** against 266,
`hdss-bulletin-issue-285…docx` 10/10 → 11/10 and a lost match, `ABCD-FE-01-00` losing its exact
page count. Page error 119 → 145.

**The scope is what is wrong, not the mechanism.** I merged every layer
`ParagraphPropertyLayers` returns — direct, the whole style chain, the table style,
`w:docDefaults` — where `DomainMapper` seeds from the style sheet's already-resolved stops and
folds only the stated ones in. Neither `w:docDefaults` nor `Normal` declares tabs in that
document, so the defaults layer is not the difference; 119 of its body paragraphs state a direct
`w:tabs` and now carry their style's stops as well, and a right stop the pen has already passed
yields no gap at all where a default-interval stop used to give one. That is the untested half,
and it is a `TabRuler` question rather than a reader one.

The corpus fixture is kept — `tests/corpus/features/tab-stops-inherited.docx`, four paragraphs
for the four cases, which LibreOffice renders distinguishably:

```
Inherited...........................................9
1.1            Added................................9
Cleared           9
Replaced                                           9
```

The reverted code is at `b62aaf38b` and the tests that assert it at `3997fb739`.

### Leads for the next round, in the order I would take them

1. **The `mcar` family, 0/5, one cause.** Over-paginating by +1, +1, +1, +4, +5 with every word
   count inside ±0.6%. On `SPA-11` the divergence is at **page 43** of 50 and pages 11–42 track
   the reference to the word, so it is one boundary decision late in a long document rather than
   accumulated drift. The TOC leader above is a separate, real defect on the same documents and
   moves them three pages closer without changing the count.
2. **The `w:tabs` scope**, with the measurement above and the reverted patch to start from.
3. **The header inheritance**, if somebody can name LibreOffice's rule rather than fit one.
4. **`WritingFieldKind.PageNumber` still has no consumer** — the sixth "read but never used", and
   now with a worked example: our `UG.CAO.00133` page 3 prints *"Page 1 of 18"* where the
   reference prints *"Page 3 of 18"*. `Paginator.cs:456` and `:615` already carry the number, so
   what is missing is the substitution at draw time. It cannot move the gate — a footer printing
   `1` on every page counts as one word exactly as `1`…`9` does — and it is a visible defect on
   every document with a page-number field.
5. **`w:pict` draws nothing at all.** `DocxLayoutSource.cs:1001` emits an anchor character for
   `w:pict`, `w:object` and `w:commentReference` and adds no frame, so a VML picture is not laid
   out or drawn. 48 of the track's 134 DOCX carry one, 332 occurrences. A feature rather than a
   fix, and unmeasured.


## After the twelfth round: words 006 and 007, one rule named and one refuted

Swept the whole 200-document track four times, at the base and after each of three forms of the two
changes. **The base reproduced the brief exactly** — 146/200, 154 documents page-exact, 120 of
absolute page error, and all twenty-one per-batch figures — and `words/batch-001`–`005` swept
separately came back 50/50 with no row differing from the base in page count, word count or verdict.

| | base | shipped | wide form | row-height rule |
|---|---|---|---|---|
| match | 146/200 | **146/200** | 146 | 145 |
| documents page-exact | 154 | **155** | 155 | 154 |
| total absolute page error | 120 | **120** | 121 | 120 |
| total absolute word error | 7110 | **7191** | 7214 | 7206 |
| documents whose output moved | — | **14** | 18 | 2 |

Per batch, unchanged from the base everywhere: 001–005 10/10, 006 9/10, 007 9/10, 008 10/10, 009
10/10, 010 7/9, 011 8/10, 012 8/10, 013 5/9, 014 3/10, 015 5/10, 016 7/10, 017 5/10, 018 4/10, 019
3/10, 020 3/10, 021 0/2. **No document changed verdict in either direction**, so the round bought no
match; what it bought is a rule named and a rule struck off, and the second cost more to establish
than the first.

Tests, per project: Core 238, Text 185, Containers 109, Vector 291, Rendering 104, Markup 259,
OpenDocument 125, WordProcessing 553, Spreadsheets 401, Presentations 485, Fidelity 538 — **0 failed
and 0 skipped in every one**. Nothing outside `Paperless.WordProcessing` was touched, so the other
two tracks were not swept.

### A title page with nothing named for it draws nothing — and one with a *footer* named for it does not

`ChosenSlot` fell through to the Default slot whenever a section stated "different first page" and no
first-page part existed, so a title page got the running head and a line's worth of room it should
not have had. The eighth round recorded the rule as needing "its own before-and-after sweep rather
than a spare hour"; this is that sweep, and the sweep is also what narrowed the rule twice.

Two corpus documents differ in exactly one thing and settle a question the eighth round left open
after four probes. Both state `w:titlePg`; neither names a first-page **header**:

| Document | What its first section names | LibreOffice's page one |
|---|---|---|
| `007 final-technical-report-template.docx` | default header, default footer | **no** running head |
| `016 JEMIT_Template.docx` | even + default headers, even + default footers, and a `w:footerReference w:type="first"` | the **default** header, which it never named for a first page |

So naming *any* first-page part, of either kind, makes a first-page style and the kind that was not
named is copied onto it; naming none leaves the first page bare. That is the shape of
`copyHeaderFooter` (`writerfilter/dmapper/PropertyMap.cxx:1117-1125`) beside the branch at `:594-598`
that forces `HeaderIsOn` false for a section that pushed no applicable header — which of the two wins
was the open question, and the pair above answers it by measurement rather than by reading.
`PageFurnitureSet.cs:98` is the test and `:145` the rule.

**The footer half of it is not established and is deliberately not applied.** `final-technical-report`'s
reference page one has no footer either, which says it should be; `017 Agile_Arc_SysDes.docx` — the
same shape of section, a default header and footer with `w:titlePg` and nothing named for a first
page — has one, which says it should not. Suppressing both cost that document eight words for
nothing. Measured across the track, the wide form moved 18 documents for +2 of page error and +19 of
word error, and the header-only form moves **14 for +1 and −4**. Eight of the fourteen are closer to
the reference and six further; the one page count that moved is
`019 ESPN-R - MCF - RA - Ed1.docx`, 58 pages to 57 against 59.

Fixture: `tests/corpus/features/title-page-header.*` in five formats, generated from one flat-ODF
source, whose DOCX export has exactly the corpus shape — a lone `w:headerReference w:type="default"`
beside a `w:titlePg`. Asserted on `.docx`, `.doc` and `.rtf`. **The `.odt` is committed and
deliberately not asserted**: ODF states the distinction as two master pages joined by
`style:next-style-name`, nothing reads that, and every page of that file comes out bare — a separate
reader defect with its fixture already in place.

### The row LibreOffice will not split: the threshold reproduces, and both explanations of it are wrong

`batch-006`'s one failure is `f445896eb008d14c1746fc37d412dc22.docx`, 15 pages against 16 with the
word counts identical at 5575. It is one 30-row table whose rows state large at-least heights — 4965,
10026, 13800 twips — and the reference splits exactly one row in sixteen pages, moving every other
one whole and leaving pages a third empty. We split all of them. Per page ours against the
reference: `370/370 355/355 416/323 409/127 439/416 439/485 474/67 …`.

**The predecessor's measurement reproduced exactly and is now much better bounded.** Rewriting row
9's `w:trHeight` in place, the row splits at 4267 twips and moves whole at 4282, with 2697 twips of
room left. The declared height is *not* the row's height there — placed at the top of a page the row
measures 5147 twips at any stated height below that, so 4200 and 4300 give identical geometry and
only the decision moves. Removing every `w:trHeight` from the document makes all its rows split and
takes it to 15 pages, so the stated height is what does it.

Varying the room as well, by rewriting row 8's height, the threshold is not the room, is not
proportional to it, and is not continuous in it:

| room left (twips) | 2697 | 2202 | 1952 | 1702 | 1202 | 1102 | 1052 | 802 |
|---|---|---|---|---|---|---|---|---|
| threshold on row 9 | 4275 | 4275 | 3323 | 3323 | 3323 | 3323 | 2107 | 2107 |

The three values are the bottom of the last line of a paragraph, plus about 70 twips, and the
paragraph is the *second* one of what would be left over. That fit was tested rather than admired: it
predicts that moving the room from 1102 to 1052 — fifty twips, across one paragraph boundary — drops
the threshold by 1216, and it does.

**Both readings then died on a controlled fixture.** `tests/corpus/features/table-row-min-height.*`
states an at-least height of 5.2 cm on a row with 3.5 cm left on the page, and LibreOffice breaks it
anyway; so does the same shape at A4 in the corpus document's own geometry, with the declared height
swept from nought to 10 cm, with symmetric and with lopsided cells. Whatever the corpus document is
doing, it is neither "a minimum that does not fit bars the break" — which is spelled out in Writer's
own source at `sw/source/core/layout/tabfrm.cxx:1188-1196` and reached only inside a splittable fly —
nor the paragraph rule the eight probes fit.

Implemented anyway and measured, because the source citation was persuasive: the whole track came
back **145/200 with page error unchanged at 120 and word error 7110 → 7206**, costing
`008 4400-91_Proposal_To_Lease_Space_10-2024.docx` its page count and buying nothing. It did take
`f445896`'s exactly-agreeing pages from 2 to 12 — pages 1–12 matched the reference word for word —
without changing its count, which is why the numbers are worth reading together. Reverted.

What is left of that document, once the rows are placed correctly, is one line: on page 13 we fit
`industry.` at the end of a cell line where the reference wraps it, our line reaching x = 519.74 in a
column whose text edge both renderers use out to about 524. Everything after that is downstream of
those 13.45 pt.

### What the next agent on this track should take

1. **`batch-010`, two documents, both −1 page with the words already right.** `195584360.docx` 19
   against 20 and `5709.16 ch.40_mgfinal.docx` 31 against 32. Untouched this round.
2. **The `mcar` family, 0/5**, unchanged from the eleventh round's list and still the largest single
   cause named on the track.
3. **`WritingFieldKind.PageNumber` still has no consumer**, and this round establishes what it would
   cost: `PageFurnitureSet` lays a slot out **once and caches it**, deliberately, because most pages
   share one header and re-shaping it per page is the largest cost in paginating a long document. A
   page number is the one thing that changes per page, so the substitution needs either a field range
   carried on `PageParagraph` — which holds a bare string today — or a per-page re-layout. It is a
   feature, not a wiring change, and `final-technical-report-template.docx` prints `Page 3 of 1`
   against the reference's `Page 1 of 1` as the worked example.
4. **The footer half of the title-page rule**, with `final-technical-report-template.docx` and
   `Agile_Arc_SysDes.docx` as the A/B pair that currently disagree.
5. **`table-row-min-height`'s corpus document**, if anyone wants to name what really stops LibreOffice
   splitting those rows. Everything above is reproducible in minutes with the probe recipe: rewrite
   `w:trHeight` in place, convert with `soffice`, and read which page each row's timecode lands on.
## Slides, round twelve

### The picture-frame fill rule is settled: storage decides, rasterisation does not

Round ten measured five cases and found two explanations fitting all of them, and could not
separate them because the corpus holds no instance that does. It named exactly what would:
**a rasterised metafile that is not inline, whose frame states a fill.** The corpus has none —
so author one.

`8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` slide 5 is the only ingredient needed. Its
`ppt/media/image8.emf` is a package entry, and the reference demonstrably rasterises it: a
692x240 RGB image with a soft mask, no extractable text. Its `p:pic` states `<a:noFill/>`.
Replace only that, in only that shape, and render both:

| deck | page 5 |
|---|---|
| the frame as found, `<a:noFill/>` | 692x240 raster + smask |
| the same frame stating `<a:solidFill><a:srgbClr val="FF0000"/>` | **byte-identical PNG at 60 dpi, 0 red pixels** |
| the same red fill, `image8.emf`'s bytes swapped for a half-clear PNG | **8736 red pixels**, bbox (188,56)-(299,133) |

The third row is the control, and it is what makes the second mean anything: the same edit to
the same shape in the same deck *is* observable when the graphic is one the rule calls
transparent, so a null result on the EMF is the renderer's answer rather than an edit that
never arrived.

The probe is also non-vacuous, which is the other way it could have been empty. The extracted
soft mask is **84.03% value 0** — the frame is transparent over five-sixths of its area, so a
fill drawn behind it would have covered most of the box in red rather than a sliver.

**So the competing explanation is refuted.** Rasterising a metafile does not make its frame's
fill appear; a package-stored metafile loses the fill whether the reference plays it as vectors
(`2014BSA_Sunday_Killion.pptx` slide 5) or rasterises it (this deck's slide 5). The shipped rule
— inline `office:binary-data` and Escher blips keep the frame's fill, package entries lose it —
is the one that fits all six measurements, and it is what the code already does. **No code
change**; the value is that the alternative is now closed rather than carried.

Reproduce with `scratchpad/sl12-frame/mk.py` + `mk2.py` (probe decks), `sample.py` (red-pixel
count) and `mask.py` (soft-mask histogram). Reference: LibreOffice 24.2.7.2 420(Build:2).

### A paragraph ending in a line break is one line short — measured, not yet implemented

`slides/batch-015/ppt/2015-Civil-Rights-Website-training.ppt` (31.84 unaccounted ink, 28 of 94
pages major, never examined) matches the word gate exactly and looks obviously different: the
reference spaces its bullets a whole line apart and sets them smaller than we do.

Two numbers off page 39, both from `pdftotext -bbox`:

| | ours | reference |
|---|---|---|
| line pitch *inside* a paragraph | 19.200 | 16.328 |
| baseline pitch *between* single-line paragraphs | 24.200 | 37.645 |
| width of the word "Proficient" | 93.70 | 80.41 |

The width ratio is **0.858** and the inner line-pitch ratio is **0.8504**, so the reference is
setting the whole frame at about 85% — an autofit shrink. Our 24.200 is 19.200 + 4.99, and
LibreOffice's own flat-ODF export states `fo:margin-top="0.176cm"` — 4.99 pt — so our paragraph
spacing is *right*. The reference's 37.645 is 4.99 + **two** line heights.

The export names the cause outright. Every one of those paragraphs ends
`…<text:span><text:line-break/></text:span><text:span/>`: the PPT text stream carries a trailing
`\x0B`, LibreOffice's EditEngine opens a line after it, and the paragraph is two lines tall.
`TextMeasurer.Fit`'s loop is `while (lineStart < text.Length)`, so a mandatory break at the very
end leaves `lineStart == text.Length` and the loop exits without emitting the line the break
opened. `PptTextBody` does carry the character through — it maps `\v` to `\n`, which
`IsMandatoryBreak` accepts.

**The two defects are one.** The frame is overfull in the reference *because* of the extra
lines, which is what triggers the 85% autofit; ours fits at 100% because it is short a line per
bullet. So the pitch ratio is downstream of the missing line rather than a second bug.

Not implemented here. `TextMeasurer` is in `Paperless.Text/Layout` and serves all three
families, and the mandatory-break set it would key on includes `\r`, `\n` and U+2029 — which
some reader may be leaving on the end of a paragraph to *mean* "paragraph ends here". Getting
that wrong adds a line to every paragraph in the corpus. It needs the probe deck below and a
sweep of all three tracks, which is more than this round had left after the `a:fontRef` work.

The probe to write first: one `.pptx` with two text boxes differing only in a trailing
`<a:br/>`, rendered through `soffice`, to confirm the extra line is a line break's own rule
rather than something in the binary reader.

### The `a:fontRef` fix, swept whole: 1635.50 -> 1583.80

Baseline swept at `5ec407cf3`, 163 documents, two workers, then again with the shape-style
change and the same reference PDFs. The baseline reproduced round eleven's figures — **152/163,
ink 1635.50 against the recorded 1635.52, 538 major pages, every page count exact, 0
`ref-failed`** — which is the tell that the base and the instrument are both right.

| | baseline | after |
|---|---|---|
| slides matching the word gate | 152/163 | **152/163** |
| pages with a correct count | 163/163 | 163/163 |
| total unaccounted ink | 1635.50 | **1583.80** |
| pages the image tool calls major | 538 | **502** |
| documents whose ink moved | — | **21: 14 down, 7 up by 5.42 between them** |

```
 37.89 ->  8.92   Sector_Skills_Insights…pptx                      major 11 ->  4
 11.10 ->  2.14   Course Selection 2025-26 Current Grade 09.pptx    major  4 ->  0
 12.88 ->  9.07   Intersil_Italy_CAN_Bus_Transceiver…pptx          major  4 ->  3
  4.18 ->  0.37   Ensemble-pour-l-amelioration…AIRBUS.pptx         major  3 ->  0
 16.43 -> 13.03   southern-classic-kennesaw-state-university.pptx   major 13 ->  7
  6.34 ->  3.50   FAAAIandtheArtandScienceofV&Vfinal.pptx          major 10 ->  0
 16.09 -> 20.27   16 - UTM - (NASA).pptx                           major  3 ->  4
```

**Exactly one of the 163 rows moved a column**, and not one page count, word count or verdict:
`16 - UTM - (NASA).pptx` now embeds 12 faces against the reference's 11. It is the deck with 62
`a:fontRef` shapes and it was already failing on `words,unembedded`; the extra face is the
typeface half of the reference reaching a run that previously fell through to the master's
`+mn-lt`. That deck is also the whole of the rise. Worth a look and not chased here.

**A note left for whoever does look.** `Shape::createAndInsert` puts the `maPhClr` assignment
*inside* `if (pFontRef->mnThemedIdx != 0)` (`oox/source/drawingml/shape.cxx:2242-2252`), so on
the checked-out source a `fontRef idx="none"` contributes neither the face nor the colour. Our
`DrawingCharacterStyle.FromShapeStyle` reads the two independently and
`AFontReferenceWithNoThemedIndexStillCarriesItsColour` asserts it, citing the same lines for the
opposite reading. That predates this round and is **not measured against the running binary** —
the tree here is a development branch and the reference is 24.2.7.2. No corpus deck states
`idx="none"` with a colour, so nothing here turns on it.

### `a:pattFill` on a table cell — measured, bounded, and the best-placed lead left

`171128IPAP.pptx` was carried into this round as "residue of round ten's fix" at 45.05
unaccounted ink, 18 of 40 pages major. Its page 24 is a five-column table whose data rows the
reference paints in a green, an orange and a red **crosshatch** and we paint white. Sampled at
512 px over one cell: the reference's commonest colour there is `(221,238,224)` at 40% of the
region — the fill's own `bgClr` of `DDEEE0` — and ours is `(255,255,255)` at 100%.

The markup names it outright, 18 times on that one slide:

```xml
<a:pattFill prst="pct25">
  <a:fgClr><a:srgbClr val="00B050"/></a:fgClr>
  <a:bgClr><a:srgbClr val="DDEEE0"/></a:bgClr>
</a:pattFill>
```

`PptxSlideLayout.cs:1156` says so in as many words — "Four of DrawingML's six kinds.
`a:pattFill` is left unpainted" — so this is a known gap rather than a discovery. What was
missing is its size.

Reach, parsed from all 112 corpus `pptx`: **65 pattern fills across 7 decks**, and three of
those decks are in the track's ink top twenty — `NAS-Infrastructure-Roadmaps-v16.0.pptx`
(368.41, nine of them), `171128IPAP.pptx` (45.05, eighteen) and
`B2B-Center-Readiness-and-Student-Retention.pptx` (16.23, eighteen). Only **five** presets
appear: `wdUpDiag` 32, `pct25` 18, `pct5` 12, `pct50` 2, `wdDnDiag` 1. So five patterns cover
the whole corpus.

It is a feature rather than a fix — a pattern is a paint kind the drawing IR does not have, so
it touches `Paperless.Core/Graphics` and both sinks — but it is the best-bounded one left on
this track: five presets, a named location, and three of the largest documents behind it.

### The text-path gap has a `.ppt` instance too, and it is one page's worth of ink

`8.16_AOD_FINAL_Provider_Training_Presentation_9_2009.ppt` (28.32 unaccounted ink, 13 of 94
pages major, never examined) is **16.05 of that on page 59 alone** — over half the document in
one page. The reference bends "Do you know what these are?" around an arch; we lay it straight.

That is the same class as `016 FAAAIandtheArtandScienceofV&Vfinal.pptx`'s `a:prstTxWarp`, which
this file already records as absent from `dotnet/src` — but through the binary path's WordArt
rather than through DrawingML, so a fix for one does not automatically reach the other. Worth
knowing before anyone scopes `prstTxWarp` as a `pptx`-only feature.

### The symbol-font bullet, and why the obvious fix draws tofu

Round eleven named this on `Framing Europe.ppt` — "the reference draws its bullets as a
Wingdings filled square 13.7 pt wide and we draw a 6.3 pt `•`". It is on
`2015-Civil-Rights-Website-training.ppt` too, where the reference draws a Wingdings arrowhead
and we draw a dot, on every bullet of 94 pages.

Three facts, so the next attempt starts from the right place:

1. **The reference keeps the private-use code point.** `pdftotext` on the reference's page 39
   extracts `U+F0D8` for each bullet, and `pdffonts` reports it drawn from a subset of
   **OpenSymbol**, which is installed here.
2. **OpenSymbol's own `cmap` does not contain `U+F0D8`.** Parsed from
   `/usr/share/fonts/truetype/libreoffice/opens___.ttf`: its whole `F000`–`F0FF` coverage is
   ten code points, `F030`–`F039`, and `F0D8`, `F0A7`, `F0FC` and `F075` are all absent. So
   asking a shaper for `U+F0D8` in OpenSymbol gets `.notdef`.
3. Therefore LibreOffice is going through a **recode table** — its `ConvertChar`/StarSymbol
   machinery — from the symbol face's byte to an OpenSymbol *glyph*, and writing the original
   code point back into the PDF's `ToUnicode`. Porting that table is the work; drawing
   `U+F0D8` in OpenSymbol is not a shortcut, it is tofu.

`PptxTextBody.Symbolised` and `OutlineNumbers.NormaliseBullet` are where our side ends: we map
the byte to `0xF000 | (c & 0xFF)` exactly as LibreOffice does and then fall back to `U+2022`
because nothing can draw the result. Point 3 is what would let that fallback go.

### A `grep -c` guard that fires on a missing file, met in the wild

The skill's note about `grep -c` reporting a count where a verdict is wanted has a second form
worth writing down, because it cost a wait here. This wait loop declared both runs finished
after four minutes:

```sh
fd=$(grep -c '^TOTAL' "$log" 2>/dev/null || echo 0)
[ "$fd" != "0" ] && ...
```

`grep -c` on a **missing** file prints its own `0` on stdout *and* exits non-zero, so the `||`
branch runs too and `fd` is the two-line string `"0\n0"` — which is not equal to `"0"`, so
every test on it is true. The failure is silent and looks exactly like success. Use `grep -q`
and set a flag, and never take a count where a verdict will do.

## Slides, round twelve measured: 1635.50 -> 1567.96 unaccounted ink, 538 -> 478 major pages

Whole track swept three times at 163 documents each — a baseline at `5ec407cf3`, the
`a:fontRef` fix alone, and all three changes — rendering **our** side each time and reusing the
baseline's reference PDFs. Nothing this round touches `soffice`, and every sweep's reference
column is the base sweep's file by construction.

| | baseline | `a:fontRef` | + trailing break | + `.ppt` U+2028 |
|---|---|---|---|---|
| matching the word gate | 152/163 | 152/163 | 152/163 | **152/163** |
| pages with a correct count | 163/163 | 163/163 | 163/163 | **163/163** |
| documents the reference could not render | 0 | 0 | 0 | **0** |
| total unaccounted ink | 1635.50 | 1583.80 | 1581.45 | **1567.96** |
| pages the image tool calls major | 538 | 502 | 501 | **478** |
| documents whose ink moved | — | 21 | 23 more | **49 in all** |

The baseline reproduced round eleven's published figures — 152/163, **1635.50** against the
recorded 1635.52, **538** major pages, every page count exact — which is the tell that the base
and the instrument are both right.

```
 37.89 ->  8.44   Sector_Skills_Insights…pptx                     major 11 ->  3
 31.84 -> 22.94   2015-Civil-Rights-Website-training.ppt          major 28 ->  9
 16.43 -> 13.03   southern-classic-kennesaw-state-university.pptx major 13 ->  7
 12.88 -> 10.62   Intersil_Italy_CAN_Bus_Transceiver…pptx         major  4 ->  5
 11.10 ->  2.18   Course Selection 2025-26 Current Grade 09.pptx  major  4 ->  0
  6.34 ->  3.50   FAAAIandtheArtandScienceofV&Vfinal.pptx         major 10 ->  0
  5.13 ->  2.19   Inducement-to-Insurance-Business.ppt            major  2 ->  0
  4.18 ->  0.37   Ensemble-pour-l-amelioration…AIRBUS.pptx        major  3 ->  0
 16.09 -> 20.54   16 - UTM - (NASA).pptx                          major  3 ->  5
```

Of the 49 that moved, **35 fell and 14 rose by 6.03 between them**; `16 - UTM - (NASA).pptx`
is 4.45 of that 6.03 and is discussed above.

**Six of the 163 rows moved a column and none changed verdict or page count.** Four are word
counts inside the 2% band. One is `16 - UTM`'s font count. The sixth is the round's clearest
result: `2015-Civil-Rights-Website-training.ppt` goes **6156/6145 to 6145/6145, an exact
match**, because the vertical tabs it ends every bullet with had been drawn as `\n` and are now
U+2028.

### Per batch, and batch-014

001 9/9 · 002–004 10/10 · 005 9/9 · 006 10/10 · 007 10/10 · **008 9/10** · 009 10/10 ·
**010 8/10** · 011 10/10 · **012 8/10** · 013 10/10 · **014 7/10** · 015 10/10 · **016 8/10** ·
**017 4/5**. Identical to the baseline in every batch.

Batch-014's three failures are `Thailand17.ppt` and `N2_E_Maestroni_Swarm_COP.pptx` — both on
`TODO.raster-ceiling.md` — and `WiGr_2021W_1_…pptx`, the `mc:Choice Requires="a14"` deck whose
`mc:Fallback` is a picture of its own text. **There is no third, winnable failure there.** The
brief for this round said "batch-014's two non-ceiling failures"; there is one non-ceiling
failure and matching it means drawing a picture instead of text, which this file already says
not to do. Recorded so the next brief does not send anyone after it again.

### The regression guards

Two of the four changed files are below the presentation family — `Paperless.Text/Layout`
(the trailing-break rule) — so the other two tracks were swept.

| | |
|---|---|
| `slides/batch-001`–`017`, whole track | **152/163**, every page count exact, 0 `ref-failed` |
| `words/batch-001`–`005` | **50/50** |
| `sheets/batch-001`–`005` | **49/50** |

**The sheets failure is not this round's, and it is checked rather than assumed.**
`sheets/batch-005/xls/Praktikastellen_-_chinesischsprachiger_Kulturraum.xls` reads 34/34 pages
and **2019/1828 words**, which is exactly the document the `render-comparison` skill describes
under "a word count can fault output that is geometrically perfect": the whole excess is
`http://www.` extracting as its own token 48 times, because our PDF sink caps a show operator
at 28 glyphs where LibreOffice writes one `TJ` array. Rendered with the *base* CLI snapshot and
the final one, it comes back **34 pages and 2019 words both times** — identical, so the change
did not cause it. The 50/50 in the record predates that defect.

Per project on the final tree, each run redirected to its own file, 0 skipped everywhere:
Core 238, Text **196** (was 185), Containers 109, Rendering 104, Markup 259, OpenDocument 125,
WordProcessing 542, Spreadsheets 401, Presentations **488** (was 485), Vector 291, Fidelity 538.
The two that moved are the fourteen tests added and nothing else.

Two fixtures, each verified by putting its bug back: `slide-trailing-break.pptx` and
`trailing-line-break.fodt`. `deck-text-style.pptx` already existed and needed no change — its
five styled boxes are exactly the `a:fontRef` ordering cases.

### What the next agent on this track should take

```
368.41  77 of 137 major  NAS-Infrastructure-Roadmaps-v16.0.pptx   (linked Excel OLE, known)
 66.12   1 of  30        N2_E_Maestroni_Swarm_COP.pptx            (the Gantt)
 56.67  10 of  41        Wildlife for REDAC September 11.pptx     (circle gradient, do not)
 49.65  18 of  54        Thailand17.ppt
 48.41   6 of 268        Reporting_responsibilities_matrix.pptx   (467 fontRefs, moved 0.00)
 45.11  18 of  40        171128IPAP.pptx                          (a:pattFill — see above)
 28.28  13 of  94        8.16_AOD_FINAL_Provider_Training…ppt     (p59 alone is 16.05: text path)
 26.67   6 of  10        Demick_JetBlue.pptx                      (chart label density)
 25.95  12 of  47        ITE106-Chapter 4.ppt                     (unlooked at)
 25.03  10 of  52        ghgp-supply-chain-initiative…pptx
 23.99  10 of  24        Framing Europe.ppt                       (bullet glyphs — see above)
 22.94   9 of  94        2015-Civil-Rights-Website-training.ppt   (residue: bullet glyphs)
 21.34   5 of  11        NAS-Infrastructure-Roadmaps-Weather.pptx (unlooked at)
 20.54   5 of  37        16 - UTM - (NASA).pptx                   (rose this round)
```

`a:pattFill` is the best-bounded of these — five presets, 65 fills, 7 decks, three of them in
this list. `ITE106-Chapter 4.ppt` and `NAS-Infrastructure-Roadmaps-Weather.pptx` have never
been looked at.

## Words, round thirteen — baseline at `1aefcdfdb`

Swept whole-track against a checksummed CLI snapshot, two workers, 200 of 200 rows and no
path twice.

| | |
|---|---|
| match | **145/200** |
| total absolute page error | **121** |
| documents with an exactly correct page count | **154** |
| total absolute word error | **7095** |

Per batch: `001`–`005` 10/10, `006` 9/10, `007` 9/10, `008`–`009` 10/10, `010` 7/9,
`011` 8/10, `012` 8/10, `013` 5/9, `014` 3/10, `015` 4/10, `016` 7/10, `017` 5/10,
`018` 4/10, `019` 3/10, `020` 3/10, `021` 0/2.

**The brief said 146/200, page error 120, 155 page-exact, and this is one document short of
it** — `015` and `019` each one lower than recorded and `018` one higher, which nets to −1
and is the shape of a document that renders differently under load rather than of a record
being wrong. Everything else reproduced exactly.

### The round, swept whole three times

Each sweep against a checksummed CLI snapshot, two workers, 200 rows and no path twice.

| | base `1aefcdfdb` | after the spacing fix | after the metrics fix |
|---|---|---|---|
| match | 145/200 | **146/200** | 146/200 |
| total absolute page error | 121 | **120** | 120 |
| pages exactly correct | 154 | **155** | 155 |
| total absolute word error | 7095 | **7083** | 7083 |

`batch-007` 9/10 → **10/10**; every other batch unchanged, and the gate `001`–`006` still
10/10 apart from `006`'s known row-split document. Two documents changed state across the
whole round and both improved. The second sweep moved nothing at all, which is the honest
result for a change whose corpus reach is one document that it does not flip.

Test counts after: Core 238, Text 196, Containers 109, Rendering 104, Markup 259,
OpenDocument 125, WordProcessing **559** (553 plus this round's six), Spreadsheets 410,
Presentations 488, Vector 291, Fidelity 538, 0 skipped in every one. Nothing outside
`Paperless.WordProcessing` was touched, so the other two tracks were not swept.

### `batch-007` is 10/10: a style that states half of `w:spacing` freezes the other half

`final-technical-report-template.docx` was five pages against six, and the reference's sixth
page holds nothing but the running head and the footer — eight trailing empty paragraphs
overflowing. Every one of its `Heading1` paragraphs sits 12 pt lower in the reference than in
ours, at the top of a page as well as in the middle of one, and the style states no space above
at all: `<w:pPr><w:numPr…/><w:spacing w:after="240"/><w:outlineLvl w:val="0"/></w:pPr>`, based
on the file's own `Heading2`.

LibreOffice keeps a paragraph's two vertical margins in **one** item, `SvxULSpaceItem`, while
writerfilter sets them through two separate UNO properties. Setting one is therefore a
read-modify-write of the pair: the importer takes what the style resolves to at that moment,
replaces the half the file states, and writes **both** back as *direct* values. Styles are
applied in the order `styles.xml` declares them, so a parent declared further down has not had
its own definition applied yet and is still sitting at Writer's pool default for the built-in
style its `w:name` names.

Measured on LibreOffice 24.2.7.2 rather than argued, with the parent stating
`w:before="480"` as a control:

| child states | parent | parent declared | space above the child |
|---|---|---|---|
| `w:after` only | `heading 2` | after the child | **12 pt** — never the 480 |
| `w:after` only | `heading 2` | before the child | 24 pt — the 480, inherited normally |
| `w:after` only | a custom style | after the child | **nought** — a suppression, not a fall-through |
| `w:after` only | a custom style | before the child | 24 pt |
| nothing | `heading 2` | after the child | nought — the read-modify-write never happens |
| `w:before` only | `heading 2` | after the child | 6 pt *below*, symmetrically |

The pool values, each one rendered rather than read off the source: **12 pt / 6 pt** under
`heading 1`–`heading 9`, `Title` and `Subtitle`; **0 / 7 pt** under `Body Text` and `List`;
**6 pt / 6 pt** under `caption`; nought for everything else including every custom style. They
line up with `DocumentStylePoolManager.cxx:810` (the `Heading` base), `:699` (`Text body`) and
`:974` (`Caption`), which is the check that the measurement describes a rule rather than a
coincidence — and worth stating, because the same file's `bNoDefault` guard says these defaults
should not apply at all, and they demonstrably do.

`WordStyles.cs:255` is the completion pass and `WriterPoolSpacing.cs:52` the table. Fixture
`tests/corpus/features/style-one-sided-spacing.docx`, four styles differing in one thing each,
whose nine rendered baselines match LibreOffice's to a tenth of a point.

**Reach, measured by rendering rather than by grepping.** Fifteen of the 134 words-track DOCX
declare a one-sided `w:spacing` against a parent declared later, and only six of those name a
parent Writer has a non-zero pool default for. Two documents changed on the sweep: this one, and
`016 AFS-050-004-F2_0i.docx` by one word. Nothing regressed.

### `w:usePrinterMetrics` was recorded as inert on DOCX, and is not

`WordCompatibility` listed it under "identified and inert — headless LibreOffice ignores it".
`DomainMapper_Impl::ApplySettingsTable` (`DomainMapper_Impl.cxx:10173`) sets
`PrinterIndependentLayout::DISABLED` from it, which is the same state `WW8Dop::fUsePrinterMetrics`
puts a DOC into and which `DocReader.cs:202` has honoured all along. Line pitch, LibreOffice
against ours, over seven face and size pairs:

| face, size | ref off | ref on | ours off | ours on |
|---|---|---|---|---|
| Calibri 11 | 13.45 | 13.45 | 13.45 | 13.45 |
| Calibri 10 | 12.20 | 12.25 | 12.20 | 12.25 |
| Arial 10 | 11.50 | 11.55 | 11.50 | 11.55 |
| Arial 12 | 13.80 | 13.95 | 13.80 | 13.95 |
| Times New Roman 12 | 13.80 | 13.95 | 13.80 | 13.95 |
| Cambria 11 | 12.65 | 12.70 | 12.65 | 12.70 |
| **Times New Roman 10** | **11.55** | 11.55 | **11.50** | 11.55 |

The last row is a separate defect and nearly cost this one: our printer-independent pitch for
10 pt Liberation Serif is 0.05 pt short, so on that pair alone both packages come out at 11.55
and the flag reads as doing nothing. The first fixture written for this was that pair, and it
said the change was compensating for an unrelated error. It is 12 pt Arial now.

**Reach is one document** — `batch-010/docx/195584360.docx`, the only words-track DOCX that
states the flag. Its body geometry is now exact against the reference on every line of page 2,
where it drifted 0.1 pt a line before. It is still 19 pages against 20, so this did not move the
match count; what is left of it is **0.5 pt lost at each table row**, which accumulates to 2 pt
a page. Kept on the evidence of the seven-pair sweep rather than on a corpus number.

### What the next agent on this track should take

1. **`batch-010`'s `195584360.docx` is now one defect away**, and the defect is named: entering
   a table row costs 0.5 pt against the reference, roughly twenty times a page. Its horizontal
   twin is in the same place — table cell text starts at x = 75.25 in the reference and 70.20 in
   ours, a flat 5.05 pt, on a document whose `w:tblPr` states `w:tblInd w:w="108"` and which has
   **no `compatibilityMode` at all**, so `DocxLayoutSource.Tables.cs:218` takes the
   measure-to-the-text branch and subtracts the padding. The reference plainly did not.
2. **Our 10 pt Liberation Serif line pitch is 11.50 against LibreOffice's 11.55**, printer
   independently. One row of the table above; no other size or face measured wrong. Cheap to
   reproduce with `research/probes/words-r13/probe-grid.py`.
3. `batch-010`'s `5709.16 ch.40_mgfinal.docx`, 31 pages against 32, untouched again.
4. Everything the twelfth round listed and this one did not reach: the `mcar` family at 0/5,
   `WritingFieldKind.PageNumber` (still no consumer, and still a feature rather than a wiring
   change), the footer half of the title-page rule, and `batch-006`'s row-split document.
## Slides, round thirteen: 1567.96 -> 1408.90 unaccounted ink, 478 -> 465 major pages

Whole track swept **five** times at 163 documents each — a baseline, then one sweep per change —
rendering our side each time against a checksummed CLI snapshot and reusing the baseline's
reference PDFs, since nothing this round touches `soffice`. The baseline reproduced the brief
**to the digit**: 152/163 on the word gate, **1567.96** ink, **478** major pages, every page
count exact, 0 `ref-failed`.

| | baseline | `a:pattFill` | + cell `a:noFill` | + hatch phase | + empty-paragraph bullet |
|---|---|---|---|---|---|
| matching the word gate | 152/163 | 152/163 | 152/163 | 152/163 | **152/163** |
| pages with a correct count | 163/163 | 163/163 | 163/163 | 163/163 | **163/163** |
| documents the reference could not render | 0 | 0 | 0 | 0 | **0** |
| total unaccounted ink | 1567.96 | 1566.38 | 1408.48 | 1408.51 | **1408.90** |
| pages the image tool calls major | 478 | 479 | 465 | 465 | **465** |
| documents whose ink moved | — | 4 | 4 more | 1 | **30 more** |

**Four of the 163 gate rows moved and none changed verdict, page count or unembedded-font
count.** Three decks now embed one or two fewer faces, because a bullet the reference does not
draw was the only thing asking for that face; one word count went 6180 to 6179.

### The headline number was not what it said it was

`NAS-Infrastructure-Roadmaps-v16.0.pptx` is 23% of the track's ink and had been carried for two
rounds as "linked Excel OLE, known". **Splitting its per-page ink by whether the page carries one
is what unstuck it**: 152.12 on the 24 pages that do and 216.29 on the other 113. The second half
had never been looked at because the attribution covered the whole document.

- The 152.12 **is** a ceiling and now has a named, verified mechanism rather than a label —
  `mc:Choice Requires="v"` around a linked `p:oleObj` with no local replacement, which
  LibreOffice takes (`oox/source/core/contexthandler2.cxx:238-249` lists `v`) and then draws
  nothing for. We take the `mc:Fallback` and draw its EMF of the table's data. Ours is the better
  output and the specification is on our side, since we have no VML reader at all. Written up in
  `TODO.raster-ceiling.md`.
- The 216.29 was a defect: **a table cell's own `a:noFill` was read as silence** and lost to the
  table style's banding. That document's slide layout carries a seventeen-column year ruler under
  `Medium Style 2 - Accent 1` with `a:noFill` on every cell, so all 137 pages were ruled in the
  theme's green — `#00cc99` sixteen times and `#ccecdd` six on page 53, against a reference that
  draws neither, while the colours the *slide* states matched on both sides. `tablecell.cxx:550`
  is the rule: the style part's fill is built and the cell's own is laid over it with
  `assignUsed`, which copies `moFillType` whenever the cell stated one.

That fix alone is **−157.90 over four documents and nothing up**: NAS `v16.0` 367.92 → 225.33
(77 major pages → 66), `…-Weather` 21.34 → 13.16, `…-HSI` 9.64 → 2.63, `PAL Block Intro 2023`
2.14 → 2.02. Reach parsed from all 112 corpus `pptx`: 8 decks, 215 cells.

### `a:pattFill` is a hatch, not a pattern — and it is worth 4.85, not 45

The lead was the best-bounded thing left and it paid the least. The gap was never "a pattern
bitmap we cannot synthesise": LibreOffice does not draw those bitmaps either.
`oox/inc/drawingml/hatchmap.hxx` maps each of the fifty-four `ST_PresetPatternVal` presets onto a
`drawing::Hatch` — a style, a distance and an angle — and `fillproperties.cxx:755-783` pushes
that beside the background colour, so the reference *is* the hatch and drawing the real bitmap
would move us away from it. The whole table is ported.

LibreOffice's own flat-ODF export of the new fixture confirms all five presets the corpus uses,
to the digit: `pct25` `double 0.2cm 450`, `pct5` `single 0.25cm 450`, `wdUpDiag`
`single 0.1cm 450`, `wdDnDiag` `single 0.1cm 1350`, `pct50` `double 0.125cm 450`, each beside a
`draw:fill-color` holding the `a:bgClr`.

```
 45.11 ->  41.40   171128IPAP.pptx                (page 24 alone 5.59 -> 1.88)
 10.29 ->   9.64   NAS-Infrastructure-Roadmaps-HSI.pptx
368.41 -> 367.92   NAS-Infrastructure-Roadmaps-v16.0.pptx
  0.47 ->   3.74   BMFE-06-03 (Gerflor) Smoke Density and Toxicity.pptx
```

**Three of the seven decks do not move at all**, and knowing why saves the next agent the search:
`B2B-Center`'s eighteen fills are all on picture placeholders in *slide layouts* whose slides
fill them with pictures; `airbus-…-diy`'s two are on `a:rPr`, a run's text fill, deliberately
left alone because painting a hatch there would make the text invisible; `3492`'s one is a single
shape on one slide.

**The `BMFE` cost is a measurement artefact and was chased far enough to say so.** Its page 3 is
the deck's one hatched column. Measured at 80 dpi across it, **49–50 crossings per raster row on
both sides** — the spacing and the count are right — and per-row ink 49 against 58, so our
hairlines are the lighter of the two. The tool's own report names what happened: the page went
from 42 small `shifted` regions to 41 with one covering 25% of the page, so the new pixels merged
pre-existing text drift into a single region and the imbalance is measured over all of it. The
page is visibly closer than the blank column it drew before.

**The hatch phase was the obvious suspect and it was wrong.** `GeoTexSvxHatch` anchors a family
on the grown box's leading edge — `fround(H/distance + 0.5)` steps, a line at `a × distance` for
`a` from 1 — where we centred it, which is up to half a step out on every line. Reproducing it
exactly took BMFE's page 3 from **3.28 to 3.25**, and the whole track from 1408.48 to 1408.51.
Kept because it is what LibreOffice draws and costs nothing, not because it fixed anything.

### An empty paragraph draws no bullet: right, wide, and worth 0.00

Both of LibreOffice's presentation readers say so in their own comments and on the same condition
— the paragraph's own character count: `oox/source/drawingml/textparagraph.cxx:193`, *"empty
paragraphs do not have bullets in ppt"*, and `filter/source/msfilter/svdfppt.cxx:2363`, *"in PPT
empty paragraphs never gets a bullet"*. `SlideTextLayout.Shaped` guarded on the paragraph having
*runs*, and an empty paragraph has one — carrying its height.

Reach, counting extracted lines holding nothing but a bullet glyph: **75 of the 163 documents
drew more than the reference, 2405 lines in all** — 293, 185, 170, 129, 127, 119 on the worst
six. **The word gate cannot see any of it**: `wc -w` in the POSIX locale ignores a token made of
non-ASCII bytes alone, so `2015-Civil-Rights-Website-training.ppt` reads 6145/6145 exactly while
drawing 293 bullets the reference does not.

And the ink metric barely sees it either: **+0.39 across 30 documents**, 18 down and 12 up, no
verdict and no major-page count moved. Only `.ppt` documents move, because `PptxTextBody` already
declines a marker for a paragraph with no text. **Kept because it is right on its own evidence** —
the deck that moved most, `ITE106-Chapter 4.ppt`, went from 119 bare bullets to none and from
25.95 ink to 26.62, its residue being a font-size difference the bullets were sitting inside.

### No document fixture could be made for it, and that was measured

A four-paragraph probe was authored as `.pptx` and converted to `.ppt` through LibreOffice.
**Both forms render identically with the rule and without it**, for two separate reasons: our
PPTX reader already declines the marker (`PptxTextBody`, `if (!hasText) return null`), and
LibreOffice's own PPT export writes those empty paragraphs with no bullet at all. The corpus
decks that show it are PowerPoint's own output. Both fixtures were deleted rather than committed
as green files that prove nothing, and the assertion is on `SlideTextLayout.Place` with a body
built in the test.

### The regression guards, and the tests

Five of the ten files changed are below the presentation family — `Paperless.Core/Graphics`
(`HatchPaint` and `Hatching`) and `Paperless.Ooxml/DrawingML` (`DrawingFill`, `DrawingHatch`,
`DrawingHatchPresets`, `DrawingTableGeometry`) — so the other two tracks were swept.

| | |
|---|---|
| `slides/batch-001`–`017`, whole track | **152/163**, every page count exact, 0 `ref-failed` |
| `words/batch-001`–`006` | **59/60** — the one failure is `batch-006`'s `f445896eb008d14c1746fc37d412dc22.docx` at 15/16 pages and 5575/5575 words, which the words round-twelve entry above takes apart at length. It is the recorded state of that batch, not this round's |
| `sheets/batch-001`–`006` | **59/60** — the one failure is `batch-005/xls/Praktikastellen_…xls` at 34/34 pages and 2019/1828 words, the `Tj`-granularity artefact in the shared PDF sink that round twelve measured on both its base and its final binary and found identical |

Per project, each run redirected to its own file, 0 failed and 0 skipped everywhere:
Core **243** (was 238), Text 196, Containers 109, Rendering 104, Markup 259, OpenDocument 125,
WordProcessing 553, Spreadsheets 410, Presentations **497** (was 488), Vector 291, Fidelity 538.
The two that moved are the fourteen tests added and nothing else.

One fixture: `slide-pattern-fill.pptx`, five presets plus a preset-less pattern plus a table
whose cells state their own, every expectation read out of LibreOffice's flat-ODF export of it.
Each test verified by reintroducing its bug — and one assertion was *dropped* for failing that
check: flipping the sign of a hatch's second family changes nothing, because a line at plus and
minus ninety degrees is the same line.

### What the next agent on this track should take

```
225.33  66 of 137 major  NAS-Infrastructure-Roadmaps-v16.0.pptx  (152.12 of it is the OLE ceiling)
 66.12   1 of  30        N2_E_Maestroni_Swarm_COP.pptx           (the Gantt)
 56.67  10 of  41        Wildlife for REDAC September 11.pptx    (circle gradient, do not)
 49.65  18 of  54        Thailand17.ppt
 48.41   6 of 268        Reporting_responsibilities_matrix.pptx  (467 fontRefs, moved 0.00)
 41.40  18 of  40        171128IPAP.pptx                         (residue of the pattFill fix)
 28.31  13 of  94        8.16_AOD_FINAL_Provider_Training…ppt    (p59 alone is 16.05: text path)
 26.67   6 of  10        Demick_JetBlue.pptx                     (chart label density)
 26.62  12 of  47        ITE106-Chapter 4.ppt                    (autofit — see below)
 25.03  10 of  52        ghgp-supply-chain-initiative…pptx
 23.99  10 of  24        Framing Europe.ppt                      (bullet glyphs: the recode table)
 22.86   9 of  94        2015-Civil-Rights-Website-training.ppt  (bullet glyphs)
 20.54   5 of  37        16 - UTM - (NASA).pptx                  (rose in round twelve)
 16.82   2 of  55        ws_prod-g-doc-Events-2007-september-M.017…ppt
```

1. **`ITE106-Chapter 4.ppt`'s text is set smaller than the reference's**, and it is now the clean
   case for it: with the bullets gone its text layer matches the reference line for line, and its
   page 11 still breaks *later* than the reference's on every bullet. That is an autofit or a
   font-size resolution difference on a `.ppt`, with 47 pages of instances.
2. **The symbol-font bullet recode table**, unchanged from round twelve's write-up and now the
   largest named item: `Framing Europe.ppt` and `2015-Civil-Rights-Website-training.ppt` are
   46.85 between them. `unotools/source/misc/fontcvt.cxx` holds the tables — `aWingDingsTab[224]`
   maps `U+F0D8` to `U+E49E`, which is a glyph OpenSymbol does have, and
   `aStarSymbolRecodeTable` names the face-to-table mapping. Porting it is mechanical.
3. **`8.16_AOD`'s page 59**, 16.05 of its 28.31 in one arched WordArt — the `.ppt` twin of
   `a:prstTxWarp`, so scoping that feature as pptx-only would miss it.
4. **`Reporting_responsibilities_matrix.pptx`**, 48.41 over 268 pages and untouched by two rounds
   of `a:fontRef` work. Nobody has looked at a page of it.

## Slides, round fourteen: the symbol-font bullet recode — ink flat, embedded faces 72 → 94

The baseline reproduced the brief **to the digit** — 152/163 on the word gate, **1408.90** ink,
**465** major pages, every page count exact, 0 `ref-failed` — which is the tell that the base
commit and the instrument are both right. (The worktree was 353 commits behind when the round
opened and was fast-forwarded before anything was measured. Every recent agent has hit this.)

Whole track swept twice at 163 documents, against a checksummed CLI snapshot, reusing the
baseline's reference PDFs since nothing this round touches `soffice`.

| | baseline | after the recode |
|---|---|---|
| matching the word gate | 152/163 | **152/163** |
| pages with a correct count | 163/163 | **163/163** |
| documents the reference could not render | 0 | **0** |
| total unaccounted ink | 1408.90 | **1409.81** |
| pages the image tool calls major | 465 | **466** |
| embed exactly the reference's face count | 72 | **94** |
| total absolute face-count error | 107 | **83** |

**The metric this round was asked to drive did not move: ink is +0.91, 0.06%, on a change
reaching 83 of the track's 163 documents.** 62 documents moved, 37 down by 7.34 and 25 up by
8.25. That is stated first because it is the honest headline; what follows is the case for
keeping the change anyway, and it rests on three measures that did move.

### What the change is

Both readers already did LibreOffice's first step — a symbol face addresses glyphs by byte
position, so `0xD8` in Wingdings means "slot 0xD8", and both moved it into the Private Use Area
as `(c & 0x00ff) | 0xf000`. Neither did the second. Nothing could draw `U+F0D8`, so it was
collapsed to `U+2022`: a 6.30 pt dot where the reference draws a 13.72 pt arrowhead, on every
bullet of every affected deck.

`unotools/source/misc/fontcvt.cxx` holds the missing half — ten 224-entry tables mapping each
face's slots to the code point holding the *same picture* in OpenSymbol, which ships with
LibreOffice and is therefore always installed. `scripts/generate-symbol-recode.py` reads them,
as `generate-font-substitutions.py` reads `VCL.xcu` and for the same reason: nothing else states
the mapping.

Reach, measured before writing anything: **83 of 163 documents** name a recodeable face — 51
`pptx` on an `a:buFont` and 32 `ppt`. Of the 900 concrete bullet uses in the `pptx` half,
**876 (97%) recode to a code point OpenSymbol actually has**, none hit a table hole, 8 do not
(one deck's Webdings `a`) and 16 are outside the symbol range entirely.

### The three measures that did move

1. **Embedded faces, and this is the strongest of the three.** The gate's third check is
   independent of both the word count and the ink figure, and it is precisely what this fix
   should move: drawing `U+2022` out of the body face embeds no OpenSymbol and drawing the
   recoded glyph does. **72 → 94 documents embed exactly the number the reference does**; 25 are
   closer and one is further. Twenty-four of them go from *N*−1 to exactly *N*.
2. **Glyph geometry, against LibreOffice's own rendering.** `Framing Europe.ppt` page 12: our
   bullet was 6.30 pt wide at x = 61.09 against the reference's 13.72; it is now **13.73 at the
   same pen**, and a 150 dpi row profile across the bullet band reports **zero differing pixels**.
   On the new fixture the recoded advances agree to a hundredth of a point — `F0D8` 15.94 against
   15.93, `F0FC` 16.56 against 16.57, `F06E` 15.25 against 15.25.
3. **Raw pixel difference.** Summed per page, which unlike `ink%` is not affected by how the tool
   groups pixels into regions, it **falls on every document examined in detail**: Framing Europe
   221.61 → 220.29 with all 23 moved pages closer and **none further**, 2015-Civil-Rights
   752.65 → 748.75, 171128IPAP 424.23 → 423.50.

### Why the ink figure can rise while the pixels fall

Worth writing down, because it is a property of the instrument this track now steers by.
`pdf-image-diff.py` computes a page's figure as `abs(sum over regions of luma_gap × area)`
(`pdf-image-diff.py:417-420`). **The sum is signed and the absolute value is taken over the
total**, so regions of opposite sign cancel before the magnitude is read — and removing a deficit
can therefore *raise* the number when it was cancelling a surplus.

That the bullet regions were doing exactly that is **inferred, not measured**: the direct check
wants the base binary's renderings, which had been deleted to free disk. What is measured is
consistent with it and does not prove it — on Framing Europe page 12 the region count falls
14 → 12, which is the two bullets, every remaining region's extent is byte-identical, and `diff%`
falls 13.84 → 13.78 while `ink%` rises 1.38 → 1.40.

The worst riser is `2015-Civil-Rights-Website-training.ppt` at +2.24, and it is a document with
an independently wrong line pitch: ours 36.14 pt against the reference's 41.73/37.64, **unchanged
by this commit**, so a correctly-sized bullet now sits in a place that was already drifting. Round
twelve measured that autofit residue and it is still open. This is the "fixes that cancel" shape
the skill describes for page counts, showing up in the ink metric.

### Two defects in this round's own wiring, both found by a test rather than by reading

Recorded because the pattern is the useful part: each was a plausible reading that worked on
most inputs.

- **The pptx bullet was normalised before the table could see it.** `PptxTextBody.Marker` applied
  `NormaliseBullet` at the call site, so a bullet stated as `char="&#xF0D8;"` became `U+2022`
  and was then re-symbolised into slot `0x22` — every Private-Use-Area-stated bullet in every
  deck recoding to the *same* wrong glyph, `U+E401`. It survived the first reading because the
  other three quarters worked: a bullet stated as a plain byte (`char="Ø"`) is untouched by
  `NormaliseBullet` and recoded correctly. **231 of the 900 pptx bullet uses are stated the first
  way.** Two tests differing only in how the same slot is spelled disagreed, which localised it.
- **The trigger was the resolved family rather than the face being absent.** Keying on "did this
  resolve to OpenSymbol" works for a face `VCL.xcu` gives a chain to — Wingdings names
  `opensymbol` fourth — and fails silently for one it does not. Nothing in that table mentions
  `monotypesorts` or `mtextra`, so those went to fontconfig, came back a text face, and fell
  through to `U+2022`. LibreOffice never asks fontconfig about a symbol font at all
  (`FcPreMatchSubstitution::FindFontSubstitute` returns false outright,
  `vcl/unx/generic/font/fontsubst.cxx:100-107`), which is why a missing chain costs it nothing.

A third trap is in the generator rather than the code: **a table hole is written as a bare `0`,
not `0x0000`**, so the obvious `0x[0-9a-f]+` regex drops it and shifts every later index —
`aWingDingsTab` then reads 223 entries and sends `U+F0D8` to `U+E49F` instead of `U+E49E`. Both
readings look entirely plausible. The generator asserts the length is 224 so the wrong one cannot
ship, and five of the unit tests fail against it.

### The fixture, and one assertion that was narrowed rather than weakened

`tests/corpus/features/slide-symbol-bullet.pptx` — nine bulleted paragraphs chosen to separate
behaviours rather than repeat one: the same slot spelled both ways (the pair that caught the
first bug), the corpus's commonest slot, the same slot through two different faces, a table hole,
a symbol face with no table, and a non-symbol face. `pdf-image-diff.py` against LibreOffice's own
rendering of it: 1 page, **0 major**, ink 0.33.

One assertion claimed every recoded bullet is wider than the `U+2022` it replaces, and failed
correctly: `Symbol`'s `0xB7` and the table hole both recode to `U+E12C`, which is OpenSymbol's
*own bullet* and is **narrower** than Calibri's at the same size, 8.23 pt against 9.96. The claim
is false for the bullets that really are bullets. It now asserts the width over the five
pictorial markers and pins the two exceptions as exceptions.

Two of the fixture's nine still differ from the reference and both are understood: our
unknown-dingbat case draws `U+2022` where LibreOffice draws a glyph, having generic symbol
handling behind the table that we do not; and the reference's text layer reports every recoded
bullet by its *original* code point, because LibreOffice writes the slot into `ToUnicode` and
draws the OpenSymbol glyph, while ours reports the recoded point. Both are Private Use Area code
points meaning nothing outside one font, so no consumer is worse off and `wc -w` sees neither.

### `NAS-Infrastructure-Roadmaps-v16.0.pptx`: the split reproduces, and the remainder is diffuse

Re-derived from this round's own sweep rather than trusted. Splitting its 225.40 by whether the
slide carries a `Requires="v"` `p:oleObj`:

| | pages | ink | major |
|---|---|---|---|
| carrying one | 24 | **152.12** | 24 |
| everything else | 113 | **73.28** | 42 |

Both halves match the record (152.12 and 73.21). **So the ceiling attribution holds** — this is
one of the few predecessor claims on this track to survive an independent check with its sentence
intact.

The unexamined 73.28 was taken apart and is **not** a second discrete defect. Its worst pages
carry none of `graphicFrame`, `a:tbl`, `a:blipFill`, `a:pattFill`, `a:gradFill`, `dgm:relIds`,
`a:prstTxWarp` or `a:outerShdw` in any concentration, and the diff report calls 40–50% of each
one *"marks displaced or reshaped"* — a reflow spread over 113 pages at about 0.65 each, worst
page 4.27. The next instrument for it is the extraction comparison, not more pixels.

### What the next agent on this track should take

```
225.40  66 of 137 major  NAS-Infrastructure-Roadmaps-v16.0.pptx  (152.12 ceiling; the rest is reflow)
 66.12   1 of  30        N2_E_Maestroni_Swarm_COP.pptx           (the Gantt)
 56.72  10 of  41        Wildlife for REDAC September 11.pptx    (circle gradient, do not)
 49.65  18 of  54        Thailand17.ppt
 48.41   6 of 268        Reporting_responsibilities_matrix.pptx  (see below)
 41.70  18 of  40        171128IPAP.pptx
 28.10  13 of  94        8.16_AOD_FINAL_Provider_Training…ppt    (p59 alone is 16.05: text path)
 27.00   6 of  10        Demick_JetBlue.pptx                     (chart label density)
 26.62  12 of  47        ITE106-Chapter 4.ppt                    (autofit)
 25.10   9 of  94        2015-Civil-Rights-Website-training.ppt  (the line pitch, below)
 25.03  10 of  52        ghgp-supply-chain-initiative…pptx
 24.48  10 of  24        Framing Europe.ppt
```

1. **`2015-Civil-Rights-Website-training.ppt`'s line pitch is the best-located thing left.** Ours
   is 36.14 pt where the reference's is 41.73 then 37.64, measured off page 39 with
   `pdftotext -bbox`, and the reference sets the frame's text about 5.5% smaller than we do (the
   word "client's" 60.97 pt against our 64.54). Round twelve attributed this to an autofit shrink
   and fixed the trailing-break half of it; this is the residue, it is now the deck's whole 25.10,
   and it is what makes the correctly-sized bullet cost ink instead of saving it.
2. **`Reporting_responsibilities_matrix.pptx` was looked at and is a thin uniform difference**, not
   repeated furniture: 48.41 over 268 pages, only 6 of them major, `diff%` running 2–15% a page
   and `ink%` 0.03–0.2. It matches the word gate exactly. Note the tool prints region detail only
   for major pages, so grouping region signatures across a document only ever describes those.
3. **`8.16_AOD`'s page 59**, still 16.05 of its 28.10 in one arched WordArt — the `.ppt` twin of
   `a:prstTxWarp`, so scoping that feature as pptx-only would miss it.
4. **`…industrymeeting18112004-Aercap.ppt` now embeds 4 faces against the reference's 3**, the one
   document this round moved *away* on the font check. Small and precisely located.
5. **The word gate's 11 failures are unchanged and all still attributed** — 8 rasterisation
   ceiling, `mc:Choice Requires="a14"`, the linked-OLE ceiling, and `Sylva`. Nothing there is
   winnable without making the output worse.
## Words, round fourteen — `batch-010`, and a border that is a band

Baseline and result are both whole-track sweeps of batches 001–019, the second at
`491d95c37`:

| | baseline `22ed440e0` | after |
|---|---|---|
| full match | 143 / 188 | **144 / 188** |
| total absolute page error | 87 | **86** |
| exactly correct page count | 150 | **151** |
| total absolute word error | 4666 | **4654** |

`batch-010` is **7/9 → 8/9**, page error 2 → 1. Batches 001–009 are unchanged at 89/90, the
one miss being the known `batch-006` trap. **No other batch moved by a single document**, which
is worth stating because two of them appeared to and did not — see the baseline note near the
top of this file.

### A cell's border is a band its text may not enter

`TableLayouter` put every grid line exactly where LibreOffice puts it and still set every cell's
text half a border too high, which no comparison of the strokes could ever see. Writer insets a
cell's content by the whole border width on top of the padding — `SwBorderAttrs::CalcTop` asks
`SvxBoxItem::CalcLineSpace`, which adds the line's width to the distance
(`editeng/source/items/frmitems.cxx`:3717–3746) — and two rows share the band between them, so a
table of *n* rows stands *n+1* borders tall rather than *n*.

Measured on a one-column fixture at borders of 0, 1 and 2 pt and at one and three rows. With a
1 pt border LibreOffice's three rows sit at 84.99, 97.54 and 110.09 against 83.99, 95.54 and
107.09 unbordered, and the paragraph after the table at 122.64 against 118.64: one border of
inset at the top, four borders of height over three rows, and the same at 2 pt. `single` and
`dotted` behave identically, so the style is not in it.

`195584360.docx` went from 19 pages against 20 to an exact match on both pages and words
(7952/7952). It carries about fourteen bordered tables to the page and each was 1 pt short.

**The lever this exposes is small per table and there are a lot of tables.** The change is one
line of geometry and half a line of accounting; the rest of the commit is the fixture and the
reasoning.

### Three measurements on `195584360.docx` that the record should keep

All three by controlled variation on the document itself, LibreOffice against LibreOffice:

- **`w:usePrinterMetrics` is a horizontal effect too.** With the flag LibreOffice's advances run
  0.6% wider than ours over a 500 pt line; delete the flag and they agree to 0.3 pt. Our
  `MetricGrid` quantises only the *vertical* metrics (`LineSpacing.cs`:61). But LibreOffice
  still paginates the document at 20 pages with the flag removed, so this is **not** what cost
  the page — and exactly one DOCX in the whole words track states the setting, so it is not
  worth a shared-layer change on its own evidence.
- **A paragraph's `w:pBdr` takes height.** An empty paragraph carrying
  `bottom single sz=4 space=1` costs LibreOffice 1.50 pt we do not spend — the line's width plus
  `w:space`. There are 26 of them in that document. Unfixed.
- **The page-number field.** Every page of our render of that document says `Page 10`; this is
  the known `WritingFieldKind.PageNumber` gap, not a new one.

### `w:pict` and `w:object` take room, and the size they state is not the size to take

An inline VML shape — the legacy picture, and every embedded OLE object — reaches
`DocxLayoutSource.cs`:1016 as a bare anchor character and contributes no height at all. That it
*should* contribute is settled: deleting the `w:object` from `5709.16 ch.40_mgfinal.docx` moves
LibreOffice's own page one up by exactly **60 pt**, and reading the shape's stated size brought
our page one from 72 pt short of the reference to 1.65 pt.

The implementation was written, tested nine ways, swept, and **reverted**, because the stated
size is the wrong number:

| | border only | border + VML |
|---|---|---|
| match | 144 / 188 | 143 / 188 |
| page error | 86 | 87 |
| word error | 4654 | 4850 |

Four documents moved, one changed state and downwards: `eTAR_External_Web_tool_Tip_Sheet_mh.docx`
went 4/4 to 5/4 on a single `Excel.Sheet.12` object whose `v:shape` says 76.2 × 49.2 pt and
whose `DrawAspect="Icon"` means LibreOffice draws it smaller. `5709.16`'s own object says
73.8 pt and LibreOffice spends 60 — one line less, exactly.

So the next attempt starts from `DrawAspect` and from `oox/source/vml/vmlshape.cxx`'s handling
of `o:OLEObject`, not from the `style` attribute; and it should probably take only a `w:pict`
carrying a `v:imagedata` and leave `w:object` alone. Reach if it works: **20 documents in the
words track carry an inline VML shape**, and 33 more carry only floating ones, which must stay
out of the text.

### `batch-010`'s remaining document, characterised

`5709.16 ch.40_mgfinal.docx` is 31 pages against 32. With the border fix its page one agrees
with the reference to 2.65 pt once the OLE object is read, and every page after page 2 shows a
*constant* offset — no drift accumulates within a page. What remains is on page 2:

- **11.4 pt at the top**, before `Table of Contents`. That page's header is `header2.xml`,
  which is a three-row table with 3 pt `w:tblBorders` followed by one empty `FSMHeader`
  paragraph, and our reserved header height is 11.4 pt under LibreOffice's. The two candidates
  are the trailing empty paragraph and the table's outer border band; they were not separated.
- **About four lines** the reference declines to place at the bottom of its page 2 while
  filling its page 3 to 700 pt. Not explained.

The header/footer fields also differ — we print the `FILENAME` field's cached result
(`FSH_5709.16_40_DD_1_0`) where LibreOffice recomputes it to the file's name, and `Page 21 of 33`
where it says `Page 2 of 32`. Neither changes the page count.

## Words, round fifteen — `batch-011`, a row that starts late, and a distance between letters

Baseline and result are whole-track sweeps of batches 001–019. The baseline at `196774051`
reproduced round fourteen's figures **to the digit** — 144/188, page error 86, exact page count
151, word error 4654 — which is the tell that the base and the harness are both right.

| | baseline `196774051` | `w:gridBefore` | + character `w:spacing` |
|---|---|---|---|
| full match | 144 / 188 | 144 / 188 | **144 / 188** |
| total absolute page error | 86 | **85** | **85** |
| exactly correct page count | 151 | **152** | **152** |
| total absolute word error | 4654 | 4659 | 4815 |

`batch-011` is **8/10 → 9/10**, its page error 3 → 2 and its word error 73 → 46. `batch-012` is
unchanged at 8/10. One document went the other way and it is stated in full below: `batch-008`
is **10/10 → 9/10** on a word count, with its pagination untouched. Every other batch held.

**Read the two columns separately.** The first change is neutral-or-better on every metric. The
second wins a document, loses a document, moves no page anywhere, and costs 156 of word error —
all of it *over*-counting on documents whose geometry it demonstrably improved.

### A row need not start at the grid's first column

`w:gridBefore` says how many grid columns a row skips before its first cell, and DOCX writes no
placeholder for them — exactly as it writes none for the columns a `w:gridSpan` swallows.
`DocxLayoutSource.Tables.cs`:332 started every row at column zero, so a skipping row's first cell
took the *first* column's width and every cell after it was shifted and mismeasured.

LibreOffice reaches the right layout by materialising the skipped columns as borderless empty
cells (`TableManager::endRow`, `sw/source/writerfilter/dmapper/TableManager.cxx`:667–702). An
absent cell draws the same nothing, so shifting the column index is the same answer with nothing
to lay out. `w:wBefore` is deliberately not read: it is the skipped span's width and is advisory
in the way `w:tcW` is.

`FG-AI4NDM-TR-skeleton_template.docx`'s ITU cover is a table whose title rows skip a narrow first
column. Measured against that column, the 36 pt `ITU-T` wrapped to `ITU` / `-T` and
`TELECOMMUNICATION` to seven characters a line, and the block grew onto a page of its own: 6 pages
against 5. The fixture `tests/corpus/features/table-grid-before.docx` renders
character-for-character identically to LibreOffice 24.2.7.2.

### A character `w:spacing` is a distance between letters

The `w:spacing` of a `w:rPr` is tracking — `SvxKerningItem`/`CharKerning`, a constant added
between characters — and shares nothing but its name with the `w:spacing` of a `w:pPr`.
`DomainMapper.cxx`:2468–2480 converts the twips straight to `PROP_CHAR_CHAR_KERNING`.

Nothing read it. **`FormattedRun.Tracking` in `Paperless.Text` was built for exactly this** and
its own doc comment names `w:spacing` as the word-processing spelling of it; only the
presentations reader ever populated it. Wired through `WordTextStyle`, `PageRun`,
`PageParagraph` (for the uniform-paragraph shortcut, which tracking must survive because it is a
width), `PageDrawing.Build` and `PageDrawing.WidthBetween`.

**Reach, measured rather than grepped.** 58 of the words track's 134 DOCX declare a character
`w:spacing`; **twelve documents' output changed**, which is the ratio this file keeps recording.
Of the twelve, one became an exact match, one stopped being one, three moved a little closer and
seven a little further away — every one of the seven by *gaining* tokens.

### The document it cost, and why it was kept anyway

`batch-008/docx/FAA-2017-0628-0002_attachment_1.docx` went 639 words against 639 to **669 against
639**, its 4 pages unchanged. The surplus is one line of its cover text box, `PADM 533: Policy
Formation – Dr. Marcia Godwin`, which `pdftotext` now returns one letter at a time.

The geometry says the change is right, by controlled variation on the document itself:

| | `PADM`, LibreOffice | ours |
|---|---|---|
| `w:spacing` deleted | 41.150 → 67.560 = **26.41 pt** | — |
| `w:spacing="60"` as shipped | 41.150 → 76.550 = **35.40 pt** | 33.850 → 69.276 = **35.43 pt** |

The reference applies the tracking; before this change we were 9 pt short on four letters. Across
the whole line ours is 338.6 pt against 341.3 — within one tracking unit. The letter gaps are
3.00 pt on both sides and the word gaps 8.26.

**So it is a tokenisation difference, not a layout one — and it is not a general property of our
sink.** A probe of one 10 pt Carlito-bold line rendered by our own CLI keeps `PADM` whole up to
70 twips and splits at 80; LibreOffice's own output of the same document keeps it whole at 60 and
splits at 80. The two thresholds are the same to within one probe step. Also checked and ruled
out: both content streams are one `TJ` array with one glyph per string and a `-300` between each,
both draw the space as a glyph mapped to U+0020, neither sets `Tc`, `Tz` or `Tw`, and rounding our
fractional `/Widths` to integers changes nothing. **Why the real document splits at 60 for us and
not for the reference is unexplained**, and a minimal probe does not reproduce it.

### `batch-011` and `batch-012`, what is left

- **`review-welsh-government-communications-mister-peter-mandelson.docx`**, 16 pages against 14.
  A long table; ours fits fewer rows per page than the reference from page 5 on (145–188 words a
  page against 187–227). **It is not the table's geometry and it is not a shrink-to-fit.** The
  document is set entirely in `Aptos Narrow` (99 runs) and `Aptos` (10), neither installed, and the
  table is *wider than its text area* — `w:tblW` 9493 twips against 9026 of column — which both
  renderers honour: the ink spans 461.4 pt in the reference and 463.3 in ours, and every column
  header sits within 0.15 pt of its counterpart on page 5.

  What differs is the width of individual strings inside those identical columns, **in both
  directions**: `DFM-00829-09` is 88.55 pt in the reference and 94.16 in ours (+6.3%), while `N116`
  on the same row is 35.05 against 32.69 (−6.7%). Both PDFs embed DejaVu Sans *and* Liberation
  Sans; the two sides assign them to different runs. So this is a **font-substitution** question
  about a family neither has, not a layout one — a plausible mechanism is VCL parsing the width
  token out of `Aptos Narrow` and searching for a condensed face where `fc-match "Aptos Narrow"`
  simply answers DejaVu Sans, but that is inferred and **not verified**. Whoever takes it should
  expect to change `Paperless.Text/Fonts`, which is shared, and sweep the other tracks.
- **`slcc-architecture-uu-architecture.docx`**, 3 pages against 4, words exact at 1221. The
  reference spills onto a fourth page carrying nothing but its page number; we fit everything in
  three. A small height, not a wrong one.
- **`手机免提系统TSB.doc`**, 2/2 pages, 36 words against 40 — and the word count is hiding the real
  defect, which is large. **We draw no CJK glyphs at all.** The page carries its Latin text, its
  digits and its rules, and every Chinese character is absent; the text layer emits one repeated
  character per paragraph, which is why `wc -w` barely notices.

### Glyph fallback is implemented and wired to nothing

That last document is a fifth instance of the pattern this project keeps finding. The machinery
is all there — `IGlyphFallbackResolver`, `SystemFontResolver.FallbackFor`,
`GlyphFallbackFamilies` ported verbatim from `ImplInitGenericGlyphFallback`, and
`FontItemiser` which cuts a run at every character the face cannot show and re-shapes it in one
that can. It is reached through `ItemisationOptions.GlyphFallback`, and

```sh
grep -rn "GlyphFallback = " --include=*.cs src/ tools/ tests/     # prints nothing
```

**Nothing in the solution ever sets it.** `PageContent.Itemisation` (`PageContent.cs`:252) builds
an `ItemisationOptions` only to carry a base direction, and passes null the rest of the time.

The machine has CJK faces installed — `fc-list :lang=zh` finds WenQuanYi Zen Hei and Unifont — and
LibreOffice uses one of them. We resolve `SimSun` through fontconfig to DejaVu Serif, which has no
CJK coverage, and then draw nothing.

Two things to know before taking it. The drawing path does **not** itemise by font: `PageDrawing`
itemises only for bidi reordering (`Pieces`, `PageDrawing.cs`:1089) and shapes each `PageRun` with
its own face, so a fallback applied in measurement alone would put the widths and the glyphs on
different faces. And it is a shared layer by consequence if not by file — it changes line breaks
on any document with a character its stated face cannot show — so it needs the other tracks swept.

### Still open from earlier rounds, untouched this round

`batch-010`'s `5709.16 ch.40_mgfinal.docx` (31 against 32, the 11.4 pt header and the four lines);
the paragraph `w:pBdr` height, whose 23 declaring documents are almost all in batches 013–020; the
VML `DrawAspect` work; and `batch-006`'s row-split trap, which remains a trap.
## Slides, round fifteen: the autofit search's grid is a length, not a round number of points

The baseline reproduced the brief **to the digit** — 152/163 on the word gate, **1409.81**
signed ink, **466** major pages, every page count exact, 0 `ref-failed`. (The worktree was 384
commits behind when the round opened and was fast-forwarded before anything was measured. Every
recent agent has hit this; it is now five in a row.)

**`|ink|%` measured across the track for the first time: 1759.63** against the signed 1409.81.
`research/probes/slides-r15/ink-columns.py` totals both columns per document and diffs two
sweeps by them, since `track-ink-sweep.sh` sums only the signed one.

### The two columns rank the track differently, and it matters for what gets worked next

Fourteen of the top thirty move at least two places between them, and three move by a dozen:

```
signed #26 -> unsigned #14    13.04 / 21.80   southern-classic-kennesaw-state-university-final.pptx
signed #65 -> unsigned #25     5.83 / 15.94   airbus-…-without-video_diy_2019-20.pptx
signed #32 -> unsigned #21    10.62 / 17.30   Intersil_Italy_CAN_Bus_Transceiver…pptx
signed #5  -> unsigned #2     48.41 / 74.66   Reporting_responsibilities_matrix.pptx
```

`Reporting_responsibilities_matrix.pptx` is the one to note. Round fourteen looked at it, called
it "a thin uniform difference, not repeated furniture", and left it fifth. Unsigned it is
**second on the whole track**, 74.66 over 268 pages — which says the thinness is a lot of
cancelling rather than a little difference. `Snowbirds_High_Show.pptx` is the same shape in
miniature: 13.92 signed, 19.18 unsigned, and **zero major pages**, so neither figure the track
has ever steered by points at it.

#### And the "not repeated furniture" half is refuted by the tool's own output

All six of `Reporting_responsibilities_matrix.pptx`'s major pages carry the **same two regions
at byte-identical extents**, which is exactly what repeated furniture looks like:

```
top-right: marks displaced or reshaped        (1.32% of page, x 0.72-0.98, y 0.02-0.11)   6 of 6
top-left:  a solid area drawn differently     (0.95% of page, x 0.01-0.08, y 0.01-0.15)   6 of 6
bottom-centre: a fill or background shading we draw and the reference does not            3 of 6
```

The master's `Rectangle 2` — the title bar, `off 1102784,188566 ext 10877549,792162` on a
12192000 × 6858000 slide — covers the top-right region's whole extent, so that one is inside the
title. The top-left region is outside every master shape. Neither was named before because the
tool prints region detail for major pages only and the earlier reading grouped signatures across
the document rather than comparing extents.

### The defect: `EE_CHAR_FONTHEIGHT` is read from hundredths of a millimetre

`SdrTextObj::autoFitTextForCompatibility` bisects a font scale and floors each candidate to a
tenth of a point **of the object's `EE_CHAR_FONTHEIGHT`** before trying it. That height is a
length, and the code converts it *from hundredths of a millimetre*
(`svx/source/svdraw/svdotext.cxx`, 24.2.7), so a 20 pt default is 706 units and comes back as
**20.0126 pt** — never 20. `SlideAutofit` hardcoded a round `12.0`.

Whole-number grids are pathological, and the mechanism is exact. At the 87.5 per cent candidate
a grid of 12 puts the scaled size on precisely 17.5 pt; `basegfx::fround` takes that *up* to 18,
the body overshoots the box, `fMaxY` drops to 87.5, and every larger candidate becomes
unreachable. The reference's 11.99055 lands the same candidate on 17.489, rounds *down* to 17,
and the search keeps climbing. **Every disagreement measured has that one shape: we settle for a
looser fit — smaller text on wider-spaced lines — than the reference.**

### How it was found, and the two hypotheses killed on the way

Four probes, none of which assumes a height model
(`research/probes/slides-r15/`). `make-autofit-probe.py` builds a deck of one autofit box per
slide over a range of box heights, plus a `--sizes` mode that renders the same text *unshrunk* at
every size; `read-autofit.py` reads the chosen font scale off `/Tf` and the chosen spacing scale
off the baseline pitch; `pdfops.py` walks the content stream, because poppler gives glyph boxes
and never a font size and a font size is what settles this; `sim-autofit.py` transcribes 24.2.7's
search and runs it on the reference's own measured line counts.

- **The unshrunk line counts and pitches agree with the reference at all 14 sizes**, so the
  height model was never in question.
- *Killed: the fit is seeded by the shared draw outliner.* `fInitialFontScaleY` is read off the
  outliner and never reset when the item states no max scale, so a box's answer could have
  depended on the shape laid out before it. It does not — the same 33 boxes emitted largest-first
  come back **identical row for row**.
- *Killed: the binary picks the tightest fit that exists.* Brute-forcing every whole-point size
  against every spacing agrees with the binary on 28 of 33. It is a search that misses, not an
  optimiser.
- *Held.* The transcribed search reproduces the binary on **33/33 and 33/33** under a grid
  derived from the body's character height, and on **27/33 and 33/33** under a round twelve.

### What moved

| probe | boxes | before | after |
|---|---|---|---|
| 20 pt text, 80% line spacing, heights 40–200 pt | 33 | 25 | **31** |
| 40 pt text, 80% line spacing, heights 80–400 pt | 33 | 31 | **31** |
| `slide-autofit-grid.pptx`, heights 90–200 pt | 23 | — | **23** |

The fixture is the committed one and covers exactly the range where the grid bites; it agrees
with the reference on the chosen size *and* the chosen spacing scale on all 23.

### Two things that are inferred, not measured

**Which run's height, when a body states several.** A deck putting a 20 pt paragraph in front of
three 40 pt ones comes back 33 of 33 under either reading, so first-run and largest-run are
indistinguishable on this evidence. The largest is taken because it is the more stable of the
two. The code says so.

**The predecessor's note said the opposite and was not wrong about its own measurement.** It
recorded a fixed twelve beating the run's own size, 225 probe boxes to 210. That experiment took
the run's size in *points*, which for its 25, 32 and 40 pt boxes is a whole number every time —
so what it actually compared was two whole-point grids. Its 227 boxes all still pass: they are
one- and two-line bodies at a single size and none of them turns on the grid, which is why they
never caught this.

### The two probe boxes still wrong, and their cause is located

`fit80`'s 40 pt and 45 pt boxes. The transcribed search gets both right, so this is our
arithmetic rather than the algorithm, and it is the **line height going through whole twips**.

`SlideTextLayout.Spacing` sends a proportional rule to `LineSpacingRule.Apply`, which computes
in whole twips because that is Writer's unit. EditEngine computes in hundredths of a millimetre:
`nHeight = fround(pLine->GetHeight() * fProportionalScale * fSpacingFactor)`
(`editeng/source/editeng/impedit3.cxx:1502`, 24.2.7). Worked through on the 40 pt box at 8 pt and
nine-tenths spacing:

| | |
|---|---|
| natural line, both | 338 |
| reference: `fround(338 × 0.8 × 0.9)` | **243** |
| ours: 338 → 191.62 twips → **192**, less 20 per cent → 154 twips → 271.6, then × 0.9 | **244.5** |
| six lines, less the 50-unit slack | 1408 against **1417** |
| the box | 1413 |

Nine hundredths of a millimetre decide it: the reference fits 8 pt at nine-tenths and we do not,
so we fall back to 7 pt at full spacing. The same rounding is worth about 0.01 pt a line on
ordinary sizes, which is why nothing else has ever noticed it. `Spacing`'s own comment already
argues that a twip is too coarse for the draw layer and then routes the proportional case
through twips anyway — the single-spacing case is the one it exempted.

Not attempted this round: folding the spacing scale into that single `fround` also moves the
ascent rule (`impedit3.cxx:1497-1500` carries an extra four-fifths factor the current
`Spaced` does not), and every iteration of it costs a whole-track sweep.

### The brief's headline lead did **not** move, and that is worth stating plainly

`2015-Civil-Rights-Website-training.ppt`'s page 39 is still set at 18 pt at nine-tenths spacing
against the reference's 17.008 at full spacing — byte-identical to the baseline. The deck's frame
is not one the grid change reaches. What the round did establish about it, which the brief did
not have:

- The line pitch is **not** a line-height rule error. Both sides give the deck's unshrunk 32 pt
  title a pitch of **38.41 pt**, exactly; the body differs only because the fit answers differ.
- The two answers are `(0.85, 1.00)` and `(0.90, 0.90)` — 0.85 against 0.81 of the natural line —
  and the size difference is what makes our last paragraph wrap to five lines where the
  reference's takes four.

So the deck is an autofit-search residue, and the twips arithmetic above is the best-placed
candidate for it rather than anything about line spacing as such.

### `…Aercap.ppt`'s fourth face is one text show on one page

Round fourteen left this as "the one document this round moved *away* on the font check,
unexplained". It is smaller than it sounds. We embed DejaVu Sans twice, OpenSymbol and
**Liberation Serif**; the reference embeds DejaVu Sans, DejaVu Sans Bold and OpenSymbol. So the
overshoot is not an extra symbol face at all — it is a *serif text* face the reference never
resolves, and it is drawn **once, on page 6, at 14 pt, in a single show operation**.

The deck names four faces: Arial, Times, Verdana and Wingdings. The reference resolves none of
them to a serif, which makes `Times` — the bare name, not `Times New Roman` — the candidate to
check first. One run, one page, and the whole of the round-fourteen regression.


## Slides, round sixteen: the round-fifteen work was not merged, and the sweep it "never ran" exists

Two corrections to the record before any measurement, both found by checking rather than by
being told.

**The autofit grid fix was not on the track branch.** The brief for this round said round
fifteen's fix "is merged and is on your base". It was not: `0264f7f86` and the five commits
around it were reachable only from `worktree-agent-a3b51381a3030c1ce`, and
`claude/paperless-odf-phase-1-rnyzcu` at `54729fdc7` did not contain them
(`git branch -a --contains 0264f7f86` names exactly one branch). This worktree was
additionally **402 commits behind** that branch — the sixth agent in a row to open on a stale
base, and the first for which fast-forwarding was not sufficient, because the work being built
on had never reached the branch at all. Both were repaired before anything was measured:
fast-forward to `54729fdc7`, then merge the six round-fifteen commits, keeping *both* sides of
the one `TODO.batches.md` conflict (the words round-fifteen section and the slides one were
appended at the same anchor).

**A post-fix whole-track sweep does exist**, contrary to "its whole-track sweep never ran":
`scratchpad/sl15-grid/` holds a complete 163-row `rows.tsv` and `ink.tsv`, written at 11:41
against a fix committed at 11:16, and its ink column differs from the pre-fix `sl15-base/`
on dozens of documents rather than being a copy. Its TSVs are preserved at
`scratchpad/slides-r16-keep/`.

**It still does not describe this tree, and that is the point worth keeping.** It was measured
18 commits back, and those 18 commits include `d3bf1c445` — a `Paperless.Text` change to how a
font substitution chain is resolved, which is a shared layer below every slide. A sweep of the
slides track taken before it is not a baseline for a tree that has it. The round's own baseline
is measured fresh, and the r15 numbers are kept only to compare against.

### The round-sixteen baseline, and the number the autofit fix actually moved

Whole track, `scratchpad/sl16-base`, 163 rows, no duplicate paths, 0 `ref-failed`:

| | round fifteen's baseline | **round sixteen's** |
|---|---|---|
| word gate | 152 / 163 | **151 / 163** |
| signed `ink%` | 1409.81 | **1402.67** |
| unsigned `\|ink\|%` | 1759.63 | **1751.40** |
| major pages | 466 | **466** |

**The three-way split says the base is right and the harness is right.** The predecessor's
pre-fix sweep, its post-fix sweep and this one differ in exactly the places they should:

| | word gate | `ink%` | major |
|---|---|---|---|
| `r15base` — no autofit fix, no shared-layer commits | 152 | 1409.81 | 466 |
| `r15grid` — autofit fix only | 151 | 1402.46 | 466 |
| `sl16-base` — autofit fix + 18 commits | 151 | 1402.67 | 466 |

`r15grid → sl16-base` moves **one document by 0.21** and nothing else. So the eighteen
commits between the two — including `d3bf1c445`, the `Paperless.Text` font-substitution
change that was the reason to re-measure at all — are worth 0.21 of ink on the whole slides
track. Re-measuring was still right; it is the only thing that could have said so.

**The autofit grid fix is therefore worth −7.35 of signed ink over 31 documents, no change in
major pages, and one document on the word gate.** The predecessor never got to state this,
and it is smaller than the probe result suggested — 31 documents move, 20 of them for the
better, and the eleven that move the other way take back a third of the gain.

#### The one document it cost, and it is a real regression rather than a gate artefact

`NWD-GLA-Community-Outreach-Day-Oct-2025.pptx` went `match → words`, **596/586 words to
537/586**, with its page count untouched at 13/13. The sign is what makes it worth naming: it
was over-drawing by 1.7% and inside tolerance, and it is now *under*-drawing by 8.4%. A fix
that makes the search settle on a larger font can only lose text by overflowing something, so
this is the autofit fix's own consequence and not the gate's ceiling. Open.

### The fold swept: right on the fixture, flat on the track, and one deck holds the whole delta

Whole track with the folded rounding, reference PDFs reused from the baseline run and verified
identical row for row on every reference-side column (`probes/slides-r16/ref-check.py`, 163
documents, 0 mismatches):

| | baseline | fold |
|---|---|---|
| word gate | 151 / 163 | **151 / 163** |
| signed `ink%` | 1402.67 | 1404.07 |
| unsigned `\|ink\|%` | 1751.40 | 1753.40 |
| major pages | 466 | **465** |

**102 documents moved: 55 better, 43 worse, 65 unchanged.** 6.63 of `|ink|%` won and 8.63
lost. That is a change that is doing something real and roughly cancelling — except that the
cancelling is not even:

```
+2.14   31.38 -> 33.52   major +1   2015-Civil-Rights-Website-training.ppt
+1.36  277.21 -> 278.57  major  0   NAS-Infrastructure-Roadmaps-v16.0.pptx
-1.21   65.04 -> 63.83   major  0   Thailand17.ppt
-1.00   11.43 -> 10.43   major -1   berlin.ppt
```

**Without `2015-Civil-Rights-Website-training.ppt` the net is −0.14.** One deck — the brief's
headline lead — carries more than the whole loss, and it is the one document the round was
pointed at. The fix is exact on the controlled fixture (6/6 against the reference's own PDF,
worst case 0.00094 pt, against 0.032 for the arithmetic it replaces) and the track is flat
without that deck, so this is not a wrong fix; it is a fix that has uncovered something on one
document. That deck is the next thing to look at, not the change.

### `2015-Civil-Rights-Website-training.ppt`: a near-tie in the search, not a line-height error

The brief named this deck's line pitch as the round's best lead — ours 36.14 pt against the
reference's 41.73/37.64, "text 5.5% larger", "it is that deck's whole 25.10 of ink". The
measurement reproduces and **the explanation is the wrong way round**: the pitch is not a
line-height error at all, and the fold that fixes the line height makes this deck worse.

Page 21, read straight out of both content streams with `probes/slides-r15/pdfops.py`:

| | reference | ours |
|---|---|---|
| body em | 18.992 pt (670 mm100) | 20.013 pt (706 mm100) |
| baseline pitch | 18.227 pt (643 mm100) | 17.291 pt (610 mm100) |
| lines on the page | 18 | 19 |
| **block height** | **328.1 pt** | **328.5 pt** |

The deck states 20 pt and 80% line spacing. The reference shrank the *font* to 19 pt and kept
the spacing; we keep the font at 20 and shrink the *spacing* to nine-tenths. Our pitch is
exactly `fround(fround(706 × 1.2) × 0.8 × 0.9)` = 610 and the reference's is
`fround(fround(670 × 1.2) × 0.8)` = 643, so **both sides' arithmetic is the same arithmetic**
— the fold is not in question here. What differs is which candidate the search keeps.

**And the two candidates are 0.4 pt apart on a 328 pt block — 0.12%.** `Solve` keeps the
tightest fit at or above one seen anywhere in the search, so a tenth of a per cent decides
between "20 pt at nine-tenths" and "19 pt at full", and the fold moved the line height by
enough to tip it. That is why this deck holds the round's whole `|ink|%` loss while the other
162 net to −0.14.

So the open question on this deck is **the search's preference between a font reduction and a
spacing reduction when both fit**, and it is not the line height. Two things make that the
right next probe rather than a guess:

- The same shape was recorded on page 39 last round — reference `(0.85, 1.00)`, ours
  `(0.90, 0.90)`. Two pages, same disagreement: the reference takes the smaller font at full
  spacing, we take the larger font at reduced spacing.
- It is **not** that the reference refuses to reduce spacing. `slide-autofit-grid.pptx`'s six
  cases have the reference choosing 0.8 and 0.9 spacings, and the predecessor's 33-box probes
  record the binary picking `(90.000, 80)` and `(94.960, 90)`. So the rule is an ordering, not
  a prohibition, and naming it needs a probe that puts two nearly-tied candidates in front of
  the binary deliberately.

### One correction to the track's parity record

The brief states "batches 001–013 and 015 at full parity". Measured on both this round's
baseline and the predecessor's, that is not the case: **batch-008 is 9/10, batch-010 8/10 and
batch-012 8/10**, and were so before anything moved this round. The batches actually at full
parity are **001–007, 011, 013 and 015** — ten of seventeen — plus 009 until the round-fifteen
autofit fix cost it a document.

### The document round fifteen cost, diagnosed: the grid's floor is six times finer

`NWD-GLA-Community-Outreach-Day-Oct-2025.pptx`, `match → words`, 596/586 words to 537/586 at an
unchanged 13/13 pages. **The pages still match**: total `|ink|%` is 0.67 over thirteen pages and
**zero of them are major**. What is missing is text drawn so small the reference's own rendering
shows it as a column of one-pixel dashes — 26 words on page 6 and 32 on page 12, which
`pdftotext` reads and a human cannot.

This deck already has a commit of its own — `068b0eb44`, "Stop the PDF sink writing text at an
em of nothing" — for a shape where autofit concludes that *no* positive scale fits, because each
of sixteen paragraphs carries an absolute 12 pt top margin that does not shrink with the font.
The guard is right and the reference agrees with it on the page it was written for.

The round-fifteen grid fix moved two more shapes across that threshold, and the arithmetic is
exact. The search floors each candidate to a tenth of a point *of the grid height*, so the
smallest non-zero scale it can reach is `0.1 / grid`:

| grid | smallest reachable font scale |
|---|---|
| a round 12 pt, as shipped before | 0.1 / 12 = **0.00833** |
| the body's own 60 pt height, as shipped now | 0.1 / 60 = **0.00167** |

**Six times smaller**, so runs that used to round to a representable em now round to zero and the
sink correctly declines to write them. The grid fix is not wrong and the guard is not wrong; what
is missing is a floor on the search itself, and the reference plainly has one because it draws
those marks. Naming what that floor is, is the work — it is not a change to either piece.

### Round sixteen, final: neutral on the track, better on major pages, one deck against it

Both changes swept whole, reference PDFs reused and verified identical row for row each time:

| | baseline | fold | + `Off` branch |
|---|---|---|---|
| word gate | 151 / 163 | 151 / 163 | **151 / 163** |
| signed `ink%` | 1402.67 | 1404.07 | **1403.48** |
| unsigned `\|ink\|%` | 1751.40 | 1753.40 | **1752.86** |
| major pages | 466 | 465 | **464** |

**Read it per document rather than as a total.** 56 better, 44 worse, 63 unchanged; 7.10 of
`|ink|%` won and 8.56 lost. `2015-Civil-Rights-Website-training.ppt` alone is +2.14 of that
loss, so **without it the round is −0.68**, and major pages are down 2 with the word gate
holding on every batch and not one verdict changing anywhere.

So the honest summary is: the change is right where it can be checked against the reference
directly, neutral-to-better on 162 documents, and it loses on the 163rd by tipping a near-tie
in a search whose preference rule is a separate, now-named defect. `|ink|%` says +1.46 and
that number should be reported as it stands; it is one document's worth of a different bug.

**Reverting is one commit** if the total is preferred to the parts — but it would restore the
whole-twip line height, which is 0 of 6 against the reference on the fixture where the folded
arithmetic is 6 of 6.

### What the next round should take, in order

1. **The search's preference between a font reduction and a spacing reduction.** Named and
   measured above on two pages of one deck, and it is what the brief's headline lead actually
   is. Needs a probe deck built to tie two candidates deliberately — the existing probes sweep
   box heights, which produces ties only by accident.
2. **A floor on the search**, which is what cost round fifteen its document. The reference draws
   marks at a scale we round to nothing; `0.1 / grid` is our smallest reachable scale and the
   grid is now six times larger than it was.
3. `Reporting_responsibilities_matrix.pptx` — still second on the unsigned ranking at 74.66 over
   268 pages, with two regions repeating at byte-identical extents on all six of its major
   pages. Untouched this round.


## Words, the tenth round: a cell's line spacing and two things a tab stop does — swept whole at `1094f13f5`

**147/200 → 150/200; total absolute page error 117 → 109; documents with an exactly correct page
count 158 → 160.** Both sweeps against checksummed CLI snapshots of the same worktree, 200 rows,
no duplicates. The baseline reproduced all twenty-one recorded batch figures exactly.

| Batch | before | after | |
|---|---|---|---|
| `batch-012` | 8/10 | **9/10** | `slcc-architecture-uu-architecture.docx` 3 pages → 4 against 4 |
| `batch-015` | 4/10 | **5/10** | `hdss-bulletin-issue-285…docx` 11 → 10 against 10 |
| `batch-016` | 7/10 | **8/10** | `150_5300_13_chg8.doc` 20 → 18 against 18 |
| every other batch | — | unchanged | |

Batches 001–005 are 10/10, 006 is 9/10, 007 10/10, 008 9/10, 009 10/10, 010 8/9, 011 9/10 — the
gate for moving on is green and unmoved.

**A trap worth putting at the top: `batch-check.sh` resolves the CLI from its own location**, four
levels up from `.claude/skills/corpus-batches/scripts`, which is the *main checkout* and not the
agent's worktree. Two sweeps this round measured a binary another session was rebuilding, and the
first of them looked entirely normal — right row count, no duplicates, and figures that matched the
scoreboard, because the main checkout happened to be near the same commit. Copy the script and take
the CLI from an environment variable, or run the worktree's own copy of it.

### A table cell grows by its last paragraph's proportional line spacing

`AddParaLineSpacingToTableCells`, which `WriterFilter.cxx`:314 switches on for every DOCX, DOC and
RTF and which a native ODF document leaves off — so it is exactly the split between
`PaginationOptions.Word` and `.Default`. `SwFlowFrame::CalcAddLowerSpaceAsLastInTableCell`
(`sw/source/core/layout/flowfrm.cxx`:1946) charges the cell for it beside the space-after this
engine already charged, and only for the cell's *last* flow frame.

The amount is the part worth writing down, because it is not the leading the layout engine
computes. `SwBorderAttrs::CalcLineSpacing_` (`sw/source/core/layout/frmtool.cxx`:2681) is

```
nFontSize × (prop − 100) × 1.15 / 100
```

in twips — the paragraph's **font size**, not its measured line height, with a 1.15 fudge added for
tdf#125300 that stands in for the ratio between the two. Reproduced to the digit including the
order of operations: the integer product is formed first and only then multiplied by a binary
`1.15` a shade under the decimal one, which is what makes 125% come out at 68 twips rather than 69.
Measured against LibreOffice on a one-cell fixture at 110, 115, 125, 150, 200 and 250 per cent —
27, 41, 68, 137, 275 and 414 twips at twelve point, all six exact.

Before it, a cell was 14.65 pt tall at every one of those six percentages. `TableLayouter.cs`,
`CellLineSpacing`.

### A paragraph's tab stops merge with its style's rather than replacing them

`DomainMapper` seeds the set from the paragraph style before it reads a single `w:tab`
(`DomainMapper.cxx`:2604, `InitTabStopFromStyle`) and folds each one in with `IncorporateTabStop`
(`DomainMapper_Impl.cxx`:1485): replace at the same position, delete on `w:val="clear"`, append
otherwise. We took the innermost `w:tabs` whole — and `w:val="clear"` is the proof that is wrong,
since a set that replaced its style's outright would leave a clear entry nothing to cancel.

### An aligned tab stop past the line's right edge clamps to it

`SwTabPortion::PostFormat` (`sw/source/core/text/txttab.cxx`:503):
`nRight = std::min(GetTabPos(), rInf.Width())` for a right, centred or decimal stop. A **left** stop
past the edge still breaks the line — `PreFormat`'s `bFull` — which is what this engine already did.

**The two tab changes have to land together.** The merge alone was +1 match and **+24 page error**:
restoring the dotted right stops that `toc 2`–`toc 4` styles declare put those stops past a
paragraph that also carries a right indent, and every contents entry then broke into four lines —
one for its number, one for its title, one of leader dots and one for its page.
`02_mcar_part-2_and_IS_v2.10.docx` went 317 → 329 pages against 312 and `SPA-02_mcar…docx`
270 → 282 against 266. With the clamp they are 315 and 267, and their contents pages now set one
entry per line with leaders exactly as the reference does.

### Where the clamp is an approximation, and by how much

Writer has three behaviours, not two, and this implements the middle one everywhere:

| flag | set for | clamp |
|---|---|---|
| `TabOverMargin` | DOCX with `compatibilityMode` ≤ 14 (`SettingsTable.cxx`:688) | none at all |
| `TabOverSpacing` | every writerfilter document (`WriterFilter.cxx`:325) | the **frame's** right edge |
| neither | — | the **line's** right edge, indents included |

We clamp at the line's edge always. On a paragraph with a right indent that is tighter than
LibreOffice's by exactly the indent, so a leader stops half a centimetre short of where the
reference draws it. Implementing `TabOverSpacing` properly needs the other half of the rule as
well — Writer lets the tabbed run overrun the indent *without breaking the line*, which this
engine's filler cannot express — so it was left alone deliberately rather than overlooked.
Measured cost: `EHEST-SMS-Safety-Management-Manual-V2.docx` went 82/82 to 78/82, the only
regression of the round, and `ABCD-FE-01-00 Flight Envelope…docx` 15/15 to 14/15 while already
failing on words. Both are documents whose lines *should* overrun and wrap.

### What is left in `batch-012`, and what was measured on 013 and 014

`batch-012` is 9/10 and its remaining failure is `手机免提系统TSB.doc`, which is the glyph-fallback
item recorded above this section and unchanged: 36 words against 40, on a page where we draw no CJK
glyphs at all.

`batch-013` is 5/9 and `batch-014` 3/10, both unmoved. Their thirteen failures were measured and
not diagnosed beyond the page deltas: `A1. EASA Form 2.docx` +2 pages and 137 words short;
`bulletin.docx` +1; `template---tpr…docx` −1; `AW-104D-RVSM…docx` page-exact and six words long;
in 014 four documents off by one page in each direction, `UG.CAO.00133…docx` page-exact and 245
words *over* — which is the raster-ceiling file's own worked false positive, so re-measure it
before subtracting anything — and `xx_SETIS_PWS_template_10.19.22.docx` page-exact and 541 short
with no text boxes to blame.

### The shared layer, and the check that it is a no-op elsewhere

`Paperless.Text` changed: `TabRuler.Segments`/`WidthOf` take a right edge and `TextMeasurer` computes
one per line. Both parameters default to null, and the clamp is gated on a new
`ParagraphFormat.ClampsTabsAtLineEdge` that only the four word-processing readers set — Impress and
Calc reach the same `ParagraphLayouter` but leave it off, so their right edge is null and `TabRuler`
does exactly what it did. That gating is not cosmetic: `SlideTextLayout.Stretches` has no line width
to hand, so an ungated clamp would have made a slide *measure* clamped and *draw* unclamped.

Checked rather than argued. Both other tracks swept on the same two checksummed snapshots:

| track | before | after | |
|---|---|---|---|
| `slides` | 152/163 | 152/163 | `parity.tsv` identical row for row |
| `sheets` | 137/171 | 137/171 | two rows differ, both in the *reference's* word count (5557→5552, 10245→10244); every one of our own figures is unchanged and no verdict moved |

The words track was also re-swept after the gating landed: 150/200 and page error 109 either way.


## After the seventeenth round: slides — the em was never on the draw layer's grid

Whole track swept before and after, 163 documents each time, both against checksummed CLI
snapshots of this worktree. The second sweep reuses the first's reference PDFs, verified
identical row for row on every reference-side column (`probes/slides-r16/ref-check.py`,
163 documents, **0 mismatches**). No duplicate paths, 0 `ref-failed`, in both.

| | baseline `7e1b7c79e` | after |
|---|---|---|
| word gate | 151 / 163 | **151 / 163** |
| signed `ink%` | 1403.48 | 1406.70 |
| unsigned `\|ink\|%` | 1752.86 | **1752.00** |
| major pages | 464 | **462** |

**The baseline reproduced all four of the brief's headline figures to the digit** — 151/163,
1403.48, 1752.86, 464 — and its per-batch correction with them: full parity is **001–007, 011,
013 and 015**, with 008 9/10, 009 9/10, 010 8/10, 012 8/10, 014 7/10, 016 8/10 and 017 4/5.

**Not one verdict changed on any of the 163 documents**, and no batch moved. 155 documents moved
on ink — 72 better, 69 worse, 22 unchanged; 11.23 of `|ink|%` won against 10.37 lost.

The signed figure rising while the unsigned falls is the pattern the ink columns exist to
separate, not a contradiction: filling a deficit uncancels a surplus elsewhere in the signed sum
and has nowhere to hide in the unsigned one.

### The em size, and the reference's own evidence for it

A slide's character height lives in an `SvxFontHeightItem` in the model's map unit, and for a
draw object that unit is **a hundredth of a millimetre**. So LibreOffice draws a 20 pt run at 706
units — **20.0126 pt** — and takes every advance width, every line break and every height the
shrink-to-fit search compares at that size. We drew exactly 20 pt: a systematic **0.06% narrower
on every run in every deck**.

It is visible directly in the reference's own PDFs, which is what makes it checkable rather than
argued. `probes/slides-r17/size-census.py` compares the `/Tf` sizes two renderings use page by
page, and `mm100-grid.py` asks whether a size could have come off that grid at all:

```
ours   12367 of 26996 show operators sit on the 1/100 mm grid  (45.81%)
ref    22167 of 26943                                          (82.27%)
```

over forty documents — and all fifteen of the commonest sizes we wrote that the reference cannot
hold are whole points: 24, 16, 20, 12, 28, 17, 10, 9, 15, 44. The reference's residual 18% is
text it rasterises or plays out of a metafile, which is on no grid by construction.

The conversion is the property setter's rather than a direct ratio.
`SvxFontHeightItem::PutValue` takes `nHeight = (long)(fPoint * 20.0 + 0.5)` to twips and then
`convertTwipToMm100`, which is `(n * 127 + 36) / 72`
(`editeng/source/items/textitem.cxx`:774-776, 24.2.7.2). For a whole number of points the twip
step is exact and the pair reduces to `o3tl::convert(pt, pt, mm100)`, which is what the PPT filter
calls directly — **so one implementation is faithful to the PPT, PPTX and ODP readers alike.** A
DrawingML `sz` of 1333 separates them: 13.33 pt is 267 twips and therefore **471** units, where
the direct ratio gives 470.

`SlideAutofit.Quantised`, applied in `Scaling.Scaled` because that is the one place every measured
and drawn em passes through — `LargestSize` reads it back off `RunStyle.Size`, the shaper takes it
as `FormattedRun.EmSize`, and the sink writes it as `/Tf`. **Nothing outside
`Paperless.Presentations` was touched**, so the words and sheets tracks cannot be affected and are
not owed a sweep.

### The brief's headline lead was the wrong half of the mechanism

The round-sixteen brief hands on *"the autofit search prefers shrinking the spacing where
LibreOffice shrinks the font … `Solve` keeps the tightest fit, so a near-tie tips the wrong way"*
and points at the search's **preference rule**.

The preference rule is already the reference's. `svx/source/svdraw/svdotext.cxx` at tag
`libreoffice-24.2.7.2` — the release that made the reference PDFs, and *not* the 27.2-alpha in
this checkout, which has replaced the function outright — was compared against `SlideAutofit.Solve`
statement by statement: the bisection, the tenth-of-a-point grid, the `{100, 90, 80}` spacing
order, the `continue` that abandons the spacings once one fits, the min/max update, the tie-break
and the ten iterations are **all identical**, ties included. Both sides walk the same grid in the
same order and keep the same candidate, so a disagreement about which candidate wins can only come
from the *heights measured*.

That pins the box on `2015-Civil-Rights-Website-training.ppt` page 21 rather than leaving it open.
The reference's 19 pt at full spacing is 18 x 643 = 11574 mm100 and our 20 pt at nine-tenths is
19 x 610 = 11590; less the search's 50 units of slack, 11524 and 11540. Ours is the *taller* block
and therefore the tighter fit, so for the reference to have kept 19 pt the 20 pt/90% candidate must
have failed to fit for it — which puts the box in **[11524, 11540)**, a window of 16 mm100, 0.45 pt,
**0.14% of the block**.

**The em fix did not close it.** That deck's differing pages fall 55 to 30 — exactly the
quantisation class disappearing — and page 21 is still 20.01 against 18.99. So 0.06% was not
enough to add the twentieth line, and the residue is a height measurement worth about a third of
a point over nineteen lines. The deck is 33.52 → 33.20.

### The census, and what the track's residue actually is

Run over the baseline (`probes/slides-r17/base-size-census.txt`, 129 of 163 documents before it
was stopped to give the sweep its CPU back; `census-summary.py` reads it):

```
pages differing on the dominant /Tf size   1642
  by <= 1%  — the 1/100 mm grid            1043
  by  > 1%                                  599
     of those, both sides drawing text      537  over 101 documents
```

Two-thirds of the page-level size disagreements are the class this round removes. **The other 537
are a size disagreement of more than one per cent on a page where both sides draw a comparable
amount of text** — so not the raster ceiling, where the reference's dominant size carries one or
two show operators against our dozens.

Its ranking lines up with the ink ranking rather than cutting across it: of its ten worst
documents, five are in the top twelve by `|ink|%` — `NAS` (42 pages), `2014BSA_Sunday_Killion`
(29), `2015-Civil-Rights` (29), `ITE106-Chapter 4` (22), `171128IPAP` (18), `FAA_Form_337` (17),
`Intersil_Italy_CAN_Bus` (14), `Reporting_responsibilities_matrix` (12).

**This is the instrument the next round should take, not more pixels.** `pdf-image-diff.py`'s
*"marks displaced or reshaped"* is what a font-size disagreement looks like at 512 pixels and the
hint cannot separate it from an indent or a line break; the census names a page and a number on
each side.

### Batches 008, 010 and 012 are at the word gate's ceiling, and two records are wrong

Every remaining failure in the three batches this round was pointed at is over-drawing, and in
four of the five the excess is confined to pages already on `TODO.raster-ceiling.md`. Split per
page with `probes/slides-r17/page-words.py`:

| Document | pages carrying the excess | residue | reference |
|---|---|---|---|
| `batch-010/…/W3_Case_Study…ppt` | 10 (+93) | 817 | 817 |
| `batch-010/…/Fundamentals_Module_1_basics.ppt` | 6 (+50) | 1096 | 1099 |
| `batch-012/…/OnTrac_StarCertification…pptx` | 9 (+46), 10 (+251) | 1047 | 1045 |
| `batch-012/…/NAS-Infrastructure-Roadmaps-v16.0.pptx` | the 24 `Requires="v"` pages | — | the named ceiling |
| `batch-008/…/8_P-Pavese…pptx` | 5 (+44), 6 (+44) | 2152 | 2108 — still out |

So none of the three can be advanced on the gate. Two corrections fall out:

**`8_P-Pavese` page 6 belongs on the raster list and is not on it.** `pdfimages -list` shows the
reference drawing the *same* 692x240 JPEG with a soft mask on pages 5 and 6 and us drawing neither.
Page 6 is +44 on a base of 180 — **24.4%**, just under the list's 25% threshold, as is page 16 at
23%. The threshold is excluding pages of the list's own class, and it should either be lowered or
described in that file as a deliberate under-count.

**Page 16 of the same deck is not the ceiling, and it is what keeps the document failing.** No
raster on either side, ink 0.30 signed over 26 regions, and **`Tm=0`** in our content stream so
nothing on it is rotated. The reference sets `chart5.xml`'s category labels on one line — *lundi
mardi mercredi jeudi vendredi samedi dimanche* — and we break each into a narrow column. The axis
states `<a:bodyPr rot="-60000000"/>` and `ObjectFormatter::convertTextRotation` discards anything
outside ±5400000 (`oox/source/drawingml/chart/objectformatter.cxx`:1087-1091, **byte-identical in
24.2.7.2**), so the rotation reads as zero and line breaking is allowed on both sides — and
`DrawingChartPlot.AxisTextOf` already implements exactly that. **The rotation clamp is not the
bug.** What differs is how much width the axis gives each category slot, in `Paperless.Core/Charts`,
which is a shared layer and was left alone. Removing pages 5, 6 and 16 leaves 2106 against 2108.

### What the next round should take, in order

1. **The 537 pages of size disagreement**, ranked above, starting with `2015-Civil-Rights` page 21,
   whose box height is now pinned to a 16 mm100 window.
2. **Quantise the shape's text area the way the em now is.** oox converts a shape's `a:ext` from
   EMU to 1/100 mm on import, so the width the reference breaks lines against is a whole number of
   units and ours carries the file's full EMU precision. Same defect, an order of magnitude
   smaller — and worth doing *after* the em rather than with it, or the two are not separable.
3. **`8_P-Pavese` page 16's axis slot width**, in `Paperless.Core/Charts`; a shared layer, so it
   owes the words and sheets sweeps.
4. **The raster list's 25% threshold**, above.
## Words, the eleventh round: a page break that keeps no space, and a table style that draws

Scope was `batch-013` then `batch-014`, with `001`–`012` re-proved. **Only batches 001–014 were
swept**, so the whole-track figure is not restated here; the gate that was measured is 138
documents.

| words 001–014 | baseline | after | |
|---|---|---|---|
| full match | 122/138 | **124/138** | +2 |
| exactly correct page count | 128 | **130** | +2 |
| total absolute page error | 12 | **10** | −2 |

Per batch, baseline → after: 001–005 10/10, 006 9/10, 007 10/10, 008 9/10, 009 10/10,
010 8/9, 011 9/10, 012 9/10, **013 5/9 → 6/9**, **014 3/10 → 4/10**. Every one of the fourteen
briefed baseline figures reproduced exactly, which is the first time that has been true here.

Both sweeps ran against checksummed CLI snapshots of this worktree, 138 rows, no duplicated path.

### The fix that moved the number: a page break keeps no space from Word 2013

`SwFrame::IsCollapseUpper` (`sw/source/core/layout/calcmove.cxx`:1120), whose own comment reads
"Word >= 2013 style: when we're at the top of the page's body, but not on the first page, then
ignore the upper margin for paragraphs". It runs *after* `HasParaSpaceAtPages` has decided the
space is due and zeroes it, so it is a second rule rather than a different setting of the first.

`Paginator.cs`:728 granted the space to every paragraph carrying an explicit break at every
compatibility mode, which is the ≤ 14 behaviour applied everywhere.

The discriminator is the mode and **not** the kind of break, which is the opposite of what
reading `HasParaSpaceAtPages` alone suggests. Eleven synthetics carrying 20 pt of space-before,
reading the first word of page two against a 72 pt top margin:

| shape | mode 15 | mode 14 / 12 / absent |
|---|---|---|
| the document's own first paragraph | 92.35 | 92.35 |
| automatic break | 72.35 | 72.35 |
| `w:pageBreakBefore` | **72.35** | **92.35** |
| leading `w:br w:type="page"` | **72.35** | **92.35** |
| `nextPage` section break | 72.35 | — |

`SettingsTable.cxx`:685 sets `TabOverMargin` for a mode of 14 or less and an absent mode defaults
to 12 (`SettingsTable.cxx`:637); a DOC always sets it (`ww8par.cxx`:2047) and a native ODF
document sets neither, so both keep the older rule and neither reader was touched.

Two documents moved and nothing else did — `batch-013/docx/bulletin.docx` 15 pages → 14 against
14, and `batch-014/docx/SPA-11_mcar_part-11_v2.9.docx` 50 → 49 against 49.

Not implemented: `IsCollapseUpper` also declines when the paragraph carries a `RES_PAGEDESC`,
"after applying a new page style (but do it after page breaks)". A plain `nextPage` section break
does not set one — measured, it collapses like any other break — so this bites only where a
section also changes the page's geometry.

### The lead the brief named: a table style's borders are never read, and it is worth measuring by ink

`Borders()` consulted only `w:tblPr/w:tblBorders` on the table itself, never the table style's,
and `w:insideH`/`w:insideV` were not read from anywhere. The brief's headline reproduces: a table
using **`Table Grid`** — which states nothing but a `w:tblBorders` and is used by tables that state
no borders of their own — drew **no line at all**.

One correction. The brief's "LibreOffice draws 19 path segments, we draw none" is half right. The
"we draw none" half is exact. The 19 does not travel: LibreOffice consolidates each grid line into
one stroke rather than drawing it per cell edge, so a three-by-three is **8** strokes, a
three-by-one 6, a one-by-three 6 and a one-cell table 4. Ours is now 8, 6, 6 and 4.

Which of the six sides reaches a given cell is `lcl_computeCellBorders`
(`DomainMapperTableHandler.cxx`:126); the style layer is merged under the table's own per property
at `:438`; and a table one row tall has `insideH` erased first (`:915-940`).

**The word gate cannot see any of it.** The sweep across 001–014 is identical before and after:
124/138, page error 10. Two rows moved and both were already matching, their word counts going to
exact agreement. The instrument that sees it is `pdf-image-diff.py`, over the 17 documents in
these batches whose table style states a `w:tblBorders` their table does not:

| major pages | before | after |
|---|---|---|
| `part-145-approval list (1).docx` | 8 | 5 |
| `part-145-approval list 2025.docx` | 7 | 3 |
| `PAT-047 - Architecture and Detailed Design…docx` | 4 | 4 |
| `slcc-architecture-uu-architecture.docx` | 3 | **0** |
| `mde087077~283.docx` | 3 | 2 |
| `form_1123_application_form_rvsm_spa.docx` | 3 | 3 |
| `Lessons-Learned-Bulletin-Dorset…docx` | 3 | 3 |
| `gpp-pr-top-7-office-markets-4q-2023.docx` | 2 | 1 |
| `UAS-SGI_waiver_approval_request_form.docx` | 2 | **0** |
| `SDL_FSDO_Part91_LOA_Checklist.docx` | 2 | 2 |
| `part-147_approval list_20230119.docx` | 2 | 1 |
| `cacs-206-1751218225.docx` | 2 | **0** |
| `2015-April-SWIM_Users_Forum-Q&A.docx` | 1 | **0** |
| `Software Architecture Document Template.docx` | 1 | **0** |
| `4400-91_Proposal_To_Lease_Space_10-2024.docx` | 1 | **0** |
| `DRX-Ascend System Course Description.docx` | 0 | 0 |
| `review-welsh-government-communications…docx` | 0 | 0 |
| **total** | **44** | **24** |

Seven documents lose every major page they had and not one document is worse. **46 of the words
track's 134 DOCX** hold at least one table in this shape, so most of that reach is in batches
015–021, which were not swept.

It is height as well as ink, and that is the half a word count could in principle see: a border is
a band the text may not enter and neighbouring rows share it, so the three-by-three fixture's last
line moved 109.945 → 111.445 pt against the reference's 111.796.

Cell margins came with it, for the same reason: `w:tblCellMar` is a table-level property and a
style stating one is stating how tall every row in the table is.

Not implemented, and worth naming: `w:tblStylePr` conditional formatting — `firstRow`, `lastRow`,
the banding — which needs `w:tblLook` read as well. Only the horizontal half of the
degenerate-table erasure is implemented; the vertical half provably cannot bite in a per-cell
model, because a lone cell in a row is both the first and the last.

### An integer attribute with a decimal point

`w:w="8730.0"`, `w:tblInd w:w="-85.0"`, `w:spacing w:line="360.0"` — 129 of them in one document.
LibreOffice reads every one, because its attribute list uses `rtl_ustr_toInt32`, which parses as
far as it can and stops. `int.TryParse` rejected the lot, so the property fell back to its default:
no cell margin, no column width, no table indent.

It truncates rather than rounds, measured on three documents differing only in `w:before` of 240,
240.9 and 241 — the reference puts the second paragraph at 96.996, 96.996 and 97.046 pt.

**Measured reach: one document of the words track's 134 DOCX**, and that is stated rather than
dressed up. What justifies it is agreeing with the reference reader on a class of malformed input.
`batch-013/docx/template---tpr…docx` now gets its cell margins and its table geometry right and
still fails its page count for an unrelated reason — see below.

### A standing difference this round measured but did not fix: 0.35 pt at the top of every page

Our first line on a page sits at exactly the top margin; LibreOffice's sits one line gap below it.
On the probe documents that is 72.000 against 72.345 with Liberation Sans at 11 pt, and it holds
on every page of every document compared this round. So LibreOffice puts a face's external leading
*above* the ascent in a line box and we put it below, which cancels within a paragraph — the pitch
agrees to 0.001 pt — and leaves us 0.35 pt more room at the bottom of each page. It is invisible
with Carlito, whose declared line gap is zero. Small, systematic, and capable of deciding a page
break on a document that is already near one.

### What is left in `batch-013`, named rather than guessed

- **`template---tpr-technical-progress-report-with-guidance.docx`** — 7 pages against 8, words
  1865/1862. The decimal fix corrected its table geometry and its cell margins; what remains is
  that the reference emits a **near-empty page 4** (six lines, ending at 118.64 pt) and we do not.
  Something after the bullet at the top of that page cannot be split or cannot be kept with what
  precedes it. Not diagnosed.
- **`A1. EASA Form 2.docx`** — 9 pages against 7 and 2091 words against 2228. Untouched this round.
- **`AW-104D-RVSM-Aircraft-Approval-Checklist.pdf.docx`** — page-exact, 145 words against 139, and
  the whole of the six is one footer. Its footer is a table cell holding `Page <PAGE> of
  <NUMPAGES>` reached through a `w:tab`. **The reference draws the word "Page" and neither number**;
  we draw "Page 1 of 2" — on *both* pages, which is the known `WritingFieldKind.PageNumber` cache
  defect showing at the same time. Two things are wrong at once here and neither is established:
  do not excuse this document on "the reference is wrong" until the mechanism that loses those two
  fields is named.

### What the next round should take, in order

1. **`w:tblStylePr` conditional formatting.** The table-style layer is now read and its
   `firstRow`/`lastRow`/banding half is not, which is the rest of the same defect and the rest of
   the same 46 documents. It needs `w:tblLook` and `GetLocalPropertiesFromMask`
   (`StyleSheetTable.cxx`:233).
2. **Re-sweep batches 015–021 for the border change.** Its reach is measured only on 001–014 here;
   the census says 29 of the 46 affected documents are in batches 015–021, including the four with
   the most affected tables in the corpus.
3. **The 0.35 pt line-gap placement**, above.
4. `A1. EASA Form 2.docx`, which is the largest single failure left in `batch-013`.

## Sheets, round twenty-one: Excel's row heights are read on a 0.75 pt grid — swept whole at `09a35cdae`

Whole track before and after, 171 documents each time, two workers, 171 rows with no duplicate
path and no `ref-failed` in either.

Two changes, each swept whole on its own.

| | baseline `09a35cdae` | + the row-height grid | + the embedded chart |
|---|---|---|---|
| matches | 142 / 171 | **144 / 171** | 144 / 171 |
| total absolute page error | 106 | 99 | **94** |
| exactly-correct page counts | 149 | 151 | **153** |
| `sheets/batch-001`–`009`, the gate | 89/89, page error 0 | 89/89, page error 0 | **89/89, page error 0** |
| `sheets/batch-010` | 6/10, error 9 | 6/10, error 9 | 6/10, **error 4** |
| `sheets/batch-011` | 5/10 | **6/10** | 6/10 |
| `sheets/batch-012` | 8/10 | 8/10 | 8/10 |
| `sheets/batch-017` | 5/10 | **6/10** | 6/10 |

**The baseline reproduces round twenty's closing figures to the digit** — 142, 106, 149 — which is
the first round on this track where the inherited numbers needed no correction at all. Per batch
at the baseline, for whoever needs it next: 001–008 10/10, 009 9/9, 010 6/10, 011 5/10, 012 8/10,
013 8/10, 014 9/10, 015 5/9, 016 4/9, 017 5/10, 018 3/4.

### The first change: a row height Excel wrote is read on a 0.75 pt grid

LibreOffice's OOXML filter rounds a row height **down** to a multiple of 0.75 pt —
`fHeight -= fmod(fHeight, 0.75)` — on `sheetFormatPr/@defaultRowHeight`
(`worksheetfragment.cxx:681`) and on every `row/@ht` (`sheetdatacontext.cxx:316`). Both are gated
on `isMSODocument()`, which is `docProps/app.xml`'s `<Application>` beginning with "Microsoft" and
nothing else (`xmlfilterbase.cxx:241-245`). BIFF12 states heights in whole twips and applies
neither, so XLSB is deliberately untouched.

The rule arrived as a killed agent's unswept commit and was re-derived rather than trusted: two
packages differing in `docProps/app.xml` and in **no other part**, round-tripped through the
installed `soffice`, come back with 18.6 pt read as 18.0 against 18.6, 29.4 as 29.25 against 29.4,
and a sheet default of 14.4 as 14.25 against 14.4 — while 15.0 and 30.0, already on the grid,
agree on both sides.

### One inherited claim did not survive, and it had a green test defending it

The commit argued that the subtraction must be spelled `h - h % 0.75` rather than
`floor(h / 0.75) * 0.75`, because the second "is free to land a unit in the last place below" and
would turn 585 twips into 584. It does not, on that value or on any other: checked over every
height Excel can write — each hundredth of a point and each twip to the 409.5 pt ceiling, 49142
values — the two never disagree. The test written for it therefore could not fail, and was
replaced by one that does.

Five faithful wrong implementations were built and run: no rounding at all (3 of 5 tests fail),
rounding without the generator gate (2), the floor-divide above (**0**), rounding only `row/@ht`
(2), and rounding only the sheet default (2).

### Reach, and the one document it costs

56 of the 109 corpus SpreadsheetML workbooks state a height the rule would move; **nine** are
documents whose rendering changes. `NAARMO_Mexico_RVSM_Approvals.xlsx` goes 17/16 → **16/16** and
`EASA_PRODUCT_LIST_-_ALL.xlsx` 270/264 → **264/264**; `Capability_List_…_unsorted.xlsx` improves
+6 → +3 pages without matching.

`ODs-February-2022-Airbus-Commercial-Aircraft.xlsx` is worse, −18 → −21 pages. That is the
mechanism working rather than failing: it already under-paginates and shorter rows under-paginate
further. Both continuous measures improve over the same change, so it stands.

NAARMO is exactly the lead round twenty exposed and predicted. It had been passing on two errors
cancelling; once the header band was fixed its drawn row pitch was measured at 14.40 pt against
the reference's 14.23, and the stated 14.4 snaps to **14.25**.

### The second change: a chart on an `.xls` worksheet took its pages down with it

`XlsDrawing.Build` kept only shapes carrying a picture or `TXO` text — right for a solver entry
or a group's own frame, wrong for a chart, whose `OBJ` is `ftCmo` type 5 and carries neither. It
therefore never reached `sheet.Drawings`, so `SheetDrawingArea` could not widen `PrintedRange`,
the range stayed invalid, and a sheet whose only content is a chart printed **nothing at all** —
not one blank page but none. Calc has the object on its draw page and `ScDocument::GetPrintArea`
takes the maximum of the cells' extent and the drawing layer's (`documen2.cxx:649-658`).

The chart substream that follows the object is now read too, and deliberately *not* the way a
chart sheet's is: an embedded chart's page records describe the chart's notional page rather than
the sheet's and arrive after the worksheet's own, so routing them onward overwrites the
worksheet's header, footer, margins and `SETUP`. Only the chart records are taken, and the
corpus fixture is built to prove it — its chart sheet carries a header and nothing else in the
workbook does.

Reach is a census rather than a grep: every `ftCmo` type across the track's 62 `.xls` gives 13
chart objects in **4 workbooks**. Five documents moved and none regressed.
`Template Pilot Logbook JAR-FCL V3.0.xls` goes 35/38 → **38/38** and
`EHEST-Pre-departure-checklist…xls` 22/24 → **24/24**. `TOGAF9-Tool-ConfReqts-CSQ.xls` was the
risk — it matched before and carries a chart — and it still matches.

**It wins no match and it is right anyway**: page error 99 → 94, exact page counts 151 → 153, and
`batch-010`'s page error 9 → 4. Neither of the two documents whose pages are now correct passes
the word gate, because an `.xls` chart states its series as cell ranges on another sheet and
nothing resolves them: the chart draws its title and axis titles and then a value axis scaled
from an empty series, 0 to 12 where the reference reads 0 to 180. That is worth 305 words on the
Logbook and 557 on EHEST and is the next piece of the same work.

**Nothing outside `Paperless.Spreadsheets` was touched** by either change, so the words and slides
tracks cannot be affected and were not swept.

### Tests

Per project, run separately, each redirected to a file. **0 failed and 0 skipped in every one.**

Core 243, Containers 109, Text 237, Vector 291, Rendering 104, Markup 259, OpenDocument 125,
WordProcessing 608, Spreadsheets **446** (was 437 — two new files, of five and four),
Presentations 517, Fidelity 542.

One trap worth passing on, because it produced a red run that meant nothing. A mutation script
that restores the source but does not rebuild leaves the *mutant's* assemblies on disk, and the
per-project loop this file recommends runs `--no-build`. That reported `Failed: 1` on exactly the
test the last mutation had been written to break. Rebuild between restoring and re-running, or
the suite measures the mutation.

### What the next round should take, in order

1. **An `.xls` chart's series are not resolved.** The chart now draws, and it draws a value axis
   fitted to no data. `XlsChartBuilder` gets the chart records; what it does not get is the cell
   ranges the `BRAI` records name, which on both corpus documents live on a different sheet. Worth
   305 words on `Template Pilot Logbook JAR-FCL V3.0.xls` and 557 on
   `EHEST-Pre-departure-checklist…xls`, which are the only two things standing between those and a
   match now that their page counts agree.
2. **`FAA-2019-0995-0002_attachment_2.xlsx`** (32/33) is a *wrap* difference, not a pitch one: the
   reference's repeating row group on its "Accessory List" sheet is 28.70 pt against our 14.93,
   because it breaks each row onto two lines where we fit one. The residue is the width the text
   is measured at, which is the ninth sweep's device model and its standing caveat.
3. **The three under-paginating documents with no furniture**, unchanged from round twenty and
   untouched by this one — the band rule cannot reach them and neither can this.
4. `Application_Compliance_Checklist_5_Apr_2021.xlsx` remains diagnosed and deliberately unfixed
   at a measured reach of 1 of 109.

## Sheets, round twenty: a header's band is measured, not stated — swept whole at `b7950ffd5`

Whole track before and after, 171 documents each time, two workers, no duplicate path and no
`ref-failed` in either.

| | baseline `b7950ffd5` | after |
|---|---|---|
| matches | 139 / 171 | **142 / 171** |
| total absolute page error | 111 | **106** |
| exactly-correct page counts | 147 | **149** |
| `sheets/batch-001`–`008`, the gate | 80/80 | **80/80** |
| `sheets/batch-009` | 8/9 | **9/9** |

**The baseline is one match above what round nineteen reported at its final commit** (138) with
the same page error and the same exact count. The row that differs is a word-count verdict, so
this is most likely LibreOffice's own non-determinism — already recorded on this track, where
`PBN Matrix NAAs (V01).xlsx` returned 5554, 5557 and 5556 across three runs of the same binary
on the same file. Every other figure in the brief reproduced.

### The change

SpreadsheetML and BIFF state a header band as the difference of two margins, and Calc keeps
neither number. Its filter measures the band's text as the bare point size, stores
`bodyDistance = statedBand − nominal`, and then re-measures the text properly at print time and
adds the distance back. So

```
printedBand = statedBand + max(0, measuredLineHeight − nominalPointSize)
```

which is about a tenth of the font size, taken off the printable body on every page. The
citations and the arithmetic — which reproduces LibreOffice's own flat-ODF export to the digit
on three separate workbooks — are in `src/Paperless.Spreadsheets/TODO.md`. The band's default
font is the workbook's own default cell font and not a fixed ten point, which two documents
disagreeing with each other were needed to establish.

This resolves the contradiction round nineteen left on `RegChangeReport.xlsx`. Its "the exported
page geometry gives 661.6 pt, which reproduces none of the breaks" is not a number Calc
paginates with: its page rectangle subtracts the footer's *height* and not its *spacing*
(`printfun.cxx:3003`). The body is 684.0 pt and the breaks want `[681.62, 682.14)`, so the gap
was 1.9 pt rather than 20. That document now matches, at 3131 words against 3137.

### Four documents gained, one lost, and the lost one is right

`NAARMO_Mexico_RVSM_Approvals.xlsx` went 16/16 to 17/16. It was passing because two errors
cancelled: its drawn row pitch is 14.40 pt against the reference's 14.23, 1.2% tall, and a body
2.4 pt too long was hiding it. Page error and exact page counts both improved over the same
change, so it stands — this is the pattern this file already records under "fixes that cancel".

`fy2011-aip-grants.xls` is a BIFF workbook and moved only on the second of the two commits, the
one that takes the band's default font from the workbook. The rule reaches both filters.

**Nothing outside `Paperless.Spreadsheets` was touched**, so the words and slides tracks cannot
be affected and were not swept.

### Tests

Per project, run separately, each redirected to a file. **0 failed and 0 skipped in every one.**

Core 243, Containers 109, Text 237, Vector 291, Rendering 104, Markup 259, OpenDocument 125,
WordProcessing 608, Spreadsheets **437** (was 432 — one new test file of five; one existing test
was rewritten rather than added to), Presentations 516, Fidelity 541.

`SheetPaginationTests.AnOoxmlTopMarginSurvivesTheHeaderBandConversion` asserted that
`TopMargin + HeaderHeight` comes back out as the file's own `top` margin. That is not an
invariant Calc keeps and the test is rewritten to the measured value rather than relaxed.

### What the next round should take, in order

1. **The three documents that under-paginate with no header or footer anywhere** —
   `flightstandards-doc-Cross-reference-table_version02.xlsx` (461/464),
   `tk-syllabus-comparison-document-v5.xlsx` (852/855),
   `sectors-defense-and-aerospace.xlsx` (225/227). The band rule cannot reach them by
   construction. On the last, both renderings put page 20's ink between y 71.8 and y 745.6, so
   the body agrees and the residue is row heights; note the sign, because rows measured too
   *tall* should over-paginate and these under-paginate.
2. **`Application_Compliance_Checklist_5_Apr_2021.xlsx`** is diagnosed and deliberately not
   fixed: collapsed row outline groups our XLSX reader never hides, worth +4 pages and +8635
   words on that one document and reaching **1 of the 109** corpus SpreadsheetML files. Take it
   only if the corpus grows more of them.
3. **`RegChangeReport.xlsx`'s footer face.** The band now honours `&"font"` and `&<size>` for its
   *height*; `SheetPageDecoration` still draws every band at one size in one face. Splitting a
   band into sized, faced runs closes both the missing Carlito embedding and the `&1` code that
   puts the reference's `#` on its own baseline.
## Slides, round eighteen: the rung a plain text box actually lands on

Swept whole twice at `b7950ffd5`, 163 documents each time, both against checksummed CLI snapshots
of this worktree, with the snapshot verified to move a document known to move before the run
started. 163 rows, no path twice, 0 `ref-failed`, both times.

| | baseline `b7950ffd5` | after |
|---|---|---|
| word gate | 151 / 163 | **151 / 163** |
| signed `ink%` | 1406.70 | **1360.08** |
| unsigned `\|ink\|%` | 1752.00 | **1680.10** |
| major pages | 462 | **438** |

**The baseline reproduced all four of round seventeen's post-fix figures to the digit**, and its
per-batch split with them. No batch moved: 001–007, 011, 013 and 015 full, 008 9/10, 009 9/10,
010 8/10, 012 8/10, 014 7/10, 016 8/10, 017 4/5.

27 documents moved, all `.pptx`. On `|ink|%`: **25 better, 2 worse, 136 unchanged — 72.26 won
against 0.36 lost.** The two against are `NAS-Infrastructure-Roadmaps-HSI.pptx` (3.28 → 3.50) and
`Sean Monogue.pptx` (1.54 → 1.68). Five documents moved on the parity row and no verdict changed;
`3492.pptx` lands on the reference's word count exactly and `Wildlife for REDAC September 11.pptx`
goes from 9 embedded faces to the reference's 11.

### The fix: `p:otherStyle` is not on the chain, and the old fixture could not show it

`PptxTextStyles.Chain` fell back to the master's `p:otherStyle` for any shape naming no
placeholder style. The reference reaches `getOtherTextStyle()` only through

```cpp
bool isOther = !getTextBody() && sServiceName != "com.sun.star.drawing.GroupShape";
```

`oox/source/ppt/pptshape.cxx`:424-429 — fetched at tag **`libreoffice-24.2.7.2`**, the release
that made the reference PDFs, and byte-identical to this checkout's 27.2-alpha. **A shape that
has text cannot satisfy it**, so a plain text box — and equally a `sldNum`, `ftr`, `dt` or `hdr`
placeholder, which name no style either — takes `getDefaultTextStyle()`: the presentation's
`p:defaultTextStyle` (`presentationfragmenthandler.cxx`:115 builds it and hands it to every
`SlidePersist`). `src/Paperless.Presentations/Ooxml/PptxTextStyles.cs`:220 is the fork.

**The mechanism had been correctly cited for two rounds and recorded as a deliberate divergence.**
`src/Paperless.Presentations/TODO.md` had the *other* route to it — `SlidePersist::createXShapes`
looping `for (int i = 0; i < 4; i++)` over a switch whose `case 4` is the style `p:otherStyle`
parses into, so the loop stops one short — beside the note that "Paperless does apply it … which
is what the file says and what PowerPoint shows".

What was missing was a document that could tell the readings apart. `deck-text-style.pptx` cannot:
its `otherStyle` states 18 pt and LibreOffice's own fallback for a slide run is *also* 18 pt, so
both readings predict the same number, and the reference duly draws its seven boxes at exactly
18.0000. **`tests/corpus/features/slide-other-style.pptx` was authored to separate them** — 12 pt
and 10 pt magenta in `otherStyle` against 24 pt green and 32 pt blue in `p:defaultTextStyle`, two
plain text boxes at those two levels stating neither size nor colour, so 12 pt magenta,
24 pt green and 18 pt black are three distinguishable answers. The reference draws
`0 0.5019607843 0 rg … 24.009 Tf` and `0 0 1 rg … 32.003 Tf`; we drew 11.99 and 10.01 magenta.
Size and colour move together, which is what says the rung is wrong rather than one property of it.

The lead that found it was round seventeen's census: thirteen pages of `171128IPAP.pptx` at
**exactly −25.00%**, ours 13.50 against 18.00 — that deck's `otherStyle` `sz="1350"` against its
`defaultTextStyle` `sz="1800"`. The deck goes `|ink|%` **50.58 → 17.40**, major pages 18 → 6, and
leaves the census's worst-documents list outright.

`research/probes/slides-r18/count-otherstyle.py`: 110 of the track's 112 pptx-family decks state a
`p:otherStyle`, 110 state a `p:defaultTextStyle`, and **33 state the two at different level-one
sizes**. That is an upper bound, not an estimate — the rung only decides a run that states no size
anywhere nearer. Measured reach is the 27 documents whose rendering moved.

### The size census cut two ways it had not been cut

**Round seventeen's em fix did exactly what it predicted.** Its census covered 129 of 163
documents and found 1642 differing pages, 1043 of them the ≤1% quantisation band. Re-run whole on
this round's baseline — a *larger* sample, 163 documents — the total is **711 and the ≤1% band is
25**. The class is gone.

The 686 that remain over 1% are not one body of work, and two probes in
`research/probes/slides-r18/` say so.

**`census-split.py` — a reference size off the 1/100 mm grid is metafile text.** Every size the
reference resolved through the shape model is `n × 72/2540`, because that is what an
`SvxFontHeightItem` holds for a draw object. **135 of the 686 are off it**, over 45 documents, and
it re-ranks the census: `NAS-Infrastructure-Roadmaps-v16.0`, first at 42 pages, is 26 metafile
against 16 model; `2014BSA_Sunday_Killion`, second at 29, is 18 against 11, its reference drawing
**208 operators at 11.400 pt** on page 6 against our 45 at 14.980.

**`size-census-2.py` — the dominant size is decided by an operator count, and the two writers
split lines differently.** `ws_prod-g-doc-Events-2007-september-M.017-(French)-France.ppt` page 3
is reported as *ours 20.010 x7 against ref 10.010 x8, +99.90%*, the largest single ratio class in
the run, and every size on the page agrees:

```
ours  [(31.02, 2), (28.01, 1), (20.01, 7), (15.99, 3), (10.01, 5)]
ref   [           (28.01, 1), (20.01, 7), (15.99, 3), (10.01, 8)]
```

The reference splits the slide-number footer into four portions at x = 635.6, 639.1, 645.8 and
649.3 where we write one at 638.7. Fifteen of that deck's census pages collapse to five, and five
decks of the family carry the class.

| | baseline | after |
|---|---|---|
| pages compared | 4199 | 4199 |
| dominant size agrees | 3488 | **3519** |
| ≤1% — the mm100 grid | 25 | 25 |
| same sizes cut up differently | 128 | 133 |
| the reference draws far less — raster ceiling | 65 | 62 |
| off the mm100 grid — metafile text | 135 | 135 |
| **none of those explains it** | **358** over 99 docs | **325** over 96 docs |

So the track's size residue is **325 pages, not 537** — and it is startlingly homogeneous.
Thirteen of the twenty commonest (ours, ref) pairs are **one point apart**: 18.99↔20.01,
24.01↔22.99, 27.01↔28.01, 17.01↔18.00, 20.01↔18.99, 15.00↔15.99, 18.00↔18.99, 25.99↔25.00,
22.00↔22.99, about 92 pages between them. That is the autofit search landing one grid step away,
which round seventeen pinned on `2015-Civil-Rights-Website-training.ppt` page 21 to a **16 mm100
window — 0.45 pt, 0.14% of the block** — and attributed to the *heights measured* rather than to
the search.

### The regression guards, and the tests

Nothing outside `Paperless.Presentations/Ooxml` was touched — `PptxTextStyles` serves no other
family — so the words and sheets tracks are not owed a sweep, and every one of the 27 documents
that moved is a `.pptx`.

Per project on the final tree, each run redirected to its own file: Core 243, Text 237,
Containers 109, Vector 291, Rendering 104, Markup 259, OpenDocument 125, WordProcessing 608,
Spreadsheets 432, Presentations 517, Fidelity 542. **Zero failed and zero skipped throughout.**

Three assertions, each verified by putting the fallback back and watching it fail:
`AMastersOtherStyleReachesNothingThatHasText`,
`APlainTextBoxTakesThePresentationsDefaultTextStyle` (both fidelity, against the reference's own
PDF) and `APlainTextBoxEndsAtTheDefaultTextStyleRatherThanTheMastersOtherStyle` (a unit test, so
the wiring is covered on a machine with no LibreOffice). The first of those replaces a test that
asserted the divergence — `Rgb(Resolved(path)[5]).ShouldBe(0xFF00FFu)` — and would otherwise have
locked the defect in.

**One test as first written could not fail.** `APlainTextBoxTakesThePresentationsDefaultTextStyle`
began by asserting only what `Drawn()` returns, which is LibreOffice's PDF — a test of the
reference, passing whatever Paperless does. It now asserts our resolved size and colour as
literals and compares the reference against those.

### What the next round should take, in order

1. **The 325-page size residue, and specifically its one-point class.** 92 of the 325 are the
   autofit search settling one grid step from the reference, and round seventeen already pinned
   the box on `2015-Civil-Rights-Website-training.ppt` page 21 to a 16 mm100 window with the
   arithmetic on both candidates written out. **The search itself is not the defect** — it was
   compared statement by statement against `autoFitTextForCompatibility` at 24.2.7.2 and is
   identical including ties — so this is a *height measurement*, worth about a third of a point
   over nineteen lines. `ITE106-Chapter 4.ppt` (16 pages, and 36.14 `|ink|%`, sixth on the track)
   is the same class on a `.ppt` and shrinks consistently *further* than the reference.
2. **Quantise the shape's text area the way the em now is.** Still open and still worth doing:
   `PptxSlideLayout.cs`:1663 returns `a:off`/`a:ext` in EMU "needing no conversion", where oox
   passes both through `GetCoordinate` — `o3tl::convert(emu, mm100)`, which is `(n + 180) / 360`
   (`oox/source/drawingml/drawingmltypes.cxx`:43) — so the width the reference breaks lines
   against is a whole number of hundredths of a millimetre and ours carries the file's full EMU
   precision. Bounded at half a unit, 0.014 pt, on a width of tens of thousands: an order of
   magnitude below the em, so expect it to move few documents and do it on its own sweep.
3. **`8_P-Pavese` page 16's axis slot width**, in `Paperless.Core/Charts`. Unchanged from round
   seventeen: a shared layer, so it owes the words and sheets sweeps.
4. **`TODO.raster-ceiling.md`'s 25% threshold**, unchanged from round seventeen.

**Do not spend a round on the census's top line.** Its raw ranking counts metafile text and
operator-granularity artefacts as size disagreements, and on the two documents it ranked first and
second those are more than half the signal. Run `size-census-2.py`, not `size-census.py`.

## Words, the twelfth round finished: the leading is Writer's rule, not every engine's

The predecessor's fix (`3db1d2816`, merged, measured and backed out) was right about where the
leading goes and wrong about who it goes for. It changed `LineMetrics.ScaledAscent` for everybody,
and nine tests across three families said so. **Seven of the nine were right**; re-deriving each
from LibreOffice's own rendering is what this round did, and the answer changed the fix rather
than the tests.

### The measurement that separates the two engines

Everything below is read out of LibreOffice 24.2.7.2's own PDF **content stream** — the text
matrix's translation at a `Tj`/`TJ`, not `pdftotext -bbox`, whose boxes come from the font
descriptor's declared ascent and which the two producers set differently.

| Engine | What LibreOffice does | Measured |
|---|---|---|
| **Writer**, own text | ascent **+** external leading | Liberation Sans 11 pt in a 72 pt top margin: first baseline **82.3008**, i.e. 206 twips (199.15 + 7.20, each rounded), not 199 |
| **EditEngine** — Impress, Calc, Writer's drawing objects | ascent only | Impress, Liberation Sans 18 pt table cell: two baselines **20.154 pt** apart, which is ascent + descent; the gap would make it 20.698. Calc, Liberation Sans 10.006 pt wrapping cell: **11.197 pt** pitch = round(ascent) + round(descent) in mm100, 320 + 75 = 395 |

LibreOffice draws the line itself, and both halves are one `if`:

- `SwFntObj::GetFontAscent` adds the external leading to the ascent, guarded only by
  `#if !defined(MACOSX)` (`sw/source/core/txtnode/fntcache.cxx`:326-329); `GetFontHeight` adds it
  to ascent-plus-descent (`:370-371`), so the descent stays the face's own and the three close.
- `ImpEditEngine::RecalcFormatterFontMetrics` adds it **only when `IsAddExtLeading()`**
  (`editeng/source/editeng/impedit3.cxx`:3133-3135), and that is false unless something turns it
  on — `ImpEditEngine` initialises it so (`impedit2.cxx`:118), as does `SdrModel`
  (`svx/source/svdraw/svdmodel.cxx`:161). Only Writer's own compatibility setting and Math's
  engine ever set it. `FormatterFontMetric::GetHeight()` is `nMaxAscent + nMaxDescent`.

**This was already in the tree.** `SlideTextLayout.cs`:613-620 states the Impress half, with the
20.15-against-20.70 measurement, in a comment written two rounds ago. A shared-layer change that
contradicts a recorded measurement one layer up is the shape to watch for.

### The shape of the fix

`LineMetrics` gains `LeadingAboveText`, **default false**. `ParagraphLayouter` and
`MeasuredParagraph.Measure` thread it, because all three families share them. The six Writer sites
opt in through `WriterLineBox.LeadingAboveText`, which carries the citations in one place.

Deliberately left at the default, and worth stating because each is a judgement rather than an
oversight: `Paperless.Vector`'s metafile and EMF+ text engines (VCL's `mnAscent` excludes
`mnExtLeading`, and a metafile is played through `OutputDevice::DrawText`), `FrameChart` (a chart's
text is drawn by its own model, not by Writer's layout), and every `Paperless.Spreadsheets` and
`Paperless.Presentations` site. The predecessor's version changed all of these.

### Re-derived, not re-fitted

| Test | Value | From | Agreed with the fix? |
|---|---|---|---|
| `SlideTablePlacementTests.ACellsTextStartsAtItsOwnMarginAndNotAtTheBodysInsets` | 91.928 | reference PDF of `slide-table-grid.pptx`, exact | **No** — assertion was right |
| `SlideTablePlacementTests.ACellsLineHeightIsTheFacesAndNotTheEm` | 20.154 | same PDF, baselines 235.9280 and 256.0820 | **No** — assertion was right |
| `SlideTablePlacementTests.ARowWithNoStatedHeightGrowsToItsText` | 263.537 | same PDF, the last two stroke paths | **No** — assertion was right |
| `SheetHardBreakTests.ABreakBreaksEvenWhenTheWholeTextWouldFit` | 11.197 | reference PDF of `sheet-cell-hard-break.fods`, 66.7848 → 77.9818 | **No** — assertion was right |
| `SheetHardBreakTests.ARichCellBreaksAtItsParagraphs` | 11.197 | same PDF, 171.8938 → 183.0908 | **No** — assertion was right |
| `SheetHardBreakTests.AnEmptyParagraphTakesALineOfItsOwn` | 22.394 | same PDF, 136.8858 → 159.2798 | **No** — assertion was right |
| `SheetPaginationTests.AnOoxmlHeaderBandIsTallerThanTheMarginsImply` | — | not named in the handover; the fourth Spreadsheets failure | **No** — assertion was right |
| `PictureReadingTests.AnInlinePictureWidensTheLineItSitsOn` ×3 | **30.95**, was 31.46 | reference PDFs of `picture-anchor.{fodt,odt,docx}`, all three identical | **Yes** — the assertion was wrong |

The one that changed is worth spelling out, because its document cannot see the defect directly.
All three spellings put LibreOffice's two baselines at **85.0508 and 98.8508**, and *both* models
predict exactly that: the gap leaves line 1's descent and arrives in line 2's ascent, so it cancels.
What does not cancel is the box boundary between them. Line 1's ascent is the picture's centimetre
(85.0508 − 56.6929 = 28.358) and line 2's ascent is Liberation Serif's plus its 87/2048 gap
(11.203), so line 1's box is 98.8508 − 11.203 − 56.6929 = **30.955**; independently of the margin,
13.80 pt of pitch − 11.203 + 28.347 = 30.944. The old 31.46 is the same arithmetic with the gap
charged to line 1's descent. Our own baselines on that document are 85.0429/98.8429 before *and*
after — unmoved, and 0.008 pt from the reference either way.

`sheet-cell-hard-break.fods`' class remark quotes `Golf` at 127.83 and `Hotel` at 150.22; today's
render puts them at 136.8858 and 159.2798. **The delta it asserts, 22.394, is unchanged** — only
the absolute positions have moved since it was written. Left alone rather than restated.

### The probe, re-measured at this commit with the fix in

First baseline below a 72 pt top margin, ours against LibreOffice's, both from the content stream.
The "before" column is the predecessor's, reproduced here at 11 pt exactly and inherited for the
rest:

| face, pt | ours before | ours after | LibreOffice | after − LO |
|---|---|---|---|---|
| Sans 8 | 79.2500 | **79.5000** | 79.5008 | 0.0008 |
| Sans 11 | 81.9500 | **82.3000** | 82.3008 | 0.0008 |
| Sans 16 | 86.5000 | **87.0000** | 87.0508 | 0.0508 |
| Sans 22 | 91.9000 | **92.6500** | 92.6008 | −0.0492 |
| Sans 40 | 108.2000 | **109.5000** | 109.5008 | 0.0008 |
| Carlito 8/16/22/40 | — | unchanged | — | 0.0008 |
| Carlito 11 | — | unchanged | 82.5008 | 0.0508 |

Up to 1.30 pt out before, at most 0.051 pt — one twip — after. **Carlito does not move at all**,
its `hhea` gap being zero, which is why a systematic error survived: Carlito is what nearly every
OOXML document in this corpus resolves to through its theme. The 16 pt residue is also the only
size whose *pitch* differs (18.40 against 18.45), so a second quantisation acts there; not chased,
because naming it would not change the placement rule.

### Refuted, and worth recording so it is not re-derived

Writer's `ADD_EXT_LEADING` really is a document compatibility setting — false for a flat ODF that
omits it (`sw/source/filter/xml/xmlimp.cxx`:1482), true for DOCX since `w:noLeading` defaults false
(`SettingsTable.hxx`:157) — so the obvious hypothesis is that it has to be modelled. It does not.
Rendering the same content with the setting present-true, present-false and absent gives
**byte-identical baselines** in 24.2.7.2, on three faces at 11 pt. Not modelled.

### Sweeps: all three tracks, because `LineSpacing.cs` is shared

Two checksummed builds (`Paperless.Text.dll` md5 `19318bb…` before, `349739f…` after) with
demonstrably different output — `page-top-line-gap.docx` first baseline 81.95 against 82.30 — and
no rebuild during either sweep.

| | rows | match before → after | page-exact | page error | word error |
|---|---|---|---|---|---|
| words 001–019 | 188 | 152 → **152** | 160 → 160 | 69 → 69 | 4604 → 4606 |
| slides 001–009 | 88 | 86 → **86** | 88 → 88 | 0 → 0 | 378 → 378 |
| sheets 001–009 | 89 | 89 → **89** | 89 → 89 | 0 → 0 | 817 → 815 |

No duplicated path in any of the six, no `ref-failed`, every row count exact. The words baseline
reproduces `b7950ffd5`'s **to the digit and per batch** — 001–005 10/10, 006 9/10, 007 10/10,
008 9/10, 009 10/10, 010 8/9, 011 9/10, 012 9/10, 013 6/9, 014 4/10, 015 5/10, 016 8/10, 017 6/10,
018 6/10, 019 3/10 — which is the tell that the base and the measurement are both right.

What actually moved: **words, two rows of 188, one word each**, neither verdict changing
(`JEMIT_Template.docx` 1678 → 1677 against 1684, `24-25_FAA_Holdover_Tables.docx` 70714 → 70715
against 70663). **Slides, nothing at all** — every one of the 88 rows byte-identical, which is the
strongest available statement that the change does not reach Impress. **Sheets, one row**, and it
moved on the *reference* side: `PBN Matrix NAAs (V01).xlsx` is 5559 ours both times against 5555
then 5557 theirs. Our output there is unchanged; treat LibreOffice's word count on that document as
not quite reproducible rather than as a result.

The gate is unmoved on purpose. The change costs 0.35 pt of vertical budget per page on a
Liberation Sans document and nothing at all on a Carlito one, so it can only flip a document
already within a third of a point of a break. It is kept on its own evidence.

### Tests, and the three defects each was watched failing under

`Paperless.Text` +3, `Paperless.Fidelity` +1 (`page-top-line-gap.docx`, recovered from the
abandoned branch and re-validated against LibreOffice before use).

| defect put back | what failed |
|---|---|
| **A**: gridless ascent excludes the gap (the original bug) | `TheLeadingSitsAboveTheTextWithoutAGridToo`; `AnInlinePictureWidensTheLineItSitsOn` ×3 at 31.45 against 30.95; the new fidelity case. Presentations and Spreadsheets **0** |
| **B**: gridless ascent always includes it (the predecessor's version) | `AnEngineThatDoesNotAddTheLeadingLeavesTheLineShortOfItsHeight`; the three `SlideTablePlacementTests`; the four `SheetHardBreakTests`/`SheetPaginationTests`. WordProcessing **0** |
| **C**: grid path never adds it | `TheLeadingSitsAboveTheTextRatherThanBelowIt` |

`AFaceStatingNoLineGapIsUnaffectedByWhereTheLeadingSits` passes under **A and B both** and is not
claimed to detect either. It is a control: it pins that the change is a placement rather than an
addition, and it would catch a constant added to the ascent.

Per project, each run redirected to its own file: Core 243, Text **240** (was 237), Containers 109,
Vector 291, Rendering 104, Markup 259, OpenDocument 125, WordProcessing 608, Spreadsheets **437**,
Presentations 517, Fidelity **543** (was 542). Zero failed and zero skipped throughout.
**Spreadsheets 437 is not this round's doing** — nothing under `Paperless.Spreadsheets` was
touched. The 432 recorded last round is one round stale; `68d68502d` and `24e525b46` are in
`09a35cdae`.

### Still open on this track

- **`w:pBdr` is not implemented at all** — no reader, no model, no drawing; `grep -rn pBdr
  dotnet/src` still returns nothing. The census was **re-run this round and reproduces the
  predecessor's exactly**: of the words track's **134 DOCX, 56 declare a `w:pBdr`** — 39 in
  `styles.xml`, 26 in a header or footer part, 23 in the body, and **36 of the 56 in batches
  013–021**. Written to `scratchpad/words-<agent>/pbdr-census.tsv` as
  `path / styles / header-footer / body`.

  Read that as a reach *ceiling* rather than a prediction: counting files that state a property is
  not counting files whose output changes, and this project has measured that overstating by an
  order of magnitude before. What makes it the strongest lead anyway is that a paragraph border
  both draws **and takes height**, so it is an ink defect and a pagination defect at once — the
  eleventh round measured an empty bordered paragraph costing LibreOffice 1.50 pt we do not spend,
  26 times in one document.
- **Refuted last round, still refuted**: `w:tblStylePr` is 14 of 134 DOCX and exactly one of them
  is in batches 013–015.
- Batches 013–015 remain the weakest of the early range at 6/9, 4/10 and 5/10, and 29 of the
  track's DOCX in batches 015–021 have never been swept with the image check.
## Slides, round nineteen: a text rectangle on the grid, and a washout that is not the pair it states

Swept whole three times at `09a35cdae` — baseline, the text-rectangle fix alone, and both fixes —
163 documents each, every one against a checksummed CLI snapshot of this worktree, each snapshot
verified to move a document I knew it moved before its run started. 163 rows, no path twice,
0 `ref-failed`, all three times.

| | baseline `09a35cdae` | text rectangle | + picture recolour |
|---|---|---|---|
| word gate | 151 / 163 | 151 / 163 | **151 / 163** |
| signed `ink%` | 1360.08 | 1357.76 | **1283.56** |
| unsigned `\|ink\|%` | 1680.10 | 1677.66 | **1603.00** |
| major pages | 438 | 438 | **430** |

**Whole round: `|ink|%` 1680.10 → 1603.00 — 77.10, or 4.6 per cent of the track's residue — and
438 major pages → 430, with the word gate flat.**

**The baseline reproduced all four of round eighteen's post-fix figures to the digit** — 151/163,
1360.08, 1680.10, 438 — and its per-batch split with them. No batch moved and **no verdict changed
on any of the 163 documents in either sweep**, so `batch-check.sh` stands as the regression guard
it is now only good for: 001–007, 011, 013 and 015 full, 008 9/10, 009 9/10, 010 8/10, 012 8/10,
014 7/10, 016 8/10, 017 4/5.

The worktree opened **481 commits behind**, the ninth in a row and the largest gap yet.

### The text rectangle was never on the draw layer's grid — worth 2.44, and it refutes its own lead

`SlideTextLayout.Place` broke and anchored text against the rectangle the reader produced, at the
file's full EMU precision. The reference cannot: oox builds a shape's matrix in EMUs and scales it
into hundredths of a millimetre at the end (`oox/source/drawingml/shape.cxx`:1226-1230 at tag
`libreoffice-24.2.7.2`), `SvxShape` hands the result to `SdrObject::SetSnapRect`, and a
`tools::Rectangle` holds four `sal_Int32` of the model's map unit. The four text distances are
`SdrMetricItem`s of the same unit. `src/Paperless.Presentations/Layout/SlideTextLayout.cs`:69.

**Both edges round and the extent does not.** `round(bottom) − round(top)` is not
`round(bottom − top)`, so a box of one extent is a whole unit taller or shorter depending on where
on the slide it sits — which is why this cannot be written as a rounding of the width and height,
and why the fixture states two origins at one extent.

`tests/corpus/features/slide-text-area-grid.pptx` separates the readings: three boxes with all four
insets at zero and one 18 pt run each, at 2540.497, 5080.503 and 8890.497 units — two directions of
rounding on two axes. LibreOffice draws 72.000 and 144.028 pt across and a 216.000 pt baseline
separation; unquantised gives 72.0141, 144.0143 and 216.0141, and we drew exactly those.

**114 documents moved — 68 better, 46 worse, 5.39 won against 2.95 lost**, `|ink|%` 1680.10 →
1677.66, and no major page either way. That is the shape the brief predicted for it: nearly every
deck by a fraction of a point, winning about twice what it loses.

#### The lead I took it on was wrong, and the negative result is the more useful half

I argued that the autofit search is a *threshold* comparison, so a box one or two units out lands
on the wrong side of round seventeen's 16 mm100 window — which reads that window's *width* as the
size of the error when it is only where the true box lies. The pinned document refutes it.
`2015-Civil-Rights-Website-training.ppt` page 21 is **`ours 20.010 x31` against `ref 18.990 x31`
before and after, unchanged**, its 30 differing pages stayed 30, and its `|ink|%` went 33.20 →
33.34.

So the box is not what decides the one-point band. With the em on the grid since round seventeen
and the box on it now — and a fixture proving both edges land where the reference's do — **the only
unquantised quantity left in that comparison is the measured text height.** Rounds seventeen and
eighteen both said so; it is now an elimination rather than an attribution.

One caveat that is specific to that deck: it is a `.ppt`, and `PptSlideLayout.cs`:1240-1245 scales a
grouped shape's rectangle by `Math.Round(width.Emu * placement.A)` before `Place` sees it. I did
not measure that group scale against LibreOffice's, so on this document the box may still not be
the reference's.

### A blip's `a:lum` is a picture recolour, and PowerPoint's washout is not the pair it states

Found by ranking the baseline by `|ink|%` and looking at the *second* document.
`N2_E_Maestroni_Swarm_COP.pptx` carries 67.34 over thirty pages with one major page, and **page 1
alone is 63.62** — the reference washes its full-bleed satellite photograph almost white and we
drew it at full strength. The slide states `<a:lum bright="70000" contrast="-70000"/>` on its blip.

`GraphicProperties::pushToPropMap` (`oox/source/drawingml/fillproperties.cxx`:797-926, 24.2.7.2)
sorts that pair into three cases using **two different arithmetics**, and the first case discards
the numbers the file gives:

1. **70 and −70 exactly** selects LibreOffice's own `ColorMode_WATERMARK`, which is a fixed **+50
   luminance and −70 contrast** (`fillproperties.cxx`:826-831; `WATERMARK_LUM_OFFSET` and
   `WATERMARK_CON_OFFSET`, `vcl/source/graphic/GraphicObject.cxx`:53-54; applied through
   `BColorModifier_RGBLuminanceContrast`, `basegfx/source/color/bcolormodifier.cxx`:367-441).
2. **Both non-zero otherwise** is baked with *MSO's* formula, half the brightness either side of
   the contrast (`Bitmap::Adjust`'s `msoBrightness` branch, `vcl/source/bitmap/bitmap.cxx`:1694-98).
3. **One alone** goes through the colour modifier, all the brightness after the contrast.

**Measured against the binary rather than read out of this checkout's 27.2 alpha.**
`research/probes/slides-r19/check-washout.py` predicts the reference's page one from ours: mean
absolute error per channel is **7.15 of 255** under the watermark mapping, **30.98** under the pair
the file states through the same modifier, and 163.03 for drawing the picture untouched. Four times
better, so the mapping is the running binary's and not merely the source's.

`src/Paperless.Ooxml/DrawingML/DrawingFill.cs`:100-104 reads it, `LuminanceRecolour`
(`src/Paperless.Core/Graphics/GlyphRun.cs`) carries it, `PptxSlideLayout.cs`:1359 attaches it and
`RasterImageDecoder.ChannelMap` carries it out — the pattern `a:duotone` already used, because
turning a JPEG into a recoloured bitmap needs a codec and a reader has none.

**Swept: four documents moved, all four better, 74.66 won against 0.00 lost**, `|ink|%` 1677.66 →
1603.00 and major pages 438 → 430. Word counts and verdicts are identical between the two sweeps,
which is what a picture recolour should do.

| document | `\|ink\|%` | major |
|---|---|---|
| `N2_E_Maestroni_Swarm_COP.pptx` | 67.30 → **3.68** | 1 → **0** |
| `3492.pptx` | 15.35 → **7.26** | 3 → 3 |
| `Presentation - Identify Components of the Airport (1).pptx` | 2.73 → **0.81** | 7 → **0** |
| `111006 COMSTAC STOWG Aero spaceports IFG.pptx` | 1.25 → **0.22** | 1 → 1 |

The third is worth reading beside the headline: 2.73 of ink and **seven major pages**, all now
gone. A recolour that is wrong over a large pale area moves the region classifier without moving
much ink, which is the case `pdf-image-diff.py`'s two columns exist to separate.

**Reach**: `count-bliplum.py` parses rather than greps, since a *colour*'s `<a:lum>` shares its tag
name. Of the track's 112 pptx-family decks, **15 carry a blip `a:lum`** — twelve blips state the
washout pair, one states 20 and 20, and thirty-two state it empty and mean nothing by it. All three
cases occur in the corpus and the commonest is the no-op, which has to stay one: recolouring costs
the JPEG pass-through. **Fifteen carry one and four move** — the reach trap the skill warns about,
in the direction it warns about, and the reported reach is the four measured by rendering them.

#### The wiring defect this round found in itself

The first build changed nothing. `RasterImageDecoder.Ensure` builds a fresh image from the encoded
bytes and copies across only the fields it names; it named `Duotone` and not the new one, so the
recolouring was dropped on **every** picture — a decoded one never reaches that branch. The symptom
is a picture that decodes perfectly and draws untouched, which looks exactly like the feature not
being implemented. Found by rendering a deck, not by reading the code.

### The size census, before and after: byte-identical

| | baseline | after both |
|---|---|---|
| pages compared | 4199 | 4199 |
| dominant size agrees | 3519 | 3519 |
| ≤1% — the mm100 grid | 25 | 25 |
| same sizes cut up differently | 133 | 133 |
| the reference draws far less — raster ceiling | 62 | 62 |
| off the mm100 grid — metafile text | 135 | 135 |
| **none of those explains it** | **325** over 96 docs | **325** over 96 docs |

`size-census-2.py` over both whole sweeps, and **the two outputs are identical byte for byte** —
worst-documents list and commonest-pairs table included. Round eighteen's split therefore
reproduces exactly on an independently-produced sweep, and **neither fix moved one page's dominant
size anywhere on the track.** The page-21 check said the box does not decide the one-point band on
that deck; this says it decides nothing about size on any deck. The em, the box and the search have
each now been eliminated by measurement rather than by argument, which is what makes the measured
text height the next thing to go at.

### The regression guards, and the tests

Per project on the final tree, each run redirected to its own file: Core 243, Text 237,
Containers 109, Vector 291, Rendering **115**, Markup 259, OpenDocument 125, WordProcessing 608,
Spreadsheets 437, Presentations **520**, Fidelity **544**. **Zero failed and zero skipped
throughout.** The three that moved are this round's own tests. Note the base's Spreadsheets is 437
and not the 432 round eighteen recorded — sheets round twenty landed in the same merge.

Five assertions, each verified by reintroducing the defect and watching it fail:

- `ATextRectangleIsAWholeNumberOfHundredthsOfAMillimetre` (two cases) and
  `TwoBoxesOfOneExtentShareABottomEdgeWhenTheirEdgesRoundAlike` — unit, so the wiring is covered on
  a machine with no LibreOffice.
- `ATextRectangleIsRoundedToTheDrawLayersOwnGrid` — fidelity, **at a 0.005 pt tolerance rather than
  this suite's usual 0.1**, which is seven times the whole effect and could not fail.
- `AChannelLandsWhereTheReferencePutsIt` (eight levels × three arithmetics) and
  `TheRecolouringSurvivesADecode` — the expectations are LibreOffice's own levels, read column by
  column off the fixture's page.
- `EveryPictureRecolourIsTheOneLibreOfficeApplies` — fidelity, bounded by the *control band's own
  error plus one level* rather than by a constant, because both writers resample a 256-pixel ramp
  to 560 and that costs two levels whatever the recolouring does.

**Nothing outside `Paperless.Presentations` decides anything here, and that is checkable rather
than asserted**: `grep 'Luminance = '` over `src/` finds one setter and it is in
`PptxSlideLayout`, and `DrawingFill.ReadBlip`'s only callers are the two slide readers. The Core
and Rendering edits are an optional record field and a branch guarded on it being non-null. **I did
not run the words or sheets sweeps**; the claim above is from the call graph, and it is the same
claim the `a:duotone` work rested on.

### What the next round should take, in order

1. **The measured text height, which is now what is left of the one-point band.** The em and the
   box are both on the reference's grid, the search was compared statement by statement at
   24.2.7.2 in round seventeen, and this round's census is byte-identical before and after — so
   `Measure`'s output is the remaining term, by elimination and not by argument. Start by settling the
   caveat above — measure `PptSlideLayout`'s group scale on `2015-Civil-Rights` against
   LibreOffice's flat-ODF export of the same deck, which states the box in mm100 outright and needs
   no inference.
2. **`Demick_JetBlue.pptx`'s charts**, 27.00 `|ink|%` over 6 major pages of 10, which is the
   densest unexplained document I looked at. Its series are drawn black where the reference draws
   them in three colours, and its legend comes out as overlapping text at one position where the
   reference lays out a horizontal legend with markers. Both are in `Paperless.Core/Charts`, a
   shared layer, so it owes the words and sheets sweeps — and `8_P-Pavese` page 16's axis slot
   width is in the same file and should be done in the same round.
3. **The `.ppt` twin of the picture recolour.** Escher states it as
   `DFF_Prop_pictureBrightness`/`DFF_Prop_pictureContrast` and LibreOffice applies the *same*
   washout rule to it — `nContrast == -70 && nBrightness == 70` selects
   `GraphicDrawMode::Watermark` (`filter/source/msfilter/msdffimp.cxx`:3906-3960) — with the
   contrast on a non-linear scale about `0x10000`. **Unmeasured**: I did not count how many corpus
   `.ppt` files carry either property. Recorded because it is the same shape as the brief's own
   `a:prstTxWarp` warning, where scoping a DrawingML feature to pptx alone missed half its
   instances.
4. **`TODO.raster-ceiling.md`'s 25% threshold is already fully documented** and needs no work:
   that file's own "The threshold is a bar, and pages sit just under it" section states the
   under-count, names `8_P-Pavese` page 6 at 24.4% as the instance, and says why raising the bar is
   not the fix. Round seventeen's action item is done; it should come off the list.

## Sheets, round twenty-four: a BIFF chart's series were never resolved — swept whole at `73934b1b5`

**The track verdict does not move again: 144/171, total absolute page error 94, 153 exact page
counts**, identical before and after, no document changing verdict in either direction. Total
absolute word error 43903 → 43694. Both sweeps 171 rows with no duplicate path, each against a
checksummed copy of the CLI. The baseline reproduces round twenty-three's figures to the digit,
per batch as well as in total — 001–009 all pass, 010 6/10, 011 6/10, 012 8/10, 013 8/10,
014 9/10, 015 5/9, 016 4/9, 017 6/10, 018 3/4.

### The chart series, built at last

A BIFF series names its values through a `CHSOURCELINK` whose payload is a formula token array.
Nothing read them, so **every chart in a `.xls` plotted the empty default scale of 0…12**. Now
decoded: `XlsChartFormula` reads the single reference token a chart link consists of,
`XlsExternSheets` reads `SUPBOOK` and `EXTERNSHEET` — the only way an `ixti` becomes a sheet — and
the workbook reader pre-scans the whole stream for the rectangles any chart names, then reads
those sheets once before the content pass.

**The two-pass shape is the finding, not an implementation detail.** A chart is built the moment
its substream ends, and its substream is embedded in the sheet its *picture* sits on, not the
sheet holding the numbers. The first version resolved at the point of use: `EHEST` worked and the
logbook produced no series at all, because `EHEST`'s chart happens to sit on the sheet it plots.

The logbook now plots 0…1400 and 0…1200 against the reference's 0…1400 and 0…1200, with its three
series carrying the names the reference's legend shows; `EHEST` plots 0…90 against 0…90.

Second, smaller: `XclImpChLabelRange::Convert` turns `CHLABELRANGE`'s label frequency into
`TextOverlap` and `TextBreak`, with the reason beside it — *do not overlap text unless all labels
are visible* — and the default frequency is 1. So a BIFF category axis draws **every** label
whatever they collide with, and none of `ChartAxisLabels`' thinning applies. `ChartPlot`
already carried `CategoryAxisText`, so this is confined to `Paperless.Spreadsheets`.
**Nothing in `Paperless.Core` was touched.**

**Reach, measured by rendering all 171 with both CLIs and comparing the PDFs with their timestamps
masked: 4 of 171 change what is drawn**, all four `.xls` carrying charts; 167 identical. A fifth
document flagged by that comparison is a false positive — an `xlsx` no BIFF change can reach,
byte-identical when both CLIs are run under the same conditions. **Our PDF writer is not
reproducible across sweep runs**, which puts a false-positive floor under any byte-level reach
measure; worth knowing before the next one uses it.

### The chart-label font gap is not a special case — the census

Round twenty-three measured `ChartLabel`'s missing font family as reaching **one** sheets document
and asked for the other two tracks to be censused before anyone sized it. Censused by opening all
534 corpus documents: **words 1 of 200, slides 15 of 163, sheets 1 of 171** carry a `chart*.xml`
part. Nine of the fifteen slide decks carry more than one; one carries twenty-one. So it is a
shared defect that is merely rare on sheets, and the figure is a floor as well as a ceiling — the
102 binary `.ppt`/`.xls`/`.doc` documents are not counted, and `EHEST` is an `.xls` with no chart
part whose labels the reference plainly sets in Carlito.

It is also load-bearing here, which the one-document figure hid. `EHEST`'s residue is now −26
words on each of eight chart pages: the reference draws its 51 category labels at a 8.28 pt pitch
and `pdftotext` reads 51 tokens, we draw the same 51 at 7.30 pt and it reads one. **Our plot area
is 365 pt wide against the reference's 414**, and the difference is the legend beside it — two
entries in Carlito against three in Liberation Sans.

Not fixed this round, deliberately: it means widening a `Paperless.Core` type and the
`IChartTextMeasurer` interface while two other agents are in that library, and it owes a
whole-track sweep on all three tracks.

### Two of batch 010's four are now diagnosed rather than open

`Template Pilot Logbook JAR-FCL V3.0.xls` **cannot pass the word gate**, for two measured reasons
and neither is the chart data. Its category axis is a *date* axis in the reference — 615 category
cells of which 17 hold anything, and 30 evenly spaced ticks running from 30/12/1899 to about 2111,
a linear scale over date serials rather than one label per category. And of the 264 text records
on the reference's page 16, **251 hold exactly one glyph**: LibreOffice writes rotated text one
`Tj` per glyph, so 30 labels of eight characters score about 240 words where drawing them as
strings scores 30. The document is 1342 against 1610 and the reachable figure is about 1409.

`INDEX_Digital_Transformation_Toolkits.xls`: the 63-against-207 figure reproduces and is a count
of images **drawn**, not of records in the file. Pages 1–13 agree exactly, images included and in
the same positions; the divergence starts on page 14, where ours holds 10 images and the
reference's 21. The reference prints the same picture block twice — alone over its pages 13–18 and
again under the text on 19–24 — and so do we, over two pages each instead of six. **A shape walk
that stopped after 25 shapes would not place the first 15 exactly and then thin the sixteenth
onwards**, so that reading does not survive its own measurement; what the figures fit is a printed
range that does not extend far enough down to cover the drawings anchored below it, which is
`SheetDrawingArea`'s question rather than the Escher reader's.

Probe data, both sweeps, the census and the page tables are in `dotnet/probes/sheets-r24/`.

## Sheets, round twenty-three: the em is quantised through the device, not after it — swept whole at `9c5bef08c`

**The track verdict does not move, and that is the honest headline.** Swept whole before anything
was changed and again after the one fix: **144/171, total absolute page error 94, 153 exact page
counts, batches 001–009 at 89/89** — identical either side, every per-batch figure identical, no
document changing verdict in either direction. Total absolute word error goes 43961 → 43903.
Both sweeps: 171 rows, no duplicate path, zero `ref-failed`, each against its own snapshot of the
CLI. The baseline reproduces round twenty-two's four closing numbers to the digit.

Per batch, unchanged before and after: 001–008 10/10, 009 9/9, 010 6/10, 011 6/10, 012 8/10,
013 8/10, 014 9/10, 015 5/9, 016 4/9, 017 6/10, 018 3/4.

### The drawn font size, finally named

Round twenty-two found that a third of the *passing* documents draw their text at a constant
ratio off the reference's size — ours `6.7606`, the reference `6.803`, on a sheet printed at 75%
— and declined to assert a cause. The brief that followed proposed the slides track's 1/100 mm
grid quantisation, with a mechanism and a precedent. **That proposal is wrong, and wrong in the
direction the grid is already right**: our number *is* the em on the 1/100 mm grid, snapped and
then multiplied by the scale, and no rounding of 9 pt to a hundredth in either order reaches the
reference's 240.

What the reference does, fitted on eight authored probe sheets — sixteen sizes at each of seven
print scales, plus two sweeps in 0.05 pt steps, **178 of 178 observations reproduced exactly**:

```
h    = round(twips * 127 / 72)          # 1/100 mm — the snap we already had
p    = round(h * scale * 720 / 2540)    # whole device pixels, PDF reference device at 720 dpi
L    = round(p * 2540 / (720 * scale))  # back to whole logical units, through the *scaled* map mode
size = L * scale
```

The 720 dpi is measured, not assumed: at 100% the emitted sizes step in units of 2540/720 and
nothing else. Two consequences worth separating. At 100% the round trip is the identity for every
whole point size from 6 to 48, which is the whole of why this hid until a zoomed sheet — it is
*not* the identity in general, 8.25 pt going out at 289 hundredths rather than 291. And under a
zoom it is the identity for even whole point sizes and not for odd ones, 9 pt and 11 pt being the
two a spreadsheet uses most, which is why the first sighting looked like a per-document constant.

Rendering the probes with both renderers: **194 of 194 emitted sizes agree after, against 117 of
194 before**, measured by building the pre-fix CLI and rendering them rather than by modelling it.

**Reach, measured by rendering all 171 documents with both CLIs: 48 of them change at least one
drawn size, 269 361 text records in total.** None changes verdict, and none was ever going to:
`batch-check.sh` measures pages, words and embedding, and this class moves none of the three.
A fix that is real and moves no verdict is still worth having; reporting it as progress is not.

Probe data, both sweeps and the arithmetic are in `dotnet/probes/sheets-r23/`.

### Batch 010 is a chart batch, and cannot be closed without the chart work

All four failures rendered by both renderers and diffed page by page. Two `.xls` charts plot an
axis of 0…12 — the default scale of a plot with no series — where the reference plots 0…90 and
0…1400; one `xlsx` chart draws every label in Liberation Sans where the reference uses the
theme's Carlito; the fourth is round twenty-two's open `INDEX_Digital_Transformation_Toolkits.xls`,
not re-derived. The deficits are confined to the pages carrying a chart.

`ChartLabel` in `Paperless.Core/Charts` carries no font family at all, so `SheetChart`,
`SlideChart` and `FrameChart` all draw chart text in one hardcoded default. **Reach on this track
is one document** — exactly one of the 171 is a container holding an `xl/charts/chart*.xml` part —
so it is recorded rather than fixed here; the reach on the other two tracks is unmeasured and
likely larger.

## Sheets, round twenty-two: the bold that was never missing — swept whole at `9cffaa02a`

**Nothing moved, and that is the honest headline.** Swept whole before anything was changed and
again after: **144/171, total absolute page error 94, 153 exact page counts, batches 001–009 at
89/89** — the same four numbers either side, every per-batch figure identical, no document
changing verdict. The baseline also reproduces round twenty-one's closing figures to the digit,
so nothing has drifted under the scoreboard.

Per batch, unchanged before and after: 001–008 10/10, 009 9/9, 010 6/10, 011 6/10, 012 8/10,
013 8/10, 014 9/10, 015 5/9, 016 4/9, 017 6/10, 018 3/4.

### The brief's headline was wrong, and the measurement behind it was right

The brief said `Praktikastellen_…xls` — 34 pages against 34, 1828 words against 1828 — draws no
bold at all. Every figure it quoted reproduces exactly. The conclusion inverts: **we draw bold
correctly and always did.** Our two subsets are Carlito Bold and Carlito Regular by their own
`name` tables, `StemV` 140 against 80, and our bold subset's `/Widths` match the reference's
`Carlito-Bold` on 68.7% of shared codes against 1.5% for `Carlito-Regular`. `pdf-image-diff`
gives 34 pages, 0 major, `ink%` 0.00 throughout, and page 1 draws bold by eye.

The defect was one expression in the **shared PDF writer**: `PdfFontCatalogue.BaseName` took
`/BaseFont` from the face's family name rather than its PostScript name. Fixed in
`Paperless.Rendering`, so **this touches all three families** and the merge should expect it.

Reach, measured by rendering all 171 sheets documents before and after: **156 of 171** changed
their font-name set, and documents whose font-name set equals the reference's went **11 → 131**.
Zero verdicts moved, because `batch-check.sh` decides on pages, words and *unembedded* fonts and
a name reaches none of them.

Cross-track spot check on the same binary, since the change is below all three families:
`words/batch-001–003` and `slides/batch-001–003` — see the round's commits for the figures.

### What the round is actually worth: the first look at the passing documents

`pdf-ops.py diff` over all **89** matching documents of batches 001–009, which nothing had ever
done. Two thirds of them differ at operator level. The largest class is **drawn font size —
27 603 records over 34 of the 89** — one constant ratio per document, on sheets printed at a
zoom, and visible in the content streams (`/F1 6.7606 Tf` against `/F1 6.803 Tf`).

**The mechanism is not established and must not be guessed at**: `airports_6.xlsx` states
`scale-to="75%"` and a 9 pt font, and 6.75 pt is what *neither* side writes. The tempting
consequence — that a 0.63% short advance explains the under-pagination cluster — is **refuted**:
all three of those documents state no zoom at all.

### Tests

Per project, run separately, each redirected to a file. **0 failed and 0 skipped in every one.**

Core 243, Containers 109, Text 240, Vector 291, Rendering **119** (115 plus this round's four),
Markup 259, OpenDocument 125, WordProcessing 608, Spreadsheets 446, Presentations 520,
Fidelity 545.

Round twenty-one recorded Text 237, Rendering 104, Presentations 517 and Fidelity 542 at an
earlier commit than this round's base, so those deltas are other agents' work merged since and
not mine. Mine is the +4 in Rendering.

### Verified on the merged branch at `9c5bef08c`

The figures above are the agent's, measured in its worktree. Re-run here after the merge, which
is the only number that describes what is on the branch: **every project green at its expected
count, 0 failed and 0 skipped in all eleven** — Core 243, Containers 109, Text 240, Vector 291,
Rendering 119, Markup 259, OpenDocument 125, WordProcessing 608, Spreadsheets 446,
Presentations 520, Fidelity 545. Zero build warnings. The merge itself was clean — no conflicts,
which is unusual on this branch and worth attributing rather than assuming: the round's only
source change was three files nobody else was in.

`dotnet/probes/sheets-r22/` holds the round's TSVs, including `ops-survey-001-009.tsv` — the
first operator-level survey of *passing* documents. Read its `README.md` before re-sweeping
anything on this track.
## Words, round twenty-two: `w:pBdr` — swept whole at `9cffaa02a`

Baseline and two fixes, each swept over all 200 documents with this worktree's own CLI (the
`measuring …` line named it every time), 200 rows, no duplicate path, **0 `ref-failed`** in any
of the three.

### The baseline, and three inherited figures that do not reproduce

| | at `9cffaa02a` |
|---|---:|
| full match | **155 / 200** |
| exactly correct page count | 165 |
| total absolute page error | 95 |
| total absolute word error | 6971 (3768 we draw and it does not, 3203 we miss) |

Per batch: 001–005 10/10, 006 9/10, 007 10/10, 008 9/10, 009 10/10, 010 8/9, 011 9/10,
012 9/10, 013 6/9, 014 4/10, 015 5/10, 016 8/10, 017 6/10, 018 6/10, 019 3/10, 020 3/10,
021 0/2.

Two of those differ from what was recorded at `22ed440e0` — `008` is 9/10 against 10/10 and
`013` is 6/9 against 5/9 — and neither is a change of anyone's; they are the baseline. And the
**format split has stopped being near-equal**: `doc` 56/66 (15% fail) against `docx` 99/134
(26% fail), where earlier rounds recorded 42% and 43%. The argument for splitting agents by
symptom rather than by reader was that near-equality; it no longer holds, and the residue is
now weighted towards the OOXML side.

### The fourth check, run over the whole track for the first time

`pdf-image-diff.py` over all 155 documents that pass the word gate, on the PDF pairs the sweep
had already produced. Nothing was unmeasurable — 0 page-count refusals, 0 page-size refusals.

**155 documents, 1562 pages, 562 major, summed |ink|% 1023.92.** Per batch (documents / pages
/ major): 001 10/20/1, 002 10/30/1, 003 10/46/12, 004 10/33/7, 005 10/58/22, 006 9/43/9,
007 10/55/14, 008 9/44/8, 009 10/47/20, 010 8/92/26, 011 9/58/17, 012 9/96/25, 013 6/53/23,
014 4/60/29, 015 5/50/13, 016 8/71/32, 017 6/128/34, 018 6/153/26, 019 3/90/58, 020 3/335/185.

**015–020 hold 827 of the 1562 pages and 348 of the 562 major ones**, and none of it had ever
been looked at. One document — `batch-020/docx/FAA 2025-26 Holdover Tables.docx` — was **every
one of its 154 pages major** while passing page count and word count exactly.

### `w:pBdr`, which was not implemented at all

`pdf-ops.py diff` named it on that document in one run: on every page, two full-width strokes
present only in the reference, at y=753.25 and y=37.45 — the `Header` style's bottom rule and
the `Footer` style's top rule, each 1.5% of the page and each on its own enough to make a page
major.

Everything about the property was measured off LibreOffice's own PDFs on 26 probes before any
code was written, and all of it held to the digit afterwards:

- a side's vertical allowance is **`w:sz/8 + w:space`**, `w:space` in *points* — the one place
  in WordprocessingML that unit appears on a border;
- it **adds to** `w:spacing` and does not collapse against it, which is why it is carried as
  `PageParagraph.BorderAbove`/`BorderBelow` rather than folded into `ParagraphFormat`. Folded
  in, it would vanish on every document whose `CollapsesSpacing` is on, which is most of them;
- the rule sits at the frame edge with the space between it and the *text*;
- **left and right borders do not narrow the text** — the box grows outward, past the page
  margin — so nothing reflows horizontally and only top and bottom reach the paginator;
- `w:pBdr` is inherited **side by side** down the style chain, and a stated `w:val="none"`
  beats the layer below and takes no room;
- two consecutive paragraphs bordered alike are **one box**, and a `w:between` draws in that
  gap at `width + 2 × space`.

| | baseline | + `w:pBdr` | + the joined box |
|---|---:|---:|---:|
| full match | 155/200 | **156** | 156 |
| exactly correct page count | 165 | **166** | 166 |
| total absolute page error | 95 | **93** | 93 |
| total absolute word error | 6971 | **6898** | 6898 |
| major pages (155 in common) | 562 | **367** | 367 |
| summed \|ink\|% (155 in common) | 1023.92 | **830.13** | 830.13 |

**Major pages −35%, unaccounted ink −18.9%, and no batch fell.** 28 documents moved on ink;
the head of the list is `FAA 2025-26 Holdover Tables.docx` at **154 major pages → 10**, then
`SPA-11_mcar_part-11_v2.9.docx` 26 → 4 and `SPA-06_mcar_part-6_and_IS_v2.9.docx` 10 → 9 with
its ink halved.

**Measured reach: 28 of 200 documents whose rendering changed.** The census — 56 of 134 DOCX
declare a `w:pBdr` — is the *ceiling*, not the reach, and reporting it as the reach would have
overstated by two-fold.

### The one document that got worse is not a regression, and the check that showed it

`batch-017/docx/Sample_SQMS_Program.docx` went 2 major pages → 3. Its page 46 draws four
0.65 pt rules at the paragraph's print width, under four lines of underscores, that the
reference does not draw — and **the baseline render draws all four too**, 1.5 pt higher. The
border allowance moved them, and the page crossed the major threshold; no new ink was
introduced. Named and unfixed: it is not a paragraph border, and it is the next thing to look
at on that document.

Finding it did turn up a real defect, though, which is the third column above: two joined
paragraphs keep whatever `w:spacing` stands between them and LibreOffice runs the box's side
rules across it, where ours stopped at each paragraph's own text and left a hole at every
boundary — nineteen of them down that one page. Fixed, and **the fix is invisible to every
instrument in the gate**: the whole-track sweep after it is byte-identical to the one before
across all 200 rows, and the image diff moves not one document, because a 0.5 pt wide hole
6 pt tall is a third of a pixel at 512 px on the long edge. It rests on the operator-level
comparison against the reference's own strokes and on nothing else. Worth stating plainly:
**a correct fix with no metric to show for it is the expected case once the borders are drawn
at all.**

### Still open on this track, in the order I would take them

1. **`w:pBdr` for `.doc`, `.rtf` and `.odt`.** The model, the layout and the drawing are
   format-agnostic; only the DOCX reader is wired. `sprmPBrcTop`'s family, `\brdrt` and
   `fo:border-*` all build the same `ParagraphBorderSet`, and 66 of the track's 200 documents
   are binary `.doc`.
2. **The remaining 367 major pages**, ranked by \|ink\|% — the head is still
   `FAA 2025-26 Holdover Tables.docx` (41.83), `150_5300_13_chg12.doc` (69.07),
   `150_5300_13_chg8.doc` (55.62) and `150_5335_5a.doc` (50.29). The three `.doc` files in that
   list are all from one producer and none has been looked at.
3. **`PAGE` in a header or footer still prints the producer's cached result.** Confirmed live
   on the Holdover Tables document: the reference prints "3 of 87" on page 3 and we print
   "6 of 87" on every page. It is a feature rather than a wiring change — a header flow is laid
   out once and reused, and the number's width differs per page.
4. **`w:tblStylePr` conditional formatting is not worth a round.** Round twenty-one listed it
   first, as "the rest of the same 46 documents". Censused over every part that can hold a
   `w:tbl` and over each used style's `w:basedOn` closure: 54 words documents name a
   `w:tblStyle` at all and **7** reach a style carrying any `w:tblStylePr`, one of which states
   `w:tblLook w:val="0000"` so no band applies to it either.
5. The largest single word-gate failures left, none of them on `TODO.raster-ceiling.md`:
   `UG.CAO.00006 … User Guide` (+670 words, 30/29 pages), `xx_SETIS_PWS_template_10.19.22.docx`
   (−541, page-exact), `150_5300_13_chg10.doc` (+488), `CRIF - Spécification technique` (−426,
   33/29). The five `TE.CAO`/`UG.CAO` documents are one producer's family and split both ways
   on sign, which is the shape of a single cause.
## Slides, round twenty: the fit's spacing scale reaches a paragraph's own space

Swept whole twice at `9cffaa02a` — a baseline and the one fix — 163 documents each, both against
checksummed CLI snapshots of this worktree, the second reusing the first's reference PDFs because
nothing this round touches `soffice`. 163 rows, no path twice, 0 `ref-failed`, both times.

| | baseline `9cffaa02a` | after |
|---|---|---|
| word gate | 151 / 163 | **151 / 163** |
| signed `ink%` | 1283.60 | **1264.88** |
| unsigned `\|ink\|%` | 1603.08 | **1583.00** |
| major pages | 430 | **428** |
| census: pages neither class explains | 325 over 96 docs | **304 over 91 docs** |

**Whole round: `|ink|%` 1603.08 → 1583.00 — 20.08, or 1.25 per cent of the track's residue — with
the word gate flat and no verdict changed on any of the 163 documents.** 57 documents moved on
`|ink|%`: **36 better (−26.17), 21 worse (+6.09)**. Only four parity rows moved at all, each by one
word, which is a token boundary rather than text.

The baseline reproduced round nineteen's post-fix figures: 151/163 and 430 major pages exactly, and
1283.60 / 1603.08 against its 1283.56 / 1603.00 — 0.005 per cent, not attributed to a document. Its
per-batch split reproduces too: 001–007, 011, 013 and 015 full, 008 9/10, 009 9/10, 010 8/10,
012 8/10, 014 7/10, 016 8/10, 017 4/5. **`size-census-2.py` over the baseline is identical to round
nineteen's line for line** — 4199 pages, 3519 agreeing, 25/133/62/135 and 325 over 96 documents — a
third independent confirmation of the same tree.

### The brief's named next step was already in the tree

The dispatch brief's headline lead was *"the shape's em size is now quantised to the draw layer's
1/100 mm grid, but the shape's text area is not — quantise the text rectangle the same way"*. That
is round nineteen's own fix and it is merged: `SlideTextLayout.Place` opens with
`OnGrid(textRectangle).Deflate(OnGrid(body.Insets))` at `1217f3a4c`, which `9cffaa02a` descends
from. Checked before anything was measured.

### What was left, and the guard whose reading was inverted

Round nineteen's elimination — em on the grid, box on the grid, search compared statement by
statement — leaves the *measured height*. One term of that height is a paragraph's own space, and
the tree said in as many words that the fit's spacing scale does not reach it:

> The reference does put a paragraph's own space through `scaleYSpacingValue` … but that helper
> returns its argument unchanged unless `maStatus.DoStretch()` is set.

The citation is right and the inference from it is backwards. `SdrTextObj::ImpSetupDrawOutlinerForPaint`
turns `EEControlBits::STRETCHING` **on** whenever `IsFitToSize() || IsAutoFit()` and only then calls
`setupAutoFitText` (`svx/source/svdraw/svdotext.cxx`:1177-1183), so an autofitted shape always
satisfies the guard. The same flag is what lets the *font* scale apply at all
(`impedit3.cxx`:3005-3012 is inside `if (maStatus.DoStretch())`), so a reading on which the guard is
clear would leave the whole search inert.

### The probe, because the corpus cannot separate the two readings

`research/probes/slides-r20/make-spacing-probe.py` builds a deck of autofit boxes holding one text
at a sweep of box heights: four paragraphs of two lines each, the second line forced by an `a:br` so
the line count cannot move with the font size, all four insets zero, a 12 pt `a:spcBef` on every
paragraph, and a throwaway shape first on each slide so nothing under test measures LibreOffice's
shared-outliner state leak.

Twelve boxes at a 20 pt base: **the reference disagreed with us on three, and every one of the three
is a box where we kept a larger font at nine- or eight-tenths spacing and it took a smaller font at
full spacing** — 15.00 against 11.99, and 18.99 against 17.01 twice. That is the one-step class the
last four rounds have been chasing, reproduced in twelve boxes of authored markup. Scaling the
paragraph space turns all three round, and the other nine are at full spacing where the change is a
no-op by construction. A second probe at a 10 pt base is 11 of 12 before and 11 of 12 after, trading
one boundary box for another.

The pinned page of rounds seventeen to nineteen closes with it:
`2015-Civil-Rights-Website-training.ppt` page 21 was **ours 20.01 against ref 18.99** in every
previous round and is now 18.99 at the reference's own baseline, 347.81. The deck goes
`|ink|%` 33.34 → 31.05 and drops from 22 census pages to 17.

### The predecessor's counter-example reproduces and its conclusion does not

The note that recorded this as "measured wrong" named
`slides/batch-002/ppt/gfopportunitiesforlinkagespres_2010_en.ppt` slide 6 as shrinking a step below
the reference. **It does** — its 27.01 pt block becomes 25.99. What the note left out is that the
*same page's* 22.99 pt block was being drawn at nine-tenths line spacing where the reference draws it
at full: pitch 24.86 pt against the reference's 27.58, and 27.60 after the change. Two errors were
cancelling on that page, and the document's `|ink|%` is 9.98 before and after.

So the residue on that page is now a pure height disagreement at full spacing on both sides, which
is a cleaner thing to chase than what was there before. `ITE106-Chapter 4.ppt` moves the same way
(36.14 → 37.67) and is the deck round eighteen recorded as shrinking *further* than the reference —
the opposite sign of the same remaining term.

### The regression guards, and the tests

Per project on the final tree, each run redirected to its own file: Core 243, Text 240,
Containers 109, Vector 291, Rendering 115, Markup 259, OpenDocument 125, WordProcessing 608,
Spreadsheets 446, Presentations **523**, Fidelity **546**. **Zero failed and zero skipped
throughout.** The two that moved are this round's own tests.

**Nothing outside `Paperless.Presentations/Layout/SlideTextLayout.cs` was touched** — one new
private method and the two properties that use it — so the words and sheets tracks cannot be
affected and are not owed a sweep. That is checkable rather than asserted: the file is reached only
through `SlideTextLayout.Place`/`Measure`, which no other family calls.

Two assertions, **each verified by putting the defect back and watching it fail**:

- `TheFitsSpacingScaleReachesAParagraphsOwnSpace` (fidelity) — the three discriminating boxes of
  `tests/corpus/features/slide-autofit-paragraph-space.pptx`, our sizes asserted as literals first
  and the reference's compared against those. With the defect back it fails on the 150 pt box.
- `TheFitsSpacingScaleReachesAParagraphsOwnSpace` (unit, three cases) — one box per spacing scale
  the search can settle on, so the σ = 1 case is a control that passes either way and the σ = 0.9
  and σ = 0.8 cases both fail with the defect back. That is what makes it able to fail; a box at
  full spacing alone could not.

### What the next round should take, in order

1. **The remaining height term, now visible from both sides.** The one-step class is 304 census
   pages rather than 325, and it is no longer confounded with the spacing scale: on
   `gfopportunities` page 6 and on `ITE106-Chapter 4.ppt` both renderers now use full spacing and
   still disagree by one step, so what is left is the height a block measures at a given size.
   Two leads worth separating first, neither chased here:
   - The probe shows a **1–3 unit disagreement in the line height of a box the search leaves
     alone**: at 20 pt the reference draws a pitch of 848 hundredths of a millimetre where
     `round(706 × 1.2)` is 847, and at 10 pt it draws **421** where `round(353 × 1.2)` is 424.
     Neither is a spacing scale — 421/424 is 0.993 — and neither is explained. Nine other sizes in
     the same probe reproduce `round(em × 1.2)` exactly, so this is a small residue at particular
     sizes rather than a wrong rule.
   - `PptSlideLayout.cs`:1240-1245 scales a grouped shape's rectangle by
     `Math.Round(width.Emu * placement.A)` before `Place` sees it, and that group scale has still
     not been measured against LibreOffice's. Round nineteen flagged it and did not do it.
2. **`Demick_JetBlue.pptx`'s charts**, 26.70 `|ink|%` over 6 major pages of 10 — unchanged from
   round nineteen, still the densest unexplained document, still in `Paperless.Core/Charts` and so
   still owing the words and sheets sweeps.
3. **The `.ppt` twin of the picture recolour** (`DFF_Prop_pictureBrightness`/`pictureContrast`),
   unchanged from round nineteen and **still unmeasured for reach**: I did not count how many
   corpus `.ppt` carry either property either.

## Three tracks merged and verified together at `73934b1b5`

Words round twenty-two, slides round twenty and sheets rounds twenty-two and twenty-three all
landed on the branch in sequence. The figures in each round's own section are the agent's,
measured in its worktree; these are the branch's.

Every project green at its expected count, **0 failed and 0 skipped in all eleven**, zero build
warnings, verified after each merge rather than once at the end:

| | before the round | now |
|---|---:|---:|
| Core | 243 | 243 |
| Containers | 109 | 109 |
| Text | 240 | 240 |
| Vector | 291 | 291 |
| Rendering | 115 | **119** |
| Markup | 259 | 259 |
| OpenDocument | 125 | 125 |
| WordProcessing | 608 | **619** |
| Spreadsheets | 446 | **470** |
| Presentations | 520 | **523** |
| Fidelity | 545 | **546** |

Two of the three merges conflicted, both only in this file and both because two rounds appended
their sections to the end of it. Resolved by keeping both sides — which is the standing rule on
this branch and worth restating, because the tempting resolution is to pick the newer section
and it silently drops a round's record.

### What the round says about the gate

Four fixes landed across the three tracks. **Every one of them is real, measured by rendering,
and moved the scoreboard by zero or one document** — see the skill's "a real fix that moves no
verdict". `batch-check.sh` decides on page count, word count and *unembedded* fonts; a face
name, a sub-pixel stroke, a 0.4% font size and a spacing scale reach none of those. Three of the
four were found only by instruments below the gate, and two of those only by running an operator
diff over documents that **already passed**, which nothing had done before this round.

The track verdicts are therefore unchanged and that is not a stalled round: words 156/200,
slides 151/163, sheets 144/171.
## Slides, round twenty-one: the autofit line height is on the device's grid — swept whole at `6bf169518`

Baseline measured before anything moved, and **for the first time on this track a predecessor's
headline reproduced without correction**: 151/163 on the word gate, `|ink|%` **1583.00**, **428**
major pages, all three exactly the briefed figures. Our renderings came from a checksummed
snapshot of the worktree's own CLI; the reference PDFs were the kept set from round twenty,
verified first against a fresh `soffice` 24.2.7.2 on two documents (page counts and `pdftotext`
md5 identical both times).

### The brief's first lead was real, and far larger than the note it came in

The note said a box the search leaves alone "draws a line pitch of 848 units at 20 pt where
`round(706 × 1.2)` is 847, and 421 at 10 pt where `round(353 × 1.2)` is 424", called it "a small
residue at particular sizes", and could not explain it. **The measurement reproduces exactly and
the sentence beside it is wrong in the usual direction** — it is not two sizes and it is not a
residue.

`research/probes/slides-r21/make-pitch-probe.py` puts the same three paragraphs twice on each
slide — once in an `a:noAutofit` box and once in an `a:normAutofit` box far too tall to shrink —
behind a sacrificial warm-up shape. One slide per size, so the only difference between the two
columns is the autofit flag. Over 53 sizes from 6 to 58 pt:

| | agrees with `fround(em × 1.2)` |
|---|---|
| plain box | **53 of 53** |
| autofitted box | **19 of 53** |

Never more than three hundredths of a millimetre away, and wrong in **both** directions — which
is what said rounding rather than a missing multiplier.

### The mechanism, and why only the unscaled case shows it

`ImpEditEngine::SeekCursor` takes a different branch whenever `maStatus.DoStretch()`, which
`SdrTextObj::ImpSetupDrawOutlinerForPaint` sets for `IsFitToSize() || IsAutoFit()` and for
nothing else. That branch pushes the font at the device, reads the size back out of the device's
own metric, and puts *that* on the font — `rFont.SetPhysFont(*pDev); Size
aRealSz(aMetric.GetFontSize()); … rFont.SetFontSize(aRealSz)`
(`editeng/source/editeng/impedit3.cxx`:2985-3062, 24.2.7). During formatting the device is the
reference device, so the 1.2 rule is applied to the item height rounded to whole device pixels
and back.

Fitting that round trip over 30 to 4000 dpi reproduces all 53 rows at **600 dpi and at no other
resolution**. Eight further fractional sizes — four with the two columns disagreeing — come back
8 of 8; Carlito reproduces Liberation Sans row for row on eleven sizes, which is what
"font-independent" line spacing should do.

When `fFontY != 1.0` the same branch immediately puts the height through `roundToNearestPt`
twice, rounding to a whole point and discarding the device grid. So the existing
`Scaling.Scaled` is already faithful for every shrunken body, and this reaches only the case
where the search settles on scale 1 — which is the reference's own condition, not a
simplification of it.

**Do not read an em off a reference PDF's `/Tf`.** There is a *second*, unrelated round trip at
paint time through the PDF export device at 720 dpi, and it applies to plain and autofitted
shapes alike: a 13.33 pt run is held as 471 units, measured at 470 and drawn as **473**. Three
numbers for one size is why this term looked like noise from the content stream alone.

### And a second quantisation, found by the same probe

`SlideTextLayout.Spacing`'s own note said it was "a pass-through for single spacing rather than a
call to `LineSpacingRule.Apply`", and the code called `Apply` — whose first line is
`naturalHeight.Twips`, so it quantised before its `_ => natural` arm handed the value back. Only
a line height that is a whole multiple of 3.6 pt survived it. An 8 pt line was drawn at 338.67
units against the reference's 338, a 10 pt line at 423.33 against 424, a 28 pt line at 1185.2
against 1186.

Ours against the reference on the probe, both columns, 53 sizes each:

| | before | after |
|---|---|---|
| plain | 39 of 53 | **53 of 53** |
| autofitted | 15 of 53 | **53 of 53** |

The predecessor's own two decks move the same way, scored on the chosen size *and* the drawn
pitch together: `spacing-probe` 10/12 → **12/12**, `spacing-small` 0/12 → **11/12**.

### The corpus does not care, and that is the finding

Whole track swept before and after, 163 documents each:

| | before | after |
|---|---:|---:|
| word gate | 151/163 | 151/163 |
| per-batch match counts | — | **identical, all seventeen** |
| `\|ink\|%` | 1583.00 | **1582.40** |
| major pages | 428 | **430** |

**No document changed verdict, in either direction.** 52 documents improved by 5.31 between
them, 35 worsened by 4.71, 76 did not move: a net −0.60 on 1583, which is 0.04 per cent.

That is a correct fix with an unmeasurable corpus effect, and it is worth saying plainly rather
than dressing up. Two readings are consistent with it and this round cannot separate them: the
term is genuinely small — one to three hundredths of a millimetre on a line — so it only decides
a page when the search is at a near-tie; or its wins and losses are both real and roughly
balanced because a near-tie is equally likely to fall either way. The one visible regression
supports the second: `berlin.ppt` is +1.37, and **all of it is page 10** (0.18 → 1.62) with the
other twenty-eight pages moving by at most 0.03. One block, one step, one page.

The three decks the brief named all improved — `2015-Civil-Rights-Website-training.ppt`
31.05 → 30.42, `ITE106-Chapter 4.ppt` 37.67 → 37.44, `gfopportunitiesforlinkagespres_2010_en.ppt`
9.98 → 10.08 is the exception and is +0.10.

### Tests

Two assertions, each verified by reintroducing its defect and watching it fail:

- `SlideAutofitTests.AnAutofittedBodyMeasuresItsLinesOnTheDevicesGrid` (unit, five cases) —
  plain and autofitted pitch per size, every expectation read out of the reference PDF.
  **12 pt is a control that passes under either reading**, since 423 units come back 423 through
  the 600 dpi grid; the other four bite in both directions. Without the device grid four of five
  fail and the control passes; without the twip fix the three plain expectations fail.
- `SlideAutofitDeviceGridComparisonTests` (fidelity) — the same ten pitches on the authored
  `slide-autofit-device-grid.pptx`, ours asserted as literals first and LibreOffice compared
  against those, at a hundredth of a point.

Per project on the final tree, each run redirected to its own file: Core 243, Text 240,
Containers 109, Vector 291, Rendering 119, Markup 259, OpenDocument 125, WordProcessing 619,
Spreadsheets 446, Fidelity **547** (546 plus this round's one), Presentations **528** (523 plus
this round's five). Zero failed, zero skipped. Only `Paperless.Presentations/Layout` was touched, and only through
`SlideTextLayout.Place`/`Measure`, so the words and sheets tracks are not owed a sweep.

### Open items this round did close, and one it did not

**Closed on reach: the `.ppt` picture recolour.** Carried as "unmeasured" for three rounds.
Counted by walking every Escher record in each file's `PowerPoint Document` and `Pictures`
streams: of **51** corpus `.ppt`, **4** carry `DFF_Prop_pictureContrast` (264) or
`DFF_Prop_pictureBrightness` (265), and one of those four states only the defaults — contrast
65536, brightness 0 — on all twenty of its shapes. So the feature is **three documents and four
shapes**, and does not deserve a round.

**Not re-run: the size census.** The machine was carrying two other agents' sweeps at a load
average above 20 and the census was starved to a standstill after forty minutes; it was killed
so the whole-track sweep could finish. The "pages neither class explains" figure is therefore
still round twenty's 304 over 91 documents, and is *not* a measurement of this commit.

**Still unmeasured, unchanged from the brief:** `PptSlideLayout.cs`:1244's group scale against
LibreOffice's, and `Demick_JetBlue.pptx`'s charts in the shared `Paperless.Core/Charts`.

## Second merge of the round, verified at `4448c2a3f`

Sheets round twenty-four (`.xls` chart sources) and slides round twenty-one (the two autofit
quantisations) landed on top of the previous merge. Verified on the branch, every project green,
**0 failed and 0 skipped in all eleven**, zero build warnings:

Core 243, Containers 109, Text 240, Vector 291, Rendering 119, Markup 259, OpenDocument 125,
WordProcessing 619, Spreadsheets **483**, Presentations **528**, Fidelity **547**.

Track verdicts are unchanged — words 156/200, slides 151/163, sheets 144/171 — and by now that
is the expected outcome rather than a surprise. Six fixes have landed across the three tracks in
this round and **not one has moved a verdict**; see the skill's "a real fix that moves no
verdict". Every one was measured by rendering the whole track before and after.

### Two things this merge changed about how the next round should measure

- **Our PDF writer is not byte-reproducible** across two runs of the same binary on the same
  input, with timestamps already masked. Byte-level comparison of two renderings is how the last
  three rounds measured reach, so there is an unknown floor under all of those numbers until
  this is fixed. It is the sheets track's first job next round.
- **An aggregate `|ink|%` cannot be smaller than the signed `ink%` beside it.** Round twenty's
  `ink.tsv` carried a derived column that violated it — off by exactly one per major page per
  document, a count summed into a percentage. Nothing depended on it. Check the invariant on
  inherited aggregates, not only produced ones.

## Words, round twenty-three: the DOC readers — swept whole at `5913f489e`

Baseline and two fixes, each swept over all 200 documents with this worktree's own CLI (the
`measuring …` line named it every run), 200 rows, no duplicate path, **0 `ref-failed`**.

### The baseline reproduced the round before it exactly

| | at `5913f489e` |
|---|---:|
| full match | **156 / 200** |
| exactly correct page count | 166 |
| total absolute page error | 93 |
| image: documents / pages / major / summed \|ink\|% | 156 / 1621 / 425 / 868.15 |

`doc` 56/66 (15% fail), `docx` 100/134 (25% fail) — the format split the round before recorded,
reproduced.

**The round-twenty-two headline is right and the way it is usually quoted is not.** Its
"562 major pages down to 367" is the comparison over the **155 documents measurable at both
ends**, not over the whole set. The same fix made a 59-page document match for the first time,
so it entered the image sweep carrying 58 major pages and 38.02 ink — and the whole-set after
figure is 425 major and 868.15, which is what this round's baseline reproduces to the digit.
Both numbers are correct and they answer different questions. When a fix changes *which*
documents the image sweep can see, only the common-set comparison is apples to apples; say
which one a figure is.

### What the brief expected, and what the track actually holds

The round was briefed to carry `w:pBdr` into "the other three readers" — `.doc`, `.rtf` and
`.odt` — on the grounds that the layout and drawing halves are format-agnostic and already
built. They are. But the words track holds **66 `.doc`, 134 `.docx`, and no `.rtf` and no
`.odt` at all**, so two of the three readers have no document here and nothing built into them
could have been measured by rendering it. Only the WW8 reader was worth the round.

### `sprmPBrcTop` and its four neighbours

Reads as `w:pBdr` does, because the two formats state the same two quantities in the same two
units: a `BRC`'s `dptLineWidth` is eighths of a point exactly as `w:sz` is, and its `dptSpace`
is whole points exactly as `w:space` is (`WW8_BRCVer9::DetermineBorderProperties` multiplies it
by twenty). Both forms of the sprm are read, the WW9 form winning over the WW8 one regardless of
the order a producer wrote them in; the two revision-bar sprms are excluded, as
`SwWW8ImplReader::Read_Border` excludes them. A sprm carrying a nil or zero-typed `BRC` is a
border *removed*, not a border unstated — `##826##` in `SetBorder`.

The joining pass moved out of `DocxLayoutSource` into `Layout/ParagraphBorders.cs`, where both
readers run one copy of it.

| | before | after |
|---|---:|---:|
| full match | 156 | 156 |
| abs page error | 93 | 93 |
| major pages (156 docs) | 425 | 423 |
| summed \|ink\|% | 868.15 | 861.81 |

**Not one of the 200 rows changed.** Reach, measured by rendering: 8 documents moved in the
image sweep and 9 of the 66 `.doc` gained or lost a fill record — the ninth fails the gate, so
the image sweep cannot see it.

That is much smaller than the same property was worth on DOCX, and the three reasons are worth
keeping:

- A raw scan for the ten sprm ids hits **54 of the 66** `.doc`. Nine draw a rule that was not
  drawn before. The census overstates by six times, in the usual direction — Word writes
  `sprmPBrcTop` with a nil `BRC` constantly, and a stated *no border* is most of what those 54
  files carry.
- A paragraph rule is half a point thick, which is a third of a pixel at 512 px on the long
  edge. **`pdf-image-diff.py` is close to blind to this class**; the operator diff is what sees
  it. Six of the eight documents the image sweep did move, moved by under a point of ink.
- The one document that got worse is the round-twenty-two lesson again.
  `150_5300_13_chg12.doc` page 27 gains seven right-edge rules at x 565.60–566.10 and the
  reference draws exactly seven at x 565.80 — one for one, matching segment lengths, 55.6 pt
  higher up the page. The rules are right; the page was already displaced, and drawing them made
  a pre-existing error visible to a metric that could not see it before.

Positively, on `150_5300_13_chg10.doc` page 1 the two new rules land at (72.00, 625.85) and
(72.00, 595.42) where the reference strokes are at (72.00, 625.80) and (72.00, 595.45), with
the same x extents to 0.05 pt.

### A DOC section states no header distance because it means half an inch

Found by ranking the remaining major pages by ink. The top three are one producer's — the brief
called that the best-value unexamined thing on the track, and it was, but the cause is not
paragraph borders.

On `150_5335_5a.doc` page 9 our header sat at y 780.80 against the reference's 744.80, and our
footer at 2.60 against 38.60. Thirty-six points, both ways, on every page — while the body
agreed to within 0.15 pt, which is what says furniture rather than reflow.

WW8's `SEP` is a structure with defaults, not a list of statements: Word writes a sprm only
where a section departs from them. `Ww8SectionTable` read a missing `sprmSDyaHdrTop` as zero,
so every DOC that accepts Word's own default had its header pinned to the paper edge.
LibreOffice states the default twice — `WW8_SEP`'s constructor (`ww8scan.cxx`,
`dyaHdrTop(720), dyaHdrBottom(720)`) and the fallback to `ReadUSprm` (`ww8par6.cxx`:1183).

It reaches the body too: `PageGeometry.HeaderHeight` is the gap between the distance and the top
margin, so a zero distance also made the header's band 36 pt too tall on every such document.

| | before | after |
|---|---:|---:|
| full match | 156 | **155** |
| abs page error | 93 | **98** |
| major pages (155 common docs) | 408 | **360** |
| summed \|ink\|% | 806.94 | **798.02** |

`150_5335_5a.doc` goes 47 major pages to 15 without changing its 63-page count;
`96-3-clement-data.doc` goes 6 to 0; `316r_a_e.doc` goes 14 to 2.

**It costs a match, and it is still right.** `150_5300_13_chg8.doc` goes 18/18 to 21/18 and
`150_5300_13_chg10.doc` 77/76 to 79/76 — both the same producer, both gaining pages because the
header band is now the right size and the body no longer has 36 pt of head-room it was never
entitled to. chg8's 18/18 was accidental: 15 of its 18 pages were already major, and its page 1
is missing the reference's whole departmental title block. Nothing else in the track moved.

Report both numbers rather than the flattering one. The binary gate went down by one and the
continuous page-error figure went up by five, while the metric the fix is actually about went
down by 48 major pages — the reverse shape of the "fixes that cancel" case in the skill, and the
same rule applies: do not revert a change that is right on its own evidence.

### Round total, base `5913f489e` to `HEAD`

| | base | final |
|---|---:|---:|
| full match | 156 | 155 |
| abs page error | 93 | 98 |
| major pages (155 common docs) | 410 | **360** |
| summed \|ink\|% | 812.53 | **798.02** |

Ten documents moved across the round, nine of them better.

### Verified by putting the defects back

Eight for the borders, four for the section defaults, each applied to the source, the library
rebuilt, the tests run, the source restored.

| defect | caught by |
|---|---|
| `dptSpace` read as twips | 5 border tests |
| `fShadow`/`fFrame` bits left in `dptSpace` | `TheShadowAndFrameFlags…` |
| a nil or zero-typed `BRC` read as silence | `ABorderRemovedIsStated…` |
| the id range run one too far, taking the bar | `ARevisionBarIsNot…(0xC653)` |
| the side offset one out | 5 × `EachBorderSprmNamesItsSide…` |
| the joining loop never entered | one DOC test and two DOCX tests |
| the join stops comparing the left rule | `TwoParagraphsBorderedDifferently…` |
| either emptiness guard removed alone | **nothing** |
| both header distances default to nothing | all 3 section tests |
| the default treated as a floor | `AStatedDistanceBeatsTheDefault` |
| only the header defaults, not the footer | 2 section tests |
| the band measured from the page edge | `TheHeaderBandIsWhatTheTopMargin…` |

`AParagraphWithNoBorderSprmsHasNoSet` is a **control rather than a detector**, and the label is
kept deliberately. `ToParagraphBorders` guards emptiness twice, so removing either guard alone
changes nothing observable; removing both together does break the test, which is what proves it
is not vacuous. No single plausible defect makes it fail.

### What is open, in the order this round would take them

1. **The remaining 360 major pages.** The top of the list is still one producer —
   `150_5300_13_chg12.doc` (70.29), `150_5300_13_chg8.doc`, `150_5335_5a.doc` — and the header
   fix has now taken the third of them from 47 major pages to 15. The next thing on chg8 is
   visible in one dump: **page 1 is missing the reference's entire departmental title block**
   ("U.S. Department of Transportation / Federal Aviation Administration", drawn in DejaVu Sans
   at y 701.95–663.55). We draw nothing there and start at "1. PURPOSE". That is a dropped
   frame or table, it is the reason chg8's page count was only accidentally right, and it is
   worth a round on its own.
2. **`PAGE` in a header or footer prints the producer's cached number.** Still true, and this
   round found a second half of it: on `150_5335_5a.doc` page 9 our footer says **13** where the
   reference says **3**, so the defect is not only the cached width but the section's page
   numbering restart as well.
3. **`w:pBdr` for `.rtf` and `.odt`.** Both readers still build nothing into the model. Neither
   has a single document on this track, so the work cannot be justified or measured here — it
   needs an authored probe compared against LibreOffice, not a corpus sweep. Say so rather than
   letting it read as unfinished corpus work.
4. **A `w:val="double"` border reserves one width, not three.** `ConvertBorderWidthFromWord`
   triples a double rule's width and doubles a thick one, and neither reader applies it. The DOC
   path already applies `DetermineBorderProperties`'s triple/wave adjustment because the table
   borders needed it, so the two readers now disagree slightly on the same markup. Not measured;
   no document was found on this track where it decides anything.

## Words round twenty-three merged at `be5df60ac`, and its trade re-checked independently

Ten projects green at expected counts, WordProcessing **619 → 647**, zero build warnings.

**This round's headline fix lowers the parity gate on purpose and it was kept.** A DOC section
stating no header distance means half an inch, not zero — `Ww8SectionTable` read a missing
`sprmSDyaHdrTop`/`Bottom` as 0 where Word's default is 720 twips, so every DOC accepting the
default had its header pinned to the paper edge and its band 36 pt too tall.

| | before | after |
|---|---:|---:|
| major pages (155 common documents) | 408 | **360** |
| summed `\|ink\|%` | 806.94 | **798.02** |
| parity gate | 156 | **155** |
| absolute page error | 93 | **98** |

**Re-measured here at the merge rather than taken from the round's report**, because a fix that
lowers the gate is the one case where the parent should not take a figure on trust. Every number
reproduced: `150_5335_5a.doc` 47 → **15** major pages, `316r_a_e.doc` 14 → **2**,
`96-3-clement-data.doc` 6 → **0**, and the two regressions at 21/18 and 79/76 pages.

The lost match is the skill's "fixes that cancel" running backwards. `150_5300_13_chg8.doc` was
18/18 **by accident**: 15 of its 18 pages were already major and its page 1 is missing the
reference's entire departmental title block. Correcting the header exposed the second error
rather than causing it. That title block is the next round's first job, and it is the same
producer as `chg10`, `chg12` and `150_5335_5a` — four documents at the head of the track's
remaining ink, plausibly one cause.

### The brief was refuted on its central premise

It named "`w:pBdr` for `.doc`/`.rtf`/`.odt`" as the largest ready lever. The track holds **66
`.doc`, 134 `.docx`, and zero `.rtf` and zero `.odt`** — two of the three readers have no
document on this corpus, so nothing built into them is measurable by rendering here. The `.doc`
half was built and moved 9 documents and 0.73% of the ink.

**Check that the corpus contains the thing before planning a round around it.** A census over
the sprm ids says 54 of 66 `.doc` carry them and 9 draw a new rule — overstating by six times,
because Word writes `sprmPBrcTop` with a nil `BRC` constantly.
## Slides, round twenty-two: a chart's text is the theme's face, and the corpus metric gets worse

Swept whole twice at `4448c2a3f` — a baseline and the one change — 163 documents each, both
against checksummed CLI snapshots of this worktree, both reusing round twenty's kept reference
PDFs. 163 rows, no path twice, **0 `ref-failed` and 0 `ours-failed`**, both times. The kept
references were verified first against a fresh `soffice` 24.2.7.2 on two documents: page counts
and `pdftotext` md5 identical both times.

| | baseline `4448c2a3f` | after |
|---|---:|---:|
| word gate | 151 / 163 | **151 / 163** |
| `\|ink\|%` | 1582.40 | **1584.92** |
| major pages | 430 | **430** |
| census: pages neither class explains | 303 over 91 docs | **304 over 91 docs** |
| documents whose `\|ink\|%` moved | — | **9**, all of them chart decks |

**The baseline reproduced the brief to the digit for the second round running** — 151/163,
`|ink|%` 1582.40 and 430 major pages are round twenty-one's post-fix figures exactly.

### The headline is that the fix is right and the number went the wrong way

`|ink|%` **1582.40 → 1584.92**, a regression of 2.52. Nine documents moved: **five better
(−1.37) and four worse (+3.89)**, and `Demick_JetBlue.pptx` alone is +2.40 of that. No document
changed verdict in either direction and the per-batch counts are identical across all seventeen
batches.

The change is kept anyway, and the reason is the skill's own rule about fixes that cancel rather
than a preference. The mechanism is settled by an authored probe and by the reference's own
embedded fonts; what the corpus now shows is a *second* error that the first was partly hiding.
Reverting would restore the number and the two errors.

### `ChartLabel` carried no font family, so every chart drew its text in one hardcoded face

`SlideChart` set every chart label in Liberation Sans, and had done for four rounds. The evidence
for the constant was `pdffonts` on LibreOffice's own PDF of `chart-bar-deck.pptx` reporting
Liberation Sans — and **that deck's chart states `<a:latin typeface="Arial"/>` eleven times**,
which fontconfig substitutes with Liberation Sans. The measurement was right and what it was
taken as evidence for was not, which is this project's most reliable regularity.

Two corpus decks separate the readings, and neither could have been found by looking at sheets:

- `Demick_JetBlue.pptx`'s theme minor Latin face is **Constantia**, which fontconfig substitutes
  with **DejaVu Serif** — and the reference draws its chart text in DejaVu Serif at 9.89 pt.
- `bitesize-writing-a-report.pptx`'s is **Calibri** → **Carlito**, and the reference draws its
  chart text in Carlito.

Neither is Liberation Sans and the first is not even a sans.

### The probe, because two corpus documents are still only two documents

`research/probes/slides-r22/make-chart-face-probe.py` builds `chart-bar-deck.pptx` three times
with one thing moved: the theme's minor Latin face set to **Liberation Mono** in all three, and
the chart part stating no `a:latin` at all, `+mn-lt`, or a literal `Liberation Serif`.
LibreOffice embeds **LiberationMono, LiberationMono, LiberationSerif**.

That is the pattern only *"the theme's minor face, with a stated `a:latin` overriding it"*
predicts. A fixed face gives three identical answers; reading only a stated face leaves the first
two on a fallback. It also matches what LibreOffice's own chart import does: all three of its
automatic text entries name `XML_minor`
(`oox/source/drawingml/chart/objectformatter.cxx`:415-434).

### What changed, and it is in `Paperless.Core`

**Flagged loudly, because it is the shared library.** `ChartPlot.TextFamily`,
`ChartLabel.Family`, `IChartTextMeasurer.Measure` taking a family, and a `ChartText` value that
binds one family to a measurer once at `ChartLayout.Place` so that the face a label is measured
in and the face it is drawn in cannot come apart. `DrawingChartPlot.FamilyOf` applies the rule
the **words track already ships** in `DocxPictures.LabelFamily`: the first literal `a:latin`
anywhere in the part, then the theme's minor Latin face.

**Only the slides consumer acts on it.** `SheetChart` and `FrameChart` take the argument and
document in the code that they deliberately ignore it — wiring either changes a track this round
does not sweep. That is checkable rather than asserted: their whole diff is a signature and a
comment, and neither reads `ChartLabel.Family`.

### Reach, measured by rendering rather than censused

The 163-document sweep moved **nine** documents and **every one of them is one of the fifteen
pptx decks carrying a `ppt/charts/chartN.xml`**. Nothing else moved by 0.01. The other six chart
decks did not move either: three state a literal `Arial`, which already resolved to Liberation
Sans, and the rest state no family and have no theme minor to fall back to. `.ppt` draws no
charts at all and the ODP reader still supplies no family, so both are unaffected by
construction.

| | `\|ink\|%` before | after |
|---|---:|---:|
| `Demick_JetBlue.pptx` | 26.49 | **28.89** |
| `1_Country-Updates_DRC_English.pptx` | 1.15 | **2.31** |
| `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 16.88 | 17.07 |
| `flying-by-numbers-presentation.pptx` | 12.78 | 12.92 |
| `Intersil_Italy_CAN_Bus_Transceiver…pptx` | 15.17 | 15.12 |
| `bitesize-writing-a-report.pptx` | 3.05 | 2.94 |
| `N2_E_Maestroni_Swarm_COP.pptx` | 3.71 | 3.48 |
| `171128IPAP.pptx` | 17.27 | **16.87** |
| `southern-classic-kennesaw…pptx` | 21.93 | **21.35** |

**`Demick_JetBlue.pptx` is reached, and this is what reaching it looks like.** The brief named it
as the densest unexplained document. Its embedded font set goes from
`{LiberationSans, Carlito, OpenSymbol}` to `{DejaVuSerif, Carlito, OpenSymbol}`, against the
reference's `{DejaVuSerif, Carlito, OpenSymbol, DejaVuSerif-Bold}` — the face is now the
reference's own — and its `|ink|%` gets worse by 2.40, all of it on page 4 (5.26 → 8.73) while
page 5 improves (4.72 → 3.21). The operator diff on page 4 goes from 36 one-sided records to 33.

### The second error, which the probe localises and this round does not fix

On the authored probe the value axis' labels land **0.36 pt** from the reference with this change
and **0.96 pt** without it — better — while the legend lands **2.49 pt** out with it and
**1.39 pt** out without. The composition reserves the legend's width wrongly, and measuring the
legend in a face too narrow was cancelling part of it. That is the next thing to chase on charts,
and it is now visible from both sides because the face is no longer confounding it.

### The census was run inside the sweep, which is the sequencing the last round asked for

Round twenty-one lost its census to running it *after* the whole-track sweep: it was starved
under load, killed so the sweep could finish, and then could not be resumed because the sweep's
PDFs had been freed. `size-census-doc.py` is `size-census-2.py`'s per-document half, importing
its `classify()` rather than copying it, and `sweep-track.sh` calls it beside the image diff.
It finished with the sweep.

**303 pages neither class explains, over 91 documents, measured at `4448c2a3f`** — against round
twenty's 304 over 91 for an older tree. The other classes: 4199 pages compared, 3541 agreeing, 24
grid, 138 granularity, 61 ceiling, 132 metafile. After the change: 304 over 91, one page having
moved from `granularity` to `REAL`.

### The sweep's signed `ink%` was inflated by one per major page, and still is upstream

The brief carries this as round twenty's defect and describes it exactly — *"a count summed into
a percentage"*, *"differing by exactly one per major page per document"*. **It is still in
`slides-r21/sweep-track.sh`**, and it was in the baseline measured this morning.

The awk matched page rows with `/^[0-9]/`, which also matches the line `pdf-image-diff.py` ends
with — `10 pages, 3 with major differences` — whose third field is the major-page count. The
unsigned column is untouched, because its fourth field on that line is `with`, which adds
nothing. Measured here: raw 1692.50 against 430 major pages gives a true 1262.50, and **74 of the
146 documents swept at the time violated the `|ink|% ≥ |ink%|` invariant individually** while the
track totals still satisfied it — which is why an aggregate-only check does not catch it.

**One correction to the brief.** It states the third column *"totalled 1264.88 where the same sum
recomputes to 1692.88"*, which names the wrong number as the correct one: 1692.88 would violate
the invariant against that round's `|ink|%` of 1583.00, and 1692.88 − 1264.88 is 428, that round's
major-page count exactly. The reported 1264.88 was right and the file was wrong.

### Refuted: the autofit device grid does not reach advance widths

The brief's third open item — `SeekCursor` sets the *whole* font from the device metric, so
advance widths at format time are taken at the realised size too, and only line height and ascent
were changed last round.

Two measurements, and the effect is not there.

- **Drawn widths.** On `slide-autofit-device-grid.pptx` every word in the autofitted column is
  the same width to four decimal places as the same word in the plain column, at all five sizes —
  *including* the four where the line pitch differs. Expected, since at paint time both go through
  the PDF export device; it means only a line boundary can answer the question.
- **Line breaking.** `research/probes/slides-r22/make-break-boundary-probe.py` steps a box width
  across a wrap boundary with the same phrase in a plain box and in an autofitted box too tall to
  shrink. At 20 pt, where the branch is demonstrably active (pitch 847 plain against 848 fitted),
  **both wrap between 349.65 and 349.70 pt** — the same 0.05 pt step. The predicted shift is
  0.206% of 350 pt, or 0.72 pt: fourteen steps of this probe, and absent.

No code change. The item is closed rather than carried.

### Tests

Two new files, and **every assertion was verified by putting its defect back**:

- `SlideChartFaceComparisonTests` (fidelity, three cases) on two authored decks,
  `chart-face-theme-minor.pptx` and `chart-face-stated.pptx`. With the consumer ignoring the
  family again — the four-round defect itself — **all three fail**.
- `ChartTextFamilyTests` (unit, four cases). Binding `ChartText` to null instead of the plot's
  family fails **one**, the measurer case; stopping `InFamily` stamping the labels fails **two**,
  the two label cases, and leaves the measurer case passing — which is what makes them separate
  claims. The unstated-family case passes under all three and is **labelled a control**.

The third fidelity case took three attempts and the two rejected quantities are worth keeping:

- **An absolute pen position carries the composition as well as the face**, and pinning it would
  have pinned the cancellation described above.
- **A word's ink width carries the two writers' show splitting.** The reference positions each
  digit of `100` separately, so poppler ends its box at the last glyph's ink and reports 17.25
  where our single show reports the full 18.03 advance.

What is asserted instead is **one digit's advance**, the gap between the pens of the axis' `100`
and `80` labels, which share a right edge: 6.009 for ours, 6.010 for the reference, and 5.556 for
the fixed face this replaces.

Per project on the final tree, each run redirected to its own file: Core **247**, Containers 109,
Text 240, Vector 291, Rendering 119, Markup 259, OpenDocument 125, WordProcessing 619,
Spreadsheets 483, Presentations 528, Fidelity **550**. Zero failed, zero skipped.

### Two process traps, both hit this round

- **Editing a script while it is running corrupts it.** Bash re-reads a script from a byte
  offset, so the baseline sweep's trailing summary died with `line 111: o: unbound variable` after
  the `ink%` fix was applied to it mid-run. Nothing was lost — the totals are recomputed from the
  per-document files — and the after-sweep ran from a frozen copy.
- **`--no-build` after a defect-reintroduction cycle measures the defect.** The first full test
  run reported `Core Failed: 2` on a clean working tree, because the binaries were still the ones
  built with the stamping defect. Rebuild before the run that counts, and read the failing test
  names rather than the count alone — the two names were exactly the two the defect was meant to
  break, which is what identified it in a minute rather than an hour.

### What the next round should take, in order

1. **The legend reservation, now separable.** The probe measures it at 2.49 pt on a chart whose
   axis labels are within 0.36 pt, so the two halves of the composition can be attributed
   separately for the first time. `Demick_JetBlue.pptx` page 4 is the corpus instance.
2. **Wire the family into `SheetChart` and `FrameChart`.** The model carries it; both consumers
   ignore it and say so in the code. Each is a one-line change plus its own track's sweep, and
   the sheets agent's own measurement — a legend of three entries in one face against two in
   another accounting for a document's whole residual word deficit — says it is worth a round
   there.
3. **`PptSlideLayout.cs`:1244's group scale**, unmeasured for four rounds now. Read and not
   probed.
4. **Bold chart text.** The reference embeds `DejaVuSerif-Bold` on `Demick_JetBlue.pptx` and we
   embed no bold face at all; `ChartLabel` carries no weight either. Unmeasured for reach.
---

## Round twenty-five — `sheets`, base `4448c2a3f`

Two things, and the second is the one worth the space.

### One reader change, and it closes `batch-010`'s page-count failure

| | baseline `4448c2a3f` | + the drawing-block `CONTINUE` |
|---|---|---|
| matches | 144 / 171 | **145 / 171** |
| total absolute page error | 94 | **90** |
| exactly-correct page counts | 153 | **154** |
| absolute word error | 43694 | 43694 |
| `sheets/batch-001`–`009`, the gate | 89/89, page error 0 | **89/89, page error 0** |
| `sheets/batch-010` | 6/10 | **7/10** |
| every other batch | — | **unchanged** |

The baseline reproduces round twenty-four's closing figures to the digit, per batch as well as in
total. Per batch: 001–008 10/10, 009 9/9, 010 6/10 → **7/10**, 011 6/10, 012 8/10, 013 8/10,
014 9/10, 015 5/9, 016 4/9, 017 6/10, 018 3/4.

**Excel stops writing `MSODRAWING` records once a sheet's Escher stream passes the 8224-byte
record ceiling and writes the rest as `CONTINUE`**, still one per `OBJ`. `BiffRecordReader` joined
each of those to the `OBJ` before it, so every shape past the ceiling was read as an object's
payload and lost. On `INDEX_Digital_Transformation_Toolkits.xls` that is 25 `MSODRAWING` records
totalling 8034 bytes, 70 `CONTINUE`s and 95 `OBJ`s: 25 of 95 pictures drawn, 20 pages against 24.
It now renders 24 and matches.

**The brief's `SheetDrawingArea` hypothesis is refuted, and so is the reading it replaced.** Round
twenty-four's own table — pages 1–13 exact, page 14 ten images against 21 — is what a walk
truncated at 25 shapes produces, not what refutes one. The truncation is real; its mechanism is
the record stream, not the shape walk and not the printed range.

Reach measured by rendering: **1 of 171 documents changes what is drawn.** A census of the same
condition — a `CONTINUE` inside a drawing block — finds it in **18** of the track's 61 OLE2
workbooks. Ceiling 18, reach 1.

### Our PDF writer is reproducible; 17 of 171 documents draw the clock

The brief carried this as a defect in `Paperless.Rendering`. **It is not, on both counts.**

| measurement | result |
|---|---|
| all 171 rendered twice in succession, timestamps masked | **171 same, 0 different** |
| all 171 rendered under two time zones a day apart, timestamps masked | **17 different** |
| the same two zones with `SOURCE_DATE_EPOCH` set, **nothing masked** | **171 same, 0 different** |

`SheetHeaderContext.Printed` defaulted to `DateTime.Now` and was read afresh in every page's
header context; `&D` and `&T` print it. That is correct behaviour and it is also a 17-in-171
false-positive floor under every byte-level reach measure this project has taken. The instant is
now taken once per printout and stamped onto the pages beside `PageCount`, and
`SheetPrintInstant` honours `SOURCE_DATE_EPOCH` — as UTC, so the pinned output does not depend on
the machine's zone. `RenderCommand` passes the same value to `PdfRenderOptions.CreationDate`, a
hook that already existed for this.

**`Paperless.Rendering` was not touched, and neither was `Paperless.Core`.** The change is
`Paperless.Spreadsheets` plus one private method in `Paperless.Cli`, and `BiffRecordReader` is not
shared with the DOC or PPT readers, so neither the words nor the slides track is owed a sweep.

Probe data, six whole-track TSVs and the record dumps behind them: `dotnet/probes/sheets-r25/`.
## Words round twenty-six, at `be5df60ac` — `PAGE` in a running head

### The baseline reproduced the round before it to the byte

`base/rows.tsv` differs from round twenty-three's own pre-fix sweep on exactly the two documents
that round's header fix moved, and nowhere else. The image sweep's TSV is **byte-identical** to
round twenty-three's `img-hdr.tsv` — same md5 — which gives major **360** and summed `|ink|%`
**798.02** over 155 documents, and the gate gives **155/200** with total absolute page error
**98**. Every figure the brief carried reproduced.

### The brief's first item is wrong, and the way it went wrong is worth keeping

The record said `150_5300_13_chg8.doc` "page 1 is missing the reference's entire departmental
title block", drawn in DejaVu Sans at y 701.95–663.55, and that we "draw nothing there". **We
draw all of it.** Our page 1 has the same four DejaVu Sans records and the same two 37 pt
"Advisory"/"Circular" records the reference has, 14.15 pt higher up the page — and looking at the
two page images settles it in one glance.

The mechanism of the mistake is a documented property of the instrument: `pdf-ops.py diff` pairs
records by nearest neighbour **within three points**, so a block displaced by 14.15 pt appears in
*both* one-sided lists. Reading only the "only in the reference" half turns a displacement into a
disappearance. The skill already says the window is three points; what this adds is that the
symptom of exceeding it is *duplication across the two lists*, and the cheap check is to grep the
"only in ours" list for the same text before concluding anything is missing.

What page 1 is really missing is its **second column**. The reference lays paragraphs 1 and 2 out
in two balanced columns — column one x 72.1–306.4, column two x 341.8–576.2 — and we lay the same
text down column one alone. Our column *width* is right, so `sprmSCcolumns` is read correctly;
what is absent is that a multi-column *text section* shorter than a page has its columns balanced
(`SwLayoutFrame::FormatWidthCols`, `wsfrm.cxx`:3955, binary-searches the section height between
`nMinimum` and `nMaximum` unless `SwFormatNoBalancedColumns` is set). Ceiling, censused properly
off the WW8 section table rather than by a byte scan and labelled as a ceiling: **6 of 66 `.doc`
and 4 of 134 `.docx`** declare a section with more than one column, eight of them continuous.
`150_5335_5a.doc` is **not** one of them, so "a single cause across four documents" does not hold
for columns either.

### `PAGE` in a running head printed the producer's cached number

The brief's second item, and much the largest thing on the track. `WritingFieldKind` was produced
by all four readers and consumed by nothing — the "read but never used" class the
`render-comparison` skill lists. Measured before the change: `batch-010/195584360.docx` printed
"Page 10" on **all twenty** of its pages where LibreOffice printed 1 to 20, and
`batch-019/150_5335_5a.doc` printed 13 on its page 9.

`PageFields` now substitutes the field's value into the running head's blocks *before*
`FlowLayouter` sees them, and `PageFurnitureSet` keys its laid-out cache on the page number only
for a head that carries one. Substituting after layout is not open to us: the number is text of a
different width from the cached one, so it has to take part in the line breaking rather than be
painted over it. The sequence comes from the section — `w:pgNumType/@w:fmt` and `sprmSNfcPgn`,
neither previously read, 21 of the 134 DOCX declaring `lowerRoman` — with a field's own `\*`
picture switch overriding it.

**The second half of the brief's second item is refuted.** It said the section's numbering restart
was wrong as well as the cache, because our page 9 of `150_5335_5a.doc` said 13 against the
reference's 3. With the field computed, that page says **3** and its page 5 says **iii**, both
matching. There was never a restart defect: a renderer printing a cached string tells you nothing
about what page it thinks it is on, and the inference drawn from it was unfalsifiable.

### Measured reach, and what the gate did

Reach measured by rendering, never by censusing: comparing the `pdftotext` of all 200 documents
between the two sweeps, **104 of 200 documents changed — 80 DOCX and 24 DOC — across 3548 pages.**
The DOCX census (86 of the 111 zip-container documents that have furniture at all carry a `PAGE`
field in it) was an over-estimate for DOCX and blind to the DOC half; the rendered figure is the
one to quote.

| | before | after |
|---|---:|---:|
| parity gate | 155 | 155 |
| abs page error | 98 | 98 |
| abs word error | 6885 | **6840** |
| major pages (155 common docs) | 360 | **362** |
| summed `\|ink\|%` | 798.02 | **797.48** |

**Say the honest thing first: the gate did not move, and it could not have.** A footer's page
number is two or three glyphs; it changes no page count and, on all but one document, no word
count. This is the skill's "a real fix that moves no verdict" case, and the evidence that it is
real is the reach measurement and the per-document text, not the scoreboard.

The two extra major pages are an **instrument artefact and not a regression**, and this is
measured rather than assumed. Both are on `easa-regulations-update-20.docx`. Running
`pdf-image-diff.py` for the same reference against each sweep's rendering gives *identical*
`|ink|%` on all 22 pages to two decimals — page 18 is 0.29 either way and page 21 is 0.29 either
way — while their verdicts flip from `shifted` to `MAJOR`. And the footers on those pages went
from "Page | 2" on every page to "Page | 5", "Page | 7", "Page | 15", "Page | 19", each now equal
to the reference's. **A verdict can move while the number it is supposedly derived from does
not**; quote the ink column, not the verdict count, when a change is this small.

### What is not done, and is not pretending to be

- **`NUMPAGES`.** The spans are recorded and deliberately not substituted: the total is not known
  while the running head is being laid out. A second pass would do it and is not written.
- **`PAGE` in body text.** Left at its cached value. Resolving it is circular in a way a running
  head's is not — the page a paragraph lands on depends on the height of the paragraphs before it —
  and every `PAGE` field in this corpus's zip-container documents is in a header or a footer.
- **RTF.** `RtfDocumentReader` records fields for extraction and its layout path does not carry the
  spans, so an RTF footer still prints the cached number. No corpus document on this track is RTF;
  this is a real gap in the library rather than unfinished corpus work, and it is one file's worth.
- **The WW8 half has no unit test.** A synthetic `.doc` cannot be built here, so the DOC path rests
  on the corpus measurement — 24 documents changed — and on `150_5335_5a.doc` matching page for
  page. Said plainly rather than papered over.

### Verified by putting the defects back

Twelve layout defects and six reader defects, each reintroduced and the test run watched:

| defect put back | caught by |
|---|---|
| draw the cached result (the original bug) | `EveryPageGetsItsOwnNumber` and four others |
| cache the head against the slot alone | `EveryPageGetsItsOwnNumber` and three others |
| always resolve against page one | `EveryPageGetsItsOwnNumber` and three others |
| ignore the section's sequence | `TheSectionsSequenceIsHonoured` |
| ignore the field's own picture switch | `AFieldsOwnPictureWins` |
| leave the runs after the span where they were | `RunsAroundTheFieldKeepTheirText` |
| drop every run the span touches | `RunsAroundTheFieldKeepTheirText` |
| splice from the span's start rather than its end | seven of the ten |
| trust a span that runs past the text | `AnOutOfRangeSpanIsIgnored` |
| do not recurse into a table in the head | `ANumberInsideAHeaderTableIsResolved` |
| substitute a `NUMPAGES` span too | `APageCountFieldIsLeftAlone` |
| apply several spans forwards, not from the end back | `TwoFieldsInOneParagraphBothResolve` |
| DOCX walker records no span | `ADocxFooterFieldIsRecordedAndResolved` |
| DOCX instruction taken from one `w:instrText` only | `ADocxFooterFieldIsRecordedAndResolved` |
| ODF `page-number` treated as ordinary markup | `AnOdfFooterFieldIsRecordedAndResolved` |
| ODF walker records no span | `AnOdfFooterFieldIsRecordedAndResolved` |
| a `MERGEFORMAT` switch read as the number picture | `ADocxFooterFieldIsRecordedAndResolved` |
| result start taken at the field's begin, not its separator | **nothing — and it is not a defect** |

The last row is the useful one. Nothing can be emitted between a DOCX field's `begin` and its
`separate`: `w:instrText` is accumulated rather than drawn, and `w:t`, `w:tab` and `w:br` are all
guarded by `!_inInstruction`. So the two offsets are provably equal and the "defect" is an
equivalent formulation. A first attempt at the twelfth layout defect had the same shape —
`Kind == PageCount` against `Kind != PageNumber` over a two-member enum — and was replaced rather
than reported as an uncaught defect.

`ARunningHeadWithNoFieldIsStillLaidOutOnce` is a **control rather than a detector** and is
labelled so in the file: no defect above breaks it, because "the cache is keyed on the slot alone
when nothing varies" is what every wrong version also does. It is kept because a regression that
keyed *every* running head on its page number would be invisible to all the others.

### A process trap this round walked into

`git add -A` was run while the defect-reintroduction script had its first patch applied, and the
commit captured the defect — `Fields = []` in `DocxLayoutSource`, which *is* "the reader records
no span". It survived one commit and was caught by `git diff` afterwards. A script that patches,
tests and reverts leaves the tree transiently wrong, and a whole-tree stage during that window is
indistinguishable from committing working code. **Stage explicit paths, or do not commit while
such a script is running.** The sweep was unaffected, because it measured a checksummed CLI
snapshot taken before the script started — which is the second argument for snapshotting, beside
the one the skill already gives.

### What the next round should take, in order

1. **Balanced columns for a continuous multi-column section.** Localised, mechanised in
   `SwLayoutFrame::FormatWidthCols`, and the reason `150_5300_13_chg8.doc` is 21 pages against 18.
   Ceiling ten documents, three of them (`chg8`, `chg10`, `chg12`) at the top of the remaining ink.
   The Paginator already models columns and fills them in order; what is missing is the binary
   search over the section's height when the section is shorter than the frame.
2. **`chg8` page 1 is 14.15 pt too high, uniformly.** Measured, not explained: every record from
   the FAA seal down to "Initiated by:" is 14.15 pt above the reference's, and the offset then
   changes to 11.40 pt at the first body paragraph. The header push-down rule is already
   implemented and tested (`HeaderOverflowTests`), so this is *not* simply that; LibreOffice's flat
   export of the document gives the section a header holding one empty paragraph, and our DOC
   reader treats an empty header story as "this section has no header". That is a hypothesis with
   a measurement attached, not a diagnosis.
3. **`NUMPAGES`**, which is now one pass away: the spans are recorded and the paginator knows the
   total by the time it returns.
4. **`w:pBdr` for `.rtf` and `.odt`**, and **`w:val="double"` reserving one width where it should
   reserve three** — both unchanged, both needing an authored probe rather than a corpus sweep, and
   both correctly left alone this round.

## Words, round twenty-seven, at `d7fd6cf13` — balanced columns, the gap that was never there, and `NUMPAGES`

### The baseline reproduced the brief exactly

Whole-track sweep against a checksummed CLI snapshot: **155/200**, absolute page error **98**,
165 documents with an exactly correct page count, absolute word error **6840**. The image
sweep over the 155 matching documents gives **1603 pages, 362 major, summed `|ink|%` 797.48**
against a signed 618.51 — the invariant `|ink|% ≥ |ink%|` holds. Every figure the brief carried
reproduced. Per batch: 001–005 10/10, 006 9/10, 007 10/10, 008 9/10, 009 10/10, 010 8/9,
011 9/10, 012 9/10, 013 6/9, 014 4/10, 015 5/10, 016 7/10, 017 6/10, 018 6/10, 019 4/10,
020 3/10, 021 0/2.

### The brief's first item had a second half nobody had looked at

`sprmSDxaColumns` — the gap between a WW8 section's columns — **defaults to 708 twips, and we
defaulted it to zero.** `ReadUSprm(pSep, pIds[4], 708)` (`ww8par6.cxx`:987), commented "default
distance 1.25 cm"; the DOCX side says the same figure as `m_nColumnDistance( 1249 )` hundredths
of a millimetre (`PropertyMap.cxx`:424). The decisive check was asking LibreOffice rather than
reading either: its flat-ODF export of `150_5300_13_chg8.doc` states
`fo:column-gap="0.4917in"`, which is 708 twips to the digit.

It is not the gutter that matters. A column's *width* is the measure less the gaps, divided by
the count, so a zero gap made that document's columns **252 pt where the reference's are 234.3**
— 8% too wide, so every line of every two-column section broke late. With the default read, our
columns measure 234.30.

**The brief's sentence "our column *width* is right, so `sprmSCcolumns` is read correctly" is
therefore wrong**, and it is the same shape of error the skill warns about: the count was read
correctly and the width was not, and only the width is visible in the output.

### Balanced columns

`WritingSection.BalancesColumns`, decided by a pass over the finished section list because the
answer is a property of the *next* section: both LibreOffice Word importers say so in the same
shape — `if (aNext == aEnd || !aNext->IsContinuous()) pRet->SetFormatAttr(SwFormatNoBalancedColumns(true))`
(`ww8par.cxx`:4576), and `pPrevSection->DontBalanceTextColumns()` reached from the page-break
branch of `SectionPropertyMap::CloseSectionGroup` (`dmapper/PropertyMap.cxx`:1919) with the
last-section case in `ApplyColumnProperties`. Both are switched off wholesale by
`fNoColumnBalance` / `w:compat/w:noColumnBalance`, which the DOP reader did not read.

The paginator gains a **column band** — a top and a bottom belonging to the section rather than
to the page — and **bisects the band's height**, which is the quantity
`SwLayoutFrame::FormatWidthCols` searches for iteratively ("nMaximum … is then maintained as the
minimum height on which the content fit into the columns", `wsfrm.cxx`:3912). Bisection is open
to us and not to Writer because our fill is a pure function of the band it is given.

Two things the mechanism needed that reading the C++ does not tell you:

- **The band must count the last paragraph's space-after.** The fitting rules count line boxes,
  and space-after is added to the running height *after* the last fitting test, so a band chosen
  from the lines alone ends a paragraph gap too high and lifts everything below the section.
  Measured at exactly **10 pt** on `chg8`: what follows the balanced box sat 21.40 pt above the
  reference before and 12.94 pt after, against a page-wide offset of 11.40 pt.
- **A `PlacedLine` has to carry its own column count.** One page holds a single-column opening,
  a two-column stretch and a single-column close, and the page is *written* when the last of
  them is current — so reading the count off the page drew the full-width paragraphs into half a
  column. This was invisible before, because a section that fills only column one never exercises
  a second column area.

The distribution now matches the reference exactly on the document the brief named: seven lines
in column one at x 72.00 against 72.10, six in column two at x 341.70 against 341.80, the same
paragraph gap in the same place.

### `NUMPAGES`

One extra fill. The document is laid out once to learn its page count, every furniture set that
carries the field is told (which discards its laid-out cache), and it is laid out again. Not
iterated to a fixed point, for the reason Writer damps the same circularity. A page *count* is
the same on every page, so it deliberately does **not** reach the running head's cache key — only
a page *number* does.

### Reach, measured by rendering

The two sweeps' PDFs byte-compared with `SOURCE_DATE_EPOCH` pinned, which is the only figure
worth quoting:

| change | ceiling (census) | reach (rendered) |
|---|---:|---:|
| column gap + balancing | 6 `.doc` + 4 `.docx` | **7 documents — 3 `.doc`, 4 `.docx`** |
| `NUMPAGES` | 42 documents mention it | **22 documents — 21 `.docx`, 1 `.doc`** |
| both | — | **29 of 200** |

Note the gate sweep alone under-counts even that: `mde087077~283.docx` has identical page and
word columns before and after and its rendering changed, which only the byte comparison and the
image diff can see.

### What the scoreboard did, said first and plainly

| | before | columns | + `NUMPAGES` |
|---|---:|---:|---:|
| parity gate | 155 | 155 | **155** |
| abs page error | 98 | 101 | **101** |
| exactly-correct page count | 165 | 165 | **165** |
| abs word error | 6840 | 7198 | **7198** |
| major pages (155 common docs) | 362 | 361 | **361** |
| summed `\|ink\|%` | 797.48 | 792.77 | **792.78** |

**The gate did not move, and two of its continuous metrics moved against us.** All of the page
error and all of the word error is on two documents: `150_5300_13_chg8.doc` (21 → 23 pages
against 18) and `150_5300_13_chg10.doc` (79 → 80 against 76).

The word figure is the gate's blind spot rather than a regression, and this is measured rather
than argued. `wc -w` compares two *sums*, so over-drawing cancels under-drawing. Counting the
token multiset against the reference's instead — over-draw plus under-draw — the three documents
go:

| document | before | after |
|---|---:|---:|
| `150_5300_13_chg10.doc` | 3565 (over 2030, under 1535) | **2438 (over 1657, under 781)** |
| `150_5300_13_chg8.doc` | 1120 (over 513, under 607) | **693 (over 384, under 309)** |
| `150_5300_13_chg12.doc` | 518 (over 305, under 213) | 614 (over 334, under 280) |

`chg10`'s under-draw halves. The gross word count rose because we now draw 754 more of the words
the reference draws while drawing 373 fewer that it does not, and the two do not cancel evenly.

The image metric agrees and is the honest headline for the columns work: **`|ink|%` 797.48 →
792.77 and major pages 362 → 361**, over three documents — `JEMIT_Template.docx` 9.50 → 2.15 with
4 major pages down to 1, `mde087077~283.docx` 4.91 → 5.13, `chg12` 70.29 → 72.71 with 2 more
major pages.

**`NUMPAGES` moves the image metric by 0.01 across five documents and no verdict anywhere**, which
is what a two- or three-character substitution should do and is stated here rather than left for a
reader to infer from a table that does not change.

### Why the page count on those two documents is still wrong, and what is left on them

Not the balancing. Three separate things remain on `chg8`, each visible in LibreOffice's own
flat-ODF export of it:

- **Unequal columns.** Two of its sections state `style:column style:rel-width="4680*"` and
  `"5112*"` — `sep.fEvenlySpaced = 0` with per-column widths in `rgdxaColumnWidthSpacing`.
  `PageGeometry` models equal columns only, and this is a model change rather than a reader one.
- **A blank page 2** we emit and the reference does not, worth exactly one of the three pages.
- **The 14.15 pt offset**, below.

### The 14.15 pt offset: the brief's hypothesis is right and its mechanism is not

The brief guessed "our DOC reader treats an empty header story as *this section has no header*",
and that is what the code does. The mechanism is more specific and worth writing down before
anyone implements it, because implementing the brief's version would be wrong:

For Word 97 and later, **`grpfIhdt` is not read from a sprm at all**. `wwSectionManager::SetSegmentToPageDesc`
synthesises it as `WW8_HEADER_ODD | WW8_FOOTER_ODD | WW8_HEADER_FIRST | WW8_FOOTER_FIRST`
(plus the even pair when `fFacingPages`), and clears a bit only when that story's length is
**zero** *and* the previous section did not have it either (`ww8par6.cxx`:1222-1258). A story
holding just a paragraph mark has length 1, so the bit stays set — and `Read_HdFt` then finds
`nLen < 2`, does not read the text, and still turns the header on
(`SwFormatHeader(true)`), which creates a header of one empty paragraph.

So the displacement is **not** universal: `SetPageULSpaceItems` gives that header a *minimum*
height of `dyaTop − dyaHdrTop` with dynamic spacing, which is the same rule `PushedDownBy`
already implements. It bites only where a document reserves no header band at all —
`chg8` has `dyaTop == dyaHdrTop == 720`, so the empty header's own line height lands on the body
in full. That is a narrow case, it is worth a round of its own, and it needs the reach measured
before it lands: turning it on unconditionally gives *every* Word 97 DOC an empty header line.

The second half of the brief's item — that the offset "changes to 11.40 pt at the first body
paragraph" — reproduces, and the 2.75 pt difference between the two is a separate, unexplained
quantity in the title block's table.

### Verified by putting nineteen defects back

Each applied alone, built, and the tests watched.

| defect put back | caught by |
|---|---|
| WW8 column gap defaults to zero | `ASectionStatingNoColumnGapTakesAnInchAndAQuarterCentimetre` |
| DOCX column gap defaults to zero | `AColumnsElementWithNoSpaceTakesAnInchAndAQuarterCentimetre` |
| nothing is ever marked as balancing | `AStretchClosedByAContinuousBreakBalances` |
| the next section's break is ignored | `AStretchClosedByAPageBreakDoesNot` |
| the last section is marked too | `TheLastSectionNeverBalances` and two `BidiColumnTests` |
| `noColumnBalance` is ignored | `TheDocumentsOwnFlagSuppressesEverything` |
| the same-sheet test is ignored | `AContinuousBreakOntoDifferentPaperDoesNotCount` |
| single-column sections are marked | `ASingleColumnSectionIsNotMarked` |
| the paginator never begins a search | four of `BalancedColumnTests` |
| a column restarts at the page top | `TheSecondColumnStartsBesideTheFirst`, `TheColumnsAreWithinOneLineOfEachOther` |
| the search accepts its first trial | six of `BalancedColumnTests` |
| the band ignores the trailing space | `TheBoxCountsTheLastParagraphsSpaceAfter` |
| the box ends where its content does | `TheBoxCountsTheLastParagraphsSpaceAfter`, `WhatFollowsStartsBelowTheWholeBox` |
| a line does not record its columns | `EachLineCarriesItsOwnColumnCount`, `TheColumnsAreDrawnSideBySide` |
| no second pass for a page count | `APageCountFieldPrintsTheDocumentsOwnTotal` and one more |
| the total does not discard the cache | the same two |
| a page count prints the page number | the same two |
| a page count varies per page | `AHeadHoldingOnlyACountIsSharedAcrossPages`, `APageCountFieldIsLeftAlone` |
| a page count resolved without a total | **nothing — and it is a real defect** |

The last row is the one that matters. `PageFields.Resolve`'s outer guard returns early when the
total is unknown *and* nothing else needs resolving, so the only test exercising the inner guard
never reached it. But a footer holding **both** fields — "Page 1 of 12", the ordinary shape — is
resolved on the measuring pass because its *number* varies, and without the guard the unknown
total renders as `0`: a head measured at the wrong width, which can hand the second pass a
different page count to print. It is not an equivalent formulation, unlike the two the previous
round found. `TheMeasuringPassLeavesTheCountAtItsCachedValue` is the detector, written after the
fact, and it fails with the guard removed.

### Test counts

Core 247, Containers 109, Text 240, Vector 291, Rendering 119, Markup 259, OpenDocument 125,
**WordProcessing 683** (660 + 23 new), Spreadsheets 498, Presentations 528, Fidelity 550,
0 skipped throughout — every project on its known-good count, with the tree rebuilt after the
last defect came back out.

### What the next round should take

1. **Unequal columns** — `fEvenlySpaced = 0` / `w:cols w:equalWidth="0"`, which needs
   `PageGeometry` to hold per-column widths rather than a count and a gap. It is on the two
   documents that hold the whole of this round's adverse page error.
2. **The blank page** `chg8` emits between its title page and its first chapter.
3. **The empty-header rule above**, with its reach measured before it lands.
4. **`PAGE` in body text** and **RTF field spans** remain where the last round left them, and
   `w:val="double"` still needs an authored probe.

## Sheets round twenty-five and words round twenty-six, merged at `f61af6b79`

**The first gate movement in three rounds, and it came from a record-stream bug.** A bare
`CONTINUE` inside a BIFF drawing block is Escher, not the preceding `OBJ`'s payload: Excel stops
writing `MSODRAWING` once a sheet's Escher stream passes the 8224-byte ceiling and writes the
rest as `CONTINUE`, still one per `OBJ`. We absorbed each into the preceding `OBJ` and lost every
shape past the ceiling.

| sheets | base | after |
|---|---:|---:|
| matches | 144/171 | **145/171** |
| absolute page error | 94 | **90** |
| exact page counts | 153 | **154** |
| batch-010 | 6/10 | **7/10** |

Reach: **1 of 171 documents changes what is drawn**, the other 170 byte-identical; the condition
censuses at 18 of the track's 61 OLE2 workbooks, so 18 is the ceiling and 1 is the reach.
Verified here at the merge — `INDEX_Digital_Transformation_Toolkits.xls` now renders **24 pages
against 24**, having been 20/24 for four rounds.

### Our PDF writer was never the nondeterminism — the clock was

The brief said the writer was not byte-reproducible. Both halves were wrong, and the round has
the measurements: all 171 rendered **twice in succession** with timestamps masked are **171 same,
0 different**. Rendered under two time zones a day apart they are **17 different** — those
documents print the date, via `SheetHeaderContext.Printed` defaulting to `DateTime.Now` and read
afresh in *every page's* header context, which `&D`/`&T` then print.

So the floor under three rounds of byte-level reach figures was **17 in 171 and calendar-shaped**,
not a writer defect. The instant is now taken once per printout, honours `SOURCE_DATE_EPOCH` read
as UTC, and is passed to `/CreationDate`; with it set, two runs under different zones are
byte-equal **with nothing masked at all**.

### `PAGE` in a running head now prints the page's own number

`WritingFieldKind` was produced by all four readers and consumed by nothing, so a document
printed its producer's cached number on every page. Substitution happens before `FlowLayouter`,
because the number has a different advance and must take part in line breaking; the sequence
comes from `w:pgNumType/@w:fmt` and `sprmSNfcPgn`, neither previously read.

**Reach measured by rendering: 104 of 200 documents changed, 80 DOCX and 24 DOC, 3548 pages.**
The DOCX census said 86 — overstating for DOCX and blind to DOC entirely. The gate did not move
and could not have: a footer number is three glyphs.

### Three of my brief's claims were refuted, one of them by its own instrument

- **The missing title block does not exist.** That page is not missing its title block; we draw
  all of it, 14.15 pt too high. The diagnosis came from reading only the *reference-only* half of
  a `pdf-ops.py` diff whose 3 pt window the displacement exceeded — so the block appears in
  **both** one-sided lists. Now recorded in the render-comparison skill as a tool trap.
- **The page-numbering restart defect does not exist.** With `PAGE` computed, that document's
  page 9 says 3 and page 5 says iii, both matching. A cached string cannot tell you what page a
  renderer thinks it is on.
- **"One cause across four documents" fails.** The real defect on that page is a continuous
  two-column section we fill one column of; censused off the WW8 section table it is 6 `.doc`
  plus 4 `.docx`, and the document I named as a sibling is not among them.

One earlier refutation was itself refuted: round twenty-four read a table showing pages 1–13
agreeing exactly and divergence from page 14 as evidence *against* a truncated shape walk. **A
walk truncated at 25 does place the first fifteen exactly and then thin** — that is what a prefix
looks like. Correct measurement, inverted reading, and the truncation was real.

## Round twenty-six — sheets: a turned cell, measured twice and moving no verdict

Base `d7fd6cf13`. The baseline sweep reproduces round twenty-five's committed after-sweep **row
for row on all 171** — 145/171, absolute page error 90, 154 exact page counts, zero `ref-failed`.

### Batch 010 is bounded by the slides track's work, and the round said so early

Its three failures are exactly the three predicted: `EHEST-Pre-departure-checklist…xls` and
`Keywords_Mapping_Graphs_and_Charts.xlsx` behind `ChartLabel`'s font, and
`Template Pilot Logbook JAR-FCL V3.0.xls` unreachable by the word gate. Nothing in
`Paperless.Spreadsheets` can move them, so the round went to the open lead instead.

### A turned cell, in both halves

**Its row is its text's width, and a quarter turn is not a rotation.**
`ScPatternAttr::GetCellOrientation` reads exactly 9000 and 27000 as `BottomUp` and `TopBottom`, and
`nRotate` is only read when the orientation came back `Standard` — so the quarter turns take a
branch with no wrap, no `SC_ROT_BREAK_FACTOR` cap and no sheet minimum, because `bStdAllowed` gates
the last two on that same test. A single letter in a 10 pt cell asks for **149 twips at 90° and 257
at 45°**. 216 of 216 probe rows now come back LibreOffice's own.

**A glyph advances by whole device pixels**, so a string's width is the sum of its rounded advances
and not the rounded sum. Fourteen of the eighteen distinct probe widths agree either way and the
four twelve-point ones differ by 1.4%. Restoring the rounded total fails 18 of the 36 test cases.

**Its lines are a block, laid out unturned from an anchor and then turned about it.** Every line was
drawn at one point — four records at exactly `(363.49, 262.53)` on `Keywords_Mapping` page 43 — and
every single-line turned cell sat an ascent from the reference. Round twenty-five's *reference* half
of that measurement was mispaired: the `(394.38, 761.89 / 731.90 / 716.91)` it quoted as the
reference stepping those lines is the ordinary data column, which we draw at the same place. The
reference's rotated heads are its `Carlito-Bold` records at six other x values, and it does not wrap
that cell at all. Seven of eight probe positions now
land within 0.15 pt; the eighth is recorded as a 2.55 pt shortfall, exactly the trailing space
EditEngine leaves out of the width it aligns by.

An obliquely turned cell is left exactly as it was. `DrawRotated` centres its block across the
column and lifts its anchor by the block's height times the cosine, which is a different rule again,
and nothing in the corpus is turned by anything but a quarter to fit it against.

| sheets | base | after |
|---|---:|---:|
| matches | 145/171 | 145/171 |
| absolute page error | 90 | 90 |
| exact page counts | 154 | 154 |
| absolute word error | 43694 | **43198** |

**No verdict moves and no page count moves, and that is the headline.** Reach measured by
rendering: **11 of 171 documents change what is drawn**, each confirmed with `SOURCE_DATE_EPOCH`
pinned so the clock cannot contribute. Nine documents' word counts change, four towards the
reference and five away — a heading that used to wrap into six lines scored six `pdftotext` tokens
and now scores one, while the reference draws it as one line a glyph at a time and poppler still
finds two or three words in it. The ink is closer and the token count is further.

### A census that under-counted, which is the less familiar direction

Counting `textRotation` in `xl/styles.xml` found **four** documents on this track. Eight of the
eleven that actually changed are `.xls`, which states its rotation in an `XF` record no zip census
can see. **A census that cannot read one of the corpus's two containers is a ceiling on the half it
can read and nothing at all on the other** — the usual failure is a census overstating reach, and
this one understated it nearly threefold.

### Also fixed, with nil corpus reach and said so

The ODF reader *clamped* a rotation angle instead of folding it, so 270° became +90 and 315° became
+90 as well. No sheets-track document is ODF with a turned cell; without the fix six of the probe's
thirty-six sheets could not be read at all.

### The chart-label font, picked up where the slides round left it

The slides round wired `SlideChart` to `ChartPlot.TextFamily` and left `SheetChart`'s measurer
unwired with a remark saying the change belonged to whichever round sweeps the sheets track. It is
now wired on both the measuring and the drawing path, because a label measured in one face and drawn
in another is centred on the wrong width.

**Measured on the merged tree before the wiring, `batch-010` is still 7/10 with the same three
failures** — so the slides merge on its own did not move this track's gate, against the prediction
that it might. With the wiring, `Keywords_Mapping_Graphs_and_Charts.xlsx` embeds exactly the
reference's two faces where it embedded Liberation Sans beside them; its word count moves 4650 to
4647 against a reference 4808, so the residue there is not the face.

Reach measured by rendering, both sides with the instant pinned: **1 of 171 documents**, and the
whole-track sweep is 145/171 with page error 90 and 154 exact — the baseline's numbers and its
per-batch line. Two rows of that sweep came back `ref-failed` under load and were re-run alone and
spliced; one of them is `ECA Sinters.xls`, the same document round twenty-five had to splice.

**The remaining half is BIFF and is named rather than guessed at.** `XlsChartReader` sets no family
at all, so `EHEST-Pre-departure-checklist…xls` still embeds no Carlito where the reference embeds
two. That work is in `Paperless.Spreadsheets` and so belongs to this track.
## Slides round twenty-three: the legend, inherited unmeasured and kept on its own numbers

A container restart killed the previous slides agent mid-round; its two commits — a probe and a
legend fix — arrived as an unverified prior attempt. Both were re-derived before being kept, and
the fix as it stood **could not have passed the round's own gate**: it left
`Paperless.Core.Tests` at 246/247, because `ChartLegendLayoutTests` asserted exactly the text
shape the fix stopped measuring.

| slides, 163 documents | base `d7fd6cf13` | after |
|---|---:|---:|
| word gate | 151 | 151 |
| `ink%` | 1265.33 | 1263.67 |
| `\|ink\|%` | 1584.92 | **1583.49** |
| major pages | 430 | **428** |
| census, unexplained pages | 304 over 91 docs | 304 over 91 docs |

Verdicts changed: **0**. Every batch holds its count.

**The measurement survived and half the explanation did not.** The salvage's numbers reproduce
to the digit — legend name pen 625.07 → 626.85 against 627.56, row pitch 17.35 → 14.18 against
14.09 — and its claim, that a legend entry's name is the one chart text carrying no
`TextShapeInset`, is confirmed by a probe varying the *font size*: the key-to-pen gap is 2.83,
2.83 and 3.07 pt at 7, 10 and 14 pt, which is `max(1 mm, 0.22 × font)` three times over with no
free parameter, where the inset-bearing reading fits none of the three.

What the same probe showed is that **the legend was not being sized by its own font at all.**
Every length in `lcl_placeLegendEntries` is a fraction of the legend's character height, and
both chart readers only ever looked for the *axis label* size. Ours was 6.00 pt of key and
14.18 pt of pitch at all three font sizes; the reference is 4.20/5.98/8.39 and
10.34/14.09/19.33.

That matters for how the salvage is judged, because the two errors were **cancelling**. Signed
error on the plot rectangle's right edge over the seven probe decks:

| deck | base | salvage alone | both |
|---|---:|---:|---:|
| 5 names, 7 pt | −16.03 | −12.43 | −0.69 |
| 5 names, 10 pt | −4.33 | −0.73 | −0.73 |
| 5 names, 14 pt | +14.41 | **+18.01** | +0.73 |
| 10 names, 14 pt | +27.13 | **+30.73** | +1.46 |
| mean absolute, all seven | 11.14 | 9.59 | **1.27** |

The salvage alone makes the 14 pt decks *worse*, by exactly the inset it removes. Kept only
because the other half went in beside it.

**Reach measured by rendering: 9 of 163 documents changed, 154 byte-identical** — and those 9
are every deck in the track with a legend on a chart. 15 have chart parts; the 6 with no legend
did not move.

**The regression it was meant to close is 60% closed, not closed.** Round twenty-two's chart
face fix cost 2.52 of `|ink|%`; this returns 1.43.

### Two items off the list, one refuted number

- **`PptSlideLayout.cs`'s group-scale branch**, read and never probed for five rounds. Now
  instrumented and censused over all 51 corpus `.ppt`: of 7514 text shapes taking the upright
  branch, **7364 have an exactly unit placement and 150 do not**, across 11 documents, at factors
  from 0.00063 to 1.385. The comment's stated premise — "the commonest group's factor is
  1.00025" — is **false**: the whole band under 0.1% is empty. The reading it defends is right,
  settled on `hofman.ppt`, whose 34 scaled shapes carry A = 1.1163 and D = 1.0315 and whose
  reference draws the same 24.01, 28.01, 32.00 and 43.99 pt we do. No code change; item closed.
- **The bold chart text lead named the wrong text.** It is the chart *title* (18 pt bold) and
  the *axis titles* (10 pt bold), which an OOXML chart takes from `objectformatter.cxx`'s
  auto-text table and never from chart2's model defaults we cite; the axis *labels* the lead
  implied are correctly not bold by default. Separately, a stated `b="1"` is not read at all —
  36 of 61 chart parts over 7 documents. Both recorded in `Paperless.Presentations/TODO.md`
  with the measurements; neither implemented, because the weight needs `ChartLabel` plus all
  three consumers and two of those are other tracks' files.

## Round twenty-four: slides — the OOXML chart auto-text table, and the wrap it uncovered

Baseline measured first and it reproduces the brief **exactly**: `MATCH 151/163`, `ink%`
1263.67, `|ink|%` 1583.49, 428 major pages, 4199 census pages with 304 unexplained over 91
documents, at `7b3704e59`.

**Two changes, and the second exists because the first uncovered it.**

An OOXML chart never reaches `chart2`'s model defaults — the import applies
`objectformatter.cxx`'s auto-text table first (`:415-434`, `TextFormatter` `:906-929`). A chart
title is **18 pt bold**, an axis title **10 pt bold**, everything else 10 pt regular, and
`mnRelFontSize` scales the first by 120% when the chart space states a size of its own. We drew
13 pt and 9 pt with no weight at all, citing `Title.cxx` — which is the right answer for an ODF
chart and the wrong one for every OOXML chart there is.

Settled against LibreOffice's own model and not only against its ink: `Demick_JetBlue.pptx`
states no `sz` and no `b` anywhere in its five chart parts, and `--convert-to odp` writes its
title as `fo:font-size="18pt" fo:font-weight="bold"`, its axis titles `10pt`/`bold`, and its
axes and legend `10pt` with no weight.

Correcting the size then exposed a second error it had been cancelling: **we never wrapped a
chart title at all.** `ChartView.cxx:1084-1085` gives a main title
`aTextMaxWidth.Width = rPageSize.Width * 0.8`. At 13 pt the corpus's titles fitted on one line;
at the correct 18 pt one of them is 659 pt inside a 634 pt frame.

The plot area's top edge is what a title's band decides, so it isolates both changes. Signed
error against the reference on the five chart pages of that deck:

| page | 4 | 5 | 6 | 7 | 8 | mean abs |
|---|---:|---:|---:|---:|---:|---:|
| base | 10.08 | 30.79 | 10.09 | 31.73 | 31.73 | 22.88 |
| + auto-text | 1.08 | 21.79 | 1.09 | 22.73 | 22.73 | 13.88 |
| + wrap | 1.08 | **0.64** | 1.09 | **1.58** | **1.58** | **1.19** |

Neither alone gets below 13.88, which is the same shape as round twenty-three's legend pair.
The two wrapped lines start at x 130.02 and 261.12 against the reference's 130.77 and 259.36 —
the break lands in the same place, which is what makes 0.8 the rule rather than a fitted
constant.

### The trade, with both numbers

| slides, 163 documents | base `7b3704e59` | + auto-text | + wrap |
|---|---:|---:|---:|
| word gate | 151 | 151 | 151 |
| `ink%` | 1263.67 | 1267.60 | 1271.64 |
| `\|ink\|%` | 1583.49 | 1588.12 | 1592.18 |
| major pages | 428 | **427** | **427** |
| census, unexplained | 304 over 91 | **303** over 91 | **303** over 91 |

Verdicts changed: **0**. Every batch holds its count.

**Reach measured by rendering: 156 of 163 byte-identical, 7 changed**, all seven chart decks
— six from the auto-text table and a seventh, `bitesize-writing-a-report.pptx`, from the wrap.
Six of the seven net −**1.62** of `|ink|%`. The whole of the aggregate regression is
`Demick_JetBlue.pptx` at **+10.31**, and it is the deck the table above measures: its five
charts' titles stopped fitting on one line at the correct size, and the wrap that then places
them correctly costs another 4.12 because the *bottom* of its plot area is still 38.72 pt out.

**So the aggregate went the wrong way while the thing being fixed became right**, on one
document out of 163, and I am keeping both changes. The auto-text values are LibreOffice's own,
read out of its `odp` export rather than fitted; the wrap's 0.8 is verbatim from
`ChartView.cxx` and puts our break within 1.8 pt of the reference's. `|ink|%` is a page-share
metric: shortening our plot area from 170.5 pt to 149.4 pt against the reference's 187.4
displaces more ink than leaving its top edge 21.8 pt wrong did, because the error that decides
its *height* is at the other end and neither change touches it.

**What is left on that deck is the band under the plot, not the band above it.** With both
changes in, the plot area's *bottom* edge on the same five pages is 12.73, 38.72, 4.88, 12.73
and 4.87 pt too high, and neither change touches it. The category labels, the category axis'
title and a bottom legend over-reserve; that is the next measurement, and until it is made the
`|ink|%` on this document is not evidence about the title band.

### Cross-track

The **weight** cannot reach the other tracks: `SheetChart` and `FrameChart` take the argument
and drop it, in code, with a comment, exactly as they did for `ChartPlot.TextFamily`. The
**sizes** and the **wrap** can, because every consumer honours them, so this round owes those
tracks a measurement. Eight documents outside slides hold a chart — one DOCX, one XLSX, and six
`.xls` with a BIFF chart substream. Rendered at both commits with `SOURCE_DATE_EPOCH` set and
byte-compared: **six identical, two changed**, and against `soffice`'s own PDF of the two that
moved the DOCX is 0.00 `|ink|%` either way and the XLSX goes 23.49 → 23.50 with its major-page
count unchanged at 10.

### Two questions answered, one refuted, two opened

- **The legend row pitch is on a 96 dpi grid.** Round twenty-three measured the row height at
  three font sizes, found it was not a constant multiple of the font, and named the probe. Run
  at eleven sizes a point apart: subtract exactly 1 mm and every pitch from 6 to 14 pt is a
  whole multiple of **0.75 pt** to within 0.028, with steps of 1.013, 2.013, 2.000, 0.987,
  2.000, 0.987, 1.013 and 2.987 pixels. The 1 mm is the floor the padding
  (`max(1 mm, 0.33 × font)`) and the key gap (`max(1 mm, 0.22 × font)`) already have. Still
  open: the rate feeding the grid — Liberation Mono's 1.1328 em predicts six of the nine — and
  a second regime above 14 pt.
- **The chart text residual is neither an offset nor a factor**, and round twenty-three's
  explanation of it is refuted twice over. 10 × 0.9889 = 9.889 and the reference draws 9.889;
  18 × 0.9889 = 17.80 and it draws 17.89 — one pair fits. Restating one title at six sizes
  gives 6.987, 9.889, 13.889, 17.890, 29.807, 39.808, whose deltas and ratios both vary. And
  the stated mechanism, "the OLE's stored visual area against the frame", cannot be it: the
  deck's frames are `cx="8046720" cy="4206240"` — 22352 × 11684 in 1/100 mm — and LibreOffice's
  own `odp` writes the chart as `svg:width="22.352cm" svg:height="11.684cm"`. Equal to the unit.
- **A secondary value axis' title has room reserved for it and is never drawn** — found while
  reading the title code. `PlotAreaOf` takes its height off the right edge and `AddTitles` adds
  only two titles. Nine chart parts over six documents across all three tracks.
- **A stated weight on axis labels, legend or data labels is still unread** — 36 of 61 chart
  parts over 7 documents, the larger half of the weight work. The design is decided and
  recorded in `Paperless.Presentations/TODO.md`: a stamping pass beside `InFamily`, not twenty
  more arguments.
## Words, round twenty-eight, at `9ef9b16fa` — the running heads a DOC does not write down

### The baseline reproduced the brief to the digit

Whole-track sweep against a checksummed CLI snapshot: **155/200, absolute page error 101,
165 exactly-correct page counts, absolute word error 7198**, and every per-batch figure the
brief carried. 001–005 10/10, 006 9/10, 007 10/10, 008 9/10, 009 10/10, 010 8/9, 011 9/10,
012 9/10, 013 6/9, 014 4/10, 015 5/10, 016 7/10, 017 6/10, 018 6/10, 019 4/10, 020 3/10,
021 0/2.

### The brief's first item does not exist, and the mechanism next to it does

The brief asked for the reach of a Word 97 header story **holding one paragraph mark** to be
measured before anything was written. Measured, over the header PLC of all 66 `.doc` files in
the track: **no story anywhere has a length of one.** Every story is of length nought or of
length two and up. So the case the brief describes — `grpfIhdt` keeping a bit that `Read_HdFt`
then declines to read — never arises on this corpus, and the fix it asked for would have moved
nothing at all.

What *is* there, at the same place, is three separate defects. The census that refuted the
first found all three, and the decisive reading for each was LibreOffice's own flat-ODF export
rather than its source:

- **`fFacingPages` was never read.** The `Dop`'s lowest bit (`ww8scan.cxx`:7643) decides two
  things: whether the even header and footer stories go into the synthesised `grpfIhdt`
  (`ww8par6.cxx`:1234), and whether the left page stops sharing the right page's head
  (`SetUseOn` adds `HeaderShare` exactly when the flag is *clear*, `ww8par.cxx`:4319). Nothing
  read it, so every DOC section had `HasDifferentEvenPages` false and its even stories, though
  read, could never reach a sheet. **Eight of the track's 66 `.doc` set it and all eight carry
  even stories.**
- **A story that exists and holds only empty paragraphs was dropped.** The comment defending
  that said "Word writes all six stories whether the section uses them or not, so most hold
  nothing but a paragraph mark"; the census says a slot the section does not use has a story of
  **no length at all**. Fourteen documents in the track hold a story of length two, which is one
  empty paragraph and its mark, and `A_320.doc` holds six of them in its first section.
- **"Cannot have left without right"** (`#i17196#`, `Read_HdFt`): every slot a section turns on
  sets the header on the *master* page format as well as on its own, so a section stating an
  even-page head alone still has an empty one on its odd pages — and a title page never borrows
  the master's, getting a blank of its own instead. `150_5300_13_chg8.doc` is the first case and
  `150_5300_13_chg12.doc` the second.

An empty running head draws nothing and is not nothing: it occupies the header band, and where
a section reserves none — `dyaTop == dyaHdrTop`, which is most sections of these documents — the
whole of it lands on the body. That is the brief's "14.15 pt offset", and `chg8`'s page one goes
from **709 one-sided `pdf-ops.py` records to 19**.

### A latent hang in round twenty-seven's balanced columns, found by exposing it

`150_5300_13_chg10.doc` rendered in ten seconds at the baseline and, with a running head
shortening its body, did not return at all — killed after six minutes and forty seconds, where
`batch-check.sh` recorded it as `ours-failed` and nothing said why. `dotnet-stack` put the
thread inside `Paginator.Fill` itself.

The bisection has two verdicts and only one of them can stop. `Fits` settles once the bounds
meet; **`TooShort` has nothing to settle**, because "too short" is never an answer on its own.
So a trial at the tallest band there is — the section's whole remaining height — that still
comes out too short hands back the same candidate for ever, restoring the fill to the section's
first block each time round. No page is emitted while that happens, so `MaxPages` never bites.
It is reachable whenever the last paragraph's space-after exceeds what is left of the body, and
it was reachable before this round: nothing about the running-head work created it.

### What the scoreboard did, said plainly

| | before | after |
|---|---:|---:|
| parity gate | 155 | **154** |
| absolute page error | 101 | **81** |
| exactly-correct page count | 165 | **164** |
| absolute word error | 7198 | **7075** |
| summed `\|ink\|%`, 154 common documents, 1570 pages | 720.07 | **692.83** |
| major pages, same 1570 | 343 | **333** |

**The gate lost a document and everything continuous improved.** The invariant holds on both
sides: 720.07 ≥ 548.14 signed, 692.83 ≥ 535.15.

The one lost match is `150_5300_13_chg12.doc`, 33 pages against 33 and now 34 — and its text is
materially closer, which the token multiset says and `wc -w` cannot:

| document | before | after |
|---|---|---|
| `150_5300_13_chg12.doc` | 614 (over 334, under 280) | **501 (over 305, under 196)** |
| `A_320.doc` | 263 (over 31, under 232) | **109 (over 31, under 78)** |
| `150_5300_13_chg8.doc` | 693 (over 384, under 309) | **679 (over 382, under 297)** |
| `150_5300_13_chg10.doc` | 2438 (over 1657, under 781) | 2439 (over 1670, under 769) |
| `762.doc` | 37 (over 9, under 28) | 54 (over 27, under 27) |

`chg12`'s under-draw falls by 30% and `A_320`'s by two thirds. The three `150_5300_13_*`
figures reproduce round twenty-seven's own to the digit, which is the corroboration that the
instrument agrees with itself across rounds.

The page-count movement is where the round is won: **`A_320.doc` 119 → 141 against 150** (its
first section states six stories of length two and reserves no header band, so every page of it
was a running head and a running foot too tall), `762.doc` 21 → 22 against 23, `chg8` 23 → 22
against 18, against `chg10` 80 → 83 and `chg12` 33 → 34 the other way. `762.doc` also reaches
the reference's word count **exactly**, 4142 against 4142, as does `FMRBullletinB-28.doc`.

### Reach, measured by rendering

Both sweeps' PDFs byte-compared with `SOURCE_DATE_EPOCH` pinned: **34 of 200 documents changed,
all of them `.doc`** — half of the track's 66. The gate sees 16 of those 34 and the image diff
is what accounts for the rest. Reference PDFs are *not* byte-comparable between runs, because
`soffice` stamps its own `/CreationDate` and honours nothing; their page and word columns are
identical row for row across the two sweeps, which is the check that says the reference held
still.

### Verified by putting five defects back

| defect put back | caught by |
|---|---|
| `fFacingPages` never read | `AnEvenHeaderIsDrawnOnEvenPagesOnly(".doc")` |
| a story's empty paragraphs dropped | `ABlankRunningHeadStillTakesItsLine` |
| no `#i17196#` completion at all | `AnEvenOnlyHeaderStillGivesTheOddPagesABlankOne`, `ATitlePageWithNoHeadOfItsOwnDrawsNoRunningHead` |
| a title page borrows the master's head | `ATitlePageWithNoHeadOfItsOwnDrawsNoRunningHead` |
| the balance search retries the same band | `ASectionOverhangingEveryBandStillEndsTheSearch` (its thirty-second timeout) |

The middle two were caught by **nothing** on the first attempt, and the reason is worth
recording because it decides how the fixtures had to be made: **no exporter writes a header
story of length nought.** LibreOffice's own DOC filter writes one paragraph mark for a slot it
does not use — length two, which reads as a blank head by the ordinary route — so on any
document `soffice` can produce, the two rules are indistinguishable. Only Word writes an empty
story, which is what `chg8` and `chg12` do.

So `blank-odd-head.doc` and `title-page-no-head.doc` are hand-edited: their `PlcfHdd` has one
story's length struck to nought, the CP going to the story before it. Both then round-trip
through `soffice` to a flat ODF that states the expected answer —
`<style:header><text:p text:style-name="Header"/></style:header>` beside a populated
`<style:header-left>` for the first, and `<style:header-first><text:p/></style:header-first>`
beside a populated `<style:header>` for the second — and that export is what the tests assert
against. `facing-pages-even-header.doc`/`.docx` and `blank-running-head.doc`/`.docx` are
ordinary `soffice` conversions and need no such note.

One existing assertion changed rather than being deleted: `TheCorpusFixtureLeavesItsTitlePageBare`
covered `title-page-header.doc`, whose first-page header story is length two. LibreOffice writes
`<style:header-first><text:p/></style:header-first>` for it, so the header exists and is blank;
the old assertion described our model rather than the reference's. Its replacement asserts the
header exists, draws nothing, and moves the body nowhere — the band there is 14.15 pt against a
13.80 pt line, so nothing does move, which is what makes it a model change and not a rendering
one.

### Test counts

Core 249, Containers 109, Text 240, Vector 291, Rendering 119, Markup 259, OpenDocument 125,
**WordProcessing 690** (683 + 8 new − 1 replaced), Spreadsheets 498, Presentations 529,
Fidelity 550, 0 skipped throughout, with the tree rebuilt after the last reintroduced defect
came back out.

### What the next round should take

1. **`150_5300_13_chg12.doc`'s extra page**, which is the round's one lost match and the only
   adverse gate movement. Its page one now matches the reference; its page **two** carries the
   even running head where the reference draws none, and the reference's page two prints `i`
   in its footer where ours prints `31` — so the divergence is a section boundary or a page-number
   restart rather than the furniture, and it should be diagnosed as one.
2. **Unequal column widths**, untouched this round and still the model gap the brief named:
   `chg8` states `style:column style:rel-width="4680*"` and `"5112*"`, which `PageGeometry`
   cannot hold. Note the census: **zero DOCX in the track declare `w:cols w:equalWidth="0"`**,
   so this is a `.doc` shape only and its ceiling is small.
3. **The blank page `chg8` emits** between its title page and its first chapter.
4. **The even page with no even story.** `Read_HdFt` gives the left page a head only on the even
   iteration itself, so a facing-pages section with no even story has **none** on its even pages;
   our furniture dictionary reads an absent slot as "fall back to Default" and cannot say it.
   Not implemented, not measured, and it needs a slot value meaning "nothing" rather than a
   missing key.
## Round twenty-seven — sheets: a BIFF chart's font and its fills

Base `e4b9cf255`, verified. Probe data, censuses and the mutation runs in
`dotnet/probes/sheets-r27/`.

| sheets, 171 documents | base `e4b9cf255` | after |
|---|---:|---:|
| word gate | 145 | **146** |
| abs page error | 90 | 91 |
| exact page counts | 154 | 154 |
| abs word error | 43203 | **42848** |

`sheets/batch-010` goes **7/10 → 8/10**: `EHEST-Pre-departure-checklist…xls` matches at
8373/8382 words against 8018 before, and its embedded-font count goes 4/6 to 5/6. Every other
batch holds its count; 001–009 are still 89/89.

**The page error moving 90 → 91 is the reference, not us.** The one row is
`ans_mappings_of_eccairs_terms.xlsx`, whose own column is 192 pages and 28195 words in both
sweeps while the reference's is 191/28183 and then 190/28181. Both reach runs report it
byte-identical across every CLI in the round.

### Two reader gaps, both named by a real record parse rather than a byte search

- **`CHFONT` was never read**, so a chart stating Calibri was measured and drawn in Liberation
  Sans. Reach measured by rendering: **2 of 171**, and they are exactly the two the census
  predicted. A record-level census of all 61 OLE2 workbooks finds **6 holding a chart substream**,
  every one stating a single family throughout — and **four of the six state Arial, which resolves
  to Liberation Sans and so cannot move**.
- **The BIFF chart path drew no fills at all** — 0 operators against the reference's 7 on `EHEST`
  page 8, where the OOXML path emits 20 against 21 on a comparable page. `CHAREAFORMAT`,
  `CHLINEFORMAT` and the `CHESCHERFORMAT` that overrides them are now read. Reach: **4 of 171**
  against a census ceiling of 6; the two that did not move hold their charts on sheets the printout
  never reaches, and neither renderer draws a fill anywhere in either.

**Reading only the palette would have looked right.** All nine of `EHEST`'s charts state their
three filled series at palette indices 24, 10 and 13 — `#9999FF`, `#FF0000`, `#FFFF00` — and the
reference draws `#6699FF`, `#FF0000`, `#FFFF00`: two agree by palette and the first is the Escher
override, as is the `#F8F8F8` plot wall. `XclImpChFrameBase::ConvertAreaBase` states the precedence
outright in a comment.

### A `mv` that restores a backup also restores its modification time

The seam check — sweep snapshot against the tree — reported three documents differing, which reads
as a stale snapshot and was the opposite. The reference settles it: on `TOGAF9` page 21 it fills
`(52.84, 318.36)-(613.70, 567.38)`, the snapshot matches and the tree did not.

The mutation cycle is the cause, one step past the trap already recorded here. Restoring a patched
file with `mv backup source` **keeps the backup's older mtime**, so MSBuild sees a source older than
the assembly and skips the rebuild — and a plain `dotnet build` then prints success over a binary
that still contains the defect. The recorded trap is that `--no-build` measures the defect; add that
`dotnet build` does too, and `touch` the file before the run whose numbers you will report.

### Left alone, and said so

Automatic chart colours (the census finds **zero** automatic areas on the track and six automatic
lines, all in the one document the brief says not to chase); `CHDATERANGE`; and a chart's text
*weight* — the reference embeds Carlito-Bold on `EHEST` for an 18 pt chart title and `ChartPlot`
carries a family and no weight, which is a `Paperless.Core/Charts` widening reaching all three
consumers rather than this track's alone.

## Round twenty-nine: slides — the band under the plot area, and the text that was turned the wrong way

Baseline measured first at `281685d90` and it reproduces the brief **exactly**: `MATCH 151/163`,
`ink%` 1271.64, `|ink|%` 1592.18, 427 major pages, 4199 census pages with **303 unexplained over
91 documents**.

The brief asked for the plot area's bottom edge and predicted the metric would turn. **The
prediction is untested, because the term that dominates that edge was not fixed.** What was
fixed is four smaller things found on the way to it, and the honest headline is that they move
`|ink|%` by half a point and move no verdict at all.

### What the probes settled about the bottom band

`chart-face-theme-minor.pptx` states its plot area's own fill, so the rectangle under study is a
filled box in both renderings and can be read off the PDF rather than inferred. Eleven variants
of it, one element of the bottom stack at a time
(`research/probes/slides-r29/make-band-probe.py`), our band against the reference's:

| probe | before | after |
|---|---:|---:|
| labels off (`tickLblPos="none"`) | 12.79 | **0.03** |
| upright labels, legend, both axis titles | −0.17 … 0.43 | unchanged |
| 26 categories, labels rotated 45° | 7.94 | **7.94** |

**LibreOffice's own answer is readable without inferring anything**, and this is the instrument
the next round should start from: `chart:coordinate-region` in its `odp` export carries the
computed plot rectangle *excluding axes*
(`SchXMLExportHelper_Impl::exportCoordinateRegion`, `xmloff/source/chart/SchXMLExport.cxx:2274`),
and on `Demick_JetBlue.pptx` it agrees with the drawn gridlines to 2 pt on all five charts.

### Four fixes

- **`c:tickLblPos val="none"` was unread.** It is not `c:delete`: the importer maps it to
  chart2's `DisplayLabels` (`axisconverter.cxx:221`), which hides the labels and leaves the axis
  line and its ticks. We drew the labels and reserved a line for them. ODF states the same
  property as `chart:display-label`.
- **The bottom edge gives up half a value label.** The bottommost value label is centred on the
  plot area's bottom-left corner exactly as the topmost is on its top-left, and it shares the
  strip with the category band rather than stacking under it — what LibreOffice reserves is the
  bounding box of everything its axes drew (`VDiagram::adjustInnerSize`), not a sum of parts.
  With the category labels off the reference's edge sits 5.65 pt below the plot against half a
  label's 5.67; adding the two instead puts it 4.25 pt low.
- **Every rotated piece of chart text was turned clockwise where LibreOffice turns it
  anticlockwise.** `ChartLabel.Rotation` is anticlockwise — both formats state it that way and
  chart2 carries it that way — and the drawing space has y growing downwards, so handing the
  angle straight to `AffineTransform.Rotation` reverses it. A probe whose axis title reads
  `Alpha Omega` came out top-to-bottom against the reference's bottom-to-top; `Demick_JetBlue`'s
  45° category labels descended to the right against the reference's ascending. Fixed at all
  three consumers, because the sign is wrong where the model meets the drawing space.
- **A secondary value axis' title had room reserved and was never drawn**, and **the category
  axis' title was drawn on top of a bottom legend.** The second is `lcl_createTitle` placing an
  `ALIGN_BOTTOM` title inside `rRemainingSpace` (`ChartView.cxx:1147-1149`) after the legend has
  come out of it (`:1966` against `:2054`); measuring from the frame instead put ours 30.30 pt
  below the reference's with a legend and 8.13 pt below without, and it now sits 6.5 pt above in
  both. **That 6.5 pt residual is unexplained and open** — the same with and without a legend,
  within 0.35 pt of the chart's two per cent bottom margin, which does not survive reading
  `createShapes2D:941-944`.

### The numbers, said plainly

| slides, 163 documents | base `281685d90` | +tick labels | +the other three | +all four |
|---|---:|---:|---:|---:|
| word gate | 151 | 151 | 151 | 151 |
| `ink%` | 1271.64 | 1271.64 | 1270.91 | **1268.44** |
| `\|ink\|%` | 1592.18 | 1592.17 | 1591.65 | **1589.24** |
| major pages | 427 | 427 | 427 | 427 |
| census, unexplained | 303 over 91 | 303 over 91 | 303 over 91 | 303 over 91 |

Verdicts changed: **0**. Every batch holds its count.

**Reach measured by rendering.** The tick-label read alone changed **one** of 163 documents —
and not one of the three the census names. Seven chart parts over three decks state
`tickLblPos="none"` and in *every one* the axis is also `c:delete val="1"`, or is a second `b`
axis the reader does not pair; the one document that moved,
`southern-classic-kennesaw-state-university-final.pptx`, moved on the half-value-label rule
instead, its charts deleting the category axis outright. **The distinction is real and the
corpus does not exercise it.** The other three fixes took the changed set to 8 of 163, and the fourth
kept it at the same 8. Over those eight, net `|ink|%`:

| document | base | +3 | +4 |
|---|---:|---:|---:|
| `Demick_JetBlue.pptx` | 39.24 | 38.99 | **36.55** |
| `171128IPAP.pptx` | 16.29 | 15.95 | 15.95 |
| `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 17.25 | 17.23 | 17.21 |
| the other five | | | +0.13 between them |
| **net** | | −0.53 | **−2.94** |

No major-page count moved on any of them and no verdict changed. **`Demick_JetBlue` alone is
−2.69 of the −2.94**, and it is the deck round twenty-four's work made worse by +10.31 — so the
band under its plot area is where its remaining error is, exactly as that round predicted, and
two thirds of that deck's regression is now back.

### Cross-track

The rotation sign, the secondary title and the bottom title all live below the family libraries
and reach `SheetChart` and `FrameChart` — unlike a weight, which those two document in code that
they drop. Rendered at both commits with `SOURCE_DATE_EPOCH` set and byte-compared over all 371
words and sheets documents: **5 changed** — four `.xls`/`.xlsx` and one DOCX, which is every
document either track holds with a chart on it. Against `soffice`'s own PDF of each, `|ink|%`
moves +0.03, +0.02, +0.37, −0.22 and 0.00, **net +0.20**, with every major-page count unchanged
and no verdict moved. The +0.37 is `Template Pilot Logbook JAR-FCL V3.0.xls`, which the first
three fixes did not touch at all and which the bottom title's placement moves — 0.37 of 26.99 on
a document already failing by a wide margin. Recorded rather than reverted: the placement is
verbatim from `lcl_createTitle` and its own probe error went 30.30 → 6.53.

### A trap walked into with the skill open

The verification cycle puts each defect back and the `git checkout --` that takes it out again
**discards the file rather than the patch**, so an uncommitted fix in that file goes with it.
That is written down in `corpus-batches` and it still cost a commit here: the bottom-title
change was wiped and its commit carries only the test. It was caught by the per-project run at
the end — `Paperless.Core.Tests` 263 of 264 on a tree whose source read as finished — which is
the argument for running that even when nothing looks wrong.

### What the next round should take, in order

1. **`PlotAreaOf` should be `VDiagram::adjustInnerSize`, not a reservation per edge.**
   LibreOffice lays the diagram out at full size, measures the bounding box of what the axes
   drew, and shrinks the inner rectangle by the overflow — twice, with a floor of a third of the
   available rectangle. The residue on `Demick_JetBlue`'s five chart pages is 12.73, 38.72, 4.88,
   12.73 and 4.87 pt of bottom edge, and the split is exactly rotated against upright labels.
2. **A stated weight on axis labels, legend or data labels** — 36 of 61 chart parts over 7
   documents, unchanged from last round, design still recorded in
   `Paperless.Presentations/TODO.md`.
3. **The legend row pitch's rate**, still the open half of the 96 dpi grid.

## Round thirty — sheets: an `.xls` cell's margin is twice everything else's

Base `946b3defc`, verified before measuring. Probe data, both sweeps and the mutation run in
`dotnet/probes/sheets-r30/`.

| sheets, 171 documents | base `946b3defc` | after |
|---|---:|---:|
| word gate | 146 | **147** |
| abs page error | 90 | 90 |
| exact page counts | 154 | 154 |
| abs word error | 42859 | **42322** |

`sheets/batch-016` goes **4/9 → 5/9**:
`underlying-holdings-…-state-street-emu-esg-screened-index-equity-fund.xls` matches at 4988/4991
words against 4743 before. Every other batch holds its count; 001–009 are still 89/89 and no page
count moves anywhere on the track.

The baseline reproduces the brief in every figure and every per-batch figure except the word error,
which is 42859 here against a briefed 42848 — one document, `Keywords_Mapping_Graphs_and_Charts.xlsx`,
whose verdict is unchanged. **`ECA Sinters.xls` converted on the first pass in both sweeps**, after
three rounds of needing to be spliced.

### `ATTR_MARGIN` is a cell attribute and was a shared constant

`XclImpXF::CreatePattern` ends by putting `SvxMarginItem(40, 40, 40, 40, ATTR_MARGIN)` on **every
pattern the BIFF filter builds**, under the comment "Excel's cell margins are different from Calc's
default margins" (`sc/source/filter/excel/xistyle.cxx:1349-1351`). It is unconditional and it is the
only line in all of `sc/source/filter` that touches the item, so an `.xls` has twice the margin of
every other format and `Paperless` had one constant for all three readers.

**It had been read as a page-origin offset**, and that reading is written into `pdf-ops.py`'s own
tolerance comment. A page origin moves every run the same way; a margin moves left-aligned text
right and right-aligned text left, and the corpus document above does the second. The
`sheet-cell-text` fixture triple then settles it without the corpus: LibreOffice puts the
left-aligned A1 at 58.68 pt in the `.xls` and 57.69 in the `.xlsx` and `.fods`.

The same `nTotalMargin` decides how much of a clipped string survives, which is the half that moves
a word count: a 49-character label in a 49.75 pt column kept `State Stree` here and `State Stre`
there, on every row of five pages.

### Reach, and why no page moved

The gate registers a difference on **35 of 171 rows**, 34 of them `.xls`. A byte-level reach run was
stopped after 35 documents under load and its partial is unambiguous: **15 of 15 `.xls` differ and
20 of 20 `.xlsx` are identical**, so the reach is the track's 62 `.xls` and nothing else.

No page count moves, and that is structural rather than lucky: `RowHeightsAreManual` is set outright
for BIFF8, so no `.xls` row height is ever recomputed and the margin cannot reach the quantity
pagination turns on. Two of the eight reintroduced defects were **not** detected by any test and the
probe README says which and why.

**Every test project was run individually on the final tree and matches its known-good count,
with zero skipped**: Core 264, Containers 109, Text 240, Vector 291, Rendering 119, Markup 259,
OpenDocument 125, WordProcessing 690, Spreadsheets **573** (566 plus this round's seven),
Presentations 538, Fidelity 550. The fidelity suite took an hour and three quarters to return at a
load average around 20 and was written down as unrun before it did — kept as a note in the probe
README, because a fidelity count taken under that load is the one that can be silently truncated.
## Words, round thirty, at `946b3defc` — a running head that reaches past a continuous section

### The baseline reproduced the brief to the digit

Whole-track sweep against a checksummed CLI snapshot, `SOURCE_DATE_EPOCH` pinned:
**154/200, absolute page error 81, 164 exactly-correct page counts, absolute word error
7075.** 200 rows, no path twice.

### `Read_HdFt` inherits from the previous section's *page descriptor*, and a continuous section has none

`Read_HdFt` copies an empty header story from `pPrevious->mpPage` — the immediately preceding
segment's page descriptor (`ww8par.cxx`:2567, `SetSwFormatPageDesc` at 4381). A continuous
section never gets one: `InsertSegments` turns it into a Writer *text* section instead
(`ww8par.cxx`:4422). So the `else if (pPrev)` arm never runs, and the slot keeps the empty
format `SwFormatHeader(true)` left there a few lines above. **The running head stops** rather
than reaching back past the continuous section to the last section that had a page of its own.

We carried one dictionary of slots forward across every section unconditionally, so a head set
in section 0 reached every later section that stated no story of its own. On
`150_5300_13_chg12.doc` that put `9/29/06 AC 150/5300-13 CHG 10` across the top of thirteen
pages LibreOffice leaves bare, and the wrong even head on several more.

Three pieces, all from the same reading:

- The six `grpfIhdt` bits are now synthesised as `InsertSegments` does — odd and first always,
  even under `fFacingPages`, and a bit whose story is empty survives only where the *previous*
  section had it on (`ww8par6.cxx`:1237-1258). `Complete`'s "which slots did this section turn
  on" now reads those bits instead of guessing from what landed in the dictionary.
- A slot is copied only out of the immediately preceding section's page descriptor.
- `if (bUseLeft) pPD->GetLeft().SetFormatAttr(...)` runs whether or not the story could be read,
  so an even slot whose bit is on and whose story is empty and uninheritable is a **blank** —
  not, as this dictionary had it, an absent slot falling back to the master's head.

The first attempt short-circuited *every* continuous section to the set in force, and that is
one case too many: `HasOwnHeaderFooter` (`ww8par.cxx`:4528) makes `InsertSegments` build a
descriptor for a continuous section that states a not-first slot of its own.
`PK_FlugzeugeStricken.doc` — second section continuous, 55-character odd header, 39-character
odd footer — went **7/7 match to 8/7, +90 words**, and was the whole of that sweep's cost. A
continuous section now computes its own furniture whenever it has any; either way it does not
become a copy source for the section after it, because the descriptor is thrown away again
unless the section holds a hard page break (`#i40766#`).

### Measured, and the census matched the reach exactly for once

Reach by byte-comparing 200 renderings before and against after, clock pinned: **4 of 200
differ**, and they are precisely the four the census predicted. The census — over the header
PLC and section descriptors of all 66 `.doc`; the other 134 are DOCX, where a section names its
headers explicitly and the case cannot arise — said the slot-blanking case reaches
`150_5300_13_chg8`, `absrc-pac-01-info-note-en`, `150_5300_13_chg10` and `150_5300_13_chg12`.
This is the first census on this track that has not over-stated; do not read it as the rule.

| | before | after | reference |
|---|---:|---:|---:|
| `150_5300_13_chg8.doc` | 22 pages, 8652 words | **21**, **8584** | 18, 8557 |
| `150_5300_13_chg12.doc` | 34, 12864 | 34, **12769** | 33, 12750 |
| `150_5300_13_chg10.doc` | 83, 24367 | 83, **24184** | 76, 23456 |
| `absrc-pac-01-info-note-en.doc` | 6, 1325 | 6, **1320** | 7, 1322 |

Whole track: **154/200 match (unchanged), absolute page error 81 → 80, 164 exact page counts
(unchanged), absolute word error 7075 → 6728.** Nothing else on the track moved a byte.

Both halves of the word move, over those four documents: **over-draw 2380 → 2063, under-draw
1282 → 1316.** That is the signature of a running head we should never have drawn going away —
the small rise in under-draw is reflow, not lost text.

Per batch at the end: 001–005 10/10, 006 9/10, 007 10/10, 008 9/10, 009 10/10, 010 8/9,
011 9/10, 012 9/10, 013 6/9, 014 4/10, 015 5/10, 016 7/10, 017 6/10, 018 6/10, 019 4/10,
020 2/10, 021 0/2.

Per-project tests, all matching the known-good counts with 0 skipped: Core 264, Containers 109,
Text 240, Vector 291, Rendering 119, Markup 259, OpenDocument 125, WordProcessing 690,
Spreadsheets 566, Presentations 538, Fidelity 550.

### `chg12`'s extra page is an odd-page break meeting a page-number restart

Diagnosed, **not fixed and not measured beyond this document.** Our page 8 now holds nothing but
a running head; the reference has no such page. It is the filler our paginator emits before
section 8, which is a **one-character section** (CP 20746 to 20747, a lone paragraph mark) with
`bkc == 4`, an odd-page break, and a page-number restart at 19.

`SwFrame::InsertNewPage` (`pagechg.cxx`:1581-1612) decides the side the new page *wants* from
the restart value when the section carries one — `bWishedRightPage = sw::IsRightPageByNumber(*pRoot, *oNumOffset)`
— and only otherwise from the physical alternation. Our filler loop tests the parity of the
**old** section's numbering: `pageNumber = geometry.RestartPageNumberAt ?? pageNumber` runs
*after* the loop, not before it (`Paginator.cs`:731-766). Two pages of `chg12` are of this shape
and one of them is the whole of its +1.

I did not settle the arithmetic — reconstructing it from `IsRightPageByNumber`'s comparison
against the layout's *first* virtual page number predicts a filler where the reference has none,
so either that first number is not 1 or a second rule is in play. Anyone taking this should
measure before writing, and should expect the change to reach DOCX `w:type="oddPage"` as well.

### Refuted

- **`chg8` does not emit a blank page.** The brief's third item: all 21 of its pages carry body
  text at `946b3defc` and after. Whatever motivated that entry is gone or was never `chg8`.
- **"The even page with no even story" is a three-document ceiling.** Measured over the header
  PLC of all 66 `.doc`: the case where the even bit is *off* while a header is on reaches
  **5 sections of 3 documents** — `150_5335_5a`, `chg10`, `chg12` — and the first of those three
  already matches. Our dictionary still cannot say "this page has no header at all"; the near
  case, where the bit is *on* and the story is empty, is now a blank and is fixed. Weigh the
  remainder against that ceiling before building the sentinel it needs.
- **Unequal column widths were not touched**, and the brief's own census — zero DOCX declaring
  `w:cols w:equalWidth="0"` — stands as the reason.

### No unit test, and why

The behaviour needs a `.doc` whose later section states an *empty* header story while an earlier
one states a real head, with a continuous section between them. LibreOffice's own DOC export
writes a resolved head into every section it emits, so no fixture generated through it can carry
the case, and none in the tree does. The claim rests on the corpus measurement above and on
LibreOffice's flat-ODF export of `chg12`, whose `Convert 1`, `Convert 3` and `Convert 5` master
pages carry an **empty** `style:header` beside a populated `style:header-left` — exactly what
the rule predicts and the opposite of what we drew.

`probes/ww8-header-stories.py` is committed: it reads the FIB, `PlcfHdd`, the piece table and
`PlcfSed` straight out of a `.doc` and prints each section's six story lengths, their text, the
synthesised `grpfIhdt` and the break kind. Every census in this entry came from it.

## Words, round thirty-one, at `150a3dac1` — the blank an odd-page break leaves is never drawn

### The baseline reproduced the brief to the digit, and every batch figure with it

Whole-track sweep against a checksummed CLI snapshot, `SOURCE_DATE_EPOCH` pinned:
**154/200, absolute page error 80, 164 exactly-correct page counts, absolute word error
6728.** 200 rows, no path twice. All twenty-one per-batch figures reproduce the scoreboard.

### PDF export drops the page an odd- or even-page break inserts

The predecessor left this "diagnosed and explicitly unmeasured", with a caution that
reconstructing `IsRightPageByNumber`'s arithmetic predicts a filler where the reference has
none. **That caution was the finding.** The filler is not predicted wrongly — it is predicted
correctly and then *not exported*.

Writer states an odd- or even-page break as a page style whose `UseOn` names one side only
(`ww8par.cxx`:4470-4479 for `sprmSBkc` 3 and 4, `PropertyMap.cxx`:1568 for
`w:type="oddPage"`), so `SwFrame::InsertPage` takes `rDoc.GetEmptyPageFormat()` when the side
the style wants and the side the alternation would give disagree (`pagechg.cxx`:1613-1616).
An *automatically inserted* empty page is then skipped by PDF export:
`SwPrintUIOptions::IsPrintEmptyPages` reads `IsSkipEmptyPages` for an export and its default
is true (`printdata.cxx`:391-399). The blank is laid out, takes a page number, and is not in
the file. We were drawing it.

**And the side is physical, not the printed number.** The style leaves one of
`GetRightFormat`/`GetLeftFormat` null, so `InsertPage`'s format-availability flip overrides
the `SetNumOffset` reading three lines above it, and `OnRightPage()` is `GetPhyPageNum() % 2`
(`frame.hxx`:757). Our loop tested `pageNumber`, which a section restart moves.

Measured on the installed 24.2 rather than read off the source, with a probe generator now
committed at `probes/odd-page-break-probes.py`. Each probe's paragraphs print their own
`PAGE` field, which is what makes an undrawn page visible at all:

| Probe | LibreOffice's PDF |
|---|---|
| two sections, second `w:type="oddPage"` | **2 pages**, second reads **3** |
| the same with page one numbered 2 | **2 pages**, second reads **4** |
| three sections, last `oddPage` | 3 pages, reading 1, 3, 4 |

The second row is the one that separates the two rules: the next *number* is three and already
odd while the next *physical* page is the second and even, and LibreOffice still leaves a blank.

### A page-number restart picks a side too, by the same machinery

Falling out of the same reading and measured the same way. When a section restarts its
numbering and states no odd/even break, `InsertPage` takes the wished side from the restart
value through `sw::IsRightPageByNumber` (`pagechg.cxx`:1590-1596), which asks only whether the
restart's parity agrees with the **first page of the layout's own number**
(`frmtool.cxx`:3146-3153) — and inserts the same undrawn blank when that disagrees with the
alternation. Three probes differing only in the restart value, each a three-section document
whose last section breaks to an odd page:

| Restart on the middle section | the three `PAGE` fields | pages exported |
|---|---|---:|
| none | 1, 2, 3 | 3 |
| 19 | 1, 19, **21** | 3 |
| 20 | 1, 20, 21 | 3 |

The 21 in the middle row is two skipped blanks — one putting an odd restart on an odd sheet,
one for the odd-page break that then lands on an even one.

### Measured, and both censuses over-stated

Reach by byte-comparing 200 renderings, clock pinned, at each step:

| Change | census ceiling | documents whose rendering changed |
|---|---:|---:|
| the blank is not drawn, and the side is physical | 6 | **3** |
| a restart picks a side | 50 | **1** |
| a column break that stayed one has no side | 0 | **0** |

The first census is over the section descriptors of all 66 `.doc` (5 carry `sprmSBkc` 3 or 4)
and the `w:sectPr` of all 134 DOCX (**one** declares `w:type="oddPage"`, none `evenPage`) — so
it covers the whole track, and it over-stated two-fold. The second is over `sprmSFPgnRestart`
in the same 66 (13) and `w:pgNumType/@w:start` in the same 134 (37), and it over-stated
**fifty-fold**. Last round's exact census remains the exception it was labelled as.

The third change is a no-op and is recorded as one: no `.doc` section in the track carries
`sprmSBkc` 1 together with `sprmSFPgnRestart`, no DOCX `sectPr` declares `w:type="nextColumn"`,
and rendering all 200 with and without it is byte-identical.

| | before | after | reference |
|---|---:|---:|---:|
| `150_5300_13_chg8.doc` | 21 pages, 8584 words | **20**, **8577** | 18, 8557 |
| `150_5300_13_chg10.doc` | 83, 24184 | **80**, **24162** | 76, 23456 |
| `150_5300_13_chg12.doc` | 34, 12769 | **32**, **12764** | 33, 12750 |

Whole track: **154/200 match (unchanged), absolute page error 80 → 76, 164 exact page counts
(unchanged), absolute word error 6728 → 6694.** Nothing else on the track moved a byte, no
document changed verdict, and no batch figure moved. **The scoreboard did not move**, and that
is the headline: three documents came closer and none of them came close enough to flip.

Both halves of the word move over the three changed documents: **over-draw 2055 → 2031,
under-draw 1323 → 1333.** That is a running head on a page that should not exist going away,
at ten tokens of reflow.

Per batch, before and after alike: 001–005 10/10, 006 9/10, 007 10/10, 008 9/10, 009 10/10,
010 8/9, 011 9/10, 012 9/10, 013 6/9, 014 4/10, 015 5/10, 016 7/10, 017 6/10, 018 6/10,
019 4/10, 020 2/10, 021 0/2.

Per-project tests, all matching the known-good counts with 0 skipped, except WordProcessing
which is 690 + the six new: Core 264, Containers 109, Text 240, Vector 291, Rendering 119,
Markup 259, OpenDocument 125, WordProcessing **696**, Spreadsheets 573, Presentations 538,
Fidelity 550.

### The tests exist this time, and both halves were watched failing

`OddPageBreakFillerTests` — three facts about the odd-page blank and a three-case theory about
the restart. Verified through `verify-test.sh` by putting each half of the defect back:
reading `pageNumber` instead of the physical page fails
`TheSideIsThePhysicalPagesNotThePrintedNumbers` alone; drawing the blank instead of skipping it
fails all three. The case is testable here where last round's was not, because a DOCX states
`w:type="oddPage"` in markup a fixture can carry.

### A lead for whoever takes batch 006: a row's declared floor is lost when the row is split

`f445896eb008d14c1746fc37d412dc22.docx` is the batch's only failure, **15 pages against 16 with
the words exactly equal at 5575**. It is one 30-row table, every row carrying a `w:trHeight`
with no `w:hRule` — so thirty "at least" floors — and none carrying `w:cantSplit`.

The cause is those floors, established by mutating the document itself rather than by a probe:

| The document | LibreOffice |
|---|---:|
| as found | 16 pages, page 3 holding 338 words |
| every `w:trHeight` reduced to 80 twips | **15 pages, page 3 holding 439 words** |
| only the four `w:trHeight="13800"` reduced | 16 pages, unchanged |
| only row 3's 2921 reduced to 100 | 16 pages, unchanged |

Neutralising all thirty floors reproduces **our** layout almost exactly — 15 pages against our
15, 439 words on page 3 against our 434 — so the floors are what we are losing, and no single
one of them is the whole of it. Ours is the layout of a document that states no minimum heights
at all.

Where it goes: `TableLayouter.LayOut` does apply the floor
(`Length.Max(heights[row], table.Rows[row].MinHeight)`), and an unsplit row is right on both
sides. But `TableLayouter.SliceRow` measures a split part purely from its cells' line bottoms
and never consults `PageTableRow.MinHeight`, so a row whose height comes from its floor rather
than from its content collapses to its content the moment it crosses a page. This table's rows
are mostly taller than a page's remainder, so most of them are split.

Two cautions for whoever takes it. Our page top is 5.1-5.7 pt below the reference's on page 3,
which is a separate small thing and not the 122 pt of row this is about. And **a hand-built
DOCX with no `word/styles.xml` is useless for measuring a height**: two probes I built that way
laid out in Liberation Serif 10 pt here against Carlito 11 pt there, a probe artefact that made
their row-splitting comparison meaningless. Mutating the corpus document is what settled it.
The page-count and `PAGE`-field probes above are unaffected -- one short paragraph per page
cannot change a page count -- but anything measuring a height needs a styles part.

### Still open, unchanged and still unmeasured

- **`chg12` is now 32 against 33 rather than 34 against 33.** The sign flipped and the absolute
  error did not. Its remaining defect is an *under*-pagination in the pages 9-10 region, where
  the reference splits content across two pages that we fit on one — a different bug from the
  one this round closed, and the filler was masking it.
- **The even page with no even story** — the three-document ceiling from last round, untouched.
- **Unequal column widths** — untouched; zero DOCX declare `w:cols w:equalWidth="0"`.
## Round thirty: slides — asking LibreOffice for the plot rectangle instead of measuring the ink

Baseline measured first at `946b3defc`, against the kept reference set with two of its 163 PDFs
re-converted by a fresh `soffice` first — `wells08_basic.ppt` 27 pages/1013 words and
`Demick_JetBlue.pptx` 10/617, both page-, word- and pixel-identical to the kept copy, which is
the seventh time that set has been checked. It reproduces the brief **exactly**: `MATCH 151/163`,
`ink%` 1268.44, `|ink|%` 1589.24, 427 major pages, 4199 census pages with **303 unexplained over
91 documents**.

### The instrument the brief asked for, and the two readings it needed before it meant anything

`chart:coordinate-region` in an `odp` export is `calculateDiagramPositionExcludingAxes`
(`xmloff/source/chart/SchXMLExport.cxx`:2274), so converting a deck to `.odp` states the plot
rectangle LibreOffice computed for **every** chart on it, including the ones that draw no
gridline. `research/probes/slides-r30/region.py` reads it and `compare-region.py` joins it to the
rectangle read off either PDF's ink, so the model is checked against LibreOffice's own ink before
it is trusted — on `Demick_JetBlue` the two agree to 0.66–1.00 pt on all five charts.

Two reader faults had to be fixed first, and **both looked like enormous defects rather than like
instrument faults**:

- Round twenty-nine's `plotrect.py` takes the *modal* stroke span, which on any axis carrying
  more ticks than gridlines returns the tick's own 4 pt span — and it needs a grid at all, which
  **47 of the 61 chart parts do not draw**. It now finds the two axis lines by the corner they
  share.
- A slide carrying four charts draws four plot rectangles on one page, and the reader returned
  the biggest for all four. Restricting it to each chart's own frame took `southern-classic`'s
  reported left error from **2161 pt to 25.64**.

At the base commit: 37 of 61 charts measurable, mean per-edge error **2.83 pt**, total `|error|`
418.77 pt of which bottom 208.65, left 103.45, right 95.47, top 11.20.

### The brief's first item is refuted in its stated form

**`PlotAreaOf`'s per-edge reservation *is* what `adjustInnerSize` computes.** Working the C++
through: the new inner rectangle is the available one shrunk on each edge by how far the consumed
bounding box overhangs the old inner rectangle on that edge, and the loop runs to a fixpoint
(`ChartView.cxx`:593-621 calls it three times, and the `bLessSpaceConsumedThanExpected` guard is
true for every chart because its last disjunct compares a top against a bottom). "A bounding box
rather than a sum" is therefore exactly the
`Length.Max(categoryHeight + categorySpace, valueHeight / 2)` round twenty-nine already landed.

The eleven-variant band probe agrees: **with the labels upright our rectangle matches
LibreOffice's own to 0.09 pt on every edge of every variant** — legend on and off, axis titles on
and off, labels on and off, 8, 12 and 26 categories. Mean over 44 edges 0.77 pt, and 0.68 of that
is the two variants that change the label *size*.

What is left splits in two, and neither is the shape of the algorithm.

### The room a category label has is the tick spacing, not 88% of it

A word that does not fit between two ticks makes chart2 give up on wrapping and start again, and
that restart is the only route from "the labels collide" to "the labels are turned 45°". So the
limit decides whether a whole axis is turned. Ours was 0.95 of the tick spacing less the text
shape's two horizontal insets — 0.88 of it at ten point — and **both corrections are in
LibreOffice's source and neither is in its binary**: `createTextShapes` reduces by 5% "to have a
visible distance between the labels" (`VCartesianAxis.cxx`:753-759) and hands the result to a
shape whose text area is that width less the insets `ShapeFactory::createText` sets.

Three boundaries, each crossed by a different variable, read off `chart:coordinate-region`:

| probe | LibreOffice | ours, before | limit ÷ spacing |
|---|---|---|---|
| 26 six-character categories down to 12 | upright at 15, turned at 16 | turned at 14 | 0.990–1.056 |
| 20 categories, 3 to 6 characters | upright at 4, turned at 5 | turned at 4 | 0.880–1.100 |
| 10 categories, 7 to 11 characters | upright at 9, turned at 10 | turned at 9 | 0.990–1.100 |

One is the only round number in the intersection. On the four probes that sat in the band the
bottom edge was 29.03, 29.03, 20.54 and 41.77 pt out and is now 0.09.

**What the corpus cannot separate** is that from `0.95 × spacing + 2 × inset`, which fits all
three boundaries equally: the two differ by at most 0.36 em and no reachable category count lands
between them. Recorded in the remarks rather than resolved.

**And it moves zero of the 163 documents.** Rendered at both commits with `SOURCE_DATE_EPOCH`
set: byte-identical on all 163, and the plot-rectangle total is 418.77 → 420.27 pt, all of that
movement the weight's. No corpus chart sits in the band the rule governs. That is a fact about
the corpus, not a reason to keep the refuted rule — the same shape as round twenty-nine's tick
labels.

### A stated weight on the axis labels and the legend

`ChartPlot.IsLabelBold` and `IsLegendBold`, `ChartLabel.IsBold` as `bool?`, and an `InWeight`
stamping pass beside `InFamily` — the design `Paperless.Presentations/TODO.md` recorded, taken
verbatim. It reaches the *measurement* as well as the drawing, which is the point: a bold face is
wider, so the widest value label reserves a different left edge and two category labels collide at
a different width. A data label takes the axis' weight rather than its series' own
`c:dLbls/c:txPr` — the same approximation `LabelSize` already makes, and no corpus document
contradicts it.

### The numbers, said plainly

| slides, 163 documents | base `946b3defc` | +weight | +wrap limit |
|---|---:|---:|---:|
| word gate | 151 | 151 | 151 |
| `ink%` | 1268.44 | **1270.05** | 1270.05 |
| `\|ink\|%` | 1589.24 | **1591.86** | 1591.86 |
| major pages | 427 | 427 | 427 |
| census, unexplained | 303 over 91 | 303 over 91 | 303 over 91 |
| plot rectangle, total \|error\| | 418.77 | 420.27 | 420.27 |

Verdicts changed: **0**. Every batch holds its count.

**The metric moved against us and the faces moved with the reference, so both numbers are
here.** Reach measured by rendering: 160 of 163 byte-identical, **3 changed** —
`southern-classic` +2.26, `171128IPAP` +0.41, `Sector_Skills` −0.05. The census that says 36 of
61 chart parts state a weight is the ceiling, and it counts only over the 15 documents carrying
an OOXML chart part at all.

**The second error is localised.** On `southern-classic` page 11 the reference draws 74 bold
records where we drew 69 regular and 5 bold; we now draw 74 bold, and pages 12 and 5 are the same
shape. What is left there is the *size*: the reference draws that page at 13.59, 14.00 and
15.49 pt where we draw one flat 14.00, so a heavier face puts more ink into a mismatch that was
already there. That is the per-element `c:txPr` size `ChartPlot` collapses into one `LabelSize`,
it is now an item in `Paperless.Presentations/TODO.md`, and it is where 2.26 of the 2.62 sits.
`171128IPAP`'s +0.41 is a different residue: its page 38 chart is drawn in LiberationSans where
the reference resolves Carlito, so the weight is now right in the wrong family; its page 36,
whose family is right, moved to Carlito-Bold exactly as the reference has it.

### Cross-track

The weight cannot reach the other two tracks by construction — `SheetChart` and `FrameChart`
measure through an `IChartTextMeasurer` that already took a weight and already documents in code
that it drops it — but the wrap limit is unconditional and does. Rendered at both commits with
`SOURCE_DATE_EPOCH` set and byte-compared over all 371 words and sheets documents: **0 changed.**
Every chart either track holds either has room for its labels or is already far past the
boundary.

### Measured and not resolved: the rotated label's depth

With the labels turned, our reserved band is **7.94 pt too deep at 10 pt** and the excess is flat
in the label's width — 7.42, 7.94, 7.96, 7.93 and 8.48 pt at four to twelve characters. Per point
of label width LibreOffice reserves 0.685 against our `sin 45°` of 0.7071, and the baseline of its
first rotated label sits at the same y whatever the label's width. Neither
`W_shape·sin θ + H_shape·cos θ` nor that formula with either inset removed reproduces it. Left
measured rather than fitted.

### Test counts

Core **275**, Containers 109, Text 240, Vector 291, Rendering 119, Markup 259, OpenDocument 125,
WordProcessing 690, Spreadsheets 566, Presentations **542**, Fidelity 550. **0 skipped.**
Core was 264 and Presentations 538 at `946b3defc`; the eleven and four new tests are the weight
and the wrap limit.

Seven mutations put back through `verify-test.sh`, all detected: the stamping pass removed, the
value-label weight dropped from the measurement, the legend's weight dropped from the measurement,
the legend label forced regular, the axis reader reading descendants instead of `c:txPr`, the
legend reader removed, and the wrap limit restored to 0.95 less the insets.

### What the next round should take, in order

1. **The chart's per-element text size.** Named by measurement rather than guessed at, and it is
   the largest single term left on the two decks that carry most of the residue. `southern-classic`
   page 11's reference draws 55 records at 14.00 pt, 12 at 15.49 and 7 at 13.59; we draw 74 at one
   flat 14.00. `ChartPlot` carries one `LabelSize` for every axis, the legend and the data labels.
2. **The rotated category label's reserved depth**, with the numbers above and the warning that
   the obvious formula is refuted.
3. **The legend row pitch's rate**, still the open half of the 96 dpi grid, and the 6.5 pt
   residual on the bottom axis title, still measured rather than fitted.
## Round thirty-one — sheets

Base `150a3dac1`, verified before measuring. The baseline sweep reproduced the brief to the
digit: **147/171, absolute page error 90, 154 exact page counts, absolute word error 42322**, and
the per-batch line as well. 171 rows, no duplicate path, no `ref-failed`.

Two defects, both ported from LibreOffice and both measured against its own output. Full
working, citations and the mutation table are in `probes/sheets-r31/README.md`.

**A collapsed outline group hides its detail rows.** SpreadsheetML states an `outlineLevel` per
row and a `collapsed` flag on the summary row beside the group, and expects the reader to derive
the rest; Excel normally also writes `hidden="1"` on every detail row, so the derivation is
invisible on almost everything. `Application_Compliance_Checklist_5_Apr_2021.xlsx` states no
`hidden` anywhere, and **329 of one sheet's 1033 rows** are hidden by the rule alone — 18 printed
pages against 14. A Python prototype of `convertOutlines` reproduced LibreOffice's own hidden set
exactly, 329 of 329, before any C# was written. XLSX and XLSB; **BIFF deliberately not**, because
a `ROW` record carries `fHidden` itself and the outline array there only records the state.

**The header and footer band was drawn in six wrong ways at once.** Only the first line of each
part was drawn; the `&n` size code was parsed and discarded; the page's own zoom was not applied;
the text was centred where Calc anchors it to the edge its margin fixes; the three parts were
sized independently rather than sharing the tallest one's height; and a section switch did not
reset the font. Ours now lands within **0.08 pt** of LibreOffice's header top and footer bottom
on two corpus pages at two different zooms, and within 0.03 pt on the authored fixture.

**The anchoring is measured right on SpreadsheetML and measured wrong on BIFF**, so it is set by
the OOXML readers only. LibreOffice's own PDFs of the `sheet-decor` fixture triple put the `.xls`
band 1.5 pt further inside the page than the `.xlsx` band at both edges, with the `.fods`
agreeing with the `.xls`. 1.5 pt is the gap between a measured line height and the bare point
size the filters use as their nominal, so the cause is probably in that arithmetic — **not
settled**, and `XlsPrintSetup` carries the measurement in a comment rather than a guess.

Whole track on the final tree, 171 rows with no duplicate path and no `ref-failed`:
**148/171 matches, absolute page error 86, 155 exact page counts, absolute word error 33174** —
from 147, 90, 154 and 42322. Per batch: 001–009 89/89, 010 8/10, 011 6/10, 012 8/10, 013 8/10,
014 9/10, **015 6/9**, 016 5/9, 017 6/10, 018 3/4. **No batch fell.**

Two verdicts moved and one of them is a match. `fm-provider-service-measures.xlsx` goes
20944/21458 words to 21364 and matches. `Application_Compliance_Checklist_5_Apr_2021.xlsx` goes
18 pages to **14 against 14** and 26353 words to 17235 against 17718 — the round's biggest single
movement, and still a `words` failure by 483 against a 354 band. What is left of it is its
**visible cell comments**, which LibreOffice draws inline and we draw not at all; that also
extends its print area four columns and is why our fit-to-width zoom is 2.6% larger than
LibreOffice's on that sheet. Diagnosed, not implemented.

Ten documents moved their word count towards the reference by twenty or more and two away, both
of them staying on the verdict they had. **Four of the ten are `.xls`** — no census of an OOXML
part could have found them, because a multi-line footer is stated in a BIFF `FOOTER` record.
Byte-level reach with the clock pinned: **54 of 171 documents' rendered bytes moved**, 28 `.xls`
and 26 `.xlsx`, against 12 the gate could see and 2 verdicts.

**Only one page count in the track changed**, and the page-error and exact-count movements are
entirely that one document. The band's height already came from the two margins through
`SheetBandHeight`, so drawing the band correctly moves ink and words and not paper; the outline
rule is what moved paper.

New fixture `tests/corpus/features/sheet-outline-collapse.xlsx`, authored to separate the
decisions rather than copied from a corpus workbook. Two test classes, 15 cases, every asserted
coordinate LibreOffice 24.2.7.2's own. Six reintroduced defects, all six detected —
`probes/sheets-r31/mutate.sh`. One of those mutations came back *undetected* on its first
version and the mutation was wrong, not the test; that is recorded rather than quietly fixed.

Per-project tests, all matching the known-good counts with 0 skipped: Core 264, Containers 109,
Text 240, Vector 291, Rendering 119, Markup 259, OpenDocument 125, WordProcessing 690,
Spreadsheets **588** (573 plus this round's 15), Presentations 538, Fidelity 550.

**The fidelity suite is what found the two corrections above**, on a run that failed 3 of 550 —
and it is the run that nearly did not happen, because it takes hours under three agents' load and
had to be started in the background before the corpus sweep. The track was then swept twice, and
between the two sweeps three documents change by seven words in total and no verdict does.

## Words, round thirty-two, at `06bede896` — the `w:trHeight` gate is real, is caused, and is not the room

### The baseline reproduced the brief exactly, and the reference was held fixed

Whole-track sweep against a checksummed CLI snapshot with `SOURCE_DATE_EPOCH` pinned:
**154/200, absolute page error 76, 164 exactly-correct page counts, absolute word error 6694.**
200 rows, no path twice. Every figure in the brief reproduced to the digit.

Our own renderer is byte-deterministic under a pinned clock — 11 of 11 documents re-rendered
identical — so every comparison below re-uses one set of reference PDFs and varies only our side.
That takes `soffice`'s own instability out of the measurement rather than assuming it away.

### The handover's stated cause is refuted: nothing is lost across a split

The brief said `SliceRow` measures a split part from its cells' line bottoms and never consults
`MinHeight`, so the floor is lost the moment a row crosses a page. Traced on
`f445896eb008d14c1746fc37d412dc22.docx`: nine rows split, and **every one of them already sums to
more than its floor across its parts** — row 5 to 220.30 pt against a floor of 185.05, row 8 to
264.80 against 248.25, row 12 to 693.90 against 690.00, row 14 to 2467.80 against 690.00, and so
on for all nine. There is nothing for the floor to add.

LibreOffice charges the floor to the sum too, so the two engines already agree here:
`lcl_CalcMinRowHeight` skips it for a row `IsInSplit()` (`sw/source/core/layout/tabfrm.cxx`:5087)
and `lcl_calcHeightOfRowBeforeThisFrame` subtracts the earlier parts' heights for a follow (:5696).

### The floor *is* the cause, and the mechanism is a gate on breaking rather than a height

Lower row 8's `w:trHeight` and change nothing else, and LibreOffice breaks the row again. The flip
is sharp, between **4250 and 4300 twips**. Below it page 3 holds 416 words (`wc -w`) — which is
exactly what we produce — and above it 323, with 140 pt of page left blank. Every one of the
document's ten split decisions fits "the row is broken only when the room left is at least its
declared floor", 10 of 10, and that rule takes pages 1–12 from diverging at page 3 to matching the
reference word for word.

**And the rule is wrong.** `dotnet/tests/corpus/features/table-row-min-height` breaks its floored
row at every declared floor from 4.8 cm to 8.0 cm with about 100 pt of room — the last of those is
the entire body height of its page, and the row still gives up exactly seven lines. So the gate is
conditional on something these two documents differ in, and the sharp boundary at ~4275 twips is
not the 2800 twips of room either. Implementing the rule anyway fails five tests in
`Paperless.WordProcessing.Tests`, four of them that fixture's own, and it was reverted.

`dotnet/probes/trheight-split-gate.py` reproduces both halves in one command and lists what is
already ruled out: the floor being lost across the split, `SwTabFrame::Split`:1188-1196 (that
branch is reached only inside a splittable fly), the row being on a follow table rather than a
master one (row 5 of the same document is on a follow table and breaks), and any ratio between
floor and room.

### Reach, measured by rendering, and what the census would have claimed

The reverted rule changed **12 of 200 renderings** and moved **one** verdict — and moved it from
15/16 pages to 17/16, so absolute page error stayed at 76, exact page counts at 164, matches at
154 and word error at 6694, with no word count anywhere changed at all. Worth recording as the
shape of this defect: it is a page-boundary rule, so it reaches few documents and each one hard.

The census over-states by about sevenfold. **89 of 134 DOCX declare at least one `w:trHeight`,
and that is 134 of the track's 200** — the other 66 are `.doc`, which state a row's height as
`sprmTDyaRowHeight` inside a binary table stream and are invisible to any zip-level count.

### The residue on that document is a line break, not a row split

With the rule in, pages 1–12 match word for word and the first divergence moves to page 13, where
one line differs: LibreOffice wraps `industry.` to the next line and we keep it, on a line whose
ink then reaches 519.7 pt where the reference's longest line on that page reaches 523.3. The
paragraph ends with a non-breaking space, which our `TrimTrailingSpaces` already declines to trim.
The nearest untested cause is that **Writer takes the cell's border line width off the cell's text
width and we take off only the margin** — `SwCellFrame::Format` uses `SwBorderAttrs::CalcLeft`/
`CalcRight`, which are `CalcLeftLine()`/`CalcRightLine()` plus the box's LR spacing
(`frmtool.cxx`:2358-2368, `tabfrm.cxx`:6118-6120), while `TableLayouter.LayOut` computes
`inner = width - cell.Padding.Horizontal`. That is a 1 pt narrowing on this table and the decision
is marginal at about that size, so it is a candidate rather than a finding — and it would change
every bordered cell's line breaking in the corpus, so it needs its own sweep.

### Still open

- **Batch 006 stays 9/10.** The one failure is the row-split gate above, now caused rather than
  guessed, with a reproducible probe and a refuted rule.
- **`chg12`'s under-pagination near pages 9–10** — inherited, still unmeasured.
- **The even page with no even story** (3 documents) and **unequal column widths** — untouched.

### The cell's border width, measured and not shipped — the strongest open lead on this track

The candidate above was implemented and swept whole, because a candidate with a C++ citation and a
plausible size is worth a sweep rather than a paragraph. `TableLayouter` now had
`inner = width − padding.Horizontal − (Borders.Left.Width + Borders.Right.Width)`, with the content
rectangle moved right by the left border to match. Against the same reference PDFs, with our own
renderer verified byte-deterministic under a pinned clock:

| | matches | abs page error | exact page counts | word error | under-draw | over-draw |
|---|---:|---:|---:|---:|---:|---:|
| `06bede896` | **154** | 76 | **164** | **6694** | 2783 | 3911 |
| with the border charged | 153 | **73** | 162 | 6789 | **2611** | 4178 |

**119 of 200 renderings changed** — much the largest reach any words change has had — and the trade
is real rather than noise. Two documents landed *exactly*: `A_320.doc` **141/150 → 150/150, now a
full match**, and `info-bulletin-601.doc` 5/6 → 6/6. Three left match, every one by gaining a page:
`part-147_approval list_20230119.docx` 2 → 3, `Agile_Arc_SysDes.docx` 20 → 21,
`ESPN-R - MCF - RA - Ed1.docx` 59 → 60. Under-draw fell by 172 words and over-draw rose by 267,
which is the signature of a narrowing that is right in direction and too large in size.

It was **not** shipped, for one reason that outweighs the page-error gain: it does not do the thing
it was derived from. On the line it was meant to explain — `students that upholders scholarship
into the industry.` — the reference wraps `industry.` and we do not, and with the border charged we
still do not; the line's ink went from 519.7 pt to 520.2, because the text also starts half a border
further right. So the premise "our cell text area is too wide" is unconfirmed by the one direct
measurement available, and a 119-document change resting on an unconfirmed premise is not something
to leave for someone else to inherit as settled.

Whoever picks it up: the `A_320.doc` result is the thing to explain, not the aggregate. Nine pages
recovered exactly is very hard to get by accident, so *something* narrows Writer's cell text area
relative to ours and the border width is the right order of magnitude. The obvious next variants are
charging half the border (the horizontal twin of what the row heights already do with a shared grid
line) and charging it only where the cell's own box actually declares a line, since
`SvxBoxItem::CalcLineSpace(side, /*bEvenIfNoLine*/ true)` returns the distance alone when there is
no line on that side.

## Words, round thirty-three — the border charge is refuted, and the rule underneath it shipped

Baseline re-measured first, on this tree's own CLI at `f1be9c412` against the reference PDFs the
previous round left on disk, with `SOURCE_DATE_EPOCH` pinned: **154 matches, absolute page error
76, 164 exact page counts, word error 6694** — under-draw 2783, over-draw 3911 by the per-document
signed split the last round used. Every figure reproduces the brief to the digit, which is the
first time on this track that a whole inherited headline has.

### The premise is refuted: a Word table's cell text width does not include its border

The lead was that `SwCellFrame::Format` insets a cell by `SwBorderAttrs::CalcLeft`/`CalcRight`,
which add the border line width to the margin. The citation is real. **The branch above it is what
decides**, and it was not read:

```cpp
// sw/source/core/layout/tabfrm.cxx, SwCellFrame::Format
if ( pTab->IsCollapsingBorders() && !pLower->IsRowFrame() )
{
    nLeftSpace  = rBoxItem.GetDistance( SvxBoxItemLine::LEFT );   // the margin ALONE
    nRightSpace = rBoxItem.GetDistance( SvxBoxItemLine::RIGHT );
}
else
{
    nLeftSpace  = pAttrs->CalcLeft( this );                       // margin + border width
    nRightSpace = pAttrs->CalcRight( this );
}
```

`RES_COLLAPSING_BORDERS` is defaulted **true** for every Writer document
(`uibase/app/docshini.cxx`:300), so the border-charging branch belongs to a Writer-native
*separating*-border table and a Word table never takes it. Measured on the installed 24.2 rather
than read off this tree, because the tree has twice been wrong about a constant the binary got
right:

| fixture | text-area width |
|---|---:|
| ODF **collapsing**, 6 pt border, no padding | 453.59 |
| ODF **collapsing**, 0.02 pt border, no padding | 453.59 |
| ODF **separating**, 6 pt border, no padding | **441.59** |
| ODF **separating**, 2 pt border, no padding | 449.59 |

One attribute apart, and it is exactly two border widths. That is the observation that *separates*
the two explanations, and **no corpus document can provide it** — a Word document cannot be made to
take the other branch, which is why two rounds of corpus measurement could not settle this and one
authored pair of files did.

Which branch a Word table takes, at Word's default cell margin: the text-area width is **identical
at borders of 0, 0.5, 2 and 6 pt** — 442.84 pt in a wide cell, 239.24 in a narrow one, 216.06 in
the interior cell of a two-cell table, and 442.84 again through the `.doc` reader.
`dotnet/probes/cell-border-inset.py` reproduces all of it in one command.

### Why `A_320.doc` landed on exactly 150, which was the thing to explain

Two answers, and the second is the one that matters.

Sweeping the charge as *k* × border on that document alone: 141 pages at k=0, 143 at 0.125, 145 at
0.25, 148 across 0.375–0.875, **150 across 1.0–1.25**, 153 at 1.5, 157 at 2.0. Pages word-for-word
identical to the reference at the same index: 2, then 49 at k=0.5, **52** at k=1.0 and k=1.25, then
back to 21 at k=1.5. So the exact landing is not luck — something really does narrow this
document's lines — but the plateau is 0.25 wide and 49 of the 52 pages are already won at half the
charge, so the *magnitude* carries almost no information and "it lands exactly" was never evidence
for the border in particular.

The second answer is decisive. Take the whole document's in-cell text, cluster every word's `xMin`,
and match each cluster to the reference's nearest:

| | columns matched | median delta | mean abs delta | within 0.3 pt |
|---|---:|---:|---:|---:|
| `f1be9c412` | 50 | −0.100 pt | **0.203** | **43 of 50** |
| with the border charged | 50 | +0.000 pt | 0.518 | 20 of 50 |

**Our cell text already starts where LibreOffice starts it.** The change buys nine pages by moving
a horizontal position that matched to a fifth of a point out to half a point — on the bordered
cells only, which is why the median barely moves while the spread doubles. A rule that were right
would take a systematically offset set *into* alignment; this one takes an aligned subset out.

So A_320's nine missing pages remain open and are **not** horizontal. It is the `fUsePrinterMetrics`
document — laid out against a 300 dpi grid — and our `MetricGrid` rounds the *vertical* metrics
only, never a glyph advance. Whether LibreOffice's printer-metric layout quantises advance widths
too is untested here and is the obvious next thing to measure: a quantum of 1/300 in ≈ 0.24 pt is
the right order for lines that wrap one word early.

### What shipped: the rule underneath the wrong one

At a cell margin *below* half the border the DOCX width does move — and the drawn border, which
Writer paints centred on the cell frame edge, proves the table did not move, so it is an inset
after all. It is neither the margin nor margin + border:

```
pad_l = max(bll/2, cml)
pad_r = max(pad_l + blr/2, cml + cmr) - pad_l
```

`lcl_adjustBorderDistance` in `writerfilter/dmapper/DomainMapperTableHandler.cxx`:318–348, whose
comment states it as MS Word's own behaviour, and it reproduces exactly across 21 margin/border
combinations: at a 6 pt border the inset is 3 pt at margins of 0, 1, 2 and 3 pt and then follows the
margin; at 0.5 pt it is 0.25 pt at a margin of 0 and the margin everywhere above.

It is a **floor, not a charge**, and the difference is the whole of what the last round got wrong: a
margin that already clears half its border is untouched however thick the border grows, and Word's
default 108 twips clears every border up to 10.8 pt. It is also an *import* adjustment rather than a
layout rule — `WW8TabDesc::SetTabBorders` (`filter/ww8/ww8par2.cxx`:3020–3042) sets a `.doc` cell's
distance with no floor at all, so the 66 `.doc` of the track's 200 are not subject to it and the
change is in `DocxLayoutSource` alone.

### Its reach, and it moved nothing

**13 of 200 renderings changed** at the byte level. Matches **154**, absolute page error **76**,
exact page counts **164** — every one unmoved. Word error **6694 → 6696**, all of it over-draw
(3911 → 3913) with under-draw unchanged at 2783: on two related FAA documents one token
`(fluid/water)` now wraps mid-token where it did not, because a zero-margin cell narrowed by five
twips. That is an adverse movement of 0.03% and it is reported rather than buried; the rule is
LibreOffice's own and the residual disagreement on that line is a fraction of a point of something
else.

**The census under-stated this time**, which is the opposite of the usual failure and the same
lesson. A zip census over `w:tblCellMar` found **7 documents of the 134 DOCX**, 171 cells of 78287;
**13 changed when rendered**, because the census read the table's margins and not `w:tcMar` on a
cell or a margin inherited from a table style. Quote it as "7 over the 134 the census can read, of
200" and it is still wrong by nearly a factor of two in the direction nobody checks.

Guarded by ten tests in `CellBorderInsetTests`, and both directions are verified by
`verify-test.sh`: removing the floor fails 5 of them, and putting the *border charge* back fails 9.
That second mutation is the point — the rejected rule cannot now be re-proposed without a test
saying so.

### Per batch at `8fbfc2ec3`, measured

001–005 10/10, 006 9/10, 007 10/10, 008 9/10, 009 10/10, 010 8/9, 011 9/10, 012 9/10, 013 6/9,
014 4/10, 015 5/10, 016 7/10, 017 6/10, 018 6/10, 019 4/10, 020 2/10, 021 0/2. Whole track
**154/200**. Identical to the baseline row for row apart from the two word counts above, so every
earlier batch is re-proved by construction rather than by a second sweep.

Tests: Core 275, Containers 109, Text 240, Vector 291, Rendering 119, Markup 259, OpenDocument 125,
WordProcessing **706** (696 + the ten new), Spreadsheets 588, Presentations 542, Fidelity 550, 0
skipped.

### Still open

- **`A_320.doc`'s nine pages are vertical, not horizontal**, and the untested cause named above is
  printer-metric quantisation of advance widths. Nothing has measured whether LibreOffice's
  300 dpi grid reaches a glyph advance; `MetricGrid` in this tree reaches only line heights.
- **Batch 006 stays 9/10**, the row-split gate, refuted rule and all — see `trheight-split-gate.py`.
- **`chg12`'s under-pagination near pages 9–10**; the even-page-with-no-even-story ceiling (3
  documents); unequal column widths (`.doc`-only, small). All untouched.

### Retired

Delete the "obvious next variants" of the border charge from anyone's list. Charging **half** the
border unconditionally is refuted by the collapsing rows above, which do not move at any width;
charging it **only where a line is declared** is refuted by the same rows, which declare one.
