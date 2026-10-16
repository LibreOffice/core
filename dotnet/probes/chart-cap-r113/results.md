# The two rows round 112 measured and did not build: the cap's numerator, and the category name's format

Round 113, seat `agent/chartcap`, based on `f160c8a2c`. Reference **LibreOffice 26.2.4.2**
(`/opt/libreoffice26.2/program/soffice`, `0229ac93…`). The C++ tree read is
`/home/user/libreoffice-core`, which declares `27.2.0.0.alpha0+` and is **not** that binary's
source: every arm below is stated either as a source reading of that tree or as a measurement of
26.2.4.2, and every arm that matters is both.

**Register numbers taken this round: O56** (the turned value label's right-hand overhang, split out
of O51's residual). O50 and O51 are closed.

## 0. What this round settles

| Question | Answer |
|---|---|
| What is the axis length the interval cap is taken against? | **The inner rectangle `adjustInnerSize` produces from the *maximum-label* pass** — the available width less what three unwrapped, unturned labels hang outside the diagram, with nothing taken off the far end. On `027` that is **[129.7, 131.1) pt** measured and **≈130.7 pt** predicted, against a drawn axis of 115.7 (reference) and 107.2 (ours) and a bracket of [127.4, 138) from r112. §1 |
| Is that confirmed at the binary and not only in source? | **Yes, and it is the sharpest arm this round has.** Widening the category axis' *widest* label — the one `MaxLabelTickIter` resets past — leaves the value axis at `$0 $2,000 … $14,000` while the drawn axis collapses **115.67 → 75.10 pt**. §1.2 |
| Which label is "the longest"? | **A character count, first one wins** — `getIndexOfLongestLabel`'s own `//todo`. Reading it as a width costs `026` a whole interval. §1.4 |
| Does the same defect exist on a *vertical* value axis? | **Not measurably.** Four variants over a 120 pt frame fit both readings; the two differ by at most half a label's height. Left alone. §2 |
| What format does a `[CATEGORY NAME]` field take? | **The category axis'**, resolved once before the loop. Three axis formats move every label; two cell formats and `sourceLinked="1"` move nothing at all. §3 |
| Reach | **2 of 176** chart-bearing corpus renderings, one per issue, `SOURCE_DATE_EPOCH` pinned; **0 of 182** chart-bearing converted-ODF files. §4 |
| The gate consequence of O50 | **`055`'s column 9 goes from 85 short to 111 short and its verdict does not move.** With the `TODAY()` confound frozen out it is **934 against the reference's 934, exact**, where it was 960. §5 |
| Tests | 9 added, **5 of them failing at the base**. 6787 unit tests pass in ten projects; `XRay.Office.Fidelity.Tests` **Failed 10, Passed 542, Skipped 0, Total 552** — exactly round 112's ten. §6 |

---

## 1. O51 — the rectangle the cap's numerator is taken against

### 1.1 The mechanism, read by hand

`VCartesianAxis::estimateMaximumAutoMainIncrementCount`
(`chart2/source/view/axes/VCartesianAxis.cxx`:1559-1618) divides `nTotalAvailable` by
`m_nMaximumTextWidthSoFar`. `nTotalAvailable` is `get2DAxisMainLine`'s length **at the moment it
is called**, and the only caller is `VCoordinateSystem::prepareAutomaticAxisScaling`:415-417,
inside `doAutoScaling`. `ChartView::impl_createDiagramAndContent` (`ChartView.cxx`:556-604) puts
that call between two resizes rather than after them:

1. `aVDiagram.reduceToMinimumSize()` — the inner rectangle becomes the available one over **2.2**
   (`VDiagram.cxx`:635-651), guarded by `!mbUseFixedInnerSize`;
2. `pVCooSys->createMaximumAxesLabels()` — every axis lays out the *three* labels
   `MaxLabelTickIter` picks (`VCartesianAxis.cxx`:1769-1809), with `m_bOverlapAllowed` forced
   **true** and `m_bLineBreakAllowed` forced **false**;
3. `aConsumedOuterRect = getRectangleOfShape(xBoundingShape)`, then
   `aNewInnerRect = adjustInnerSize(aConsumedOuterRect)` — the inner rectangle grows back by
   `available − consumed` per dimension, floored at a third of the available one
   (`VDiagram.cxx`:653-698);
4. `setTransformationSceneToScreen(aNewInnerRect)`, and **then** `doAutoScaling`.

The `2.2` cancels: the consumed rectangle is the reduced diagram plus whatever hangs outside it, so
`newInner = reduced + (available − reduced − overhang) = available − overhang`. On a bar chart the
near-side overhang is the *vertical* category axis' own maximum labels, and the far side is
**nothing**, because the value labels the maximum pass drew are `{0, 1, 2}` — all at the near end
of the axis. So the numerator is wider than the plot as drawn, and wider than the reference's own
drawn plot.

Two supporting readings, both needed and both easy to get wrong:

* **The maximum pass never turns or wraps a label.** `canAutoAdjustLabelPlacement`
  (`:539-556`) returns false the moment `m_bOverlapAllowed` is set, and
  `isAutoRotatingOfLabelsAllowed` is an alias of it. A rotation the *file* states still applies.
* **A chart stating its own inner rectangle skips all of it.** `mbUseFixedInnerSize` guards
  `reduceToMinimumSize` and every `adjustInnerSize` alike (`ChartView.cxx`:559, :594, :619, :690)
  and is the diagram's `PosSizeExcludeAxes`, which `c:layoutTarget val="inner"` and ODF's
  `chart:coordinate-region` both set.

### 1.2 Confirmed at the binary, on the axis at right angles to round 112's

`cat-set-027.py` rewrites **one category at a time** in `027`'s savings chart's own `c:strCache`.
(Its first cut rewrote the *sheet* cells `AA11:AA16` instead and moved nothing at all: that chart
is a pivot chart and Calc does not relink it to the cells. The cache is the source; the sheet edit
is the control that says so.)

Its five categories are `Other 1`, `Other 2`, `Cash Reserves`, `Savings/Investment`,
`401(k)/Etc`. The longest is index 3 — `nMaxIndex-1` — so `MaxLabelTickIter`
(`VCartesianAxis.cxx`:472-495) resets to zero and holds `{0, 1, 2}`, whose widest is
**`Cash Reserves`** and not `Savings/Investment`.

```sh
python3 probes/chart-cap-r113/cat-set-027.py /tmp/cat
/opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir /tmp/cat/ref /tmp/cat/027_*.xlsx
python3 probes/chart-cap-r113/cat-set-027.py --read /tmp/cat/ref
```

| variant | value-axis labels | step | widest of {0,1,2} | drawn axis |
|---|---:|---:|---:|---:|
| control | 8 | 2,000 | 37.76 | 115.67 |
| **`w8i3`** — eight `W` on `Savings/Investment` | **8** | **2,000** | 37.76 | **75.10** |
| **`w8i4`** — eight `W` on `401(k)/Etc` | **8** | **2,000** | 37.76 | 99.34 |
| `w8i0` / `w8i1` — eight `W` on `Other 1` / `Other 2` | 4 | 5,000 | 59.99 | 97.91 |
| `w8i2` — eight `W` on `Cash Reserves` | 4 | 5,000 | 78.33 | 79.57 |

**A drawn axis a third shorter with the same interval is what rules the plot rectangle out**, and
it is the same shape of experiment round 112 ran on the value axis' own labels. All eighteen rows
are in `cat.tsv`.

### 1.3 The numerator, pinned rather than bracketed

The fine sweep pads `Cash Reserves` with `l`, worth **1.4225 pt** each at that document's drawn
scale (measured off the sweep itself: 37.76 → 39.18 → 40.60 → 42.03 → 43.45). Seven intervals
survive three `l` and drop to three at four, so with `w("$4,000") = 17.915` pt — read off the
same page's *upright* `$4,000` on the expenses chart —

> **`nTotalAvailable` ∈ [129.68, 131.10) pt.**

The model's prediction, with every term measured off the reference's own page 6:

| term | pt |
|---|---:|
| chart frame (`svg:width` 4.3205 in from 26.2.4.2's own `--convert-to fods`) × the sheet's scale | 177.2 |
| less the 2 % margins (`getAvailablePosAndSizeForDiagram`, in 1/100 mm: 10974 → 10536) | **170.1** available |
| less the widest of `{0, 1, 2}` (`Cash Reserves`, 37.76) and the label-to-axis gap (1.66) | −39.4 |
| less the far-end overhang at the maximum pass | **0** |
| = | **≈130.7** |

which is inside the measured window. The alternative in which the value labels also consume on the
far side predicts 128.7 and is outside it. `130.7 / 17.915 = 7.29 → 7` intervals, which is the
`$0 … $14,000` the reference draws; the whole `k`-suffix ladder of round 112's `boundaries.tsv`
falls out of the same number (one `X` → 6, seven `X` → 3, eight `X` → 2).

### 1.4 And "the longest label" is a character count

`VAxisBase::getIndexOfLongestLabel` (`chart2/source/view/axes/VAxisBase.cxx`:195-212) compares
`getLength()` under its own `//todo: get real text width (without creating shape) instead of
character count`, with a strict `>`, so **the first of two equally long labels wins**.

This is not a detail. `026_Monthly_cash_flow_statement`'s expenses chart (`chart31`) has eighteen
categories whose two longest are `Disability premiums` at index 12 and `Federal/SS/Medicare` at
index 16 — nineteen characters each, and the second is `nMaxIndex-1`. Choosing by *width* picks
index 16, the reset then sends the set to `{0, 1, 2}` — `Other 1`, `Other 2`, `Garbage` — and the
numerator comes out 25 pt too large, which is exactly one interval there. This tree built that
reading once and drew `$0 $2,000 … $16,000` where 26.2.4.2 draws `$0 $5,000 … $20,000`; counting
characters reproduces the reference and takes that document back to byte-identical.

**It was found by the reach sweep and not by the probe**, which is the argument for sweeping before
committing rather than after.

### 1.5 Implemented

`ChartLayout.MaximumPassWidth` is the numerator and `WidestMaximumCategoryLabel` is
`MaxLabelTickIter` over the category texts; `IntervalsThatFit` takes the width from the first and
its *horizontal* branch alone uses it. A chart stating `PlotArea` or `PlotAreaFraction` — ODF's
`chart:coordinate-region` and `c:layoutTarget val="inner"` — is returned the drawn plot area
unchanged, and the result is floored at a third of the available width, which is
`adjustInnerSize`'s own clamp.

### 1.6 What is left on `027`, and it is a different seat — **O56**

The interval is now right and the plot's **left** edge agrees to the hundredth of a point
(645.05 on both sides), but our axis is **108.16 pt against the reference's 115.67** — 7.51 pt
short at the right end. `PlotAreaOf` reserves half the rotated last label,
`(w·cos + h·sin)/2 = 11.4 pt` at 45°, where the reference consumes about **1.1**: its own
`$14,000` is drawn overhanging the axis end by 2.00 pt, so it is anchored so as to extend down and
to the left rather than centred on the tick. That is `LabelPositionHelper::correctPositionForRotation`
and not the cap.

---

## 2. The vertical branch is left alone, and that is measured

On a column chart the crossing axis is horizontal and the cap divides by
`m_nMaximumTextHeightSoFar` — one line of digits whatever the labels say — so the numerator is the
only variable left. `colcap.py` puts a minimal column chart over 0…12,000 into
`038_Competitive_Advantage_Card`'s slide 1 at a **120 pt** frame and varies only the length of the
five identical category names:

| categories | drawn axis | value labels | step |
|---|---:|---:|---:|
| `Aa` | 96.61 | 8 | 2,000 |
| 20 characters | 86.12 | 8 | 2,000 |
| 40 characters | 60.36 | 4 | 4,000 |
| 60 characters | 42.40 | 4 | 4,000 |

**The numerator is not constant** — so the maximum pass' horizontal labels take the depth they take
when drawn, unlike the bar chart's vertical ones — and the observable is too coarse to separate
*the drawn plot height* (predicting caps 8, 7, 5, 3) from *the available height less one line*
(predicting 9, 8, 6, 4): both fit, because the visible answer only distinguishes `cap ≥ 7` from
`3 ≤ cap ≤ 6`. The two readings differ by at most the half label the top edge gives up, so the
vertical branch keeps `area.Height` and this is recorded rather than guessed at.

---

## 3. O50 — a `[CATEGORY NAME]` field takes the axis' number format

### 3.1 The mechanism

`VSeriesPlotter::getCategoryName` (`VSeriesPlotter.cxx`:2213-2224) is
`m_pExplicitCategoriesProvider->getSimpleCategories()[n]`, and `:511-513` is the `CATEGORYNAME`
field's own case. Those strings are built by
`ExplicitCategoriesProvider::convertCategoryAnysToText`
(`chart2/source/tools/ExplicitCategoriesProvider.cxx`:186-227), which resolves **one** number
format before the loop — `getAxisByDimension2(0, 0)` through
`AxisHelper::getExplicitNumberFormatKeyForAxis` — and writes every numeric category through it. A
category that is already a string is passed straight out (`aAny >>= aText`).

So the field and the category axis' own tick labels are the same strings, and the source cell's
format reaches neither.

### 3.2 Confirmed at the binary, one attribute at a time

`catname-055.py` makes seven variants of `055_Project_timeline_with_milestones`, whose thirteen
milestone labels are a `CELLRANGE` field over a `CATEGORYNAME` field, whose `c:dateAx` states
`[$-409]d\ mmm;@ sourceLinked="0"` and whose `C20:C36` cells are `m/d/yyyy`.

| variant | spans changed | what moved |
|---|---:|---|
| `axis_yyyy` | 51 | every label becomes `2023`/`2024`/`2025`/`2026` |
| `axis_mmmm` | 51 | `April`, `August`, … |
| `axis_hash` (`0.00`) | 51 | `45021.00`, … — the serials |
| `axis_linked` (`sourceLinked="1"`, code unchanged) | **0** | nothing |
| `cell_yyyy`, `cell_hash` | **0** | nothing |

`catname.tsv` has the counts. The 51 is 38 axis tick labels plus **13** data labels, and in
`axis_yyyy` exactly thirteen of the twenty-two `2023` spans are the data labels — the reference
writes them from the *cached* 2023 serials while its axis is scaled on the union of cache and
recalculation (r112 §1.3), which is why the values agree and only the format did not.

`axis_linked` moving nothing is `ObjectFormatter::convertNumberFormat`
(`oox/source/drawingml/chart/objectformatter.cxx`:1143-1147) measured: for an **axis** it sets
`LinkNumberFormatToSource` from `maFormatCode.isEmpty()` and ignores `sourceLinked` outright,
under a comment saying the property *"does not really work, at least not for axis"*.

### 3.3 Implemented, and why it needed the date axis' serials

`ChartLayout.CategoryTextAt` replaces the five `plot.Categories[index]` at the `Compose` call
sites. `ChartDataLabel.WriteCategory` alone was not enough: an OOXML chart in a workbook resolves
its `c:cat` range against the **live sheet**, so `Categories[n]` arrives as `4/5/2023` — already
written through the cell's format — and there is no number left to reformat. The serials survive
on `plot.DateAxis.CategoryValues`, which is where the reference's own numbers come from too, so a
date axis' field is written from those and everything else falls through to `WriteCategory`.

### 3.4 The census, with the base rate beside it

`catname-census.py` over the whole 947-document corpus, and it says what it counted over: the
**zip** formats, which is 167 of the 176 chart-bearing documents (a `.xls` chart lives in a BIFF
substream and no zip walk sees one).

| | |
|---|---:|
| corpus documents | 947 |
| chart-bearing renderings (`probes/chart-fit/census.tsv`) | 176 |
| zip documents holding a chart part | **167** |
| chart parts in them | 307 |
| parts stating a category-name label (`c:showCatName val="1"` or an `a:fld type="CATEGORYNAME"`) | **35**, in **25** documents |
| …of those, whose `c:cat` cache is numeric — the only ones a number format can reach | **1** |
| …of those, over a `c:dateAx` | **1** |

The one is `055`. The other 34 parts have string categories, which
`convertCategoryAnysToText` passes straight out and this change leaves untouched — which the reach
sweep then confirms rather than assumes.

---

## 4. Reach, measured on renderings

`../chart-fit/census.tsv`'s **176 chart-bearing corpus documents**, rendered three times by
`../chart-collide-r110/confine.py` — at the base `f160c8a2c`, with the cap fix alone, and with
both — with `SOURCE_DATE_EPOCH=1700000000` so the PDF's own timestamp is out of the digest. All
176 succeeded on all three legs; the three binaries were built **before** the sweep started and
none of them was rebuilt during it.

```sh
export SOURCE_DATE_EPOCH=1700000000
tail -n +2 probes/chart-fit/census.tsv | cut -f1 > /tmp/chartdocs.txt
for leg in base o51 after; do
  python3 probes/chart-collide-r110/confine.py --out /tmp/conf-$leg \
      --cli /tmp/cli-$leg/XRay.Office.Cli --rows /tmp/rows-$leg.tsv --jobs 3 < /tmp/chartdocs.txt
done
```

**2 of 176 move, one per issue**, and 174 are byte-identical across all three legs. `reach.tsv`
has the row per document.

| document | pages | glyphs base → after | 26.2.4.2 | moved by |
|---|---|---|---:|---|
| `027_Simple_personal_cash_flow_statement` | 10 → 10 | 7932 → **7949** | **7949** | O51 |
| `055_Project_timeline_with_milestones` | 2 → 2 | 960 → **934** | 1045 (934 frozen) | O50 |

The reference was rendered **twice** for each of the three candidates, per C11: `026` 7852 both
runs, `027` 7949 both runs, `055` 1045 both runs, so the per-document deltas are real.

**`027` lands on the reference's alphanumeric count exactly**, 7949 of 7949, from 17 short. Its
savings chart draws `$0 $2,000 … $14,000` turned 45° where 26.2.4.2 draws `$0 $2,000 … $14,000`
turned 45°, label for label.

**And the converted ODF corpus does not move at all.** All 182 chart-bearing files of
`/home/user/corpus-odf` — `.ods` 99, `.odp` 73, `.odt` 10 — rendered at both ends: **0 moved**,
182 byte-identical. `odf-reach.tsv`. The reference half was not re-rendered, which is sound because
the diff is confined to `dotnet/src` and cannot reach `soffice`.

---

## 5. The gate consequence of O50, stated rather than buried

`055_Project_timeline_with_milestones` is `pages 2/2, glyphs 960/1045` at the base and
`pages 2/2, glyphs 934/1045` after, against a band of `max(2 % × 1045, 15) = 20.9`.

| | column 9 | shortfall | verdict |
|---|---:|---:|---|
| base | 960 / 1045 | 85 | mismatch (`glyphs`) |
| **after** | **934** / 1045 | **111** | mismatch (`glyphs`) |

**So this correct change makes that document's gate column worse by 26 characters, and its verdict
does not move.** The row was failing before and fails after; nothing else in the corpus moves in
either direction.

The reason is C12 and it is not recoverable from a chart seat: the reference recalculates
`055`'s thirteen `<f ca="1">DATE(YEAR(TODAY()),m,d)</f>` cells, scales its date axis on the union
of the recalculated range and the import-time cache, and therefore draws **23 extra axis labels**
worth +111 characters. The 85 was `111 − 26`; taking the 26 away leaves the 111 standing alone.

**The evidence that this is right does not depend on that number.** Round 112's two frozen
variants remove the volatile formulas and change nothing else:

| variant | 26.2.4.2 | this tree, base | this tree, after |
|---|---:|---:|---:|
| `055_static2023` | 934 | 960 | **934** |
| `055_static2026` | 934 | 960 | **934** |

Both pages, both variants, **exact**. That is the reference's own count on the same document with
the one confound frozen out, and it is what the −26 buys.

---

## 6. Tests

Two new classes in `tests/XRay.Office.Core.Tests`, nine cases, **five of which fail at the base**
(checked by reverting the three source files, rebuilding and re-running: `Failed 5, Passed 4`).

| case | what it holds | fails at base |
|---|---|---|
| `ChartIntervalCapAreaTests.WideningTheWidestCategoryLabelDoesNotChangeTheInterval` | §1.2's `w8i3` as a unit test | **yes** |
| `…WideningTheLastCategoryDoesNotChangeItEither` | `w8i4`, the other half of the reset | no |
| `…WideningACategoryInsideTheMeasuredSetDoesChangeIt` | `w8i2`, the control | no |
| `…TheLongestCategoryIsTheOneWithTheMostCharactersAndNotTheWidestOne` | §1.4, `026`'s regression pinned | **yes** |
| `…AStatedInnerRectangleIsCappedOnItself` | the `mbUseFixedInnerSize` arm the scope must not cross | no |
| `ChartCategoryNameFieldTests.ACategoryNameFieldIsWrittenThroughTheAxisFormat` | §3.2's control row | **yes** |
| `…ItFollowsTheAxisFormatWhenThatChanges` | `axis_yyyy` and `axis_hash` | **yes** |
| `…TheCachedCellTextDecidesNothing` | `cell_yyyy` / `cell_hash` | **yes** |
| `…AStringCategoryIsDrawnAsItStands` | the branch a string category takes | no |

**Totals, one project at a time, `--no-build` against this diff's own build:**

```
Containers 109   Core 588   Markup 259   OpenDocument 160   Presentations 1180
Rendering  164   Spreadsheets 1352   Text 728   Vector 309   WordProcessing 1938
                                                   6787 passed, 0 failed, 0 skipped
```

Every one of the ten is at or above round 112's, so none is a truncated run; Core is +9, which is
this round's nine, and Presentations +38 comes from the slides round merged since.

**`XRay.Office.Fidelity.Tests`, the one that can skip silently: `Failed: 10, Passed: 542,
Skipped: 0, Total: 552`** — 0 skipped, so it covered everything, and the ten are exactly round
112's, none of them a chart:

```
TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop    x4
PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt x4
SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt x1
JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt x1
```

---

## 7. What is open, and why

* **O56 — the turned value label's right-hand overhang.** §1.6. `027`'s axis is 7.51 pt short at
  the right end because `PlotAreaOf` reserves half the rotated label where the reference reserves
  about a tenth of it. One document known; it moves no character and no page, so no gate column
  can see it.
* **The vertical branch of the cap.** §2. Measured and left: the two readings differ by at most
  half a label's height and no observable this round could build separates them.
* **A numeric category on a `c:catAx` rather than a `c:dateAx`.** §3.3 writes the field from the
  date axis' serials because the resolver has already turned everything else into text. A numeric
  category axis stating its own format would be the same defect; **0 of the 307 corpus chart parts
  is that shape**, so it is not implemented and the census is the reason.
* **`055`'s remaining 111 characters.** C12, unchanged, and still not a chart question — it needs
  `TODAY()` evaluated *and* the import-time cache kept beside it. **O49 / NOT WORK.**
* **A page that deserves an independent reader.** None this round: both findings are decided by
  counts read out of two PDFs and by one-attribute variants at the reference, and neither rests on
  a visual reading. Blind readings are not available in this container (established three times);
  no conclusion here needs one.

## 8. Files

* `results.md` — this.
* `cat-set-027.py` — the eighteen one-category variants of `027`'s savings chart, and the reader.
* `cat.tsv` — what 26.2.4.2 drew for them.
* `colcap.py` — the column-chart probe of §2, over `038`'s frame.
* `catname-055.py` — the seven one-attribute variants of `055`'s axis and cell formats.
* `catname.tsv` — the span-level diff of each against the control.
* `catname-census.py`, `catname-census.tsv` — the category-name-label census and its per-document rows.
* `reach.tsv` — the 176 chart-bearing documents at three commits, with the mover attribution.
* `odf-reach.tsv` — the 182 chart-bearing converted-ODF files at both ends.
