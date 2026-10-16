# Round 53 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r53`, base
`fead153ec7c`. Read `prediction.md` (`409e444b85d`), `prediction-addendum.md` (`ffd41a33ad4` — see
its own process note) and `prediction-newline.md` beside this file first; each was committed before
the sweep that measures it.

## 1. Baseline reproduced

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 284 MISMATCH 41`. Scored against
`MANIFEST.tsv`'s 307 sheets paths — the raw total counts 18 case-alias directory entries twice —
that is **271 match, 36 mismatch**, and the 36 mismatching paths are **exactly** the 36 rows the
manifest marks `open`. Reproduced to the document.

## 2. Result

**sheets 271 → 274 of 307.** Predicted **+1**, then **+1**, then **0**; measured **+1**, **+1**,
**+1**.

**Zero regressions.** No document went from `match` to anything else, and **0 of 307 page counts
changed**, as all three predictions said.

Five documents moved on **our** side and every one of them was named in a prediction:

| document | before | after | ref | verdict |
|---|---|---|---|---|
| `029_Annual_budget…xlsx` | 341 | **315** | 312 | `words` → **`match`** |
| `DynamicBubbleChart.xlsx` | 349 | **339** | 341 | `words` → **`match`** |
| `019_Free_Blood_Sugar_Chart…xlsx` | 798 | **870** | 872 | `words` → **`match`** |
| `041_Business_budget_ef942467.xlsx` | 234 | **230** | 230 | `match`, held (now exact) |
| `005_Contextures_chart_sample…xlsx` | 289 | **293** | 300 | `words`, still failing |

Three more rows moved **on the reference side only** and none of them is this round:
`047_Date_tracker_Gantt_chart` (844 → 822), `PBN Matrix NAAs (V01)` (5544 → 5547),
`FAA-2019-0995-0002_attachment_2` (9994 → 9985); `ans_mappings_of_eccairs_terms` and `SIL_TDB648`
oscillated across the three sweeps and returned. **Our column never moves for a document this round
did not touch.**

## 3. Change one — a chart data range stops before an Excel table's totals row

### 3.1 Round 52's `plotVisOnly` hypothesis is refuted, by two measurements

Round 52 handed over *"`029_Annual_budget` and `plotVisOnly` — measure whether its source rows are
hidden"*. **They are not.**

- `sheet11.xml` carries **no `hidden` attribute on any row or column**, and the reference's own
  printout draws rows 18 and 39 in the sheet body (`Total $4,000 …`, `Total $2,476 …`). Nothing on
  that sheet is hidden in either sense.
- An authored variant with `<c:plotVisOnly val="0"/>` on both charts renders through `soffice` with
  **the observable unchanged**: left chart still empty, right chart still seventeen bars.

The round's own instruction — *measure before building* — is what caught it, and this is the third
consecutive round on this track to refute the mechanism it was handed while keeping the observable.

### 3.2 What it actually is

`ScChart2DataSequence::BuildDataCache` (`sc/source/ui/unoobj/chart2uno.cxx:2616-2632`) skips a cell
when it is the **last row of the range being read**, a database range covers it, that range **has a
totals row**, and it **ends** on that row. Its own comment: *"Excel behavior: if the last row is the
totals row, the data is not added to the chart. If it's not the last row, the data is added like
normal."* A SpreadsheetML `table` part becomes such a **named** database range with `TotalsRow` set
from `totalsRowCount` (`sc/source/filter/oox/tablebuffer.cxx:133-137`), and
`ScDBCollection::GetDBAtCursor` searches the named ranges first — the sheet-local and global
anonymous ranges, which is what a plain `autoFilter` becomes, never carry totals, so a **table is
the only thing that can hide a cell from a chart**.

Five authored variants of `029`, one thing varied at a time, each rendered through `soffice`:

| variant | left chart | right chart |
|---|---|---|
| the corpus file | **empty, axis 0–12** | **17 bars, axis 0–$900** |
| `totalsRowCount="1"` → `"0"` on both tables | **plotted** | **18 bars, axis 0–$3,000** |
| left chart's values `$C$18:$M$18` → `$C$14:$M$14` | **plotted** | 17 bars |
| `plotVisOnly` `1` → `0` | empty | 17 bars |
| right chart's ranges shortened to `…$38` | empty | 17 bars |

**One varied attribute turns both observables on and off together** — which is the single mechanism
round 52 predicted would explain an empty plot, a missing category and an axis rescale at once. It
just is not the one it named.

It explains the shapes exactly. `029`'s left chart takes both value series from `$C$18:$M$18` and
`$C$39:$M$39`, which are *entirely* the two tables' totals rows, so every column is dropped and both
series are empty; its right chart's `$B$22:$B$39` / `$O$22:$O$39` end on one, so one point goes and
the axis falls from $3,000 to $900.

**And Excel's own caches agree, independently, on three different documents**: `ptCount` is 17 for
an 18-cell range on `029`, **8 for a 9-cell range** on `040_Blood_pressure_tracker`, and **3 for a
4-cell range** on `041_Business_budget`. Excel writes the cache without the totals row; Calc
reproduces that from the sheet.

### 3.3 Null and empty are different answers

The one design point worth carrying. `XlsxChartRanges.Resolve` already answered **null** for "cannot
resolve", which leaves the cached points standing — `ExcelChartConverter::createDataSequence`
throwing. A range every cell of which is a totals row is a different state: it **resolved**, and it
names no readable cell. Falling back to the cache there draws the entire plot the reference leaves
blank. So `Resolve` now returns an empty sequence for it and the two gates in `XRay.Office.Ooxml` drop
their `live.Text.Count > 0` test.

### 3.4 Census and reach

Every corpus document opened; tables resolved through each worksheet's relationships, honouring the
two conditions `Table::finalizeImport` bails on; every chart `c:f` parsed and intersected.

| | documents |
|---|---:|
| corpus scanned | 946 |
| with a totals-row table at all | 23 |
| **with a chart range ending on such a totals row** | **3 — all sheets, 0 words, 0 slides** |

`029` (open, moved), `040_Blood_pressure_tracker` (`done`, **unchanged** — its charts carry no data
labels, so dropping a point moved ink but no token) and `041_Business_budget` (`done`, 234 → 230,
now exact against the reference).

### 3.5 Tests

Nine in `XlsxChartTotalsRowTests`, plus one in `DrawingChartFormulaTests` for the empty-sequence
gate. **Eight distinct mutations through `verify-test.sh`, all detected:**

| mutation | detected by |
|---|---|
| the skip removed | `TheLastRowOfARangeThatEndsOnATableTotalsRow…`, `ARangeThatIsWhollyATotalsRow…`, `AColumnTheTableDoesNotCover…` |
| the range's-last-row test dropped (the rule read as a property of the table) | `ATotalsRowAboveTheEndOfTheRangeIsReadLikeAnyOtherCell` |
| `area.LastRow != row` widened to `<` | `ATableWhoseTotalsRowIsBelowTheEndOfTheRangeHidesNothing` |
| the empty sequence answered as null | `ARangeThatIsWhollyATotalsRowResolvesToNoPointsRatherThanToNull` |
| the `Count > 0` gate put back in `DrawingChartPlot` | `AResolverThatAnswersAnEmptySequenceDoesNotFallBackToTheCache` |
| the `displayName` guard dropped | `ATableWithNoDisplayNameHidesNothing` |
| the `id` guard dropped | `ATableWhoseIdIsNotPositiveHidesNothing` |
| `totalsRowCount="0"` treated as a totals row | `ATableWithoutATotalsRowHidesNothing` |
| the per-column test dropped | `AColumnTheTableDoesNotCoverKeepsItsLastCell` |

Two of those tests exist because the **first** mutation round found the mutation *undetected* and
the honest reading was "equivalent formulation, not drift guard" — the cases were then authored to
make the two formulations differ. `TheSameRangeWithNoTablePartAtAllKeepsItsLastRow` is a deliberate
**shape control** rather than a detector: it fails if the package builder stops attaching the table
relationship, which is the round-52 lesson (*when a test encodes a corpus shape, assert the shape is
present*) applied before the fact rather than after it.

### 3.6 A prediction miss, and its direction

`040` was predicted at 164–166 and measured **167, unchanged**. The rule fired on it — the census
was right — but its charts carry no data labels, so removing a point moved no token at all. The
error is in the safe direction and the document held `match`.

## 4. Change two — a pivot table's repeated row labels

### 4.1 `DynamicBubbleChart`'s whole 8-word gap, named

| | ours | reference |
|---|---|---|
| four department names on page 1 | **3 times each** | **once each** |
| the slicer advisory's line break | `2010.If` (1 token) | `2010.` + `If` (2) |
| a pivot filter caption on page 2 | absent | `(empty)` |

Rows 29–41 of its `Chart` sheet are a **pivot table** (`<location ref="A29:E41" firstHeaderRow="1"
firstDataRow="1" firstDataCol="5"/>`, five row fields, no data fields). Excel wrote the laid-out
result into the cells and filled the repeats down because all five fields carry
`<x14:pivotField fillDownLabels="1"/>`. **Calc regenerates the output through `ScDPOutput` and
writes a row field's label only where its group starts.**

Three authored variants settle it:

| variant | labels per department |
|---|---|
| the corpus file | **1** |
| `fillDownLabels="1"` → `"0"` | **1** — the attribute is ignored |
| **the pivot table part removed** | **3** — which is exactly what we drew with it present |

The third is the decisive one: with no pivot part the reference prints what we printed, so the
blanking is the pivot's doing and nothing else's.

### 4.2 The rule is a prefix test, and it is self-limiting

A row-label cell is blanked when the labels **from the outermost row field through this one** all
repeat the row above. Testing the cell alone would blank the `Cost` column, which holds `150` twice
in a row under two different risk values and which the reference prints both times.

**The scan stops at the first already-blank label**, and that is the property the whole census rests
on: a pivot Excel laid out *without* "Repeat All Item Labels" already has its repeats blank, so this
rule can only ever remove text Excel **filled down**. It is a no-op on the corpus's other ten pivot
documents by construction rather than by luck, and there is a test that says so.

### 4.3 Census

| | documents |
|---|---:|
| zip documents scanned | 802 |
| carrying a `pivotTable` part | 11 |
| stating `fillDownLabels="1"` | **1** — `DynamicBubbleChart`, open |

The other ten are `done` and **all ten are unchanged in the sweep**, including the two with more
than one row field (`037_Personal_money_tracker` at 501 and `alle einzeln.xlsx`) and the two other
pivot documents the round rendered directly (`049` at 330, `053` unchanged).

`DynamicBubbleChart` landed at **339** where the addendum's spot render said 339 — 10 tokens, not
the 5 first estimated, because `Information Technology` is two tokens and each of four names repeats
twice.

### 4.4 Tests

Eight in `XlsxPivotLabelTests`. **Six distinct mutations, all detected**; two of them
(`labelColumns` widened to the location's whole width, and the scan started at the location's first
row instead of `firstDataRow + 1`) came back *undetected* on the first pass and were reported as
equivalent formulations, then made to differ by two authored cases —
`ADataColumnIsPrintedEvenWhenTheWholeLabelPrefixRepeats` and
`TheFirstDataRowIsNotBlankedByAHeaderThatReadsTheSame`. `TheSameCellsWithNoPivotPartAtAllAreAllDrawn`
is the shape control.

## 5. Change three — `SheetChart` fused a two-line data label, and it was worth a verdict

The defect the words track fixed in `FrameChart` in round 52 and left here deliberately: a label
joined by a newline was shaped as one glyph run, drawing the break as a zero-width nothing.

**Predicted 0 verdicts; measured +1.** The prediction's census keyed on
`showPercent` + another `show*` and found five sheets documents; it explicitly named as blind spot
#1 that *"a label fused by a separator that is not `showPercent` would not be found"*. That is
exactly what happened: **`019_Free_Blood_Sugar_Chart` gained 72 tokens** (798 → 870 against 872) and
matched, because its **multi-level category axis** joins a date and a time with a newline — 54
`0:00`, 18 `12:00` and a long tail of `11/2`, `11/3`, … that had all been fused into `11/212:00`
shapes. The named blind spot is where the miss landed, and it landed in the useful direction.

`005_Contextures_chart_sample` moved as predicted, 289 → 293, and **still fails**: its remaining gap
is `Sales` seven times (§ 7.1). The four `_advanced_excel_pie` documents were predicted unchanged
and are unchanged.

Three tests in `SheetChartLabelLineTests`, driven through `SheetChart.Draw` so that two plots
differing in one field separate "the break is honoured" from "this chart happens to draw two runs".
Two mutations, both detected; the third assertion (`TheSecondLineIsDrawnBelowTheFirst`) exists
because two runs drawn at the same origin extract as two tokens and look right to the gate while
overprinting on the page.

**`SlideChart` still carries the identical defect** and is the slides track's.

## 6. The 24.2.7.2 audit — four of the nine `XRay.Office.Spreadsheets` sites re-checked

Every one is an **authored probe against the installed 26.2.4.2**, not a reading of the C++.
`dotnet/probes/sheets-r53-totalsrow/audit_*.py` reproduces them.

| site | probe | result |
|---|---|---|
| `Layout/SheetFonts.cs` — the digit-width model and `DigitWidthCarry` (**2 sites**) | 30 workbooks: 6 faces × 5 stated column widths; the x of a glyph in the *next* column is the first column's width | **30 of 30 exact to 0.001 pt — still correct** |
| `Layout/SheetGeneralWidth.cs` — the `###` threshold | 27 workbooks: 3 face/size pairs × 9 column widths over `123456.789` | **27 of 27 agree**, across all three threshold crossings (`###`, `1E+05`, the rounded decimal) — **still correct** |
| `Layout/SheetDeviceUnits.cs` — the 720 dpi round trip | 45 workbooks: 3 faces × 15 sizes from 6 pt to 48 pt | **45 of 45 within 0.1% relative**, max 0.47 pt on a 484.81 pt line — **still correct** |

All four comments now name **26.2.4.2** and **2026-08-21**, which is what stops the next round
re-checking them. Five `XRay.Office.Spreadsheets` sites remain: `SheetNotes`, `SheetOptimalRowHeights`,
`SheetPageDecoration`, `SheetShapeText`, `SheetText`, plus `Ooxml/XlsxNoteCaptions`.

### 6.1 The instrument needed a control, and it failed it first

The first run of the device probe reported the reference drawing a **constant 101.08 pt** at every
stated size and in every face — 46 of 48 cases "outside tolerance", which would have read as a
spectacular calibration failure. It was the fixture: **a workbook with no `<cellStyles>` element has
its `cellXf` font discarded by LibreOffice entirely** (confirmed by converting to flat ODF and
reading back `fo:font-size`, which said 10 pt for a stated 48). Adding `<cellStyles>` made all 48
agree. This is `HANDOVER.md` § 7's *"an instrument can manufacture a defect out of nothing"*, caught
by asking a question whose answer was already known rather than by believing the first big result.

**An incidental real finding from it, recorded and deliberately not acted on:** in that same
malformed shape **we honour the `cellXf` font where LibreOffice does not**. Censused over the 242
`.xlsx`/`.xlsm` sheets documents, **5 lack `<cellStyles>`** — `AFCforPtF-`, `AMOC-`, `MinCh-` and
`MajCh-Digital-Certificate-Publication-Report` and `STC_WebList` — and **all five currently pass**.
Changing this could only cost verdicts today; it is written down instead.

## 7. What was refuted, and what is left open with a mechanism attached

### 7.1 `005_Contextures_chart_sample`'s remaining 7 words are an automatic chart title

Five of its six charts state `<c:title><c:overlay val="0"/></c:title>` — a title element with **no
text at all** — and each has exactly one series named `Sales`.
`ChartSpaceConverter::convertFromModel` (`oox/source/drawingml/chart/chartspaceconverter.cxx:181-204`)
fills such a title from `PlotAreaConverter::getAutomaticTitle()`, which
`TypeGroupConverter::getSingleSeriesTitle` (`typegroupconverter.cxx:272-281`) answers with the single
series' cached name — but only when the axes set holds exactly **one type group**. We draw no title.

**Not implemented, and the costing is why.** It is a `XRay.Office.Ooxml` change reaching all three
tracks, and the same block carries a second branch that substitutes the localized literal
`Chart Title` when a title element exists, is empty, and there is *no* single series — so the cheap
half cannot be shipped without deciding the expensive half. Worth 7 tokens on one open document
(which would land it at 300 against 300), and a cross-track census is owed before it is written.

### 7.2 A `c:strLit` category sequence is lost in a workbook, and it is worth nothing here

`029`'s third chart writes `<c:cat><c:strLit>` holding `Income` / `Expenses`; the reference draws
`1` / `2`. Calc's converter hands a literal sequence to
`createDataSequenceByRangeRepresentation`, which cannot take an array, so the categories are lost
(`excelchartconverter.cxx:94-114`). Two tokens against two tokens — **net zero on the gate** — and
the direction of the "fix" is to draw *less*. Recorded, not implemented.

### 7.3 The four `_advanced_excel_pie` documents are still the `bestFit` item

Their 5-token gap is unchanged and is **not** a fused label: the reference draws `M1;`, `Actual;`
and two two-digit tokens that we do not, and our `17%` and `trend` come out as `7%` and `rend` —
**clipped by the horizontal page split**, which is the item the round-53 brief described. Untouched.

## 8. Shared layer

**Yes, once.** Change one edits `XRay.Office.Ooxml` — three files, `ChartRangeResolver.cs` (doc only),
`DrawingChart.cs` and `DrawingChartPlot.cs`, each dropping one `live.Text.Count > 0` clause. **The
reach is zero by construction and the argument is not a census but a type**: the clause can only
change behaviour when a `ChartRangeResolver` is supplied, and the only implementation in the tree is
`XlsxChartRanges`, reached only from `XlsxReader`. Words and slides pass `null` — `DrawingChart.cs`'s
own remarks say why — so no `.docx`, `.doc`, `.pptx` or `.ppt` can take that branch at all. Changes
two and three are `XRay.Office.Spreadsheets` only.

`WordProcessing` (1083) and `Presentations` (781, +1 for the new gate test) are green, and `Fidelity`
is **521 passed / 31 failed / 552**, byte-for-byte the base's figure. **The parent should still run
the cross-track sweep**; the prediction is 0 of 666 changed.

## 9. Build and tests

`dotnet build -v q -nologo` → **0 warnings, 0 errors.**

Ten non-Fidelity projects, run one at a time and totalled by hand: **4660 passed, 0 failed, 1
skipped**, against the base's 4639/0/1 — a delta of exactly the **21** new tests: 20 in
`XRay.Office.Spreadsheets` (905 → 925: nine `XlsxChartTotalsRowTests`, eight `XlsxPivotLabelTests`,
three `SheetChartLabelLineTests`) and one in `XRay.Office.Presentations` (780 → 781).

Whole track swept after every change: base `TOTAL 325 MATCH 284`, after change one `MATCH 286`,
final `MATCH 288` — **274 of 307** against the manifest's path list, **zero regressions, zero
page-count changes**. `sheets/done-*` is inside every one of those sweeps.

## 10. Proposed `MANIFEST.tsv` reclassification

`MANIFEST.tsv` lives in the corpus repository and was not touched. Three rows, `status=open` →
`status=done`:

| path |
|---|
| `sheets/chartset-009/xlsx/029_Annual_budget_Use_this_template_30324a97.xlsx` |
| `sheets/chartset-004/xlsx/DynamicBubbleChart.xlsx` |
| `sheets/chartset-013/xlsx/019_Free_Blood_Sugar_Chart_for_Excel_-_Track_Your_Blood_Sugar_Level_36f4a782.xlsx` |

## 11. What the next round should do first

1. **The automatic chart title (§ 7.1).** It is the only open item on this track with a located
   mechanism, a named source line and a document that lands *exactly* on the reference if it works
   (`005_Contextures`, 293 → 300 against 300). It is a shared-layer change and it owes a cross-track
   census of `<c:title>` elements with no text **before** a line is written, because the second
   branch substitutes a literal `Chart Title` string.
2. **The four `_advanced_excel_pie` documents** — 4 documents, one defect, 5 tokens each, and the
   defect is now known to be *clipping at the horizontal page split* rather than a fused label. The
   reference places the `M1` data label outside the pie so it lands wholly on page 2.
3. **The eight-blank-line header** — twelve probes in `probes/sheets-r51-bands/` bracket it and none
   explains it; 20 words on `FAA-2019-0995-0002`, whose reference half is date-volatile.
4. **Five more 24.2.7.2 sites** in `XRay.Office.Spreadsheets` — `SheetOptimalRowHeights` first, since
   row heights are the axis this track has already established for a 14-document cluster. The probe
   harness (`audit_mkwb.py`) is in this directory and the `<cellStyles>` trap is documented in it.

Still unworked from earlier rounds' blind readings: `068_Blue_inventory_list`'s two undrawn arrow
autoshapes and grey-for-teal title; `017_Timeline_Templates`' missing navy spine, five year badges
and every leader line; `065`'s literal `aaaa` where the reference draws `Thursday`;
`070_Equipment_inventory_list`'s advisory shape wrapping at different points — which is the same
text-box-width question as `DynamicBubbleChart`'s residual `2010.If`, now with a second witness.
