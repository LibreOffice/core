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
| `batch-012` | 10 | 306–333 | doc:4 docx:6 | 8/10 |
| `batch-013` | 9 | 338–370 | docx:10 | 5/9 |
| `batch-014` | 10 | 372–422 | doc:4 docx:6 | 3/10 |
| `batch-015` | 10 | 424–471 | doc:3 docx:7 | 4/10 |
| `batch-016` | 10 | 473–537 | doc:5 docx:5 | 7/10 |
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
| `batch-009` | 9 | 421–540 | xls:2 xlsx:8 | 6/9 |
| `batch-010` | 10 | 560–691 | xls:7 xlsx:3 | 5/10 |
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

## Sheets, round seventeen (in progress): the substitution chain is not what the binary follows

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
