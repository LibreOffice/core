# The three chart successors: O27 closed, O31 re-characterised, O26 refuted and re-seated

Round 99, seats **O27**, **O31** and **O26**, worktree `/home/user/wt-chartresid2`, branch
`agent/chartresid2`, base `9e981e16a`. Reference `/opt/libreoffice26.2/program/soffice`
(LibreOffice **26.2.4.2**); `/usr/bin/soffice` is 24.2.7.2 and was not used.

**The reference half of every rendering figure is the bank at `/home/user/gate-orig-r83/ref/`,
reused rather than re-rendered**; only our half was swept, which a diff confined to `dotnet/src`
makes sound. The one thing re-run through 26.2.4.2 itself is a single `--convert-to fods`, in §1.

**There is no `Task`/subagent tool in this container**, so no page was read by anyone
uncontaminated — and nothing below rests on a reading. Every reference figure is a number pulled
out of 26.2.4.2's own flat ODS, its own PDF content stream, or its own embedded raster.

**The C++ tree at `/home/user/libreoffice-core` is not the reference binary's source**
(`configure.ac`:21 is `27.2.0.0.alpha0+`; the binary is 26.2.4.2). Every line number below is
*this tree*, and could not be checked against 26.2. Each arm's other leg — the `.fods`, the
rendered PDFs — measures the actual reference and stands on its own.

| seat | ends |
|---|---|
| **O27** | **closed, fixed in this tree.** Reach **7 of 307** spreadsheet renderings; 300 byte-identical. |
| **O31** | **still seated, re-characterised and shrunk.** The seat is not the labels: 26.2.4.2 draws the whole 3D pie *body* as a raster, and 86 % of the missing ink is that body. |
| **O26** | **still seated. Its stated mechanism is wrong, its stated counter-example is not one, and the permissive rule is measurably worse on all four documents including the two it was filed for.** |

---

## 0. What the round did to the brief

Three of the brief's claims did not survive its own round, and each is corrected in place below.

* **O27 had not closed** (the brief asked first whether it had). At `9e981e16a` the logbook's plot
  area was still 596.30 pt against 632.66, to the centipoint — §1.
* **O31 is not "the label placement of a 3D pie".** The labels are **13.5 %** of the missing ink
  on `021`; the other 86.5 % is a pie body that 26.2.4.2 does not draw as vectors at all — §2.
* **O26's `044_Cash_flow_forecast` is not a counter-example.** `clip-seats-r97`'s own
  `overreach.tsv` gives it **`over_px = 0` on every one of its five pages** — the block clip
  removes no ink there that the reference keeps. Its "6.55 of `|ink|%`" came from suppressing
  *every* block clip on the page, which lets our own excess spill; it is not evidence that a
  chart is clipped at the block on `044`. And the two documents the seat *is* filed for get
  **worse** under the permissive rule — §3.

---

## 1. O27 — a BIFF chart states its plot area, and we were computing one

### 1.1 The seat, re-measured at this HEAD

`Template Pilot Logbook JAR-FCL V3.0.xls` pages 17-18 carry one chart across two column bands.
Its plot rectangle is the wall's fill and the six gridlines drawn across it — seven paths that
agree on one *x* extent on each side (`clip-rects.py --paths`, from `probes/clip-seats-r97`):

| | left | right | width | top | bottom |
|---|---:|---:|---:|---:|---:|
| 26.2.4.2 | 153.07 | 785.73 | **632.66** | 130.93 | 427.66 |
| ours, at `9e981e16a` | 165.45 | 761.75 | **596.30** | 130.80 | 414.61 |
| ours, after | **152.99** | **785.40** | **632.41** | 130.80 | 427.00 |

So the seat is real at this HEAD and is now closed to **0.08 pt on the left edge, 0.33 on the
right, 0.25 on the width**, from 12.38 / 23.98 / 36.36.

### 1.2 Why: BIFF charts have no automatic plot area

`XclImpChChart::Convert` (`sc/source/filter/excel/xichart.cxx`:4030-4048, this tree):

```
// plot area position and size (there is no real automatic mode in BIFF5 charts)
XclImpChFramePosRef xPlotAreaPos = mxPrimAxesSet->GetPlotAreaFramePos();
if( IsManualPlotArea() && xPlotAreaPos ) try
{
    const XclChFramePos& rFramePos = xPlotAreaPos->GetFramePosData();
    if( (rFramePos.mnTLMode == EXC_CHFRAMEPOS_PARENT) && (rFramePos.mnBRMode == EXC_CHFRAMEPOS_PARENT) )
    {
        css::awt::Rectangle aDiagramRect = CalcHmmFromChartRect( rFramePos.maRect );
        …
        else
            xPositioning->setDiagramPositionIncludingAxes( aDiagramRect );
    }
}
```

Three things follow, and all three are measured in §1.3 rather than taken on the source's word:

* the rectangle is the **primary axes set's `CHFRAMEPOS`** (0x104F) and no other — the corpus'
  chart substreams carry the same record under `CHTEXT` and under `CHLEGEND` as well;
* it is the **outer** rectangle, `setDiagramPositionIncludingAxes` — the axes' labels come out of
  it, their titles do not go into it;
* the units are 1/4000 of the chart frame, converted by `CalcHmmFromChartRect`
  (`xichart.cxx`:320-327) as `unit * n + gap` with `unit = (frame - 2*gap)/4000` and
  `gap = GetHmmFromPixelX(5.0)` (`xlchart.cxx`:1245-1251). **The gap is added to the width as
  well as to the position**, which looks like a slip and is what the function does.

`IsManualPlotArea()` (`:3973-3977`) is `EXC_CHPROPS_USEMANPLOTAREA` **or** BIFF5-and-earlier.
Only the flag is modelled; the generation arm has no witness, because every one of the corpus'
sixteen BIFF chart substreams sets the flag (§1.4).

### 1.3 26.2.4.2's own resolved model, and the gap solved out of it

`soffice --convert-to fods` prints the reference's resolved chart model without rendering
anything (`fods-geometry.txt`, produced this round). The logbook's two charts:

| | frame | plot area 26.2.4.2 resolves | `CHFRAMEPOS` in the file |
|---|---|---|---|
| sheet `GraphHDV` | 22.66 x 14.447 cm | x 1.192 y 1.418 w 18.454 h 12.147 cm | `(170, 335, 3286, 3412)` |
| sheet `Real TT` | 28.609 x 14.447 cm | x 1.192 y 1.418 w **23.623** h 12.147 cm | `(134, 335, 3326, 3412)` |

(the `CHFRAMEPOS` values are read straight out of the `.xls` by `biffchart.py`, written this
round; both records state `TL=2 BR=2`.)

Solving `unit * n + gap = resolved` for the gap, in hundredths of a millimetre:

* chart 1, x: `170*(22660-2g)/4000 + g = 1191.5` → **g = 249.7**
* chart 1, y: from the pair (335 → 1418) and (3412 → 12147), `unitY = 3.4868` → **g = 249.4**,
  and `(14446-500)/4000 = 3.4865` closes the loop.

So **g = 250** in both axes, and 250 is `5 x 50.0`: `XclRootData`'s `mfScreenPixelX` is
initialised to **50.0** hundredths of a millimetre (`sc/source/filter/excel/xlroot.cxx`:105) and
replaced only when an active frame can be asked for its output device (`:150-163`) — which
headless conversion cannot. **This is a headless-only constant**, and headless 26.2.4.2 is what
this project calibrates against. It is stated as a measurement, not as a reading.

With g = 250 the arithmetic reproduces the reference exactly: chart 2's plot area resolves to
x **1192**, width **23623** — which is what `ChartStatedOuterPlotAreaTests` asserts, and 23.623 cm
is what the reference's own `.fods` says.

Two further checks that the rectangle is the **outer** one, taken off the rendered page rather
than off the source:

* the reference's leftmost value-axis label starts at x = 116.16 on page 17, and the plot area's
  stated left edge resolves to 33.79 pt from the chart frame's left — the same point to
  **0.03 pt**. The label begins exactly where the stated rectangle begins.
* the reference's wall ends at x = 785.73 and the stated rectangle's right edge resolves to
  703.32 pt from the frame's left — again the same point. Nothing is reserved on that side, so
  outer right and wall right coincide.

### 1.4 What was implemented

* **`ChartPlot.OuterPlotAreaUnits`** — `(X, Y, Width, Height)` in 1/4000 of the frame, or null.
  It stands beside `ChartPlot.PlotArea` (ODF's *inner* `chart:coordinate-region`) rather than
  replacing it, because what it supplies is the rectangle the labels are then taken **out of**.
* **`XlsChartBuilder.ReadPlotAreaPos`** — `CHFRAMEPOS` under the primary `CHAXESSET` only, both
  modes `EXC_CHFRAMEPOS_PARENT`, and only when `CHPROPERTIES` sets `EXC_CHPROPS_USEMANPLOTAREA`.
* **`ChartLayout.StatedOuterArea`** — the `CalcHmmFromChartRect` arithmetic, run in hundredths of
  a millimetre because that is the unit its integer truncation is defined in; `PlotAreaOf` takes
  it in place of `DiagramAreaOf`'s computed rectangle and reserves the axis labels out of it
  exactly as before.

**Two branches of `:4040-4046` are not modelled, and neither has a witness.** A pie category
takes `setDiagramPositionExcludingAxes` (the inner rectangle) and a 3D chart takes
`setDiagramPositionIncludingAxesAndAxisTitles`. `census-3d.py` and `biffchart.py` over all 64
corpus `.xls` files: **0 of 16 BIFF chart substreams is a pie**, and the **2 that are 3D**
(`TOGAF9-Tool-ConfReqts-CSQ.xls`, `orbus_togaf_tool_csq.xls`) state **no axis title at all** —
their only `CHOBJECTLINK` targets are 1 (the chart title) and 4 (data), never 2 or 3 — so the
axis-title branch and the plain one coincide on both. Modelling either would be a change with no
measurement behind it.

### 1.5 Tests

| test | at the base | after |
|---|---|---|
| `XlsChartPlotAreaTests` (6, `XRay.Office.Spreadsheets.Tests`) | 1 fails | all 6 |
| `ChartStatedOuterPlotAreaTests` (4, `XRay.Office.Core.Tests`) | **all 4 fail** | all 4 |

"At the base" is the two arms switched off in place (`StatedOuterArea` bypassed;
`OuterPlotAreaUnits` forced null), because reverting the files makes the round's own tests fail
to *compile* and a compile error is not a failing test. The five reader cases that pass at the
base are the negative ones — a title's `CHFRAMEPOS`, the secondary axes set's, a non-parent
position mode, an empty rectangle, and an automatic layout — and they pass at the base for the
trivial reason that nothing is read at all; they exist to stop the *permissive* reading, which
the corpus would have broken immediately (`Template Pilot Logbook` writes four `CHFRAMEPOS`
records under `CHTEXT` before the axes set's).

### 1.6 Corpus reach, measured

`sweep-ours.py` over the whole `sheets/` column — the only column that can reach
`XlsChartBuilder` — with the arm off and again with it on, `SOURCE_DATE_EPOCH=1757462400`.
**307 of 307 rendered on both sweeps and none failed.**

```
compared 307 of 307; differ 7
```

and the seven are exactly the seven documents `census-plotpos.tsv` predicts — every `.xls` in the
corpus holding a chart substream with a primary-axes-set `CHFRAMEPOS` and the manual flag. **The
census predicted 7 and the sweep found 7**, with no under-count in either direction; the eighth
`.xls` naming a chart, `Special-Procedures_2025-07-10.xls`, is not an OLE2 file at all.

| document | diff% before | diff% after | `\|ink\|%` before | `\|ink\|%` after | pages |
|---|---:|---:|---:|---:|---:|
| `014_Contextures_chart_sample_991ecfc5` | 8.64 | **2.65** | 1.11 | **0.50** | 3 |
| `2012-GA-Survey-Chapter-5-Tables` | 4.50 | **3.92** | 0.40 | **0.31** | 3 |
| `2012-GA-Survey-Chapter-6-Tables` | 6.68 | **6.49** | 0.14 | **0.13** | 5 |
| `EHEST-Pre-departure-checklist` | 83.56 | **73.85** | 6.88 | **8.65** | 24 |
| `TOGAF9-Tool-ConfReqts-CSQ` | 51.44 | 51.68 | 16.76 | **16.75** | 28 |
| `Template Pilot Logbook JAR-FCL V3.0` | 38.33 | **27.13** | 4.66 | **1.72** | 38 |
| `orbus_togaf_tool_csq` | 341.30 | 341.44 | 51.20 | **50.90** | 75 |
| **total** | **534.45** | **507.16** | **81.15** | **78.96** | 176 |

No document changed its page count. `Template Pilot Logbook` loses one of its two MAJOR pages;
no other verdict moves.

### 1.7 Every mover, and what 26.2.4.2 draws there

Round 97's hardest-won lesson was that ranking on a total would have missed a change that cut
three header banners the reference draws at full width. So each of the seven was opened:

| document | what moved | ours before | ours after | 26.2.4.2 |
|---|---|---|---|---|
| `Template Pilot Logbook` p17 | the chart wall | 165.45 … 761.75 | **152.99 … 785.40** | 153.07 … 785.73 |
| `014_Contextures` p1 | the chart wall | 69.88, 125.85 – 432.73, 270.19 | **92.88, 120.08 – 509.75, 274.39** | 93.68, 120.87 – 509.22, 274.08 |
| `2012-GA-Survey-Ch5` p1 | the value axis line | x 679.91 | **x 635.80** | x 635.84 |
| `2012-GA-Survey-Ch6` p1 | the value axis line | x 681.99 | **x 637.26** | x 636.46 |
| `EHEST` p17 | the chart wall | 74.61, 589.47 – 446.47, 728.81 | **73.31, 581.98 – 478.06, 732.77** | 65.19, 579.62 – 478.95, 739.81 |
| `TOGAF9` p21-22, `orbus` p27-28 | a 3D bar chart's plot | — | — | — |

Five of the seven move decisively toward the reference. The two 3D bar charts move by almost
nothing on either metric (`|ink|%` −0.01 and −0.30, `diff%` +0.24 and +0.14): their pages are
dominated by an unrelated defect — 26.2.4.2 draws the page's banner 505.4 pt wide where we draw
208.4, and it draws *one* text span past x = 250 on `TOGAF9` p21 where we draw a whole column of
category names — so the plot rectangle is not what those documents' ink is about.

**`EHEST` is worse on `|ink|%` and better on everything else, and this is the metric, not the
tree.** Its plot wall on p17 goes from 41.90 pt too narrow to 9.01 pt too narrow, its right edge
from 32.48 pt out to **0.89**, its bottom from 11.00 to 7.04 — every edge closer. Its raw
disagreeing-pixel count falls **83.56 → 73.85**, 12 %. `|ink|%` rises because a region's
`luma_gap` is the *signed* mean brightness difference over the region and `page_ink_abs` takes
the absolute value of each region's mean (`pdf-image-diff.py`:246-272 and :434-437) — so ink we
wrongly draw on one side of a connected region cancels ink we wrongly miss on the other, *inside*
the column this project ranks on, and making the wall bigger stops a cancellation. That is
`probes/clip-seats-r97` §2.2's finding, met again. The geometry is the arbiter here and it is
unambiguous; `score-both.py` reports both columns from now on so the two can be told apart
without re-deriving it.

### 1.8 Confinement

* **300 of 307** spreadsheet renderings byte-identical.
* The other 640 corpus renderings cannot be reached: `OuterPlotAreaUnits` is set by
  `XlsChartBuilder` and by nothing else, and `StatedOuterArea` returns null when it is null, so
  `PlotAreaOf` is unchanged for every OOXML, ODF and BIFF-less chart in the corpus. That is an
  argument and not a sweep; the sweep covers every document that can reach the code.
* Whole suite, read out of this run's own last lines: ten non-fidelity projects **0 failed of
  6514** (Containers 109, Core 528, Markup 259, OpenDocument 146, Presentations 1047,
  Rendering 164, Spreadsheets 1286, Text 728, Vector 309, WordProcessing 1938), and
  `XRay.Office.Fidelity.Tests` **542 passed / 10 failed of 552** — PageDrawing x4, TabStop x4,
  SheetDrawing, JustificationShrink, and no eleventh name.

---

## 2. O31 — 26.2.4.2 does not draw a 3D pie as vectors, and the labels are 13 % of the seat

The seat says `021_Unit_Circle_Chart_3D_Pie_Chart` places its pie's data labels 23-68 pt from
26.2.4.2's, and asks for the **1.84 → 3.47** regression round 97 accepted to be paid back.

**It cannot be paid back by moving labels, because the labels are not where the ink is.**

### 2.1 The reference's pie is a raster

`021`'s page 1 in the bank carries two images. One is a 1370 x 732 `DCTDecode` raster placed at
**28.75, 246.95 – 551.05, 525.75** (522.3 x 278.8 pt): the entire 3D pie. The reference's own
`get_drawings()` on that page returns **no pie paths at all** — only the chart's white background
and the seven white boxes behind the body text. We draw the pie as four filled vector wedges.

All three of the corpus' 3D pies are rasters in the reference, at 791 x 602 for the two small
ones (`pie3d-scores.tsv`).

### 2.2 The split, counted

`inksplit.py` renders both pages at 120 dpi, takes the ink pixels of each, and asks of every
disagreeing pixel whether it falls inside **any** drawn text span of *either* rendering — the set
a label-placement fix could possibly move:

| | pixels | in a text box | outside one |
|---|---:|---:|---:|
| the reference inks, we do not | **97,802** (7.02 % of the page) | 13,189 | **84,613** |
| we ink, the reference does not | 16,535 (1.19 %) | 4,402 | 12,133 |

**86.5 % of the reference's missing ink on `021` is the pie body.** A perfect label placement
leaves 84,613 + 12,133 px of body error untouched. `pdf-image-diff.py` scores the whole chart as
*one* region ("middle-centre: ink missing from ours — a graphic, glyphs or a fill, 14.05 % of the
page"), inside which the body and the labels cancel — which is also why the seat's 1.84 → 3.47
tracks the body's overlap and not the labels'.

### 2.3 What the reference's projection is, as far as it is measured

From the raster itself (saturated pixels, `>40` between the max and min channel):

* the coloured body spans **469.31 x 278.04 pt**;
* its widest row is at y = 362.74, x 54.67 … 523.98 — so the disc's horizontal semi-axis is
  **a = 234.66** and its centre x is 289.33;
* the top of the colour is y = 246.95, giving a vertical semi-axis **b = 115.79**, and
  **b / a = 0.4934** — within half a per cent of **sin 30°**, and the file states
  `<c:rotX val="30"/>`;
* the remaining **46.5 pt** below the widest row is the extrusion, `<c:depthPercent val="100"/>`.

We draw a circle of diameter **325.14 pt** (x 135.08 … 460.22, y 233.43 … 540.18) centred on the
frame — inscribed in the shorter side of the diagram rectangle, which is what a flat pie wants.
So the shape is wrong before the labels are: the reference's body is 44 % wider and 9 % shorter
than ours.

**What is not established is where `a = 234.66` comes from.** The diagram rectangle is 575.5 pt
wide, so the reference's disc is 0.815 of it; a rule fitted to one document is exactly the kind
of plausible reading this session keeps having to retract, and none is offered.

### 2.4 The seat's size

`census-3d.py` over all 947 corpus documents: **5 hold a 3D chart**, and 3 of those are 3D pies
(`021_Unit_Circle_Chart_3D_Pie_Chart`, `pie-chart-result`, `pie-chart-template`; the other two
are the BIFF 3D bars of §1.4). Summed `|ink|%` over the three pies is **4.05**, of which
`021` is **3.47** and the other two are 0.54 and 0.04 — their pies are 190 x 144.6 pt.

### 2.5 State

**Still seated, and re-worded.** The seat is *"we draw a 3D pie as a flat vector disc inscribed in
the diagram rectangle; 26.2.4.2 projects it through its 3D engine and rasterises the result, an
ellipse of semi-axes 234.66 x 115.79 pt — b/a = sin(rotX) — plus a 46.5 pt extrusion"*. Worth
**4.05 summed `|ink|%` over 3 of 947 documents**, of which the labels are at most 13.5 %. The
round-97 regression is a consequence of the body's overlap and **is not repaid**; the frame it
sits in is still provably the reference's own, so the O23 fix is not in question.

The distinguishing question left for whoever takes it: **what sets the projected disc's radius?**
Both a rule fitted to `021` alone and a rule fitted to the two 190-pt pies would be a guess; three
documents with three different frame aspect ratios is enough to separate "a fraction of the
diagram width", "a fraction of the diagram height" and "the projected bounding box fitted to the
diagram", and none of the three has been tested.

---

## 3. O26 — the mechanism is misattributed, the counter-example is not one, and the permissive rule is worse everywhere

The seat says a chart's own clip **replaces** the block clip rather than intersecting it, cites
`MetaClipRegionAction::Execute` against `MetaISectRectClipRegionAction::Execute`, and says it is
blocked because nothing says *which* charts escape — `044_Cash_flow_forecast` being clipped at the
block and `012_Contextures` not.

### 3.1 `044` is not a counter-example: it has no over-clipping at all

`probes/clip-seats-r97/overreach.tsv`, the census that round produced, gives `044` **`over_px = 0`
on every one of its five pages**. There is no ink on `044` that our block clip removes and
26.2.4.2 keeps. The "6.55 of `|ink|%`" was measured by forcing `SheetPageGraphics.cs:153`'s
`cut` to false, which suppresses **every** block clip on the page and lets our own excess spill —
it measures what the block clip is worth as a brake on our own errors, not what the reference
does. So the seat's stated discriminating pair is not a pair.

### 3.2 The permissive rule, run and measured — worse on all four, `012` and `013` included

`SheetPageGraphics` clips a drawing that leaves the block to the block. The rule the seat implies
is that a chart is clipped to its own frame instead. That was implemented behind a constant and
both halves rendered:

| document | diff% block | diff% escape | `\|ink\|%` block | `\|ink\|%` escape |
|---|---:|---:|---:|---:|
| `DynamicBubbleChart` | 9.72 | 15.62 | 4.55 | **9.05** |
| `044_Cash_flow_forecast` | 5.96 | 17.59 | 1.84 | **11.10** |
| `012_Contextures_chart_sample` | 2.40 | 3.04 | 0.47 | **0.78** |
| `013_Contextures_chart_sample` | 2.96 | 3.55 | 0.51 | **0.80** |

**Every document is worse on both metrics — including the two the seat is filed for.** The change
has been backed out; nothing of §3 is in the tree.

The reason is visible in `013`. Our own clip rectangles nest (`clipops.py` reports the depth), and
in the reference's stream the chart's rectangles are nested too — a frame, a wall, a plot — with
only the outermost exceeding the block. Giving the whole drawing its anchor box lets everything
inside spill, where the reference is still bounded by the rectangle in force at that moment.

### 3.3 The mechanism is `MetaPopAction`, not `MetaClipRegionAction`

`MtfTools::UpdateClipRegion` (`emfio/source/reader/mtftools.cxx`:1254-1289, this tree) emits, for
*every* clip change a Windows metafile makes:

```
mpGDIMetaFile->AddAction( new MetaPopAction() );                    // taking the original clipregion
mpGDIMetaFile->AddAction( new MetaPushAction( vcl::PushFlags::CLIPREGION ) );
…
mpGDIMetaFile->AddAction( new MetaISectRectClipRegionAction( … ) );   // :1285
```

The clip that follows is an **intersect**, not a replace. What discards the block clip is the
**`MetaPopAction` in front of it**, which unwinds the device's clip state to whatever was saved
before the replay — and any ink drawn between that pop and the next intersect is bounded only by
the page. That is what produces the `Q q … re W* n` the seat read as a replace, and it is a
different action in a different module from the one the seat cites.
`EMR_EXTSELECTCLIPRGN` with `RGN_COPY` reaches it through `WinMtfClipPath::setClipPath`'s
`RGN_COPY` arm (`mtftools.cxx`:135-137), which assigns rather than intersects.

**This is read in a tree that is not the reference binary's source and is not confirmed against
26.2.** Its rendered leg — §3.4 — is.

### 3.4 What actually escapes, and why a chart predicate is the wrong shape

`012` and `013` each hold **one `xdr:pic` and one or two `xdr:graphicFrame` charts**, on different
sheets. Pairing `overreach.tsv`'s pages against which pages the experimental chart-predicate
moved:

| document | pages where the reference escapes the block | pages the chart predicate moved |
|---|---|---|
| `012_Contextures` | 1, 2 (1,470 and 7,221 px) | 3, 4 |
| `013_Contextures` | 1, 2 (953 and 7,625 px) | 3, 4 |

**They are disjoint.** The object that escapes on `012`/`013` is the sheet's **EMF picture**, and
the `graphicFrame` chart — the one our `drawing.Chart` predicate matched — is clipped at the block
**by the reference itself**: 26.2.4.2's own stream on `013` page 3 carries the block rectangle
`(54.000, 419.046)-(513.638, 720.000)` and no clip of the object's own, while our escape rule
replaced it with `(123.024, 432.170)-(548.816, 634.082)` and spilled 35 pt past the block edge.

The picture is an EMF, and it carries **25 `EMR_EXTSELECTCLIPRGN` records** (`xl/media/image1.emf`,
35,412 bytes) — one per clip change, each of which `UpdateClipRegion` turns into a pop.

`DynamicBubbleChart` holds **no picture at all** — one `graphicFrame` and two `xdr:sp` — and its
escaping ink (2,573 px at x 538.8…579.0 on page 1, 6,069 px at x 0…49.8 on page 2) is in the
column the two shapes occupy, which we cut at the block's 538.725. So "an EMF picture escapes" is
not the whole predicate either.

### 3.5 State

**Still seated, and the distinguishing question is now a different one.** The seat is no longer
*"which charts escape"* — no chart in the corpus demonstrably escapes; the objects that do are an
EMF picture on `012`/`013` and two shapes on `DynamicBubbleChart`. The question is:

> **What does a drawing object have to emit for `MtfTools::UpdateClipRegion`'s pop to reach the
> block clip?** Concretely: does the block clip sit on the device's push/pop stack at all when
> `SdrPageView::DrawLayer` replays an object's metafile, and which object kinds get a push of
> their own in front of it?

Reach is unchanged and small: **61 pages, 31,384 px, 2.319 summed page-`%`**, of which 1.92 is
`012_Contextures` (0.645), `DynamicBubbleChart` (0.642) and `013_Contextures` (0.637)
(`clip-seats-r97` §2.4). Against that, §3.2 shows the permissive direction costs **14.36 more summed
`|ink|%` across those same four documents** — an order of magnitude more than the seat is worth,
which is why it is left seated rather than guessed at.

---

## 4. Files

| file | what |
|---|---|
| `biffchart.py` | the BIFF chart-record dumper: `CHCHART`, `CHPROPERTIES`, `CHAXESSET`, `CHFRAMEPOS` |
| `census-plotpos.py`, `census-plotpos.tsv` | every `.xls` chart in the corpus and the plot area it states — §1.4, §1.6 |
| `census-3d.py`, `census-3d.tsv` | the 3D-chart census over all 947 documents — §1.4, §2.4 |
| `fods-geometry.txt` | 26.2.4.2's own resolved frame, chart, plot area, legend and titles for the logbook — §1.3 |
| `clipops.py` | every clip operator on a page, in stream order, in page coordinates, with its nesting depth — §3 |
| `bigdraw.py`, `draws.py`, `spans.py` | the drawn-rectangle and drawn-span readers behind §1.1, §1.7 and §2 |
| `inksplit.py` | the ours-only / reference-only pixel split, by whether a pixel is in a text box — §2.2 |
| `score-both.py` | summed `diff%` **and** `\|ink\|%` per document; the two-column ranking §1.7 needs |
| `ink-movers.tsv` | the per-document table of §1.6 |
| `o26-escape-experiment.tsv` | the four documents of §3.2, both ways |
| `pie3d-scores.tsv` | the three 3D pies of §2.4 |
| `sweep-before-hashes.tsv`, `sweep-after-hashes.tsv` | the `sheets/` sweeps either side of the O27 arm |
| `movers.txt` | the seven identities the sweep found |

`sweep-ours.py`, `movers.py` and `clip-rects.py` were reused from `probes/chart-fit-r97` and
`probes/clip-seats-r97` rather than copied.
