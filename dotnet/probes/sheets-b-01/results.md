# sheets-b-01 — the user's render-review items, measured

Seven of the user's twenty-four render-review items, taken in the briefed order. **No source was
modified.** Every "ours" figure is a render of the CLI at `HEAD`; every "reference" figure is a
render of the installed `soffice` **26.2.4.2**, made *after* `fonts-dejavu-core` was installed, and
cross-checked page-for-page against `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/` (7 of 7 agree).
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`, one profile per concurrent `soffice`.

**Measured** means read out of a PDF or produced by an authored probe. **Inferred** means read out
of source and reasoned about. The two are separated per item and never mixed in a claim.

---

## 0. The prediction, as committed before measuring

The full text is in `prediction.md` beside this file. It was written after locating the seven
documents and reading `ChartScale.cs`, and **before** opening any PDF or unzipping any `.xlsx`.
It was committed while the brief still said there was no CLI; the CLI arrived mid-round, which
turned four items from inference into direct measurement and is why this file can be as specific
as it is.

Scored honestly:

| # | what I predicted | outcome |
|---|---|---|
| 1 | not `ChartScale`'s arithmetic — the *data* fed to it is the wrong set | **right**, and it is exactly that |
| 1 | …specifically **stacking** | **refuted** — every chart is `clustered`, no stacking anywhere |
| 1 | table/chart border is a border we omit, not one we mis-place | **half right** — table border yes; "chart border" is the reverse, we draw *extra* |
| 2a | the 45° is **automatic**, not stated, and the angle is exactly 45 | **right** on both counts |
| 2a | we have no auto-rotate stage at all | **refuted** — we have the whole ladder, it never fires |
| 2b | possibly a 3-D chart flattened, or a synthesised rectangle | **refuted** — it is a 2-D area chart and the seat is missing-point handling |
| 3 | overflow-vs-clip predicate takes the wrong branch | **refuted** — nothing is clipped differently anywhere in the document |
| 4 | we drop the cell before the style resolves — *"the item I am most confident about"* | **refuted twice**, by probe and by census |
| 5 | wrapping differs because of accumulated advance error; row heights nonetheless exact | **half right** — row heights exact, but the **wrapping is identical too** |
| 6 | the reference colour is **not** `#0000FF` | **right** — it is `#000080`, and the override is unconditional |
| 7 | covered by the known 14-document row-height cluster | **refuted** — the document is exact, 147/147 |

**Verdict movement predicted: zero.** That holds. Five of the seven items are invisible to page
count, extractable words and unembedded fonts by construction. Of the two that could have moved a
verdict (5 and 7), one is exact and the other reproduces the reference's wrapping exactly.

---

## 1. `Keywords_Mapping_Graphs_and_Charts.xlsx` — three separate defects

Reference and ours are both **46 pages**, and the pages align one-for-one, so every comparison
below is page-for-page.

### 1a. "chart vertical scale is very different" — confirmed, and it is a data-plumbing fault

**Measured.** Eleven charts, eleven value axes read out of both PDFs:

| page | reference | ours | |
|---|---|---|---|
| 19 | 0..90 step 10 | 0..25 step 5 | ✗ |
| 21, 22 | 0..40 step 5 | 0..8 step 1 | ✗ **5×** |
| 23 | 0..16 step 2 | 0..16 step 2 | ✓ |
| 25 | 0..35 step 5 | 0..16 step 2 | ✗ |
| 27 | 0..44 step 4 | 0..18 step 2 | ✗ |
| 29 | 0..48 step 6 | 0..14 step 2 | ✗ |
| 31 | 0..14 step 2 | 0..6 step 1 | ✗ |
| 33 | 0..15 step 1 | 0..4 step 1 | ✗ |
| 35 | 0..18 step 2 | 0..4 step 1 | ✗ |
| 37 | 0..4 step 1 | 0..3 step 1 | ✗ |
| 39 | 0..18 step 2 | 0..4 step 1 | ✗ |

Ten of eleven diverge. **The one that agrees, page 23, is the only chart in the file that states a
`c:max`.** That is the whole shape of the fault in one line: when the file fixes the maximum we are
exact, and when the maximum is automatic we are not.

**Measured — the cause.** Every one of the eleven series caches is exactly **one point short of its
declared range**:

```
chart2.xml   c:cat ref='Literature Mapping'!$A$4:$A$16   cells=13  ptCount=12  pts=12
             c:val ref='Literature Mapping'!$B$4:$B$16   cells=13  ptCount=12  pts=12
```

The missing 13th cell is the pivot table's grand-total row. In the stored sheet, `A16` = `Grand
Total`, `B16` = `35`. LibreOffice resolves the range and plots 13 categories; we read the cache and
plot 12. Read straight off the page-22 bar geometry: the reference draws **13** bars, the last
141.19 pt tall on a 4.03 pt/unit axis = **35.0 units**; we draw **12**, and our tallest is 7.

**Measured — the cross-check that fixes it as the whole cause.** Feeding *our own*
`ChartScale.Resolve` the live maximum instead of the cached one reproduces the reference's axis on
**11 of 11** charts, including the two where the reference's top *label* is 18 while the axis
actually runs to 19 — the reference draws **20 gridlines = 19 intervals** on pages 35 and 39, and
labels every second tick, exactly as our resolver + a rhythm of 2 would. Page 33's 16 gridlines = 15
intervals likewise. So `ChartScale` is not at fault in any respect; only its input is.

**Measured — the reference's rule.** `sc/source/filter/oox/excelchartconverter.cxx:65-105`:

```cpp
if (!rDataSeq.maFormula.isEmpty())      { … createDataSequenceByFormulaTokens(aTokens); }  // live cells
else if (!rDataSeq.maData.empty())      { … }                                              // cache, fallback only
```

The formula wins; the cache is the fallback. Note that the *base* `ChartConverter::createDataSequence`
(`oox/source/drawingml/chart/chartconverter.cxx:117-152`) does the opposite and reads the cache —
that is the Impress/Writer path, where the data really is in a second document.

**Seat (source, inferred).**
`dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingChart.cs:363-371` — `ReadSequence` takes
`strCache`/`strLit`/`numCache`/`numLit` and nothing else. The choice is documented as deliberate at
`:341` ("the `c:f` beside it is read past, deliberately, and so is the workbook it names") and
justified at `:352-356` ("doing otherwise would mean opening a second document from inside a reader
that must not depend on the spreadsheet library"). **That justification is correct for PPTX and DOCX
and wrong for XLSX**, where `c:f` names a range in the workbook already open — which is precisely
where LibreOffice splits its two implementations.
`dotnet/src/XRay.Office.Spreadsheets/Ooxml/XlsxCharts.cs:76` calls `DrawingChart.Read(chartSpace)`
with no workbook context, so today there is no seam to hand sheet data through.

### 1b. "table border" — confirmed; the reference invents a border the file does not state

**Measured.** Page 21, black strokes: reference **11**, ours **0** (our 24 black strokes on that
page are all chart-axis furniture at x 491–872). The reference's eleven form a frame at
x 49.38–377.18, y 547.00–758.92 plus a field-button box at x 280.55–376.70, y 772.41–788.43.

**Measured.** The file states **no borders** on those cells: `A3:B17` use `cellXfs` 7, 8 and 9, all
`borderId="0"`, and `styles.xml` defines only two borders.

**Inferred (reference source).** LibreOffice re-materialises the DataPilot and applies its own
frame — `sc/source/core/data/dpoutput.cxx:297-313 lcl_SetFrame`, a 20-twip solid box, called at
`:825` and `:998`, plus `ScDPOutputImpl::OutputBlockFrame` at `:217`. **We are faithful to the file;
the reference is not.** Closing this means re-laying-out pivot tables, which is a much larger piece
of work than 1a and should be costed separately.

### 1c. "chart border" — refuted as stated; the real difference is the opposite sign

**Measured.** We *do* draw the chart-area border: reference
`stroke (-22.73,330.80)-(469.70,727.40) #D9D9D9`, ours `(-23.10,330.56)-(469.96,727.83) #D9D9D9` —
same colour, within 0.4 pt. What actually differs is that **we draw an axis spine and tick marks
that the reference does not draw at all**: page 22 black strokes, reference **0**, ours **24**
(one vertical spine, nine value ticks, one category axis line, thirteen category ticks).

---

## 2. `Template Pilot Logbook JAR-FCL V3.0.xls` — two defects, one shared root

38 pages both sides. The chart is on **page 16** in both.

### 2a. Angled horizontal axis rendered horizontally — confirmed

**Measured.** Text rotation across the whole document, from the `Tm` matrices:

| | 0° | 45° | 90° |
|---|---:|---:|---:|
| reference | 33 | **848** | 33 |
| ours | — | — | — |

Our PDF contains **zero `Tm` operators of any kind**: all our text is axis-aligned, so we render
neither the 45° category labels nor the 90° axis title.

**Measured.** The rotation is **not stated**. Converting the `.xls` through the reference itself
yields `<c:catAx>…<a:bodyPr rot="0"/>`, and the sheet's own cell formats use only
`textRotation="0"` and `="255"`. So the 45° is LibreOffice's automatic rotation.

**Inferred (reference source).** `chart2/source/view/axes/VAxisProperties.cxx:403-408`:

```cpp
void AxisLabelProperties::autoRotate45()
{ m_fRotationAngleDegree = 45; m_bLineBreakAllowed = false; m_eStaggering = SideBySide; }
```

fired from `VCartesianAxis.cxx:937` and `:1068` when a label overlaps its neighbour; otherwise the
tick rhythm is incremented instead. Measured 44.90° in the PDF is 45° after the matrix is rounded.

**Seat (inferred).** We have the whole ladder — `ChartAxisLabels.cs:113` (`AutoRotation = π/4`) and
`:186-202` (`canAdjust` → `rotation = AutoRotation`) — and it is a faithful port. **It never fires
because it is never given the labels.** The category range is `GraphData!$A$2:$A$616`, 615 points;
615 labels across one plot width collide unconditionally. This is the same root as 1a: the axis
sees only what the cache carries, so nothing collides, so nothing rotates.

### 2b. Chart area drawn as a rectangle instead of polygons — confirmed, separate seat

**Measured.** The chart is an `areaChart`, `grouping="standard"` (read back through the reference's
own converter). On page 16 the reference fills the series with a closed polygon:

```
1 1 0.8 rg
599.726 167.668 m
638.844 201.117 l  638.872 201.117 l  639.213 201.117 l
639.213 167.668 l  599.726 167.668 l  h
f*
```

and ours fills it with an axis-aligned rectangle spanning the whole plot at the baseline:
`(153.00, 155.89)-(603.95, 190.34)` — 451 pt wide, 34.5 pt tall, sitting on the axis.

**Measured.** The data is why. The three series declare `GraphData!$C$2:$C$616` etc. — 615 points —
of which only **17** carry a value.

**Inferred — the reference's rule.** `chart2/source/view/charttypes/AreaChart.cxx:691-706`: a point
whose Y is NaN is `continue`d and contributes **no vertex**, so the polygon joins the surrounding
real points directly.

**Seat (inferred).** `dotnet/src/XRay.Office.Core/Charts/ChartLayout.cs:2288-2292`:

```csharp
double value = at < series.Values.Count && series.Values[at] is { } stated
               && double.IsFinite(stated)
    ? stated
    : 0.0;
```

A missing point becomes **`0.0`** rather than being skipped, which pins the polygon to the baseline
at every gap. With 598 of 615 categories empty, our polygon hugs the baseline across 97% of its
width — which is a rectangle. `AddAreas` itself builds a correct polygon; only this fallback is
wrong. Note this is a *different* seat from 2a and from 1a, and fixing 1a alone would make 2b worse
rather than better, because it would deliver 615 points of which 598 are zeros.

---

## 3. `grants-2005.xls` — "header text not cropped to cell size": **refuted**

**Measured, whole document.** Pages 1–78 are the aligned prefix (`first-divergence.py` puts the
first difference at page 79). Across them: **7,780 matched text positions, 0 glyph-count
mismatches.** Not one run is longer in ours than in the reference. Page 1's header row is identical
to 0.03 pt: `"Region"` 6 glyphs at x 60.60/60.62, `"ADO"` 3 glyphs at 105.99/106.01, `"State"` 5
glyphs at 144.43/144.45.

**Measured, past the alignment.** Matching pages by glyph-count signature: **every one of the
reference's 201 pages has an exact twin among our 219.** Nothing is clipped, nothing is lost.

**What is real instead.** Our 18 extra pages are near-empty — our pages 157 onward that lack a twin
carry **1 or 2 text records each**. That is an empty-page defect and it is a page-count matter, so
it belongs to the page-split cluster and I have deliberately not pursued it. Also real on page 79 is
the stroke-coalescing difference recorded in §8.

**Font control.** The reference's extracted text is byte-identical before and after
`fonts-dejavu-core` was installed (`md5 f3a74592a412` both times), and the render uses
LiberationSans only. This item is not a font artefact in either direction.

---

## 4. `sectors-defense-and-aerospace.xlsx` — "empty cells missing shading": **refuted**

This was the item I said in advance I was most confident about. Both independent measurements
refute it.

**Measured — authored probe.** A hand-written minimal `.xlsx`
(`scratchpad/sheets-b/probe/mk/probe4.xlsx`) with (i) a styled empty `<c r="A1" s="1"/>`, (ii) a
styled non-empty control, (iii) an empty cell whose fill is reachable only through `cellStyleXfs`,
(iv) a `<row s="1" customFormat="1"/>` with no cells at all. Rendered both ways:

- **(i) styled empty cell: drawn by both.** We draw it as its own rect, the reference merges it with
  its neighbour into one — a coalescing difference (§8), not a missing fill.
- **(iv) row-level `customFormat` fill with no `<c>` at all: drawn by both**, across the used range.
- **(iii) fill reachable only through `cellStyleXfs`: drawn by neither.** An agreement worth
  recording, because it is the second candidate I named in the prediction and it turns out we match
  the reference by not honouring it.

**Measured — whole-document census.** Every fill rectangle in both PDFs, by colour, with total area:

| | `#FFFF00` count / area | `#C5FFC5` count / area |
|---|---|---|
| ours | 720 / 4,437,517 pt² | 63 / 245,465 pt² |
| reference | 715 / 4,422,886 pt² | 63 / 248,001 pt² |

Within **0.3%** and **1.0%**. There are 572 styled-but-empty filled cells in this workbook across
twelve sheets, and we are not dropping them.

**What the user almost certainly saw.** This document is **227 pages for us against 449** for the
reference. Past the first few pages, page *N* on one side shows an entirely different band of rows
from page *N* on the other, so shaded cells appear where they should not and are absent where they
should be. That is the page-split cluster presenting as a shading symptom. The observation was a
true report of what was on the screen; the shading is not the mechanism.

---

## 5. `T0A0D0000090006XLSE.xls` — "text sizing causing different wrapping": wrapping is **identical**

162 pages both sides.

**Measured.** Every wrapped cell on page 3 breaks at the same word, with the same glyph count, at
the same x, in both:

| reference | ours |
|---|---|
| `(238.79, 759.97)` 29 glyphs `"Types and characteristics of"` | `(238.79, 760.02)` 29 glyphs, same |
| `(238.79, 748.77)` 23 glyphs `"SDH network protection"` | `(238.79, 748.84)` 23 glyphs, same |
| `(238.79, 737.57)` 13 glyphs `"architectures"` | `(238.79, 737.66)` 13 glyphs, same |

…and so on for all fourteen wrapped runs on the page. **Row heights are exact**: row 1 top to row 2
top is 51.14 pt in both.

**Measured — what does differ, and it is much finer than "wrapping".**

1. **Line leading inside a wrapped cell is 11.18 pt for us and 11.20 pt for the reference** — 0.02 pt
   per line, consistent across every multi-line cell. The row height is nonetheless identical, so
   the reference's row height is not simply *n* × leading.
2. **The reference sets each line as many shows, we set it as one**: `"29 glyphs in 16 show(s)"`
   against `"29 glyphs in 1 show"`. That is per-glyph advance rounding to device units — the
   reference re-positions the pen wherever the device-rounded advance departs from the font
   advance. It is the same family as the existing `probes/printer-metric-advance.py` work.

**Font control.** LiberationSans only; the reference's extracted text is byte-identical before and
after the DejaVu install (`md5 4847b41c4d8d` both times). Not a font artefact.

---

## 6. `ans_mappings_of_eccairs_terms.xlsx` — link colour: **confirmed, and fully diagnosed**

**Measured, whole document.** Text fill colours:

- **ours** `0 0 1` = **`#0000FF`**, 342 occurrences; no `#000080` anywhere.
- **reference** `0 0 0.5019607843` = **`#000080`**, 131 occurrences; no `#0000FF` anywhere.

**Measured.** The file *states* `#0000FF`: `styles.xml` `font4` is `<u/><color rgb="FF0000FF"/>`.
We honour it. The reference overrides it.

**Measured — authored probe, and this is the part that makes it actionable.**
`scratchpad/sheets-b/probe/mk/probe6.xlsx` has a hyperlink cell whose font is stated **red**
`#FF0000`, a second hyperlink cell stated **green + underlined** `#00B050`, and two non-linked
controls in the same colours.

- **reference:** both hyperlink cells painted **`0 0 0.5019607843`**; both controls keep their
  stated colour. The override is **unconditional** — it beats red and green alike.
- **ours:** all four keep their stated colour.

**Inferred (reference source).** `svtools/source/config/colorcfg.cxx:534` —
`{ COL_BLUE, Color(0x1D99F3) }, // LINKS` — and `include/tools/color.hxx:443` —
`inline constexpr ::Color COL_BLUE ( 0x00, 0x00, 0x80 );`. The light-mode `LINKS` colour is
`#000080`, which is what the PDF carries to the digit. A Calc hyperlink cell becomes an
`SvxURLField`, and the EditEngine paints a URL field in the configured link colour rather than in
the character colour.

**Seat (inferred), and the predicate already exists.**
`dotnet/src/XRay.Office.Spreadsheets/Layout/SheetLayout.cs:126-146` — `HoldsField(row, column)` — is
exactly the condition under which the reference substitutes the link colour: it already models the
hyperlink cell as one indivisible EditEngine field, already restricts itself to text cells
(`CellAt(row, column) is { Value: null or string }`, matching `insertHyperlink`'s
`CELLTYPE_STRING`/`CELLTYPE_EDIT` gate), and is already consumed for the wrap/clip consequence in
`SheetTextLayout.cs:328` and `:430` and in `SheetOptimalRowHeights.cs:426`. It is **not** consumed
for colour. `SheetCellFormat.Colour` (`SheetCellFormat.cs:150`) carries the stated colour straight
through to the painter. The remark at `SheetCellFormat.cs:140` — "the hyperlink style is an
underlined blue font" — encodes the assumption that the file's colour is the one to use, which is
the assumption the probe refutes.

This is the smallest fix in this round: force `#000080` on text whose cell satisfies `HoldsField`.
It moves no verdict.

---

## 7. `Capability_List…xlsx` — "some cells are taller": **does not reproduce**

**Measured.** 147 pages ours, 147 reference. `first-divergence.py`: *"first diverge — none, every
common page agrees."* `pdf-image-diff.py`: **"147 pages, 0 with major differences."**

**Control, because this is exactly where a font artefact would hide.** The same comparison run on
the **pre-DejaVu** renders of both sides also reports *"147 pages, 0 with major differences"*. So
this is not something the corrected font set fixed — the document was already exact. The reference
render uses Carlito only and never reaches the fallback chain at all.

**Answer to the briefed question.** Neither: this document is not covered by the 14-document
row-height cluster, and it is not a separate axis. At `HEAD` it is exact on every page. The
observation was true when the user made it and has since been closed by other work; there is
nothing left here to fix. Recording it as closed is the result.

---

## 8. Cross-cutting, found three times and not in the brief: we do not coalesce cell borders

**Measured.** LibreOffice merges collinear, identically-styled cell borders into one stroke; we
emit one stroke per cell edge.

`T0A0D0000090006XLSE.pdf` page 3 — reference **19** strokes, ours **103**:

| reference | ours |
|---|---|
| `(53.80, 142.87)-(53.80, 771.45)` — one vertical for the whole table | `(53.83, 719.55)-(53.83, 771.44)` — one per row |
| `(53.43, 771.08)-(380.69, 771.08)` — one horizontal across all five columns | four segments: `53.45→108.94`, `108.19→159.54`, `158.79→237.18`, `236.43→380.73` |

Same shape on `grants-2005` page 79 (reference 60 strokes, ours 370 — and 52 fills against 159) and
on `Keywords_Mapping` page 21 (11 against 24). It is the dominant reported difference class (`box`)
on the first diverging page of both `.xls` documents.

Two consequences that are visible rather than cosmetic: our segments **overlap at the joins**
(`53.45→108.94` then `108.19→…`, 0.75 pt of overlap), which doubles ink on hairline borders; and
our fill rectangles are likewise per-cell where the reference's are per-run, which is why our fill
counts run roughly 3× the reference's on dense pages while the **total inked area matches to within
0.3%** (§4).

---

## 9. What I could not establish

- **`grants-2005`'s 18 extra pages** and **`sectors-defense-and-aerospace`'s 227-against-449**. Both
  are page-split matters and belong to the other track's cluster; I measured them only far enough to
  show that they, not clipping (§3) and not shading (§4), are what the user was looking at.
- **Whether the 0.02 pt/line leading difference in §5 has any reach beyond that document.** It needs
  a leading-specific sweep over the corpus, which I did not run.
- **The cost of §1b.** Drawing the pivot-table frame the reference draws means re-materialising the
  DataPilot; I established that the border is generated rather than stored, and did not scope the
  work.
- **Nothing was built or tested.** No source was modified, so `dotnet build` is untouched and no
  `verify-test.sh` run was owed.

## 10. Verdict movement

**None, as predicted.** A chart's axis range, a chart's tick marks, a pivot frame, an axis-label
rotation, an area polygon's vertices, a hyperlink's colour and a border's segmentation are all
invisible to page count, to the 2% extractable-word band and to unembedded fonts. §1a is
nevertheless the largest fidelity error found in this round — a value axis five times the reference's
on one chart and wrong on ten of eleven — and it is invisible to every gate the project has.
