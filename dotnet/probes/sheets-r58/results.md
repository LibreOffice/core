# Round 58 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r58`, base
`32f946bf612`. Read `prediction.md` (`e4e6b22402d`) beside this file first — it was committed
before a line of either change was written and before anything was rendered post-change.

## 1. Baseline, reproduced exactly

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 290 MISMATCH 35`. Scored against
`MANIFEST.tsv`'s 307 sheets paths by `score.py`, which refuses to print unless every manifest path
found a row: **276 match / 31 mismatch**, the briefed figure to the document. No `done` document
mismatched. `fse_identification_form.xlsx` reads 440/427 — our side pinned, the reference's clock
13 words behind — which is the calendar volatility round 57 separated, not movement.

## 2. Result

**276 → 276 → 276 of 307 across two changes and three full-track sweeps. Zero verdict movement,
which is what the prediction file said.** Corpus 795 of 946.

| | base | after `colorScale` | after the chart face |
|---|---:|---:|---:|
| sheets verdicts | **276** | **276** | **276** |
| colour-scale fill rectangles, ours | 22 | **441** | 441 |
| the same, reference | 452 | 452 | 452 |
| documents drawing all their predicted scale colours | 1 of 36 | **35 of 36** | 35 of 36 |
| `003_advanced_excel_pie` chart title | 13.00 pt Liberation Sans | — | **18.00 pt Carlito Bold** |
| its embedded font list | …+LiberationSans | — | **the reference's exactly** |
| our word counts that moved | — | **0** | **10, every one toward the reference** |
| page counts, anywhere | — | 0 | 0 |
| tests, ten non-Fidelity projects | 4855 / 0 / 1 | — | **4868 / 0 / 1** |

## 3. `colorScale`: the law was read off the reference's own page, and it is a truncated *delta*

`003_advanced_excel_pie`'s sheet 2 states one three-stop scale over `B2:B13`, holding 93 to 170 in
steps of seven. Its reference PDF draws twelve fills on page 3 — `#F8696B`, `#F9806F`, `#FA9874`,
`#FBAF78`, `#FDC77D`, `#FEDF81`, `#F1E784`, `#D5DF82`, `#B9D780`, `#9CCF7F`, `#80C77D`, `#63BE7B` —
and we drew none. Fitting those twelve before opening any source gives

```
channel = c1 + (int)((v − v1) / (v2 − v1) × (c2 − c1))
```

**exact on 36 of 36 channel values**, and it is exact only because the truncation is on the
*delta*: a rising channel floors (the second cell's green is 128.64 → 0x80) and a falling one
truncates toward zero, so it appears to round up (the seventh cell's red is 240.82 → 0xF1).
Rounding the sum satisfies the second and not the first; flooring the sum satisfies the first and
not the second. `sc/source/core/data/colorscale.cxx:599` says the same thing and is corroboration,
not the measurement.

### The fixtures, controls first

`probe-colorscale.py`, sixteen authored workbooks rendered through the installed binary.

| case | what it varies | reference |
|---|---|---|
| `00-control-none` | **no rule at all** | **0 fills** — the instrument can answer nothing |
| `01-control-solid` | a stated solid fill on the same twelve cells | **1 rectangle** of the stated colour |
| `02-two-minmax` | a two-stop scale, nothing else in the workbook | 11 fills |
| `03-three-mid50` | the corpus witness's own stops | the witness's twelve colours |
| `04-num-2-8` / `11-formula-cfvo` | `num` against a numeric `formula` | **identical**, 7 rectangles each |
| `05-percent-25-75` | `percent` stops | 7 — the clamped ends coalesce |
| `06-percentile-90` | a percentile upper stop | 10 |
| `07-with-own-fill` | the cells state a solid green | **11 scale fills and no green** |
| `08-text-in-range` | eleven text cells | **0 of 11** |
| `09-mixed-blanks` | six numbers among five blanks | **6**, and the six colours are `02`'s alternate entries |
| `10-range-past-data` | the rule reaches row 40, data reaches row 12 | 11 fills, **still one page** |
| `12-three-num` | three `num` stops with the midpoint on a cell | `#FFEB84` exactly |
| `13-negatives` | −5…5 | identical ramp to `02` |
| `14`/`15-two-scales(-swapped)` | **a discriminating pair** | see below |

`01-control-solid` is worth its own sentence: it reads **one** rectangle where twelve cells state
the same fill, because LibreOffice coalesces a run. That is why the post-check below counts
rectangles against the reference's own count rather than against the cell count.

### Two things the C++ in this checkout gets wrong about the running binary

1. **`fillinfo.cxx:776` applies a colour scale only when some *other*, style-named condition also
   matched on the sheet** — `if (bAnyCondition && pInfo->mxColorScale)`, and `bAnyCondition` is
   raised only by `aData.aStyleName`. On 26.2.4.2 a workbook whose *only* conditional formatting is
   a colour scale draws it: `02-two-minmax` gives eleven fills and `003_advanced_excel_pie`, which
   states nothing else, gives twelve. **The tree here is 27.2.0.0.alpha0+ and the binary wins.**
2. **Priority decides, not document order.** Case `14` alone cannot separate the two, because its
   winner was both last in document order and `priority="1"`. Case `15` is the same two rules with
   the order reversed and the same red→green ramp paints; the loser's blue→magenta ramp appears in
   neither rendering.

### The census: what a rule *resolves to*, not what it declares

`census-colorscale-reach.py` over the 243 xlsx-family manifest rows; 243 produced output, 0
failures. **38 documents carry a `colorScale`, 43 rules, and 423 cells resolve to a fill in 36
documents.** Two documents declare one and paint nothing:

* `Data-Architecture-Tool-Fit-Assessment-Template` declares **eight** scales over `C6:J66` — 488
  declared cells — and every cell in the range holds text (`"3 (Neutral)"`), so
  `ScColorScaleFormat::GetColor`'s `hasNumeric` guard returns nothing for all of them.
* `036_Simple_to-do_list` declares one over `F6:F19` with `type="formula"` string operands, also
  all text.

A census over `sqref` would have said 500 cells and named the wrong documents. **That is the
"resolves to, not declares" rule firing on its own terms**, and it is the reason the prediction
said 423 and 36 rather than 500 and 38.

### Post-check, and the one shortfall, which is not conditional formatting

Counting fill rectangles of the predicted colours over the 35 documents whose colours the census
can compute: **reference 452, ours 22 before, ours 441 after.** Thirty-four documents go from 0 of
12 predicted colours present to 12 of 12, positions agreeing with the reference to **0.03 pt**
(`003_advanced_excel_pie` page 3, twelve rectangles at x 102.13–150.26 against 102.10–150.26).
`075_Idea_planner_tasks`, whose stops are `theme`+`tint` and which the census explicitly could not
predict, goes from **0** of the reference's three colours to **all three at the reference's own
counts** (2/1/1) — so `XlsxTint` answers that arm correctly, which was an open question in the
prediction.

The single shortfall is `003_Contextures_chart_sample_9bda2719.xlsm`, **33 against 44**, and the
prediction said 44. **That prediction is refuted and the cause is measured**: the missing eleven
are not conditional formatting at all but the **`c:dPt` per-point fills of a bar chart** on the
same page, which we draw in the series' own `#FFC000` while the reference draws the eleven scale
colours the chart part states. The workbook's two chart parts carry 11 `c:dPt` each; we honour
them on the doughnut and not on the bar. Censused across all three families, `c:dPt` fills with a
stated colour: **sheets** doughnut 48 in 5 documents, bar 35 in 7, pie 31 in 5, scatter 3 in 2;
**slides** pie 144 in 10, doughnut 61 in 8, bar 31 in 4, line 4 in 1; **words** ofPie 22 in 2, pie
16 in 1, pie3D 7 in 3, doughnut 5 in 1.

### Where it lives, and the regression that was designed out

The overlay is a layer of its own on `SheetFormatting` and deliberately does **not** reach
`Cells`, `Rows` or `ColumnRuns`, because `SheetDecorationArea` reads those three to decide how far
a sheet prints and one corpus rule is declared over `N18:Q1048576`. Fixture `10` is the
measurement that 26.2.4.2 does not extend the print area either. `AScaleIsInvisibleToThePrintAreaScan`
is the test, and the mutation that interns the conditional fill as a stated cell format is caught
by it from the whole 974-test run with no filter.

## 4. The chart face: a paragraph default read where a run states otherwise

`DrawingChartPlot.SizeOf`, `BoldOf` and `LiteralFamily` took **the first `a:defRPr` or `a:rPr` in
document order**. A `c:rich` writes `a:pPr/a:defRPr` before `a:r/a:rPr`, so they read exactly the
value the run exists to override. On `003_advanced_excel_pie` the title's paragraph default is
`sz="1300" b="0"` in **Arial** and its one run is `sz="1800" b="1"` in **Calibri**:

| | before | after | reference |
|---|---|---|---|
| chart title runs | 2 × 13.00 pt LiberationSans | **2 × 18.00 pt Carlito-Bold** | 2 × 18.01 pt Carlito-Bold |
| chart label runs | 20 × 10.00 pt LiberationSans | **20 × 10.00 pt Carlito-Regular** | 10.01 pt Carlito-Regular |
| embedded fonts | Caladea-Bold, Carlito-Regular, **LiberationSans** | **Caladea-Bold, Carlito-Regular, Carlito-Bold** | the same three |

This is the whole of round 57's item 2, and it was a font-resolution question exactly as that
round guessed — but not a *fallback* question: nothing was mis-resolved, the wrong element was
read.

**Ten word counts moved on our side and every one moved toward the reference**, eight of them to
exact agreement:

| document | ours before → after | reference | \|ours − ref\| |
|---|---|---:|---|
| `006`/`022_advanced_excel_scatter` | 130 → **129** | 129 | 1 → **0** |
| `030`/`014_advanced_excel_scatter` | 131 → **130** | 130 | 1 → **0** |
| `025_advanced_excel_bar` | 139 → **140** | 140 | 1 → **0** |
| `028_advanced_excel_doughnut` | 112 → **113** | 113 | 1 → **0** |
| `013_advanced_excel_area` | 136 → **137** | 137 | 1 → **0** |
| `031_advanced_excel_bubble` | 126 → **127** | 127 | 1 → **0** |
| `064_Small_business_cash_flow` | 507 → **508** | 520 | 13 → 12 |
| `Keywords_Mapping_Graphs_and_Charts` | 4511 → **4513** | 4519 | 8 → 6 |

**Zero further, zero unchanged-distance, zero page counts, no document changed verdict in either
direction.**

## 5. Prediction against measurement

| | predicted | measured |
|---|---|---|
| sheets verdicts | **276 → 276**, zero movement either way | **276, twice, over two changes** |
| page counts | 0 change | **0** |
| word counts, the `colorScale` change | 0 on all 307 | **0 on all 307** |
| documents whose ink changes | **36** | **36** |
| cells that gain a fill | **423**, 419 with a census-predicted colour | **423**; 441 of 452 rectangles |
| `003_advanced_excel_pie` | 0 → 12 scale fills, the reference's colours | **0 → 12, exact, positions to 0.03 pt** |
| `003_Contextures…xlsm` | each colour **2 → 4** | **WRONG — 2 → 3** (§ 3; the fourth is a `c:dPt`) |
| the two that declare and resolve to nothing | 0 fills before and after | **0 and 0** |
| theme+tint stops | *unverified by the census* | **correct** — three colours at the reference's counts |
| words / slides | 0 | **0 from `colorScale`**; the chart face is shared (§ 6) |
| tests | +6 to +12, `XRay.Office.Spreadsheets` only | **+13, `XRay.Office.Spreadsheets` only** — one above the range |
| `MANIFEST.tsv` | no row changes status | **no row changes status** |

**Ten of twelve.** Both misses are counts, one over and one under, and the Contextures miss paid
for itself: chasing it is what found the `c:dPt` gap.

## 6. Shared layer — **yes**, and named from a census that had to be written twice

The `colorScale` change is `XRay.Office.Spreadsheets` only. **The chart-face change is
`XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs` and reaches all three tracks.**

The first census counted every site where a run states something different from its paragraph's
default and answered **37 sheets, 1 slides, 1 words**. It over-counts, and the over-count is
instructive: the old reader *skipped* a `defRPr` that did not state the field at all and went on
to the run, so "the paragraph states nothing" is not a change. Simulating **both readers** and
counting only where the answer moves:

| family | documents whose answer changes | sites |
|---|---:|---|
| sheets | **36** | `title.sz` 55, `title.b` 44, `title.latin` 44 |
| slides | **1** — `slides/done-011/pptx/171128IPAP.pptx` | `title.b` 0→1 |
| words | **0** | — |

**The one affected deck is measured, not reasoned about**: `batch-check.sh` over `slides/done-011`
gives `TOTAL 10 MATCH 10 MISMATCH 0`, and `171128IPAP.pptx` reads 40/40 pages, 4653/4670 words,
**match**. The words candidate — `ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx` — moves no
answer at all under the simulation. **The parent still owes the full slides and words sweeps**;
what is offered here is the named document and the reach, not a substitute for them.

## 7. The vision round: one confirmation, one new measured defect, one refutation

Three fresh subagents, one composed page each, no project documents, no source, no shell, each
asked to describe the halves separately and to give a direction. Every page was chosen for a
stated reason and none by `--worst`.

* **`003_advanced_excel_pie` p3** (chosen because it is the page the `colorScale` change targets).
  The reviewer described the heat-map band cell by cell on both halves and then: *"I compared M1
  (coral red), M5 (amber), M8 (yellow-green) and M12 (medium green) specifically and could not see
  a difference in either lightness or hue … Beyond the banner, I did not find any difference I can
  assert."* **That is the fix seen from outside by a reader who has never seen the code.**
* **`003_advanced_excel_pie` p1** (chosen for the chart title and the label wrap). Two real
  findings, one of them new:
  - *"the top's pie is substantially larger in diameter than the bottom's (roughly half again as
    wide)"*. The direction is right and the magnitude is not: measured from the slice bounding
    boxes, **ours is 235.8 pt across and the reference's 199.8 pt — 18 % larger** — with our
    centre 6.4 pt left and 3.2 pt below theirs. **This is new and it is not the label wrap.**
  - *"the bottom wraps every data label onto two lines … the top keeps each on one line"*, and a
    label collision on our side. That is round 57 § 7 arriving again from a different reader.
* **`Data-Architecture-Tool-Fit-Assessment-Template` p3** (chosen for the `cellIs` `dxf` fills).
  *"I find no difference in the rendered page content itself."* **My page choice was wrong, not
  the reader**: that document's seven `dxf` fills are not on page 3. Recorded so the next round
  picks the page from `pdf-ops.py` rather than from the sheet index.

**Refuted, by an instrument answering the exact claim.** *"The bottom has a legend entry at the
upper right … the top has no legend at all."* Both sides draw **five legend swatches and five text
runs on page 1**, ours at x 643.28/652.11 against the reference's 643.49/652.29 — agreeing to
0.2 pt horizontally. `HANDOVER.md` § 7 already records this exact misreading twice on this exact
page and attributed it partly to `--worst` selection. **This page was chosen for a stated reason
and the misreading happened anyway**, so the cause is the page and not the selection: the chart is
split across a horizontal page break, the legend's first entry sits at the far right of page 1 and
its text continues onto page 2, and a reader sees a truncated legend on one side and reads the
other side's identical one as absent. *Three rounds, three readers, one page, the same sentence.*

The same reviewer also reported the reference's M1 label as *"a small blank grey rectangle"* —
which is round 57's finding that the run's origin is off the MediaBox, seen as a picture.

## 8. Tests

**+13, all in `XRay.Office.Spreadsheets`** (961 → 974). The ten non-Fidelity projects, re-derived:
Containers 109, Core 337, Markup 259, OpenDocument 125, Presentations 819, Rendering 153 (+1
skipped), Spreadsheets 974, Text 617, Vector 295, WordProcessing 1180 — **4868 passed, 0 failed,
1 skipped**, against a base of 4855 by the same count. `dotnet build -v q -nologo` → **0 warnings,
0 errors**.

One run of `XRay.Office.Vector` reported **17 failed of 295** on a binary this round does not touch;
three re-runs of that project alone gave **0 failed, 295 passed** each time. That is the load
artefact `CLAUDE.md` records, seen at 1 and at 16 before; it is written down here rather than
acted on.

**Six mutations through `verify-test.sh`, all six detected:**

| mutation | detected by |
|---|---|
| round the interpolation's **sum** instead of truncating the delta | 4 tests, including `TheTwelveColoursOfTheCorpusWitness` |
| drop the priority sort between two overlapping scales | `ThePriorityAttributeDecidesBetweenTwoOverlappingScales`, both orders |
| let the conditional fill reach the stated formats | `AScaleIsInvisibleToThePrintAreaScan`, from the unfiltered 974-test run |
| let the cell's own fill beat the scale | `AScaleReplacesTheFillTheCellStates` |
| let text cells join the range and set its minimum | `ACellThatIsNotNumericTakesNoColourAndDoesNotSetTheRange` |
| round the **total** width instead of per glyph (§ 9) | `SheetRotatedRowHeightTests`, **26 of its 36 cases** |

## 9. The 24.2.7.2 audit — `Layout/SheetText.cs`, **VERIFIED**, and the list is finished here

The claim: a string's width on Calc's measuring device is the **sum of the rounded glyph advances**
and not the rounded sum. `audit_rotatedwidth.py` round-trips `sheet-row-height-rotated.fods`
through the installed 26.2.4.2 and compares all 216 row heights against the stored 24.2.7.2
figures: **216 of 216 unchanged, 0 moved**, including all **72 quarter-turn heights** — the ones
where `ScPatternAttr::GetCellOrientation` puts the string's *width* straight into the row height,
so nothing stands between `GetTextWidth` and the number.

**The discriminator is in the fixture rather than in an argument.** Four of its eighteen distinct
widths differ between the two readings of the rounding by up to 1.4 %, and the mutation that
rounds the total instead fails **26 of 36** cases through `verify-test.sh`. The reference moved
none.

**`XRay.Office.Spreadsheets` is now ten of ten re-checked and nine correct.** Counters re-derived
with the file's own commands: at this tree **39 open sites, 21 marker lines (18 `VERIFIED`, 2
`FIXED`, 1 `WRONG`, 0 `UNDECIDED`)**; at the base commit **39 open, 20 lines (17/2/1/0)**, read
with `$3` rather than `$2` as the file itself warns. The stored per-project table reproduced this
time — the first round in five where it did.

And round 56's "furniture claims are the ones that break" is now **nine to one and dead**: the
only wrong sheets site remains `SheetPageDecoration.cs`, and both re-checks taken on the strength
of that prior have come back correct.

## 10. An operational finding: a sweep's `TOTAL` grew from 325 to 363 because a page was looked at

The third sweep reported `TOTAL 363` where the first two reported 325, on the same 307 documents.
The cause is **38 all-lower-case alias directory entries materialised on the corpus mount by
`look.py`/`pair.sh`**, which resolve a document identifier case-insensitively; `ls -i` confirms
`065_Weight_loss_tracker_ff1c89af.XLSX` and its lower-case twin are one inode. The distinct
lower-cased path count is **307 in all three sweeps** and `score.py` keys on the manifest, so no
figure in this report is affected.

**But it means the alias count is not fixed, and a sweep total can grow purely from having looked
at a page.** `CLAUDE.md` records the aliases as a static nuisance worth 18 rows; they are a
*growing* one. Score against `MANIFEST.tsv`, always, and never `rm` an alias.

## 11. `MANIFEST.tsv`

Lives in the corpus repository and was **not touched**. **No row changes status** — no verdict
moved, and that was predicted. No change is proposed.

## 12. What the next round should do first

1. **The pie data-label wrap**, now with a second measured defect beside it: **our pie is 18 %
   larger than the reference's** (235.8 pt against 199.8 pt on `003_advanced_excel_pie`) and its
   centre is 6.4 pt left and 3.2 pt low. The wrap moves a centred label 27 pt sideways and the
   diameter moves every label at once; both are `XRay.Office.Core` and owe a corpus gate. These four
   pie documents are the only sheets failures with a chart cause left.
2. **`cellIs` conditional formatting** — 18 documents, and now cheap: the reader, the range walk,
   the priority order and the overlay all exist, and `cellIs` needs a comparison against a literal
   plus a `dxf` reader. Note what 26.2.4.2 does where the two meet: `fillinfo.cxx` applies the
   style's `ATTR_BACKGROUND` first and **the colour scale overwrites it**, so the scale must keep
   winning. `expression` (34 documents, the FAA grey fills) still needs `MID`, `AND` and a
   per-row relative-reference rewrite and should stay last.
3. **`c:dPt` per-point fills on bar charts.** Measured on `003_Contextures_chart_sample`: 11 stated
   per-point colours drawn as one series colour. 35 fills in 7 sheets documents, and the slides
   track has 144 on pie charts and 31 on bars, so this is a shared-layer item with a real
   cross-track census already written (§ 3).
4. **`.xls` colour scales are still uncensused.** Blind spot 1 of the prediction file, unfired this
   round because nothing was implemented there: a BIFF colour scale lives in `CF12` (0x087A) and
   the futures records, which no census on this project has read. Five `.xls` documents carry 187
   `CF` records between them and none of them is a scale by construction.
5. **The `x14` extension arm**, blind spot 2 — a `colorScale` that exists only inside
   `<extLst><x14:conditionalFormattings>` is invisible to every census written so far, and
   `036_Simple_to-do_list` proves the shape is in this corpus.
6. **`ChartLayout.IntervalsThatFit`** — untouched for a fourth round; round 56 § 9 has the census
   (256 automatic axes, 129 documents, all three tracks).
7. Still unworked, all ink: the chart area's light-grey border; a data label group's stated `bg1`
   fill (we emit a 221 pt white rectangle behind the pie that the reference does not); a band's
   `&K` colour.
