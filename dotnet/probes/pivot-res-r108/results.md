# What the pivot round declined, and the oracle bug that made one part of it unmodellable

Round 107 took `alle einzeln.xlsx` from 225.44 to 0.07 and left three things. This round closes
two of them, measures the third, and retracts one paragraph of r107's write-up along with the
reasoning that produced it.

**The headline is small and is stated as such.** Five of the eleven pivot documents render
differently and the net is **−3.32** of summed unsigned ink, against the −240.63 r107 moved;
three improve, one worsens by 0.02, one is level, and no gate or MAJOR verdict moves anywhere.
`alle einzeln` was already at the floor and does not move at all. What is worth more than the
number is *why* r107 declined the hidden header: its oracle was reading the wrong rows.

## The correction: `<table:table-header-rows>` shifted the oracle by one row

`check-grid.py`'s `oracle()` walked `table:table`'s **direct** `table:table-row` children. ODF
does not put every row there. A sheet with print titles carries one or more rows inside
`<table:table-header-rows>`, in document order among the others, and an outline puts rows inside
`<table:table-row-group>` — so a direct-child walk silently drops those rows and reads every row
after them one place too early.

`033_Event_planning_tracker`'s pivot sheet has exactly one such row, and it is the pivot's own
header. Read the wrong way, its table appeared to start at row 5 with a *data* row taking an
inner rule and to end at row 10, one above the range Excel wrote. Read correctly it starts at
row 5 with the twelve data-field names on it, takes the table's own outer rule there, and ends at
row 11 — which is the stated range exactly, and is what `mnHeaderSize = 0` predicts.

So r107's *"with `firstHeaderRow="0"` accepted, `033` gets 24 of 131 edges right, misses 26,
invents 56 and draws 25 at the wrong width"* is an artefact of the instrument, and *"where the
reference's table actually starts is not settled"* is withdrawn. The fix is in this round's copy
of the script, `check-grid.py`'s `walk()`.

**Two independent checks that the correction is the right way round.** The reference's own
round-trip of `033` back to `.xlsx` writes
`<location ref="B5:N11" firstHeaderRow="0" firstDataRow="1" firstDataCol="1"/>` — the same
geometry Excel wrote — and `xepivotxml.cxx`:1258-1275 builds those four numbers from
`GetOutputRangeByType(TABLE)` and `(RESULT)`, so they are `mnTabStartRow`, `mnTabEndRow`,
`mnDataStartRow` and `mnDataStartCol` read straight out of the reference's own model. And the
same workbook with only `firstHeaderRow` changed to `1` comes back as `B5:N12` with
`firstDataRow="2"`: one row taller, exactly the row the header size adds.

**What the bug did *not* touch, which is why r107's 0/0/0 stands.** Of the 58 sheets in the eleven
pivot-bearing workbooks, 11 hold rows inside a `table:table-header-rows`; **none of them carries a
pivot r107 accepted.** The three it does touch are the three r107 declined — `033`, `035` and
`037`'s `Monthly summary` — which is why the bug and the declines coincide.

## (a) A hidden header: `mnHeaderSize` is `firstHeaderRow`

`mnHeaderSize` is `0` when the pivot hides its header, `1` otherwise, and `2` only for a grid
header layout that the OOXML path cannot reach (`dpoutput.cxx`:884-889; `SetHeaderLayout` is
called only from `sc/source/filter/excel/xipivot.cxx` and `sc/source/filter/xml/xmldpimp.cxx`).
The OOXML import ties the first to the stated first header row outright —
`mpDPObject->SetHideHeader(maLocationModel.mnFirstHeaderRow == 0)`,
`sc/source/filter/oox/pivottablebuffer.cxx`:1368, verified — so for the two values that can arise
`mnHeaderSize == firstHeaderRow`, and the acceptance test is `firstDataRow == firstHeaderRow +
colFields`. Nothing else about the layout changes: `outputColumnHeaders` already guards the
field-button row with `mnMemberStartRow > mnTabStartRow` (`:1010`), and with the header hidden
that is false.

## (b) Compact packing of more than one row field

`GetColumnsForRowFields` (`:854-868`) packs a run of compact row fields into one column: one
column per non-compact field, plus one more when the innermost field is compact. Three things
follow, and all three had to be modelled:

- **`firstDataCol` is checked against the packed count**, not against the field count.
- **The button cell above a packed column is not boxed.** `outputRowHeader` writes `FieldCell`
  while `!mbHasCompactRowField || nNumRowFields == 1` and otherwise writes `MultiFieldCell` for
  the first field only (`:1087-1090`), and `MultiFieldCell` (`:794-812`) writes the caption, the
  button flag and the `Field` style and never calls `lcl_SetFrame`. `outputColumnHeaders` makes
  the same test — on `mbHasCompactRowField`, the *row* fields' flag, against its own
  `nNumColFields == 1` (`:1010-1015`) — so a packed row axis can take the box off the column
  buttons too.
- **Each packed field adds an indent step to the fields inside it.**
  `nIndent = 13 * (bLast ? nFieldIndentLevel : nMinIndentLevel + nFieldIndentLevel)` px
  (`:1135-1137`) with `nFieldIndentLevel` incremented for every compact field and reset by a
  non-compact one, and `bLast` counting *fields* rather than packed columns — so the innermost
  field of a packed column drops the drill step and keeps the depth the fields outside it added.
  Both fields of `037`'s `Monthly summary` come out at 195 twips, which is what the reference's
  own `.fods` writes on all sixteen of its member cells.

r107's model applied the indent only inside the `nField + 1 < size` branch. In `dpoutput.cxx` it
is outside it, and only `bLast` keeping the step at zero made that invisible while no field was
packed.

## The oracle, both directions, on nineteen pivots

`check-grid.py` walks the whole of each accepted pivot's rectangle, page-field rows included, and
counts an edge the reference states and the prediction does not (MISS) as well as one the
prediction invents (EXTRA) and one drawn at the wrong width (WRONG). Generated cell styles — read
off the `Pivot_20_Table_20_*` parent the reference gives each automatic style — and the generated
indent are compared the same way, and the indent against the rectangle's own floor.

| Workbook | pivots accepted | edges | MISS | EXTRA | WRONG | styles |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `alle einzeln.xlsx` | 1 | 30250 | 0 | 0 | 0 | 6060 ✓ |
| `Keywords_Mapping_Graphs_and_Charts.xlsx` | 11 | 749 | 0 | 0 | 0 | 142 ✓ |
| `DynamicBubbleChart.xlsx` | 1 | 296 | 0 | 0 | 0 | 60 ✓ |
| `037_Personal_money_tracker.xlsx` | **2 of 2** | 293 | 0 | 0 | 0 | 62 ✓ |
| `007_Contextures_chart_sample.xlsm` | 1 | 120 | 0 | 0 | 0 | 18 ✓ |
| `033_Event_planning_tracker.xlsx` | **1** | 105 | 0 | 0 | 0 | 30 ✓ |
| `035_Project_plan_for_law_firms.xlsx` | **1** | 105 | 0 | 0 | 0 | 30 ✓ |
| `049_Expenses_calculator.xlsx` | 1 | 94 | 0 | 0 | 0 | 23 ✓ |
| **total** | **19 of 19** | **32012** | **0** | **0** | **0** | **6425 ✓** |
| `features/sheet-pivot-packed.xlsx` (fixture) | 3 | 135 | 0 | 0 | 0 | 25 ✓ |

**Every worksheet-cached pivot in the corpus is now accepted and every one of them is exact.**
r107 accepted 16 of 19 with 31596 edges; this is 19 of 19 with 32012.

The null this is against: the nineteen rectangles hold **9970** cells and **39880** edge slots, of
which the reference states **32012**, 80.3 %. A predictor that stated every slot would agree on
32012 and invent 7868; one that stated none would miss 32012. Neither would get the widths, and
**2622** of the 32012 are the 40-twip outer rule against 29390 of the 20-twip inner one, so a
predictor with every edge's presence right and 20 guessed everywhere would still be wrong 2622
times. This score has 0 in all three columns.

`pivot-grid.py` is the probe's own transcription and not the shipped C#; the fixture is where the
two meet, and the rendered ink below is the measurement of the shipped code.

## (c) The clearing, measured — and it is the one thing that costs ink

`PivotTable::finalizeImport` clears the stated range of
`VALUE | DATETIME | STRING | FORMULA | HARDATTR | STYLES | EDITATTR | FORMATTED`
(`pivottablebuffer.cxx`:1331-1336, verified) and `ScDPOutput::Output` then deletes the computed
range outright (`dpoutput.cxx`:1226), so what a pivot cell shows is only what `ScDPOutput` puts
back. This tree merged instead. r107 recorded that and left it unmeasured for alignment and
indent; it is measured now.

**What clearing leaves is the Default cell style, not nothing.** That is the half a reading of
the source does not give and the `.fods` does: `049_Expenses_calculator` states
`horizontal="left" … indent="1"` in **`cellStyleXfs[0]`** — the entry its `Normal` `cellStyle`
names, which becomes Calc's `Default` — as well as in `cellXfs[0]`, and 26.2.4.2's own `Default`
table-cell style for that workbook carries `fo:text-align="start" fo:margin-left="0.1457in"`.
Forty-seven cells of its pivot resolve their justification and indent that way and the reference
keeps all forty-seven. A fixture whose `cellXfs[0]` states a justification its `cellStyleXfs[0]`
does not shows the other side: 26.2.4.2 clears it.

Over the 19 rectangles' **9970** cells, **330** state an alignment or an indent. Scored against
26.2.4.2's own resolved view of each one:

| merge base | alignment disagreements | indent disagreements |
| --- | ---: | ---: |
| what the cell states (the behaviour r107 shipped) | 81 | 98 |
| the sheet's default `cellXf` (what this round ships) | **0** | **0** |

`clearing-census.py`, both runs banked in `clearing-census.txt`.

**And it makes the ink very slightly worse.** Measured the same way as the table below, with the
clearing as the only difference: `033`'s summed unsigned ink goes **11.20 → 11.27** and `049`'s
**0.90 → 0.92**, and nothing improves. That is 0.09 of the 21.01 total, on two documents whose
residual is dominated by something else — of `033`'s 11.27, page 3 carries 11.20 and *"a fill or
background shading the reference has and we do not"* covers 17.4 % of that page. **It is shipped
anyway**, on the grounds that the cell-for-cell score above is a direct measurement of the
reference's own answer for 9970 cells while the ink is a second-order consequence of text moving
inside a region that is wrong for an unrelated reason. That is a judgement, and the number that
argues against it is written here rather than left out.

**One approximation is knowingly in it.** The shipped code clears back to
`SheetCellFormats.SheetDefault`, which this tree builds from `cellXfs[0]`
(`XlsxSheetFormats.cs`:52), where the reference clears back to the `Normal` `cellStyleXf`.
`XlsxStyles.DefaultFormatId` already records that the two are different questions and that *every
workbook in the corpus gives them the same id* — so no corpus document can tell them apart, and
the fixture deliberately does not either.

## What is still declined, and the census that says nil

*The cache.* `PivotCache::finalizeImport` finalizes only `XML_worksheet`; the `XML_external`,
`XML_consolidation` and `XML_scenario` arms are empty (`pivotcachebuffer.cxx`:1093-1114). The
three workbooks whose caches are all `type="external"` write **0** `table:data-pilot-table`
elements for their **9** pivot parts and the other eight write exactly one for each of their
**19**. 19/19 against 0/9.

*The data-layout dimension on the row axis.* `bSkip` drops it when it is empty (`:595-596`) and
Calc lays out one row-label column fewer than Excel wrote. **No document in the corpus states
it**: `pivot-census.py` reports `rowDataPH=False` for all 28 pivot parts.

*The pivot's own `<format>`/`dxf` records.* Still not read, and now censused: **0 of the 28 pivot
parts in the eleven workbooks state a single `<format>` element** (`format-census.py`,
`format-census.txt`). Nil reach on this corpus; it is not a refusal to model something a document
asks for.

*`mnTabEndCol` widened to `mnTabStartCol + 1`* (`:916-918`) cannot bite: acceptance requires at
least one row-label column and one column item, so the table is always two columns wide.

## Reach: the eleven pivot-bearing documents

**The eleven are all of them.** Every one of the sheets track's 307 documents was opened as a zip
and searched for an `xl/pivotTables/*.xml` part; eleven have one, and they are these eleven.

Base is `3a926f63f`, head is this branch. Ink is `|ink|%` from `pdf-image-diff.py` summed unsigned
over the document's pages against the banked 26.2.4.2 reference in `/home/user/gate-orig-r83/ref`;
MAJOR is that script's own verdict. Both legs were rendered today, in the same run, with
`SOURCE_DATE_EPOCH` fixed.

| document | pages | base ink | head ink | Δ | base MAJOR | head MAJOR |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `037_Personal_money_tracker` | 5 | 2.88 | **1.25** | −1.63 | 1 | 1 |
| `033_Event_planning_tracker` | 3 | 12.36 | **11.27** | −1.09 | 1 | 1 |
| `035_Project_plan_for_law_firms` | 4 | 1.75 | **1.13** | −0.62 | 0 | 0 |
| `049_Expenses_calculator` | 4 | 0.90 | 0.92 | +0.02 | 0 | 0 |
| `alle einzeln` | 186 | 0.07 | 0.07 | 0.00 | 0 | 0 |
| `Keywords_Mapping_Graphs_and_Charts` | 46 | 1.68 | 1.68 | 0.00 | 0 | 0 |
| `DynamicBubbleChart` | 5 | 0.94 | 0.94 | 0.00 | 1 | 1 |
| `007_Contextures_chart_sample` | 7 | 0.13 | 0.13 | 0.00 | 0 | 0 |
| `027_Simple_personal_cash_flow_statement` | 10 | 3.03 | 3.03 | 0.00 | 1 | 1 |
| `026_Monthly_cash_flow_statement` | 11 | 0.59 | 0.59 | 0.00 | 0 | 0 |
| `053_Personal_asset_inventory` | — | 0.00 | 0.00 | 0.00 | 0 | 0 |
| **total** | | **24.33** | **21.01** | **−3.32** | **4** | **4** |

`053_Personal_asset_inventory` is unscoreable: `pdf-image-diff.py` refuses a comparison whose page
counts differ and reports no pages, at base and at head alike, so its `0.00` is an absence of
measurement. It has one pivot and its cache is external.

`DynamicBubbleChart` renders differently and scores the same to two decimals — the clearing takes
one aligned cell off it. `033`'s base is 12.36 here against the 12.65 r107 recorded; both legs of
this round were rendered the same day, so the comparison inside this table is sound.

r103 sized the remaining prize on the ODF side at 0.55 for `033`, 0.53 for `035` and part of
`037`'s 3.59. Measured, they came to 1.09, 0.62 and 1.63. **No MAJOR verdict moves**, and none was
going to: `033`'s page 3 and `037`'s page 1 are MAJOR for reasons that are not the pivot grid.

## Regressions

`sweep.sh` rendered all **307** sheets-track documents at both binaries with `SOURCE_DATE_EPOCH`
fixed, checked `%%EOF` before hashing, deleted each render as it went, and would have stopped
itself with 1 GiB left.

It did not stop and no render failed or was truncated, so all **614** hashes are of complete PDFs.
**Five documents moved** and the other 302 are byte-identical:

| document | in the reach table |
| --- | --- |
| `033_Event_planning_tracker` | −1.09 |
| `037_Personal_money_tracker` | −1.63 |
| `035_Project_plan_for_law_firms` | −0.62 |
| `049_Expenses_calculator` | +0.02 |
| `DynamicBubbleChart` | 0.00 to two decimals — the clearing takes one aligned cell off it |

All five hold a pivot the grid is generated for. The other six pivot-bearing workbooks did not
move: three are all-external caches, `alle einzeln`, `Keywords_Mapping` and `007` were already
exact and state nothing for the clearing to remove. Full hashes in `sheets-sweep-r108.tsv`.

The other two tracks cannot be reached: the change is inside `XlsxPivotGrid`, which only
`XlsxFile`'s OOXML spreadsheet path enters, and the 64 `.xls` in the sweep go through
`XRay.Office.MsBinary` instead.

## Tests

All eleven projects were run one at a time so a truncated run cannot hide as a pass, and the
totals below are read out of that run's own output (`tests.log`).

Ten non-fidelity projects — Containers 109, Core 560, Markup 259, OpenDocument 160,
Presentations 1101, Rendering 164, Spreadsheets 1323, Text 728, Vector 309, WordProcessing 1938 —
**6651 passed, 0 failed**, and the run reached the last of them.

`XRay.Office.Fidelity.Tests`: **Failed: 10, Passed: 542, Total: 552**, and the ten are the known ten
and no eleventh — `PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt` on
`paginated.fodt`/`.doc`/`.docx`/`.rtf`,
`TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop` on
`list-label-overrun.doc`/`.fodt`/`.docx`/`.odt`,
`SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt` on `sheet-rich-text.xlsx`,
and `JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt` on
`justify-shrink-2013.docx`.

`SheetPivotPackedTests` is eight tests over `features/sheet-pivot-packed.xlsx`, written by this
round's `make-fixture.py`: three pivots over one cache on three sheets, `Hidden` stating
`firstHeaderRow="0"`, `Packed` two compact row fields, and `Cleared` the same pivot as `Hidden`
over cells that hard-state a centring, an indent and a bold weight. Every expected number was read
out of 26.2.4.2's own `.fods` of that workbook. With `XlsxPivotGrid.cs` taken back to
`3a926f63f`, **six of the eight fail**; the two that pass are `TheDataSheetIsUntouched` and
`ACellOutsideThePivotKeepsWhatItStates`, which both assert that something does *not* change.

`SheetPivotGridTests`, r107's seven tests over `features/sheet-pivot-grid.xlsx`, are unchanged and
still pass, and that fixture still scores 99 edges / 22 styles at 0 MISS / 0 EXTRA / 0 WRONG under
the corrected oracle.

## What is left unsettled

- **The `Normal` `cellStyleXf` is not read as a format.** The clearing falls back to `cellXfs[0]`
  instead. No corpus workbook distinguishes them; a workbook that did would be cleared to the
  wrong base.
- **`033`'s residual 11.27 is not the pivot.** Its page 3 is missing a fill or background over
  17.4 % of the page, and the pivot grid on that page is now exact edge for edge. That is the
  next seat on that document and it is not a pivot question.
- **What else `clearContents` removes is still merged**: a font face, a size, a colour, a fill and
  a border on a pivot cell all survive here and none survives in the reference. Borders remain nil
  — **not one cell of the corpus's 28 pivot ranges has a `cellXfs` entry stating a border**
  (`statedborders.py`, `statedborders.txt`) — and the other four are unmeasured. Seated as O45.
- **The pivot's `<format>`/`dxf` records** reach `maFormatOutput` and can put formatting back after
  the generated styles. Nil on this corpus, so there is nothing here to check an implementation
  against.
- **`053_Personal_asset_inventory` still cannot be scored at all.** Its page counts differ from
  the reference's, at base and at head, so `pdf-image-diff.py` reports no pages. Its one pivot is
  externally cached and nothing is generated for it either way, so this round could not have moved
  it — but its `0.00` in the table above is an absence of measurement and not a score.

## Citations, re-checked by hand

Every `file:line` below was re-opened in `/home/user/libreoffice-core`. **That tree is not the
reference binary's source** — its `configure.ac` declares `27.2.0.0.alpha0+`, the binary is
26.2.4.2, and the checkout is one bulk import — so each is *this tree*, and the arm that measures
the actual reference is the `--convert-to fods` oracle above and the `--convert-to xlsx`
round-trip.

| Citation | What it should be | Verified |
| --- | --- | --- |
| `dpoutput.cxx`:76-79 | `SC_DP_FRAME_INNER_BOLD 20` / `OUTER_BOLD 40` / `COLOR Color(0,0,0)` | yes |
| `dpoutput.cxx`:127 | `ScDPOutputImpl::OutputDataArea` | yes |
| `dpoutput.cxx`:217 | `ScDPOutputImpl::OutputBlockFrame` | yes |
| `dpoutput.cxx`:264-296 | `lcl_SetStyleById`, and the three styles that carry anything | yes |
| `dpoutput.cxx`:297 | `lcl_SetFrame` | yes |
| `dpoutput.cxx`:424-431 | `lcl_MemberEmpty` | yes |
| `dpoutput.cxx`:595-596 | `bSkip` drops an empty data-layout row dimension | yes |
| `dpoutput.cxx`:596-611 | `maRowCompactFlags` / `mbHasCompactRowField` from `COMPACT_LAYOUT` | yes |
| `dpoutput.cxx`:748 | `ScDPOutput::HeaderCell` | yes |
| `dpoutput.cxx`:794-812 | `MultiFieldCell`, which does **not** call `lcl_SetFrame` | yes |
| `dpoutput.cxx`:854-868 | `GetColumnsForRowFields` | yes |
| `dpoutput.cxx`:871-923 | `CalcSizes` | yes |
| `dpoutput.cxx`:884-889 | `mnHeaderSize` 0 for a hidden header, 2 for a grid header layout | yes |
| `dpoutput.cxx`:1010-1015 | `FieldCell` vs `MultiFieldCell` on the column-button row | yes |
| `dpoutput.cxx`:1087-1090 | the same test on the row-button cell | yes |
| `dpoutput.cxx`:1135-1137 | `bLast`, `nMinIndentLevel`, `nFieldIndentLevel`, 13 px | yes |
| `dpoutput.cxx`:1226 | `DeleteAreaTab(…, InsertDeleteFlags::ALL)` | yes |
| `dpoutput.cxx`:1304-1322 | `GetOutputRange`, and what its three region types are | yes |
| `dpobject.cxx`:1064-1065 | `maOutputRange = mpOutput->GetOutputRange()` after every output | yes |
| `pivottablebuffer.cxx`:296 | `mbCompact = mbSubtotalTop && mbOutline && compact` | yes |
| `pivottablebuffer.cxx`:812-814 | `mbCompact` becomes `COMPACT_LAYOUT` | yes |
| `pivottablebuffer.cxx`:1331-1336 | `clearContents(… HARDATTR \| STYLES …)` over the stated range | yes |
| `pivottablebuffer.cxx`:1368 | `SetHideHeader(mnFirstHeaderRow == 0)` | yes |
| `pivottablebuffer.cxx`:1421-1426 | the start moved up by `pageFields + 1`, clamped at row one | yes |
| `pivotcachebuffer.cxx`:1093-1114 | only `XML_worksheet` is finalized | yes |
| `xepivotxml.cxx`:1258-1275 | the exported `location` is built from the two output ranges | yes |
| `XlsxSheetFormats.cs`:52 | `SetSheetDefault(pooled[0])` — `cellXfs[0]`, not the `Normal` style | yes |
