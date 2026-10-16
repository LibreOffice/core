# Round 61 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r61`, base
`3f079cea621`. Read `prediction.md` (`2bf1b306ddc`) beside this file first — it was committed
before a line of behavioural code was written and before anything was rendered post-change.

## 1. Baseline: **279 of 307**, the briefed figure exactly

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 363  MATCH 304  MISMATCH 59`; scored by
round 58's `score.py` against `MANIFEST.tsv`'s 307 sheets paths, which refuses to print unless
every manifest path found a row: **279 match, 23 `words`, 5 `pages,words`**.
`ans_mappings_of_eccairs_terms.xlsx` **matched** in this baseline — the 8-in-9 outcome round 60
measured, not a change — and matched again after, so the volatile document never had to be argued
about.

## 2. Result: **279 → 281**, and **zero regressions**

| document | before | after | which side moved |
|---|---|---|---|
| `chartset-002/xlsx/003_advanced_excel_pie.xlsx` | `words` 139/143 | **`match` 145/143** | ours, a gain |
| `chartset-002/xlsx/027_advanced_excel_pie.xlsx` | `words` 136/140 | **`match` 142/140** | ours, a gain |
| `chartset-002/xlsx/011_advanced_excel_pie.xlsx` | `match` 137/140 | `match` **142**/140 | ours, no verdict |
| `chartset-004/xlsx/019_advanced_excel_pie.xlsx` | `match` 140/140 | `match` **142**/140 | ours, no verdict |

Every other row that moved moved on **the reference's side only**, on date-bearing documents:
`047_Date_tracker_Gantt` ref 822→842, `PBN Matrix NAAs (V01)` 5549→5537, `SIL_TDB648` 7496→7499,
`FAA-2019-0995-0002_attachment_2` 9995→9990. **No document regressed, and no page count changed on
our side anywhere in the track.**

`003` re-closes — the item round 60 opened — and `027` closes with it, which is what round 60
predicted it would take and could not deliver.

### What the gate cannot see, and it is the whole point of the change

| | before | after | reference | |
|---|---:|---:|---:|---|
| `003` pie centre | (382.80, 467.68) | **(408.81, 464.81)** | (408.84, 464.74) | 26.04 pt → **0.03** |
| `011` pie centre | (383.04, 467.46) | **(411.14, 464.64)** | (411.11, 464.57) | 28.07 → **0.03** |
| `019` pie centre | (412.44, 467.77) | **(408.90, 464.88)** | (408.95, 464.82) | 3.49 → **0.05** |
| `027` pie centre | (382.92, 467.57) | **(411.25, 464.72)** | (411.22, 464.68) | 28.30 → **0.03** |
| `003` radius | 104.70 (+4.93%) | **100.01 (+0.23%)** | 99.78 | |
| chart title baseline, all four | 601.44 | **592.20** | 591.87 | 9.57 pt → **0.33** |

Read at the wedge corner, not the path bbox (round 59 § 3). The **+0.23–0.26% residual radius is
identical on all four documents** and is the same sign and size as the −0.25% round 59 measured at
`ctr`, so it is one systematic and not four errors; it is left alone and named in § 8.

## 3. The mechanism: a pie's first pass is `reduceToMinimumSize`, not the diagram

Round 60 solved the arithmetic backwards from the reference's answer and named the unknown
correctly — *which labels pass 1 rebuilds outside* — but assumed pass 1 ran at the diagram's full
rectangle. It does not. `ChartView.cxx:557-560`:

```cpp
xSeriesTargetInFrontOfAxis = aVDiagram.getCoordinateRegion();
// It is preferable to use full size than minimum for pie charts
if (!rParam.mbUseFixedInnerSize)
    aVDiagram.reduceToMinimumSize();
```

**The comment is a complaint, not a description.** The guard is on `mbUseFixedInnerSize` — a manual
`c:layout` on the plot area, which none of these four documents has — and not on the chart type, so
a pie is reduced too. What normally undoes it immediately is the axis-label pass at `:588`, whose
`adjustInnerSize` grows the diagram straight back out — and **that pass is guarded by
`!bIsPieOrDonut`**. `git blame` puts the `mbUseFixedInnerSize` line at **2019-05-28**, so it is in
26.2.4.2; this is not a 27.2-alpha artefact.

`reduceToMinimumSize` is `round(side / 2.2)` on each axis, positioned at `(x + w, y + h)`, then
`adjustPosAndSize` — intersect, then aspect ratio, then centre. On `003`'s diagram
`(174.90, 269.52)-(630.06, 490.96)` that is a first-pass **radius of 50.33 pt, not 110.72**, and its
centre is 82.8 pt right of the diagram's own. The best-fit wrapping allowance falls from 88.6 pt to
40.3, every one of the five labels fails the inner fit instead of one, and the consumed rectangle
overruns on all four sides — which is the shape round 60 could see the reference's answer had to be
solved back to and could not produce.

`VDiagram::adjustInnerSize` needed no change; it was transcribed correctly, exactly as round 60
said.

### One of the round's own stated blind spots is refuted by arithmetic

`prediction.md` blind spot 3 worried that swapping `adjustPosAndSize`'s intersect and aspect-ratio
steps would move the pass-1 centre by tens of points. **It cannot move it at all.** The reduced
rectangle's right edge is `x + 2·round(W/2.2) ≤ x + 0.910·W`, and its bottom edge likewise, so the
intersection with the available rectangle is **always a no-op** and the two orders are the same
function. `verify-test.sh` found this independently: the swap is the one mutation of five that no
test detected, and it is an **equivalent formulation rather than an undetected defect** — those are
different findings and § 6 reports it as the second.

## 4. The 9.57 pt title, measured as a law and closed to 0.33 pt

The item was one number on two documents. It is **four** documents — `003`, `011`, `019`, `027` all
read ours 601.44 against the reference's 591.87, x agreeing to 0.15 pt — and it is two terms both
of which were already in the tree, in the *reservation*, and neither of which was in the *pen*:

* `lcl_createTitle` puts a `MAIN_TITLE` shape's top at
  `rRemainingSpace.Y + int(pageHeight × 0.02) + 135` hundredths of a millimetre
  (`ChartView.cxx:1058-1069`; the flat 135 is added for `MAIN_TITLE` alone). `DiagramAreaOf` has
  carried that 135 as `TitleGap` since the layout was written.
* `ShapeFactory::createText` then insets the text inside the shape by
  `round(fontHeight_mm100 × 0.30)` (`ShapeFactory.cxx:2283-2286`). `DiagramAreaOf` has carried that
  as `Shape()`'s `TextShapeInsetY`.

`probe-titlepos.py` renders **eighteen** one-variable rewrites of `003`'s own chart part — nine
title sizes from 6 to 36 pt × bold and regular — through the installed binary *and* through our CLI,
and measures `y_ours − y_ref` directly, so the frame's own position cancels and there is nothing to
fit:

| size pt | 6 | 8 | 10 | 12 | 14 | 18 | 22 | 28 | 36 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| measured | 6.040 | 6.600 | 7.220 | 7.810 | 8.390 | **9.570** | 10.780 | 12.540 | 14.920 |
| `(135 + round(0.30·size))/100 mm` | 5.641 | 6.236 | 6.831 | 7.427 | 8.022 | **9.213** | 10.431 | 12.217 | 14.627 |
| residual | 0.399 | 0.364 | 0.389 | 0.383 | 0.368 | **0.357** | 0.349 | 0.323 | 0.293 |

Slope and constant both right over a six-fold range with **no free parameter**. Bold and regular
give the same y to the hundredth on both sides, which is worth recording: Carlito and Carlito-Bold
quantise to the same ascent on the 96 dpi chart device.

**The 0.29–0.40 pt residual is not fitted out.** It shrinks slightly with size, so it is neither a
constant nor a proportion, and it is not the 0.75 pt quantum of the chart grid. Differencing the
two sides' ascents across the series gives 1.465/1.525/2.235/2.225/3.76/3.76/5.225/8.249 for the
reference against 1.50/1.50/2.24/2.24/3.77/3.74/5.25/8.25 for ours — agreeing to 0.04 pt — so the
residual is very nearly an **absolute** ascent offset that round 60's slope-based control could not
have seen. Naming a suspect is worth more than a constant: if it is real, round 60's chart ascent
is about a third of a point out at every size, on every chart in the corpus.

### The obvious extension was tested and does **not** close the rest of it

Blind spot 4 of `prediction.md` said a pie whose labels are *not* best-fit still goes through the
reference's reduce-and-regrow while our `HasBestFitLabels` gate skips it entirely, and the reader on
`005_Contextures_chart_sample_6e279b08` reported exactly that shape — *"the right half's pie is
smaller and sits lower and further right"*, high confidence. She is right and it is measured: that
document's page-1 pie is `dLblPos="inEnd"`, our second pass never runs, and the wedge corner gives
ours **radius 131.82 at centre (195.32, 437.91)** against the reference's **125.21 at
(195.36, 436.45)** — 5.3% of radius, the same size of error the four best-fit pies carried before
this round, and **byte-identical before and after the change**.

**The obvious repair does not fix it.** Widening the gate to *any* drawn pie label was implemented,
built and rendered: `005`'s pie moved **0.81 pt in x and did not shrink at all** (131.82 before and
after), because our `PieLabels` places an `inEnd` label at half the radius along the bisector — the
`CENTER` arm — so the consumed rectangle stays inside the reduced wall and `adjustInnerSize` grows
it straight back to the full square. The missing piece is the **`INSIDE`/`OUTSIDE` placement
geometry**, not the gate. The experiment was reverted, the tree restored by `cp` and `touch`,
rebuilt, and the shipped state re-rendered to prove it: `003` back at centre (408.81, 464.81),
radius 100.01.

## 5. Prediction against measurement — **12 of 13**

| | predicted | measured |
|---|---|---|
| sheets verdicts | **279 → 281** | **281 — right** |
| `003` | `words` → `match` at 143 | **`match` at 145** — verdict right, number wrong |
| `027` | `words` → `match` at 140 | **`match` at 142** — verdict right, number wrong |
| `011` stays `match` | at 137 | **`match` at 142** — verdict right, number wrong |
| `019` stays `match` | at 140 | **`match` at 142** — verdict right, number wrong |
| `003` centre x within 6 pt of 408.84 | | **408.81 — right, and 0.03** |
| `003` centre y within 1 pt of 464.74 | | **464.81 — right** |
| `003` radius within 12 pt of 99.78 | | **100.01 — right, and 0.23** |
| title baseline 592.2 ± 0.1 | | **592.20 on all four — right** |
| regressions among the other 58 `done` titled-chart documents — "most likely to be wrong" | **0** | **0 — right** |
| page counts on our side | 0 | **0 — right** |
| tests | +10 to +25 | **+11 — right** |
| `MANIFEST.tsv` rows | 2 | **2 — right** |

The one miss is a systematic and it is the same one round 60 recorded: all four documents come out
**exactly two words over** the reference. The chart straddles the A4 page break and the labels that
now sit where the reference's sit are duplicated onto the page-2 sliver one token differently. Two
words on 140 is inside the 2% band on all four, which is why four verdicts came out right off a
number that was wrong four times.

## 6. Tests: **+11**, and five mutations

`XRay.Office.Core` **376 → 387**. Re-derived by running each project: Containers 109, Core 387,
Markup 259, OpenDocument 125, Presentations 846, Rendering 153 (+1 skipped), Spreadsheets 1020,
Text 624, Vector 302, WordProcessing 1220 — **5045 passed, 0 failed, 1 skipped**.
`dotnet build -v q -nologo` → **0 warnings, 0 errors**.

| mutation | outcome |
|---|---|
| pass 1 runs at the full diagram (the pre-round arithmetic) | **detected** — `APiesFirstPassIsLaidOutAtOneTwoPointTwothOfTheDiagram` |
| the reduction factor is 1.1 instead of 2.2 | **detected** — the same test |
| the pen drops the flat 135 | **detected** — 5 of the title theory's cases |
| the pen drops the text shape's upper inset | **detected** — 5 of the title theory's cases |
| `adjustPosAndSize` squares *before* it intersects | **NOT detected — and it is an equivalent formulation, not a drift guard.** § 3 shows the intersection is unreachable: the reduced rectangle is always strictly inside the available one |

The pie test is a **discriminator and not a golden number**: a twenty-glyph unbreakable token is
100 pt wide under the stand-in ruler, whose diagonal clears `0.975 × 140.1` and does not clear
`0.975 × 63.7`, so the label fits inside its slice under a full-sized first pass and not under a
reduced one. Its four-glyph sibling is the control and must not shrink at all. The two title
theories **are** drift guards — they restate the arithmetic — and are labelled as such at the site;
what gives them teeth is that dropping either term fails them.

## 7. Shared layer — this diff is `XRay.Office.Core` and the parent owes a cross-track sweep

`ChartLayout.cs` and `ChartLayout.PieLabels.cs` are reached by `SheetChart`, `SlideChart` and
`FrameChart` alike. `census-piestitles.py`, both readers, all **946** manifest paths, case-folded
where it accumulates:

| | sheets | slides | words |
|---|---:|---:|---:|
| documents with a **titled** chart (the title change) | **62** — 54 `done`, 8 `open` | 14 | 3 |
| documents with a **best-fit pie** (the first-pass change) | **7** — 4 `done`, 3 `open` | 5 | 2 |
| BIFF documents holding a chart substream, undecoded (upper bound on both) | 7 | 0 | 1 |

Named for the pie change, because it is the small set — slides `bitesize-writing-a-report.pptx` and
`3495.pptx` are `done`; words `pie-chart-result.docx` and `pie-chart-template.docx` are `done`;
slides `076_Hexadonut…`, `031_Alarm_Clock_Pie-Chart…` and `061_Four-Stage_Serpentine_Chart…` are
`open`. **Falsifiable prediction for the parent: 0 verdicts move on words and 0 on slides.** The
title change moves 17 non-sheets documents' title text down 5–15 pt and the pie change re-lays-out 7
non-sheets pies; neither can change a page count, and on a slide a label cannot cross a boundary at
all.

## 8. The vision round — three fresh reviewers, and **two new findings both confirmed by a second
instrument**

Three subagents, one composed pair each at 200 dpi, `Read` on one image path only, no project
documents, no source, no shell, each asked to describe the halves separately, give a direction, and
say what looked identical. **No page was chosen by `--worst`.**

### `003_advanced_excel_pie` p1 — chosen because it is the document the round set out to close

> *"The M4 label: identical text, identical two-line wrap, identical position in both halves."*
> *"The pie itself: same centre y, same page-relative centre x (within ~1 px), same radius
> (~172 px), same start angle, same clockwise slice order, same five colours, same slice angles."*

The pie geometry is now reported as **identical** by a reader who had never seen the page — where
round 60's reader on the same page measured *"about 45 px further right"*. `pdf-ops.py` agrees:
0.03 pt of centre.

The reviewer's first-ranked *difference* is real and is the next item on this track:

> *"The M3 data label is placed completely differently … in the left half it sits outside the pie,
> below it, as one line with a green legend swatch; in the right half the same label sits inside
> the yellow-green slice as two stacked lines, with no swatch."*

Confirmed by a second instrument on the same objects: `pdf-ops.py` counts **two** small isolated
key squares over our pie against the reference's **one** — a ghost key is exactly what
`performLabelBestFitInnerPlacement` leaves behind when a label is rebuilt outside, so our inner-fit
test rejects one label of five that the reference keeps. That is also where the two surplus words
come from. The reviewer counted the squares as 2 against 1 unprompted and flagged the count as
*medium-low* confidence; the instrument says she was right.

Also reported and **confirmed**: *"the M1 label … stops after 'M1; Actual;' … cut off at roughly the
chart frame's right border"*, which is the A4 MediaBox, and the chart title *"may sit about 2 px
lower"* on the reference, **flagged low confidence** — 0.33 pt is 0.55 px at this composite's scale,
so the direction is right and the magnitude is at the edge of the instrument, exactly as she said.

### `005_Contextures_chart_sample_6e279b08` p1 — chosen because it holds three best-fit pies, passes the gate, and round 60 left three unconfirmed leads on it

Two of the three are now settled, in opposite directions.

**A. The chart area's default border is real, and round 60's refutation was of the wording.** The
reviewer:

> *"The right half draws a thin light-grey outline rectangle around each of its three chart
> objects. The left half draws no such outline around any of its three charts … note that both
> halves do draw the inner plot-area rectangle and gridlines; the difference is only the outer
> chart frame."*

That distinction is what round 60's stroke count could not make. `pdf-ops.py`: the reference draws
**exactly three `#D9D9D9` strokes** on that page and we draw **zero** — one per chart, at
(74.78, 393.68)-(315.95, 620.16), (401.78, 372.05)-(654.52, 543.83) and
(401.78, 547.85)-(655.26, 727.62). Round 60 read 63 strokes against 66 and correctly said "none" was
wrong as stated; **the three it could not account for are the three chart frames.** And the control
separates the rule from the observation: on all four `advanced_excel_pie` documents both sides draw
**one** `#D9D9D9` stroke, because those chart parts state a `c:spPr`. **The rule is that 26.2.4.2
draws a light-grey chart-area border when `c:chartSpace` states no `c:spPr` at all, and we draw one
only when the file states a line.** A third reviewer, on an unrelated document, reported the same
thing independently — § below.

**B. Our pie data labels are black where the reference draws them white, and the file says so.**

> *"The left half's pie data labels are bold black; the right half's are white … this is the single
> most obvious difference."*

Round 60's reviewer said the same and round 60 filed it **not yet checked**, because `pdf-ops.py`
does not expose a text fill. Two instruments now do. The file: `005`'s pie states
`<c:dLbls><c:txPr>…<a:defRPr sz="1400" b="1"><a:solidFill><a:schemeClr val="bg1"/></a:solidFill>`
— white, and bold, which is why round 60's refutation of the *weight* half was also right. The
raster, over the pie's own rectangle at 150 dpi: ours **4203** dark pixels and 410 light non-page
pixels, the reference **720** dark and **3053** light. `ChartDataLabel` carries **no colour field at
all** and `ChartLayout` draws every label in a hardcoded `AxisColour = Colour.Black`.

### `microsoft_learn_multi_chart_examples` p1 — chosen because it is an `open` document with a best-fit pie whose verdict did *not* move

A third, independent reviewer, on a document neither of the others saw:

> *"Each of the three charts on the right half is drawn inside a visible thin light-grey
> rectangular border … the left half's three charts have no border at all — only the axis lines
> and gridlines."*

`pdf-ops.py`: reference **3** `#D9D9D9` strokes, ours **0**. **Two readers, two unrelated documents,
the same object, and an instrument that can measure it agreeing on both.** That is the shape of
reading this project has recorded as its strongest, and it is the first time it has arrived twice
for the same rule.

Her other findings are **leads, not findings**, and are recorded as such: the reference's category
axis is far wider (*"the right half's plotted data stops roughly 130 px short of where the left
half's data is still continuing"*, and its first category is inset from the axis where ours sits on
it) — the same claim round 60's reader made about `005` and still unmeasured; the reference draws
**no title** on its clustered-column chart where we draw one, which is the *opposite* direction to
everything else here and worth a look; and a stacked-area band that is green on our side and blue
on the reference's.

### Reach of the two new findings, over all 946 documents

`census-framelabelcolour.py`:

| | sheets | slides | words | BIFF, undecoded |
|---|---:|---:|---:|---|
| chart parts with no `c:chartSpace` `c:spPr` (the default border) | **10 documents / 23 parts** — 9 `done`, 1 `open` | 2 / 5 | 1 / 1 | 64 / 51 / 66 documents |
| `c:dLbls` stating a text colour | **22 documents / 40 parts** — 14 `done`, 8 `open` | **49 / 93** | 7 / 7 | as above |

The label colour is the larger of the two and it is **mostly a slides item** — 49 decks — which is
worth saying on a sheets round rather than leaving for the track that trips over it.

## 9. The 24.2.7.2 audit — one site re-checked, **verified**, and the counters re-derived

Re-derived at this tree with the file's own commands: **37 open hits in 26 files**; **26 marker
lines — 21 `VERIFIED`, 4 `FIXED`, 1 `WRONG`, 0 `UNDECIDED`**. That is the brief's 37/21/4/1 exactly.
(The naive `git grep -l '24\.2\.7'` gives **30** files, because four of them carry only marker
lines; the file's own rule is to count files over the *open* sites, which gives 26.)

**`XRay.Office.Ooxml/DrawingML/DrawingFill.cs`:115 — `a:lum` as a whole per cent — VERIFIED on
26.2.4.2.** The site claims
`getLimitedValue<sal_Int16>(value / PER_PERCENT, -100, 100)`, i.e. **truncation** to whole per cent,
and that the washout branch tests that integer for exactly 70 and −70.

The whole discriminator lives on the reference's side, so `audit_lumpercent.py` never runs our
renderer: five minimal `.docx` packages around an authored saturated red/blue checkerboard, one
`a:lum` value each, compared by the mean channel of the rendered page.

| case | mean R / G / B |
|---|---|
| no `a:lum` (control) | 234.447 / 213.926 / 234.444 |
| `70000 / −70000` | 251.842 / 248.718 / 251.841 |
| `70999 / −70999` | **251.842 / 248.718 / 251.841 — identical** |
| `69500 / −69500` | 250.795 / 246.624 / 250.794 |
| `71000 / −71000` (control) | 251.117 / 247.268 / 251.116 |

`70999` renders **identically** to `70000`, which only truncation can do; `69500` renders
**differently**, which rounding could not. **The two live cases fail under the two candidate
readings in opposite directions**, so neither is a one-sided test, and both controls came out as
they had to before either was read. The washout itself is visible in the numbers — `WATERMARK` is a
near-neutral pale wash where `applyBrightnessContrast` is not. C#'s integer division truncates
toward zero for a negative operand exactly as C++'s does, so the contrast half needs no second arm.

`XRay.Office.Ooxml` is now **one of one** re-checked. The site is marked at the file and the outcome
table and the progress table in `TODO.24-2-7-audit.md` are updated.

## 10. `MANIFEST.tsv`

Lives in the corpus repository and was **not touched**. Two rows change, both gains:

| path | proposed |
|---|---|
| `sheets/chartset-002/xlsx/003_advanced_excel_pie.xlsx` | `open` → **`done`** (145/143) |
| `sheets/chartset-002/xlsx/027_advanced_excel_pie.xlsx` | `open` → **`done`** (142/140) |

`ans_mappings_of_eccairs_terms.xlsx` matched on both sweeps this round; round 60's proposal to move
it to the `unstable` treatment still stands on its own evidence and this round adds nothing to it
either way.

## 11. What the next round should do first

1. **Our inner-fit test rejects one label of five that the reference keeps** — `003`'s M3, found by
   a blind reader and confirmed by the ghost-key count (2 against 1). It is what makes all four
   documents come out two words over, and `BestFitInner` is a port whose *inputs* are our own text
   measurements. Small, localised, and the last of the pie geometry.
2. **A data label's stated text colour** — `c:dLbls/c:txPr//a:defRPr/a:solidFill`, **22 sheets
   documents / 40 parts, 49 slides / 93, 7 words / 7**. `ChartDataLabel` carries no colour field and
   `ChartLayout` draws every label in a hardcoded black. Two readers, two rounds, and now three
   instruments.
3. **The chart area's default `#D9D9D9` border** where `c:chartSpace` states no `c:spPr` — 10 sheets
   documents / 23 parts, 2 slides, 1 words. Three readers across two rounds, measured to the stroke,
   with a four-document control that separates the rule from the observation.
4. **`cellIs` conditional formatting** — 123 rules in 18 documents, two arms, unchanged since round
   59 § 9.
5. **The 0.33 pt title residual and the +0.24% pie radius**, both of which are now the *only* things
   left on their quantities and both of which look like absolute offsets in the chart device's
   ascent and half-width rather than in the layout.
6. **The same 96 dpi law for `SlideChart` and `FrameChart`** — still untouched, still a cross-track
   change, still needs a census and a say-so.
