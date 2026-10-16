# Round 61 — sheets — prediction

Committed **before** a line of behavioural code was written and before anything was rendered
post-change. Base `3f079cea621`, branch `wt-sheets-r61`, reference **26.2.4.2 620(Build:2)**,
`fc-match "DejaVu Sans"` → `DejaVuSans.ttf`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## Baseline, reproduced first

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 363 MATCH 304 MISMATCH 59`, scored against
`MANIFEST.tsv`'s 307 sheets paths by round 58's `score.py` (which refuses to print unless every
manifest path found a row): **279 match, 23 `words`, 5 `pages,words`**. **The briefed 279 of 307,
exactly.** `ans_mappings_of_eccairs_terms.xlsx` matched this time — the volatile document fell on
its winning side, which is the 8-in-9 outcome round 60 measured, not a change.

## What this round changes, and why — both read off the reference before our source was opened

### A. `VDiagram::reduceToMinimumSize` — the pie's pass-1 rectangle

`ChartView.cxx:557-560` runs

```cpp
xSeriesTargetInFrontOfAxis = aVDiagram.getCoordinateRegion();
// It is preferable to use full size than minimum for pie charts
if (!rParam.mbUseFixedInnerSize)
    aVDiagram.reduceToMinimumSize();
```

before any series is created, and for a **pie** nothing grows it back before pass 1: the
`!bIsPieOrDonut` guard at `:588` skips the axis-label `adjustInnerSize` entirely, so the pie's
first pass is drawn at

```
w = round(availW/2.2)   h = round(availH/2.2)
rect = (availX + w, availY + h, w, h)  ∩ available,  then squared and centred
```

`git blame` puts that line at 2019-05-28, so it is in 26.2.4.2. The comment is a **complaint**,
not a description: the code reduces a pie too, and the pie's own second pass is what grows it
back. **We model pass 1 at the full diagram rectangle**, which is why round 60's trace read
`consumed.Left = 291.76` — at the full radius all but one of `003`'s labels fit *inside* the pie
and nothing reaches left of it. At `availW/2.2` the radius is 50.33 instead of 110.72 and every
label fails the inner fit, which is how the reference's pass 1 comes to put labels on the left.

Hand arithmetic on `003` with crude 10 pt block estimates (60-70 pt wide, two lines) gives a
final centre of about **(403.7, 377.1)** and radius **88.8** against the reference's
**(408.84, 377.15)** and **99.78**, where we ship **(382.80, 374.21)** and **104.70**. The centre
*y* is the striking one: 377.11 against 377.15 falls out with nothing fitted. The radius and the
last 5 pt of *x* depend on our own label measurements, which is exactly the blind spot below.

### B. The main title's draw position — `135` + `TextUpperDistance`

`lcl_createTitle` (`ChartView.cxx:1058-1069`) puts a `MAIN_TITLE`'s shape top at
`rRemainingSpace.Y + int(pageHeight × 0.02) + 135` hundredths of a millimetre, and
`ShapeFactory::createText` (`ShapeFactory.cxx:2279-2299`) then insets the *text* by
`round(fontHeight_mm100 × 0.30)`. We draw at `frame.Y + frame.Height × 0.02` and neither term is
there — although `DiagramAreaOf` **already reserves both** (`TitleGap` = 135, and `Shape()` adds
the 0.30 inset), so the reservation and the drawing disagree with each other.

`probe-titlepos.py` renders **eighteen** one-variable rewrites of `003`'s own chart part — nine
title sizes from 6 to 36 pt × bold and regular — through the installed binary and through our CLI,
and measures `y_ours − y_ref`:

| size | 6 | 8 | 10 | 12 | 14 | 18 | 22 | 28 | 36 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| measured | 6.040 | 6.600 | 7.220 | 7.810 | 8.390 | **9.570** | 10.780 | 12.540 | 14.920 |
| `(135 + round(0.30·size))/100 mm` | 5.641 | 6.236 | 6.831 | 7.427 | 8.022 | **9.213** | 10.431 | 12.217 | 14.627 |
| residual | 0.399 | 0.364 | 0.389 | 0.383 | 0.368 | **0.357** | 0.349 | 0.323 | 0.293 |

The law is right in slope and in constant with **no free parameter**; a residual of 0.29–0.40 pt
survives against a term that runs from 5.6 to 14.6 pt. **The residual is not fitted out.** The
9.57 pt figure is confirmed on **four** documents, not the two the brief cited — `003`, `011`,
`019` and `027` all read ours 601.44 against the reference's 591.87, x agreeing to 0.15 pt.

## Predictions

| # | claim | number |
|---|---|---|
| 1 | sheets verdicts, 279 → | **281** |
| 2 | `003_advanced_excel_pie` | `words` 139/143 → **`match` 143** |
| 3 | `027_advanced_excel_pie` | `words` 136/140 → **`match` 140** |
| 4 | `011_advanced_excel_pie` stays `match` | 137/140 |
| 5 | `019_advanced_excel_pie` stays `match` | 140/140 |
| 6 | `003` pie centre x, after | within **6 pt** of 408.84 (we ship 382.80) |
| 7 | `003` pie centre y, after | within **1 pt** of 464.74 PDF (we ship 467.68) |
| 8 | `003` pie radius, after | within **12 pt** of 99.78 (we ship 104.70) — the loosest band here, and deliberately so |
| 9 | title baseline on all four pie documents | 601.44 → **592.2 ± 0.1**, reference 591.87 |
| 10 | regressions among the other **58** `done` sheets documents with a titled chart | **0** — *and this is the number most likely to be wrong* |
| 11 | page counts changed on our side, anywhere in sheets | **0** |
| 12 | tests | **+10 to +25**, `XRay.Office.Core` |
| 13 | `MANIFEST.tsv` rows to propose | **2** |

## Shared layer — this diff is `XRay.Office.Core` and the parent must sweep the other two tracks

`ChartLayout.cs` and `ChartLayout.PieLabels.cs` are reached by `SheetChart`, `SlideChart` and
`FrameChart` alike. `census-piestitles.py`, both readers, all **946** manifest paths, case-folded
where it accumulates:

| | sheets | slides | words |
|---|---:|---:|---:|
| documents with a **titled** chart (change B) | **62** — 54 `done`, 8 `open` | 14 | 3 |
| documents with a **best-fit pie** (change A) | **7** — 4 `done`, 3 `open` | 5 | 2 |
| BIFF documents holding a chart substream (upper bound on both) | 7 | 0 | 1 |

Named, for change A, because it is the small set: sheets `011`, `019`,
`005_Contextures_chart_sample_6e279b08` (3 parts), `003_Contextures_chart_sample_9bda2719.xlsm`
are `done` and **at risk**; `003`, `027`, `microsoft_learn_multi_chart_examples` are `open`.
Slides `bitesize-writing-a-report.pptx` and `3495.pptx` are `done` and at risk; words
`pie-chart-result.docx` and `pie-chart-template.docx` are `done` and at risk.

**Cross-track prediction, falsifiable: 0 verdicts move on words and 0 on slides.** Change B moves
17 non-sheets documents' title text down by 5–15 pt and change A re-lays-out 7 non-sheets pies;
neither changes a page count or a word unless a chart title or a pie label crosses a page or slide
boundary, which on a slide it cannot.

## What the census and the arithmetic **cannot** see — written down before the sweep

1. **BIFF charts are not decoded to the label-placement field.** Eight `.xls`/`.doc` documents hold
   a chart substream and any of them could be a best-fit pie. The census reports them as their own
   column rather than folding them in, so change A's reach is a *lower* bound of 14 OOXML
   documents plus an unread 8.
2. **Which labels pass 1 puts outside is decided by our own text measurement, not LibreOffice's.**
   At the reduced radius the wrapping allowance is 40.3 pt instead of 88.6, so *every* label is
   near the inner-fit boundary and a fraction of a point of width can flip one. Prediction 8's
   band is wide for exactly this reason and prediction 6's is wider than it looks.
3. **`Squared` is applied to the reduced rectangle too**, and the reduced rectangle is *not*
   centred in the available one — it sits at `+W/2.2, +H/2.2`. If 26.2.4.2's `adjustPosAndSize`
   ordered the intersect and the aspect-ratio step the other way round, the pass-1 centre moves
   by tens of points and every number above is wrong. The C++ reads intersect-then-square and it
   was transcribed from the file, not from a round's prose.
4. **A pie whose labels are *not* best-fit still goes through the reference's reduce-and-regrow**,
   and our `HasBestFitLabels` gate skips it entirely. That is left alone this round, so any pie
   with `ctr`/`inEnd`/`outEnd` labels whose pass-1 labels overflow the *reduced* wall is still
   modelled wrong. Round 59 measured `ctr` at 110.44 — i.e. it regrows to full — which is why the
   gate is defensible, but it is one document's evidence.
5. **The 0.29–0.40 pt title residual has no mechanism.** It shrinks slightly with size, so it is
   not a constant and not a proportion, and it is not the 0.75 pt quantum of the 96 dpi grid. If
   it is an ascent difference then round 60's ascent law is 0.35 pt out at every size, which its
   own control did not have the resolution to see.
6. **`ans_mappings_of_eccairs_terms.xlsx` can lose a page at random on the reference's side**
   (round 60 § 6, one rendering in nine). It matched in this round's baseline. If it mismatches
   after, **that is not this round's regression** and the check is which side moved.
