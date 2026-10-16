# Round 59 — sheets — prediction

Committed **before a line of the change was written and before anything was rendered
post-change**. Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch
`wt-sheets-r59`, base `dc9ca5900c2`.

## 0. The baseline is 277, not the briefed 276, and only the reference side moved

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 363 MATCH 301 MISMATCH 62`; scored against
`MANIFEST.tsv`'s 307 sheets paths by round 58's `score.py`, which refuses to print unless every
manifest path found a row: **277 match / 30 mismatch**.

Differenced document by document against round 58's final sweep, **exactly one row moved and it is
the reference half**: `unstable-001/xlsx/fse_identification_form.xlsx`, `REF words 427 → 440`, our
side pinned at 440 in both. That is the calendar volatility round 57 separated and round 58
recorded on this very document — the reference's clock has caught up with ours. No code moved it
and nothing else moved at all. The comparable-to-round-58 figure is **276**; the live figure is
**277**, and every number below is against 277.

`TOTAL` was **363** here where round 58's first two sweeps said 325 — the alias count is not
static, exactly as `CLAUDE.md` now records. Distinct lower-cased paths: 307.

## 1. What is being changed

### A. The pie `bestFit` data label — `XRay.Office.Core/Charts/ChartLayout.cs`, **shared layer**

Round 58 left "our pie is 18% larger than the reference's, 235.8 pt against 199.8" and "the
reference wraps each label onto two lines". Both were measured again before any code was read.

**The 18% is refuted and it is an instrument artefact** (§ 4 of the results). `pdf-ops.py`'s
bounding box includes a bezier's *control points*, and we emit one cubic per arc segment where
LibreOffice emits a polygonised arc, so our wedge boxes bulge outside the circle by up to 8.1 pt
and the reference's by 0.11. Read off the wedge corner and the vertical radius instead — the
first wedge lies wholly in the upper-right quadrant, so its box's lower-left corner *is* the pie
centre — the true figures on `003_advanced_excel_pie` page 1 are

| | ours | reference |
|---|---:|---:|
| pie centre | (402.48, 461.56) | (408.84, 464.74) |
| radius | 110.62 | 99.78 |
| diameter | 221.24 | **199.56** |

so we are **10.9%** larger, not 18%. **The centre offset reproduces exactly** — 6.36 pt left and
3.18 pt low, against the briefed 6.4 and 3.2.

`probe-pieradius.py` renders sixteen one-variable rewrites of the witness's own chart part through
the installed binary. The law:

* `dLblPos="ctr"` and `"inEnd"` give radius **110.44** — which is our 110.62 to 0.16%, so **our
  geometry is LibreOffice's `ctr` geometry and it is right for `ctr`**.
* `"bestFit"` and `"outEnd"` give radius **99.78** and centre (408.84, 464.74) — *identical to each
  other, to the digit*.
* Short label text (`04-cat-only`, `05-val-only`, `15-cat-only-16pt`) gives **110.44**: the shrink
  is not a constant, it is driven by what the labels consume. 8 pt labels give 101.28, 16 pt give
  72.08.
* `01-nolabels` 110.44, `03-notitle` 121.61, `12-bare` 131.44 — the controls separate the label
  term from the title and legend terms.

The mechanism, corroborated afterwards in `PieChart.cxx` and `ChartView.cxx`: `bestFit` is
`AVOID_OVERLAP`, which is drawn as `CENTER` and then run through
`performLabelBestFitInnerPlacement`; a label that will not fit inside its slice is **rebuilt at the
`OUTSIDE` anchor**, and the pie/donut branch of `impl_createDiagramAndContent` then **recreates the
whole diagram at `adjustInnerSize(consumedOuterRect)`**. Four measurements confirm the details
against the reference's own page before any of that was read:

* the label's legend key is `int(fontHeight × 0.6)` = 211 mm100 = **5.98 pt** — measured 5.98;
* the gap from key to text is `key + max(100, fontHeight × 0.22)` = 311 mm100 = **8.818 pt** —
  measured 8.81;
* the `OUTSIDE` anchor is the rim point on the bisector plus **150 mm100 = 4.252 pt** radially —
  predicted x 462.87 against a measured 462.90, and on all five slices of the `outEnd` variant the
  four alignment families place the block's left or right edge on that anchor to **0.12 pt**;
* the block's height is **n × (ascent + descent)** with the first baseline at `blockTop − ascent`,
  which lands all five `outEnd` baselines to **0.09 pt**. The vertical text inset is *not* in it,
  and the model that includes it misses by 4.5 pt.

**And a LibreOffice quirk that identifies the mechanism from outside**: the reference draws **six**
label keys for five labels on this chart. `xShapes->remove(xTextShape)` in the outside fallback
removes the text and leaves the key of the discarded inner attempt behind. We currently draw
**none**.

### B. `cellIs` conditional formatting — `XRay.Office.Spreadsheets`, sheets only

18 documents, 123 rules. Second, and only if A lands with time to sweep it.

## 2. The documents I expect to change, and the verdict movement

**Verdict movement predicted: +4, from 277 to 281. Every one of the four is an
`_advanced_excel_pie`.**

The gate's word rule is `d > ref×0.02 && d > 3` — **both** conditions. All four sit at `d = 5`, so
**two words** flips each of them:

| document | ours/ref now | needs | where the words are |
|---|---|---|---|
| `chartset-002/xlsx/003_advanced_excel_pie.xlsx` | 138/143 | ≥ 140 | page 2 |
| `chartset-002/xlsx/011_advanced_excel_pie.xlsx` | 135/140 | ≥ 137 | page 2 |
| `chartset-002/xlsx/027_advanced_excel_pie.xlsx` | 135/140 | ≥ 137 | page 2 |
| `chartset-004/xlsx/019_advanced_excel_pie.xlsx` | 135/140 | ≥ 137 | page 2 |

**The whole gap is on page 2 and page 1 already agrees exactly** (39/39, then 12/17, then 87/87 on
`003`). The chart straddles a horizontal page break; the reference's M1 label is rebuilt at the
`OUTSIDE` anchor, which puts its origin at x = 471.71 on page 1 and **x = 8.94 on page 2**, inside
the MediaBox, so `pdftotext` decodes all four of its tokens there. Ours sits at 390.43 → −72.19 and
yields one. That single label is worth **+3**; the wrap of M2 and M3 is worth another +2.

Controls that bound it, all rendered through the installed binary on rewrites of the witness:
`ctr` → 139 words, `inEnd` → **138 — our exact figure**, `bestFit` → 143, `outEnd` → 146. So
**`outEnd` for every label overshoots the band as surely as `ctr` undershoots it**; the prediction
depends on the inner-fit test keeping four labels in and moving one out.

### Regression risk, named

| document | why it is at risk | today |
|---|---|---|
| `sheets/chartset-006/xlsx/005_Contextures_chart_sample_6e279b08.xlsx` | the only other sheets `bestFit` | match |
| `sheets/done-010/xlsx/Keywords_Mapping_Graphs_and_Charts.xlsx` | `showLegendKey=1`, no `bestFit` | match (4511/4519) |
| `slides/chartset-006/pptx/031_Alarm_Clock_Pie-Chart…pptx` | `bestFit` ×2 | — |
| `slides/done-007/pptx/bitesize-writing-a-report.pptx` | `bestFit` ×2 | — |
| `slides/done-011/pptx/3495.pptx` | `bestFit` ×1 | — |
| `words/chartset-001/docx/pie-chart-result.docx` | `bestFit` ×2 | — |
| `words/chartset-001/docx/pie-chart-template.docx` | `bestFit` ×2 | — |

**`cellIs` is predicted to move zero verdicts.** It paints fills; the gate counts words, pages and
fonts. `colorScale` moved zero and touched 423 cells in 36 documents.

## 3. Numbers I am committing to

| | predicted |
|---|---|
| sheets verdicts | **277 → 281** |
| page counts, anywhere | **0 change** |
| `003_advanced_excel_pie` words | 138 → **between 140 and 145** (band [140.14, 145.86] by the 2% arm, [140, 146] by the `d>3` arm) |
| its pie radius | 110.62 → **99.78 ± 0.5** |
| its pie centre | (402.48, 461.56) → **(408.84, 464.74) ± 0.5** |
| label legend keys drawn on `003` p1 | 0 → **6** (five labels; the sixth is the reference's own discarded inner attempt) |
| documents whose ink changes, sheets | **5** (four pies + `005_Contextures`) |
| `Keywords_Mapping…` | keys appear; word count unchanged |
| tests | **+8 to +18**, `XRay.Office.Core` |
| `MANIFEST.tsv` | **4 rows change status**, all `open` → and the parent decides `done` |

## 4. What the census cannot see — the blind spots, stated before the sweep

1. **The ODF arm.** `chart:data-label-symbol` and ODF's own best-fit placement were not censused
   at all. The corpus holds no ODF chart with a pie label that the reach census can see, but the
   census only walked OOXML `c:` elements, so **the ODF reader's behaviour after this change is
   unmeasured**.
2. **`.xls` charts.** `XlsChartReader` synthesises a `ChartPlot` and the census read `c:dLblPos`
   only, which a BIFF chart does not have. A BIFF `DATAFORMAT`/`ATTACHEDLABEL` best-fit placement
   would be invisible here.
3. **Inheritance.** The census counted `c:dLblPos` where it is *stated*. A pie whose labels take
   `bestFit` from a `c:dLbls` at series or chart level with no `c:dLblPos` at all resolves to the
   OOXML default, which for a pie is `bestFit` — **so the true reach is a floor, not a ceiling**,
   and the 16 sheets pie parts against 5 bestFit documents is where that gap would show.
4. **The inner-fit test is a port, not a fit.** `performLabelBestFitInnerPlacement` is being
   transcribed from the C++ and its *inputs* (the label's own box) are measured on our text
   measurer, not LibreOffice's. Two ports of one formula can disagree because one was written from
   the other's prose. If our box is a little wider, a label that the reference keeps inside goes
   outside and the count overshoots toward the `outEnd` figure of 146.
5. **The shrink is iterative in the reference and one-shot here.** `adjustInnerSize` is applied
   once by `impl_createDiagramAndContent` for pies; if the corpus holds a chart where the reference
   needs the axis-label loop as well, this will not reproduce it.
6. **`c:dPt`, still unimplemented on bars**, is on the same page of `003_Contextures…xlsm` and is
   not touched by this round.

## 5. Two censuses that had never been run, and both came back negative for `colorScale`

Both are in `census-blindspots.py`, which reads all **946** manifest rows and refuses to summarise
unless every one produced output.

* **`.xls` `CF12` (0x087A), read for the first time on this project**: **42 records in 2
  documents**, and **all 42 are `ct=1`, `cellIs`**. Plus 18 `CONDFMT12` in one document. **There is
  no `.xls` colour scale, data bar or icon set anywhere in this corpus** — round 58 guessed that by
  construction and it is now measured.
* **The `x14` extension arm**: **17 blocks, 27 rules, 11 sheets documents** — **9 `dataBar` and 18
  `iconSet`, and zero `colorScale`**. So round 58's colour-scale reach of 38 documents / 43 rules
  is the whole corpus figure and not an OOXML-arm subset. What the arm *does* add is eleven
  documents' worth of `dataBar`/`iconSet` that no census here had seen.
