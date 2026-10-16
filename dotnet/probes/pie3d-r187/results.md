# The one three-dimensional chart there is anything to measure — r187

Measured 2026-09-28 against **LibreOffice 26.2.4.2** (`/opt/libreoffice26.2/program/soffice`,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`), `SOURCE_DATE_EPOCH=0`. The C++ read is this
checkout, which is 27.2.0.0.alpha0+ and **not** the reference binary's source; every arm is
confirmed a second time against 26.2.4.2's own output.

## 0. What the chart review found

`DrawingChartPlot.KindOf` reads `c:pie3DChart` as `ChartPlotKind.Pie` and nothing else, so every
three-dimensional pie was drawn as a flat circle. It is the largest *shape* error the chart
sweep holds — the reference draws an extruded solid with an elliptical top face and a darker
curved wall, filling the plot area's width, and this tree drew a circle under half as wide.

**The reach is three documents and all three are pies.** Censused over every corpus package
holding a `*3DChart` element: `pie3DChart` 3, and no `bar3DChart`, `line3DChart`, `area3DChart`
or `surface3DChart` anywhere. A general 3-D scene buys nothing beyond the pie.

**And the `|ink|%` column could not see it.** `pie-chart-result.docx` scores 0.30 unsigned ink
and 2.76 differing pixels with a wholly wrong chart on it, because both are fractions of a page
that is mostly white and the signed ink cancels within it. That instrument correction is
`probes/chartsweep-r186` §"`|ink|%` is the wrong instrument for a chart".

## 1. 26.2.4.2 rasterises a 3-D chart

The pie is not vector geometry in the reference's PDF at all. Its whole content stream for
`pie-chart-result.docx` holds 30 `m`, 86 `l`, **no `c` at all**, and one

```
q 190 0 0 144.55 223.85 539.289 cm
/Im9 Do Q
```

— a 791 × 602 `DeviceRGB` image with a soft mask. The labels and the chart's frame are still
operators; the solid is a bitmap. Two consequences. **The geometry has to be read out of the
silhouette**, which `geometry.py` does. And **a vector rendering can never be byte-equal to
it**, so a residual against this document is not a defect to chase to zero.

## 2. The measurement

`variants.py` writes 36 one-attribute rewrites of `021_Unit_Circle_Chart_3D_Pie_Chart`'s own
chart part — nine `c:rotX` by four `c:depthPercent`, the chart part alone touched —
`render-ref.sh` renders each through 26.2.4.2, and `geometry.py` fits each solid's outline.
`geometry.tsv`:

| rotX | 2A | B/A | sin(rotX) | depth | ellipse fit, mean px |
|---:|---:|---:|---:|---:|---:|
| 10 | 478.4 | 0.1640 | 0.1736 | 52.43 | 0.28 |
| 20 | 479.6 | 0.3401 | 0.3420 | 49.79 | 0.31 |
| 30 | 468.9 | 0.4989 | 0.5000 | 43.46 | 0.33 |
| 40 | 385.3 | 0.6475 | 0.6428 | 30.08 | 0.34 |
| 50 | 340.7 | 0.7626 | 0.7660 | 19.87 | 0.32 |
| 60 | 307.9 | 0.8655 | 0.8660 | 13.21 | 0.41 |
| 70 | 289.7 | 0.9390 | 0.9397 | 7.92 | 0.45 |
| 80 | 289.7 | 0.9653 | 0.9848 | 1.25 | 0.41 |
| 90 | 280.3 | 0.9974 | 1.0000 | 0.11 | 0.42 |

Four things fall out of it.

**The projection is parallel in practice.** The upper boundary fits an ellipse to a mean
residual of 0.28 to 0.45 pixels on images 1370 across, at every elevation. A conic with real
perspective in it would not, and the residual is how one would know — so although
`View3DConverter::convertFromModel` asks for `ProjectionMode_PERSPECTIVE` (a `c:perspective` of
30 halves to 15, and 15 is not zero, `oox/source/drawingml/chart/plotareaconverter.cxx`
:299-308), the camera is far enough away that it does not show.

**The squash is `sin(rotX)`**, within 0.7% at seven of the nine. The two that miss are where the
silhouette is hardest to fit — at 10° it is nearly a line and at 80° nearly a circle.

**`c:depthPercent` is not read.** All four values give geometry identical to the hundredth of a
point at every one of the nine elevations. The 36 rows of `geometry.tsv` are nine distinct
answers repeated four times each.

**The solid is fitted to the plot area isotropically, on its own projected bounding box.** A unit
disc of diameter 1 and thickness 0.10 — `PieChart::getPreferredDiagramAspectRatio`'s
`Direction3D(1, 1, 0.10)`, `chart2/source/view/charttypes/PieChart.cxx`:252-256 — projects to a
box 1 wide and `sin θ + 0.10·cos θ` tall, and the scale is whichever limit binds. Against the
plot rectangle this document gives (478.4 wide, 280 tall):

| rotX | predicted 2A | measured | predicted depth | measured |
|---:|---:|---:|---:|---:|
| 20 | 478.4 | 479.6 | 45.0 | 49.8 |
| 30 | 477.4 | 468.9 | 41.3 | 43.5 |
| 40 | 388.0 | 385.3 | 29.7 | 30.1 |
| 50 | 337.2 | 340.7 | 21.7 | 19.9 |
| 60 | 307.7 | 307.9 | 15.4 | 13.2 |
| 70 | 287.5 | 289.7 | 9.8 | 7.9 |
| 90 | 280.0 | 280.3 | 0 | 0.1 |

**The width is within 2% at every elevation and the changeover from width-limited to
height-limited between 20° and 30° is reproduced.** The depth is the weaker half — right to a
fifth at the ends — and it is a twentieth of the figure's height, so what it costs on the page
is small. That residual is the one part of the model that is fitted rather than derived: a
constant `z` of 0.10 predicts a `depth/(2A·cos θ)` of 0.05 and the measured value drifts from
0.111 at 10° to 0.080 at 70°. Nothing in `VDiagram::adjustAspectRatio3d` explains the drift and
it is left named.

## 3. The shading

Counted over every opaque pixel of the corpus document's bitmap, matched against the theme
accents the chart part names:

| drawn | of the declared colour | what it is |
|---|---:|---|
| (108, 165, 68) | accent6 × **0.959** | a top face |
| (87, 148, 203) | accent5 × **0.955** | a top face |
| (102, 157, 64) | accent6 × 0.907 | the front of its wall |
| (76…82, 128…139, 176…192) | accent5 × 0.83…0.90 | its wall, turning |

So the top face is a flat **0.957** and the wall runs from about 0.83 at the sides to 0.91 at
the front, which is one light turning with the surface. A single flat **0.87** is used for the
wall rather than a lighting model: the band is narrow and what the page shows is the shape.

> **`(64, 99, 41)` is not a wall.** It is accent6 × 0.574 and it occupies the upper-left
> quadrant — the 10% sector's *top face*, in a darker shade of the same accent, which is what
> `c:varyColors` hands the fourth point. Reading it as the wall's shade would have made every
> wall 0.57 of its face. Locate a colour's pixels before deciding what surface they are.

## 4. What was implemented, and the one thing that made it fit

`ChartPlot.Elevation`, `DrawingChartPlot`'s read of `c:view3D/c:rotX`, and
`ChartLayout.Pie3D` — the fit, the sector faces, the back-face cull against the view vector
`(0, −sin θ, cos θ)`, and the painter's order, which is exact rather than heuristic because the
solid is convex and seen from above.

**The change that mattered most is none of those: a three-dimensional diagram is not squared.**
`VDiagram::adjustPosAndSize` branches on the dimension count
(`chart2/source/view/diagram/VDiagram.cxx`:89-101) and `adjustPosAndSize_3d` fits the scene's own
projected bounding box into the available rectangle (`:409-421`), so the preferred ratio never
squares anything. `ChartLayout.Squared` squared every pie. With the solid drawn but the
rectangle still squared, `021` came out 274 pt across against the reference's 469 and its
`diff%` went **12.20 → 16.33, worse than before the feature**; with the square removed it is
**5.64**.

## 5. Reach and cost

`measure.sh`, before and after:

| document | diff% before | after | \|ink\|% before | after |
|---|---:|---:|---:|---:|
| `021_Unit_Circle_Chart_3D_Pie_Chart` | 12.20 | **5.64** | 5.18 | **1.07** |
| `pie-chart-result` | 2.76 | **1.66** | 0.30 | **0.22** |
| `pie-chart-template` | 0.21 | 0.21 | 0.04 | 0.04 |

**Confinement: 2 of the 168 chart-bearing corpus documents move and 166 are byte-identical**,
rendered at the round's base and again with the change, `SOURCE_DATE_EPOCH=0`, one output
directory per document. The 166 include all 15 `pieChart`, all 13 `doughnutChart` and both
`ofPieChart` documents, which is what exercises the `PieLabels` refactor that threads the rim's
vertical semi-axis through; every flat call site passes the radius for it, so the flat path is
an identity.

**`pie-chart-template` does not move, and that is correct**: its one series' cached value is 0,
so the total is zero and neither renderer draws a pie at all. Its 0.21 was never this chart.

On `021` the drawn solid is now **493.8 × 289.7 pt centred at x 291.6** against the reference's
**469.3 × 278.0 at 289.5** — 5% wide, 4% tall, 2 pt right.

## 6. What is left on that document

Two things, both separate:

* **Its labels are 14 pt bold and we draw them at the plot default.** The size and weight are
  stated only on each *per-point* `c:dLbl`'s own `c:txPr` — `sz="1400" b="1"` — and never on the
  series-level `c:dLbls`, which states only `c:dLblPos`. `DrawingChartPlot` reads the series and
  plot levels. That is a reader gap of its own and wants its own reach census.
* **The 5% of width.** The fit law's width is within 2% across the probe's nine elevations, so
  the residual here is more likely to be in the plot rectangle handed to it than in the fit.

## Files

| file | what it is |
|---|---|
| `variants.py` | 36 one-attribute rewrites of one corpus chart part |
| `render-ref.sh` | renders them through 26.2.4.2, three at a time |
| `geometry.py` | fits a solid's silhouette out of the reference's own bitmap |
| `geometry.tsv` | §2's table, as measured |
| `measure.sh` | scores the three corpus `pie3DChart` documents both ways |
