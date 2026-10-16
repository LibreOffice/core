# Round 60 — sheets — prediction

Committed **before** the change is written and before anything is rendered post-change.
Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r60`, base
`c17996f89cb`.

## Baseline, reproduced before anything was touched

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 363  MATCH 303  MISMATCH 60`; scored against
`MANIFEST.tsv`'s 307 sheets paths by round 58's `score.py`, which refuses to print unless every
manifest path found a row: **278 match, 24 `words`, 5 `pages,words`**. That is the briefed figure
exactly, and `fse_identification_form.xlsx` is *inside* the 29 mismatches at 440/427 — so the live
278 already carries the volatile document on its losing side.

## The law this round implements, measured off the reference before the C++ was opened

`probe-chartvmetrics2.py` renders 117 one-variable rewrites of `003_advanced_excel_pie.xlsx`'s own
chart part through the installed binary: thirteen sizes from 6 to 40 pt on three faces (Carlito,
Liberation Sans, Liberation Serif), each as a one-line and a three-line chart title, plus a
single-line `dLblPos="ctr"` data label series. `law-chartvmetrics.py` reads them back.

**Every one of the 39 measured line pitches is an integer multiple of 0.75 pt** — one pixel at
96 dpi — and the integer is

    hpx   = round(size_pt × 96 / 72)
    pitch = ( round(ascender/upem × hpx) + round(-descender/upem × hpx) ) × 72/96

with the `hhea` **line gap excluded**. Carlito's gap is zero and both Liberation faces' is not, so
the three faces separate the gap term outright. Against the four candidate laws:

| law | max error | mean |
|---|---:|---:|
| pixel, no gap | **0.089 pt** | **0.036** |
| pixel, with gap | 1.498 | 0.473 |
| continuous, no gap | 0.988 | 0.309 |
| continuous, with gap — **what we ship** | 2.476 | 0.613 |

The ascent is the same rounding: a CENTER label's block centre `C` must be size-independent, and
`C = y1 − (H/2 − A)` comes out constant to **0.042–0.069 pt** over ten sizes under the pixel law
against **0.58–0.70 pt** under ours.

This is not a new mechanism in the tree — `MetricGrid` already carries exactly this arithmetic for
Impress (600 dpi) and Calc (720 dpi), including EditEngine's max-of-two-roundings line height and
its no-external-leading ascent. The defect is that `SheetBandText.Ungridded` drops the grid
*entirely* for chart text. **chart2 does not use no device; it uses a different device.**

## What will change in the code

* `XRay.Office.Text/Fonts/LineSpacing.cs` — **additive only**: a new `MetricGrid.Chart` constant
  (96 dpi, 1/100 mm). No existing caller's arithmetic changes, so words and slides cannot move
  **by construction** rather than by census.
* `XRay.Office.Spreadsheets/Layout/SheetBandText.cs` — `ChartLineHeightAt` takes that grid; a new
  `ChartAscentAt`; and `SheetShapePainter`'s two calls move to a separately named function that
  keeps today's ungridded arithmetic byte for byte.
* `XRay.Office.Spreadsheets/Layout/SheetChart.cs` — the two drawing paths take `ChartAscentAt`.

## Reach, from a census rather than from the pool

`census-charttext.py` reads all **946** manifest rows with two readers — an OOXML `charts/chartN.xml`
part, and a BIFF substream whose BOF document type is `0x0020` — case-folding where it accumulates,
and refuses to summarise unless every row is read:

| family | documents holding a chart | parts |
|---|---:|---:|
| **sheets** | **97** | 154 |
| slides | 67 | 159 |
| words | 10 | 10 |

**Every one of the 97 sheets documents is re-laid-out**, and 78 of them are `done`. That is the
risk this round carries: the change is small, the surface is a quarter of the track.

### What this census cannot see

1. **It counts documents that hold a chart, not chart text on a gated page.** A chart on a hidden
   sheet, or below the print area, or with every label suppressed, is counted and cannot move.
   The census therefore **over-reaches**, which is the safe direction for a risk estimate and the
   wrong one for a yield estimate.
2. **It cannot see which of the 97 have a label whose *wrap* changes.** A verdict only moves when
   a word count crosses the 2% band, which needs a line to break differently or a label to leave
   the page — not merely to move. Most of the 97 will move ink and no verdict.
3. **The `.xls` arm scans for an unaligned `0x0809` BOF anywhere in the file**, so it can count a
   chart inside an embedded object; the document figure is an upper bound.
4. **It says nothing about slides and words**, deliberately. Their chart text goes through
   `SlideChart` and `FrameChart`, which have their own measurers and are **not touched**. If they
   carry the same defect — and the law is a property of chart2, not of Calc, so they probably do —
   that is a separate round's work and this one must not be read as having fixed it.
5. **`ChartLayout.cs:1081` still hard-codes `ChartLineHeight = 1.1499`** for one legend-offset site.
   It is Liberation Sans' continuous figure and it is left alone; a census of what reaches it has
   not been run.
6. **The grid Calc's *drawing shapes* format against is unmeasured.** `SheetShapePainter` keeps the
   old arithmetic on purpose, so text boxes do not move — but "unchanged" is not "verified".

## The prediction

| | predicted |
|---|---|
| sheets verdicts | **278 → 280** |
| `chartset-002/xlsx/011_advanced_excel_pie.xlsx` | `words` → **match**, 136 → 140 |
| `chartset-002/xlsx/027_advanced_excel_pie.xlsx` | `words` → **match**, 136 → 140 |
| `003_advanced_excel_pie` | stays `match` at 143; its pie radius 104.13 → **within 1.5 pt of 99.78** |
| `019_advanced_excel_pie` | stays `match` at 140 |
| `003`'s pie centre | (412.64, 468.05) → **within 2 pt of (408.84, 464.74)** |
| page counts, anywhere | **0 change** |
| regressions among the 78 `done` chart documents | **0**, and this is the number most likely to be wrong |
| `microsoft_learn_multi_chart_examples` | moves words, stays `words` |
| tests | **+6 to +14**, in `XRay.Office.Spreadsheets` and `XRay.Office.Text` |
| other tracks | **0 documents**, structurally — the shared-layer edit is a new constant only |
| `MANIFEST.tsv` | 2 rows change status, proposed not made |

**Zero verdict movement is a legitimate outcome** and the gate cannot see most of what this
changes. The measurement that decides whether the round worked is the pie radius and centre on
`003`, which are read off the wedge corner (round 59 § 3) and not off a bounding box.

## What would refute it

* If `011` and `027` do not reach 140, the residual is **not** the vertical metric and round 59's
  attribution was wrong.
* If any `done` chart document loses a verdict, the 96 dpi grid is wrong for some path that reaches
  chart text, and the right answer is a narrower change.
* If the radius does not close to within 1.5 pt, something *else* in the second pass is also wrong
  and the label box is not the only input.
