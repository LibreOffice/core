# The three rows O43 handed on: one is a confound, one is a label set, one is now wired

Round 112, seat `agent/chartaxisrange`, based on `4f4d0ddc8`. Reference **LibreOffice 26.2.4.2**
(`/opt/libreoffice26.2/program/soffice`, `0229ac93…`). The C++ tree read is
`/home/user/libreoffice-core`, which declares `27.2.0.0.alpha0+` and is **not** that binary's
source: every arm below is stated either as a source reading of that tree or as a measurement of
26.2.4.2, and every arm that matters is both.

**Register numbers taken this round: O49, O50, O51, O52.**

## 0. What this round settles

| Question | Answer |
|---|---|
| Is `055_Project_timeline`'s date axis a range-and-step defect? | **No. It is `TODAY()`.** With its thirteen volatile formulas frozen — at the cached 2023 serials *and* at the 2026 serials the reference itself recalculates to — 26.2.4.2 draws **exactly the fifteen upright 10-day labels this tree draws**, `5 Apr` to `23 Aug`. §1 |
| Then where does the reference's 38-label three-year axis come from? | The axis range is the **union of the import-time cache and the recalculated cells** — minimum from the cache, maximum from the recalculation. Proved by changing *only* a cached `<v>` and watching the reference's axis minimum follow it. §1.3 |
| Does `055`'s gate verdict move? | **No, and nothing in this seat could move it.** Its 85-character shortfall decomposes into **+111** the confound and **−26** a second, opposite-signed defect. §1.4 |
| What decides `027`'s value-axis interval? | **The widest of the axis' first three tick labels, and no others.** `MaxLabelTickIter`, measured directly at 26.2.4.2 by making the two halves of one axis disagree. §2 |
| Is that implemented? | **Yes.** `ChartLayout.MeasuredTicks`. §2.3 |
| Does it close `027`? | **No.** The label set was one of two terms; the other is the axis length the cap is taken against, which is bracketed at **[125.4, 138) pt** against our 107.2. §2.4 |
| Is a value axis wired to `ChartAxisLabels.Resolve`? | **Yes**, for a value axis running along the bottom. §3 |
| Reach | **2 of 176** chart-bearing corpus renderings, `SOURCE_DATE_EPOCH` pinned. A alone moves both; B alone moves one of them, and only in how it sets the labels. Both movers move **toward** the reference. §4 |
| Tests | 6 added, 2 of them failing at the base. 6740 unit tests pass in ten projects; `XRay.Office.Fidelity.Tests` **Failed 10, Passed 542, Skipped 0, Total 552**, exactly round 111's ten. §5 |

---

## 1. `055_Project_timeline_with_milestones` — the axis is right and the dates are volatile

### 1.1 What the file states, and what round 111 read off the page

Its one visible axis is a `c:dateAx` in `xl/charts/chart11.xml` with
`baseTimeUnit="days"`, `majorUnit="10"`, `majorTimeUnit="days"`, `numFmt="[$-409]d\ mmm;@"`, and
**no stated minimum or maximum**. Its categories are `'Project timeline'!$C$20:$C$36`, cached as
thirteen serials from 45021 (2023-04-05) to 45169 (2023-08-31) with `ptCount="17"`.

Rendered as it stands, on 2026-09-12:

* 26.2.4.2 draws **38** labels at 30-day steps, all turned 45°, `5 Apr` to `19 Apr`;
* this tree draws **15** upright at 10-day steps, `5 Apr` to `23 Aug`.

(Round 111 recorded ours as 14. It is 15, counted out of the PDF's text layer:
`5 Apr, 15 Apr, 25 Apr, 5 May, 15 May, 25 May, 4 Jun, 14 Jun, 24 Jun, 4 Jul, 14 Jul, 24 Jul,
3 Aug, 13 Aug, 23 Aug`.)

### 1.2 Freezing the volatile formulas makes the two agree exactly

Every cell of C20:C32 is `<f ca="1">DATE(YEAR(TODAY()),m,d)</f>`. The reference recalculates them
on load — its own `--convert-to fods` prints `office:date-value="2026-04-05"` … `"2026-08-31"` for
all thirteen — and this tree renders the cached 2023 serials. That is `CLAUDE.md`'s sixth confound,
and it is worth 111 characters here.

`freeze-055.py` makes two variants that differ from the file by one edit each: the `<f>` elements of
C20:C32 removed, once with the cached 2023 serials left standing and once with each shifted by
`46117 − 45021` so the frozen dates are the ones the reference recalculates to. The second variant
is what separates *the freeze* from *the year*.

```sh
python3 probes/chart-axis-r112/freeze-055.py /tmp/probe
/opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir /tmp/probe \
    /tmp/probe/055_static2023.xlsx /tmp/probe/055_static2026.xlsx
SOURCE_DATE_EPOCH=1700000000 XRay.Office.Cli render --outdir /tmp/ours /tmp/probe/055_static*.xlsx
```

| variant | 26.2.4.2 | X-Ray.Office |
|---|---|---|
| as authored | 38 labels, 30-day, turned, `5 Apr`…`19 Apr` | 15 labels, 10-day, upright, `5 Apr`…`23 Aug` |
| **`static2023`** | **15, 10-day, upright, `5 Apr`…`23 Aug`** | 15, 10-day, upright, `5 Apr`…`23 Aug` |
| **`static2026`** | **15, 10-day, upright, `5 Apr`…`23 Aug`** | 15, 10-day, upright, `5 Apr`…`23 Aug` |

Label for label. **This tree's date-axis automatism reproduces 26.2.4.2 exactly on this document
once the two sides are given the same data**, and the year the data is frozen at does not enter it.
`freeze.tsv` has the rows.

### 1.3 The reference's axis range is the union of the cache and the recalculation

That leaves the 38-label axis to explain, and the explanation is not a scaling rule.

`cache40000` leaves the `TODAY()` formulas **in place** and changes only C20's cached `<v>`, 45021
→ 40000 (2009-07-06). A value that lives only in the cache cannot reach a recalculated cell, so if
the axis minimum follows it, the axis is being scaled against data the recalculation has replaced.

It follows it. 26.2.4.2 draws 41 labels beginning `6 Jul`, `6 Dec`, `6 May`, `6 Oct`, `6 Mar`,
`6 Aug` — five-month steps from 2009-07-06 — where the unmodified file gives 30-day steps from
2026-04-05.

Both renderings then fall out of `ChartDateScale`'s own rules with the range taken as
`[min(stale, fresh), max(stale, fresh)]`, which is what `ScaleAutomatism::expandValueRange` does:
it only ever widens.

* unmodified: `[45021, 46265]`, 1244 days. The stated 10-day interval survives
  (`1244/10 = 124 ≤ 499`), giving ~125 ticks, and `ChartAxisLabels`' collision ladder settles on a
  rhythm of 3 — 30 days between labels, which is what is drawn.
* `cache40000`: `[40000, 46265]`, 6265 days. `6265/10 = 626 > 499`, so **the stated interval is
  discarded** and the automatic rule runs on nominal days: `6265/499 = 12`, which is above a week,
  so months, `floor(12/31) = 0 → 1` month. Monthly ticks from 6 July at a rhythm of 5 give
  `6 Jul, 6 Dec, 6 May, 6 Oct, 6 Mar, 6 Aug` — the observed sequence, and 41 labels over 206 months
  against the observed 41.

So the same three rules `ChartDateScale` already implements produce both of the reference's axes;
what this tree does not have is the *data* they are computed from. That is not this seat's to fix
and arguably not any seat's — reproducing it means evaluating `TODAY()` **and** keeping the
import-time cache alive beside it.

### 1.4 The 85 characters, decomposed — and the diagnosis handed to this seat was incomplete

`gate-r112` records `055` as `pages 2/2, glyphs 960/1045`, failing on column 9 alone with a band of
`max(2% × 1045, 15) = 20.9`.

Measured with `batch-check.sh`'s own rule (`pdftotext` then `sum(1 for c in text if c.isalnum())`),
on the same UTC day:

| | glyphs |
|---|---:|
| 26.2.4.2, as authored | **1045** |
| 26.2.4.2, `static2023` and `static2026` (identical) | **934** |
| X-Ray.Office, all three | **960** |

* **+111** is the confound: 23 extra date-axis labels the union range gives the reference. Not
  recoverable here.
* **−26** is a *second and opposite* difference, and it is real. The chart's thirteen milestone
  data labels are `[CELLRANGE]` + `[CATEGORYNAME]` fields. **26.2.4.2 writes the category name
  through the date axis' own `[$-409]d mmm`** — `5 Apr`, `24 Apr`, `1 May` — and this tree writes
  it through the source cell's `m/d/yyyy` — `4/5/2023`, `4/24/2023`, `5/1/2023`. Two alphanumeric
  characters per label, thirteen labels, **exactly 26**.

`85 = 111 − 26`. **So the arithmetic the brief asked to be checked does not come out**: the missing
labels are worth 111 characters and not 85, and the residual is a defect with the opposite sign.

**The consequence for the gate is uncomfortable and is stated rather than buried.** Fixing the
data-label format alone would take us from 960 to 934 against the reference's 1045 — a shortfall of
**111 instead of 85**, a worse row on a metric that cannot see that the labels now read what the
reference's read. It is recorded as **O50** with this measurement and deliberately not implemented
in the same round as work whose reach is being measured on the same documents.

**`055`'s verdict does not move, before or after this round.** 960/1045 both legs.

---

## 2. `027_Simple_personal_cash_flow_statement` — the interval cap measures three labels

### 2.1 What differs, and which chart it is

Page 6 carries four horizontal bar charts. The one that differs is `xl/charts/chart44.xml`, the
savings chart (`401(k)/Etc`, `Savings/Investment`, `Cash Reserves`, `Other 1`, `Other 2`), data
0…12,000, a `c:valAx` along the bottom in `"$"#,##0` with **no stated `c:majorUnit`**.

* 26.2.4.2: **eight** labels, `$0 … $14,000` at 2,000, turned 45°.
* this tree at the base: **four**, `$0 … $15,000` at 5,000, upright.

`chart22`, which *does* state `majorUnit="2000"`, is a different chart on the same page and agrees
on both sides — the reference's own `--convert-to fods` writes `chart:interval-major="2000"` for
that one and nothing for the other three, which is how the two were told apart without rendering.

The reference's rotation is not stated either: no `style:rotation-angle` appears on any of the four
value axes in that fods, and `rot="-60000000"` on the `c:txPr` is out of range and reads as zero. So
the 45° is auto-rotation, and it follows the interval rather than the other way round — at a 5,000
step the four labels do not collide.

### 2.2 Which labels the cap is measured over, measured at the binary

`estimateMaximumAutoMainIncrementCount` divides the axis' length by `m_nMaximumTextWidthSoFar`
(`VCartesianAxis.cxx`:1559-1618). That field is **not** the widest label on the axis: it is the
widest of the labels `createMaximumLabels` built, and that pass iterates a `MaxLabelTickIter`
(`:455-511`, `:1517-1530`) which for a value axis (`m_bUseTextLabels` false) is seeded with **zero**
and holds indices `{0, 1, 2}`.

`label-set-027.py` tests exactly that, with the axis' `c:numFmt` as the single variable:

```sh
python3 probes/chart-axis-r112/label-set-027.py /tmp/probe
/opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir /tmp/probe /tmp/probe/027_*.xlsx
```

| variant | format code | labels drawn | step | turned |
|---|---|---:|---:|---|
| `k0` | `[<5000]"$"#,##0;[>=5000]"$"#,##0;General` | 8 | 2000 | yes |
| **`lateWide`** | `[<5000]"$"#,##0;[>=5000]"$"#,##0"WWWWWWWW";General` | **8** | **2000** | yes |
| **`earlyWide`** | `[<5000]"$"#,##0"WWWWWWWW";[>=5000]"$"#,##0;General` | **3** | **10000** | no |

Widening every label from 6,000 up by eight characters changes **nothing**. Widening the first
three alone collapses the axis to three labels. A cap taken over the widest label on the axis
predicts the opposite of both.

The boundary is sharp. Sweeping the suffix on the first three from zero to eight characters, at the
drawn 5.1 pt where one `X` is 3.51 pt and `$4,000` is 17.91:

| pad on the first three | labels | step |
|---|---:|---:|
| none | 8 | 2000 |
| `X` … `XXXXXXX` | 4 | 5000 |
| `XXXXXXXX` | 3 | 10000 |

**One character** takes the cap from ≥7 to ≤6. All eleven rows are in `boundaries.tsv`.

### 2.3 Implemented

`ChartLayout.MeasuredTicks` is `scale.MajorTicks().Take(3)`, and `IntervalsThatFit`'s horizontal
branch measures those rather than every tick. The vertical branch is untouched deliberately: it is
capped on `m_nMaximumTextHeightSoFar`, and one line of digits is as tall as any other, so the two
readings coincide there — which is also why round 87's two calibration witnesses
(`chart-bar-sheet.xlsx` at 54.6/11.5 and `.ods` at 108.8/11.5) are unaffected.

### 2.4 It does not close `027`, and the residual is bracketed rather than guessed

With the fix, this tree's cap on that chart is `available / w("$4,000")`; the reference's is
`nTotalAvailable / w("$4,000")`. The *numerator* still disagrees.

From the sweep above, at the drawn scale: `available_ref / 17.91 ≥ 7` and
`available_ref / (17.91 + 3.51) < 7`, and `k8` gives `available_ref / 45.99 < 3`. So

> **`available_ref` ∈ [125.4, 138) pt**, where the axis this tree draws is **107.2 pt** and the
> axis the *reference* draws is **115.7 pt**.

The reference's cap is therefore taken against a rectangle **larger than either side's final plot
area**, and where that rectangle comes from — `reduceToMinimumSize`, `adjustInnerSize` and
`mbUseFixedInnerSize` in `ChartView::impl_createDiagramAndContent`:559-640 — is a plot-rectangle
question and not an interval one. Left as **O51** with this bracket, which is the useful part: a
round taking it has a number to hit rather than a mechanism to find.

---

## 3. A value axis is wired to `ChartAxisLabels.Resolve`

Round 111 gave a `c:valAx` chart2's own `TextBreak`/`TextOverlap`/`ArrangeOrder` and measured nil,
because nothing that draws consulted them. `ChartLayout.ArrangeValueLabels` is the half that draws:
a value axis running along the bottom now goes through `ChartAxisLabels.Resolve` with its own
stated flags, and `AddValueAxis` takes the rotation and the rhythm from the answer;
`PlotAreaOf` takes the same rotation, so the band the plot gives up and the last label's overhang
follow it. Same two-step shape as the category axis: arrange, recompose the rectangle, arrange
again.

**Round 111's reader fix is confirmed a second time here, against the binary rather than the
tree.** `--convert-to fods` on `027` gives all four of its value axes
`text:line-break="false"` with **no `chart:label-arrangement`** attribute at all, against
`text:line-break="true" chart:label-arrangement="side-by-side"` on the category axes beside them;
the same holds for `055`'s `c:dateAx` (`text:line-break="false"`, no arrangement) against its
`c:catAx` (`true`, `side-by-side`). That is `axisconverter.cxx`:306-326's table, read out of
26.2.4.2's own resolved view.

Scope stated rather than implied: **only the horizontal branch**. A value axis running down the
left has one label per tick on separate lines and cannot collide with itself, and `AddValueAxis`'s
rotation path carries `LabelPositionHelper`'s `_Bottom`/`_Top` corrections and not its
`_Left`/`_Right` ones. A scatter chart's domain axis is left to `AddDomainAxis`, which is a separate
seat.

---

## 4. Reach, measured on renderings

`../chart-fit/census.tsv`'s **176 chart-bearing corpus documents**, rendered three times by
`../chart-collide-r110/confine.py` — at the base `4f4d0ddc8`, with the label-set fix alone, and with
both — with `SOURCE_DATE_EPOCH=1700000000` so the PDF's own timestamp is out of the digest. Each
rendering's sha256, page count and alphanumeric characters are banked and the PDF deleted; all 176
succeeded on all three legs.

```sh
export SOURCE_DATE_EPOCH=1700000000
tail -n +2 probes/chart-fit/census.tsv | cut -f1 > /tmp/chartdocs.txt
for leg in base aonly after; do
  python3 probes/chart-collide-r110/confine.py --out /tmp/conf-$leg \
      --cli /tmp/cli-$leg/XRay.Office.Cli --rows /tmp/rows-$leg.tsv --jobs 3 < /tmp/chartdocs.txt
done
```

**2 of 176 move.** `reach.tsv` has the row per document and the `moved_by` column attributing each.

| document | pages | glyphs base → after | moved by |
|---|---|---|---|
| `026_Monthly_cash_flow_statement` | 11 → 11 | 7864 → **7879** | the label set **and** the arrangement |
| `045_Check_register_with_chart` | 1 → 1 | 736 → 736 | the label set alone |

The pin is not decoration: r111 recorded an unpinned sweep of this same set reporting 176 of 176.
The three legs here differ on two rows and are byte-identical on 174.

### 4.1 Both movers move toward the reference, and one of them exactly

Reference rendered **twice** for each, per C11's procedure; both are stable
(`026` 7852 both runs, `045` 764 both runs), so the per-document deltas below are real.

**`026_Monthly_cash_flow_statement`, page 7, the first chart.** Base draws three upright labels
`$0 $50,000 $100,000`; after draws **six turned 45°**, `$0 $20,000 … $100,000`. The reference's
text layer holds **none** — and its page 7 carries **39 glyph-sized filled paths** in that label
band against our one, where the six labels are **38 characters**. So 26.2.4.2 draws exactly the
axis this tree now draws and *outlines* it, which is `CLAUDE.md`'s shear rule arriving on a value
axis. The glyph count moves the wrong way (+15 against a reference of 7852, band 157, verdict
unchanged) for the same reason it did on `038`: we draw searchable text where the reference draws
filled paths. This is a raster-ceiling row, not a regression.

**`045_Check_register_with_chart`.** No label moves — that chart's axis is deleted — but the finer
interval lowers the value-axis maximum and the bars grow. Longest bar:

| | width |
|---|---:|
| base | 47.56 pt |
| **after** | **63.41 pt** |
| 26.2.4.2 | 79.24 pt |

The error halves, 31.68 → 15.83. The residual is §2.4's numerator.

### 4.2 Attribution

* **The label set (`MeasuredTicks`)** moves both documents. On `026` it alone already draws the six
  labels, upright.
* **The value-axis arrangement** moves **one** document and changes no character on it: `026`'s six
  labels go from upright to 45°. That is what it should do — the arrangement only fires when the
  labels collide, and after the interval cap is right they rarely do. It is worth carrying anyway,
  because on the one document where the reference turns a value axis, this is what turns ours.

---

## 5. Tests

`tests/XRay.Office.Core.Tests/ChartValueAxisArrangementTests.cs`, five cases over one fixture, plus
one in `ChartDateScaleTests.cs`.

| case | what it holds |
|---|---|
| `WideningTheLabelsPastTheThirdDoesNotChangeTheInterval` | **fails at the base** — §2.2's `lateWide` arm as a unit test |
| `WideningTheFirstThreeAloneDoesChangeIt` | the control that makes it mean something — `earlyWide` |
| `AHorizontalValueAxisTurnsItsLabelsWhenTheyCollide` | **fails at the base** — the arrangement reaches the drawing |
| `AVerticalValueAxisIsNotArrangedHere` | the arm the scope must not cross |
| `AStatedRotationIsNotOverriddenByTheArrangement` | the file's own statement still outranks it |
| `TheProjectTimelineAxisIsFifteenTicksTenDaysApart` | §1's refutation: the axis is right for the data it is given |

Confirmed against the base by swapping `ChartLayout.cs` back for one run: **2 failed, 4 passed**
(the sixth is in the date-scale file and passes on both sides, which is the point of it).

**Totals, one project at a time, from this diff's own build**
(`probes/chart-axis-r112/` cites them; the raw logs are not committed):

```
Containers 109   Core 579   Markup 259   OpenDocument 160   Presentations 1142
Rendering  164   Spreadsheets 1352   Text 728   Vector 309   WordProcessing 1938
                                                   6740 passed, 0 failed, 0 skipped
```

Every one of the ten is at or above round 111's, so none is a truncated run; Core is +6, of which 6
are this round's, and Presentations +22 and Spreadsheets +6 come from rounds merged since.

**`XRay.Office.Fidelity.Tests`, the one that can skip silently: `Failed: 10, Passed: 542,
Skipped: 0, Total: 552`** — 0 skipped, so it covered everything, and the ten are exactly round
111's, none of them a chart:

```
TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop    x4
PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt x4
SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt x1
JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt x1
```

---

## 6. What is open, and why

* **O50 — the chart data label's `CATEGORYNAME` on a date axis.** §1.4. Measured at 26 characters
  on `055` and correct against the reference in content; **not implemented**, because on the one
  document it is measured on it takes column 9 from 85 short to 111 short. It wants its own reach
  census over every chart with a `[CATEGORY NAME]` field and a date axis, measured on ink rather
  than on glyphs.
* **O51 — the axis length the interval cap is taken against.** §2.4, bracketed at
  [125.4, 138) pt where the drawn axis is 107.2 (ours) and 115.7 (the reference's). It is a
  plot-rectangle question: `reduceToMinimumSize` then `adjustInnerSize` then the estimate, with
  `mbUseFixedInnerSize` deciding whether either runs.
* **`055`'s remaining 111 characters.** Not a chart question at all — it needs `TODAY()` evaluated
  *and* the import-time cache kept beside it, because the reference's range is the union of the
  two. Recorded as **O49 / NOT WORK** rather than left as a chart seat.
* **O43's two-word restart.** Untouched by this round and still exactly as round 111 left it.
* **The value axis' `Resolve` on a *vertical* axis and on a scatter's domain axis.** §3. Neither is
  wired and no corpus document was found that needs either; the vertical one additionally needs
  `LabelPositionHelper`'s `_Left`/`_Right` corrections, which nothing in this tree has.

## 7. Files

* `results.md` — this.
* `freeze-055.py` — the three one-edit variants of `055`: two freezes and one cache-only change.
* `label-set-027.py` — the eleven one-attribute variants of `027`'s savings-chart number format.
* `boundaries.tsv` — the eleven rows the reference drew for them.
* `freeze.tsv` — `055` and its three variants, both renderers, with column-9 counts.
* `reach.tsv` — the 176 chart-bearing documents at three commits, with the mover attribution.
