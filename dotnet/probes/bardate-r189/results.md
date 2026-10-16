# A bar on a date axis was laid out by its index — r189

Measured 2026-09-28 against **LibreOffice 26.2.4.2** (`/opt/libreoffice26.2/program/soffice`,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`), `SOURCE_DATE_EPOCH=0`. The C++ read is this
checkout, which is 27.2.0.0.alpha0+ and not the reference binary's source; every rule below is
confirmed a second time against 26.2.4.2's own rendering.

## 0. How it was found

`171128IPAP.pptx` sat at 16.01 `diff%` — fourth in `probes/chartsweep-r186`'s ranking — and
`dotnet/CLAUDE.md` attributed it to the `c:smooth` spline. **That attribution was eleven rounds
stale**: round 102 implemented the spline and the note was never corrected. Its worst page went
to a blind reader instead, who reported, first of nine differences, that *"ours draws grey only
from x≈620 to x≈965 (2009 → ≈2016), leaving the last ≈195 px empty of columns; the reference
draws columns all the way"*, and asked for a bar-edge count to separate truncation from
compression.

The count settles it. On page 40, out of the PDFs' own operators:

| | bars | x span | width | pitch |
|---|---:|---|---:|---|
| ours, before | **67** | 348.96 … 526.43 | 2.02 | 8.07 × 8, then 2.02 × 59 |
| 26.2.4.2 | **44** | 350.05 … 637.97 | 2.21 | a flat 6.63 |

A pitch that changes partway along is not a compression and not a truncation: it is a layout
that does not know what the x coordinate means. `ChartLayout.AddBars` placed every bar at
`(double)at / categories`. A line series has gone through the date scale here since it was
written — which is why the same chart's red line spanned the plot correctly — and a bar series
had no such path.

## 1. The rules, and which of them had to be measured

`BarChart::createShapes` (`chart2/source/view/charttypes/BarChart.cxx`:694-705, this tree) takes
the point's **own x**, rasterises it to the axis' resolution, drops it outright when it falls
outside the range — three consecutive `continue`s before any geometry — and hands the rest to
`BarPositionHelper::getScaledSlotPos`. Four things follow, and only the first two are readable
from the source alone.

**A category is one unit of the axis' time resolution wide.**
`PlottingPositionHelper::setTimeResolution` sets the category width to 1, and to 12 at year
resolution because the axis' own scaling counts months
(`chart2/source/view/main/PlottingPositionHelper.cxx`:670-690). Not one n-th of the points.

**The axis is linear in that unit, not in days.** `DateScaling::doScaling`
(`chart2/source/view/axes/DateScaling.cxx`:56-91) returns the serial itself only at day
resolution; at month and year it returns `year × 12 + month` plus the fraction of the month
elapsed. Over `171128IPAP`'s 132 months the difference is under a tenth of a percent and
invisible; over `044_Cash_flow_forecast`'s eleven it is 5%, and the reference spaces that
chart's twelve monthly bars at a flat **35.26 pt** where days give 30.2 to 33.4.

**A shifted axis carries one extra interval**, and **its categories lead their own dates.** The
first half is in the source — `"for explicit scales we need one interval more (maximum
excluded)"`, `ScaleAutomatism.cxx`:565-597, guarded by `m_bShiftedCategoryPosition`. The second
half is not: `CategoryPositionHelper::getScaledSlotPos` subtracts half a category width
unconditionally, and something adds it back. `AllowShiftXAxisPos` and `isStrongLowerRequested`
are the territory; **the exact line is not pinned here** and the behaviour was solved instead.

## 2. Solving it: two stated ranges, one plot edge

One rendering cannot separate "centred on the date" from "running from the date", because the
plot's left edge is unknown and both readings fit at different edges. `variants.py` writes two
copies of `044_Cash_flow_forecast` differing only in the range its date axis states, and
`geometry.py` reads the plot rule, the ticks and the bars out of each:

| variant | stated range | plot rule | ticks | bars |
|---|---|---|---|---|
| `exact` | 44958 … 45292 (11 months) | **414.31 … 837.47** | n=13, first 414.31, pitch 35.24 | n=12, first 420.15, width 23.53 |
| `lo` | 44927 … 45323 (13 months) | **414.31 … 837.47** | n=15, first 414.31, pitch 30.19 | n=12, first 449.55, width 20.15 |

Four readings fall straight out of it.

* **The plot's left edge is stated, not inferred** — the widest horizontal rule is the plot's
  own, 414.31 in both. There was nothing to solve for after all; the first cut of this probe
  inferred it from the bars and got "centred", which the tick row then refuted.
* **The maximum is extended by one interval even when the range is stated.** 11 months of range
  gives **12** intervals and 13 gives **14**.
* **The category leads its date.** `exact`'s first point is the axis minimum, so its bar should
  start at `414.31 + (35.24 − 23.53)/2 = 420.17` if the category runs from the tick and at
  402.6 if it straddles it. Measured **420.15**.
* **And again one category in.** `lo`'s first point is one month past the minimum: predicted
  `444.50 + (30.19 − 20.15)/2 = 449.52`, measured **449.55**.

`171128IPAP`'s page-40 chart states `c:crossBetween="midCat"` instead, and there the reference
*does* straddle and does not extend — which is the control that keeps the two arms apart. Two
more variants of the same file pin both arms directly, rendered the same way:

| variant | ticks | bars |
|---|---|---|
| `c:crossBetween` **deleted** | n=13, pitch 35.23 | n=12, first 15.16, width 23.53 |
| as authored, `between` | n=13, pitch 35.23 | n=12, first 15.16, width 23.53 |
| rewritten to `midCat` | n=12, pitch 38.46 | n=12, first 9.33 (clipped), width 12.81 |

So **an absent `c:crossBetween` renders identically to `between`** — `AxisConverter`'s fallback,
which is what lets this tree apply it without a second measurement for the line case.

> **One thing in the `midCat` row is not explained and is recorded rather than modelled.** Its
> bar is 12.81 pt in a 38.46 pt category, a third, where the `between` rendering gives 23.53 in
> 35.26, two thirds — and the file's `c:gapWidth` is the same in both. The corpus's one
> `midCat` bar-on-date chart is `171128IPAP`'s page 40, whose bar this tree draws at 2.22 pt
> against the reference's 2.21, so the question does not arise on any real document here.

## 3. Where the flag comes from

`AxisConverter::convertFromModel` reads `ShiftedCategoryPosition` off the **crossing** axis — the
value axis — as `mnCrossBetween == XML_between`, with type overrides for a 3-D bar and a radar
above it and a fallback of true for a bar, line or stock group
(`oox/source/drawingml/chart/axisconverter.cxx`:292-301). Censused over the five corpus
documents with a bar group on a date axis: four state `between` throughout and `171128IPAP`
states `between` on three of its four date-axis charts and `midCat` on the two axes of the
fourth — which is chart9, the page-40 one.

**`ChartDateScale.Resolve`'s own default is `false`, not true.** The true is an OOXML fallback
and belongs to the OOXML reader, which passes the real value; the BIFF reader does not pass one,
so every legacy date chart keeps the geometry it had. `CHVALUERANGE`'s own crossing flag has not
been measured against the reference and guessing it would move seven `.xls` on a hunch.

## 4. Reach and cost

`census.py` finds **7 chart parts in 5 documents** with a bar group on a date axis. Rendering
our half of all 168 chart-bearing corpus documents at the round's base and again with the change
moves **8 and leaves 160 byte-identical** — the five, plus three whose *line* series and ticks
moved because the axis is now month-linear and one interval longer:

| document | worst `diff%` before → after |
|---|---|
| `171128IPAP` | **16.01 → 12.91** |
| `001_Contextures_chart_sample_b089bc34` | **2.57 → 2.12** |
| `044_Cash_flow_forecast` | 1.94 → 1.99 |
| `006_Contextures_chart_sample_afa23b53` | 2.69 → 2.69 |
| `southern-classic-kennesaw-state-university-final` | 7.64 → 7.64 |
| `055_Project_timeline_with_milestones` | 3.66 → 3.66 |
| `8_P-Pavese_AIRBUS-ATB-journee-CRATB` | 9.83 → 9.83 |
| `062_Run_chart_cb7476ea` | 6.43 → 6.43 |

**Page 40 itself goes 16.01 → 7.94 `diff%`, 1.91 → 0.39 `|ink|%`, and MAJOR → shifted**; the
document's headline figure only falls to 12.91 because page 24 becomes its worst. Its bars now
read **44, 350.07 … 635.85, width 2.22, pitch 6.64** against the reference's **44, 350.05 …
635.75, width 2.21, pitch 6.63**.

The one row that worsens does so by 0.05 on a chart whose bars were already within 0.13 pt of
the reference's before and after, which is the same pre-existing offset measured twice.

## Files

| file | what it is |
|---|---|
| `census.py` | corpus chart parts with a bar group on a date axis |
| `variants.py` | two copies of one workbook differing only in the stated axis range |
| `geometry.py` | plot rules, ticks and bars out of a PDF page |
| `score.sh` | scores a before/after pair of ours-only renders against 26.2.4.2 |
