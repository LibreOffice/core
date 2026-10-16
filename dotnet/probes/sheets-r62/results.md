# Round 62 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r62`, base
`337bc9fe17c`. Read `prediction.md` (`9cb107edf55`) beside this file first — it was committed
before a line of behavioural code was written and before anything was rendered post-change.

**Sweeps ran with `TMPDIR` on `/c/sandbox/workdir`, and `verify-test.sh` was run only between
sweeps, never beside one.** A document re-rendered after the final sweep is **byte-identical** to
that sweep's copy.

## 1. Baseline: **282 of 307**, and the one above the brief moved on the reference's side

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 363 MATCH 308 MISMATCH 55
REF-CANNOT-RENDER 0`; scored by round 58's `score.py`, which refuses to print unless every one of
`MANIFEST.tsv`'s 307 sheets paths found a row: **282 match, 20 `words`, 5 `pages,words`**.

The brief says 281. The extra one is `unstable-001/xlsx/fse_identification_form.xlsx`, and **ours
did not move**: 440 words in round 61's sweep, in this baseline, and in this round's final sweep.
The reference went 427 → 440 → 427 across the three. Four other rows moved on the reference's side
only and changed no verdict. `ans_mappings_of_eccairs_terms.xlsx` matched in every sweep.

## 2. Result: **282 → 280**. The named item closed exactly; one verdict lost on our side, one on
the reference's

> **This section was written at 279 and re-measured at 280.** § 11a traced both regressions to a
> *calibration* this round invalidated rather than to a defect, and § 11b acts on that: restoring
> the wrap limit's units brings `023_Waterfall` back to `match` and takes `058` from 100 to 200
> against a reference of 194. The tables below are the final sweep, `final2/parity.tsv`. Where a
> number appears as "279" it is the intermediate state and is labelled.

| document | before | after | which side |
|---|---|---|---|
| `chartset-002/xlsx/003_advanced_excel_pie.xlsx` | `match` **145**/143 | `match` **143/143** | ours, exact |
| `chartset-002/xlsx/011_advanced_excel_pie.xlsx` | `match` **142**/140 | `match` **140/140** | ours, exact |
| `chartset-002/xlsx/027_advanced_excel_pie.xlsx` | `match` **142**/140 | `match` **140/140** | ours, exact |
| `chartset-004/xlsx/019_advanced_excel_pie.xlsx` | `match` **142**/140 | `match` **140/140** | ours, exact |
| `chartset-008/xlsx/023_Waterfall_Chart_Template_for_Excel` | `match` 881/868 | `match` **872**/868 | ours, and it went through `words` 843 first — § 11a/b |
| `chartset-008/xlsx/046_Cost_analysis_with_Pareto_chart` | `match` 159/157 | **`words` 161/157** | **ours — a regression** |
| `unstable-001/xlsx/fse_identification_form.xlsx` | `match` 440/440 | `words` 440/**427** | **the reference** |

Other rows that moved without changing a verdict, ours: `058_Social_media_engagement_data`
206 → **200** against a reference of 194 (`words` either way, and closer than it started; it read
100 in the intermediate state — § 5),
`055_Project_timeline` 269 → 262, `029_Annual_budget` 315 → 310, `001_Contextures` 883 → 881,
`008_Contextures` 176 → **175/175 exact**, `033_Event_planning` 496 → 497, `064_Small_business`
508 → 507, `TOGAF9-Tool-ConfReqts-CSQ` 23620 → 23617, `Template Pilot Logbook` 1587 → 1588. **No
page count changed on our side anywhere in the track.**

**The four pies come out word-exact, which is what round 61's one miss was about.** They were two
words over on all four; they are now zero.

## 3. The brief's item 1 is a real observation with the wrong seat, and the seat is measurable

The brief: *"the inner-fit test rejects one label of five the reference keeps"*. The observation is
right. The diagnosis is not: **the predicate is a faithful port and its input was 1.7 pt too wide.**

`probe-labelfit.py` reads both renderings' page 1 — nothing of ours runs inside it, both sides are
read off rendered PDFs — and before the change gave:

```
ours  M3; Actual; 107; 20%   lines 1  box 86.62 x 12.21  centre (486.39, 358.10)  d 131.93 outside
ref   M3; Actual; 107; 20%   lines 2  box 65.43 x 23.43  centre (438.17, 400.95)  d  70.21 inside
```

Fed **our** block, the port answers `FAIL(CM 36.33 > 36.00)` — it misses by **0.33 of a degree**.
Fed the reference's own box it fits. The threshold at radius 100.01 and height 22.5 is a block
width of **75.23**; ours was **75.90** and the reference's is **74.25**.

### Where the 1.7 pt comes from, read off the reference at fourteen sizes

Round 60 put a chart's *vertical* metrics through `chart2`'s own **96 dpi** device
(`MetricGrid.Chart`) and left the **advance width** on the face's unquantised metrics. 26.2.4.2
instantiates the em at a whole number of device pixels, so at 10 pt it sets **13** for 13.333 and
every advance comes back 2.5% narrower — and at 11 pt it sets **15** for 14.667 and they come back
2.3% **wider**.

`probe-chartwidth.py` renders fourteen one-variable rewrites of `003`'s own chart part and reads
the drawn advance out of the reference's own `TJ` arrays. Those per-glyph adjustments are in
thousandths of the text em, so the measurement is **scale-free**: independent of the chart frame,
the page, and the size the PDF writer chose. **Our renderer never runs.**

```
stated  drawn    measured  ppem/px     predicted residual
 6.00    5.993    0.99611    8/7.991    1.00117   -0.00506
 8.00    8.000    1.02799   11/10.667   1.03125   -0.00326
 9.00    8.990    0.99873   12/11.987   1.00111   -0.00238
10.00   10.008    0.97344   13/13.344   0.97422   -0.00078
11.00   10.997    1.01867   15/14.663   1.02301   -0.00434
12.00   11.987    0.99780   16/15.983   1.00108   -0.00328
13.00   13.004    0.97897   17/17.339   0.98047   -0.00150
14.00   13.994    1.01425   19/18.659   1.01829   -0.00404
16.00   15.888    0.98932   21/21.184   0.99131   -0.00200
18.00   18.008    0.99861   24/24.011   0.99956   -0.00095
20.00   19.987    1.00874   27/26.649   1.01316   -0.00442
22.00   21.909    0.99172   29/29.212   0.99274   -0.00102
28.00   27.903    0.99316   37/37.204   0.99452   -0.00136
36.00   23.991    1.00075   32/31.988   1.00038   +0.00038
```

**A sawtooth in the size is a signature nothing but a pixel em produces.** It goes above one at 8,
11, 14 and 20 pt and below it at 10, 13, 16, 22 and 28, in the right places and by the right
amounts, with no free parameter. The residual is ≤0.005 and almost always negative, which is the
estimator's own known bias — it takes the modal adjustment per glyph code and a real kern pair
narrows one occurrence. The 36 pt row is a control that arrived by accident and is worth keeping:
the reference clamped it to 23.991 pt and the law still holds *at the size it drew*.

### What that predicts, and what it did

Predicted before the change, from the reference's law alone: `M3`'s block falls to 74.18 and lands
inside the slice with its text centre at (438.15, 400.80). Measured after:

| label | our box | ref box | our text centre | ref text centre |
|---|---:|---:|---|---|
| `M1` (1 line, outside) | **79.69** | 79.66 | (511.55, 559.56) | (511.54, 559.25) |
| `M2` | **65.46** | 65.43 | (472.39, 460.87) | (472.39, 460.80) |
| `M3` | **65.46** | 65.43 | (438.10, **400.85**) | (438.17, **400.95**) |
| `M4` | **65.46** | 65.43 | (357.32, 446.66) | (357.48, 446.60) |
| `M5` | **65.46** | 65.43 | (362.48, 495.99) | (362.63, 495.85) |

**Every one of the five boxes is within 0.03 pt of the reference's and every centre within 0.16
pt**, where `M3` was 60 pt away. `M1` stays outside and stops wrapping, exactly as the law says it
must (its inner attempt measures 79.48 against an 80.01 allowance).

### And the two surplus words are located, not inferred

The chart straddles the page break and page 2 carries the clipped remainder:

| | page 1 | page 2 | page 3 | total |
|---|---:|---:|---:|---:|
| ours, before | 43 | **20** | 101 | 145 |
| ours, after | 43 | **18** | 101 | 143 |
| reference | 43 | **18** | 101 | 143 |

Ours drew `Actual; 107; 20%` on page 2 where the reference draws `07;` — three tokens against one.
That is the whole of the two-word surplus on all four documents, and page 2 now matches character
for character.

## 4. Prediction against measurement — **10 of 14**, and the miss is the one the prediction named

| | predicted | measured |
|---|---|---|
| sheets verdicts | **282 → 282**, 0 regressions | **280 — WRONG, −2** (one of them the reference's) |
| `003` | 145 → **143**/143 | **143/143 — right** |
| `011` | 142 → **140**/140 | **140/140 — right** |
| `019` | 142 → **140**/140 | **140/140 — right** |
| `027` | 142 → **140**/140 | **140/140 — right** |
| `003` M3 inside, text centre within 1 pt of (438.17, 400.95) | | **(438.10, 400.85) — right, and 0.12** |
| `003` M1 outside, one line | | **right** |
| `003` radius within 1 pt of 100.0 | | **100.01 — right** |
| page counts on our side | 0 | **0 — right** |
| documents whose chart text moves | 97 sheets | **not directly measurable; 20 moved a word count** |
| tests | +6 to +15 | **+16 — just outside, and stated as a range** |
| `MANIFEST.tsv` rows | 0 | **2 — wrong, and both are losses (§ 9)** |
| "most likely to be wrong: a regression among the other 93 chart-bearing documents. I predict 0 and I expect that to be the prediction that fails" | 0 | **2 at first, 1 after § 11b — the prediction failed exactly where it said it would** |
| stated acceptable band | −2 to +1 | **−1 on our side** (the other is the reference's) |

**The round called its own failure and the failure arrived.** That is the only part of this write-up
that is unambiguously good news about the method rather than about the code.

## 5. The two regressions, measured rather than argued

Both are the **axis-label arrangement** — rotation and thinning — and neither document agrees with
the reference either way.

### `046_Cost_analysis_with_Pareto_chart`: the character stream is identical

The charstream test (COMMON.md § 3): strip all whitespace from both `pdftotext` extractions.

```
ours before 754 characters   ours after 754 characters   reference 753
```

Same length, and a `SequenceMatcher` over the two shows nothing but **re-ordering** — every
difference is an interleave of the same letters (`'fits' -> ''`, `'B' -> 'siohins'`, …), which is
what a page of rotated axis labels does to poppler's reading order when the labels move. **Nothing
was gained or lost.** The token count went 159 → 161 against a reference of 157, i.e. from 1.27% to
2.55% with the band at 2%. Ink is flat: `diff%` 0.484 → 0.483.

**This verdict was a coin flip on a document whose text is scrambled on both sides**, and the round
lost the toss. It is a tokenisation ceiling, not a content change.

### `023_Waterfall_Chart_Template_for_Excel`: real, understood, and **recovered** — 872/868

Ours 881 → 843 → **872**, reference 868. Ink *improved* at the intermediate state already:
`diff%` 2.766 → 2.725. The paragraphs below describe the intermediate 843, because that is the
state the diagnosis was made in and § 11a/b is what closed it.

The reference's own tokens say why it counts 868: it draws the nine `Delta n` category labels
**rotated**, so `pdftotext` reads each as three tokens — the reference's page-1 token multiset
holds `De` ×9 and `lta` ×9. We draw them horizontally, one token each. **Our 881 was not agreement;
it was our own fragmentation elsewhere happening to make up the difference**, and the change
removed 18 spurious `0` tokens at the same time as it changed the label count.

A blind reviewer who had never seen the page (§ 8) ranks the document's real defects, and they are
not small: *"the right half draws all nine [green and red delta bars] as filled floating
rectangles; the left half draws none of them at all"*; *"the left runs −5000 to 25000 … the right
runs −2000 to 8000"*; *"the right half draws [dashed connector lines] between every pair of bars;
the left half draws none"*; and, exactly the item at issue, *"the right half draws twelve of them
and the left draws six … the right half's are rotated ~45°, the left half's are horizontal"*, all
at high confidence.

**So the loss was real and the pass before it was not.** It is worth saying plainly rather than
netting it away: this round first moved a document from a passing verdict it did not deserve to a
failing verdict it did — while getting closer to the page — and only then found the reason and
closed it. The document now passes at 872 against 868 *and* draws the reference's twelve rotated
category labels rather than six horizontal ones, which is a different thing from passing at 881.

## 6. Two decisions inside the law, both argued at the site rather than fitted

1. **The pixel count comes from the size in points, not from the map unit.** Going through
   hundredths of a millimetre — as `ToPixels` and `ToEmSize` do, because that is the device's own
   map mode — makes 9 pt come out at 12.0189 pixels rather than 12, applying a 0.16% correction at
   a size where the law says there is none. The probe **cannot separate the two readings**: at 9,
   12 and 18 pt they differ by less than the estimator's own 0.005 noise. Taking the reading that
   leaves the exact sizes exactly alone means a chart stating 9, 12 or 18 pt renders byte for byte
   as before. `verify-test.sh` finds the map-unit formulation **detected by `MetricGridTests` and
   not by the sheets theory**, which is the right shape: the decision is pinned where it was made.

2. **The reference's per-glyph rounding is measured and deliberately not shipped.** On `003`'s page
   1 every one of the thirty `;` and twenty-two spaces carries an *identical* per-glyph `TJ`
   adjustment, which rounding the cumulative position cannot produce, and
   `round(designUnits × ppem / upem)` in whole hundredths of a millimetre matches **9 of 9**
   distinct glyphs where `floor` fails 3. It is worth at most **0.014 pt a glyph**. It was
   implemented first, swept over the whole track, and then removed: it changed exactly **one**
   document's word count relative to the shipped rule (`057_Simple_balance_sheet`, 474 → 464 → 474)
   and moved no verdict either way. A perturbation that small moving a knife-edge label decision,
   with no measurement saying it moves it the right way, is not worth shipping for its own sake.

## 7. Tests: **+16**, and four mutations

`XRay.Office.Spreadsheets` **1020 → 1035** (a 13-case theory and two facts), `XRay.Office.Text`
**624 → 625**. Re-derived by running each project: Containers 109, Core 390, Markup 259,
OpenDocument 125, Presentations 872, Rendering 153 (+1 skipped), Spreadsheets 1035, Text 625,
Vector 302, WordProcessing 1225 — **5095 passed, 0 failed, 1 skipped**.
`dotnet build -v q -nologo` → **0 warnings, 0 errors**.

| mutation | outcome |
|---|---|
| the chart run drops the pixel-em scale (the pre-round behaviour) | **detected** — 10 of the 13 theory cases *and* `TheChartRunReproducesTheReferencesOwnDrawnLabelWidths` |
| the scale floors the pixel count instead of rounding it | **detected** — `TheChartDevicesEmIsAWholeNumberOfPixelsAndTheCorrectionIsASawtooth` |
| the pixel count is taken through the map unit, `XRay.Office.Text` filter | **detected** — the same test, by the exact-size assertion |
| the pixel count is taken through the map unit, `XRay.Office.Spreadsheets` filter | **NOT detected**, and that is the documented ambiguity rather than a hole: the sheets theory's 0.006 tolerance is set by the twenty-glyph sample's own rounding, and the two readings differ by 0.0006 at 10 pt |

The 13-case theory is a **discriminator and not a golden number**: three of its sizes have a ratio
above one, three exactly one and seven below, so a rule that always narrows fails, a rule that never
quantises fails, and a wrong constant fails at a different size from a wrong exponent.
`TheChartRunReproducesTheReferencesOwnDrawnLabelWidths` asserts **both** answers — the new one and
the old one — so a later change cannot quietly move the chart path back.

## 8. The vision round — four blind readers, and the one instrument-refuted claim was
self-flagged as low confidence by the reader who was right

Four subagents, one composed pair each, `Read` on one image path only, no project documents, no
source, no shell, each asked to describe the halves separately, give a direction and a confidence,
and say what looked identical. **No page was chosen by `--worst`.**

### `003_advanced_excel_pie` p1, twice — chosen because it is the page the round set out to close

The whole page at 110 dpi and the chart alone at 200 dpi, two different readers.

Both list the four in-pie data labels among what is **identical**: *"Four of the five data labels …
same text, same two-line wrapping at the same break point, same font, same colour, same
inside-slice placement"*, and *"the in-wedge label placement pattern — each label in the same wedge,
same horizontal position, same two-line layout"*. **Round 61's reader on this page reported `M3` as
"placed completely differently … outside the pie, below it, as one line"; two fresh readers now
report it as one of the identical things.** `probe-labelfit.py` agrees at 0.12 pt.

Both readers then ranked first the same *new* difference, and **the instrument refutes one half of
it and confirms the other**:

> *"the first half renders the whole string including the value and percentage; the second half
> renders only 'M1; Actual;' and drops '93; 17%'. The truncation point coincides with the
> reference's frame right edge, so it reads as clipping to the chart area."* — high confidence
>
> *"the second half closes [the chart frame] with a visible right border … the first half's frame
> has no right border"* — high confidence from one reader; the other read the same thing and
> **flagged it low-to-medium**, adding *"a crop explanation is plausible and I'd lean that way"*.

- **The frame claim is refuted.** `pdf-ops.py`: our chart-area rectangle is
  (164.98, 341.02)-(674.99, 624.37) and the reference's (165.37, 341.26)-(674.82, 623.93) — the
  same rectangle to **0.4 pt**, and both run past the 595.28 pt MediaBox so neither has a visible
  right edge. The reader who flagged her own confidence as low-to-medium and named the crop as the
  likely cause **was right**, and the one who ranked it first at high confidence was not.
- **The truncation claim is real, and round 61's attribution of it is refuted.** A 200 dpi column
  profile of the `M1` label band: our ink runs to **550.80 pt** and the reference's stops at
  **516.24**, with zero ink in every column from 517 to 560 on the reference's side. Round 61
  recorded this as *"cut off at roughly the chart frame's right border, which is the A4 MediaBox"*.
  **516 is not 595.** The reference clips page-1 drawing content at the last printed column's own
  right edge and we do not. It moves **no words**, because a clipped glyph is still in the text
  layer — which is exactly why it needed a raster to see.

### `023_Waterfall_Chart_Template_for_Excel` p1 — chosen because it is the document the round lost a verdict on

Reading the page that moved against you is the point. Quoted in § 5; the reader independently and
at high confidence gave the direction on the exact object the token diff had implicated (*"the right
half draws twelve … the left draws six"*, *"the right half's are rotated ~45°"*), and ranked ahead
of it three larger defects this round did not touch — nine missing delta bars, a y-axis running to
25000 against the reference's 8000, and no dashed connectors.

She also reported, unprompted and at high confidence, *"the right half draws a thin grey rectangle
around the entire chart; the left half draws none"* — and separately that the `#N/A` helper cells
and both footer strings are **grey in the reference and black in ours**, at high and medium
confidence respectively.

### `microsoft_learn_multi_chart_examples` p1 — chosen because it is an `open` chart document whose verdict did *not* move

A fourth reader, ranking it first of everything on the page at high confidence:

> *"the right half draws a thin rectangle around each chart as a whole; the left half draws none. On
> the right I can trace the top, left and bottom edges of the box on all three charts; on the left
> the charts float on bare white with no frame anywhere."*

**`pdf-ops.py`, re-measured at this tree after the change: reference 3 `#D9D9D9` strokes, ours 0.**
On `023_Waterfall` the same instrument gives reference **1** — a stroke at
(68.17, 425.79)-(530.67, 755.77) — and ours **0**.

**That is now four readers across two rounds, on three unrelated documents, naming the same object,
with an instrument that can measure it agreeing every time.** It is the strongest-shaped reading
this project has recorded and it has now arrived a fourth time.

Her second finding is a **lead, corroborated across rounds and not yet measured**: *"the left half
draws [the clustered-column chart's title]; on the right half I see no title text at all"*, medium
confidence, with the crop honestly named as an alternative. Round 61's reader on the same document
reported the same thing in the same direction. Two readers, two rounds, same object, same
direction, **no instrument yet** — that is the shape that has been right before and wrong before,
and it is recorded as unresolved.

Her explicit negative is worth as much: asked directly about the colour of text drawn over a
coloured shape, she answered that the page holds no such case and that the only one (the harness's
own banner) matches. **A reader saying "the thing you asked about is not here" is a reader worth
believing on the things she does report.**

## 9. `MANIFEST.tsv`

Lives in the corpus repository and was **not touched**. **One** row changes, a loss, proposed
rather than made:

| path | proposed |
|---|---|
| `sheets/chartset-008/xlsx/046_Cost_analysis_with_Pareto_chart_7db5ef69.xlsx` | `done` → **`open`** (161/157) |

`023_Waterfall_Chart_Template_for_Excel_349f7689.xlsx` stays `done` (872/868) and **no row is
proposed for it** — but it should be read as a `done` document whose chart draws three of twelve
bars, which § 5 and § 8 both say and the status column cannot.

`fse_identification_form.xlsx` moved on the reference's side in both directions within this round
and belongs with `ans_mappings_of_eccairs_terms.xlsx` under the `unstable` treatment; this round
adds one more observation to that case and proposes nothing on its own.

## 10. The 24.2.7.2 audit — counters re-derived, **no re-check run**, next site named from the file

Re-derived at this base with the file's own commands: **37 open hits in 26 files** — the brief's
figure exactly — and **31 marker lines, 26 `VERIFIED`, 4 `FIXED`, 1 `WRONG`, 0 `UNDECIDED`**. The
brief's *26 / 21 / 4 / 1* **does not reproduce**: it was derived at round 61's own pre-merge base
and the slides and words branches merged beside it carried five more marker lines in. The open
count is unchanged, which is why the discrepancy is in the marker column alone — the fifth time a
quoted number in that file has failed to reproduce. `TODO.24-2-7-audit.md` is corrected.

**This round pointed no probe at any site**, and says so rather than filing a hurried one: the
budget went to the width law and two whole-track sweeps. The next site is named **with the command
and not from anyone's prose**: `XRay.Office.Core/Graphics/GlyphRun.cs` :347 and :369 — the only two
open hits in `XRay.Office.Core` — which are **not** the claim round 61 verified. Round 61 settled how
`a:lum` is *read*; these state what is then *done* with the pair, in three cases of which claims 2
and 3 are two different arithmetics on the same inputs and are separated outright by one fixture
set (brightness alone, contrast alone, both together).
`probes/sheets-r61/audit_lumpercent.py` already authors the packages and reads the reference's mean
channel. `XRay.Office.Rendering/Images/RasterImageDecoder.cs`:239 is the same `Bitmap::Adjust` branch
from the decoder's side and belongs in the same run.

## 11. Shared layer

`XRay.Office.Text/Fonts/LineSpacing.cs` gains **one method**, `MetricGrid.PixelEmScale`, and
`git grep` shows **no call site outside `XRay.Office.Spreadsheets`**. No words or slides rendering can
move. `SlideChart` and `FrameChart` are untouched and still measure chart text on the face's
unquantised advances — the same line round 60 drew for the vertical half.

**Falsifiable prediction for the parent: 0 verdicts move on words and 0 on slides, and 0 bytes of
their renderings change.**

Reach of the *sheets* change, `census-chartreach.py`, all 946 manifest paths, case-folded where it
accumulates: **90 sheets documents hold a text-bearing chart part** (76 `done`, 14 `open`) plus 7
BIFF chart documents; the untouched cross-track reach is **67 slides** and **10 words**.

## 11a. The mechanism behind both regressions, traced to the line — and it invalidates a
calibration rather than exposing a bug

`ChartAxisLabels.Resolve`'s only route from "the labels collide" to "turn them 45°" is through
`Wraps`: `canAutoAdjustLabelPlacement` refuses while line breaking is on, and an OOXML axis has it
on, so the axis is rotated **only** if some label first *breaks inside a word* in the room one tick
gives it (`VCartesianAxis.cxx`:889-903, `lcl_hasWordBreak`). If no label wraps, the loop falls
through to `rhythm++` and **thins instead of rotating**. Making the labels 2.5% narrower is exactly
the way to flip `Wraps` from true to false — and that is what happened on `023` (twelve rotated
labels became six horizontal ones) and on `058` (twenty-four consecutive dates became ten).

**The limit `Wraps` compares against is a fitted constant, and it was fitted with the ruler this
round corrected.** Its remarks record the fit: three boundary series on
`research/probes/slides-r30/make-rot-probe.py`'s decks bracket the limit at **[0.990, 1.056] of the
tick spacing**, one is "the only round number in the intersection", and the C++'s own
`0.95 × spacing` (`VCartesianAxis.cxx`:753-759, "to have a visible distance between the labels")
is explicitly rejected as "0.88 of it on those decks". **Every one of those boundaries was found by
comparing LibreOffice's rotation against *our* word widths, which were 2.5% too wide at 10 pt.** So
the fitted 1.000 is a measurement of `true limit ÷ 0.975`, and the true limit is near **0.975**,
which is within a hair of the C++'s 0.95 plus the two horizontal text insets the same remarks say
they could not separate it from.

**This round therefore did not break the rotation rule; it invalidated the rule's calibration**,
and the calibration cannot be repaired by picking a new constant here because the correction is a
sawtooth in the size and the decks were at one size. Two repairs, both stated so the next round
does not have to find them:

1. **Re-derive the bracket against the reference with the corrected ruler.** The decks and the
   generator already exist and the boundaries are read off LibreOffice's own
   `chart:coordinate-region`, so this is a re-run and not a new probe — the shape round 59's
   `a:fillToRect` re-check had, which was the cheapest re-check on the audit list. **Not done this
   round**, and it is § 12's first item.
2. **Compare the width against the limit in the units the limit was fitted in**, which is what
   this round ships. See § 11b.

## 11b. What was done about it, and why it is not a fit

The experiment first: a flat `× 0.975` on the limit — the correction at 10 pt — was implemented,
built and rendered. `023_Waterfall` went **843 → 872** against a reference of 868 and the four pies
did not move. That says the direction is right, but a flat constant is wrong on its face, because
the correction is a **sawtooth** and those decks were at one size.

The shipped form is not a constant and is not fitted. The calibration says
`w_old ≤ 1.000 × spacing`, and `w_old = w_new / scale(size)`, so the same test in the new ruler's
units is `w_new ≤ spacing × scale(size)` — an identity, at every size, with nothing chosen.
`IChartTextMeasurer` gains a **defaulted** `AdvanceScale(size) => 1.0`, overridden only by the
sheets measurer with `MetricGrid.Chart.PixelEmScale`. A slide's and a document's chart text takes
the default and is therefore **provably** unchanged; every test in both projects passes untouched.

| document | base | intermediate | shipped | reference |
|---|---:|---:|---:|---:|
| `023_Waterfall` | 881 | 843 | **872** | 868 |
| `058_Social_media_engagement_data` | 206 | 100 | **200** | 194 |
| `046_Cost_analysis_with_Pareto_chart` | 159 | 161 | **161** | 157 |
| `003` / `011` / `019` / `027` | 145 / 142 / 142 / 142 | 143 / 140 / 140 / 140 | **143 / 140 / 140 / 140** | 143 / 140 / 140 / 140 |

`058` is 3.1% from the reference where it started at 6.2%, and still fails. `046` is untouched by
this and is the one verdict this round costs on our side.

**It is a stop-gap and the site says so in those words.** When the bracket is re-derived against the
reference, `AdvanceScale` returns to one everywhere and the method can go.

## 12. What the next round should do first

1. **The axis-label wrap limit, re-derived against the reference with the corrected ruler** —
   § 11a/b. This round ships a units correction that restores the *old* calibration exactly, which
   is honest but is still a fitted 1.000 underneath; the C++ says `0.95 × spacing` less two text
   insets and the bracket that rejected it was measured with a ruler that was 2.5% long. Round 30's
   decks and generator both still exist, so it is a re-run. `058` is the document that will say
   whether it worked: the reference draws ~24 consecutive horizontal dates and we draw ten, at
   200 against 194.
2. **`023_Waterfall`'s chart itself**, which is worse than its verdict ever suggested: **nine of
   twelve bars are not drawn at all**, there are no waterfall connector lines, and the value axis
   runs to 25000 against the reference's 8000.
3. **The default `#D9D9D9` chart-area border** — four readers, two rounds, three documents, and
   `pdf-ops.py` at 3-against-0 on `microsoft_learn_multi_chart_examples` and 1-against-0 on
   `023_Waterfall`. 10 sheets documents / 23 parts, 2 slides, 1 words.
4. **A data label's stated text colour** — `ChartDataLabel` still carries no colour field; 22 sheets
   documents / 40 parts, **49 slides**, 7 words. Unchanged by this round.
5. **The page's own right-hand clip.** The reference stops page-1 drawing content at **516.24 pt**
   on `003` and we run to 550.80. Two readers reported it, round 61 attributed it to the MediaBox
   and that is refuted. It moves no words, which is why it needs a raster or a clip-path reader.
6. **The same 96 dpi width law for `SlideChart` and `FrameChart`** — now a two-line change on each,
   still a cross-track change, still needs a census and a say-so.
7. **`cellIs` conditional formatting** — 123 rules in 18 documents, two arms, unchanged since round
   59 § 9. It is the only item on this list that is not a chart.
