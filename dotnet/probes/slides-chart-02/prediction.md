# `slides/chart-001` — prediction, written before any sweep

Written after the code was changed and **before** the corpus sweeps, the regression run, the
whole test suite and the blind review. Committed first so it can be scored honestly.

The group is two documents:

- `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx`
- `southern-classic-kennesaw-state-university-final.pptx`

## What was changed

Six behaviours, all in the chart path.

| # | change | where |
|---|---|---|
| 1 | `c:dateAx` resolves a real date scale, and the points are sorted into date order with it | `DrawingChartPlot`, `ChartDateScale.SortByDate` (moved down from `XlsChartReader`) |
| 2 | `a:prstDash` on a series reaches the stroke | `DrawingChartPlot.DashOf`, `DashPresets` (moved from `XRay.Office.Presentations` into Core) |
| 3 | a line series' legend key is a line sample, dashed where the series is, and 1600 wide when it is | `ChartLayout.AddLegend`, `LineKeyWidth` |
| 4 | `percentStacked` normalises each category by its own total and fixes the axis at 0–1 in ten steps | `ChartPlot.StackTotal`, `ChartLayout.AddBars`/`AddAreas`/`PercentAxis` |
| 5 | a chart part's `themeOverride` wins over the deck's theme | `PptxSlideLayout.ChartTheme` |
| 6 | the `chartUserShapes` part is read and its shapes drawn over the chart | `PptxSlideLayout.UserShapes` |

Three things named in the brief were **deliberately not** attempted, and the prediction says so
in advance rather than after the fact: the legend's `c:manualLayout` (which is why the reference
shows three legend entries and we show four), the value-axis automatic interval on page 10
(`$370/$420/$470/$520` against `$360/$400/$440/$480`), and the chart-area / plot-area automatic
fills and gradient bar fills.

## Claims

### A. Neither gate verdict flips

1. **`8_P-Pavese`** stays `pages 26/26`, verdict `words`, and its word column gets **worse**, not
   better — exactly as `slides-b008-01/results.md` predicted of any real fix here. Ours was 2118
   against a reference 2010. I predict **2122–2130** after this round, i.e. between +112 and +120.
   The movement is two additions and nothing else:
   - the four `chartUserShapes` labels — `88%`, `(548/621)`, `72%`, `(317/439)` — **+4**;
   - the value axis going from eight ticks (`0%`, `10000%` … `70000%`) to eleven (`0%` … `100%`)
     — **+3**.
2. **`southern-classic`** does not flip either. Its page count was already exact and I predict the
   word column moves by **0 to ±3**: nothing this round adds or removes text on it except at the
   margins of the date axis' label thinning.
3. So the group reads **0 of 2 passing before and 0 of 2 after**. That is the expected result and
   is not a failure of the round.

### B. Reach across the 163-document slides track

Measured by rendering all 163 twice — once with a CLI built from a pristine `git archive HEAD`
tree and once with this one — with `SOURCE_DATE_EPOCH` set, and diffing the PDFs byte for byte.

4. I predict **18 to 40 of 163** documents change. The changes compose: a document is touched if
   it has any chart with a resolvable `c:dateAx`, any chart series naming an `a:prstDash`, any
   line/scatter/radar series with a legend, any `percentStacked` group, a `themeOverride`, or a
   `chartUserShapes` part.
5. **The legend key is the widest of the six** and I expect it to account for more than half the
   moved documents on its own, because every line chart with a legend has one and line charts are
   common. If the count comes in under 10 the legend change did not reach, and that is the first
   thing to check.
6. The other five are narrow. `themeOverride` and `chartUserShapes` are **4 documents each** by
   package inspection (`8_P-Pavese`, `RPA P4 - Advanced Material`, `171128IPAP`, `3492` for the
   first; `8_P-Pavese`, `RPA P4`, `171128IPAP`, `bitesize-writing-a-report` for the second), and
   at most those 5 distinct documents can move from them.
7. The change is in `XRay.Office.Ooxml`, so it reaches `.xlsx` and `.docx` charts too. I predict
   the sheets and words tracks would also move if swept, and I am **not** sweeping them; the
   Fidelity suite is the control that says nothing broke there.

### C. Regression

8. Every `slides/done-*` group — 29 documents — still matches: **29 of 29**, no verdict flips
   either way. This is the claim I am least confident about, because the legend key change alters
   the plot rectangle on any line chart whose key was previously 800 wide and is now 1600, which
   moves every bar and label on that chart. If a `done-*` document regresses, this is why.

### D. Tests

9. `XRay.Office.Fidelity.Tests` reproduces **30 failed of 550, 0 skipped**, both on the pristine
   HEAD tree and on this one. The baseline is established on HEAD first so the two are comparable.
10. Every other project stays at **0 failed**, with the counts `slides-b008-01/results.md` records
    — Containers 109, Core 332 (+5 new), Markup 259, OpenDocument 125, Presentations 679 (+8 new),
    Rendering 150 passed and 1 skipped, Spreadsheets 762, Text 349, Vector 295, WordProcessing 819.
11. The 13 new tests: **10 fail against the unfixed behaviour and 3 are negative controls that
    must pass either way** (an automatic date axis over plain numbers, a plain stack, a bar
    series' box key). Already run and recorded; scored here for the record.

### E. What the blind reviewer will say

12. Sent `southern-classic` page 2 as a labelled pair with no numbers and no repository access,
    a fresh reviewer will report the two halves' price curves as **the same shape and the same
    direction** — the mirror image is gone — and the three threshold lines as **dotted in both**.
13. It will still find the legend different: **three entries in the reference and four in ours**,
    with the reference's third wrapping onto two lines. That is the `c:manualLayout` I did not
    implement and I expect it to be reported as the largest remaining difference on that page.
14. Sent `8_P-Pavese` page 8, it will report the reference's chart as sitting on a **black or very
    dark panel** that ours does not draw, and will report the axis as reading **0% to 100% in both
    halves**. It may or may not resolve the serif/sans difference now that the theme override is
    applied; I predict it reports the two as the **same** face, or does not mention the face at all.

### F. What could go wrong, named in advance

15. The date-axis sort permutes values but deliberately **not** per-point labels or per-point
    fills, on the reading that `VDataSeries::doSortByXValues` sorts the value sequences and
    `getDataPointProperties` is still asked by model index. If that reading is wrong,
    `southern-classic` page 2's three threshold labels will be at the *left* end of the reference
    and the right end of ours. Measured before the change they were at x≈557 in ours and x≈549 in
    the reference, so I expect them to stay put; if they move, this is the reason.
16. `PercentAxis` fixes the scale at 0–1 for any percent-stacked chart. A percent stack whose
    file states a `c:max` still wins, but one whose data legitimately exceed the total — which
    cannot happen — would clip. Low risk, stated for completeness.
17. Moving `SlideDashes` into Core as `DashPresets` is a pure rename plus namespace move with two
    call sites. If a table border's dash changes anywhere, the move was not pure.
