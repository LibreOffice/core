# The automatic interval count is not hardcoded — a print zoom reaches it

Round 87, worktree `wt-chartaxis`, base `3466245b1`. Reference `/opt/libreoffice26.2/program/soffice`,
**26.2.4.2** (`0229ac93fcf0…`). Reference half reused from `/home/user/gate-orig-r83/ref`,
947 PDFs rendered 2026-09-08 against that binary; our half rendered twice, at the base and
with the change, on the same day.

## 0. What the brief said, and what is actually there

The brief (`probes/chart-axis-r86/findings.md`) reads `ChartScale.cs:131-136`'s
`MaximumAutoIntervalCount = 10` as a constant standing in for a rule, and dispatches a round to
"feed the axis's drawn length and measured label height into the interval cap, mirroring
`estimateMaximumAutoMainIncrementCount`".

**That is already there and has been for several rounds.** `ChartLayout.IntervalsThatFit`
(`ChartLayout.cs`:1598-1646) is `nTotalAvailable / nSingleNeeded`, clamped into `[2, 10]`, with
the axis' own rectangle and a measured label; `Compose` runs it as LibreOffice's second pass and
re-resolves both value scales when it comes out under ten (`:1181-1204`). Its own remarks name
`chart-bar-sheet.xlsx`, `chart-bar-sheet.ods` and `chart-bar-deck.odp` as the three documents
that pin the denominator. The constant is the *first pass* — the count an axis is given while
nothing about it has been measured, which is exactly what
`VCartesianAxis::estimateMaximumAutoMainIncrementCount` returns under
`m_nMaximumTextWidthSoFar==0 && m_nMaximumTextHeightSoFar==0`.

So the defect is real, the witness decomposes, and **the seat named in the brief is not the
seat.** What reaches the cap is the worksheet's **print zoom**.

## 1. The witness, reproduced

`sheets/chartset-007/xlsx/040_Blood_pressure_tracker_872b6833.xlsx`, one page, both `c:valAx`
fully automatic (no `c:min`, `c:max` or `c:majorUnit` — checked in `xl/charts/chart11.xml`).
Tick labels read out of the two PDFs with `axisticks.py`:

| axis | 26.2.4.2 | ours at base |
|---|---|---|
| primary value (left) | `160 140 120 100 80 60 40 20 0` | same |
| secondary value (right) | `80 75 70 65 60` (step 5) | `80 78 76 … 62` (step 2) |

*(The brief's table says the primary runs to 140 and the secondary from 60 to 75; it is 160 and
60 to 80. The step is what matters and the step is as briefed.)*

## 2. The chain, and where the ratio goes wrong

Four sites, all read:

- `ScaleAutomatism.cxx`:43-49 — `lcl_getMaximumAutoIncrementCount` is 10, or 500 for a DATE axis.
- `ScaleAutomatism.cxx`:143-151 — `setMaximumAutoMainIncrementCount` clamps into `[2, that]`.
- `VCoordinateSystem.cxx`:412-417 — the argument is
  `pVAxis->estimateMaximumAutoMainIncrementCount()`, after an early `return` for a date X axis.
- `VCartesianAxis.cxx`:1559-1618 — that estimate is `nTotalAvailable / nSingleNeeded`: the axis
  main line's own length (`get2DAxisMainLine`, which for a 2D Y axis runs `fMinY` to
  `getLogicMaxY()`, `:1303-1311` — the whole plot height) over `m_nMaximumTextHeightSoFar`, the
  tallest label shape recorded during `createMaximumLabels`. The tdf#48041 arm lowers it further
  when the number format makes neighbouring ticks read the same, and the horizontal branch swaps
  in width. Neither fires here.

**The step is the linear routine's, not the logarithmic one's.** The brief quotes
`Distance = approxCeil((approxCeil(max) − approxFloor(min)) / n)` incremented by 1.0; that is
`calculateExplicitIncrementAndScaleForLogarithmic` (`:294-540`). A linear axis takes
`calculateExplicitIncrementAndScaleForLinear` (`:738-964`), which normalises `(max − min)/n` to
1, 2 or 5 times a power of ten and climbs that ladder until the count fits. **So our "round
numbers" magnitude logic is not decoration to be justified — it is the reference's own rule**,
and the question the brief asks about it is answered: keep it.

Both of this witness' axes fall out of one cap, and only one value of it produces both:

| cap | primary (0…142) | secondary (63…78) |
|---|---|---|
| 10 | 14.2 → **20** ✓ | 1.5 → **2** ✗ |
| 9 | 15.8 → **20** ✓ | 1.67 → **2** ✗ |
| **8** | 17.75 → **20** ✓ | 1.875 → 2, count 9 > 8, → **5** ✓ |
| 7 | 20.3 → **50** ✗ | 2.14 → **5** ✓ |

`nMaxMainIncrementCount = 8` is the only value that draws the reference's two axes, and the
iteration at `:958-963` is what produces the 5: the first distance is 2, the rhythm expansion
pushes the axis to 62…80, nine intervals is one too many for a cap of 8, and the ladder steps
2 → 5.

## 3. The zoom, which is the actual seat

`IntervalsThatFit` computed **10.16** where the reference computes 8.84, and the numerator was
right. Traced out of our own layout on the witness:

    avail=76.340 pt   needed=7.512 pt   size=7.040 pt   ratio=10.1627

`plot.LabelSize` is 7.04 pt and the chart states `sz="1100"`. The sheet is
`fitToPage="1"`, `<pageSetup scale="80" fitToHeight="0">`, and `SheetChart.Draw` takes the print
zoom through the type sizes before the layout runs — deliberately, so that every glyph run stays
in page coordinates rather than under a transform.

Every other length in the composition is multiplied by that zoom on both sides of a comparison
and cancels. **The interval cap is a ratio of a length to a device-quantised height, and that
does not cancel.** A chart's text is measured on `chart2`'s 96 dpi `VirtualDevice`, so a face is
instantiated at a whole number of pixels and its line height is `round(asc·px) + round(desc·px)`
of them. For DejaVu Sans — which is what both renderers draw these labels in, read out of both
PDFs — that is

| size | 96 dpi ppem | asc + desc px | line height | em |
|---|---|---|---|---|
| 11.00 pt (stated) | 15 | 14 + 4 = 18 | 13.50 pt | **1.2273** |
| 7.04 pt (zoomed) | 9 | 8 + 2 = 10 | 7.50 pt | **1.0653** |

`119.28 / 13.50 = 8.84 → 8`, and `76.34 / 7.512 = 10.16 → 10`. The whole defect is those two
rows of the same table.

### The measurement that identifies it rather than fitting it

Rendering the same workbook with `fitToPage` removed and `scale="100"` — one attribute pair,
nothing else touched — makes **the tree at its base draw the reference's axis exactly**, on both
axes, before any change was made:

    [chartcap] avail=119.281  needed=13.493  size=11.000  ratio=8.8403
    ref   0 20 … 160   ·   60 65 70 75 80
    ours  0 20 … 160   ·   60 65 70 75 80

That is the seat named without a free parameter: the cap model was right and the zoom was
reaching it.

### And the reference's own flip points agree

`vary-h.py` sweeps the witness' `c:layoutTarget="inner"` plot height — the one quantity the file
states, and one `VDiagram::adjustInnerSize` never touches, because all four calls are guarded on
`!mbUseFixedInnerSize` (`ChartView.cxx`:589-619) — and reads the drawn step back. 26.2.4.2 flips
`cap 7 → 8` between axis lengths 70.039 and 71.031 pt and `cap 8 → 9` between 78.542 and 79.038,
which brackets its label height at **8.755 … 8.782 pt of drawn axis**. Predicted from the table
above: `13.50 × (7.12 / 11) = 8.738`. The residual is 0.2-0.5%, which is the resolution of a font
size `pymupdf` reports to two places; the discrete predictions are right at all fourteen sweep
points.

## 4. The change

- `ChartPlot.TypeScale` — the zoom already applied to this plot's type sizes, 1.0 by default.
  `SheetChart.Sized` sets it where it multiplies the sizes; `SlideChart` and `FrameChart` do not
  scale type at all and never set it.
- `ChartLayout.IntervalsThatFit` measures the label at `LabelSize / TypeScale` and multiplies the
  result back, so the ratio is taken in the coordinates `chart2` takes it in.
- `ChartScale.MaximumAutoIntervalCount`'s remarks corrected: ten is a clamp **and the first-pass
  default**, the date-axis ceiling is 500, and the four sites that decide the real count are
  named.

Nothing else reads `TypeScale`.

## 5. Reach and cost

Whole corpus, 947 documents, our half rendered twice — at the base and with the change — against
the **same** banked reference (`gate-orig-r83/ref`, 26.2.4.2, rendered 2026-09-08). Rendering only
our half is sound because the diff is confined to `dotnet/src`, which cannot reach `soffice`
(`sweep-ours.py`; the rule is `dotnet/CLAUDE.md`'s, from `probes/overflow-r69`). No render failed
on either side.

| | match | pages | pages,words | words | unembedded |
|---|---:|---:|---:|---:|---:|
| before (`3466245b1`) | **915** | 7 | 4 | 20 | 1 |
| after | **916** | 7 | 4 | 19 | 1 |

**Exactly one gate verdict moves**, and it is the witness:

    words -> match   sheets/chartset-007/xlsx/040_Blood_pressure_tracker_872b6833.xlsx
                     glyphs 737/719 -> 727/719   (band 15; +18 before, +8 after)

The residual +8 is the volatile-date confound — the reference recalculates the category axis'
`TODAY()`-derived dates and prints `9/8/2026` where we print the cached `11/6/2022`, one character
per label over eight labels. That is `dotnet/CLAUDE.md`'s sixth confound and is not this round's
to close.

**No other row moves a single scored column** — pages, words, fonts, unembedded, rawwords and
glyphs are identical on the other 946.

Byte-compared with the PDF date stamps masked (`movers.py`): **945 of 947 renderings are
byte-identical**, and of the two that differ, one is the witness and the other,
`PBN Matrix NAAs (V01).xlsx`, differs by five glyphs in a `&T` **time header** — `22:14` in one
sweep and `22:30` in the other. Its page count, glyph count, every text position and every drawing
are identical; the content-stream diff is one `Tj`. So the change moves **one rendering of 947**.

The slides and words tracks cannot move by construction: `SlideChart` and `FrameChart` do not
scale a chart's type at all, so `TypeScale` is 1 on every deck and every Writer document, and the
sweep confirms it — 302 + 338 renderings byte-identical.

**Reach, censused rather than inferred:** of the 90 corpus `.xlsx`/`.xlsm` carrying a chart part,
**38 state `fitToPage="1"` or a `pageSetup/@scale` other than 100** (`census.py`). Those are the
documents this can reach; that only one of them changes its axis is because the cap has to land on
a different integer, which needs the ratio to sit within one label height of a boundary.

## 5b. The instrument, and what it says once the change is in

The gate's glyph column cannot score this cluster — ten of the fourteen `chartset` failures
recalculate `TODAY()`, so their deltas move every day the gate is run — so the round's own
instrument is `axisticks.py`, which reads the **drawn tick labels of every vertical numeric label
column** out of a PDF, and `score-axes.py`, which pairs the two sides' axes by page and nearest x
and compares the label sets.

**It had to be calibrated before it could be believed, and its first two forms were both wrong in
the same direction — they invented differences.**

- Bucketing the labels by `round(x / 1 pt)` split an axis whose one-digit and three-digit labels
  differ by 0.03 pt in their right edge. On `002_advanced_excel_line` that read as the reference
  drawing eight ticks to our nine; both draw the same nine, and the `0` is at 190.52 against
  190.49. Single-linkage at 2 pt fixes it: **28 differences → 10**.
- Keying the *pairing* on a 40 pt grid split the witness' own secondary axis, at 534.74 here and
  543.09 in the reference — the same axis, 1.2% apart, because our chart is 1.2% narrower.
  Nearest-x pairing fixes it: **10 → 9**, and `040_Blood_pressure_tracker` scores `same`.

Over the 412 `chartset` documents of all three tracks, with the change in:

    same 128    differ 9    none 275

and the nine are worth listing, because **not one of them is an interval-count disagreement of the
kind this round closed except one**:

| document | ours | 26.2.4.2 | kind |
|---|---|---|---|
| `048_Expense_trends_budget` p2 | 0…500 step **50** | 0…500 step **100** | **cap: ours 10, the reference's ≤ 9** |
| `004_Contextures_chart_sample_de7c0671` | 0…300 | 0…160 | range |
| `006_Contextures_chart_sample_afa23b53` | 0…800 | 0…400 | range |
| `023_Waterfall_Chart_Template` | −5000…25000 | −2000…8000 | range |
| `014_…991ecfc5` `.xls`, `017_4-Stage_Vertical_Funnel` `.pptx`, `044_Cash_flow_forecast`, `047_Date_tracker_Gantt`, `microsoft_learn_multi_chart_examples` | — | — | one side draws a numeric column the other does not; the detector's own over-trigger (it finds a *sheet's* number column too) |

`048` is the one to take next, and it is the residual §7 names rather than a fresh cause: only a
cap of exactly 10 draws step 50 on that range (a cap of 9 gives `500/9 = 55.6 → 1.0 × 100`), so we
are sitting on the top of the clamp where the reference is anywhere below it, and the plot
rectangle we divide is still built from reservations measured at the zoomed size. Its sheet states
`fitToPage="1"` with scales 59, 83 and 100.

## 6. The two clean rows, and what they actually are

The brief asks that these two be re-read after the change and that the cause not be stretched to
cover them. Neither is this cause, and both are the **outlining ceiling** — the reference draws
glyphs as filled paths where we draw real text, so `pdftotext` scores our better output as a
surplus. Measured with `probes/odp-chart-r72/classify.py` over the banked r83 pair:

| document | page | ref chars | ours | Δ | ref glyph-sized fills | ours | ours rotated lines |
|---|---:|---:|---:|---:|---:|---:|---:|
| `057_Simple_balance_sheet` | 3 | 112 | 420 | +308 | **340** | 31 | **20** |
| `038_Competitive_Advantage_Card` | 1 | 684 | 752 | +68 | **75** | 11 | 0 |
| " | 2 | 684 | 752 | +68 | **75** | 11 | 0 |

`057` is the case `dotnet/CLAUDE.md` already records: twenty category labels turned 45°, which
26.2.4.2 outlines under the shear rule
(`VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D`,
`drawinglayer/source/processor2d/vclprocessor2d.cxx`:126-141). Its axis tick labels agree.

`038` is a `.pptx`, so `TypeScale` is 1 on it by construction and this round cannot reach it. Its
+136 is **exactly** the five diagram labels the reference outlines and we draw as text —
`Innovation`, `Product Quality`, `Brand Reputation`, `Cost Efficiency`, `Customer Service`, 68
characters a page over two identical pages — and **neither side draws a single rotated line**, so
the shear rule as stated (turned *and* anisotropically squeezed) does not explain it. Whatever
outlines them is a second trigger and is not identified here. It is not an axis question: the
deck's charts draw the same ticks on both sides.

## 7. What this leaves open

- **The rest of the composition is still measured at the zoomed size.** Only the interval cap is
  corrected. Every label *reservation* on a print-zoomed sheet still measures its text at the
  zoomed size, so where a chart2 metric is device-quantised the reserved band is out by the same
  kind of step — on the witness, a label height of 7.512 pt where `13.493 × 0.64 = 8.635` is what
  the reference reserves, 13% small. It is confined to the *reservations*, which are compared
  against lengths carrying the same zoom, so it moves a plot edge rather than a decision; the cap
  was the one place it changed a discrete outcome. Correcting it properly means laying the chart
  out at model size and scaling the finished `ChartDrawing`, which is a larger change than this
  round's evidence supports.
- **`ChartAxisLabels`' thinning is the same shape of ratio** — an axis length over a measured
  label *width* — and advance widths go through the same 96 dpi device (`MetricGrid.Chart`). It
  is left because the width quantisation is a few per cent where the line-height quantisation is
  a step of 13%, and no corpus document was found where it flips a label count.

## 8. Tests

Ten non-fidelity projects, each run alone:

    Core 521 · Containers 109 · Markup 259 · OpenDocument 143 · Presentations 1044
    Rendering 164 · Spreadsheets 1226 · Text 728 · Vector 302 · WordProcessing 1827

all `Failed: 0, Skipped: 0`. `XRay.Office.Fidelity.Tests` is **542 passed / 10 failed / 0 skipped**
with exactly the expected names — `PageDrawingComparisonTests.EveryLineIsDrawn` ×4,
`TabStopComparisonTests.AListLabelsTabAdvance` ×4, `SheetDrawingComparisonTests.APictureIsDrawn`,
`JustificationShrinkComparisonTests`. Build is warning-free.

`ChartAxisIntervalZoomTests` is the new coverage, and its ruler is the point of it: a measurer
whose height is *proportional* to the size cannot see this defect at all, so the test carries
`chart2`'s own 96 dpi rounding and a control that fails without the fix.

## 9. Files

| file | what |
|---|---|
| `axisticks.py` | reads a PDF's drawn value-axis label sets |
| `score-axes.py` | pairs two renderings' axes and compares the label sets |
| `sweep-ours.py` | renders our half and scores it against a banked reference |
| `movers.py` | byte-compares two sweeps with the PDF date stamps masked |
| `vary-h.py`, `vary-h.tsv` | the reference's flip points as the witness' plot height is swept |
| `census.py` | which chart-bearing workbooks state a print zoom |
| `rows-before.tsv`, `rows-after.tsv` | the two whole-corpus gate tables |
| `axes-chartset.txt` | the axis score over the 412 `chartset` documents, after |
