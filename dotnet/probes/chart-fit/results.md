# An embedded chart is fitted to its drawn extent, and three things that had to be true first

Measured 2026-09-07 in `/home/user/wt-chartfit`, branch `agent/chartfit`, base `fdab86b6a`.
Reference `/opt/libreoffice26.2/program/soffice` **26.2.4.2**, its Latin metric duplicates, its
Latin `NotoSans`/`NotoSerif` and its four `LiberationSansNarrow` faces moved aside;
`/usr/bin/soffice` is **24.2.7.2** and is what the banked gate's own reference half was rendered
with. Corpus `/home/user/sample-files`, 947 documents. Ink is the mean absolute grey difference at
30 dpi, page for page, over the shared pages — the same measure as `probes/chart-layout/ink.py`,
taken here from banked renderings so the same documents can be scored three times without
re-rendering the reference.

The brief was `probes/chart-layout/results.md` §0 — the anisotropic fit that round found, named and
deliberately left — plus four residuals from `probes/chart-secaxis` and the category axis' crossing
position. **The fit is implemented and measured, the crossing is implemented and measured, the
date-axis maximum is settled, and the three BIFF residuals are untouched.**

---

## 0. The census, and it reproduces the recorded one

`census.py` counts a chart two ways, because a `c:chartSpace` zip part misses every `.xls` chart:
those live in a BIFF substream, a `BOF` (0x0809) of substream type `0x0020`, in **any** OLE2
stream. Over the 947:

| | |
|---|---:|
| documents carrying a chart | **176** — sheets 99, slides 67, words 10 |
| OOXML chart parts | **309**, in 169 documents |
| BIFF chart substreams | **16**, in 7 documents |

Those are `CLAUDE.md`'s own figures to the document, which is the check that the instrument is
the right one before anything is scored with it.

---

## 1. The fit itself

### The mechanism

`ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithParameters`
(`svx/source/sdr/contact/viewcontactofsdrole2obj.cxx`:88-116) asks
`ChartHelper::tryToGetChartContentAsPrimitive2DSequence` for the chart's primitives **and** for
`aRetval.getB2DRange(…)`, the bounding box of every one of them
(`svx/source/svdraw/charthelper.cxx`:96-100). It then translates that range's minimum to the
origin, scales it by `1/width, 1/height` and multiplies by the OLE object's own matrix. So a chart
whose labels overflow its page is squeezed until the overflow fits, **by two different factors, one
per axis**.

The page rectangle is always inside that range, so the fit can only ever shrink: `formatPage`
(`chart2/source/view/main/ChartView.cxx`:1295-1312) creates a rectangle at `(0, 0)` of the whole
page size on every chart, whatever the page's fill is.

### The seat

`ChartLayout.Place` already composed the chart at its own space and stretched the whole picture
onto the frame, carrying the residual `sx/sy` on `ChartLabel.Stretch`; the rectangle it stretched
*from* was the chart's page. It is now `DrawnExtent`, and `Stretch` subtracts that rectangle's own
origin — which may be negative — exactly as the `createTranslateB2DHomMatrix(-minX, -minY)` above.

### Measured on the witness

`N2_E_Maestroni_Swarm_COP.pptx` page 7. Read off the two PDFs' own filled rectangles, not
`pdftotext -bbox`:

| | chart's own background rectangle | across | down |
|---|---|---:|---:|
| 26.2.4.2 | `119.083 … 719.660` × `92.58 … 516.983` | 0.834135 | **0.948534** |
| this tree, before | `0 … 720` × `92.572 … 540` | 1.0 | 1.0 |
| this tree, after | `1.847 … 720` × `92.572 … 518.002` | 0.99743 | **0.95084** |

in a graphic frame of `0 … 720` × `92.57 … 540.0`.

**The vertical factors agree to 0.24% with no free parameter.** The overflow there is that chart's
value-axis date labels, turned −45°, which both stacks draw the same way and which run below the
chart's page: the fit is what pulls them back onto it.

**The horizontal ones do not, and the reason is not the fit.** That chart's category labels are
drawn by 26.2.4.2 on *one line each*, right-aligned on the plot's left edge, and the longest —
`EFI Step C3 - TII Gain Calibration and Attitude maneuvers [2324]` — reaches `x = 0.02` on the page,
which is 142.7 pt left of the chart's own page. We wrap them into the 127.65 pt band the manual
layout leaves, so ours overflow by 1.86 pt and the horizontal factor is 0.997 where the reference's
is 0.834. **The plot rectangle itself agrees exactly** — ours at 127.65 pt in the chart's own
coordinates against the reference's `(225.581 − 119.083)/0.834135 = 127.65` — so what separates the
two pictures on that axis is entirely which labels were wrapped.

The seat of *that* is `VCartesianAxis::createTextShapes`:888-905: the first label after the first
that breaks **inside a word** makes chart2 set `m_bLineBreakAllowed = false`, throw the shapes away
and start again with breaking off, which is how a chart ends up with labels longer than the band.
`ChartAxisLabels.Wraps` models the same test and answers false here, because none of that chart's
*words* is wider than the band. `probes/chart-layout` §2 raised `lcl_hasWordBreak` and refuted one
of its two arms — a line *starting* with punctuation — on three probe decks; the other arm, an
over-long word, is what this document needs and is still open. **Left, named.**

### Reach, and the ink

Over the 176 chart-bearing documents, rendered with a binary built at the base commit and again
with this one and compared byte for byte with `/CreationDate` masked: **20 renderings move**, and
none of the other 156 changes by a byte.

Against 26.2.4.2, over those 20: **12 improve, 6 worsen, 2 are level**, and the sum of their means
goes **125.09 → 122.38**. The witness's worst page goes **34.977 → 30.617**.

| document | mean before | mean after | worst before | worst after |
|---|---:|---:|---:|---:|
| `057_Simple_balance_sheet` | 3.759 | **2.665** | 9.851 | **6.567** |
| `027_Unit_Circle_Chart_Graphical_Chart.doc` | 12.948 | **11.864** | 12.948 | **11.864** |
| `056_Quarterly_sale_report` | 0.683 | **0.279** | 1.960 | **0.529** |
| `021_Unit_Circle_Chart_3D_Pie_Chart.docx` | 12.202 | **11.999** | 12.202 | **11.999** |
| `037_Personal_money_tracker` | 4.069 | **3.898** | 7.743 | 7.743 |
| `N2_E_Maestroni_Swarm_COP.pptx` | 3.827 | **3.682** | 34.977 | **30.617** |
| `171128IPAP.pptx` | 5.313 | **5.199** | 14.082 | 14.082 |
| `019_Free_Blood_Sugar_Chart` | 5.314 | **5.261** | 14.785 | 15.008 |
| `026_Monthly_cash_flow_statement` | 1.044 | **1.022** | 6.760 | **6.518** |
| `065_Weight_loss_tracker` | 46.863 | **46.831** | 72.318 | 72.318 |
| `southern-classic-kennesaw-state…pptx` | 4.540 | **4.523** | 9.958 | 9.958 |
| `060_Monthly_company_budget` | 7.052 | **7.043** | 14.106 | 14.106 |
| `006_Contextures_chart_sample` | 0.442 | 0.561 | 1.207 | 1.298 |
| `046_Cost_analysis_with_Pareto_chart` | 0.338 | 0.449 | 0.338 | 0.449 |
| `059_Milestone_and_task_project_timeline` | 4.114 | 4.326 | 7.181 | 7.815 |
| `Keywords_Mapping_Graphs_and_Charts` | 1.867 | 1.941 | 5.484 | 5.484 |
| `8_P-Pavese_AIRBUS-ATB…pptx` | 4.349 | 4.419 | 17.372 | 17.372 |
| `055_Project_timeline_with_milestones` | 1.849 | 1.900 | 3.515 | 3.617 |
| `052_Manufacturing_output_chart` | 4.069 | 4.065 | 4.069 | 4.065 |
| `001_Contextures_chart_sample` | 0.448 | 0.451 | 3.854 | 3.913 |

**The six that worsen are one class and it is worth stating plainly: the fit makes our own
label-arrangement errors visible as a global squeeze instead of as local overflow.** Where our
labels overflow the chart's page and the reference's do not, we shrink and the reference does not.
Measured on `046_Cost_analysis_with_Pareto_chart`, whose whole chart is 121 × 94 pt with a manual
inner layout filling it: 26.2.4.2 draws its plot wall at `341.01 … 461.93` × `130.98 … 224.70` and
our unfitted wall was at `341.04 … 461.96` × `130.98 … 224.70` — the same rectangle to a
hundredth — while our 45° category labels reach 9.81 pt below the page where the reference's do
not, so we now fit that chart at `sy = 0.9053`. The fit is right and the labels are the residual;
tuning the fit to hide them would be the wrong repair.

---

## 2. Two things that had to be true before the fit could be

### 2.1 A bar is clipped to its value axis' range

`PlottingPositionHelper::clipYRange`
(`chart2/source/view/inc/PlottingPositionHelper.hxx`:401-415) clamps a bar's two values to the
axis' minimum and maximum and answers false when nothing of it remains; `BarChart::createShapes`
(`chart2/source/view/charttypes/BarChart.cxx`:789) calls it before it computes any geometry and
`continue`s on false, so a rejected point gets no data label either. `AddBars` did neither.

**A stack with a stated minimum is what makes it visible.** A Gantt is written as an invisible
"start" series with a visible "duration" one on top and an explicit `c:min` at the first date, so
the start segment runs from serial zero. On `N2_E_Maestroni_Swarm_COP.pptx` page 7 that is 41 600
days and **192 386 points** left of the plot, and every bar of that chart was drawn from there —
read out of the PDF as `x0 = −192386.875` on all 55 of them.

It is also the prerequisite: an unclipped bar is part of the drawn extent, and fitting a 720 pt
frame onto a 193 000 pt range collapses the chart to nothing.

### 2.2 A label with no ink is not the point (0, 0)

Found by the corpus rather than by the source. `LabelExtent` and `PathExtent` answered
`DocRect.Empty` for a label with no text and a path with no commands, and taking that as a
rectangle is taking the origin. On `048_Expense_trends_budget` — a chart whose content fits its page
exactly — **one** value-axis label that formats to nothing fitted the whole picture at
`sy = 0.6561`, which moved every bar of every one of its fourteen pages. Both now answer null.

### 2.3 A series mark contributes only what falls inside the plot

Every plotter clips its polygon to the scaled logic rectangle before it makes a shape of it —
`Clipping::clipPolygonAtRectangle` at `AreaChart.cxx`:318, 336, 343, 359 and 445, `NetChart.cxx`:138,
144 and 213, `BarChart.cxx`:533, and `VSeriesPlotter.cxx`:1423 for a regression curve — and a
marker is created only for a point `isLogicVisible` answers true for. **We do not clip a polyline
yet**, so taking one's own extent lets a line drawn off the plot decide the whole chart's scale: on
`171128IPAP.pptx` one series runs **932 pt left of a 576 pt chart page**, which fitted that chart at
`sx = 0.3546` where 26.2.4.2 does not fit it at all. `DrawnExtent` therefore intersects a shape's
extent with the plot rectangle, which is exactly what the reference's own range holds.

**The unclipped polyline itself is left**, and it is a real defect of its own: we emit a line
segment 932 pt outside the chart's frame. Clipping the geometry needs
`Clipping::clipPolygonAtRectangle` ported for open and closed paths, which is a round of its own;
the rectangle case is done above because a bar is two numbers.

---

## 3. The category axis' crossing position

`c:catAx/c:crosses val="autoZero"` — the default — asks for value zero on the axis it crosses;
`AxisProperties::initAxisPositioning` turns that into `m_pfMainLinePositionAtOtherAxis = 0.0`
(`chart2/source/view/axes/VAxisProperties.cxx`:224-225), `getAxisIntersectionValue`
(`VCartesianAxis.cxx`:1092-1101) reads it back, and `get2DAxisMainLine` clamps it into the value
axis' range (`:1253-1256`). Under the default `c:tickLblPos val="nextTo"` the labels take the same
line, `getLabelLineIntersectionValue` (`:1103-1113`) falling through to it. And labels drawn inside
the plot take no band off it, because `VDiagram::adjustInnerSize` shrinks the inner rectangle by how
far the drawn labels *overflow* the available one (`VDiagram.cxx`:661-669).

**Measured on `Demick_JetBlue.pptx` page 5**, a column chart running −44 587 … 1 200 000, from the
PDF's own strokes:

| | plot | category axis line | labels |
|---|---|---|---|
| 26.2.4.2 | 214.58 … 401.56 | **374.83**, its own `$-` gridline, drawn twice | turned 45°, band 364.14 … 407.59 |
| before | 215.04 … 376.18 | 376.18, the plot's own edge | turned 45°, band 385.29 … 417.91 |
| after | 215.04 … **405.62** | **378.40** | turned 45° |

Reach: **6 of the 176 chart documents move**, against the 4 a census of `c:val` caches finds — the
census under-counts because an `.xlsx` chart's data comes from the worksheet and not from its
caches. Against 26.2.4.2 over those six: 4 improve, 2 worsen, and the sum of their means goes
**42.09 → 41.79**.

`Demick_JetBlue` is one of the two that worsen, by **0.085**, and the reason is worth keeping: its
geometry is now the reference's, and what is left is that our 45° labels sit about ten points lower
than 26.2.4.2's, so moving them from outside the plot to inside it puts that difference on top of
the bars rather than on empty paper. An ink measure charges for that; the axis is right.

*One reading in this round was wrong and was caught by arithmetic.* Those labels first read as 26
upright runs 32.62 pt wide at a 19.27 pt pitch — overlapping, and an apparent fourth defect. They
are rotated: PyMuPDF reports a **rotated** span's axis-aligned box, which for a 45° label is square,
and the giveaway is the line's own `dir` of `(0.7071, −0.7071)`. Instrumenting `ArrangeCategories`
showed it returning `rot = 0.785` for that chart all along. **Read the direction vector, not the
bounding box.**

---

## 4. The date-axis maximum: `TODAY()`

Settled, in `probes/chart-datemax/results.md`. `055_Project_timeline`'s DATE column is twelve
volatile formulas `DATE(YEAR(TODAY()),m,d)`; LibreOffice recalculates them on load and we print the
cached 2023 values. Substituting `2023` for `YEAR(TODAY())` makes 26.2.4.2 draw **this tree's exact
axis** — fifteen labels, 5 Apr to 23 Aug 2023, ten-day step — and substituting `2026` reproduces the
reference. Nothing in `ScaleAutomatism` or `ChartDateScale` is involved, the stated
`c:majorUnit val="10"` is honoured throughout, and the apparent 30-day step is the label *rhythm*.
It is `CLAUDE.md`'s known volatile-formula class and the same thirteen dates are wrong in the
sheet's own cells before the chart is reached.

---

## 5. What is left, with its reach

- **The BIFF legend's stated position** — `CHLEGEND` dock 7 plus `CHFRAMEPOS`
  (`sc/source/filter/excel/xichart.cxx`:2561-2620), 14 of 16 BIFF substreams across 5 documents.
  Untouched.
- **The automatic chart frame border**, which needs Excel's chart palette. Untouched.
- **`CHMARKERFORMAT`**, the series markers, unread. Untouched.
- **The unclipped polyline** (§2.3), which this round measured at 932 pt outside a 576 pt page on
  `171128IPAP.pptx` and worked around rather than fixed.
- **The label wrap that decides the horizontal half of the fit** (§1), whose seat is
  `VCartesianAxis.cxx`:888-905 and whose second arm `probes/chart-layout` §2 did not test.

## 6. How the five interact

- **The bar clip is under the fit**, not beside it: without it the drawn extent of any chart with a
  stated axis minimum is unbounded.
- **The fit changes what §3 is scored against.** It moves the plot's slot and the drawn label size,
  so the crossing was implemented and measured *after* it, and the two were swept separately —
  20 renderings for the fit and 6 more for the crossing, with `Demick_JetBlue` and `171128IPAP` in
  both.
- **The fit does not change what §4 is about**: that document's ink moves by 0.05 for the fit and
  its axis is a spreadsheet-engine question.

## 7. Both changes together, and the gate

Over the 176 chart-bearing documents, base binary against this one, byte for byte with
`/CreationDate` masked: **24 renderings move** and the other 152 do not change by a byte. Against
26.2.4.2 over those 24: **13 improve, 8 worsen, 3 are level**, and the sum of their means goes
**157.46 → 154.45**.

The whole corpus was re-scored against the banked gate at `/home/user/gate-2f47/` — our half
re-rendered and streamed, the reference half the bank's own, which is sound because a diff confined
to `dotnet/src` cannot reach `soffice`. The verdict rule reproduces **947 of 947** of that gate's
stored verdicts before it scores anything.

**947 of 947 scored: 871 match, 76 mismatch**, against the bank's 860 / 87 at `2f4709c08`.
Thirteen verdicts differ from the bank and **none of them is this round**: each of the 24 moved
documents was scored with a binary built at this round's base as well, and every one carries the
same verdict before and after. The other eleven are commits between `2f4709c08` and `fdab86b6a`,
which is what `probes/chart-layout` and `probes/chart-secaxis` found for the same rows.

| document | the bank at `2f4709c08` | at this round's base | now |
|---|---|---|---|
| `N2_E_Maestroni_Swarm_COP.pptx` | `words` | `match` | `match` |
| `057_Simple_balance_sheet` | `pages,words` | `words` | `words` |

## 8. Verification

| | baseline at `fdab86b6a` | after |
|---|---|---|
| `dotnet build XRay.Office.slnx -v q -nologo` | 0 warnings, 0 errors | **0 warnings, 0 errors** |
| ten non-fidelity projects, run individually and totalled | 5985 / 0 / 0 | **6001 / 0 / 0** — the 16 are this round's own new tests |

Per project after: Containers 109, Core 507, Markup 259, OpenDocument 129, Presentations 966,
Rendering 162, Spreadsheets 1147, Text 723, Vector 302, WordProcessing 1697. Nothing failed once
and no project needed a second run.

## Files

| file | what it is |
|---|---|
| `census.py`, `census.tsv` | every corpus document carrying a chart, counted as a zip part and as a BIFF substream; reproduces `CLAUDE.md`'s 176 / 309 / 16 |
| `render.py` | renders a list of documents in parallel, one directory per document keyed on a hex digest of its path |
| `ink.py`, `ink-movers.tsv` | ink against 26.2.4.2 for three banked renderings — the reference, the base binary's and this one's |
| `sweep.py`, `sweep-after.tsv` | the whole corpus re-scored against the banked gate, our half re-rendered and streamed; the verdict rule is validated against the bank's own 947 stored verdicts before anything is scored |
| `../chart-crossing/census.py`, `ink-crossing.tsv` | the category-axis crossing's own census and its ink |
| `../chart-datemax/` | the `TODAY()` finding |
