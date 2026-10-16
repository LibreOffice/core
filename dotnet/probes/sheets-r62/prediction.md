# Round 62 — sheets — prediction

Committed **before a line of behavioural code is written and before anything is rendered
post-change**. Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`, worktree `wt-sheets-r50`, branch
`wt-sheets-r62`, base `337bc9fe17c`.

## 0. Baseline, and which side moved

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 363 MATCH 308 MISMATCH 55
REF-CANNOT-RENDER 0`; scored against `MANIFEST.tsv`'s 307 sheets paths by round 58's `score.py`,
which refuses to print unless every manifest path found a row: **282 match, 20 `words`, 5
`pages,words`**.

The brief says **281**. The difference is **one document and it moved on the reference's side**:

| document | r61 after | r62 base | which side |
|---|---|---|---|
| `unstable-001/xlsx/fse_identification_form.xlsx` | ours 440, ref 427 → `words` | ours 440, ref **440** → `match` | **reference** |

Ours is 440 in both sweeps. Four further rows moved on the reference's side only and changed no
verdict (`047_Date_tracker_Gantt`, `PBN Matrix NAAs (V01)`, `SIL_TDB648`,
`FAA-2019-0995-0002_attachment_2`, `microsoft_learn_multi_chart_examples` ref 225 both times, ours
205→204 — that last one is **ours** and is recorded here as an unexplained one-token drift on an
`open` document that stays `words` either way). `ans_mappings_of_eccairs_terms.xlsx` **matched**.

So the baseline is **282**, reproduced, with the discrepancy against the brief accounted for on the
reference's side.

## 1. What the round changes, and why it is not the item the brief named

The brief's item 1 is *"the inner-fit test rejects one label of five the reference keeps"*. It is
real and it is measured — but **the inner-fit test is not what is wrong**. `probe-labelfit.py`
reads both renderings' page 1 and gives:

```
ours  centre (408.81, 464.81)  radius 100.01
   M3; Actual; 107; 20%   lines 1  box  86.62 x 12.21  centre ( 486.39, 358.10)  d 131.93 outside
ref   centre (408.84, 464.74)  radius  99.78
   M3; Actual; 107; 20%   lines 2  box  65.43 x 23.43  centre ( 438.17, 400.95)  d  70.21 inside
```

Feeding **our** box into the port gives `FAIL(CM 36.33 > 36.00)` — it misses by **0.33 of a
degree**. Feeding the box the reference actually drew gives a fit. The predicate is a faithful
port; its **input** is 1.7 pt too wide.

And the input is too wide for a reason with a mechanism. Round 60 put a chart's *vertical* metrics
through `chart2`'s own **96 dpi** device (`MetricGrid.Chart`) and left the **advance width** on the
face's unquantised metrics. 26.2.4.2 does not: it instantiates the em at a whole number of device
pixels — `round(size × 96/72)` — so at 10 pt every advance comes back **2.5% narrower** than the
size the file asks for, and at 11 pt **2.3% wider**.

**Read off the reference alone; our renderer never runs.** `probe-chartwidth.py` renders fourteen
one-variable rewrites of `003`'s own chart part and reads the drawn advance out of the reference's
own `TJ` arrays, which carry a per-glyph adjustment in thousandths of the text em and are therefore
independent of the chart's scale, the page and the writer's chosen font size:

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

**The law is a sawtooth in the size and nothing else is.** It goes above 1 at 8, 11, 14 and 20 pt
and below at 10, 13, 16, 22 and 28, in the right places and by the right amounts, with no free
parameter. The residual is ≤0.005 and almost always negative, which is the estimator's own known
bias: it takes the modal adjustment per glyph code and a real kern pair narrows a glyph, so
measured runs slightly under. The 36 pt row is a bonus control — the reference clamped it to 23.991
and the law still holds *at the size it drew*.

At the glyph level the rule is finer than a scale and is measured too: on `003`'s labels the drawn
advance of every one of nine distinct glyphs is `round(designUnits × ppem / upem)` in **whole
hundredths of a millimetre** (`round` matches 9 of 9; `floor` fails 3), and identical glyphs carry
identical adjustments in all 30 occurrences of `;` and all 22 of the space — so the rounding is
per glyph, not of the cumulative position.

## 2. Where the two surplus words are, measured

`003`'s chart straddles the page break and page 2 carries the clipped remainder.

| | page 1 | page 2 | page 3 | total |
|---|---:|---:|---:|---:|
| ours | 43 | **20** | 101 | 145 |
| reference | 43 | **18** | 101 | 143 |

The page-2 difference is M3's label alone: ours draws `Actual; 107; 20%` there where the reference
draws `07;`. **Three tokens against one is the whole of the two-word surplus**, on all four
documents.

## 3. What I predict

| | prediction |
|---|---|
| sheets verdicts | **282 → 282**, and **0 regressions** |
| `003_advanced_excel_pie` | `match` 145/143 → `match` **143**/143 |
| `011_advanced_excel_pie` | `match` 142/140 → `match` **140**/140 |
| `019_advanced_excel_pie` | `match` 142/140 → `match` **140**/140 |
| `027_advanced_excel_pie` | `match` 142/140 → `match` **140**/140 |
| `003` M3 block centre | inside the slice, text centre within **1 pt** of the reference's (438.17, 400.95) |
| `003` M1 | stays outside, on **one** line (its inner attempt no longer wraps: 79.48 against an 80.0 allowance) |
| `003` pie radius | within **1 pt** of 100.0; it is not the quantity this round moves |
| page counts on our side | **0 change anywhere** |
| documents whose chart text moves at all | **97 sheets** — 90 OOXML (76 `done`, 14 `open`) + 7 BIFF |
| tests | **+6 to +15** |
| `MANIFEST.tsv` rows | **0** |

**The item the brief ranked first moves zero verdicts**, because all four documents already pass
inside the 2% band. That is the honest prediction and the brief says a round that predicts no
movement and is right is a well-run round.

**The thing most likely to be wrong**: a regression among the other **93** chart-bearing documents.
Every one of them has every axis label, legend entry, title and data label re-measured, by up to
2.5% either way; an axis label that stops wrapping, a legend that changes width, or a category
label that starts colliding could move a word count. I predict **0** such regressions and I expect
that to be the prediction that fails. The band I will accept without calling the round a failure is
**−2 to +1** verdicts.

## 4. What the census cannot see

1. **BIFF.** The 7 `.xls`/`.xlsb` documents with a chart substream are counted by the substream's
   existence only; their labels' text, sizes and faces are not decoded here, so they are a *reach*
   figure and not a *movement* figure.
2. **The size a chart actually draws at.** The probe shows the reference rescales a chart's text to
   the page reference size — 16 pt stated is drawn at 15.888 — and **we do not do that at all**. So
   for a chart whose stated size sits near a pixel boundary, we will pick a different `ppem` from
   the reference and the sign of the correction can flip. `003` is not such a chart (10.008 and 10
   both give 13) but some of the 90 will be, and this census cannot say which.
3. **`SlideChart` and `FrameChart` are not changed**, so 67 slides and 10 words documents carry the
   old width. This is deliberate — it is exactly where round 60 drew the same line for the vertical
   half — and it means the two consumers now disagree about one chart's measurement.
4. **Inherited and defaulted label text.** The census counts a chart part as text-bearing when it
   holds a `c:title`, an axis, a legend or a `c:dLbls`; a chart drawing *only* synthesised category
   names still goes through the same measurer and is counted, but one drawing no text at all would
   be over-counted rather than missed.
5. **The predicate's own second-order inputs.** The line height and ascent already go through the
   chart device; the *key* and *key gap* do not — they are `int(fontHeight × 0.6)` and
   `max(100, fontHeight × 0.22)` in hundredths of a millimetre and are read from the stated size,
   not the device's. If the reference takes those from the pixel em too, the block width is a
   further ~0.2 pt out and this round will not see it.

## 5. Shared layer

The diff adds one method to `MetricGrid` in **`XRay.Office.Text`** (`Fonts/LineSpacing.cs`) and
changes **`XRay.Office.Spreadsheets`** (`Layout/SheetBandText.cs`, `Layout/SheetChart.cs`). The
`XRay.Office.Text` change is **additive only** — a new public method with no call site outside
`XRay.Office.Spreadsheets`, which will be shown with `git grep` — so no words or slides rendering can
move. **Falsifiable prediction for the parent: 0 verdicts move on words and 0 on slides, and 0
bytes of their renderings change.**
