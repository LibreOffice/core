# L7-charts — findings, by root cause

14 documents. They collapse to **seven chart root causes**, of which six have a patch, plus **four
documents that are not chart bugs at all**, one where the **reference** is the worse renderer, one
bar-of-pie the 24.2 reference cannot draw, and one (#186) deliberately left unpatched.

Nothing in `/home/user/libreoffice-core` was modified, built or tested. Every patch under
`patches/` is a `git diff`-format unified diff against the repository root, one root cause per
file. Apply order is in `summary.md`; only `unnamed-series-legend.diff` has a prerequisite, and it
is stated in a `#` header inside that file.

---

## 0 · Reference-version check, done first

**Which binary I measured.** Every claim below is read off the **24.2.7.2** reference PDFs already
banked in `/data/bench/lo/` (producer string "LibreOffice 24.2"), against our own
`/data/bench/pl/` PDFs, at 150–200 dpi crops or via `pdftotext`.

**Dating the C++ is not available in this checkout.** The tree is
`AC_INIT([LibreOffice],[27.2.0.0.alpha0+])` — master, newer than both references — and its history
is squashed: `git log -1` on every chart file cited below returns the same `Update git submodules`
commit. Per-line blame across releases cannot be run here.

**What replaces it.** Each behaviour is bracketed: read off **24.2.7.2 output**, and independently
implemented by the **27.2 master source** in this checkout. 24.2 and 27.2 agreeing brackets 26.2 —
the rule would have had to change and change back. On top of that, two of the seven are separately
confirmed at **26.2.4.2** by the project's own probes:

- `probes/sheets-r50-chartex/results.md` (26.2.4.2 per `probes/PROVENANCE.tsv`): 5 sheets documents
  carry `c15:datalabelsRange`, **5 of our PDFs contain the literal `[CELLRANGE]` and 0 reference
  PDFs do** — *"The defect is real and it is ours"* — and it names `047_Date_tracker_Gantt_chart`,
  my #002.
- Same probe's blind readings: `057_Simple_balance_sheet` (my #188) named by a reviewer as *"the
  reference draws a legend, ours draws none"*. Same document, same direction, at 26.2.

**Where the version warning actually bit, and I acted on it:** #186's Grand Total. The same 26.2
probe records *"PivotTable regeneration — probably a reference-side divergence… ours may be the
faithful side. Measure whose string is in the cell before implementing anything here."* I have
**not** patched it. See §8.

**Two instrument cautions from that probe, obeyed.**

1. `TODO.batches.md:16815,16966` records that two independent reviewers both wrongly read "no
   markers, no vertical gridlines" off a *composed pair* of `Demick_JetBlue` page 4, refuted by a
   200 dpi crop. My own 150 dpi crop reproduces the refutation — so I **corrected** the #055 case
   note rather than acting on it (§4).
2. *"a reviewer's 'we draw no legend' was refuted for the third round, third reader, same page"*.
   So for both legend findings I used `pdftotext`, not pixels: reference page 3 of #188 contains
   `Column C` and `Column D` and ours contains neither; reference page 3 of #166 contains 9
   `Product [ABC]` hits and ours 0.

**The striped fill is not a defaults question and not a hatch.** See §1: `grep -c pattFill` over
all four chart parts of the three affected workbooks is 0.

---

## 1 · The grid and the axes are painted over the bars, not under them

**Patch:** `patches/grid-under-series.diff`
**Documents:** #158, #161, #188 — the most repeated chart fault in the lane.
**Confidence:** high.

### What the pages show

Every bar carries evenly spaced light rules across it at exactly the value axis' major-tick
heights, and the reference's bars are solid. `pl188-3.png` at 200 dpi is clearest: the rules on
our red and blue columns line up pixel-for-pixel with the `#D9D9D9` gridlines that run across the
empty part of the plot either side. Rule spacing = tick spacing, rule colour = grid colour, and
the rules continue outside the bars at the same y. It is the grid, not a fill.

### What the documents actually contain

`057_Simple_balance_sheet…xlsx`, `xl/charts/chart11.xml`:

```xml
<c:ser><c:idx val="0"/><c:order val="0"/>
  <c:spPr>
    <a:solidFill><a:schemeClr val="accent4"><a:lumMod val="75000"/></a:schemeClr></a:solidFill>
    <a:ln><a:noFill/></a:ln><a:effectLst/>
  </c:spPr>
…
<c:majorGridlines><c:spPr>
  <a:ln w="9525" cap="flat" cmpd="sng" algn="ctr">
    <a:solidFill><a:schemeClr val="tx1"><a:lumMod val="15000"/><a:lumOff val="85000"/></a:schemeClr></a:solidFill>
```

`grep -c pattFill` over all four chart parts of the three workbooks returns **0**. Both hypotheses
the lane brief offered — a pattern fill synthesised where the file specifies a solid one, and a
solid fill falling through to a hatch default — are refuted by the parts. The fill is
`a:solidFill`, it is drawn solid, and the stripes are the `tx1 lumMod 15000 lumOff 85000` grid
(`#D9D9D9`) drawn on top of it.

### Where it lives in the source

`ChartDrawing` (`dotnet/src/XRay.Office.Core/Charts/ChartLayout.cs:199-206`) hands the consumer four
lists, and all three consumers paint them in the same fixed order — `Boxes`, `Lines`, `Shapes`,
`Labels`:

- `dotnet/src/XRay.Office.Spreadsheets/Layout/SheetChart.cs:53,62,70,79`
- `dotnet/src/XRay.Office.Presentations/Layout/SlideChart.cs:59,76,90`
- `dotnet/src/XRay.Office.WordProcessing/Layout/FrameChart.cs:55,65,72,79`

`AddValueAxis` puts the gridlines in `Lines` (`ChartLayout.cs:1651-1656`, `:1683-1688`).
`AddBars` puts the bars in `Boxes` (`ChartLayout.cs:3069`) and `AddCandles` puts a candle body in
`Boxes` (`ChartLayout.Plots.cs:466-472`). `Boxes` is painted first, so the bars go **under** the
grid.

Every other series mark already goes into `Shapes` and therefore already paints over the grid —
areas (`AddAreas`), polylines (`AddLines`), wedges (`AddWedges`), bubbles, radar. **A bar and a
candle are the only two series marks in the whole layout emitted into `Boxes`, and they are
exactly the two the fault appears on.** That internal inconsistency is the bug.

`SheetChart.cs:20-26` states the intended order in prose and it is the right one — *"the chart's
own background, the plot area's wall, then the axes and their ticks, then the bars over them"* —
so the intent was already correct and unreachable through the record's four lists.

The reference agrees. `VCoordinateSystem::initPlottingTargets`
(`chart2/source/view/axes/VCoordinateSystem.cxx:91-115`) creates, in order, the **grid** group, the
series-behind-axis group and the **axis** group as children of the diagram's coordinate region —
*"create group shape for grids first thus axes are always painted above grids"* — and
`ChartView::createShapes2D` then calls `createAxesShapes()`/`createGridShapes()`
(`chart2/source/view/main/ChartView.cxx:638-646`) **before** the series plotters' `createShapes()`
(`:648-680`). A plotter goes into `xSeriesTargetInFrontOfAxis`, i.e. the parent region itself and
therefore after all three child groups, unless `ChartType::isSeriesInFrontOfAxisLine` says
otherwise — and that returns false for a **filled net only**
(`chart2/source/model/template/ChartType.cxx:609-615`). Every chart type in this corpus paints its
series over both the grid and the axes.

### The proposed change

Emit a bar and a candle body as a `ChartShape` rectangle instead of a `ChartBox`. `Shapes` is
painted after `Lines` in all three consumers, so this moves the data over the grid without
touching a single consumer, without adding a fifth list to `ChartDrawing`, and without moving
gridlines into a channel (`Boxes`) that cannot carry their dash pattern or their hairline width.

The patch also rewrites `ChartDrawing`'s `Boxes`/`Shapes` documentation so the invariant is stated
where the next author will read it: **`Boxes` is the furniture, `Shapes` is everything a series
draws.**

**Test fallout the applying pass must expect.** Six assertions read bars or candles out of
`drawing.Boxes` and will need to read `drawing.Shapes`. They are *not* in the patch, because tests
are outside this lane's declared ownership:

- `dotnet/tests/XRay.Office.Core.Tests/ChartLayoutAxisTests.cs:102,105,106,250`
- `dotnet/tests/XRay.Office.Core.Tests/ChartLayoutTests.cs:145,146`
- `dotnet/tests/XRay.Office.Core.Tests/ChartPercentAndLineKeyTests.cs:99,185`
- `dotnet/tests/XRay.Office.Core.Tests/ChartPlotTypeLayoutTests.cs:301,304`

`ChartPercentAndLineKeyTests.cs:146` (`Boxes.ShouldNotContain(… Line == 0x1A2557)`) still passes
and becomes vacuous; it should move to `Shapes` too.

### The probe that would refute me

A one-series column chart with `<c:majorGridlines/>` on the value axis, a bar tall enough to cross
three gridlines, and a bar fill that contrasts with the grid. Render both ways and sample a pixel
where a gridline crosses the bar's interior: my explanation says the reference gives the bar's
fill there and we give the grid colour. Then **halve `c:max`**: on my account the stripes move with
the ticks; on a synthesised-pattern account they do not.

### Not established

Whether the reference paints the *axis line and its tick marks* over or under a bar as well. The
C++ says it does (both live in `m_xLogicTargetForAxes`, under the series target), and this patch
makes us agree, but no page in this lane shows a bar touching an axis line where it would be
visible.

---

## 2 · A legend is dropped whenever the series' names cannot be read

**Patches:** `patches/series-name-from-range.diff` (a), `patches/unnamed-series-legend.diff` (b)
**Documents:** #166 (a — three of five charts), #188 (b)
**Confidence:** (a) high; (b) medium-high on the mechanism, medium on the exact wording.
**(b) requires (a).**

### What the pages show

`pdftotext -f 3 -l 3` on #166: the reference contains **9** `Product [ABC]` hits (three legends of
three entries — the line, the clustered column and the area charts), ours contains **0**. The pie
chart's legend is right on both sides and the scatter has none on either.

`pdftotext -f 3 -l 3` on #188: the reference contains `Column C` and `Column D`; ours contains
neither. The file states `<c:legend><c:legendPos val="b"/>…</c:legend>`, so the legend is not
suppressed.

Both were taken from the PDFs' own text rather than from the composed pair, because
`probes/sheets-r50-chartex/results.md` records a "we draw no legend" reading being refuted three
rounds running on composed pairs.

### What the documents actually contain

**(a)** `microsoft_learn_multi_chart_examples.xlsx` — every sequence in every one of the five chart
parts is a bare reference with **no cache anywhere in the part** (`grep -c strCache` = 0):

```xml
<ser><idx val="0"/><order val="0"/>
  <tx><strRef><f>'Examples'!B4</f></strRef></tx>
  …
  <cat><numRef><f>'Examples'!$A$5:$A$8</f></numRef></cat>
  <val><numRef><f>'Examples'!$B$5:$B$8</f></numRef></val>
```

The three charts whose legends we drop are exactly `chart1.xml` (`lineChart`), `chart2.xml`
(`barChart`) and `chart3.xml` (`areaChart`) — the three whose legends name *series*. `chart4.xml`
is the pie, whose legend names *categories*, and it is right on both sides. That correspondence is
exact and is what identifies the mechanism.

**(b)** `057_Simple_balance_sheet…xlsx` — both `c:ser` state a `c:spPr`, a `c:cat` and a `c:val`
and **no `c:tx` at all**. Their value references are

```
('Balance sheet'!$C$6:$C$11,'Balance sheet'!$C$15:$C$18,'Balance sheet'!$C$22,…)
('Balance sheet'!$D$6:$D$11,…)
```

### Where it lives in the source

**(a)** `DrawingChartPlot.ReadSeries` builds every *data* sequence through
`ReadSequence(…, ranges)` (`DrawingChartPlot.cs:1107,1110,1133,1143`), which prefers the resolved
cells and falls back to the cache — but the series *name* is read by
`DrawingChartText.Label(Child(element, "tx"))` (`DrawingChartPlot.cs:1165`), and
`DrawingChartText.Label` (`:2647-2665`) only ever walks `c:pt/c:v`. **The resolver is threaded into
the same method and is not consulted for the one sequence that decides whether a legend exists** —
the parameter is in scope three lines above the call. That is the "read but never consumed"
pattern the brief says to grep for, and this project's fourth instance of it.

The consequence is `ChartLayout.Entries` (`ChartLayout.cs:3516`):
`if (series.Name is not { Length: > 0 } name) continue;` — no names, no entries — and `AddLegend`
returns at `ChartLayout.cs:3353` (`if (named.Count == 0) return;`).

**(b)** The same `continue` at `ChartLayout.cs:3516`, reached because the file states no name at
all. The reference does not leave it unnamed: `DataSeries::getLabelForRole` →
`getLabelForLabeledDataSequence` falls through to
`xValueSeq->generateLabel(LabelOrigin_SHORT_SIDE)` when the label sequence is absent or empty
(`chart2/source/model/main/DataSeries.cxx:641-672`), and Calc answers it —
`ScChart2DataSequence::generateLabel` sums the columns and rows of every range token, labels along
the **short** side, and `GenerateLabelStrings` writes
`ScResId(STR_COLUMN) + " " + <column letter>` (`sc/source/ui/unoobj/chart2uno.cxx:3147-3243`), with
equal counts returning an empty sequence and hence no label.

Run by hand over series 1's actual formula: six one-column fragments totalling 6 columns and 20
rows; rows > columns, so the label is a column label, first column C — **`Column C`**, which is
what the reference prints. Series 2 gives `Column D`.

### The proposed changes

**(a)** `SeriesName(tx, ranges)`: resolve `c:tx`'s `c:f` through the resolver first, fall back to
the cache — the same order and the same reason `ReadSequence` already uses
(`ExcelChartConverter::createDataSequence`, `sc/source/filter/oox/excelchartconverter.cxx:76-105`).
With no resolver — every host but Calc — behaviour is bit-for-bit what it was.

**(b)** `GeneratedSeriesName(valueSource, ranges)`: the `generateLabel` rule above, applied only
when a resolver is present (the Calc host). The gate matters: a deck's sequences are
`CachedDataSequence`s whose `generateLabel` returns an empty sequence, so the reference names those
series by a different route entirely, and firing this ungated would invent legends on every
unnamed `pptx` series.

### The probes that would refute me

**(a)** Paste a `<strCache><ptCount val="1"/><pt idx="0"><v>Product A</v></pt></strCache>` into one
series' `c:tx`. If the legend appears for that one series and not the others, the cause is the
missing cache and nothing else. A "legend suppressed by position or auto-format" account predicts
no change.

**(b)** A two-column workbook charted with no `c:tx`, once with the values in a tall range
(20 rows × 1 column) and once transposed. My account predicts `Column B` in the first, `Row 4` in
the second, and *nothing at all* for a square range. An account that says LibreOffice numbers
unnamed series predicts `Series1`/`Series2` in all three.

### Not established / deliberate divergence

**Excel itself would print `Series1` here, not `Column C`.** `Column`/`Row` are localized
resources (`sc/inc/globstr.hrc:178`), exactly like `Chart Title`, which this codebase already
hardcodes with the same caveat (`DrawingChartTitle.DiagramTitle`). Patch (b) matches the reference
we are gated against and the divergence from Office is deliberate and stated. If the project would
rather not encode a Calc-side string, **drop (b)**: (a) is independent and larger in document
count.

Note also that the 26.2 probe saw legends disagreeing in the *reverse* direction on two other
documents (`037`, `029`: ours draws a legend the reference does not). Neither patch can cause
that — both only add entries where the file states a `c:legend` and the series names were
unreadable.

---

## 3 · `[CELLRANGE]` data labels: the placeholder is drawn because the cache holding the real text is never opened

**Patch:** `patches/cellrange-data-labels.diff`
**Document:** #002
**Confidence:** high; independently confirmed at 26.2.4.2 by `probes/sheets-r50-chartex`.

### What the pages show

Nine data labels on the Gantt chart, every one of them the literal `[CELLRANGE]`. The reference's
own text for the same chart, on its page 8: `Today`, `Activity 1` … `Activity 7`, `Milestone 1`
(`pdftotext -f 8 -l 8 /data/bench/lo/sheets_chartset-010_…/out.pdf`).

### What the document actually contains

`xl/charts/chart11.xml`, per point:

```xml
<c:dLbl><c:idx val="0"/><c:tx><c:rich><a:p>
  <a:fld id="{552A2309-…}" type="CELLRANGE"><a:rPr lang="en-US"/><a:pPr/><a:t>[CELLRANGE]</a:t></a:fld>
</a:p></c:rich></c:tx>
…
<c:extLst><c:ext uri="{CE6537A1-D6FC-4f65-9D91-7224C49458BB}" …>
  <c15:dlblFieldTable/><c15:xForSave val="1"/><c15:showDataLabelsRange val="1"/>
</c:ext></c:extLst>
```

and, on the series:

```xml
<c:extLst><c:ext uri="{02D57815-91ED-43cb-92C2-25804820EDAC}" …>
  <c15:datalabelsRange>
    <c15:f>'Dynamic Chart Data Hidden'!$B$15:$B$21</c15:f>
    <c15:dlblRangeCache><c:ptCount val="7"/>
      <c:pt idx="0"><c:v>Activity 1</c:v></c:pt> … <c:pt idx="6"><c:v>Activity 7</c:v></c:pt>
    </c15:dlblRangeCache>
  </c15:datalabelsRange>
</c:ext></c:extLst>
```

The text is in the file. `a:t` is Excel's localizable placeholder, not the value.

### Where it lives in the source

`DrawingChartPlot.CustomLabel` maps `@type="CELLRANGE"` to `ChartLabelField.CellRange` and stores
the run's own `a:t` as the part's text (`DrawingChartPlot.cs:1569,1584`). `ChartDataLabel.Resolve`
has no case for `CellRange` and falls to `default: built.Append(part.Text)`, so the placeholder is
what gets drawn. The belief is written down as a fact in `ChartDataLabel.cs:78-82` — *"used only
when the field cannot be resolved — a `ChartLabelField.CellRange`, whose cached string is all
there is"* — and it is wrong: `git grep -n datalabelsRange -- dotnet/` returns nothing.

The reference does look: `SeriesConverter::createDataSeries` reads
`mrModel.mrParent.mpLabelsSource->mxDataSeq->maData.find(mrModel.mnIndex)` and calls
`xCustomLabel->setString(oaLabelText)` (`oox/source/drawingml/chart/seriesconverter.cxx:366-410`),
gated on `mrModel.mobShowDataLabelsRange` (`seriescontext.cxx:134-139`), with `mpLabelsSource`
bound at `seriescontext.cxx:461` and the sequence parsed at `datasourcecontext.cxx:221,348`.

### The proposed change

Read the series' `c15:dlblRangeCache` into a per-point string list (`DataLabelsRangeOf`) and, for
each `c:dLbl` carrying `c15:showDataLabelsRange`, rewrite its `CellRange` parts' text to the
string at that point index (`WithCellRange`). The corrected belief replaces the wrong one in
`ChartDataLabel.cs`'s XML doc.

### The probe that would refute me

Delete the `c15:datalabelsRange` extension from a copy of the workbook and re-render. My
explanation predicts the labels go back to `[CELLRANGE]` on our side **and on the reference's**; an
explanation that says LibreOffice resolves the cells live from `c15:f` predicts the reference still
prints `Activity 1`. (The source reads the parsed cache, not the provider, but it is worth one
render to confirm.)

### Correction to the case note

"The Gantt chart's plot area is filled black where the reference leaves it white" is **wrong**.
`c:chartSpace/c:spPr` states `<a:solidFill><a:schemeClr val="tx1"/></a:solidFill>` and the theme's
`dk1` is `windowText`/`000000`, so the black is the file's. The reference draws the same black
chart — on **its page 8**, which is 58% dark ink where its pages 1–7 are 0.2–0.7%. Our pagination
(5 pages against 8) put a different page under the page-5 comparison. **The black is correct; the
pagination is a `pagination`-lane matter.**

---

## 4 · A line series' legend key draws the rule and not the marker

**Patch:** `patches/legend-marker-key.diff`
**Document:** #055
**Confidence:** high for the legend; the plot-side half of the case note is **corrected, not acted on**.

### What the pages show

Rendered at 150 dpi and cropped to the legend band, the reference's three keys are a filled
**square** on `RPM`, a **diamond** on `ASM` and a **down-arrow** on `LF`, each centred on its rule.
Ours are three bare rules.

### Correction to the case note

In the *plot* we **do** draw the markers — the same crop at (200,330)–(420,470) shows down-arrows
on the blue series and diamonds on the dark red on both sides. This is not a new observation: the
tree already records two independent reviewers making exactly this misreading of exactly this page
off a composed pair, refuted by a 200 dpi crop (`TODO.batches.md:16815,16966` — *"The pair is a
good instrument for structure and a bad one for small marks, and the round nearly took 'we draw no
markers' as a finding"*). `MarkerOf`/`AutomaticMarker` (`DrawingChartPlot.cs:1310-1367`) already
ports `VDataSeries::getSymbolProperties`' `StandardSymbol = m_nGlobalSeriesIndex`
(`chart2/source/view/main/VDataSeries.cxx:874-883`) and yields square/diamond/triangle for indices
0/1/2. **Only the legend key is missing.**

### What the document actually contains

`ppt/charts/chart1.xml` has `<c:marker val="1"/>` on each `c:lineChart` group and **no
`<c:marker>` element inside any `c:ser`** (`grep -c '<c:marker>'` = 0). Every symbol on this chart
is therefore automatic, which is why it is the legend and not the plot that separates the two
renderers.

### Where it lives in the source

`ChartLayout.AddLegend`, the `entry.IsLine` arm (`ChartLayout.cs:3404-3419`), emits one `ChartLine`
across the key and stops. `LegendEntry` (`ChartLayout.cs:3540-3547`) carries `Fill`, `Line`,
`Width`, `IsLine`, `Dash` and `Cap` and **no symbol at all**, so `Entries`
(`ChartLayout.cs:3518-3520`) has nowhere to put `series.Marker`, which it reads for the plot and
drops here.

`VLegendSymbolFactory::createSymbol`'s `LegendSymbolStyle::Line` arm
(`chart2/source/view/main/VLegendSymbolFactory.cxx:115-155`) draws the rule and *then*
`createSymbol2D` at the key's centre at `min(keyWidth, keyHeight)`, painting it in the series' own
colour for both fill and border — *"take series color as fill color … border of symbols always
same as fill color"*.

`probes/slides-chart-02/results.md` (26.2.4.2) is the round that made this key *be* a line sample
at all, and it did not add the symbol — so this is new work, not a re-tune of a recent decision.

### The proposed change

`LegendEntry` gains `Marker` and `MarkerSize`; the `IsLine` arm emits `Marker(...)` into `Shapes`
at the key's centre, sized from `c:marker/c:size` where the file states one and from the key's own
square extent otherwise, in the sample colour for fill and stroke.

### The probe that would refute me

Set `<c:symbol val="none"/>` on one of the three series and re-render. My account predicts that
series' key loses its symbol and keeps its rule while the other two keep theirs; an account that
blames the automatic-symbol cycle predicts all three change together.

### Not established, on this document

- **"The horizontal gridlines come out tan where the reference draws them grey."** Not reproduced.
  Both renderings draw a dense grey primary grid *and* the secondary axis' orange grid, in the same
  two colours; the orange lines are the secondary `c:valAx`'s stated `c:majorGridlines` and they
  are in the reference too. Separately, the *automatic* gridline colour is a known open 26.2-era
  item (`TODO.batches.md:16975`: `tx1` tint 75000/50000 → `0x666666`/`0x8B8B8B`, we draw
  `0xB3B3B3` for both) — not re-derived here.
- **The plot area is wider in ours**, which stretches the series and moves the rotated category
  labels. Real, visible, not diagnosed. `TODO.batches.md` already names "the chart plot rectangle"
  as the next slides item, citing this document's floor being 5.5 pt low.

---

## 5 · A multi-level category range is resolved as one flat run of cells

**Patch:** `patches/multilevel-categories.diff`
**Document:** #161
**Confidence:** high.

### What the page shows

The reference's category axis has **eight** slots, each labelled `AM` (or `PM`) over its date, and
eight bar pairs filling the plot. Ours has **sixteen** slots labelled
`11/6/2022, AM, 11/6/2022, PM, 11/7/2022, AM, …` — the two levels interleaved — with the eight bar
pairs crowded into the left half and eight empty labelled slots to the right. That is the "empty
category slots are labelled instead of skipped" observation, and it is the same fault as the label
interleaving, not a second one.

### What the document actually contains

`xl/charts/chart11.xml`:

```xml
<c:cat><c:multiLvlStrRef>
  <c:f>'BLOOD PRESSURE DATA'!$C$12:$D$19</c:f>
  <c:multiLvlStrCache><c:ptCount val="8"/>
    <c:lvl><c:pt idx="0"><c:v>AM</c:v></c:pt> … <c:pt idx="7"><c:v>PM</c:v></c:pt></c:lvl>
    <c:lvl><c:pt idx="0"><c:v>11/6/2022</c:v></c:pt> … </c:lvl>
  </c:multiLvlStrCache>
</c:multiLvlStrRef></c:cat>
```

`$C$12:$D$19` is 2 columns × 8 rows = **16 cells**; the cache says **8 points in 2 levels**. The
value reference is `$E$12:$E$20`, nine cells against a cache of eight — the extra empty slot.

### Where it lives in the source

`DrawingChartPlot.ReadSequence` (`DrawingChartPlot.cs:2466-2490`) tries the resolver **first**:

```csharp
if (ranges is not null && FormulaOf(source) is { } formula && ranges(formula) is { } live)
    return ([.. live.Text], [.. live.Numbers]);

if (Child(Child(source, "multiLvlStrRef"), "multiLvlStrCache") is { } levelled)
    return ReadMultiLevel(levelled);
```

`FormulaOf` explicitly includes `multiLvlStrRef` (`DrawingChartPlot.cs:2593-2601`), so a
multi-level category **always** takes the resolver arm when a resolver exists, and
`ReadMultiLevel` — sitting right below it, with a comment saying it exists precisely so the levels
are not walked as one — is dead code on the Calc host. `ChartRangeValues` carries a flat
`IReadOnlyList<string?>` with no shape, so the rectangle cannot be recovered afterwards.

The reference builds one category per *row* and stacks the columns as that category's label levels
(`ExplicitCategoriesProvider`), which is why it draws eight.

### The proposed change

Test for the multi-level cache **before** the resolver. This is not a preference: the resolver's
return type states no shape, so the only source in the file that states the level structure is the
`c:multiLvlStrCache` Excel wrote. On a chart with a flat `c:cat` nothing changes at all.

### The probe that would refute me

Widen the range to `$C$12:$E$19` (3 columns) without touching the cache. My account predicts our
category count goes to 24 with the resolver and stays 8 with the patch, and that the reference
stays at 8 either way. An account blaming `ReadMultiLevel` itself predicts the labels are wrong
even with no resolver — testable by opening the same chart in a `.docx`.

### Also on this document

- The `Axis Title` caption is §6; the stripes on the bars are §1.
- The dates (`11/6/2022` against the reference's `9/1/2026`) are stored values against recalculated
  `TODAY()` ones, and the missing red conditional-format fills are cell formatting. Neither is a
  chart matter.

---

## 6 · An axis `c:title` with no text in it draws nothing, where the reference draws "Axis Title"

**Patch:** `patches/empty-axis-title.diff`
**Document:** #161
**Confidence:** high.

### What the page shows

The reference prints `Axis Title` centred under the category axis of the Charted Progress chart;
we print nothing. Both sides draw the two value-axis titles (`BLOOD PRESSURE`, `HEART RATE`)
correctly, so it is specifically the untexted one that is lost.

### What the document actually contains

The category axis carries a title element with a `c:overlay`, a `c:spPr` and a `c:txPr` and **no
`c:tx` whatever**:

```xml
<c:catAx>… <c:title><c:overlay val="0"/><c:spPr>…</c:spPr><c:txPr>…</c:txPr></c:title> …
```

`re.findall(r'<c:title>(.*?)</c:title>')` over the part returns three titles whose `a:t` contents
are `[]`, `['BLOOD PRESSURE']` and `['HEART RATE']`.

### Where it lives in the source

`DrawingChartPlot.cs:235-236,265` read the axis titles with `TitleText(...)`
(`DrawingChartPlot.cs:2428-2445`), which returns null for a body with no runs. The chart title
already has the counterpart substitution — `DrawingChartTitle.Automatic`
(`DrawingChartTitle.cs:89-120`), a careful port of `chartspaceconverter.cxx:185-205` including both
tdf#146487 escapes — and the axis title has none.

`AxisConverter::convertFromModel` passes `OoxResId(STR_DIAGRAM_AXISTITLE)` as the default string
whenever the model holds a title element and the type group is not a radar
(`oox/source/drawingml/chart/axisconverter.cxx:461-470`), and
`TextConverter::createStringSequence` reaches that default once the rich body, the `c:txPr`
paragraphs and the `c:tx` cache have all come back empty (`titleconverter.cxx:141-160`).
`STR_DIAGRAM_AXISTITLE` is `"Axis Title"` (`oox/inc/strings.hrc:16`). Unlike the chart title's,
this arm has **no** `autoTitleDeleted` and **no** tdf#146487 escape — the element being present is
the whole condition.

### The proposed change

`AxisTitleText(title, kind)`: the stated text if there is any, else `"Axis Title"` unless the plot
is a radar. `DrawingChartTitle` gains the constant beside `DiagramTitle`, with the same
localization caveat.

### The probe that would refute me

Delete the empty `<c:title/>` from the category axis. My account predicts the caption disappears
from the reference too; an account that says LibreOffice always captions a visible category axis
predicts it stays.

### Not established

Whether a *radar* chart is really the only exemption. The source says so at that one call site and
no radar chart in this lane has an empty axis title, so the exclusion is carried on the C++'s word.
This is also the root cause most exposed to the version question in principle — the string is a
localized resource — though it is present unchanged in 27.2 master and drawn by 24.2.

---

## 7 · A category label that the reference wraps is measured, and thinned, as one line

**Patch:** `patches/wrapped-category-labels.diff`
**Documents:** #158 (certain). #188's overlapping rotated labels are **not** the same cause.
**Confidence:** medium-high on the mechanism, medium on the patch being regression-free. This is
the riskiest of the eight.

### What the page shows

Both charts on #158: the reference labels all six categories, each on **two lines** — `ACCOUNT`
over `MANAGER`, `PROJECT` over `MANAGER`, `STRATEGY` over `MANAGER`, `DESIGN` over `SPECIALIST`,
then `EVENT STAFF` and `ADMIN STAFF`. We label three — `ACCOUNT MANAGER`, `STRATEGY MANAGER`,
`EVENT STAFF` — each on one line, every other one dropped. The bars under the unlabelled slots are
drawn correctly, so the categories exist; only their labels were thinned.

### Where it lives in the source

`ChartAxisLabels.Resolve` measures each label **once, as a single line**, before the arrangement
loop starts:

```csharp
DocSize[] boxes = new DocSize[count];
for (int at = 0; at < count; at++)
    boxes[at] = texts[at] is { Length: > 0 } text ? Shape(measurer, text, size, bold) : default;
```

(`dotnet/src/XRay.Office.Core/Charts/ChartAxisLabels.cs:157-165`; `Shape` at `:456-457` is
`measurer.Measure(text, size, bold)` — one line, always.)

`stated.LineBreakAllowed` — which the OOXML importer sets **true**, as the type's own doc comment
records (`ChartAxisLabels.cs:40-44`) — is consulted at exactly one place, `Wraps`
(`ChartAxisLabels.cs:176-183`), and only ever to turn *itself* off when some word is wider than
0.95 of a tick. **When it survives, nothing consumes it.** The labels are then collided as
single-line runs (`Collides`, `:355-405`), the collision is found, and `rhythm++` (`:205`) drops
every other one.

`ACCOUNT MANAGER` at this chart's type size is roughly two ticks wide on one line and under one
tick on two, so the whole difference between six labels and three is the line the box was measured
on. That is the fourth instance in this project of a property read and never consumed, and it sits
in a file whose header comment already names the mechanism it fails to apply
(`TextMaximumFrameWidth`, `:274-276`).

### The proposed change

Inside the arrangement loop, when line breaking is still on and the labels are upright, wrap each
label greedily at its blanks to the same `spacing × (staggered ? 2 : 1) × 0.95` limit `Wraps`
already uses, measure the wrapped shape (widest line by summed line heights), collide *that*, and
carry the wrapped strings out on a new `ChartAxisLabelLayout.Texts` for `AddCategoryAxis` to draw.
`Shape` gains a multi-line arm. Nothing changes for a chart whose labels fit, whose labels are one
word, or whose axis is rotated.

Wrapping is done at blanks only, not at the hyphens and slashes `Words()` also splits on, because
`Words` measures runs and never rebuilds the string while this rewrites it: rebuilding a
hyphen-split with a space turns `Oct-12` into `Oct- 12`. The cost is a hyphenated label the
reference wraps and we leave on one line, which errs towards *finding* a collision rather than
hiding one.

`SheetChart.Text` already draws an unrotated `\n`-bearing label as two lines
(`dotnet/src/XRay.Office.Spreadsheets/Layout/SheetChart.cs:36-40`) and `FrameChart` was fixed the
same way, so no consumer change is needed for the sheets or words tracks. **`SlideChart` still runs
a `\n` label together into one glyph run** — its own comment says so — so a `pptx` chart whose
category labels wrap would draw them concatenated. Recorded as a cross-lane dependency below; it is
a pre-existing defect this patch can newly expose.

### The probe that would refute me

The same chart with `ACCOUNT MANAGER` renamed to `ACCOUNTMANAGER` (one word, same width). My
account predicts the reference then *cannot* wrap it, turns line breaking off, and thins or rotates
exactly as we do — i.e. the two renderings agree. An account that blames the collision test itself
predicts the reference still labels all six.

### Not established — #188's overlapping rotated labels

Both engines rotate the twenty category names to 45° and both draw all twenty; ours collide
(`Goodwill` over `Less accumulated depreciation`, `Other` over `Pre-paid expenses`) and the
reference's do not. This is a **rotated** axis, so the wrap above cannot be the cause. The likely
seat is the anchor: `AddCategoryAxis` places a rotated label by the centre of its rotated bounding
box (`ChartLayout.cs:1994-2005`), on a measurement taken from `bnc889755.pptx`, whereas the
reference offsets the shape by `LabelPositionHelper::correctPositionForRotation` for the axis' own
`LabelAlignment` (`chart2/source/view/main/LabelPositionHelper.cxx:414-462`), which for a bottom
axis is `lcl_correctRotation_Bottom` with `bRotateAroundCenter = m_bComplexCategories` — **false**
for a simple category axis. Centre-anchoring and corner-anchoring differ by half the label's
rotated extent, which is the order of the overlap seen. Not carried to a measurement, not patched.
The discriminating probe is one chart, twenty long single-word categories, comparing the x of each
label's top-right corner against its tick in both PDFs.

---

## 8 · #186 · the Grand Total category — deliberately not patched

The brief asks to establish which side is right *"by reading the chart's own `c:ser` references
rather than by preference"*. **The `c:ser` references do not settle it.**

`053_Personal_asset_inventory_5446d84b.xlsx`, `xl/charts/chart11.xml`:

```xml
<c:cat><c:strRef><c:f>Assets!$H$24:$H$30</c:f>
  <c:strCache><c:ptCount val="6"/> … Car, Bonds, Stocks, Savings, 401k, House </c:strCache>
<c:val><c:numRef><c:f>Assets!$I$24:$I$30</c:f>
  <c:numCache><c:ptCount val="6"/> … 7500 … 250000 </c:numCache>
```

The declared range is **seven** cells; the cache holds **six**. `xl/worksheets/sheet11.xml` has
`H30 = "Grand Total"` and `I30 = 363500`, both real and neither hidden. We resolve the range, get
seven, and draw a `Grand Total` bar that rescales the axis to $400,000; the reference draws six and
scales to $300,000.

**The cache length is not the discriminator.** `ChartRangeResolver`'s own documentation
(`dotnet/src/XRay.Office.Ooxml/DrawingML/ChartRangeResolver.cs:52-60`) records the opposite case with
the identical file signature — `Keywords_Mapping_Graphs_and_Charts.xlsx`, twenty-two sequences each
exactly one cell short, grand-total row stated in `c:f` and absent from `c:numCache` — where the
reference **includes** the extra point and the axis runs to 40 rather than 8. Same signature,
opposite answer. A rule derived from the cache would fix this document and break eleven charts on
that one.

Two candidate real discriminators, both cross-lane and both needing the *sheet* model:

1. `ScChart2DataSequence::BuildDataCache` — *"Excel behavior: if the last row is the totals row,
   the data is not added to the chart. If it's not the last row, the data is added like normal"*
   (`sc/source/ui/unoobj/chart2uno.cxx:2612-2632`). This workbook does state it:
   `xl/pivotTables/pivotTable1.xml` carries
   `<location ref="H23:I30" firstHeaderRow="1" firstDataRow="1" firstDataCol="1"/>`, so row 30 is
   the grand-total row of a pivot table whose `rowGrandTotals` defaults on.
2. **The 26.2.4.2 probe's own warning**, which is why I stopped:
   `probes/sheets-r50-chartex/results.md` §5.2 — *"PivotTable regeneration — probably a
   reference-side divergence. LibreOffice re-generates the pivot from its cache with its own
   captions ('Total Result', 'Row Labels', 'Account Checking') and draws pivot borders; we replay
   Excel's cached strings ('Grand Total') and draw no borders. Reviewers 2 and 7 both flagged,
   unprompted, that ours may be the faithful side. Corroborated by the token census: `Grand`
   only-in-ours on 3 documents. **Measure whose string is in the cell before implementing anything
   here.**"*

So it is not established that the reference is right, the mechanism is not in `Core/Charts` or
`DrawingChart*.cs`, and I have written no patch. Filed under cross-lane dependencies.

#186's page-count divergence (4 against 2) is independent of the chart and belongs to the
pagination lane; the compared pages are pixel-identical.

---

## 9 · Documents in this lane that are not chart defects

Four of the fourteen are tagged `chart` because a reviewer described a *picture* of a Gantt chart
or a timeline. **None of them contains a chart part**: `unzip -l | grep -i chart` returns nothing
for all four, and their drawings are DrawingML shapes and Word tables.

| Case | Document | What it actually is |
|---|---|---|
| #014 | `015_Project_Timeline_Template_Colored_Background_…docx` | Page gradient not painted, table cell fills not painted (so white text on them vanishes — the 21% ink), chevron preset drawn as a plain rectangle. Shapes/backgrounds. |
| #114 | `013_Project_Timeline_Template_Blue_Background_…docx` | Page gradient, chevron preset, and two blocks of missing table text. Shapes/backgrounds + table text. |
| #173 | `045_Visual_Product_Roadmap_Template_…docx` | Page gradient, drop shadows, chevron preset, and white text inside a black box. Shapes/backgrounds. |
| #095 | `016_Project_Timeline_Template_Complete_Guide_…docx` | Tagged `lo-broken`, and it is: the **reference** loses the title banner, the header columns and the month band, keeping a stub of day numbers. We draw all of them. Our only defect is that our title banner is set too tall and covers the word beneath it — a table row height, not a chart. |

The chevron and the page gradient recur across three of the four and are one shape-lane root cause
between them, not three.

**#022 (`EHEST-Pre-departure-checklist…xls`)** does hold charts — nine `BOF` substreams of type
`0x0020` in its `Workbook` stream — but the divergence on its page 1 is not in any of them: the
sheet is not scaled to the printable width so the text column is clipped mid-word, and the
bold-underlined run-ins are printed plain. Sheet scaling and character formatting, not charts.

**#175 (`028_Unit_Circle_Chart_Optimized_Graph_…docx`)** is the mirror of #095. Its
`word/charts/chart1.xml` states `<c:ofPieChart>` with `<c:ofPieType val="bar"/>`,
`<c:secondPieSize val="75"/>` and `<c:serLines>`; we draw the bar-of-pie with its leader lines and
all sixteen legend entries, and the reference — LibreOffice **24.2**, which is what
`/data/bench/lo/…/out.pdf` reports as its Producer and which predates of-pie support — draws the
main pie only and truncates the legend at twelve. **Ours is the better output.** Our two real
defects on it are smaller: the plot is drawn smaller than the reference's and the inner data labels
overlap. Recommend re-tagging `lo-broken` rather than chasing it.

*This is the one case in the lane that is genuinely version-sensitive in the coordinator's sense: a
26.2 reference may well draw the secondary bar, which would turn this into a real defect about our
plot size rather than a reference deficiency.*

---

## Cross-lane dependencies

1. **`dotnet/src/XRay.Office.Spreadsheets/Ooxml/XlsxDrawings.cs:328-330` passes `styles: null` to
   `DrawingChartPlot.Read`.** With no `DrawingStyleMatrix` the automatic series line width is
   forced to zero — `AutoLineWidth` returns `Length.Zero` on its first line
   (`DrawingChartPlot.cs:1793-1806`) — so every Calc chart series that states no `a:ln w` is drawn
   at a hairline instead of at `lnStyleLst[1].w × relative/100`. On #166 that is 0 pt against the
   theme's `w="9525"` × 300% = **2.25 pt**; sampling our page 3 raster at 200 dpi finds the three
   line-chart series at exactly `4F81BD`, `C0504D`, `9BBB59` — the colours are right and the lines
   are one pixel wide, which is the whole of the "so pale they nearly vanish" reading. The needed
   change is to load the workbook theme's style matrix and pass it, exactly as
   `PptxSlideLayoutChart.cs:58-64` already does — and whose comment records that this precise gap
   cost several rounds on the slides side. **`dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxPictures.cs:266-269`
   has the same gap.**

2. **The range resolver must skip a trailing totals row.** See §8. Whoever owns
   `XRay.Office.Spreadsheets`' chart range resolution needs the pivot-table
   `location`/`rowGrandTotals` and the Excel-table `totalsRowShown` facts to reproduce
   `sc/source/ui/unoobj/chart2uno.cxx:2612-2632` — **and needs the 26.2 pivot-regeneration question
   settled first**, because it may be the reference that is wrong. Until then
   `053_Personal_asset_inventory` draws a category the reference does not.

3. **`dotnet/src/XRay.Office.Presentations/Layout/SlideChart.cs` runs a `\n`-bearing chart label
   together into one glyph run** — its own comment at `SheetChart.cs:29-35` says the slides path
   still has the defect the sheets and words paths fixed. §7 makes wrapped category labels possible
   on any host, so a `pptx` chart with a wrapping category axis would newly draw `ACCOUNTMANAGER`.
   The fix is the same three lines `SheetChart.Text` already has.

4. **Tests read bars and candles out of `drawing.Boxes`.** §1 moves them to `drawing.Shapes`; the
   six assertions are listed there. They are in `dotnet/tests/XRay.Office.Core.Tests/`, which no lane
   owns.

---

## Confidence, and what is not established

| RC | Documents | Confidence | What is unproven |
|---|---|---|---|
| 1 grid over bars | #158 #161 #188 | high | whether the axis *line* should also go under a bar |
| 2a legend, name via resolver | #166 | high | — |
| 2b legend, generated name | #188 | medium-high | the localized wording; Excel would say `Series1` |
| 3 `[CELLRANGE]` | #002 | high | — |
| 4 legend marker key | #055 | high | #055's plot width and the "tan gridlines" reading |
| 5 multi-level categories | #161 | high | — |
| 6 empty axis title | #161 | high | radar being the only exemption |
| 7 wrapped category labels | #158 | medium-high | regression reach; #188's rotated overlap is a different, undiagnosed cause |

Also not established: #166's y-axis label clipping and its `axPos="l"` category axis; the automatic
gridline colour (a known open 26.2 item, not re-derived); and whether 26.2.4.2 changes any of the
seven — bracketed by 24.2 output and 27.2 source throughout, and directly confirmed at 26.2 for
RC-2b and RC-3 only.
