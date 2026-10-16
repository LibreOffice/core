# `slides/chart-001` — eight chart defects fixed, three left, and what moved

Group: `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` and
`southern-classic-kennesaw-state-university-final.pptx`.

**Neither gate verdict flipped, and that is the expected result rather than a failure.** Both
documents were classified `chart` because their word columns are measurement artefacts while
their pages are genuinely wrong; `slides-b008-01/results.md` had already measured that fixing
`8_P-Pavese`'s real defects makes its word column *worse*. It does — from +108 to +112 — and
every page that moved, on every document that moved, moved **towards** the reference.

The useful figures are not in the verdict column:

| | before | after |
|---|---:|---:|
| `8_P-Pavese` page 8, our words against the reference's | **25 / 32** | **32 / 32** |
| `171128IPAP` pages 34, 36, 37, 38, 39, our surplus over the reference | +55, +55, +72, −3, −2 | **0, 0, 0, 0, 0** |
| `171128IPAP` page 40 | +25 | +2 |
| `RPA P4 - Advanced Material` page 10 | −15 | −5 |
| `bitesize-writing-a-report` page 12 | −5 | −1 |

Measured at `4f5ded8fb00` (the prediction commit) plus this round's code. One caveat on that
commit: `git commit` picked up a `git mv` that was already staged, so it also carries the
`SlideDashes` → `DashPresets` file move without the namespace edit that goes with it. That commit
therefore does not build on its own and the pair has to be read together; `prediction.md` itself
is untouched, which is the property that matters, so it was left rather than rewritten.

Reference: the banked
`26.2.4.2` renderings at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides/`, with
`SOURCE_DATE_EPOCH=1755000000` on both halves.

## What was fixed

Eight behaviours. The first six were planned from the brief; the last two came out of the blind
reviews and are the reason those reviews are worth the round they cost.

| # | change | where |
|---|---|---|
| 1 | **`c:dateAx` resolves a date scale**, and the points are sorted into date order with it | `DrawingChartPlot.DateAxisOf`, `ChartDateScale.SortByDate` |
| 2 | **`a:prstDash` on a series** reaches the stroke | `DrawingChartPlot.DashOf`, `DashPresets` |
| 3 | **a line series' legend key is a line sample**, dashed where the series is and 1600 wide when it is | `ChartLayout.AddLegend`, `LineKeyWidth` |
| 4 | **`percentStacked` normalises** each category by its own total, and the axis is fixed at 0–1 in ten steps | `ChartPlot.StackTotal`, `ChartLayout.PercentAxis` |
| 5 | **a chart part's `themeOverride`** wins over the deck's theme | `PptxSlideLayout.ChartTheme` |
| 6 | **the `chartUserShapes` part** is read and drawn over the chart | `PptxSlideLayout.UserShapes` |
| 7 | **`a:ln/@cap`** reaches the stroke, so a dotted line is dots | `DrawingChartPlot.CapOf` |
| 8 | **`a:noFill` on a series** suppresses the automatic fill instead of being read as "unstated" | `DrawingChartPlot.SuppressesFill` |

### 1. The reversed axis, which was the brief's most serious item

`southern-classic`'s page-2 chart stores its 254 daily closes **newest first** — 12 January 2017
down to 12 January 2016 — under a `c:dateAx`. The reader knew `dateAx` only as a spelling of
`catAx`, so the points were plotted in cell order and the whole price series was drawn mirrored:
the reference's axis reads `Jan-16 … Jan-17` and ours read `Jan-17 … Jan-16`.

Everything needed was already in the tree and unreachable from this vocabulary.
`Core/Charts/ChartDateScale` had the whole scale, the tick generation and the calendar
roll-over; `XlsChartReader` had the sort. Only the binary `.xls` reader ever built a
`ChartDateAxis`. So the fix is a reader-side resolution plus a move: `SortByDate` came down into
`ChartDateScale` and `XlsChartReader` now calls it, which is one implementation instead of two.

The resolution rule is the one the `.xls` reader already applies, reached from the other
vocabulary: a `c:dateAx` is a date scale when `c:auto` is `0`, or when it is automatic and the
categories' number format is a date format. chart2 asks
`ExplicitCategoriesProvider::isDateAxis` before it uses the scale, and an automatic date axis
over a column of plain numbers is drawn as a category axis. Both halves are pinned by tests.

**It is worth more on a document nobody was looking at than on the one it was found on.**
`171128IPAP.pptx` — in `slides/done-011`, passing the gate throughout — has four `c:dateAx`
charts on slides 36, 37, 38 and 40. We drew one label per *category* there, thinned by the
collision ladder; the reference draws date *ticks*. Those four pages carried +55, +55, +72 and
+25 words of surplus. Three of them are now exact and the fourth is +2.

### 4. The percent stack, and why the axis read 70000%

`8_P-Pavese`'s chart is `c:grouping val="percentStacked"` over raw 548/73 and 317/122, with a
`0%` format on its value axis. `IsStacked` was set for it, nothing normalised it, so the axis
auto-scaled to the raw total of 621 and *then* applied the percent format: **eight ticks reading
`0%, 10000% … 70000%`** against the reference's eleven reading `0% … 100%`.

Two separate things were wrong and both are needed:

- **The geometry.** Each category is divided by the sum of its own absolute values. Applied where
  the bars are computed and not to the stored values, so a `c:showVal` label still reads 548.
- **The scale.** A percent axis is 0 to 1 in ten steps and is not derived from the data at all.
  Normalising alone gives a maximum of exactly 1, which the automatic rule rounds up to 1.2 and
  steps by 0.2 — six ticks reading `0% … 120%`. A stated `c:min`/`c:max`/`c:majorUnit` still wins.

The bars now land where the reference's do. In the PDF operators, our first column is
`(137.34, 85.44)-(320.27, 342.87)` against the reference's `(137.31, 85.55)-(320.26, 342.94)`,
and the second column's top is `296.09` against `296.16` — **0.07 pt**, which is 317/439 to four
figures.

### 6 and 5. The four missing tokens, and the serif face

`chart1.xml.rels` carries a `chartUserShapes` relationship to `ppt/drawings/drawing1.xml`, whose
entire text content is `88%`, `(548/621)`, `72%`, `(317/439)` — every token the reference draws
on page 8 that we did not, and none of them anywhere else in the package. We did not read the
part at all.

They are read by rewriting each `cdr:sp` into the `p:sp` the shape walk already understands: only
the three `cdr` wrappers carry that namespace and everything inside them is DrawingML already, so
the rewrite is three elements deep and every shape feature keeps working. **The anchor places the
shape and its own `a:xfrm` does not** — the `a:off` inside `cdr:spPr` is in the coordinate space
of whatever document the chart came from, while `cdr:from`/`cdr:to` are fractions of this frame.

Page 8 is now **32 words against the reference's 32**, exactly.

The same rels part carries a `themeOverride` whose `minorFont` is Palatino Linotype. A blind
reviewer had named that cause from the picture alone, with no source access, in the previous
round. It is now applied, and the operator census agrees: the chart's title, value-axis labels
and category labels resolve to `DejaVuSerif` in both halves, and this round's blind reviewer —
also given nothing but the image — reported the category-label glyph runs starting at *exactly*
the same 13 x positions in both, "same face, same size, same position — only the colour differs".

### 7 and 8. Two the blind reviews found

Both were found by reviewers who had never seen the document, could not read the repository, and
were given no numbers.

**The dash was the right pattern drawn with the wrong cap.** Sent `southern-classic` page 2, the
reviewer reported the reference's threshold lines as "clearly round dots" and ours as "narrow,
tall, rectangular dashes with wide gaps", and named the line cap as its first candidate —
suggesting the exact measurement that would settle it. The operator census then settled it:
the reference emits `1 J` beside every `[0.03 5.97] 0 d`, and we emitted `0 J` for all 893 of our
strokes. `DashPresets` had already taken 99% off each ink length because a round cap is measured
*inside* the dash, so without the cap the array draws hairline rectangles. We now emit 27 `1 J`
against the reference's 28, and every `[0.03 5.97]` array is paired with one.

**A series stating `a:noFill` was being given the automatic colour.** Sent `8_P-Pavese` page 8,
the reviewer reported that ours caps each column with a silver block reaching 100% where the
reference shows only the plot background, and listed "drawn but with a fill equal to the
background" among the causes it could not separate. The file settles it: that series, `Non
suivi`, is `<c:spPr><a:noFill/></c:spPr>` and nothing else. `FillOf` returned null for both "no
fill" and "nothing stated", and the `?? autoFill` behind it then substituted accent 2. This is
exactly the distinction `SuppressesLine` already drew one element up; `SuppressesFill` is its
twin.

## What was deliberately **not** fixed

Named here rather than left for the next reader to re-derive. Each is real, each was measured,
and none is a half-measure of something above.

1. **The legend's `c:manualLayout`** — the "Optimistic" entry. `southern-classic`'s legend states
   `x=0.834 y=0.528 w=0.166 h=0.320` of the frame. LibreOffice honours it: three entries, two of
   them wrapped mid-word (`Pessimisti` / `c`), and no fourth. We ignore it and lay out four
   unwrapped entries. The reviewer ranked this second among the page's differences and observed
   that ~60 px of empty space remains below `Baseline`, which argues against simple vertical
   clipping — so the mechanism is not settled either. Implementing it means legend text wrapping
   and a manual legend rectangle, and it changes the plot rectangle on every chart that states one.
2. **The value axis' automatic interval**, page 10. The file states `c:min val="320"` and no
   maximum over data peaking at 468. The reference ticks `320/360/400/440/480`; we tick
   `320/370/420/470/520`. The difference is the automatic maximum and step, i.e.
   `ChartScale`'s core rule, which every chart in the corpus goes through. Changing it is a
   corpus-wide cascade and it is not a chart-reader question.
3. **The chart-area and plot-area automatic fills, and gradient bar fills.** Confirmed in the
   operators, not merely in a raster: the reference draws `#000000` over `(0, 36)-(720, 427)` and
   `#454545` over `(69, 86)-(709, 377)`, and paints each bar as ~30 stepped fills from `#FEFEFE`
   to `#DDDDDD`. We draw neither fill and one flat colour per bar. Two separate bodies of work:
   `DrawingChartAutoFormat` has the series tables and none of the `spChartSpaceFills` /
   `spPlotArea2dFills` ones, and `ChartBox.Fill` is a `Colour?` so a gradient needs the chart IR
   to carry a `Paint`.

Two more found this round and left, both from the reviews:

4. **Axis tick marks and the axis line.** On `southern-classic` page 2 the reference draws no
   value-axis line and no ticks — the file says `c:majorTickMark val="none"` and `a:ln/a:noFill` —
   and we draw a black axis and a tick at every mark. `ChartLayout` has one `AxisColour` constant
   and no per-axis line or tick-mark properties at all. **Fix 1 made this worse on that chart**,
   honestly: the category axis went from ~254 category ticks to 367 daily ones, which the reviewer
   described as "a dense comb … visually a prominent black hatched band". It is the largest
   remaining visible difference on the page and it is the next thing to do here.
5. **The `chartUserShapes` default face.** Their runs name no typeface. The reference draws them
   in `LiberationSerif-Bold` and we draw them in `DejaVuSerif-Bold`, ~30% wider, which wraps
   `(548/621)` onto two lines. The chart's own text agrees exactly in both, so this is specific to
   the drawing part inheriting the chart's theme font where LibreOffice gives it the drawing
   layer's default. Not guessed at, because guessing a default face is how a whole class of text
   gets quietly moved.

## Sweeps

`sweep-banked.sh` in this directory (`batch-check.sh`'s three checks and verdict rule verbatim;
only the reference's source differs). Run twice over the whole slides track: once with a CLI
built from a pristine `git archive HEAD` tree, once with this one.

| | total | match | mismatch |
|---|---:|---:|---:|
| pristine HEAD | 163 | **144** | 19 |
| this tree | 163 | **144** | 19 |

**The same 19 documents, and no verdict moved in either direction.** All **144** documents in
`slides/done-001` … `done-015` match on both runs — the regression check, and it is clean.

The two group documents:

| document | pages | words before | words after | reference | verdict |
|---|---|---:|---:|---:|---|
| `8_P-Pavese` | 26/26 | 2118 | **2122** | 2010 | `words`, unchanged |
| `southern-classic` | 23/23 | 2217 | **2217** | 2270 | `words`, unchanged |

`8_P-Pavese`'s +4 is two pages moving in opposite directions and both towards the reference:
page 8 **+7** (25 → 32 against 32, exact) and page 16 **−3** (221 → 218 against 183). Page 16 is
the outlined-glyph ceiling `slides-b008-01` recorded; it is +35 now where it was +38.

## Reach: 9 of 163

Measured by byte-diffing the two sweeps' PDFs, not by grepping for markup. Nine documents render
differently; 154 are byte-identical.

Attributed by turning each change off on its own, rebuilding, and re-rendering the nine
(`attribute.py`, kept in this directory):

| change | documents it moves |
|---|---|
| `legend-line-key` | 171128IPAP, 1_Country-Updates_DRC_English, Demick_JetBlue, RPA P4, southern-classic |
| `line-cap` | 171128IPAP, 1_Country-Updates_DRC_English, RPA P4, flying-by-numbers-presentation, southern-classic |
| `theme-override` | 171128IPAP, 3492, 8_P-Pavese, RPA P4 |
| `user-shapes` | 171128IPAP, 8_P-Pavese, RPA P4, bitesize-writing-a-report |
| `date-axis` | 171128IPAP, 8_P-Pavese, southern-classic |
| `dash` | RPA P4, southern-classic |
| `percent-stacked` | 3492, 8_P-Pavese |
| `series-nofill` | 8_P-Pavese, southern-classic |

Six of the nine changed their word count and **all six moved closer to the reference**; the other
three (3492, Demick_JetBlue, flying-by-numbers) changed ink only.

The change is in `XRay.Office.Ooxml` and `XRay.Office.Core`, so it reaches `.xlsx` and `.docx` charts
too. Those tracks were not swept; `XRay.Office.Fidelity.Tests` is the control and it did not move.

## Verification: the blind reviews

Two fresh subagents, one page each, given the composed pair and nothing else — no numbers, no
brief, and forbidden to read any file but the image or to run any command.

**They confirmed the two headline fixes.** On `southern-classic` page 2: *"Both axes run in the
same direction … category axis runs Jan-16 on the left → Jan-17 on the right"*, and *"the
closing-price line traces the same path in both — same start near $47, same trough near $33 at the
same horizontal position, same December peak"*. On `8_P-Pavese` page 8: *"0% to 100% in 10% steps
in both"*, bar tops at *"88.2%/88.3% and 72.3%/72.3%"*, and *"Bar geometry, pixel for pixel"*.

**They found two defects I had not, both now fixed** — the line cap and the `a:noFill` series,
above. Neither is visible in any number the gate produces and neither had been recorded before.

**They reported the three deliberate omissions independently**, which is the check that the
omissions are real and not rationalisations: the legend's missing fourth entry, the black chart
panel and gradient bars, and the axis-tick comb.

Both were careful about the limit of the instrument, and one of them usefully so: *"anything black
inside the bottom chart is invisible against the black chart background … 'not visible' is not the
same as 'not drawn', and I have not claimed otherwise"*. Every absence claimed in this file is
confirmed in the PDF's operators rather than in a raster, for exactly that reason.

## Tests

Baseline established on the pristine `HEAD` tree **first**, so the two are comparable. Counts,
not colours, run per project and totalled by hand.

| project | failed | passed | skipped | total | vs pristine HEAD |
|---|---:|---:|---:|---:|---|
| `XRay.Office.Containers.Tests` | 0 | 109 | 0 | 109 | — |
| `XRay.Office.Core.Tests` | 0 | 337 | 0 | 337 | +5 new |
| `XRay.Office.Fidelity.Tests` | **30** | 520 | **0** | **550** | identical |
| `XRay.Office.Markup.Tests` | 0 | 259 | 0 | 259 | — |
| `XRay.Office.OpenDocument.Tests` | 0 | 125 | 0 | 125 | — |
| `XRay.Office.Presentations.Tests` | 0 | 688 | 0 | 688 | +9 new |
| `XRay.Office.Rendering.Tests` | 0 | 150 | 1 | 151 | — |
| `XRay.Office.Spreadsheets.Tests` | 0 | 770 | 0 | 770 | — |
| `XRay.Office.Text.Tests` | 0 | 349 | 0 | 349 | — |
| `XRay.Office.Vector.Tests` | 0 | 295 | 0 | 295 | — |
| `XRay.Office.WordProcessing.Tests` | 0 | 827 | 0 | 827 | — |
| **total** | **30** | **4429** | **1** | **4460** | |

`XRay.Office.Fidelity.Tests` is **30 failed of 550, 0 skipped** on the pristine tree and identical
here — the briefed baseline, established rather than assumed.

**A correction to the counts `slides-b008-01/results.md` records.** It gives Spreadsheets 762 and
WordProcessing 819; the pristine HEAD tree gives **770 and 827**. That is drift on the branch base
between the two rounds, not movement caused by this change — both figures were taken from the
`git archive HEAD` tree, which contains none of it.

### The 14 new tests, and that they fail without the fixes

`DrawingChartDateAxisTests` (9) and `ChartPercentAndLineKeyTests` (5). Verified by neutering the
eight behaviours in place — keeping the API so the assertions still compile and the failures are
about behaviour rather than about a missing member — and re-running:

**11 fail against the unfixed behaviour. The other 3 are negative controls that must pass either
way**: an automatic date axis over plain numbers stays a category axis, a plain stack is not
normalised, a bar series' legend key is still a box.

### One thing worth recording about the test host

`XRay.Office.Vector.Tests` reported **16 failed of 295** on one run and **0 failed of 295** on the
four runs immediately after it, with the same binary and no change between them; the failing run
came straight off the back of the Fidelity project. No file under `XRay.Office.Vector` is touched by
this round. `CLAUDE.md` warns that a loaded host makes a run *truncate* and still print success;
this is the same instability with the opposite sign, and it is worth knowing that a red run can be
a flake as well as a green one. Repeat before believing either.

## Prediction, scored

`prediction.md`, committed at `4f5ded8fb00` before any sweep.

| # | claim | outcome |
|---|---|---|
| 1 | `8_P-Pavese` stays 26/26 and `words`, word column worse, **2122–2130**, made of +4 user shapes and +3 ticks | **hit, at the bottom of the range** — 2122. The mechanism is right and the arithmetic is not: page 8 is +7 and page 16 is −3, not +7 alone |
| 2 | `southern-classic` moves 0 to ±3 | **hit** — 0 |
| 3 | 0 of 2 passing before and after | **hit** |
| 4 | reach **18–40** of 163 | **miss** — 9. I predicted composition and got it, and over-estimated how many slides documents have a chart the changes touch at all |
| 5 | the legend key is the widest single change, more than half the moved documents | **hit** — 5 of 9, tied widest with the line cap, which did not exist when the prediction was written. The stated falsifier ("under 10 means it did not reach") was **wrong reasoning**: reach was 9 and it did reach |
| 6 | `themeOverride` 4 documents, `chartUserShapes` 4, at most 5 distinct | **hit exactly**, all three numbers and all five names |
| 7 | the sheets and words tracks would also move; Fidelity is the control | **consistent** — Fidelity unchanged at 30/550 |
| 8 | every `done-*` document still matches — "29 of 29" | **hit on substance, miss on the count.** `slides/done-*` holds **144** documents, not 29; I counted directory entries rather than files. 144 of 144 match |
| 9 | Fidelity 30/550/0 skipped on both trees | **hit** |
| 10 | every other project 0 failed, at the counts `slides-b008-01` records | **half.** 0 failed everywhere, but two of the recited counts were stale by 8 each — see the correction above. And one run of Vector was red and not reproducible |
| 11 | 10 of 13 new tests fail unfixed, 3 controls | **hit in spirit** — 11 of 14 after a fourteenth test was added for a defect found later; the 3 controls are the 3 predicted |
| 12 | a blind reviewer reports the curves the same shape and direction, and the thresholds dotted in both | **hit on direction, half on the dash.** Dotted in both — and the *wrong dots*, which is what produced fix 7 |
| 13 | it will still report 3 legend entries against our 4 as the largest remaining difference | **hit**, ranked second of eight rather than first |
| 14 | `8_P-Pavese`'s reviewer reports a black panel we do not draw, 0–100% in both, and the same face | **hit on all three** |
| 15 | the three threshold labels stay put; if they move, the sort permutes too much | **hit** — x≈557 ours against 549 the reference's, before and after |
| 16 | `PercentAxis` clipping risk | not triggered |
| 17 | the `SlideDashes` → `DashPresets` move is pure | **hit** — no document moved for any reason but the eight changes, and no table border anywhere |

**Twelve of seventeen, and the two misses that matter share a shape.** Claims 4 and 8 are both
counts of *how many documents are in scope*, and both were guessed from the wrong quantity — the
first from an intuition about how common charts are, the second from `ls` output that counts
directories. The generalisable form: **a reach or a regression claim is a count over the corpus,
and a count over the corpus should be counted, not estimated.** The claim that held up best,
number 6, is the one where I had actually opened the packages first.

The other lesson is claim 12's half. I predicted "dotted in both" and would have written the
round up as a clean hit on the dash, because *dotted* is what a metric would have told me. The
reviewer said dotted **and the wrong shape**, which is a defect no number in this file could have
surfaced and which took one line to fix once named.
