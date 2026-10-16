# r102 — the smoothed line, the 3-D determination, and the of-pie

Round 102, seats **O35** (`c:smooth` unread), the **3-D** determination and the **of-pie**
question, plus the documentation defect `chart-types-r101` §5.1 left. Worktree
`/home/user/wt-chartsmooth`, branch `agent/chartsmooth`, base `50adaf649`.

Reference `/opt/libreoffice26.2/program/soffice` — LibreOffice **26.2.4.2**, build
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`. `/usr/bin/soffice` is 24.2.7.2 and was not used for
anything. The reference *renderings* are the banks at `/home/user/gate-orig-r83/ref` (947 PDFs,
`Producer: LibreOffice 26.2.4.2`) and `/home/user/gate-odf-r80/ref`, reused rather than
re-rendered; every `--convert-to` in this round was run fresh through 26.2.4.2 itself.

**The C++ tree at `/home/user/libreoffice-core` is 27.2.0.0.alpha0 and is not the reference
binary's source** (C8). Every `file:line` below is *this tree* and could not be checked against
26.2. That is why each arm has a second leg — a count of operators in 26.2.4.2's own PDF, or its
own `--convert-to` of the same file — and those legs stand on their own.

**There is no `Task`/subagent tool in this container**, so no page was read by anyone
uncontaminated. I looked at four rendered pages myself and say so where it matters; nothing below
rests on a reading — every figure is a count taken out of a PDF's content stream or out of
26.2.4.2's own flat ODF.

| seat | ends |
|---|---|
| **O35** `c:smooth` / `chart:interpolation` | **closed, fixed in this tree.** The flattening is reproduced to **0.0495 pt** against 26.2.4.2's own polyline, which is 1.75 hundredths of a millimetre — the grid the reference's geometry sits on. **Its reach is not the briefed 11 documents in either direction**: it is **12** by markup (an absent `c:smooth` means *smooth* outside Office 2007) and **2** in drawn geometry (nine of the eleven state arithmetic-progression data, whose spline is the straight line, and one suppresses its line altogether). |
| **the 3-D determination** | **measured, and it forbids most of an implementation.** **5 of the 6** corpus 3-D charts are a raster in the reference — the three OOXML pies, and *both* BIFF 3-D bars, which `chart-resid-r99` had not measured. The sixth, a `.ppt` OLE 3-D pie, is **vector**, 362 and 309 path items. So "we draw them flat" is a geometry defect on **1 of 947**, not 6. |
| **the of-pie** | **the brief's premise is refuted and the seat closed the other way round.** 26.2.4.2's *rendering* of the pie-of-pie witness does drop the sub-type — but **we did not match it, we drew the second pie** — so the reach was 1, not nil. Fixed: `\|ink\|%` **3.76 → 0.55**. The ODF half is fixed too (`loext:sub-bar`, 1 of 947, **3.54 → 1.41**). |
| **`ChartLayout.Plots.cs`:14-52** | **fixed.** The comment is withdrawn and replaced with what it was reaching for. |

---

## 0. What in the brief did not survive its own round

Three of the brief's claims are corrected in place below, and one of its two open questions was
answered by measurement rather than by the source it pointed at.

* **O35's reach is 12 documents by markup, not 11 — and 2 in ink.** The audit's own
  "corroborated at 12 documents by `chart:interpolation` in the reference's export" was treated as
  a discrepancy; it is not. `SeriesModel`'s `mbSmooth( !bMSO2007Doc )` makes an **absent**
  `c:smooth` mean *smooth*, and one corpus document is caught by it — §1.2. Going the other way,
  nine of the eleven witnesses hold arithmetic-progression data, so their cubic spline **is** the
  polyline and nothing about them can move — §3.1.
* **"All three corpus 3-D pies are drawn as a raster" understates it.** So are both BIFF 3-D bars.
  And one document the `r99` census never saw — a `.ppt` OLE 3-D pie — is drawn as **vectors**.
  §2.
* **"If the reference's rendering also drops it, we already match and the seat closes as nil
  reach."** The reference's rendering does drop it, and we did not match: at the base we drew five
  wedges in two circles where 26.2.4.2 draws four in one. §4.
* **The brief's two open questions** — what the spline is parameterised by, and whether the
  endpoints are natural — are settled by the source *and*, independently, by a fixture from
  26.2.4.2's own PDF that separates the candidates by a factor of 3.4 and 37. §1.1, §1.3.

---

## 1. O35 — the flattening, measured before it was implemented

### 1.1 Twenty segments per interval, counted in the reference's own output

`AreaChart::impl_createLine` hands a smoothed series to
`SplineCalculator::CalculateCubicSplines( *pSeriesPoly, aSplinePoly, m_nCurveResolution )` and
strokes the polygon that comes back (`chart2/source/view/charttypes/AreaChart.cxx`:331-336, this
tree). `m_nCurveResolution` is `20` at `AreaChart.cxx`:64 and
`ScatterChartTypeTemplate.cxx`:64. **That is a hypothesis about the binary**, so it was counted
instead: `segment-census.py` reads every long stroked all-straight path out of the reference
bank's own PDFs.

```
002/026/034/018/010_advanced_excel_line        220 segments per series   (12 points, 11 intervals)
006/022/030/014_advanced_excel_scatter         220                       (12 points, 11 intervals)
microsoft_learn_multi_chart_examples           60 and 200                (4 and 11 points)
171128IPAP                                     801, 830, 874             (145 points — see §1.4)
```

`220 = 11 × 20`, `60 = 3 × 20`, `200 = 10 × 20`. **Eleven documents, twenty-nine series, and no
other granularity fits any of them** — 10 would give 110, 30 would give 330. `segments.tsv`.

### 1.2 An absent `c:smooth` means *smooth*, and the corpus witnesses it both ways

`SeriesModel`'s constructor is `mbSmooth( !bMSO2007Doc )`
(`oox/source/drawingml/chart/seriesmodel.cxx`:124) and both the line and the scatter series
contexts read the element as `getBool( XML_val, !bMSO2007Doc )` (`seriescontext.cxx`:618, :726).
`bMSO2007Doc` is true only when `docProps/app.xml`'s `Application` begins with "Microsoft" **and**
`AppVersion` begins with `12.` (`oox/source/core/xmlfilterbase.cxx`:240-264), which is genuine
Office 2007 and almost nothing else.

Three corpus documents hold a line or scatter group that states no `c:smooth` at all
(`census-ooxml-smooth.py`, `ooxml-smooth-groups.tsv` — 48 documents hold such a group, 42 series
state nothing, 149 state `0`, 26 state `1`). They split exactly on the discriminator, and the
reference's own export agrees with the source reading on all three:

| document | `Application` / `AppVersion` | MSO 2007? | `chart:interpolation` in 26.2.4.2's own export |
|---|---|---|---|
| `slides/ceiling-002/pptx/Demick_JetBlue.pptx` | Microsoft Office PowerPoint / 12.0000 | yes | none |
| `slides/done-011/pptx/171128IPAP.pptx` | Microsoft Office PowerPoint / 12.0000 | yes | only on the four objects whose series state `val="1"` |
| `sheets/chartset-004/xlsx/microsoft_learn_multi_chart_examples.xlsx` | Microsoft Excel Compatible / Openpyxl 3.1.5 / **3.1** | **no** | **`cubic-spline` on two of its five charts** |

and the third one's reference PDF then draws **60 and 200** segments for a four-point and an
eleven-point series, which is the granularity of §1.1 on a document that states no `c:smooth`
anywhere. **Two independent legs, and they agree on both sides of the test.**

So the markup reach is **12 of 947**: the eleven `c:smooth val="1"` documents plus
`microsoft_learn_multi_chart_examples.xlsx`. That is the same 12 the ODF export leg found, and
they are the same twelve documents (`odf-chart.tsv`).

### 1.3 What it is parameterised by, and what the endpoints are

Both from `chart2/source/view/charttypes/Splines.cxx`:520-624 (this tree):

* **the parameter is the point's index**, `aParameter[nIndex] = aParameter[nIndex-1] + 1`
  (`:548-552`), under the function's own comment *"uniform parametric splines with subinterval
  length 1, according ODF1.2 part 1, chapter 'chart interpolation'"* — and **both** coordinates
  are splined against it, `aSplineX` and `aSplineY` being two separate calculations
  (`:571-611`);
* **the endpoints are natural**, `std::numeric_limits<double>::infinity()` passed as both first
  derivatives (`:584-591`), which `lcl_SplineCalculation::Calculate` reads as *"natural spline"*
  (`:150-155`) — with a **periodic** spline instead when the first and last points coincide in all
  three coordinates and there are at least three of them (`:578-583`).

**Neither of those is taken on the tree's word.** 26.2.4.2's PDF of
`microsoft_learn_multi_chart_examples.xlsx` page 3 strokes an eleven-point scatter series with
**unevenly spaced x** as 200 segments, so the file states both the input (every twentieth vertex,
which `Splines.cxx`:600-606 copies through unevaluated) and the expected output (all 201).
Feeding the first back through each candidate (`spline-hypotheses.py`, `hypotheses.txt`):

| | 4-point line, page 1 | **11-point scatter, uneven x, page 3** |
|---|---:|---:|
| **parameterised by index, natural ends** | **0.0427 pt** | **0.0495 pt** |
| parameterised by index, clamped ends | 0.1586 | 0.1700 |
| parameterised by x value, natural ends | 0.0416 | 1.8219 |
| no spline at all, the straight polyline | 0.5613 | 1.8183 |

The four-point series separates nothing much; the eleven-point one separates everything, because
its x is uneven. **And 0.0495 pt is the channel, not the model**: every one of those 201 vertices
is a whole number of hundredths of a millimetre from the first — `0.02835 pt` — to within 0.031 of
a unit, so the residual is 1.75 grid steps with the knots themselves quantised too.

`ChartSplineTests.TheFlatteningReproducesTheReferencesOwnPolyline` is that fixture, with a
tolerance of two grid units.

### 1.4 What is deliberately not reproduced, and why it does not show

The reference runs `lcl_removeDuplicatePoints` over the flattened polygon before clipping it
(`AreaChart.cxx`:335), dropping any point falling in the same cell as the last kept one on a grid
of `2 × 1000 × plotWidth / pageWidth` by the same in y
(`VCoordinateSystem::getCoordinateSystemResolution`, and
`PlottingPositionHelper::isSameForGivenResolution`,
`chart2/source/view/inc/PlottingPositionHelper.hxx`:280-320). It fires on dense series only:
`171128IPAP.pptx`'s 145-point series would flatten to 2880 segments and the reference strokes 801.
Its worst geometric effect is one grid cell — under a twentieth of a point on a corpus chart — so
it changes the operator count and not the ink, and **this project ranks on ink**. Recorded, not
implemented.

---

## 2. The 3-D determination — five of six are a raster, and the sixth is not a raster *or* a defect this seat can name

`threed-raster.py` takes each of the six documents `chart-types-r101` §4 counts, opens the
reference bank's own PDF at the chart's page, and asks two things of the chart's plot area: what
images the reference places over it, and what vector paths it draws inside it.

| document | 3-D chart | images over the plot area | vector paths inside it | |
|---|---|---|---|---|
| `words/chartset-001/docx/pie-chart-template.docx` | `c:pie3DChart` | 791×602 `/DCTDecode`, 190.0 × 144.6 pt | none | **raster** |
| `words/chartset-001/docx/pie-chart-result.docx` | `c:pie3DChart` | 791×602 `/DCTDecode`, 190.0 × 144.6 pt | six 5-item white label boxes | **raster** |
| `words/chartset-012/docx/021_…3D_Pie_Chart….docx` | `c:pie3DChart` | 1370×732 `/DCTDecode`, 522.3 × 278.8 pt | none | **raster** |
| `sheets/done-010/xls/TOGAF9-Tool-ConfReqts-CSQ.xls` | BIFF `CH3D` bar | 1528×656 `/FlateDecode`, 423.8 × 181.8 pt | none | **raster** |
| `sheets/missing-001/xls/orbus_togaf_tool_csq.xls` | BIFF `CH3D` bar | 1099×197 `/DCTDecode`, 264.0 × 47.2 pt | none | **raster** |
| `slides/done-004/ppt/undp_presentation_revised_17_may.ppt` | 3-D pie in an OLE `Excel.Sheet.8` | **none** | 362, 309, 55, 4, 4 items | **vector** |

`threed.tsv`. Three things follow and each of them is a scope decision rather than an
implementation.

**The two BIFF 3-D bars are raster too, and that is new.** `chart-resid-r99` §2.4 measured the
three OOXML pies and left the bars unmeasured. They behave identically: the reference draws the
chart's wall, its title, its axis labels and its legend keys as vectors and text, and puts the
**3-D scene alone** into an image. On `TOGAF9-Tool-ConfReqts-CSQ.xls` page 21 that image is the
floor and its ten gridlines and no bars at all, because the chart's source cells are empty — the
sheet prints `(empty)` above it. So even a correct 3-D bar implementation would have nothing to
draw there.

**The `.ppt` one is vectors, and it is the only one of the six a geometry fix could ever
match.** Slide 19 carries two filled-and-stroked paths of 362 and 309 items in the OLE object's
own frame — a blue 3-D pie of two slices with its extrusion — and no image anywhere near it. I
looked at the page myself at 110 dpi to confirm the shape is a pie and not something else; that
reading was not blind, and the 362/309 item counts are the claim.

**So the honest outcome is the brief's own "a seat that says so".** An implementation of 3-D
geometry can be measured against **1 of 947 documents**. For the other five, no vector drawing can
match a raster and no path metric would read the difference honestly — which is
`chart-resid-r99` §2.2's point arriving on four more documents. **No code was written for this.**

What is *not* settled: why the reference rasterises a 3-D scene reached through Writer and Calc
and does not rasterise one reached through Impress. Both are `chart2` output; the difference is in
what the host does with the primitive sequence, and this round did not chase it.

---

## 3. O35 in the corpus — per document, and the confinement

### 3.1 Nine of the eleven witnesses cannot move, and that is measured at the reference

`collinear.py`: every cached value sequence in the nine `advanced_excel` witnesses rises by a
constant, and a natural cubic spline through collinear points is the chord. The reference agrees —
its own flattened polylines depart from their own chords by:

```
002 0.0403   026 0.0405   034 0.0483   018 0.0429   010 0.0448        (line, 3 series each)
006 0.0357   022 0.0372   030 0.0326   014 0.0376                     (scatter, 1 series each)
microsoft_learn_multi_chart_examples                    4.2627        (the control: genuinely curved)
```

all inside the 1/100 mm grid. `collinear.tsv`. A tenth witness,
`055_Project_timeline_with_milestones`, states `c:smooth val="1"` on a series whose `a:ln` is
`a:noFill`, so no line is drawn at all and the reference strokes **no** long polyline on either of
its pages (`segments.tsv`, empty row).

**So the geometric reach of O35 is two documents**: `171128IPAP.pptx` and
`microsoft_learn_multi_chart_examples.xlsx`, plus their two ODF twins.

### 3.2 Confinement, measured rather than argued

Rendering the **48 corpus documents holding any line, scatter or 3-D-line plot group** and the
**12 ODF renderings stating `chart:interpolation`**, at the round's base and again after, under
`SOURCE_DATE_EPOCH`, with the `/CreationDate` and XMP dates masked:

**22 of 60 renderings change and 38 are byte-identical** — the eleven documents of §1.2 that draw
a line, in both their OOXML and their ODF form, and nothing else. `055` does not move, in either
form, which is the control §3.1 predicts. `confine-before.tsv`, `confine-after.tsv`,
`confine-ooxml.txt`, `confine-odf.txt`.

The same sweep re-run after the of-pie work of §4 gives **the identical 22**, so the two changes
do not overlap.

### 3.3 The scores, per document and not summed

`scores.tsv`; `|ink|%` summed over the document's pages, from
`.claude/skills/render-comparison/scripts/pdf-image-diff.py` against the two reference banks, at
the gate's own 512-pixel long edge and again at 1600.

| | 512 before → after | 1600 before → after |
|---|---|---|
| 002 / 006 / 010 / 014 / 018 / 022 / 026 / 030 / 034 `__xlsx` | 0.00→0.00 ×5, 0.14→0.14, 0.24→0.24 ×2 | 0.03→0.03 ×3, 0.06→0.06 ×2, 0.11→0.11 ×2, 0.69→0.69 ×2 |
| the same nine as `__ods` | 0.64…0.71, all unchanged | 0.60…0.69, all unchanged |
| `microsoft_learn_multi_chart_examples__xlsx` | 0.34 → **0.35** | 0.34 → 0.34 |
| `microsoft_learn_multi_chart_examples__ods` | 1.57 → 1.57 | 1.19 → 1.19 |
| `171128IPAP__pptx` (40 pages) | 10.28 → **10.29** | 6.93 → **6.94** |
| `171128IPAP__odp` (40 pages) | 30.98 → 30.98 | 25.12 → **25.35** |

**The ink does not move, and on three renderings it moves the wrong way by 0.01 to 0.23.** That is
the honest headline and it needs the paragraph below rather than a summed total.

### 3.4 What the ink metric is registering on the two documents that could move

The nine collinear witnesses are exactly 0.00 either way, as §3.1 says they must be. The two that
change shape are `171128IPAP` and `microsoft_learn`, and on those the pixel metric at 512 or 1600
cannot resolve a one-to-two-point displacement of a one-point stroke. `curve-deviation.py` asks
the question the metric cannot: for each of the reference's own series curves, the mean distance
from its vertices to ours, before and after — and again after removing the constant offset between
the two centroids, which separates *the wrong shape* from *the wrong place*.

| `171128IPAP.pptx` | ref segs | our segs before → after | mean pt before → after | shape only, before → after |
|---|---:|---|---|---|
| p36 series 0 | 830 | 42 → 830 | 1.57 → **1.11** | 2.02 → **0.66** |
| p37 series 0 | 830 | 42 → 830 | 2.15 → **1.81** | 2.34 → **0.66** |
| p38 series 0 | 801 | 41 → 801 | 1.99 → **1.82** | 1.61 → **0.91** |
| p38 series 1 | 801 | 41 → 801 | 2.13 → **1.73** | 1.53 → **0.94** |
| p38 series 2 | 801 | 41 → 801 | 2.03 → **1.89** | 1.78 → **1.11** |
| p40 series 0 | 874 | 44 → 871 | 26.63 → **25.43** | 6.79 → **5.65** |

**Six of six improve on both columns**, and the shape-only column roughly halves on five of them —
which is what a correct flattening should do and a wrong one could not. `curve-deviation.tsv`.

What the ink is registering instead is the residual *position* error, which is a different seat:
those curves are still 1 to 2 pt from the reference's on pages 36-38 and **44.7 pt** below it on
page 40, where that chart's value axis is wrong. A curve that follows the reference's shape but
sits two points away from it disagrees on more pixels than a chord that cut the corner and
happened to overlap; that is arithmetic about a one-point pen, not evidence about the spline.
`171128IPAP.odp`'s +0.23 is the same four pages (1.32→1.37, 1.46→1.54, 0.52→0.60, 1.82→1.84) and
its signed `ink%` rises with it, so it is ink we now draw and the reference draws two points away.

**So O35 is closed on the measurement that can see it and does not pay for itself on the one that
cannot.** Whoever takes the residual on `171128IPAP` should take the axis, not the curve.

---

## 4. The of-pie — the brief's premise refuted, and 4.34 of `|ink|%` recovered

### 4.1 What 26.2.4.2 draws for the one pie-of-pie witness

`words/chartset-011/docx/029_Unit_Circle_Chart_Pie_Theme_8a922142.docx` states
`<c:ofPieType val="pie"/>` over four data points. Its page in the reference bank carries **one
circle of four wedges**, four data labels reading `1st Qtr 50%`, `2nd Qtr 25%`, `3rd Qtr 15%`,
`4th Qtr 10%`, a four-entry legend, **no composite slice, no second plot and no connector lines** —
read out of the content stream, and the wedge count is unambiguous once the per-point gradient
fills are removed (`029-no-dPt` in `ofpie.txt` draws 15 paths, of which four are the wedges). Its
`--convert-to fodt` carries a bare `chart:class="chart:circle"` with no `loext:sub-pie`.

So the brief's question is answered: **the rendering drops the sub-type exactly as the export
does.**

**The inference drawn from it is wrong.** At the round's base *we* drew five wedges in two
circles — three at x 124…410 and two at x 490…631 — so the seat was a real one-document
divergence of ours, in the opposite direction, worth `|ink|%` **3.76**.

### 4.2 Why, measured one attribute at a time and in both directions

`ofpie-variants.py` builds twelve variants of the corpus's two of-pie documents, each differing
from its original in one element, and puts each through 26.2.4.2 twice — `--convert-to fodt` for
the resolved model and `--convert-to pdf` for the drawing. `ofpie.txt`:

| variant | model |
|---|---|
| `029` as it is | no of-pie |
| `029` with `c:ofPieType` swapped to `bar` | no of-pie |
| `029` with its **`c:explosion` removed** | **`loext:sub-pie="true" loext:split-position="2"`** |
| `029` with its seventeen `c:dPt` removed, explosion kept | no of-pie |
| `029` with both removed | **`loext:sub-pie="true"`** |
| `028` as it is | `loext:sub-bar="true" loext:split-position="2"` |
| `028` with `c:ofPieType` swapped to `pie` | `loext:sub-pie="true"` |
| `028` with its `c:dPt` removed | `loext:sub-bar="true"` |
| `028` with **`<c:explosion val="1"/>` added** to its series | **no of-pie** |

**`c:explosion` on the series decides it, in both directions, on both documents.** `c:dPt` does
not: `028` carries seventeen of them and keeps its sub-bar, and removing `029`'s while leaving its
series' explosion does not bring the sub-pie back.

The mechanism is **not** established. `PieChartTypeTemplate::matchesTemplate` reads the series'
`Offset` and answers `PieChartOffsetMode_ALL_EXPLODED`
(`chart2/source/model/template/PieChartTypeTemplate.cxx`:307-366, this tree), which is the obvious
candidate for re-templating the diagram out of its of-pie type — but that code requires every
point's offset to equal the series', and `029`'s do not. It is a lead, not the answer.

### 4.3 The fix, and its reach

`DrawingChartPlot.IsExploded`: an `c:ofPieChart` group whose series states a non-zero
`c:explosion` is read as a plain pie. Censused over every OOXML document in the corpus: **two
of-pie groups exist, in two documents, and one of them is exploded.**

| | `\|ink\|%` before | after |
|---|---:|---:|
| `029_Unit_Circle_Chart_Pie_Theme_8a922142.docx` | 3.76 | **0.55** |
| `028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx` (the control) | 3.47 | 3.47 |

### 4.4 The ODF half, which is a different document and a different spelling

ODF has no of-pie class — `aXMLChartClassMap` holds thirteen and none is one
(`xmloff/source/chart/SchXMLTools.cxx`:136-151) — so LibreOffice writes the sub-type as two
extension attributes on `chart:chart` beside `chart:class="chart:circle"`:
`SchXMLExport.cxx`:1334-1337 and :1370 write `loext:sub-bar` / `loext:sub-pie` and
`loext:split-position`, and `SchXMLChartContext.cxx`:446-458 read them.

**Only the `loext:` spelling exists.** This is the sixth instance of the namespace trap the family
notes record; a reader looking for `chart:sub-bar` finds it in no file at all. Censused over
`/home/user/corpus-odf`: **1 of 947 renderings** states one, and it states it as `loext:` —
`words/chartset-010/odt/028_Unit_Circle_Chart_Optimized_Graph_83d9c756.odt`, whose `chart:chart`
reads `chart:class="chart:circle" loext:sub-bar="true" loext:split-position="2"`
(`odf-chart.tsv`). Reading it takes that rendering from `|ink|%` **3.54 → 1.41**; its `029` twin,
which correctly states nothing, is 0.79 before and after.

### 4.5 What is left open on this seat

Two things, both measured and neither modelled.

* **The pie sub-type carries a second condition beyond the explosion.** With the explosion
  removed, `029`'s model carries `loext:sub-pie="true"` and 26.2.4.2 still draws **one** pie — at
  four, five, six and seven points — while the same file with `c:ofPieType` changed to `bar` draws
  a bar-of-pie at four points, and `028` with `pie` forced onto it draws a full pie-of-pie at
  sixteen. So something between eight and sixteen points, or something else about the geometry,
  gates the pie form and not the bar form. The 27.2 tree's own gate is
  `nPointCount >= OfPieDataSrc::minPoints` with `minPoints = 4`
  (`chart2/source/view/charttypes/PieChart.cxx`:1053-1056, `PieChart.hxx`:108), which the
  measurement does not fit — this is C8 again.
* **It costs nothing here**, because the corpus's only pie-of-pie is the exploded one and the
  explosion rule already covers it. Modelling a threshold fitted to two documents is exactly the
  plausible reading this session keeps having to retract, so none is offered.

---

## 5. The documentation defect

`ChartLayout.Plots.cs`:7-52 argued at length that *"a surface chart's frame stays empty"* and that
drawing one as a bar chart was *"the tempting shortcut"*, which
`DrawingChartPlot.KindOf`:578 and `OdfChartPlot.KindOf`:384 have contradicted for several rounds.
Withdrawn and replaced with what it was reaching for: **`Surface` has never been a chart type in
LibreOffice, in either version** — `template.Surface` is commented out of the registry
(`ChartTypeManager.cxx`:223) and there is no `SurfaceChartType` among the nineteen services in
`servicenames_charttypes.hxx`:23-59, nor either name in 26.2.4.2's own `libmergedlo.so` or
`services.rdb` — and all three importers substitute a column chart with the same `// Todo`
(`typegroupconverter.cxx`:79, `xlchart.cxx`:469, `SchXMLTools.cxx`:148). Its reach is nil in all
four streams, twice over, per `chart-types-r101`. **Nothing renders differently.**

---

## 6. Deliverable state

Built clean, `TreatWarningsAsErrors` on, **0 warnings 0 errors**.

The ten non-fidelity projects, run one at a time and totalled from this run's own output:

```
Containers 109   Core 535   Markup 259   OpenDocument 160   Presentations 1066
Rendering  164   Spreadsheets 1292   Text 728   Vector 309   WordProcessing 1938
                                                   6560 passed, 0 failed, 0 skipped
```

`XRay.Office.Fidelity.Tests`: **Failed: 10, Passed: 542, Total: 552**, and the ten names read out of
that run's own log are exactly the briefed set — `PageDrawingComparisonTests` ×4 (`paginated`
`.fodt/.doc/.docx/.rtf`), `TabStopComparisonTests` ×4 (`list-label-overrun`
`.fodt/.docx/.doc/.odt`), `SheetDrawingComparisonTests` (`sheet-rich-text.xlsx`) and
`JustificationShrinkComparisonTests` (`justify-shrink-2013.docx`). No eleventh failure.

**New tests**, 21 in all, every one of which needs API this round added and so cannot compile at
the base:

* `XRay.Office.Core.Tests/ChartSplineTests.cs` — 7. The reference fixture of §1.3, the segment
  count and copied knots, the collinear case, one and two points, and two through `ChartLayout`:
  twenty segments per interval against one, and a gap ending the curve rather than being smoothed
  across.
* `XRay.Office.Presentations.Tests/DrawingChartSmoothTests.cs` — 11. The stated flag on all three
  non-frame groups, the Office-2007 default both ways, one series smoothing its group, the five
  group kinds that are never smoothed, and a combination chart smoothing only its line half.
* `XRay.Office.OpenDocument.Tests/OdfChartInterpolationTests.cs` — 10 (4 new beyond the smoothing
  ones): `loext:sub-bar` / `loext:sub-pie`, the stated split position, and the control.
* `XRay.Office.Presentations.Tests/ChartPlotTypeReaderTests.cs` — 1 added, the exploded of-pie.

**Files here**, all present and non-empty: `results.md`; five probes — `census-ooxml-smooth.py`,
`census-odf-chart.py`, `segment-census.py`, `spline-hypotheses.py`, `collinear.py`,
`threed-raster.py`, `curve-deviation.py`, `ofpie-variants.py` — and their outputs
`ooxml-smooth-groups.tsv` (83 rows), `odf-chart.tsv` (20), `segments.tsv` (13), `hypotheses.txt`,
`collinear.tsv` (11), `threed.tsv` (7), `curve-deviation.tsv` (7), `ofpie.txt` (13),
`scores.tsv` (27), and the confinement pair `confine-before.tsv` / `confine-after.tsv` (60 rows
each) with the two document lists they were taken over.

---

## 7. Register

`dotnet/probes/OPEN-ISSUES.md`:

* **O35 — closed, fixed in this tree.** Reach 12 of 947 by markup and 2 of 947 in drawn geometry;
  the flattening reproduces 26.2.4.2's own polyline to 0.0495 pt over 262 vertices; the mean
  curve-to-reference distance halves on six of six series of the one document where the ink can be
  seen at all.
* **N29 — nil, and it forbids an implementation.** 5 of the 6 corpus 3-D charts are drawn by
  26.2.4.2 as a raster over the plot area — the three OOXML pies *and both* BIFF 3-D bars — so no
  vector geometry can match them and no path metric would read the difference honestly. The sixth,
  a `.ppt` OLE 3-D pie, is vector; a 3-D implementation has **one** corpus document to be measured
  against.
* **O38 (new, replacing the of-pie half of O36's neighbour) — the exploded of-pie is closed; the
  pie sub-type's second condition is seated.** The exploded rule is fixed in both streams (OOXML
  `|ink|%` 3.76 → 0.55 on 1 of 947; ODF `loext:sub-bar` 3.54 → 1.41 on 1 of 947). What is left is
  §4.5: with the explosion gone, 26.2.4.2 still draws a *pie*-of-pie as a plain pie at four to
  seven points and as a full one at sixteen, and the gate is not established.
