# r108 — four chart-geometry seats, and the one routine three of them are about

Round 108 was handed **O41** (a gauge chart's plot rectangle), **O42** (an of-pie's unit radius),
**O43** (the plain width/collision rotation limit) and **O31** (a 3-D pie), with an explicit
instruction to test first whether they are one cause.

**They are one *routine* and four different arithmetics, and the round says so with measurements
rather than with a reading.** One of them is now solved completely and is not implemented; one has
half of it implemented and measured; one has its search space cut by an order of magnitude and
stays open; one is unchanged.

| seat | state at the end of this round |
|---|---|
| **O41** | **half fixed, seat stays open.** The `CHTICK` record and the `CHAXISLINE`/`CHLINEFORMAT` pairing are read; the gauge's worst gridline goes **7.20 → 2.95 pt** and all 18 change from black to the reference's `#808080`. `EHEST` summed \|ink\|% **8.06 → 5.56**. The residual plot-rectangle band is named (§1.4) and not closed. |
| **O42** | **mechanism solved, twice confirmed, deliberately not implemented.** §3. |
| **O43** | **open, and much smaller.** Three of the seat's own candidate constants are refuted by measurement and the boundary is bracketed. §4. |
| **O31** | **unchanged.** §5. |

---

## 0. The common cause: confirmed as a routine, refuted as one arithmetic

The brief's hypothesis was *"a common cause in how a plot is placed and sized inside a frame both
renderers agree on"*. It is right, and it is more general than the brief supposed:

**26.2.4.2 does not subtract computed bands from a plot rectangle at all.** It lays the diagram out
at `VDiagram::reduceToMinimumSize` — one 2.2th of the available rectangle, `VDiagram.cxx`:635-651 —
draws everything, takes `ShapeFactory::getRectangleOfShape` of the whole diagram group, and grows or
shrinks the inner rectangle by exactly what that over- or under-ran, with a floor of a third of the
available rectangle (`VDiagram::adjustInnerSize`, `VDiagram.cxx`:653-699). `impl_createDiagramAndContent`
calls it for an axed chart at `ChartView.cxx`:586-635 and for a pie or doughnut at `:682-690`
(this tree; not checked against 26.2.4.2's own source, which is not in this checkout).

This tree models that routine in exactly one place — `ChartLayout.PieLabels`' `ReducedToMinimum` /
`PieConsumedRect` / `AdjustInnerSize` — and gates it on `HasBestFitLabels`. Everywhere else it
computes bands. **That is the shared cause of O41 and O42.**

It is not one fix, and the two seats do not share an arithmetic:

* O41's chart is an axed BIFF **area** chart whose available rectangle is the file's own stated
  plot area and whose consumed rectangle is its axis labels'.
* O42's is a **pie**, whose available rectangle is `DiagramAreaOf` and whose consumed rectangle is
  the of-pie composition plus its bar labels — and whose decisive term is a LibreOffice defect that
  has nothing to do with axes (§3.3).
* O43 is a third routine entirely, `VCartesianAxis::createTextShapes`' rotate-or-thin ladder, and
  O31 a fourth.

### 0.1 One thing the brief says is diagnostic, and is not

> *A residual linear in index through the middle is a scale error about a centre, not an offset.
> That is the single most diagnostic fact in these four seats.*

It carries no information beyond the two edges of the rectangle. If our gridline *i* of *n* sits at
`y0 + f·h` with `f = i/(n−1)` and the reference's at `y0' + f·h'`, the residual is
`(y0 − y0') − f·(h' − h)` — **linear in the index by construction, and zero at
`f = Δy0/Δh`**. On `EHEST` page 15, Δy0 = 6.66 and Δh = 9.26, so the zero is at f = 0.719 and the
step is 9.26/7 = 1.323 pt, which is r107's measured 1.32 exactly. Any two rectangles of different
height produce it. The brief's "scale about a centre" is one parameterisation of "two edges
disagree" and not evidence for one.

---

## 1. O41 — what was read out of the file, and what it bought

### 1.1 The two things the reader never looked at

`XlsChartBuilder` read neither the `CHTICK` record (0x101E) nor the `CHLINEFORMAT` that follows a
`CHAXISLINE`. So **every BIFF axis took `ChartPlot`'s OOXML default of an outward tick** — a 4.25 pt
mark drawn at every gridline *and* a 4.25 pt band taken off the plot rectangle for it — and drew its
axis line and gridlines black.

Both are wrong on this document and both are confirmed twice.

**Source.** `XclImpChTick::ReadChTick` reads the major tick type as the record's first byte
(`sc/source/filter/excel/xichart.cxx`:3194-3218) and `lclGetApiTickmarks` (`:3166-3172`) reads it as
two flags, `EXC_CHTICK_INSIDE` 0x01 and `EXC_CHTICK_OUTSIDE` 0x02 — so 0 is no tick and 3 is a
crossed one. `XclChTick`'s own constructor defaults `mnMajor` to both
(`sc/source/filter/excel/xlchart.cxx`:304-313) and `XclImpChAxis::Finalize` makes a default object
for an axis that read no record (`xichart.cxx`:3303-3304), so **BIFF's default is crossed, not
outward**. An automatic `CHLINEFORMAT` falls back to
`GetPalette().GetColor(EXC_COLOR_CHWINDOWTEXT)`, which `xlstyle.cxx`:150 answers `COL_BLACK` for
(`XclImpChLineFormat::Convert`, `xichart.cxx`:466-484).

**The reference's own resolved view**, which needs no rendering:
`soffice --convert-to fods` on `EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls` takes eight
seconds and prints, for **all eighteen** of that workbook's chart axes,

```
chart:tick-marks-major-inner="false" chart:tick-marks-major-outer="false"
```

and gives the chart frame and the axis/gridline styles `svg:stroke-color="#808080"`. Its rendering
of page 15 draws no tick mark anywhere and strokes all eight gauge gridlines `#808080`.

### 1.2 What it moved on the witness

`gauge-rules.py` from `sheet-draw-r107`, re-run over pages 13 and 15 (`gauge-after.tsv`):

| population | count | worst \|dy\| before | after | our colour before | after | reference's |
|---|---:|---:|---:|---|---|---|
| the sheet's own rules | 32 | 0.02 | **0.02** | `#000000` | `#000000` | `#000000` |
| the gauge's gridlines | 18 | 7.20 | **2.95** | `#000000` | **`#808080`** | `#808080` |

The plot rectangle on page 15 goes from **404.75 × 150.78** to **409.00 × 155.02** against the
reference's 413.75 × 160.04 — the width error 9.00 → 4.75 pt and the height error 9.26 → 5.02 —
and the 18 spurious 4.25 pt outward ticks per gauge are gone.

### 1.3 Reach, per document, over every BIFF workbook in the corpus

`xls-confine.py` renders all **64** `.xls`/`.xlt` in the corpus at the round's base and again after,
under `SOURCE_DATE_EPOCH=0`, one output directory per document, compares byte for byte, and scores
each mover against its banked 26.2.4.2 rendering with `pdf-image-diff.py`, summing the `|ink|%`
column over the document's pages. **6 move and 58 are byte-identical.**

| document | reference pages | base | after | Δ |
|---|---:|---:|---:|---:|
| `EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls` | 24 | 8.06 | **5.56** | **−2.50** |
| `TOGAF9-Tool-ConfReqts-CSQ.xls` | 28 | 12.93 | **12.24** | −0.69 |
| `orbus_togaf_tool_csq.xls` | 75 | 45.72 | **45.62** | −0.10 |
| `014_Contextures_chart_sample_991ecfc5.xls` | 3 | 0.50 | 0.50 | ±0.00 |
| `2012-GA-Survey-Chapter-6-Tables-16Dec2013-V2.xls` | 5 | 0.25 | 0.25 | ±0.00 |
| `2012-GA-Survey-Chapter-5-Tables-16Dec2013-V2.xls` | 3 | 0.30 | **0.37** | **+0.07** |

Sum **67.76 → 64.54**. **One document is worse and it is worse while its geometry improves**, which
is why the per-document table is here: on `2012-GA-Survey-Chapter-5` **fourteen strokes and six
labels change and nothing else does** — the whole diff is on page 1 of 3 — and what they are is the
plot rectangle's left edge and the gridlines and tick labels hanging off it. It goes
**114.21 → 109.96** against 26.2.4.2's **110.49**: a 3.72 pt error becomes 0.53. At 512 px on a 842 pt page
one point is 0.6 px, so a 3.2 pt improvement can and here does cost 0.07 of a percentage point of
ink. Both figures are reported; neither is the whole story on its own.

The reach is bounded by code as well as by measurement: `XlsChartBuilder` is reached only by
`XlsWorkbookReader`, so no `.xlsx`, `.ods`, `.docx`, `.pptx` or `.ppt` can see the change, and the
sweep confirms it on the 58 BIFF workbooks that do not move.

**No gate verdict can move**: a tick, a band of 4.25 pt and a stroke colour add no alphanumeric
character and no page.

### 1.4 What is left on O41, and what it is

The residual bands, in the chart's own page coordinates (from 26.2.4.2's own `fods`, which prints
`<chart:plot-area>` — the rectangle the diagram was given — and `<chart:coordinate-region>`, the
inner rectangle it settled on):

| | left | right | top | bottom |
|---|---:|---:|---:|---:|
| 26.2.4.2, chart-page pt | 12.76 | 4.99 | 5.64 | 14.09 |
| ours, same units | 21.42 | 7.08 | 6.35 | 23.60 |

The reference's `chart:coordinate-region` for the page-15 gauge is
`svg:x="0.771cm" svg:y="1.453cm" svg:width="22.457cm" svg:height="8.688cm"` on a
`26.278 × 10.638 cm` chart page, which maps onto the sheet at **65.21, 264.61, 413.73 × 159.78** —
reproducing the rendering's own 65.19 / 264.85 / 413.75 / 160.04 to a quarter of a point. So the
whole question is answerable without a rasteriser, and that is the instrument the next round should
start from.

**And the mechanism is §0's.** A BIFF chart that is not a pie and not 3-D goes through
`xPositioning->setDiagramPositionIncludingAxes(aDiagramRect)` (`xichart.cxx`:4038-4046), which sets
`PosSizeExcludeAxes` **false** (`DiagramWrapper.cxx`:843-853), so
`getAvailablePosAndSizeForDiagram` leaves `mbUseFixedInnerSize` false
(`ChartView.cxx`:946-981) and the stated rectangle becomes the *available* one. The inner rectangle
is then `adjustInnerSize` of the drawn extent, not a band subtraction — which is why the
reference's value-axis labels begin at x = 56.93 on page 15, **left of the stated plot area's own
56.91 edge**, something no band model can produce.

---

## 2. The instrument this round wants recorded

Every quantitative claim about the reference below came from **`--convert-to fods` or `--convert-to
fodt`** rather than from a rendering:

* the eighteen `chart:tick-marks-major-*` and the `#808080` (§1.1);
* the gauge's exact inner rectangle (§1.4);
* the of-pie's available and inner rectangles at every point of a ladder (§3.4), where reading the
  bar out of a PDF had already produced two wrong tables.

It costs two seconds on a 21 cm chart and has no tolerance in it anywhere.

---

## 3. O42 — the of-pie, solved

### 3.1 What is wrong, in one sentence

`028_Unit_Circle_Chart_Optimized_Graph` states `<c:dLblPos val="inEnd"/>`, so
`ChartLayout.HasBestFitLabels` is false, so **the pie's second pass never runs** — and 26.2.4.2 runs
it for every pie and doughnut, `if( bIsPieOrDonut )` at `ChartView.cxx`:682, with no test on the
label placement at all (`lcl_IsPieOrDonut`, `:339-346`).

### 3.2 That the reference shrinks a non-best-fit pie is measured, not read

`ofpie-fit.py` writes one-element variants of `028` and reads R off the of-pie's bar, which is
exactly `0.5R × 1.0R` (`PieChart::getBarRect`, `PieChart.cxx`:381-397, and `createOneBar`'s
`fBarTop = -0.5`, `:1415-1416`). `ofpie-fit.tsv`:

| variant | R |
|---|---:|
| as it stands | **135.40** |
| data labels off | **184.90** |
| labels at `outEnd` | 135.40 |
| labels at `ctr` | 135.40 |
| one point labelled instead of sixteen | 184.90 |

**Turning the labels off is worth 1.366× on the radius, and moving them is worth nothing.** No
best-fit label is involved anywhere in that table.

### 3.3 What consumes the space, and it is a LibreOffice defect

`createBarLabelShape` computes the wrap width for a bar-of-pie bar's label as

```cpp
const double fTextMaximumFrameWidth = 0.8 * (m_fBarRight - m_fBarLeft);   //  = 0.8 * 0.5 = 0.4
const sal_Int32 nTextMaximumFrameWidth = ceil(fTextMaximumFrameWidth);    //  = 1
```

(`chart2/source/view/charttypes/PieChart.cxx`:780-782, this tree). `m_fBarRight − m_fBarLeft` is in
**unit-circle radii** and is never transformed to screen, so every bar label is wrapped into a frame
**one hundredth of a millimetre wide** — one character per line.

**26.2.4.2 does exactly that, in its own rendering of the unmodified corpus document.** On `028`'s
page 1 the two split-off points' labels are drawn as single characters at x ≈ 437.8, 10.47 pt apart
down the page, spelling `B r a n c h   3   S t e m   6   L e a f   1 5   4 %` — 26 lines for a
23-character caption. That column is 270 pt tall on a 370 pt diagram, and it is what
`adjustInnerSize` is charged for.

Three independent measurements agree with it and with nothing else:

* **The shrink follows the character count and not the width.** `ofpie-region.tsv`: categories of
  `W×10`, `i×10` and `M×10` — advances 105, 26 and 122 pt — give **the identical inner rectangle,
  271.73 pt square**, and `i×30` reaches the floor.
* **The floor is the available *height* over three.** The ladder bottoms out at exactly
  **144.00 = 432.00/3**, not at 575.43/3 = 191.81, so the height binds and `adjustInnerSize`'s
  `aAvailableOuterRect.getHeight()/3` (`VDiagram.cxx`:667-670) is the constant reached.
* **The slope is three lines per category character.** Over the linear part of the ladder the inner
  square loses **31.5 pt per character**, against three category levels joined into one label ×
  10.47 pt of line height = 31.4.

### 3.4 The available rectangle is ours already

26.2.4.2's own `fodt` of `028` gives `<chart:plot-area svg:x="0.35cm" svg:y="0.35cm"
svg:width="20.3cm" svg:height="13.071cm">` = **9.92, 9.92, 575.43 × 370.53 pt**, against this
tree's `DiagramAreaOf` of **9.92, 9.92, 575.46 × 370.68**. Fifteen hundredths of a point. So
nothing before the second pass is at fault, and `<chart:coordinate-region>` gives the answer the
pass must produce: **169.00, 42.18, 271.29 × 271.29**, against the 370.68 square this tree draws.

### 3.5 Why it is not implemented

Reproducing it means (a) running the pie second pass for every pie and doughnut rather than for a
best-fit one, (b) putting the of-pie's own drawn composition into `PieConsumedRect` — it runs from
`m_fLeftShift − m_fLeftScale = −1.4167` to `m_fBarRight = 1.25` across and ±2/3 down, so it
overflows the diagram square by design — and (c) drawing a bar-of-pie's bar labels one character per
line. (a) reaches every pie in the corpus and a plain pie's ratio is 1.017, so the change would have
to be measured over all of them; (c) changes the ink on the one document that can score it.

**The corpus reach is 1 of 947 plus its `.odt` twin** (r104 §4.1), so the work is a day and the
scoreboard movement is one document. Everything an implementation needs is above, and the two
probes that measure it are in this directory. **The seat stays open.**

---

## 4. O43 — three candidate constants refuted, and the boundary bracketed

### 4.1 The instrument, which the seat did not have

`width-ladder.py` writes `038_Competitive_Advantage_Card`'s five category labels as a run of one
repeated letter, so the label's width is exactly *n* glyph advances and **nothing about it but its
width can matter** — no hyphenation point exists in a run of one letter, which removes r106's
confound. Three letters of very different advance (`i` 2.56 pt, `n` 5.92, `W` 10.00 at this chart's
11 pt) × 17 lengths × two label shapes = **102 renderings on each side**.
`width-ladder-twoword.tsv` (labels `Cost <run>`) and `width-ladder-oneword.tsv` (the run alone):

| | reference turns at | this tree turns at |
|---|---|---|
| `Cost i×n` | **12** | 13 |
| `Cost n×n` | **8** | 9 |
| `Cost W×n` | **5** | 6 |
| `i×n` alone | never, to 18 | never, to 18 |
| `n×n` alone | **9** | 10 |
| `W×n` alone | **6** | 6 |

**One character, in the same direction, on four of the six ladders.**

### 4.2 What is *not* the seat, each refuted by a measurement

* **Not the wrap limit.** The reference's own flip from one line to two brackets its
  `TextMaximumFrameWidth` at **[50.74, 51.97] pt** on this chart's 54.20 pt pitch, and this tree's
  own trace prints `spacing=54.202 limit=51.491` — `0.95 × spacing`, `VCartesianAxis.cxx`:753-759.
  `WrapFraction` and `Spacing` are right to a tenth of a point.
* **Not the metric.** Our measured word widths agree with the reference's drawn ink to **0.05 pt**
  at six lengths of three letters (35.302/35.257, 41.207/41.174, 263.935/263.829 for a whole row),
  and an independent computation from the installed Carlito through `chart2`'s 96 dpi pixel-em
  rounding agrees with ours to **0.01 pt** on every letter run (`fine-width.py`).
* **Not `ShapeFactory`'s 0.18/0.30 em text insets.** They are set in the `createText` overload at
  `ShapeFactory.cxx`:2168-2300 (`#i109336#`, `:2279-2297`) and `createSingleLabel` uses the one at
  `:2042`, which sets no text distance at all. `ChartAxisLabels.Shape`'s existing remark already
  says so and has a corpus measurement behind it, and this round did not overturn it.

### 4.3 What the seat *is*, bracketed

Once the wrap restart has fired, a label is a single line and `doesOverlap`
(`VCartesianAxis.cxx`:186-207) intersects the two label **shapes**. This tree's box is the text
exactly — traced `box0=53.189` for `n×9` against a 54.202 pt pitch, so no overlap and no rotation.

`fine-width.py` samples the 2.7 pt window between the wrap limit and the pitch, where the two
candidate rules disagree: fifteen single words from 51.702 to 54.267 pt.
**All fifteen are turned by the reference and one of the fifteen by this tree** (`fine-width.tsv`)
— the one, `nnnnnnnnis` at 54.267, being the only one wider than the pitch. The narrowest is
0.211 pt above the wrap limit and 2.50 pt below the pitch.

So at 11 pt the reference's collision box is **at least 2.50 pt wider than the text**, or
equivalently its collision threshold is the *wrap limit* rather than the tick pitch. That is
enough to fix every single-word row, and it is not enough to close the seat, because:

* the **two-word** rows put the boundary somewhere else entirely. Of r106's twenty-two authored
  words, `Screeched` (47.06 pt) and `Squelched` (47.34) turn and `Scratched` (44.70),
  `Stretched` (44.05), `Strengths` (43.42), `Thoughts` (42.50), `Splashed` (40.86),
  `Straights` (39.86) and `Strength` (39.02) do not — a boundary in **(44.70, 47.06]**, which is
  6 pt *below* the two-word ladder's own and 4 pt below the wrap limit. **No single width threshold
  fits both label shapes.**
* two arms of `lcl_hasWordBreak`'s consequence are entangled in the two-word case that are not in
  the one-word case: the hyphenating fill (r106's `Hyphenates`, which fires on `Cost i×12` in this
  tree and not on `Cost n×8`) and whatever makes the reference restart at 47 pt.

**Nothing was changed for O43.** Its seat is now a two-line question with 117 banked renderings
under it instead of three corpus documents and a hunch.

---

## 5. O31 — unchanged, and why this round did not take it

r102 §2 measured the population and it is not disputed here: **five of the six corpus 3-D charts are
a raster in 26.2.4.2** — the three OOXML pies and both BIFF 3-D bars — and the sixth, a `.ppt` OLE
3-D pie on `undp_presentation_revised_17_may.ppt`, is vector. So an implementation of 3-D geometry
can be scored against **1 of 947 documents**, and 86.5 % of the seat is a projection, an extrusion
and a paint order for that one document.

This round spent its budget on the three seats where a measurement could change what is believed,
and O31's beliefs were already measured. It stays exactly where r102 left it: the projection the
`b/a = 0.4934` against `rotX 30°` agreement supports is implementable, the 46.5 pt of extrusion with
it, and neither can be scored anywhere else in the corpus.

---

## 6. Deliverable state

Built clean, `TreatWarningsAsErrors` on, **0 warnings 0 errors**.

The ten non-fidelity projects, one at a time, totalled from this run's own output
(`tests-nonfid.log` in the round's work directory):

```
Containers 109   Core 560   Markup 259   OpenDocument 160   Presentations 1101
Rendering  164   Spreadsheets 1325   Text 728   Vector 309   WordProcessing 1938
                                                   6653 passed, 0 failed, 0 skipped
```

**New tests**, 10 in all — `XRay.Office.Spreadsheets.Tests/XlsChartAxisLineTests.cs`. Seven cases and a
four-case theory: each `CHTICK` major value read as its own pair of flags, each axis taking its own
record, an axis with no record being crossed (BIFF's default, not OOXML's outward), the
`CHLINEFORMAT` after a `CHAXISLINE` colouring the line that record named, an automatic format
leaving it black, and a `CHAXISLINE` with no format not claiming the next series' line. All ten
fail at the round's base, checked by building the round's own base reader against them: **7 of the
10 fail and 3 pass**, and the three that pass are the ones the base is right about by accident —
the theory's `major: 2 → Outer` case, which is the old default; the automatic format, which is black
either way; and the guard that a `CHAXISLINE` with no format must not claim the next series' line.

## 7. Files here

* `results.md`.
* `xls-confine.py` → the §1.3 sweep, run over all 64 BIFF workbooks.
* `ofpie-fit.py` / `ofpie-fit.tsv` (14 rows) — §3.2.
* `ofpie-ladder.py` — the first cut of §3.3, kept because it is the instrument that reads R out of
  a PDF and it was superseded by the next one.
* `ofpie-region.py` / `ofpie-region.tsv` (18 rows) — §3.3 and §3.4, read out of 26.2.4.2's `fodt`.
* `width-ladder.py` / `width-ladder-twoword.tsv` (52 rows) / `width-ladder-oneword.tsv` (52) — §4.1.
* `fine-width.py` / `fine-width.tsv` (16 rows) — §4.3.

## 8. What this round could not settle

* **O41's plot-rectangle bands.** §1.4 names the mechanism and gives the reference's exact numbers;
  no band model can reach them, because the reference's own labels overflow the rectangle they are
  measured against.
* **O42 is solved and not implemented**, §3.5. The seat is open on work, not on knowledge.
* **O43's constant.** §4.3 — the one-word and two-word boundaries do not admit a single threshold
  and this round did not find the second rule.
* **Whether the `2012-GA-Survey-Chapter-5` regression is real.** Its geometry improves by 3.19 pt
  and its summed ink worsens by 0.07 at a raster resolution of 0.6 px per point. Both numbers are
  in §1.3 and neither is qualified away.
* **The BIFF tick default was changed from `Outer` to `Cross`** on the strength of
  `xlchart.cxx`:304-313 and `xichart.cxx`:3303-3304. No corpus document separates the two — every
  BIFF axis that draws a tick in this corpus states a `CHTICK` — so that half is a source reading
  with no rendered witness, and it is called out here rather than folded into the measured half.
