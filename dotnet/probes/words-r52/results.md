# words-r52 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`fc-match Calibri` → `Carlito-Regular.ttf`, corpus `/c/sandbox/workdir/sample-files`,
worktree `wt-words-r50` on branch `wt-words-r52`, base `166a019c6b0`,
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

`prediction.md` beside this was committed at `95c9287d674`, **before anything was changed**.

## Scoreboard

| | words |
|---|---|
| baseline (`MANIFEST.tsv` status column, reproduced) | **316 / 337** |
| after | **318 / 337** |
| gains | **2** |
| regressions | **0** |

### Baseline reproduction

`batch-check.sh … 'words/*' … 8` reported `TOTAL 355  MATCH 333  MISMATCH 22`. Scored against
`MANIFEST.tsv`'s 337-path list rather than that total — the extra 18 rows are the case-insensitive
mount's alias entries — **316 match, 21 open, and 0 disagreements with the manifest's status
column, document for document.** The briefed baseline reproduces exactly.

## Prediction against measurement

| | predicted | measured |
|---|---|---|
| fix A, the VML paint | **0** verdicts | **0** verdicts |
| fix B, the chart categories and the bar-of-pie | **+2**, to 318 | **+2**, to 318 |
| regressions | 0 | **0** |
| cross-track verdicts (`XRay.Office.Ooxml`) | 0 | **0** |

**Seven of 355 rows changed at all** between the two whole-family sweeps, and every one moved
towards the reference. The `done-*` re-sweep is inside that, because the sweep was the whole
family rather than a selection.

## Verdict movement, per document

| document | batch | before | after |
|---|---|---|---|
| `027_Unit_Circle_Chart_Graphical_Chart` | `chartset-009` | `words` 261/378 | **`match`** 376/378 |
| `029_Unit_Circle_Chart_Pie_Theme` | `chartset-011` | `words` 107/114 | **`match`** 111/114 |

**No regressions, and no other verdict moved in either direction.**

Movement that did not change a verdict, all towards the reference:

| document | before | after | reference |
|---|---:|---:|---:|
| `028_Unit_Circle_Chart_Optimized_Graph` | 191 | **317** | 327 — still open |
| `pie-chart-result.docx` | 30 | **36** | 40 — still open |
| `023_Unit_Circle_Chart_Circular_Percentage` | 108 | **107** | 107 — was `match`, now exact |
| `021_Unit_Circle_Chart_3D_Pie_Chart` | 108 | **107** | 107 — was `match`, now exact |

`029` passes on a gap of exactly 3 against a band of exactly 3, because the test is `d > 3`. It is
the one row here with no slack and the prediction said so in advance.

## Cross-track measurement, taken rather than argued

`XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs` sits below all three families. The census named
**23 charts in 23 documents holding a `c:multiLvlStrRef`** — 2 words, 18 slides, 3 sheets — so the
seven batches holding the slides and sheets ones were swept **both ways**: once at this branch,
once with that single file checked out at `166a019c6b0` by `git show` + `cp` + `touch` (never
`git checkout`, and `git diff --cached` asserted empty afterwards, which is the trap
`HANDOVER.md` §7 records).

Of **71 rows**, exactly **one** changed:

| document | before | after | reference | verdict |
|---|---:|---:|---:|---|
| `slides/done-011/pptx/171128IPAP.pptx` | 4640 | **4653** | 4670 | `match` → `match` |

Nothing else moved — not a page, not a word, not a font. The other 17 slides charts and all 3
sheets charts are **unchanged**, which is the over-reach the prediction named in advance for the
sheets side (`ChartRangeResolver` resolves the `c:f` and never enters the multi-level branch) and
which turns out to hold for most of the slides side as well.

`XRay.Office.WordProcessing/Layout/FrameChart.cs` and
`XRay.Office.WordProcessing/Ooxml/DocxVmlFrames.cs` are word-processing only and reach neither other
track. **`SheetChart` and `SlideChart` carry their own copy of the label painter and therefore
still run a multi-line label together** — see "left open".

## The four changes

### A. A VML shape was drawn with no fill and no stroke — 10 documents, 0 verdicts

Measured before the change, from the PDFs' own content streams:

| document | ref strokes | ref fills | ours strokes | ours fills |
|---|---:|---:|---:|---:|
| `065_Work_Breakdown_Structure_Template_Blue_Theme` | 43 | 21 | **0** | **0** |
| `068_Work_Breakdown_Structure_Template_Green_Theme` | 71 | 41 | **0** | **0** |
| `069_Work_Breakdown_Structure_Template_Professional_Format` | 59 | 22 | **0** | **0** |

`DocxVmlFrames` now reads `fillcolor`, `strokecolor`, `filled`, `stroked`, `strokeweight` and the
`v:fill`/`v:stroke` child equivalents into the `Fill`, `BorderColour`, `BorderWidth`, `IsLine` and
`IsLineMirrored` that `PageFrame` already carried and `PageDrawing` already painted.

Three facts were established from the reference rather than assumed:

- **A theme-indexed VML colour resolves to the literal RGB beside the index and the index is never
  consulted.** `fillcolor="#e2efd9 [665]"` is `#E2EFD9`. `ConversionHelper::decodeColor` separates
  the value at its space and returns on a seven-character `#RRGGBB`
  (`oox/source/vml/vmlformatting.cxx:252-257`) long before the palette branch at line 282.
  Confirmed in the reference's operators on `068` (41 fills, all `#E2EFD9`) and on `069`
  (18 `#F2F2F2`, 3 `#D5DCE4`, 1 `#8496B0` — the file's three stated colours, in the file's counts).
- **`strokeweight` is honoured; its absence is a hairline.** Read off the 300 dpi raster, because
  the whole reference PDF carries a single `0.1 w` which is *not* the drawn width: a `v:rect`
  border stating no weight comes out **1 device pixel** and a connector stating `strokeweight="1pt"`
  comes out **4 px at 300 dpi, 0.96 pt**.
- **`v:shapetype` inheritance cannot matter under a stated-colour-only rule**, and that was
  measured rather than waved past: all 64 colour-bearing `v:shapetype` elements in the words
  corpus carry only `filled="f"` / `stroked="f"` and **not one carries a colour**.

The conservatism is deliberate and is the same rule `DocxFrames.Appearance` already documents.
LibreOffice defaults an unstated VML fill to white and an unstated stroke to black; reproducing
that would put a white fill and a black box around all **37** `#_x0000_t75` picture shapes in the
words corpus, none of which states either. So an absent colour means no paint, a fill and a
rectangular border go only to `v:rect` and `v:roundrect`, and a diagonal stroke goes only to a
straight connector. `#_x0000_t136` WordArt (15 shapes) and `#_x0000_t15` (3) keep drawing nothing.

The zero-extent rejection is relaxed **only** for a straight connector, which is how VML writes a
rule (`width:0;height:12.75pt`; 87 of them in the corpus). The inline arm `One()` keeps its extent
check, because a zero-extent inline shape reserving line room would move a page count.

**After, ours against the reference:** `065` 31 strokes / 21 fills, `068` 53 / 41, `069` 47 / 22 —
and the fill *colours and counts* are now identical to the reference on all three. We draw more
stroked paths than the reference does (53 against 36 on `068`); the reference's own PDF has one
stroked path per subpath, so this is not path merging and is left open below.

**The measurement that matters is the page, and it was taken blind.** A fresh reviewer given the
after-pair for `068`, who had never seen the before, reported: *"I cannot see any structural line
art in one half that is absent from the other … Both halves draw the same rectangular border
around every box, the same stub from the centre box down to the trunk, the same single full-width
horizontal trunk, the same five vertical drops, the same short vertical connector between every
pair of vertically adjacent boxes … no colour shift I can detect."* The reviewer of the *before*
pair, equally blind, had reported: *"the reference draws pale-green filled boxes with green
borders around every label; ours draws nothing — bare text on white."*

**Zero verdicts moved and none of the ten documents changed a page or a word count**, exactly as
predicted.

### B. A multi-level category was read at one level, and it was the wrong one

`DrawingChartPlot.ReadSequence` flattened a `c:multiLvlStrCache` by walking
`cache.Descendants(pt)` — every level's points, all keyed on `@idx`. Each `c:lvl` numbers its own
points from zero, so **each level overwrote the one before it and the last written won**, which is
the outermost. The brief called this "taken at one level"; the specific failure is an overwrite,
and the level that survives is the outermost, which is why every label read `Branch 1`.

Now joined with a space from the outermost level inwards, skipping levels that state nothing at an
index — `lcl_getExplicitSimpleCategories`,
`chart2/source/tools/ExplicitCategoriesProvider.cxx:376-395`, which is what `getSimpleCategories`
hands to the legend and to every data label. `DrawingChart`'s *extraction* reader has always joined
them this way, so this also stops the two readers of one element disagreeing with each other.

### C. A bar-of-pie is a pie

`LabelOf` gated `c:showPercent` on `kind == ChartPlotKind.Pie` and `c:ofPieChart` maps to
`ChartPlotKind.OfPie`. LibreOffice's type table puts `TYPEID_OFPIE` in `TYPECATEGORY_PIE` beside
`TYPEID_PIE` and `TYPEID_DOUGHNUT` (`typegroupconverter.cxx:103-105`), and that category is what
`bShowPercent` is ANDed with (`seriesconverter.cxx:140`). Two words documents hold an
`ofPieChart` — `028` and `029` — and both state `showPercent`.

### D. A chart label holding a line break was drawn as one run

`FrameChart.Text` shaped the whole label as a single glyph run, so the `"\n"` separator a
percentage-without-a-value label carries (`seriesconverter.cxx:168-172`, already implemented in
`ChartDataLabel.Separator`) was shaped as a zero-width nothing and ran the halves together:
the reference's `Leaf 11` / `15%` came out of `pdftotext` as the single token `Leaf 1115%`.

This was found by arithmetic *after* B and C had landed — `027` measured 370 against 378 where the
prediction said 373 — and the token multiset diff named it exactly: `1115%`, `1415%`, `415%`,
`515%`, `154%` on our side against `15%`, `4%`, `Leaf`, `Stem` on the reference's. It is worth
6 more words on `027`, and it is what moved `023`, `021` and `pie-chart-result` as well.

## Refutations

### 1. The brief's item 2 is not a one-line predicate, and shipping the predicate alone would have done nothing

The brief said `056`'s ~15 missing connectors are `wsp` members with `ext cx="0"` that
`DocxFrames.Leaf` rejects for having a zero-extent rectangle, and asked for the observable to be
checked before it was built on. Checked, and it does not hold:

- `056` holds **39** zero-extent members, not ~15: 34 `prstGeom prst="line"` and 5
  `straightConnector1`.
- **34 of the 39 state no `a:ln` at all.** Their line comes entirely from
  `wps:style/a:lnRef idx="1"` with an `a:schemeClr val="dk1"` — the theme's
  `a:fmtScheme/a:lnStyleLst`, which `DocxFrames.Appearance` deliberately does not read.

So relaxing `Width <= 0 || Height <= 0` on its own would admit 34 frames carrying
`Fill = null, BorderColour = null` and **paint nothing**. The predicate is necessary and is not
sufficient; a round that shipped it would have measured a confident zero and had no way to tell
that from the fix not being needed.

The real seat is that **`DrawingStyleMatrix` never reaches `DocxFrames`.** It exists, it is
correct, it already resolves `a:fillRef` and `a:lnRef` against `a:fmtScheme` for the slides path,
and `DocxFrameContext` carries only the colour scheme. That is the "route, not a rule" shape this
project has now hit seven times. Its reach is **458 shapes across 40 words documents** — census in
`paint-reach-census.py` — most of which currently pass, so it wants its own round.

The blind reading of `056` agrees with the mechanism from the other side and adds detail no
census produced: *"the reference draws a full set of connectors — one arrow from the root box,
four stacked arrows down the centre column, ten short horizontal links, and two or three tall
bracket spines — where ours draws nothing at all. Ours has zero line art besides the box fills."*
**Arrowheads and elbow brackets are more than a stroked diagonal**, so even with the style matrix
read, `056` will not come out right without arrow ends.

### 2. `024_Unit_Circle_Chart_Colorful_Circles` is not a chart document

The brief grouped it with `027`, `028` and `029`. It carries **no chart part at all**: its graphic
is a `word/diagrams/` SmartArt — `data1.xml`, `layout1.xml`, `quickStyle1.xml`, `colors1.xml`,
`drawing1.xml`. It is 95/105 before and 95/105 after, and nothing in this round could have touched
it. It is a different defect and should be briefed as one.

### 3. `069`'s "PROJECT NAME" is not something the fills broke — they exposed it

The blind reviewer of `069` after fix A reported our top box as *"a solid filled bar with no
visible text"* with `PROJECT NAME` overprinting `DEVELOPMENT` one row below. That reads as a
regression and is not one: `pdftotext -bbox` puts `PROJECT` at `yMin=156.128` in our rendering
**both before and after the change, to the thousandth of a point**, against the reference's
`78.878`. We have placed that text ~77 pt too low for as long as it has been drawn; the box that
now appears around where it *should* be is what made it visible. `069`'s `_x0000_s1026` states
`margin-top:-6.35pt`, which is 78.35 pt above the `1in` the three boxes below it state.

## The `pages 1/2` cluster — item 4, and it now has a mechanism and a magnitude

Not implemented. What was measured is decisive and is committed as `br-paragraph-probe.py`.

### `097`, line by line

Every text row on page 1, ours against the reference (`pdftotext -bbox`, y in points):

| ours | ref | Δ | row |
|---:|---:|---:|---|
| 21.0 | 20.4 | +0.6 | `BUSINESS CASE` |
| 72.5 | 69.5 | +3.0 | `Document Control` |
| 173.6 | 136.4 | **+37.2** | `Document Information` |
| 219.1 … 344.8 | 182.6 … 309.1 | +35.6 … +36.5 | the five table rows |
| 386.9 | 350.7 | +36.2 | `Document History` |
| 440.4 | 431.2 | **+9.2** | `Versions Issue Date Charges` |
| 627.1 | 618.2 | +8.9 | `Document Approvals` |
| 679.8 | 697.8 | **−18.0** | `Role Name Signature Date` |

Three discrete places, not a drift: we are **34 pt too tall** between `Document Control` and
`Document Information`, then **27 pt too short** above the History table, then **27 pt too short**
again above the Approvals table. Net −20 pt, which is the brief's "our table is about 20 pt too
short" — now localised.

### What the two 27 pt gaps are

Both sit at a paragraph whose entire content is a `<w:br/>`. Nine authored variants, each `AAA`,
one paragraph under test, `BBB`, Cambria on A4, rendered both ways — points added by the paragraph
under test:

| case | reference | ours |
|---|---:|---:|
| one empty paragraph | 12.65 | 11.50 |
| **one paragraph holding only `<w:br/>`** | **25.30** | **0.00** |
| two empty paragraphs | 25.30 | 23.00 |
| **one paragraph holding two `<w:br/>`** | **37.95** | **0.00** |
| `X<br/>Y` | 25.30 | 23.00 |
| `<br/>Y` | 25.30 | 23.00 |
| `Y<br/>` | 25.30 | 23.00 |
| `<space><br/>` | 25.30 | 23.00 |

**A paragraph with N breaks is N+1 lines in the reference, and we agree in every case where the
paragraph holds any other content at all — one space is enough. The paragraph whose whole content
is breaks contributes nothing on our side.** That is 25.30 pt lost per occurrence, twice on `097`,
which with the ~9% line-height difference beside it is the 27 pt measured on the page.

**The reader is not the seat.** `xray-office extract` on the `a-br` variant already prints
`AAA`, blank, blank, `BBB` — the two lines the reference draws. `DocxLayoutSource` emits U+2028
and builds the `PageParagraph` with it, and `TextMeasurer` already carries the trailing-separator
rule (`lines.Count > 0 && IsLineSeparator(text[^1])`) with a comment saying exactly why. The seat
is between those two and was not found in the time this round had.

**Reach: 469 such paragraphs in 66 of the 271 distinct words documents**, including
`FAA 2025-26 Holdover Tables` (66), `24-25_FAA_Holdover_Tables` (58),
`OM template for non-complex NCC operators` (37) and `EHEST-SMS` (35) — several of them large
page-count documents that currently pass. **This is not a change to make without a whole-family
sweep**, and that is why it was measured and left rather than shipped at the end of a round.

**It is `097`'s cause and not the other two's.** `097` holds 3 such paragraphs; `012` holds **0**
of 142 and `015` holds **0** of 158. The brief's "at least two causes" is confirmed, and the split
is now on a countable property rather than on a shape.

## Tests

Three new files, 24 tests, every one **verified by reintroduction** with
`verify-test.sh` — five separate mutations, each detected:

| mutation | detected by |
|---|---|
| `bool box = false` (never paint a rect) | 9 of `VmlShapePaintTests` |
| restore the zero-extent rejection for connectors | `AZeroWidthConnectorIsDrawnAsADiagonal`, `AGroupMemberIsPaintedToo` |
| put `multiLvlStrCache` back on the flat walk | 3 of `DrawingChartCategoryTests` |
| `kind == ChartPlotKind.Pie` | `APercentageIsAPiesBusinessAndABarOfPiesToo`, `APercentageWithoutAValueIsWrittenOnItsOwnLine` |
| remove the `Lines(...)` call in `FrameChart` | `ALabelWithALineBreakIsDrawnAsTwoRuns` |

**None of the 24 is only a drift guard.** `NoPaintIsInvented` is the one that decides reach and it
is a detector too — it fails the moment a default fill or stroke is invented.

Ten non-Fidelity projects, run one at a time:

```
Core 337  Containers 109  Text 596  Vector 295  Rendering 150(1 skipped)  Markup 259
OpenDocument 125  WordProcessing 1083  Spreadsheets 886  Presentations 779     = 4619
0 failed
```

**4595 → 4619, delta +24**, which is 15 + 2 in `WordProcessing` and 7 in `Presentations` — the new
tests and nothing else. `dotnet build -v q -nologo`: **0 warnings, 0 errors.**

## Proposed `MANIFEST.tsv` reclassification

`/c/sandbox/workdir/sample-files` is a separate checkout and was **not** committed to from here.
Two rows for the parent to apply, `status open` → `status done`:

```
words  chartset-009  words/chartset-009/docx/027_Unit_Circle_Chart_Graphical_Chart_5462a579.docx
words  chartset-011  words/chartset-011/docx/029_Unit_Circle_Chart_Pie_Theme_8a922142.docx
```

## Left open, in the order the next round should take it

1. **The `w:br`-only paragraph.** Measured, magnitude known (25.30 pt each), witness set known
   (469 paragraphs, 66 documents), `097` explained. The seat is in layout, not in the reader, and
   is *not* `TextMeasurer`'s trailing-separator rule. Sweep the whole family behind it: several of
   the heaviest witnesses currently pass.
2. **`097`'s other 34 pt**, between `Document Control` and `Document Information`, where we are
   *too tall*. Unexplained. Fixing item 1 alone moves `097` by about +46 pt, which is more than
   the 20 pt it needs, so the two interact and should be measured together.
3. **`012` and `015`** share nothing with `097` — no `w:br`-only paragraphs at all — and remain
   unexplained.
4. **`DrawingStyleMatrix` does not reach `DocxFrames`.** 458 shapes in 40 words documents take
   their fill or their line from a `wps:style` reference we do not read. This is what `056`'s
   missing connectors actually are, and it will also want **arrow ends**, which the blind reviewer
   saw and no census counts.
5. **We stroke more paths than the reference does** on the VML documents — 53 against 36 on `068` —
   with the fills exact. Not path merging (the reference's own stroked paths are one subpath each).
   Worth one measurement before anything is built on it.
6. **`SheetChart` and `SlideChart` still run a multi-line label together.** `FrameChart` was fixed
   for the words track only, deliberately, to keep this diff off the other two tracks. The same
   `\n` separator reaches both.
7. `028` at 317/327 is the largest words gap left. Its reference extraction is itself garbled —
   the bar-of-pie's segment labels come out one character per line in the reference, which is a
   ceiling rather than a defect on our side.
8. `024` is SmartArt (`word/diagrams/`) and has never been looked at.
9. Our empty paragraph is 11.50 pt against the reference's 12.65 on Cambria — a ~9% line-height
   deficit, visible in every row of the probe table above, and adjacent to the standing advance
   divergence rather than part of it.

## Files

- `prediction.md` — committed at `95c9287d674`, before anything was changed.
- `br-paragraph-probe.py` — the nine authored variants above; re-run reproduces the table exactly.
- `vml-shape-census.py` — every reachable VML shape in a corpus tree by type, geometry and colour.
- `paint-reach-census.py` — the VML and the `wps:style` reach counts, per document.
- `chart-multilevel-census.py` — `c:multiLvlStrRef` and `c:ofPieChart` across all three families.
