# r104 — the block clip's push, the chartex reader, and the of-pie's second condition

Round 104, the chart track's last three seats: **O26** (which charts escape the block clip),
**O36** (a chartex chart draws an empty frame) and **O38** (a second gate on the pie-of-pie form).
Worktree `/home/user/wt-chartrest`, branch `agent/chartrest`, base `7d5b130a2`.

Reference `/opt/libreoffice26.2/program/soffice` — LibreOffice **26.2.4.2**.
`/usr/bin/soffice` is 24.2.7.2 and was not used for anything. The reference *renderings* are the
bank at `/home/user/gate-orig-r83/ref` (947 PDFs), reused rather than re-rendered; every
`--convert-to` and every variant render below was run fresh through 26.2.4.2 itself.

**The C++ tree at `/home/user/libreoffice-core` is 27.2.0.0.alpha0 and is not the reference
binary's source.** Every `file:line` below is *this tree* and could not be checked against 26.2.
That is why each arm has a second leg taken out of 26.2.4.2's own output, and those legs stand on
their own.

**There is no `Task`/subagent tool in this container**, so no page was read by anyone
uncontaminated. Nothing below rests on a reading: every figure is a count taken out of a PDF's
content stream, out of a rasterised page, or out of 26.2.4.2's own flat ODF.

| seat | ends |
|---|---|
| **O26** | **closed as nil reach for a rule of its own shape, and the question it asks is answered.** The block clip *is* on the device's push/pop stack — `Push(CLIPREGION)` + `IntersectClipRegion`, once per **layer**, not per object. What replaces it is neither `MetaClipRegionAction` (the seat's first reading) nor a `MetaPopAction` out of `emfio` (round 99's correction): it is `VclMetafileProcessor2D::processMaskPrimitive2D`'s `SetClipRegion`, which is a *replace* over a clip polygon that never contains the block. So the escaping unit is **a mask, not an object kind** — and round 99's "no chart in the corpus demonstrably escapes" is refuted on the same document it was written about. |
| **O36** | **closed, fixed in this tree.** A `cx:chartSpace` is read and drawn. `\|ink\|%` **0.14 → 0.00** and **0.50 → 0.03** on the two witnesses; the drawn polylines match 26.2.4.2's own corner for corner to **0.05 pt** and their colours exactly. |
| **O38** | **the seat's premise is refuted and there is no second condition.** The gate is exactly `nPointCount >= 4`, identically for both forms, measured over 28 one-attribute variants — and this tree already implements it. Round 102's contrary reading was an instrument artefact and is retracted. |
| **new, seated** | **the of-pie's unit radius is wrong and the corpus can see it.** 1.37× the reference's on the one drawn of-pie, worth `\|ink\|%` 3.47. §4.1. |

---

## 0. What in the brief did not survive its own round

Four of the brief's claims are corrected in place below.

* **"No chart in the corpus demonstrably escapes"** (O26, from `chart-resid-r99` §3.4). On
  `DynamicBubbleChart` the escaping ink on *both* over-clipped pages is the chart's own bubble
  markers, each drawn inside its own clip; on page 2 those clips run to x −76 where the printed
  block starts at 50.400. The two `xdr:sp` r99 named are one slicer fallback, and the light-blue
  ink beyond the block edge on page 1 is a **cell** background, which no drawing clip touches.
  §1.4.
* **"The most likely correct outcome is that this closes as an EMF-picture question."** It does
  not. **2 of the 39 over-clipping documents hold any vector media at all** — they carry 17,269 of
  the 31,384 pixels, 55 % of the total and 61 % of what is left once the boundary artefacts are
  removed — and the other 37 hold none. §1.5.
* **"The reference resolves both witnesses to a full `ClusteredColumnChartType` diagram — 4
  series, 4 data points"** (O36, from `chart-types-r101` §6). Four series, of which **two** carry
  an **eleven**-cell value range and two carry none, over **eleven** categories. And what
  26.2.4.2 *draws* is far less than what it resolves: a white plot rectangle and **two eleven-point
  polylines**, no columns, no axis, no label, no legend, no title. §2.1.
* **"With the explosion removed, 26.2.4.2 still draws `029` as a plain pie at four, five, six and
  seven points"** (O38, from `chart-smooth-r102` §4.5). It draws an of-pie at every one of them.
  §4.1.

---

## 1. O26 — the block clip is a push, and what discards it is a mask

### 1.1 Where the block clip comes from, and that it is on the push/pop stack

The seat's sharpened question was *"is the block clip on the device's push/pop stack when
`SdrPageView::DrawLayer` replays a metafile, and which object kinds get a push in front of it?"*
The first half is yes and the second half is *none* — the push is once per layer.

The chain, in this tree:

* `ScOutputData::PrePrintDrawingLayer` (`sc/source/ui/view/output3.cxx`:41-102) builds `aRect`
  from the page's own printed columns `mnX1…mnX2` and rows `mnY1…mnY2`, converts it to 1/100 mm,
  and hands it to `pLocalDrawView->BeginDrawLayers(mpDev, aRectRegion, true)` — so the printed
  cell block becomes the paint window's **redraw region**.
* `SdrPageWindow::RedrawLayer` (`svx/source/svdraw/sdrpagewindow.cxx`:365-425) copies
  `GetPaintWindow().GetRedrawRegion()` into `aDisplayInfo.SetRedrawArea(rRegion)`.
* `ObjectContactOfPageView::DoProcessDisplay`
  (`svx/source/sdr/contact/objectcontactofpageview.cxx`, the `#114359# save old and set clip
  region` hunk) does, for the whole layer:

  ```cpp
  if(!rRedrawArea.IsEmpty())
  {
      bClipRegionPushed = true;
      pOutDev->Push(vcl::PushFlags::CLIPREGION);
      pOutDev->IntersectClipRegion(rRedrawArea);
  }
  ```

  …processes every primitive of every object, and pops once at the end.

Two things follow that the seat did not have. **Calc itself sets no clip around the drawing layer
at all** — `ScPrintFunc::PrintArea` writes *"don's set Clipping here (Mapmode is being moved)"*
(`sc/source/ui/view/printfun.cxx`:1650) and the one `SetClipRegion` near it, at `:1681`, is
immediately cleared at `:1683`. And **the block clip is pushed once for the layer, not per
object**, so no object kind "gets a push in front of it": what an object can do is replace it.

### 1.2 What replaces it is a mask, and the EMF's pops never reach the page

Round 99's correction — that `MtfTools::UpdateClipRegion` emits a `MetaPopAction` before every
metafile clip change — is right about `emfio` and does not describe what reaches the page.
Those actions go into the **EMF's own** `GDIMetaFile`, and
`MetafilePrimitive2D::create2DDecomposition`
(`drawinglayer/source/primitive2d/metafileprimitive2d.cxx`:38-70) hands that file to
`wmfemfhelper::interpretMetafile`, which turns it into primitives. The page's `OutputDevice`
never sees a pop from it.

What the page's device *does* see is this, once per mask, in
`VclMetafileProcessor2D::processMaskPrimitive2D`
(`drawinglayer/source/processor2d/vclmetafileprocessor2d.cxx`:2395-2453):

```cpp
auto popIt = mpOutputDevice->ScopedPush(vcl::PushFlags::CLIPREGION);
mpOutputDevice->SetClipRegion(vcl::Region(maClipPolyPolygon));
```

`SetClipRegion` **replaces**; `IntersectClipRegion` would not. And `maClipPolyPolygon` is the
processor's own accumulator — default-constructed empty, built only from masks seen inside this
primitive tree — so it never contains the block. The scoped push restores the block afterwards,
which is why the reference's own stream shows the two clips **alternating** rather than the block
disappearing for the rest of the page.

So the original seat's instinct (a *replace*, not an intersect) was right about the action and
wrong about the module: it is `drawinglayer`, not `emfio` and not `vcl/source/gdi/metaact.cxx`.

**This is read in a tree that is not the reference binary's source.** §1.3 and §1.4 are the
rendered leg and stand alone.

### 1.3 The rendered leg: the reference's own clip in force at every painting operator

`o26-clipstate.py` walks a page's content stream, keeps the `q`/`Q` stack, accumulates a clip
bounding box from **any** path a `W`/`W*` closes — not only from `re`, which is what
`chart-resid-r99`'s `clipops.py` saw — and tags every painting operator with the clip in force.
`o26-clipstate.txt` holds ten pages.

On `012_Contextures_chart_sample` page 1, where the block clip removes 1,470 px the reference
keeps:

```
  NO CLIP                                    n=  6  {'S': 4, 'Tj': 2}      the sheet's own cells
  (   0.000,   0.028)-( 611.972, 792.000)    n=  1  {'f*': 1}              the page
  ( 118.942, 448.299)-( 583.739, 668.806)    n= 18  {'Tj': 1, 'TJ': 17}    the EMF's own clip
  ( 119.977, 448.305)-( 583.739, 668.059)    n=  2
  ( 128.259, 464.749)-( 569.262, 666.564)    n= 32
  ( 177.949, 489.415)-( 499.902, 655.352)    n=  3
```

**The block rectangle does not appear on that page at all.** The picture's own clip, which runs
to x 583.7, is what is in force for everything it draws.

### 1.4 A chart does escape, and the same chart's other ink does not

`DynamicBubbleChart.xlsx` page 2, same instrument:

```
  (  50.400, 122.995)-( 431.291, 738.000)  n= 45  {'f*': 2, 'S': 15, 'Tj': 19, 'TJ': 9}
  ( -76.085, 460.102)-( -32.459, 503.681)  n= 29  {'f*': 29}
  ( -34.639, 430.040)-(  13.177, 477.805)  n= 28  {'f*': 28}
  (  -25.778, 459.310)-(  28.039, 513.070) n= 21  {'f*': 21}
  (  23.170, 559.675)-(  74.072, 610.522)  n= 15  {'f*': 15}
  …twelve more of the same shape…
```

The first row is the **block** clip and it holds the chart's axis strokes and its text; the rows
under it are the chart's **bubble markers**, each drawn inside a clip of its own, and four of
them lie wholly or partly left of the block's 50.400. The reference's rasterised page carries
9,078 ink pixels left of that edge at 120 dpi and 24,004 beyond the right edge on page 1.

So one object, one page, two answers, and the discriminator is the mask — which is exactly what
§1.2 predicts. The bubbles are masked because their fill is drawn as a band gradient: 15 to 29
`f*` per bubble, greys stepping 0.690 → 0.898, inside a clip that is the bubble's own outline.
`PolyPolygonGradientPrimitive2D::create2DDecomposition` returns
`new MaskPrimitive2D(getB2DPolyPolygon(), …)`
(`drawinglayer/source/primitive2d/PolyPolygonGradientPrimitive2D.cxx`), and
`VclMetafileProcessor2D::processPolyPolygonGradientPrimitive2D` forces that decomposition for a
PDF export outright — *"tdf#150551 for PDF export, use the decomposition for better gradient
visualization"*.

**Round 99 §3.4's reading of this document is withdrawn.** It attributed the escape to two
`xdr:sp` shapes; the document holds one `graphicFrame` and one `mc:AlternateContent` whose
fallback is a single slicer-placeholder `xdr:sp`, and 11,956 of page 1's 24,004 escaping pixels
are the light-blue `(198, 217, 241)` fill of two **cell** rows, which `ScOutputData` draws
outside the drawing layer under no block clip at all.

### 1.5 The census, and the base rate beside it

`o26-objects.py` takes every rendering in `clip-seats-r97/overreach.tsv` with `over_px > 0` — the
census that established the seat's reach — and asks the source document what it holds.
`o26-objects.tsv`, 39 documents over 61 pages, 31,384 px.

| | pages | px | share |
|---|---:|---:|---:|
| the span is one column ≤ 1 pt wide — a boundary artefact | **36** | 2,864 | 9 % |
| the two documents holding an EMF (`012`, `013`) | 4 | 17,269 | 55 % |
| `DynamicBubbleChart`, whose escaper is its chart | 2 | 8,642 | 28 % |
| the remaining 36 documents, none holding any vector media | 19 | 2,609 | 8 % |

**The base rate matters here and it is 59 %**: 36 of the 61 pages a census of `over_px > 0`
returns are a single pixel column at the block's own edge, so any claim of the form "N pages show
X" has to exclude them before it says anything. Of the 39 documents, **2 hold an `.emf` or `.wmf`
member** and the other 37 hold none; both of those two carry 102 clip records apiece.

The 19 remaining pages are a third cause and not this one: on `microsoft_learn_multi_chart_examples`
page 4 and `EHEST` page 17 the reference emits only one clip rectangle, and the over-clipped ink
lies *inside* it — so what differs there is where our block rectangle is, not what is in force.

### 1.6 State

**Closed, as nil reach for a rule of the seat's shape.** The seat asked which *charts* escape; the
answer measured twice is that no object kind escapes and no object kind is safe — the unit is the
mask, and within one chart on one page 26.2.4.2 cuts the unmasked ink at the block and lets the
masked ink through. There is therefore no predicate on `drawing.Chart` to write, which is the
question the seat was filed to settle, and the permissive approximation r99 already measured is
worse by **+14.36 summed `|ink|%`** across the four documents.

**No code was written.** What remains is a mask-level parity question — we draw a bubble as one
solid ellipse where the reference draws a clipped band gradient, so there is no mask of ours for
a replace rule to apply to — and it is re-seated as such rather than closed, with its reach
measured: **6 pages, 3 documents, 25,911 px, 82.6 % of the seat's own total**.

---

## 2. O36 — a chartex chart, read and drawn

### 2.1 What 26.2.4.2 resolves, and the much smaller thing it draws

`--convert-to fods` of `054_Problem_analysis_with_Pareto_chart_11058329.xlsx`, run fresh through
26.2.4.2, gives

```xml
<chart:chart chart:class="ooo:com.sun.star.chart2.ClusteredColumnChartType" …>
 <chart:plot-area …>
  <chart:axis chart:dimension="x" chartooo:axis-type="text">
   <chart:categories table:cell-range-address="'PROBLEM DATA AND CHART'.B9:….B19"/>
  <chart:axis chart:dimension="y" …/>
  <chart:series chart:values-cell-range-address="….C9:….C19" chart:class="chart:add-in">
   <chart:data-point chart:repeated="11"/>
  <chart:series chart:values-cell-range-address="….E9:….E19" chart:class="chart:add-in">
   <chart:data-point chart:repeated="11"/>
  <chart:series chart:values-cell-range-address="" …/>
  <chart:series chart:values-cell-range-address="" …/>
  <chart:wall/><chart:floor/>
```

So four series of which **two carry an eleven-cell range and two carry none**, eleven categories,
two axes, wall and floor. `chart-types-r101` §6's *"4 series, 4 data points"* is corrected: it is
eleven points per series.

**And its rendering carries almost none of that.** Read out of the reference bank's own content
stream, the chart region of each witness holds exactly one white filled rectangle and **two
ten-segment polylines**:

| | region | ink at 120 dpi | what is drawn |
|---|---|---:|---|
| `054` p1 | (63.649, 106.531)-(556.833, 383.591) | 1,964 px, 5.15 % of the page's | white rect; polyline (63.649, 141.149)-(556.833, 369.706) in `4D62EF`; polyline (63.649, 376.640)-(556.833, 381.966) in `62D382` |
| `051` p1 | (122.969, 64.102)-(431.092, 236.696) | 2,029 px, 0.40 % | white rect; two polylines in `E95A29` and `3C9FE0` |

**No columns, no axis line, no tick, no tick label, no category label, no legend and no title.**
The two `cx:paretoLine` series draw nothing at all — they carry no `cx:dataId` — and the two
`clusteredColumn` series are drawn as lines.

The value axis is shared and auto-scaled to 0…40, which the geometry confirms with no free
parameter: the blue polyline's endpoints sit at 0.875 and 0.050 of the plot's height and the
series runs 35 down to 2 (35/40 and 2/40); the green one's at 0.02509 and 0.00587, and its series
runs 1.0 down to 0.2318 (1.0/40 and 0.2318/40).

### 2.2 Why nothing is shifted, and where the categories come from

Two arms, each with a source leg and a measured one.

**Unshifted categories.** A chartex `clusteredColumn` is `TYPEID_CLUSTEREDCOLUMN` in
`TYPECATEGORY_CLUSTEREDCOLUMN` (`oox/source/drawingml/chart/typegroupconverter.cxx`:111, this
tree), which is none of the four categories `AxisConverter`'s shift table names
(`axisconverter.cxx`:292-301) — and a `cx:axis` states no `c:crossBetween` — so
`ScaleData::ShiftedCategoryPosition` keeps `AxisHelper`'s default of **false**. Measured: the
reference's polylines span 63.649…556.833, which is the plot rectangle exactly, so the eleven
points are at i/10 and not at category centres. `ChartPlot.ShiftedCategories` already answers
false for a line chart stating nothing, so nothing was needed for this.

**The categories are the first data block's, not the longest.** Both witnesses' second
`cx:data` names a category range one cell longer than the first — the header row — and taking the
longer one spreads eleven points over twelve slots. The reference takes
`rTypeGroups.front()->createCategorySequence()` (`axisconverter.cxx`:290). Measured, on `054`: with
the longer range our polyline ended at x **512.02** against 26.2.4.2's 556.83; with the first
block's it ends at **556.86**.

### 2.3 The numbers are in the workbook, and that is what the change to `XlsxChartRanges` is for

A `c:` chart caches its points; chartex has a cache element (`cx:lvl`/`cx:pt`) and **neither
witness uses it**. Both state a bare `cx:f` naming a hidden workbook-scope defined name —
`_xlchart.v1.0` … `v1.5`, resolving to `'Defect data and chart'!$C$7:$C$17` and the like — and
`XlsxChartRanges.Resolve` declined a defined name outright.

`DefinedName` now expands a workbook-scope `definedName` whose value is one sheet-qualified area,
and nothing else: a sheet-scoped name, a formula, a union and a name naming another name all still
answer null, which is the same outcome as before. **Censused prefix-agnostically over the 803 zip
documents of the corpus (`o36-census.py`, `o36-census.tsv`): 2 state a chartex part and
`0` state a plain `c:f` naming a workbook-scope defined name**, so the change reaches no `c:`
chart in the corpus by markup. §5 measures the same thing by rendering.

### 2.4 The fix, and the scores

`DrawingChartex.Read` builds a `ChartPlot` of kind `Line` from the series that name a
`cx:dataId`, with every axis, tick, label and legend switched off and the plot rectangle painted
white. `XlsxDrawings` routes the chartex graphic-data URI to it.

| | `\|ink\|%` before → after (512) | at 1600 | chart-region ink at 120 dpi, ref / before / after |
|---|---|---|---|
| `054_Problem_analysis_with_Pareto_chart_11058329__xlsx` | 0.14 → **0.00** | 0.04 → **0.01** | 1,964 / **0** / 1,823 |
| `051_Manufacturer_defect_analysis_53db27ea__xlsx` | 0.50 → **0.03** | 0.50 → **0.03** | 2,029 / **148,032** / 1,937 |

`051`'s 148,032 is the point of that column: with no chart drawn, the sheet's cream background
showed through the whole chart frame, and the white plot rectangle is most of what removes it.

The drawn geometry against the reference's, corner for corner:

| | reference | this tree | worst |
|---|---|---|---:|
| `054` blue | (63.649, 141.149)-(556.833, 369.706) | (63.666, 141.169)-(556.859, 369.735) | 0.03 pt |
| `054` green | (63.649, 376.640)-(556.833, 381.966) | (63.666, 376.661)-(556.859, 381.982) | 0.03 pt |
| `051` orange | (122.969, 85.667)-(431.092, 228.047) | (123.015, 85.692)-(431.119, 228.076) | 0.05 pt |
| `051` blue | (122.969, 232.366)-(431.092, 235.684) | (123.015, 232.391)-(431.119, 235.706) | 0.05 pt |

with the stroke colours identical to four decimal places on all four.

### 2.5 What is deliberately not done

* **The six chartex types with no witness stay unimplemented** — `funnel`, `treemap`, `sunburst`,
  `waterfall`, `boxWhisker`, `regionMap`, `N25`, still 0 of 803. The reader dispatches on
  `cx:dataId` rather than on `@layoutId`, so a witness for any of them would be drawn as a line
  chart rather than as an empty frame, which is what 26.2.4.2 does to a `clusteredColumn`; that is
  a guess about the other six and it is recorded as one rather than defended.
* **The extraction path is untouched.** `XlsxCharts` still ignores a chartex part. Both witnesses
  are at `62/62` and `70/70` glyphs against the reference and adding the chart's numbers to the
  content tree would move that column with nothing to justify it: 26.2.4.2 draws no chart text on
  either page.

---

## 3. O38 — there is no second condition

### 3.1 The instrument round 102 did not have

`PieChart::createShapes` draws, for `PieChartSubType_BAR` and `PieChartSubType_PIE` and for
neither other case, exactly **two two-point connector lines** through
`ShapeFactory::createLine2D` (`chart2/source/view/charttypes/PieChart.cxx`:1073-1105 and
:1121-1150, this tree). A plain pie draws none. So the question is binary in the reference's own
output, and it does not depend on clustering marks by x — which is what
`chart-smooth-r102/ofpie-variants.py` did, and why it read a pie-of-pie's two rings as one pie.

`o38-ofpie.py` builds 28 one-attribute variants of `029_Unit_Circle_Chart_Pie_Theme_8a922142.docx`
(with its `c:explosion` and its seventeen `c:dPt` removed, which is round 102's own `-neither`
base) and puts each through 26.2.4.2 twice. `o38-ofpie.tsv`:

| points | `ofPieType="pie"` | `ofPieType="bar"` |
|---:|---|---|
| 3 | **plain pie**, 0 connectors, 3 wedges | **plain pie**, 0 connectors, 3 wedges |
| 4, 5, 6, 7, 8, 10, 12, 16, 20 | of-pie, 2 connectors, **n + 1** wedges | of-pie, 2 connectors, **n − 1** wedges |

and, at eight points, over the split position — which neither original states and which the probe
adds as `c:splitType val="pos"` + `c:splitPos`:

| split | pie wedges | bar wedges |
|---:|---:|---:|
| 1 | 9 | 8 |
| 2 | 9 | 7 |
| 3 | 9 | 6 |
| 5 | 9 | 4 |

**That is `OfPieDataSrc::getNPoints` exactly**: the left ring is `total − splitPos + 1` and the
right plot is `splitPos`, which the pie form draws as wedges and the bar form as rectangles. And
the fallback fires at exactly `minPoints = 4`, **in both forms**.

So the 27.2 tree's own gate `nPointCount >= OfPieDataSrc::minPoints` *does* fit the measurement,
and round 102's *"the 27.2 tree's own gate … does not fit"* is retracted along with the
observation it was built on.

**`028` is a second document and only its sixteen-point rows are usable**, which is worth saying
because the file exists. `o38-ofpie-028.tsv` is the same sweep over
`028_Unit_Circle_Chart_Optimized_Graph`: at sixteen points it agrees exactly — 17 wedges for
`pie` and 15 for `bar`, `n + 1` and `n − 1` — and at three, four and eight points the probe's
cache rewrite does not take on that file's differently-shaped `c:numCache`, so those six rows
measure the unmodified document and say nothing. Its connector count is unusable throughout for
a different reason: the page carries 129 unrelated one-segment strokes of its own, which is why
the discriminator above was run on `029`, whose page carries none.

### 3.2 State, and what needed doing

**Nothing.** `ChartLayout.Plots.cs`:560 already holds `OfPieMinimumPoints = 4` and `:608` already
falls back to `AddWedges` below it, and `SplitPosition` is already read and applied. The seat
closes as *the condition does not exist*; the reach of the fix that would have been written is
nil twice over — the corpus's only pie-of-pie is the exploded one, which round 102 already reads
as a plain pie, and the corpus's only drawn of-pie has sixteen points.

`ChartPlotTypeLayoutTests.AnOfPieDrawsTheReferencesOwnWedgeCounts` is the measurement, nine cases
covering both forms, the three-point fallback and four split positions. The doc-comment on
`AddOfPie` said *"the installed LibreOffice used as the oracle here is 24.2, which predates of-pie
support … so the geometry below is a port of the tree's source rather than a match against a
rendering"*; that is now false and is replaced with what was measured.

---

## 4. What the of-pie work found instead, which is a new seat

### 4.1 The unit radius is wrong, and the corpus can see it

Rendering the same variants through this tree and comparing wedge extents:

| | 26.2.4.2's main ring | this tree's | ratio |
|---|---|---|---:|
| `028_Unit_Circle_Chart_Optimized_Graph` as it stands, 16 points, bar form | 181.0 × 180.5 pt, centred (202.8, 425.4) | 247.5 × 256.4, centred (158.5, 372.5) | **1.37** |
| `029` at 8 points, pie form | 260.5 × 260.4 | 271.3 × 277.4 | 1.065 |
| `029` at 3 points — a plain pie, the control | 390.7 tall | 397.2 | 1.017 |

The ratio is not a constant, so this is not a scale factor to apply: the reference is fitting
something wider than a unit circle into the plot rectangle, and the of-pie's own extent runs from
`m_fLeftShift − m_fLeftScale = −1.4167` to `m_fBarRight = 1.25` across against ±`m_fLeftScale`
down (`PieChart.hxx`:258-269) — 2.667 by 1.333 units, whose aspect the plot rectangle's own aspect
then decides against. **That is a hypothesis and it is deliberately not implemented from one**;
this session's retractions have mostly been plausible readings promoted into findings.

Its reach is **1 of 947** — `028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx`, `|ink|%` 3.47
and unmoved by round 102 — and its `.odt` twin, whose `loext:sub-bar` round 102 taught the ODF
reader to read and which stands at 1.41.

---

## 5. Confinement

Rendering the corpus's **243 `.xlsx`/`.xlsm` documents** twice — at the round's base and again
after, under `SOURCE_DATE_EPOCH`, one output directory per document — and comparing the PDFs byte
for byte. That is the whole reach of the diff: `XlsxDrawings` and `XlsxChartRanges` are
`XRay.Office.Spreadsheets`, `DrawingChartex` has one caller and it is `XlsxDrawings`, and the two
other edited files are doc-comments.

**2 of the 243 move and 241 are byte-identical**, and the two are
`054_Problem_analysis_with_Pareto_chart_11058329__xlsx` and
`051_Manufacturer_defect_analysis_53db27ea__xlsx` — the two chartex witnesses and nothing else.
`o36-confine.py`, `o36-confine.tsv`. Neither sweep recorded a failure.

That is the same answer §2.3's markup census gives — 0 of 803 zip documents state a plain `c:f`
naming a workbook-scope defined name — arrived at by a method that shares no code with it.

---

## 6. Deliverable state

Built clean, `TreatWarningsAsErrors` on, **0 warnings 0 errors**.

The ten non-fidelity projects, run one at a time and totalled from this run's own output
(`/home/user/r104-work/tests-nonfid.log`):

```
Containers 109   Core 544   Markup 259   OpenDocument 160   Presentations 1074
Rendering  164   Spreadsheets 1295   Text 728   Vector 309   WordProcessing 1938
                                                   6580 passed, 0 failed, 0 skipped
```

`XRay.Office.Fidelity.Tests`: **Failed: 10, Passed: 542, Total: 552**, and the ten names read out
of that run's own log are exactly the briefed set — `PageDrawingComparisonTests` ×4,
`TabStopComparisonTests` ×4, `SheetDrawingComparisonTests` and
`JustificationShrinkComparisonTests`. No eleventh failure.

**New tests**, 17 in all:

* `XRay.Office.Presentations.Tests/DrawingChartexTests.cs` — 8. Which series are drawn and as what,
  where the categories come from, that no axis furniture is drawn, the white plot rectangle, the
  series' own line colour, a cached `cx:lvl`, a part whose every series derives from another, and
  a plain `c:chartSpace` being refused.
* `XRay.Office.Core.Tests/ChartPlotTypeLayoutTests.AnOfPieDrawsTheReferencesOwnWedgeCounts` — 9
  cases: the three-point fallback in both forms, `n + 1` for the pie form, and `n − split + 1`
  for the bar form at four split positions.

**Files here**, all present and non-empty: `results.md`; five probes — `o26-clipstate.py`,
`o26-objects.py`, `o36-census.py`, `o36-confine.py`, `o38-ofpie.py` — and their outputs
`o26-clipstate.txt` (90 lines, ten pages), `o26-objects.tsv` (40 rows), `o36-census.tsv` (3),
`o36-confine.tsv` (4), `o38-ofpie.tsv` (37) and `o38-ofpie-028.tsv` (9).

---

## 7. Register

`dotnet/probes/OPEN-ISSUES.md`:

* **O26 — closed, nil reach for a rule of its own shape.** The block clip is a
  `Push(CLIPREGION)` + `IntersectClipRegion` taken once per layer in
  `ObjectContactOfPageView::DoProcessDisplay`; what replaces it is
  `VclMetafileProcessor2D::processMaskPrimitive2D`'s `SetClipRegion`, a replace over a clip
  polygon that never contains the block. So the escaping unit is a mask and not an object kind:
  on `DynamicBubbleChart` page 2 the same chart's axis strokes are cut at the block and its
  gradient-masked bubble markers are not. 36 of the 61 pages the seat's own census returns are a
  one-pixel boundary column; of the rest, 4 pages are the two EMF documents (17,269 px), 2 are the
  chart (8,642) and 19 are a block-geometry difference (2,609).
* **N30 (new) — mask-level parity, seated with its reach.** 6 pages, 3 documents, 25,911 px, and
  nothing to apply a replace rule to until this tree draws a gradient under a mask.
* **O36 — closed, fixed in this tree.** Reach 2 of 947. `|ink|%` 0.14 → 0.00 and 0.50 → 0.03; the
  drawn polylines match 26.2.4.2's corner for corner to 0.05 pt.
* **O38 — closed; the second condition does not exist.** The gate is `nPointCount >= 4` for both
  forms, over 28 one-attribute variants, and this tree already implements it. Round 102 §4.5 is
  retracted.
* **O42 (new) — the of-pie's unit radius is 1.37× the reference's on the one drawn of-pie.**
  Reach 1 of 947 in each stream, `|ink|%` 3.47 and 1.41.
