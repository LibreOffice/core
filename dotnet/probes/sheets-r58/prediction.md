# Round 58 — sheets — prediction

Committed **before** a line of the change was written and before anything was rendered
post-change. Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch
`wt-sheets-r58`, base `32f946bf612`.

## Baseline, reproduced

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 290 MISMATCH 35`. Scored against
`MANIFEST.tsv`'s 307 sheets paths (`score.py`, which refuses to print unless every manifest path
found a row): **276 match / 31 mismatch**, exactly the briefed 276. No `done` document mismatched.
`fse_identification_form.xlsx` reads 440/427 — our side pinned, the reference's clock 13 words
behind, the same calendar volatility round 57 separated.

## The target: `cfRule type="colorScale"`

Measured before predicting, and both halves have a control that ran first.

* **`probe-colorscale.py`, sixteen authored workbooks.** `00-control-none` (the same sheet with
  no `<conditionalFormatting>` at all) reads **0** scale fills, so the instrument can return zero;
  `01-control-solid` (an ordinary solid fill on the same twelve cells) reads **one** rectangle of
  the stated colour, so it can find something — and that single rectangle is itself the
  measurement that LibreOffice coalesces a run of identically-filled cells, which is why a
  per-cell count is not the right instrument.
* **The interpolation law**, read off `003_advanced_excel_pie`'s reference PDF before any source
  was consulted: `channel = c1 + (int)((v − v1)/(v2 − v1) × (c2 − c1))`, a truncation **of the
  delta**, which is why an increasing channel floors and a decreasing one appears to ceil. Exact
  on 36 of 36 channel values of that document's twelve fills, and on every authored case.
  `sc/source/core/data/colorscale.cxx:599` says the same thing, and is corroboration, not the
  measurement.
* **The 27.2-alpha tree in this checkout says a colour scale is only applied when some *other*,
  style-named condition also matched** (`fillinfo.cxx:776`, `if (bAnyCondition && …)`). **That is
  not 26.2.4.2's behaviour**: `02-two-minmax`, whose only conditional formatting is a colour
  scale, draws eleven interpolated fills, and so does `003_advanced_excel_pie`. The binary wins.
* **Priority decides, not document order** — a discriminating pair, `14` and `15`, the same two
  overlapping scales with the document order reversed. The `priority="1"` rule paints in both;
  the loser's blue→magenta ramp appears in neither.
* A colour scale **replaces** the cell's own solid fill (`07-with-own-fill`), paints **nothing**
  on a non-numeric cell (`08`, 0 of 11) or an empty one (`09`, 6 of 11), and does **not** extend
  the printed area when its range runs past the data (`10`, range to row 40, data to row 12,
  still one page).

## The census, and what it resolves to rather than declares

`census-colorscale-reach.py` over the 243 xlsx-family manifest rows; 243 produced output, 0
failures. It predicts each cell's colour by the law above and then looks for that colour in the
stored reference and in our own rendering.

| | |
|---|---:|
| documents carrying a `colorScale` rule | **38** |
| rules | 43 |
| cells that **resolve** to a fill | **423** |
| documents that gain ink | **36** |
| documents that declare a scale and resolve to **nothing** | **2** |

The two that resolve to nothing are the lesson: `Data-Architecture-Tool-Fit-Assessment-Template`
declares eight colour-scale rules over `C6:J66` — 488 declared cells — and paints **zero**,
because every cell in the range holds text (`"3 (Neutral)"`), and `ScColorScaleFormat::GetColor`
returns nothing for a cell that is not numeric. `036_Simple_to-do_list` declares one over `F6:F19`
with `type="formula"` string operands and is likewise all text. A census over `sqref` would have
predicted 500 cells where 423 is the answer, and would have named the wrong documents.

Of the 36, **34 are the `NNN_advanced_excel_*` chart workbooks**, each with one three-colour scale
over twelve numbers on its data sheet; the census finds **all twelve predicted colours in the
reference PDF and none of them in ours**, on all 34. `003_Contextures_chart_sample_9bda2719.xlsm`
has eleven, and is the one document where we already draw those colours — twice each, against the
reference's four — because the workbook also states them as static fills, so its post-check is a
**count**, 2 → 4 per colour, not a presence test. `075_Idea_planner_tasks` has four cells whose
stops are `theme`+`tint`; the census declines to compute those and the implementation will take
them through `XlsxTint`, so those four are predicted **unverified by this census**.

## What I predict

| | prediction |
|---|---|
| sheets verdicts | **276 → 276 of 307. Zero movement, either way.** |
| page counts, anywhere | **0 change** |
| word counts, anywhere | **0 change on all 307.** A fill draws no text |
| documents whose ink changes | **36**, all xlsx-family; 34 of them the `advanced_excel` cluster |
| cells that gain a fill | **423**, of which 419 have a colour this census predicts exactly |
| `003_Contextures…xlsm` | each of eleven scale colours goes **2 → 4** fills on our side |
| `003_advanced_excel_pie` | **0 → 12** scale fills, colours exactly the reference's twelve |
| the two that declare and resolve to nothing | **0 fills**, before and after |
| words / slides tracks | **0** — no shared layer touched (see below) |
| tests | **+6 to +12**, `XRay.Office.Spreadsheets` only |
| `MANIFEST.tsv` | no row changes status |

Zero verdict movement is the honest answer and I am stating it as the headline rather than as a
hedge: the gate is page count, extractable words and font embedding, and a cell background is
invisible to all three.

## What this census cannot see — written down before the sweep

1. **`.xls` colour scales are not censused at all.** The BIFF arm counts `CONDFMT` (0x01B0) and
   `CF` (0x01B1) and finds 187 CF records in 5 of the 64 `.xls` documents — but a colour scale is
   **not expressible in those records**. It lives in `CF12` (0x087A) and the `XFEXT` futures
   records, which this census does not read. So the `.xls` figure is a lower bound of unknown
   tightness, and the change will draw nothing there either way. **This is blind spot 1 and it
   has fired in each of the last three rounds.**
2. **`x14` extension rules.** Only `cfRule` in the main SpreadsheetML namespace is counted. A
   document whose colour scale exists only inside `<extLst><x14:conditionalFormattings>` is
   invisible here, and `036_Simple_to-do_list` proves the shape is present in this corpus (its
   `dataBar` carries an `x14:id`).
3. **`.ods` / `.fods` / `.xlsb`.** The sheets corpus holds none of the three — 241 `xlsx`, 2
   `xlsm`, 64 `xls` — so `calcext:color-scale` and BIFF12 `CFRULE` are unmeasured in both
   directions. `OdsCellDecoration` and `XlsbSheetReader` will still draw nothing.
4. **Cached values.** The census and the reader both read `<v>`. LibreOffice recalculates. A
   colour scale over a volatile formula (`TODAY()`, `RAND()`) would diverge, and 16 of the 40 open
   sheets documents are already known to carry volatile dates.
5. **Booleans.** `t="b"` cells are excluded here; Calc's `hasNumeric()` is true for them. No
   corpus rule was seen over a boolean column, which is not the same as there being none.
6. **Tint.** Four cells in one document take their stops from the theme with a `tint`. The census
   cannot compute them; only the rendering will say whether `XlsxTint` puts them where the
   reference does.
7. **Interaction with the rule types we still do not draw.** Where a `cellIs` dxf fill and a
   colour scale cover one cell, 26.2.4.2 lets the **colour scale** overwrite the style's
   background (`fillinfo.cxx:776` runs after the `ATTR_BACKGROUND` block). We implement neither
   half today, so this cannot be wrong yet — but it is the first thing the `cellIs` round will
   have to get right, and this round does not measure it.
8. **The print area is the regression risk, not the fill.** Our `SheetDecorationArea` decides how
   far a sheet prints from `SheetFormatting`'s stated cells, rows and column runs. If a
   conditional fill were interned into any of those three, a rule declared over `N18:Q1048576`
   would extend the printed area and move **page counts** — the one way this change could break a
   `done` document. The design keeps the overlay out of all three, and fixture `10` is the
   measurement that the reference does not extend either; but that fixture tests one shape and
   the corpus has others.

## Shared layer

**No.** Every file the change touches is in `XRay.Office.Spreadsheets`. `XRay.Office.Core` is read
(`Colour`, `Length`) and not modified. The words and slides tracks cannot see it and no
cross-track sweep is owed. I will re-derive this from `git diff --stat` before reporting it.
