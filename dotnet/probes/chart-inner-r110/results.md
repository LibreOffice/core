# r110 — O41 and O42, and the one of them that was never `adjustInnerSize`

Round 110 was handed **O41** (a BIFF gauge chart's plot rectangle) and **O42** (an of-pie's unit
radius) with the finding that both are `VDiagram::adjustInnerSize`, and with an instruction to
re-derive rather than inherit.

Re-deriving moved both seats, in opposite directions.

| seat | state at the end of this round |
|---|---|
| **O42** | **fixed.** The pie's second pass runs for every pie and doughnut, as `ChartView.cxx`:682 does, and is skipped for a chart that states its own inner rectangle, as every call inside that function is. §2. |
| **O41** | **fixed, and it was not `adjustInnerSize`.** The band model was already right; the *space it was computed in* was wrong. A sheet's print zoom was being applied to the chart's rectangle **and to the absolute constants inside it**. §1. |
| the of-pie | **decided, not implemented, and the LibreOffice defect behind it is established.** §3. |

Everything quantitative about the reference below was read out of **26.2.4.2's own resolved
view** — `--convert-to fods` / `fodt`, which writes `<chart:chart svg:width>` (the chart's own
page), `<chart:plot-area>` (the rectangle the diagram was given) and `<chart:coordinate-region>`
(the one `adjustInnerSize` settled on) — or out of a rendering by the same binary. The C++ in this
checkout is `27.2.0.0.alpha0+` and is used only to name a mechanism, never to fix a number.

---

## 0. What the routine is, read by hand

`VDiagram::adjustInnerSize` (`chart2/source/view/diagram/VDiagram.cxx`:653-699, this tree):

```
nDeltaWidth  = available.width  - consumed.width
nDeltaHeight = available.height - consumed.height        // floored at available/3
newSize      = current + delta
                                                          // then the position, from the slack
nDiffLeft = consumed.minX - available.minX;  if >= 0 → newPos.X -= nDiffLeft
…
return adjustPosAndSize(newPos, newSize)                  // intersect with available, then square
```

`adjustPosAndSize` (`:89-127`) intersects the new rectangle with the available one and then, when
the plotter states a preferred aspect ratio — a pie's is 1 — takes
`calculateNewSizeRespectingAspectRatio` and re-centres. `reduceToMinimumSize` (`:635-651`) is what
the first pass is drawn at: one 2.2th of the available rectangle, both axes, offset by that much.

`impl_createDiagramAndContent` (`ChartView.cxx`:557-690) runs it on two paths and one of them is
iterated:

* **an axed chart** — `:586-635` — `createMaximumAxesLabels`, `adjustInnerSize`, re-autoscale,
  `createAxesLabels`, and then up to two more `adjustInnerSize` rounds under
  `bLessSpaceConsumedThanExpected`.
* **a pie or doughnut** — `:682-690` — `if( bIsPieOrDonut )`, once, and the whole series set is
  released and recreated at the result.

Every one of those calls is guarded by `if (!rParam.mbUseFixedInnerSize)`.

**The first of the two is a band model at its fixed point, and that matters.** If the consumed
rectangle is the inner rectangle plus the labels' bands, then
`new = current + (available − consumed) = available − bands`, which is exactly what
`ChartLayout.PlotAreaOf` computes in one step. So "implement `adjustInnerSize` for the axed path"
is not a change of arithmetic at all — and the measurement below says the arithmetic was never the
problem.

---

## 1. O41 — the residual is a coordinate space, not a band

### 1.1 The control that says the band model is right

`014_Contextures_chart_sample_991ecfc5.xls` holds one BIFF chart on a sheet printed at 100%.
26.2.4.2's own `fods` against this tree's own layout, in the chart's own points:

| | `<chart:plot-area>` | `<chart:coordinate-region>` |
|---|---|---|
| 26.2.4.2 | 33.68, 26.45, 477.67 × 184.31 | 59.13, 30.98, **417.09 × 154.66** |
| this tree | 33.65, 26.42, 477.38 × 183.94 | 59.09, 30.93, **416.88 × 154.32** |

**0.35 pt on the worst of eight numbers, with no change to this tree.** Whatever O41 is, it is not
the band arithmetic, and it is not `adjustInnerSize`.

### 1.2 What it is

`EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls`'s sheet is `style:scale-to-X="1"`,
`scale-to-Y="10"` — fit to one page wide — and Calc resolves that to **65%**. The chart is an OLE
object with a page of its own: `<chart:chart svg:width="26.278cm" svg:height="10.638cm">` =
744.89 × 301.55 pt, drawn on the sheet as 484.16 × 195.97 pt. The chart is *laid out at 744.89 and
then scaled*.

This tree laid it out **inside the already-scaled 484.05 pt box**, with the type sizes multiplied
by the zoom to compensate (`SheetChart.Sized`). Everything proportional to the zoom cancels under
that; three families do not.

* **The BIFF unit grid's border gap.** `XclChRootData::InitConversion`
  (`sc/source/filter/excel/xlchart.cxx`:1245-1251) takes `GetHmmFromPixelX(5.0)` = **250** off each
  edge of the *chart's* page and divides the rest into `EXC_CHART_TOTALUNITS` = 4000. Applied to a
  frame that is 0.65 of the chart page, 250 hundredths of a millimetre is **385** in the chart's
  own space — 135 too many, which is **3.83 chart pt** on the left edge and on the top, and a unit
  size of 1.6005 against the correct 1.6474.
* **Every absolute constant in `ChartLayout`.** `AXIS2D_TICKLABELSPACING` is 100 hundredths of a
  millimetre (`chart2/source/view/inc/ViewDefines.hxx`:31) — 2.835 pt, which the reference spends
  in chart space and this tree spent in sheet space, so 4.36 chart pt. `AXIS2D_TICKLENGTH`'s 150
  and a pie's flat 350 margin are the same shape.
* **The 96 dpi whole-pixel em.** A chart's text is measured on a device that instantiates a face at
  a whole number of pixels, so a line height is a step function of the size. The project already
  knew this — `ChartPlot.TypeScale` exists solely to keep the zoom out of the *one* ratio that
  noticed, `IntervalsThatFit` — and it is the general case of the same thing.

Measured on the page-15 gauge, in chart-page points, against 26.2.4.2's own `fods`:

| | left | top | width | height |
|---|---:|---:|---:|---:|
| 26.2.4.2's `<chart:plot-area>` | 9.10 | 35.55 | 654.32 | 266.00 |
| this tree, before | 12.91 | 39.25 | 651.2 | 262.4 |
| this tree, after | 9.10 | 36.20 | 654.15 | 265.49 |

**3.81 and 3.70 chart pt on the two edges the gap is on, predicted to a tenth of a point by
250 × (1/0.65 − 1) = 135 hundredths of a millimetre = 3.83 pt.**

### 1.3 The fix

`SheetChart.Draw` lays the chart out on `box / scale` with its stated type sizes and maps the
finished `ChartDrawing` onto the box afterwards — positions, sizes, stroke widths, dash patterns
and type, one plain scale about the box's corner, which is what scaling the OLE object does.
`Sized` is gone.

### 1.4 What it bought, on the witness

Drawn, page 15, in the PDF's own points:

| | left | right | bottom | top | worst of the 8 gridlines |
|---|---:|---:|---:|---:|---:|
| 26.2.4.2 | 65.19 | 478.94 | 417.00 | 577.04 | — |
| before | 69.06 | 478.06 | 419.42 | 574.44 | 2.95 pt |
| after | **65.18** | 477.73 | 417.50 | 576.88 | **0.50 pt** |

Plot rectangle **409.00 × 155.02 → 412.55 × 159.38** against 413.75 × 160.04: the left edge from
3.87 pt out to **0.01**, the width from 4.75 to **1.20**, the height from 5.02 to **0.66**.

r108 left this seat at "the worst gridline is 2.95 pt and the band arithmetic is what is left".
It is 0.50 pt and the band arithmetic was never touched.

### 1.5 What is left on O41

The **right** band, 6.74 chart pt against the reference's 4.98 — 1.14 pt on the sheet, the whole of
the remaining 1.20 pt width error. It is `EndLabelOverhang`'s half-width of the last category
label, which this tree makes 13.48 chart pt wide against the reference's 9.96. That is the same
few-per-cent label-width disagreement that shows up in §1.6, not a band-model fault.

### 1.6 And one residual the fix introduces

A chart's text is now **measured** at the chart's own size and **drawn** at the zoomed size, and
the 96 dpi quantisation does not commute with the scale, so the two disagree by a fraction of a
per cent. On `057_Simple_balance_sheet_Use_this_template_e2d4cbb2.xlsx` the drawn value labels go
from `8.15w` to `8.17w` and the document's summed `|ink|%` from 0.14 to 1.63 — while its plot
rectangle, in the chart's own space, goes from agreeing with 26.2.4.2's
`<chart:coordinate-region>` by construction to agreeing with it by measurement:
ours **27.99, 50.87, 641.96 × 251.88** against **27.98, 50.85, 641.93 × 251.86**.

Closing it means shaping at the chart's own size and scaling the shaped run, which is what the
reference does and what this tree cannot express without pushing a transform onto the sink — the
one thing `SheetChart`'s own remarks say it deliberately does not do. It is named here and left.

---

## 2. O42 — the pie's second pass runs on the chart type

### 2.1 The gate

`if( bIsPieOrDonut )`, and `lcl_IsPieOrDonut` is `xDiagram->isPieOrDonutChart()`
(`ChartView.cxx`:339-346, :682). This tree gated it on `HasBestFitLabels`, on the reasoning that
only a best-fit label can leave the diagram rectangle. Three measurements refute that, and each is
26.2.4.2's own answer:

* **A pie with no best-fit label anywhere is still shrunk.**
  `027_Unit_Circle_Chart_Graphical_Chart_5462a579.docx` states `outEnd` on every label. Its
  `<chart:coordinate-region>` is **0.876** of the square inscribed in its `<chart:plot-area>`, and
  its *drawn* pie — the union of the wedge fills in 26.2.4.2's own PDF — is
  **350.40 × 352.10 pt**, where this tree drew **406.28 × 390.45**.
* **A doughnut runs the pass and consumes nothing.** All fifteen corpus doughnuts give a
  coordinate region **0.9999** of their inscribed square, because `AVOID_OVERLAP` becomes `CENTER`
  for a ring chart and a ring's labels never leave the wall. So a doughnut must be in the gate and
  must not be measured with this file's single-pie label placer, which would put labels outside it
  and shrink every one of them.
* **A chart that states its own inner rectangle is not adjusted at all.**
  `getAvailablePosAndSizeForDiagram` sets `mbUseFixedInnerSize` from the diagram's
  `PosSizeExcludeAxes` (`ChartView.cxx`:946-981), which `c:layoutTarget val="inner"` and ODF's
  `<chart:coordinate-region>` both set, and every `adjustInnerSize` call is guarded by it.
  `003_Contextures_chart_sample_9bda2719.xlsm`'s pie states one: 26.2.4.2 draws it
  **179.86 × 169.68** and this tree, running the pass on it, drew **142.69** square.

### 2.2 What changed

* the gate is `plot.Kind is Pie or OfPie`, with `PlotArea`/`PlotAreaFraction` null;
* `PieConsumedRect` no longer runs the pie label placer for a **ring** chart;
* `PieConsumedRect` adds the **of-pie's own drawn composition**, which overflows the diagram square
  by design — `m_fLeftShift − m_fLeftScale` = −1.4167 unit radii to `m_fBarRight` = 1.25, so 2.667
  radii against the square's 2 (`PieChart.hxx`:258-269).

### 2.3 What it bought

Measured against the reference's own square, over the 37 corpus documents holding a pie, of-pie or
doughnut chart (70 matched reference charts): **6 charts closer, 1 further, 63 unchanged** — and
the one further draws nothing (see below).

| chart | reference | before | after |
|---|---:|---:|---:|
| `003_Contextures…xlsm` pie (states its own inner rect) | 169.68 | 142.69 | **169.69** |
| `005_Contextures…xlsx` pie | 194.00 | 182.05 | **193.97** |
| `027_Unit_Circle…Graphical` (all `outEnd`) | 352.49 | 402.41 | **354.41** |
| `021_Unit_Circle…3D_Pie` | 279.41 | 325.14 | **274.30** |
| `bitesize-writing-a-report.pptx` chart 0 | 22.42 | 51.76 | **22.68** |
| `pie-chart-result.docx` | 166.79 | 134.16 | 145.15 |
| `pie-chart-template.docx` | 166.79 | 161.32 | 145.15 |

**Three of those seven are charts 26.2.4.2 rasterises, and the round says so rather than banking
the improvement or the regression.** `021_Unit_Circle…3D_Pie` is r102's O31 — its reference page
carries one 522 × 279 pt `image` XObject over the chart and no wedge fill — so its 45.73 → 5.11 pt
of coordinate-region error is real and its **+1.71 of `|ink|%` in §4 is our flat projection against
a raster**, not a geometry regression. And the last two rows: 26.2.4.2 draws both of those charts as a **single image XObject** — one `image`
operator at 190.00 × 144.55 pt and no wedge fill anywhere on the page — so its
`<chart:coordinate-region>` is not what it put on the paper and neither rendering can be scored
against it. They are the same chart in two documents, which is why they now agree with each other,
and `pie-chart-result` improves on ink (0.54 → 0.30) while **`pie-chart-template` renders
byte-identically in both legs** — it is a template whose pie has no values, so its plot rectangle
moves and not one mark does. The one row counted "further" therefore draws no ink at all.

The drawn check on the one document that can carry it, `027`, is the strongest single result here:
the reference's pie is **350.40 × 352.10** at (330.20, 83.70) and this tree's goes from
406.28 × 390.45 at (296.37, 70.00) to **355.29 × 354.10 at (329.36, 95.03)** — the left edge to
0.84 pt.

---

## 3. The of-pie, decided

### 3.1 The LibreOffice defect, established

`PieChart::createBarLabelShape` (`chart2/source/view/charttypes/PieChart.cxx`:780-782, this tree):

```cpp
const double fTextMaximumFrameWidth = 0.8 * (m_fBarRight - m_fBarLeft);   //  0.8 * 0.5 = 0.4
const sal_Int32 nTextMaximumFrameWidth = ceil(fTextMaximumFrameWidth);    //  = 1
```

`m_fBarLeft` = 0.75 and `m_fBarRight` = 1.25 (`PieChart.hxx`:267-269) are **unit-circle radii** and
are never transformed to screen; the value is handed to `createDataLabel` as a maximum frame width
in hundredths of a millimetre. So every bar-of-pie bar label is wrapped into a frame **one
hundredth of a millimetre wide**.

**26.2.4.2 does exactly that**, and this round re-measured it rather than inheriting it: its own
rendering of the unmodified `028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx` draws the
split-off points' captions as **single characters at x ≈ 437.6–440.1, 10.47 pt apart down the
page**, spelling `B r a n c h   3   S t e m   6   L e a f   1 5   4 %` — 26 lines for a
23-character caption, a column 270 pt tall on a 370 pt diagram.

### 3.2 The decision

**This tree does not reproduce it, and its of-pie is therefore legitimately larger than the
reference's.** The arithmetic says the cost exactly, and it is the whole of the residual:

* the reference's available rectangle is 575.43 × 370.53 and this tree's `DiagramAreaOf` is
  575.46 × 370.68 — 0.15 pt, so nothing before the second pass is at fault;
* the reference's `<chart:coordinate-region>` is **271.29** square, which
  `adjustInnerSize` reaches from a first pass of 168.4 square only if the consumed rectangle is
  **267.6 pt tall** — the 26-line label column plus the bar, and nothing else in the chart is that
  tall;
* with bar labels wrapped the way any renderer that is not carrying this defect wraps them, the
  consumed rectangle at the first pass is 224.7 × 168.5, both deltas are positive, and the pass
  hands the diagram back the whole available rectangle: **370.66** square.

So running the pass on `028` changes its **position** and not its size: the of-pie's composition
hangs 0.4167 radii off the left of its square, so the square is pushed right — measured, from
x = 112.31 to **119.33** of the chart's own space, at an unchanged 370.68. Its summed `|ink|%`
against 26.2.4.2 goes **3.47 → 3.58**. That is the measured cost of not reproducing the defect,
and it is one document in the corpus.

The alternative was to wrap a bar-of-pie's bar labels one character per line. It would close the
99.4 pt of diagram-square error on that one document by making our own output unreadable, and the
brief rules it out; this section is here so that nobody re-derives the defect a third time.

---

## 4. Reach, measured by rendering

`confine.py` renders every one of the corpus's **947** documents at the round's base and again
after, under `SOURCE_DATE_EPOCH=0`, one output directory per *document*, and compares the PDFs
byte for byte; the base pass runs with `XRAY_OFFICE_CHART_TRACE=1` so the same sweep records how many
charts each document lays out. Movers are scored against their banked 26.2.4.2 rendering with
`pdf-image-diff.py`, summing the `|ink|%` column over the document's pages.

**42 of 947 renderings move and 905 are byte-identical.** Every mover lays out at least one chart
— 168 of the 947 do — and the split by extension is `xlsx` 34, `docx` 4, `pptx` 2, `xls` 1,
`xlsm` 1, which is what the two changes reach by code: the print-zoom half only a spreadsheet, the
pie half any family.

Summed `|ink|%` over the 42 against their banked 26.2.4.2 renderings: **93.57 → 92.64**, with
**20 better, 11 worse and 11 level**.

| document | base | after | Δ |
|---|---:|---:|---:|
| `027_Unit_Circle_Chart_Graphical_Chart.docx` | 3.22 | **0.84** | **−2.38** |
| `005_Contextures_chart_sample_6e279b08.xlsx` | 1.91 | **0.75** | **−1.16** |
| `EHEST-Pre-departure-checklist….xls` | 5.56 | **5.29** | −0.27 |
| `pie-chart-result.docx` | 0.54 | 0.30 | −0.24 |
| `bitesize-writing-a-report.pptx` | 1.89 | 1.68 | −0.21 |
| … 15 more between −0.16 and −0.01 | | | |
| … 11 level, 6 between +0.01 and +0.10 | | | |
| `028_Unit_Circle_Chart_Optimized_Graph.docx` | 3.47 | 3.58 | +0.11 |
| `063_Sales_pipeline.xlsx` | 1.62 | 1.83 | +0.21 |
| `3495.pptx` | 9.80 | 10.25 | +0.45 |
| `057_Simple_balance_sheet….xlsx` | 0.14 | **1.63** | **+1.49** |
| `021_Unit_Circle_Chart_3D_Pie_Chart.docx` | 3.47 | **5.18** | **+1.71** |

**The two largest worseners are both explained and neither is a geometry regression.**

* `021_Unit_Circle_Chart_3D_Pie_Chart` is **a raster in 26.2.4.2** — one 522 × 279 pt `image`
  XObject over the whole chart and no wedge fill on the page — which is r102's O31 ceiling, and
  our own flat projection is compared against it. Its *geometry* improves at the same time: the
  square goes 325.14 → 274.30 against the reference's coordinate region of 279.41, an error of
  45.73 pt → 5.11. Both numbers are here and neither is qualified away.
* `057_Simple_balance_sheet` is §1.6 — the measure/draw quantisation seam, on a document whose
  plot rectangle now agrees with the reference's coordinate region to 0.03 pt.

**No gate verdict can move, and that is measured rather than asserted.** `gate-movers.py` renders
all 42 movers at both binaries and counts pages with `pdfinfo` and alphanumeric characters with
`pdftotext` — `batch-check.sh`'s column 9, `glyphs`, not its token columns 4 and 8. **42 of 42
have the identical page count and the identical glyph count on both sides**, so the two checks the
gate can see are untouched by construction; the third, font embedding, cannot be reached by a
chart's geometry at all. The 905 byte-identical documents are unchanged in every column trivially.

`gate-movers.tsv` holds the table, with the banked 26.2.4.2 counts beside ours so the *distance*
to the reference is visible as well as the invariance: e.g. `EHEST` 39744 reference against 40006
both sides, `057` 1877 against 2185 both sides.

**The sweep ran while two peer worktrees were sweeping and building**, at a load average of 15-27
throughout. That cannot bias it: both legs are our own renderer, deterministic under
`SOURCE_DATE_EPOCH=0`, with no `soffice` in the loop and no timeout reached — the failure mode
`dotnet/CLAUDE.md` records for contention is a *reference* render timing out, and there is no
reference render here.


---

## 5. Deliverable state

Built clean, `TreatWarningsAsErrors` on, **0 warnings 0 errors**.

The ten non-fidelity projects, one at a time, totalled from this run's own output:

```
Containers 109   Core 567   Markup 259   OpenDocument 160   Presentations 1107
Rendering  164   Spreadsheets 1337   Text 728   Vector 309   WordProcessing 1938
                                                   6678 passed, 0 failed, 0 skipped
```

`XRay.Office.Fidelity.Tests`: **552 discovered, 542 passed, 10 failed, 0 skipped** — and the failure
set is **identical at the round's base**, name for name, checked by building the base and running
the project again (`fid-base.txt` against `fid-head.txt` in the round's work directory). All ten
are the families `dotnet/CLAUDE.md` records as failing on purpose: `TabStopComparisonTests`'
`list-label-overrun` in four formats and `PageDrawingComparisonTests`' `paginated` in four, which
are the PDF glyph-positioning channel's own resolution; `SheetDrawingComparisonTests`'
`sheet-rich-text.xlsx`, which is 26.2.4.2 clamping a full-cell anchor offset; and
`JustificationShrinkComparisonTests`. **None of the ten is a chart comparison.**

**New tests, 11 in all.**

`XRay.Office.Core.Tests/ChartPieSecondPassTests` — seven cases: an `outEnd`-labelled pie with no
best-fit label anywhere is shrunk; a centred-label pie keeps the whole square; a doughnut is not
shrunk whatever its labels say (a three-case theory); a stated inner rectangle is not adjusted; and
an of-pie's own composition is in what the pass measures.
`XRay.Office.Spreadsheets.Tests/SheetChartPrintZoomTests` — four: the chart at 65 %, 50 % and 40 % is
the 100 % chart scaled about the box's corner to a hundredth of a point, and an unzoomed chart
takes the box it is given.

**Five of the eleven fail at the round's base**, checked by building the base with the new tests
against it: the two pie ones whose subject is the widened gate, and the three zoom ratios. The
other six pass at base and are guards rather than witnesses — the doughnut theory and the stated
rectangle exist to stop the widened gate reaching where the reference does not run the pass, and
they are exactly the cases an intermediate version of this change got wrong (running the pass on a
stated inner rectangle took `003_Contextures`' pie to 142.69 square against 179.86).

**One assertion in these tests had to be a ratio and the reason is worth carrying.**
`ChartLayout.Place` squeezes the finished drawing to its own drawn extent afterwards — by two
factors, one per axis — so a pie whose labels overflow is smaller in the finished drawing *whatever
the second pass did*, and an assertion on a length measures the fit rather than the pass. The fit
scales the plot rectangle and the diagram rectangle by the same per-axis factor, so their ratio is
invariant under it. The first cut of these tests asserted on lengths and failed on the doughnut for
that reason alone.


---

## 6. Files here

* `results.md`.
* `confine.py` — §4, run over all 947 corpus documents.
* `refrects.py` — 26.2.4.2's own chart page, `<chart:plot-area>` and `<chart:coordinate-region>`
  for every chart in a document, out of `--convert-to fodt/fods/fodp`. This is the instrument the
  round is built on and it costs two to eight seconds a document.
* `ourrects.py` / `match.py` — the same rectangles out of this tree, through a temporary
  `XRAY_OFFICE_CHART_TRACE` hook in `ChartLayout.Draw` that is **not committed**: it printed the
  frame, the diagram rectangle and the plot rectangle to stderr, and was removed before the final
  build. Re-add it as three lines before `boxes.Add(new ChartBox(area, wall))` to reproduce
  anything here.
* `pie-refrects.tsv` — 96 reference charts from the 37 corpus documents holding a pie, of-pie or
  doughnut.
* `pie-shrink.tsv` — the §2.3 table in full: every reference pie and doughnut matched to ours,
  before and after.
* `gate-movers.py` / `gate-movers.tsv` — §4's page and glyph counts over all 42 movers.
* `confine-rows.tsv` — the §4 sweep's own 947 rows.

---

## 7. What this round could not settle

* **The of-pie's 99.4 pt**, deliberately. §3.2. It is now `L3` in the register.
* **The last category label's half-width overhang**, §1.5 — 1.14 pt on the EHEST gauge's right
  edge and the whole of what is left of that seat.
* **Measuring at the chart's own size and drawing at the zoomed one**, §1.6. The seam is named and
  the one document that scores it is measured.
* **`3495.pptx`, +0.45.** Its four pies' *available* rectangles are 118, 90, 171 and 68 pt wide
  against 26.2.4.2's 231, 216, 212 and 209 — a legend or title reservation defect that predates
  this round and that nothing here touches. The second pass then shrinks three of them further,
  which is the right routine applied to a wrong rectangle. Seated nowhere yet.
* **Whether a chart the reference rasterises should be scored at all.** Three of the corpus's pies
  are a single image XObject in 26.2.4.2's own output, as five of the six 3-D charts are (r102 §2).
  Nothing in the harness knows that, and all three sit in the tables above carrying numbers that
  cannot mean what they appear to — including the largest single worsening in §4.
