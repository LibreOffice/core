# Every chart-bearing corpus document, scored by chart type — r186

Measured 2026-09-27 against **LibreOffice 26.2.4.2** (`/opt/libreoffice26.2/program/soffice`,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`), `SOURCE_DATE_EPOCH=0`, one output directory per
document.

## What this is for

A round was asked to review every chart implementation against LibreOffice's source and
implement what is missing. Reading source finds gaps; it does not say which of them matter.
This is the other half: **168 documents holding a DrawingML or chartex chart part, rendered
both ways and grouped by the set of plot elements their chart parts state**, so that a type
that is systematically wrong shows up as a type rather than as a document.

Grouping is by the *set*. A combination chart is its own row rather than being counted under
each of its members, because the ink of a page carrying two plot kinds cannot be attributed
to one of them.

`census-charts.py` produces the input, `sweep.sh` is the run, `rows.tsv` the result
(`kinds`, `path`, `worst |ink|%`, `summed |ink|%`, `pages`, `worst diff%`, `status`) and
`aggregate.sh` the table.

> **The census in the first run was short by one and the reproduction is what found it.**
> `chartdocs.tsv` was originally built with an extension filter that omitted `.xlsm`, so
> `003_Contextures_chart_sample_9bda2719.xlsm` was not swept; it was measured afterwards and
> is row 168. `census-charts.py` filters on the *package contents* and not the extension,
> which is the same trap this project already records for `.xls` charts in BIFF substreams —
> and the reason `probes/chartbiff-r186/census.py` walks OLE2 files rather than `.xls`.

## `|ink|%` is the wrong instrument for a chart, and this round published a wrong conclusion on it first

**The first cut of this table ranked on `|ink|%` and concluded that no chart type is
systematically wrong.** That conclusion was false, and the failure is the instrument's:

- **Both figures are fractions of the whole PAGE.** A chart occupies a small part of one, so
  a chart drawn entirely wrongly on an A4 page that is otherwise white scores almost nothing.
  `pie-chart-result.docx` is **0.30** unsigned ink — and the reference draws a 3-D extruded
  pie there where we draw a flat circle, which is as wrong as a pie can be.
- **`|ink|%` is signed per region before the page's absolute value is taken**, so ink we add
  where the reference removes it cancels *within* the page. A wrong-shaped pie in the right
  colours is the exact shape that cancels.
- `diff%` — the fraction of the page's pixels that differ at all — has neither property. On
  that same document it is **2.76**, and on the round's `021_Unit_Circle_Chart` it is 12.20
  against an `|ink|%` of 5.18.

**Rank a chart on `diff%`.** The project's standing rule, *rank on the worst page and not on
summed ink*, still holds and is not what went wrong here; what went wrong is the quantity.

> A second defect in the first cut, found at the same time: its `aggregate.sh` averaged
> column 4, the **summed** `|ink|%`, while printing it as the worst page's. The `barChart`
> row read 0.76 mean and 15.17 worst and is 0.26 and 1.91. Print the column name you are
> reading.

## The table

| plot elements stated | docs | mean diff% | worst diff% | mean \|ink\|% | worst \|ink\|% |
|---|---:|---:|---:|---:|---:|
| `areaChart,scatterChart` | 2 | 18.63 | 34.37 | 13.80 | 26.78 |
| `barChart` | 55 | 2.81 | 28.50 | 0.26 | 1.91 |
| `lineChart` | 14 | 2.59 | 20.31 | 0.32 | 2.37 |
| `ofPieChart` | 2 | 11.00 | 16.47 | 2.06 | 3.58 |
| `barChart,lineChart` | 14 | 3.57 | 16.01 | 0.45 | 1.91 |
| `doughnutChart` | 13 | 3.32 | 12.39 | 0.38 | 0.90 |
| `pie3DChart` | 3 | 5.06 | 12.20 | 1.84 | 5.18 |
| `scatterChart` | 15 | 2.93 | 11.57 | 0.55 | 2.70 |
| `pieChart` | 15 | 3.57 | 10.49 | 0.61 | 3.06 |
| `barChart,scatterChart` | 1 | 9.83 | 9.83 | 0.60 | 0.60 |
| `barChart,doughnutChart` | 1 | 8.37 | 8.37 | 0.70 | 0.70 |
| `barChart,bubbleChart,doughnutChart,lineChart,radarChart` | 1 | 7.64 | 7.64 | 2.19 | 2.19 |
| `radarChart` | 5 | 2.18 | 5.12 | 0.27 | 0.82 |
| `barChart,lineChart,scatterChart` | 1 | 4.84 | 4.84 | 1.45 | 1.45 |
| `bubbleChart` | 9 | 0.58 | 1.69 | 0.07 | 0.47 |
| `barChart,pieChart` | 2 | 1.25 | 1.66 | 0.28 | 0.42 |
| `areaChart` | 9 | 0.39 | 0.72 | 0.05 | 0.20 |
| `lineChart,scatterChart` | 1 | 0.40 | 0.40 | 0.05 | 0.05 |
| `cx:clusteredColumn,cx:paretoLine` | 2 | 0.16 | 0.17 | 0.01 | 0.02 |

Both `|ink|%` columns are the *worst page's*, averaged and maximised over the group.

## What it says

**Read down the `diff%` column and three groups separate.**

*Types that are right.* `cx:` chartex 0.16, `areaChart` 0.39, `lineChart,scatterChart` 0.40,
`bubbleChart` 0.58, `barChart,pieChart` 1.25. These are at or near the raster floor and need
nothing.

*Types that are right in the main and have individual bad documents.* `barChart` 2.81 over 55
documents with a worst of 28.50, `lineChart` 2.59 over 14 with a worst of 20.31,
`scatterChart` 2.93 over 15, `doughnutChart` 3.32 over 13, `pieChart` 3.57 over 15. A mean
near 3 with a worst near 20 is a per-document layout question, not a type.

*Types that are wrong wherever they appear.* **`ofPieChart` 11.00 over both of its documents**
and **`pie3DChart` 5.06 over all three of its, worst 12.20** — and the 3-D pie is the sharper
case, because all three of its documents are wrong in the same way and two of them
(`pie-chart-result.docx` at 0.30 and `pie-chart-template.docx` at 0.04) look *exact* in the
`|ink|%` column. The reference draws an extruded solid with an elliptical top face and a
darker curved side wall, sized to the plot area's width; we draw a flat circle under half as
wide. Task #19 in this session's list, and the reason the instrument section above exists.

The `areaChart,scatterChart` row at 34.37 is `065_Weight_loss_tracker`, a **measured raster
ceiling**: the reference outlines its turned labels, 120 glyph-sized filled paths against our
12, so closing it would mean outlining glyphs to green a text gate.

The individual pages at 8 diff% or worse:

| diff% | \|ink\|% | document | status |
|---:|---:|---|---|
| 34.37 | 26.78 | `065_Weight_loss_tracker` | **ceiling**, outlined labels |
| 28.50 | 0.50 | `N2_E_Maestroni_Swarm_COP` | open; the anisotropic chart fit's own witness |
| 20.31 | 2.37 | `Demick_JetBlue` | open |
| 18.08 | 1.91 | `Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final` | open |
| 16.47 | 3.58 | `028_Unit_Circle_Chart_Optimized_Graph` | known: ofPie layout, task #20 |
| 16.01 | 1.91 | `171128IPAP` | open; **NOT the `c:smooth` splines** — those closed in round 102, `probes/chart-smooth-r102` |
| 15.93 | 0.94 | `Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack` | open |
| 12.39 | 0.88 | `093_Insightful_Zoom_Chart` | open |
| 12.20 | 5.18 | `021_Unit_Circle_Chart_3D_Pie_Chart` | known: 3-D pie, task #19 |
| 11.57 | 1.47 | `019_Free_Blood_Sugar_Chart_for_Excel` | open |
| 10.49 | 2.79 | `100_Lime_and_Lemon_Data-Driven_Chart_for_PowerPoint` | open |
| 10.48 | 0.62 | `Sylva introduction session` | open |
| 9.83 | 0.60 | `8_P-Pavese_AIRBUS-ATB-journee-CRATB` | open |
| 9.82 | 1.05 | `bitesize-writing-a-report` | open |
| 9.48 | 0.57 | `027_Unit_Circle_Chart_Graphical_Chart` | open |
| 8.37 | 0.70 | `3492.pptx` | open |
| 8.31 | 0.32 | `040_Blood_pressure_tracker` | open |

**Note how little the two columns agree.** The `|ink|%` ranking would have put
`065_Weight_loss_tracker` first and then `021`, and would have shown eleven of these
seventeen at under 1. Six of the seventeen are under 0.7 unsigned ink.

## The census the type review produced

Read from LibreOffice's source rather than measured, and recorded here because the next round
should not re-derive it:

| path | state |
|---|---|
| DrawingML `c:` | all 16 plot elements read; `pie3DChart` and `ofPieChart` are drawn wrongly, the rest measure clean |
| chartex `cx:` | the 8 layout ids of `PlotAreaContext::onCreateContext`; drawn as 26.2.4.2 draws them, 0.16 |
| ODF `chart:class` | **gap** — an unrecognised class drew nothing where the reference draws bars. `probes/chartclass-r186` |
| XLS BIFF | **gap** — no doughnut and no bubble chart. `probes/chartbiff-r186` |

`chart2/source/inc/servicenames_charttypes.hxx` declares nineteen chart types and
`VSeriesPlotter::createSeriesPlotter` (`VSeriesPlotter.cxx`:2882-2907) has a plotter for
eleven; the other eight reach `UnsupportedChart`, which draws
`STR_UNSUPPORTED_CHART_TYPE` and nothing else (`UnsupportedChart.cxx`:88-111). **So a type
LibreOffice "supports" in its service list is not one it draws**, and reproducing those eight
means reproducing a string, not a plot.

**A 3-D chart's reach is three documents and all three are pies.** Censused over every corpus
package holding a `*3DChart` element: `pie3DChart` 3, and **no `bar3DChart`, `line3DChart`,
`area3DChart` or `surface3DChart` anywhere**. So the 3-D work the corpus can witness is the
pie alone, and a general 3-D scene would buy nothing beyond it. All three state
`c:rAngAx val="0"` and no `c:perspective`, which `View3DConverter::convertFromModel`
(`oox/source/drawingml/chart/plotareaconverter.cxx`:262-320) turns into
`RotationHorizontal = clamp(rotX, 0, 90) - 90`, `Perspective = 30/2 = 15` and, because 15 is
non-zero and the axes are not right-angled, `ProjectionMode_PERSPECTIVE`.
