# round 84 — a worksheet shape's fill and outline, in all three spreadsheet formats

`SheetDrawing.Fill` and `.Stroke` had exactly one writer, `XlsxNoteCaptions`, and every other
shape on every worksheet was drawn as bare text over whatever was under it. This round reads the
fill, the outline and the geometry in **SpreadsheetML, ODF and BIFF**, paints them through the
shape's own preset, and closes the print-area half of it for ODF.

**No gate verdict moves in either direction, and that is the expected result rather than a
disappointment**: the whole-corpus gate is `914 of 947` before and after, and of the 35 corpus
renderings that changed, **not one changed a page count, a word count or a glyph count**. The
evidence is PDF marks and pixels.

## Environment

| | |
|---|---|
| tree | `/home/user/wt-sheetfill`, branch `agent/sheetfill`, base `6d19fd594` |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything here** |
| original corpus | `/home/user/sample-files`, 947 documents (words 338, slides 302, sheets 307) |
| converted corpus | `/home/user/corpus-odf`, the `.ods` column being 307 files |
| reference bank, original | `/home/user/gate-orig-r83/ref`, 947 PDFs at 26.2.4.2 |
| base bank, original | `/home/user/gate-orig-r83/ours`, 947 PDFs rendered at this round's base `6d19fd594` |
| reference bank, ODF | `/home/user/gate-odf-r78/ref` — the same bytes round 82 scored against |
| C++ read-only | `/home/user/libreoffice-core` |
| fonts | system fontconfig; the tarball's confound families aside |
| date | 2026-09-08 |

**The disk was the binding constraint and it changed how the sweeps were run.** `/` was at
**100% with 73 MB free** when this round started; `dotnet nuget locals http-cache --clear` and
`temp --clear` recovered about 400 MB and that is all there was. One rendering of the
947-document corpus is roughly 700 MB, so `sweep-ours.sh` here **scores, hashes and unlinks each
PDF before moving to the next** and its footprint is one file per worker. Anyone reproducing this
should check `df -h /` first; a full disk looks exactly like a catastrophic regression, which
`dotnet/CLAUDE.md` already records twice.

## What was measured, and with what

The gate cannot see a fill. Three instruments were used instead, and each is in this directory:

| | |
|---|---|
| `census.py`, `census-ods.py` | what the corpus declares, per shape, resolved rather than grepped |
| `marks.py` | filled and stroked paths per page, ours against the reference, from `page.get_drawings()` |
| `make-fixture.py` | the five-shape fixture the unit tests are established on, and its 26.2.4.2 ground truth |
| `sweep-ours.sh`, `score-bank.py`, `compare.py` | the gate rows, and the byte-level "did this document move at all" column |

**A gate row's verdict is decided on column 9, `glyphs`, and not on column 4, `words`.** Confirmed
from `batch-check.sh`:279 and the block above it: the `awk` band reads `$og`/`$rg`, which are
alphanumeric *characters*, and fails when `d > b*0.02 && d > 15` — `max(2%, 15)`, an AND rather
than a sum. `words` is a letter-or-digit *token* count kept for history. The brief is right and
the correction it carries is real.

## 1. The census the brief quotes is right, and the conclusion drawn from it is not

`probes/overflow-r69/census.py shapebox` reproduces exactly, against the current corpus:
**644 worksheet `xdr:sp` in 49 documents**, 421 stating an `a:solidFill`, 205 an
`a:ln/a:solidFill`, **583 an `xdr:style`**, 332 of the 626 explicit colour references being
`schemeClr`, and **28 distinct presets** of which only 206 are `rect`.

**What the brief gets wrong is what the 583 buys.** *"Over half the work is theme-colour
resolution before any of it is drawing"* is true of the colours and false of the format matrix.
Resolved per shape rather than counted per element (`census.py style`), **497 of those 583 styled
shapes also state a fill of their own**, and the shape's own statement wins —
`Shape::getActualFillProperties` takes the reference shape, then the theme's `a:fillRef`, then
`assignUsed`s the shape's own over both (`oox/source/drawingml/shape.cxx`:2816-2841). The
effective source of a fill across the whole corpus is:

| where the fill really comes from | shapes | documents |
|---|---:|---:|
| the shape's own `a:solidFill` | **421** | 32 |
| `a:grpFill` — the enclosing group's | 98 | 2 |
| the theme's `a:fillRef`, a solid entry | 54 | 2 |
| the theme's `a:fillRef`, a gradient entry | 32 | 1 |
| `a:noFill` | 29 | 9 |
| `a:blipFill` | 10 | 10 |

So **the format matrix decides 86 shapes in 3 documents**, and what is needed everywhere is theme
*colour* resolution, because the `schemeClr` sits on the shape's own fill. A round that had
budgeted by the 583 would have built the expensive half first.

**Two more figures the brief does not have, and both changed the shape of the work.** *(a)*
**174 of the 644 sit inside an `xdr:grpSp`**, in 6 documents, and 31 are `xdr:cxnSp` connectors in
2 — a group's anchor carries one fill and its leaves carry many, so the model needed ink on
`SheetDrawingPart` as well as on `SheetDrawing`. *(b)* **31 shapes in 3 documents are turned into
the quarter-turn ranges Excel rewrites an anchor for**, which is §4 below and was invisible until
the outlines were drawn at all.

## 2. The rules, each verified in this checkout

| rule | seat |
|---|---|
| every object on the draw page widens the printed block; the only exclusion is the hidden-comment layer, and the line above the layer test reads `//TODO: test Flags (hidden?)` | `sc/source/core/data/drwlayer.cxx`:1344, 1397-1414 |
| the cells' print area is maxed with the drawing layer's | `sc/source/core/data/documen2.cxx`:644-664 |
| a drawing prints after the strings, on the front layer | `sc/source/ui/view/printfun.cxx`:1651, 1703 |
| the theme is the base and the shape's own fill wins over it; `a:grpFill` takes the parent's | `oox/source/drawingml/shape.cxx`:2816-2841 |
| `a:fillRef idx` ≥ 1000 indexes `a:bgFillStyleLst`, and an index past the end is clamped | `oox/source/drawingml/theme.cxx`:41-58 |
| an anchor states a quarter-turned shape's rectangle *after* the turn, so Calc reflects it in `y = x` and swaps width for height | `sc/source/filter/oox/drawingfragment.cxx`:299-330 |
| a hidden **BIFF** object is not processed at all — the opposite of the DrawingML rule | `sc/source/filter/inc/xiescher.hxx`:118 |
| a BIFF object whose anchor is under 3/100 mm by 1/100 mm is dropped as a phantom of a deleted row or column | `sc/source/filter/excel/xiescher.cxx`:414-420, 3658-3665 |
| a group's `sp`, `cxnSp`, `grpSp`, `graphicFrame` and `pic` are all shapes on the sheet | `sc/source/filter/oox/drawingfragment.cxx`:194-198 |

## 3. The fixture, and the ground truth it is pinned to

`make-fixture.py` builds `features/sheet-shape-ink.xlsx` — five shapes, one per arm — and
26.2.4.2's own conversions of it are committed beside it as `.ods` and `.xls`, so all three
readers answer for the same shapes and every divergence between them is ours.

26.2.4.2 renders the `.xlsx` on **two pages** with these five marks on the first:

```
  fs    53.80 109.22 197.77 217.19   fill #FF0000  stroke #0000FF  w 2.239   the star
  fs   242.59 109.22 386.59 181.22   fill #4472C4  stroke #4472C4  w 2.013   the themed box
  s     53.83 198.82 233.83 234.82                 stroke #008000  w 1.502   the turned elbow
  f    242.59 198.82 314.59 270.82   fill #FFA500                            the group's left
  f    314.59 198.82 386.59 270.82   fill #00A0A0                            its a:grpFill twin
```

This tree reproduces all five to **0.06 pt in x and 0.14 pt in y**, and both page counts. Four
things are established by those five lines and by the shape that is *not* among them — the bare
box, which states `a:noFill` and `a:ln/a:noFill` under the same `xdr:style` and is drawn nowhere
on either side, so a shape's own statement beats the matrix. The themed box is painted in
`accent1` at 25400 EMU, and **neither number is anywhere in the drawing part**: the colour is the
`phClr` the reference substitutes and the width is the theme's *second* line style.

The `.ods` twin agrees to **0.07 pt** and is closer in x than the `.xlsx` path manages — 242.62
against the reference's 242.59, where the `.xlsx` gives 246.36 — because ODF states the rectangle
outright and SpreadsheetML states cells and offsets.

## 4. A quarter-turned shape's anchor is the turned rectangle, and that was ours

The defect the outlines made visible. `016_Free_Organizational_Chart_Template` draws fifteen
`bentConnector3` elbows; twelve are written `rot="10800000" flipH="1" flipV="1"`, whose net effect
is the identity, and three are `rot="5400000"` or `rot="16200000" flipH="1"`. Drawing the first
twelve needed only the flips — **the two mirror the shape's own space before `@rot` turns it**, so
honouring the rotation alone leaves the path reflected in both axes. Drawing the other three
needed the anchor rule: Excel rewrites the anchor cells of a shape rotated into `[45°, 135°)` or
`[225°, 315°)`, and Calc undoes that with `X += (w − h)/2`, `Y += (h − w)/2` and a swap before any
rotation is applied (`drawingfragment.cxx`:299-330, whose own comment says the anchor "contains
the original not-rotated shape" only outside that range).

Read out of the PDFs — every stroked path on page 1 of that document, in points:

| | ours before | ours after | 26.2.4.2 |
|---|---|---|---|
| the trunk from `President` | 275.4–315.1 × 79.6–298.0 | **404.5–622.9 × 169.0–208.7** | 404.5–614.2 × 168.3–208.1 |
| one of the twelve elbows | absent | 300.6–335.4 × 235.1–314.5 | 317.2–335.3 × 234.6–305.2 |

Before, the trunk was a tall vertical running off the top of the page and none of the twelve
elbows was drawn at all. After, the trunk's left edge is **exact**, its top within **0.7 pt** and
its right **8.7 pt** long; the elbows' right edge agrees to **0.1 pt** and their top to **0.5 pt**,
while their left edge is 16.6 pt out and their foot 9.3 pt. Both residuals are one thing and it is
not the transform: these connectors state `adj1 = -92308`, a **negative** adjustment, and this
evaluator clamps it into the preset's declared range where 26.2.4.2 does not. That is a
`CustomShapeGeometry` question and it is left with its seat.
**Reach: 31 shapes in 3 corpus documents** — 24 `xdr:sp`, 6 `xdr:cxnSp` and one `xdr:pic`.

## 5. What moved on the original corpus, and how it was measured

The gate is `914 of 947` before and after — sheets 292, words 329, slides 293 — so **no verdict
moves in either direction, which is the expected result**: a fill adds no glyph and no page.

The column that can see the work is the byte column. `sweep-ours.sh` hashes every rendering with
its `/CreationDate` and XMP dates masked and compares it against `/home/user/gate-orig-r83/ours`,
which is this round's base commit's own output for all 947 documents:

| track | renderings | byte-identical to the base | moved | page or glyph counts changed |
|---|---:|---:|---:|---:|
| sheets | 307 | 272 | **35** | **0** |
| words | 338 | **338** | 0 | 0 |
| slides | 302 | **302** | 0 | 0 |

**Not one of the 35 movers changed a page count, a word count or a glyph count** — checked row by
row with `compare.py` against `score-bank.py`'s scoring of the base bank. The two untargeted
tracks are byte-identical document for document, which is what the gating predicts: the diff is
confined to `XRay.Office.Spreadsheets` plus one new file in `XRay.Office.MsBinary/Escher` whose only
caller is `XlsDrawing`.

### Marks, before and after

`marks-table.py`, filled and stroked paths over the whole document, base / after / 26.2.4.2:

| document | pages | fills | strokes |
|---|---|---|---|
| `025_Printable_Reward_Charts_for_Kids` | 5/5/5 | 246 / **446** / 260 | 135 / **335** / 871 |
| `016_Free_Organizational_Chart_Template` | 4/4/4 | 114 / **159** / 914 | 18 / **123** / 128 |
| `027_Simple_personal_cash_flow_statement` | 10/10/10 | 2888 / **2930** / 587 | 57 / **106** / 404 |
| `038_Baby_growth_chart` | 2/2/2 | 119 / **168** / 79 | 33 / 33 / 31 |
| `017_Timeline_Templates_for_Excel` | 3/3/3 | 52 / **62** / 11 | 35 / **44** / 53 |
| `076_Inventory_list_accessibility_guide` | 9/9/9 | 688 / **703** / 81 | 4 / 4 / 4 |
| `079_Org_charts_visual` | 3/3/3 | 1874 / 1889 / 246 | 157 / 157 / 159 |

**And the instrument has a caveat that has to travel with those numbers, or the table reads as
massive over-drawing.** A path count is not comparable between the two sides wherever a *gradient*
is involved: on `016` the reference's page 1 emits **16 `Do` operators and no `sh`** — one Form
XObject per gradient-filled box, each holding a stack of band fills — while ours emits **16 `sh`
and no `Do`**, one PDF shading each. That is the whole of 914 against 159. The comparable column
there is the strokes, **123 against 128**, and the picture. Count `sh` and `Do` before reading a
fill count as a defect.

### Pixels

The reward chart is the clearest case and it is a whole feature rather than a nuance: at the base
this tree drew **none** of its hundred `star5`, `verticalScroll`, `smileyFace`, `heart`, `cloud`,
`pie`, `sun`, `lightningBolt`, `flowChartDelay` and `irregularSeal1` stamps, and its pages 1 and 2
were an empty grid. They are all present now, in the right cells, at the right size, in the right
colours. What the reference still has and we do not is the **soft drop shadow** behind each stamp
(`a:effectRef` → `a:outerShdw`), which is most of its 871 strokes against our 335.

**These readings are my own and are therefore contaminated**, and the check is two calls rather
than a guess: this container has no `Task`/subagent tool, and `mcp__Claude_Code_Remote__create_session`
spawns a sibling in its own container that cannot open `/home/user/...` and has no way to report
back — the fourth round in a row to record this. Every reading above is corroborated by arithmetic
that does not depend on it: the fixture's five marks against 26.2.4.2's own to 0.14 pt, and the
connector coordinates in §4.

## 6. What moved on the converted `.ods` column

`.ods` is **267 of 306 comparable rows plus one timeout = 268 of 307**, which is exactly round
82's figure. `compare.py --exclude-failed` against `probes/ods-draw-r82/after.tsv`: **0 verdicts
moved and 0 rows changed a page or a glyph count.**

That is the right result for the ink and it is *not* the whole of the ODF half, because the ODF
half also moved a page — just not on a corpus document. **The reader answered null for a shape
carrying neither text nor a picture, and such a shape is an object on Calc's draw page like any
other.** On `features/sheet-shape-ink.ods` the rightmost object is a rectangle stating
`draw:fill="none" draw:stroke="none"`; 26.2.4.2 prints **two** pages, the second empty, and this
tree printed **one** until it stopped dropping the shape. The `.xlsx` twin was already two of two,
because an `xdr:sp` has always produced a drawing whatever it holds — so *the brief's "an
inked-but-textless shape does not widen the printed block at all" is a statement about the ODF and
BIFF readers and not about the SpreadsheetML one.*

Censused over the 307 converted `.ods` (`census-ods.py`), resolving each shape's style through its
parent chain: **589 shapes state `draw:fill="solid"` and 490 `draw:stroke="solid"`**; **444 custom
shapes in 26 documents carry ink and no text at all**, against 198 that carry both. Every one of
the 444 was drawn nowhere and counted nowhere.

**A turned ODF shape's ink is turned with it**, which the first cut of this round got wrong and the
picture showed: `Placement` answers the *bounding* box of a `draw:transform`, which is what the
print area wants and what a rotated star does not fill, so painting the geometry into it drew the
reward chart's stamps up to 20% too large and squared up. The ink now goes on a
`SheetDrawingPart` — the only thing that carries an angle — exactly as a turned picture's already
did. **Reach: 255 turned `draw:custom-shape` in 8 of the 307**, and the confinement is checked
rather than argued: all 13 `.ods` holding a rotating transform were re-rendered afterwards and
every one keeps the page and glyph counts the sweep recorded.

## 7. What this brief got wrong

**(a) "`SheetDrawing.Fill` and `.Stroke` … are set from exactly one place, `XlsxNoteCaptions` —
verify with `git grep` before you believe it."** Verified and true.

**(b) "583 an `xdr:style` … so over half the work is theme-colour resolution before any of it is
drawing."** The count is right and the inference is wrong; see §1. 497 of the 583 state a fill of
their own, which wins, so the format matrix decides **86 shapes in 3 documents**. The
theme-*colour* half is the one that is everywhere.

**(c) "the rectangle `SheetPageGraphics` strokes for a note caption is the wrong outline for 438
of the 644."** Right, and the arithmetic holds: 644 − 206 `rect` = 438.

**(d) "an inked-but-textless shape does not widen the printed block at all."** True of the ODF and
BIFF readers and **false of the SpreadsheetML one**, which has always produced a drawing for every
`xdr:sp`. The fixture separates them: the same five shapes print on two pages as `.xlsx` and
printed on one as `.ods`.

**(e) "`CustomShapeGeometry` already answers `FillOutline`/`StrokeOutline` for the presets
involved" and "`DrawingStyleMatrix` already resolves the `xdr:style` theme references".** Both
true, both reused unchanged. What was *not* already there, and is most of the round's real work,
is the **transform**: the two flips, the quarter-turn anchor reflection, and putting a grouped or
turned shape's ink on a part instead of on the anchor.

**(f) "`ods-draw-r82` … recorded that the fill and stroke are 'a few lines from where they are
needed'."** Fair for the reading — `OdsShapeInk` is 130 lines and most of it is prose — and an
under-estimate of the rest: the ODF half also needed the null-return removed, the rotation moved
onto a part, and the shared preset resolution lifted out of `OdsShapeText`.

**(g) "the second half is a page-count defect and the gate can see it."** The gate can see it *in
principle* and does not see it *on this corpus*: no `.ods` row changed a page count, because the
uninked shapes in real templates sit inside the used range. It took an authored fixture to
measure, which is the `w:pgBorders` lesson once more.

## 8. What is left, with its seat

- **A shape's `a:effectRef`/`a:outerShdw` shadow.** Most of the reference's remaining stroke and
  fill count on every themed template — 871 strokes against our 335 on the reward chart.
  `DrawingEffects` already reads the element for the slide path; nothing on the sheet path asks.
- **`a:blipFill` and `a:pattFill` on a worksheet shape**, 10 and 0 shapes; and ODF's
  `draw:fill="gradient"` and `"bitmap"`, **32 and 14 shapes**. The ODF gradient cannot be resolved
  from `XRay.Office.Spreadsheets` at all: `OdpFills`, which reads the `draw:gradient` and
  `draw:fill-image` tables, lives in `XRay.Office.Presentations` and is a sibling rather than a
  parent. Moving it down is the same test `Core/Numbers` passed.
- **An arrowhead.** `a:ln/a:tailEnd` is read by `DocxFrames` into `FrameAppearance.TailEnd` and by
  nothing on the sheet path, so every connector in an organisation chart ends bluntly.
  `LineEnds` already draws them.
- **A BIFF shape carrying no ink is still dropped, and it costs the page the reference keeps.**
  Keeping every BIFF shape reproduces the `.xls` fixture's second page and **costs two gate
  verdicts** — `activespecs.xls` 267 pages against 266, `orbus_togaf_tool_csq.xls` 74 against 75 —
  because the BIFF path has neither of LibreOffice's two guards: `IsProcessSdrObj()`'s
  `!mbHidden`, which drops a hidden BIFF object where DrawingML keeps one, and `IsValidSize`'s
  phantom test. A guard on the anchor's own cell span was tried and reaches neither document, so
  the phantoms in them are not zero-span. `XlsShapeInkTests` pins the gap and fails the moment
  either guard lands.
- **Escher's rotation, property 4**, is read by nothing, so a turned `.xls` shape is drawn upright.
  Visible on the `.xls` fixture's elbow, which the `.xlsx` and `.ods` twins both get right.
- **A group's leaves in BIFF carry no `ftCmo`**, so the two ellipses of the `.xls` fixture reach
  neither the model nor the page. The DrawingML and ODF paths both draw them.
- **`a:custGeom`** is not resolved on the sheet path; such a shape falls back to its box.
- **ODF `draw:modifiers`** are still not carried across, so a preset is resolved at its default
  adjustment — `round1Rect`'s corner and `star5`'s point depth differ slightly from the
  reference's on `076_Inventory_list_accessibility_guide`, which is the one shape-class document
  that sits further from 26.2.4.2 than before round 82.
- **A negative `a:avLst` adjustment is clamped and the reference does not clamp it.** Every elbow
  connector in an organisation chart states one — `adj1 = -92308` — and it is the whole of the
  16.6 pt and 9.3 pt residuals in §4. `CustomShapeGeometry`, which the slide and words paths
  share, so this is not a sheets question.

## 9. Files

| | |
|---|---|
| `census.py`, `census-style.txt` | the `xdr:style` half resolved per shape, and every `xdr:` child of an anchor by name |
| `census-ods.py`, `census-ods.txt` | the ODF column's shapes by kind, fill, stroke and whether they carry text |
| `census-shapebox.txt` | `probes/overflow-r69/census.py shapebox` re-run, reproducing the brief's 644 |
| `make-fixture.py` | builds `features/sheet-shape-ink.xlsx`; the `.ods` and `.xls` beside it are 26.2.4.2's conversions |
| `marks.py`, `marks-table.py` | filled and stroked paths per page, and the base/after/reference table |
| `sweep-ours.sh` | render our half, score against a bank, hash against the base bank, delete as it goes |
| `score-bank.py` | score two banked directories with the gate's own rule, rendering nothing |
| `compare.py` | verdict and count movement between two row files, with the failed-row exclusion |
| `orig-sheets-before.tsv`, `orig-sheets-after.tsv` | the sheets track at `6d19fd594` and at this round's HEAD |
| `orig-words-after.tsv`, `orig-slides-after.tsv` | the two untargeted tracks, byte-identical to the base |
| `ods-before.tsv`, `ods-after.tsv` | the converted `.ods` column, round 82's and this round's |
