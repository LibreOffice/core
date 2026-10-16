# The pivot table's generated formatting

`alle einzeln.xlsx` stood at **225.44** of summed unsigned ink over 186 pages with 36 MAJOR
pages, the largest single residual in the sheets corpus, and every one of those 36 pages was its
`Pivot` sheet. The workbook states one border and it is empty. The formatting on a pivot table's
cells is not read from the file at all: Calc throws away what Excel wrote there and regenerates
it from the pivot definition when the table is imported.

This round resumed a session that a container restart killed mid-work. What it inherited —
commit `005470558`, "salvaged from a container restart, unvalidated" — had never been built,
tested or measured. It is now all four, one of its rules was wrong, and two numbers written into
its own test were wrong.

## What is generated, and where it is read from

`ScDPOutput` rules the regenerated output in two widths of plain black, `SC_DP_FRAME_INNER_BOLD`
20 twips and `SC_DP_FRAME_OUTER_BOLD` 40, `SC_DP_FRAME_COLOR` `Color(0,0,0)`
(`sc/source/core/data/dpoutput.cxx`:76-79), and applies four cell styles it creates on demand
(`lcl_SetStyleById`, `:264-296`), of which only three carry anything: `Pivot Table Category` is
left justified, `Pivot Table Title` left justified and bold, `Pivot Table Result` bold, and
`Value`, `Field`, `Corner` and `Top` state nothing. Beside them a row field that is not the
innermost indents its member cell by `13 px` — 195 twips (`:1135-1140`).

Placing those lines needs the member tree, which `ScDPOutput` takes from the data pilot's own
result sequences. It does not have to be recomputed: SpreadsheetML's `rowItems` and `colItems`
*are* that tree already laid out, one `<i>` per output row or column, `@r` naming how many
leading fields the entry inherits and `@t` marking a subtotal. Read that way an entry's `@r` is
exactly `MemberResultFlags::CONTINUE`, the fields it states are `HASMEMBER`, and a non-`data`
`@t` is `SUBTOTAL` on the innermost field it names
(`offapi/com/sun/star/sheet/MemberResultFlags.idl`:31/36/41,
`ScDPResultMember::FillMemberResults`, `sc/source/core/data/dptabres.cxx`:1425).

The implementation is `XlsxPivotGrid`; the generated text formatting is a differential overlay,
`SheetPivotStyle`, laid under any conditional format.

## Citations, re-checked by hand

Every `file:line` in the shipped code and its test was re-opened in
`/home/user/libreoffice-core`. **That tree is not the reference binary's source** — its
`configure.ac` declares `27.2.0.0.alpha0+`, the binary is 26.2.4.2, and the checkout is one bulk
import — so each of these is *this tree*, and the arm that measures the actual reference is the
`--convert-to fods` oracle below.

| Citation | What it should be | Verified |
| --- | --- | --- |
| `dpoutput.cxx`:76-79 | `SC_DP_FRAME_INNER_BOLD 20` / `OUTER_BOLD 40` / `COLOR Color(0,0,0)` | yes |
| `dpoutput.cxx`:127 | `ScDPOutputImpl::OutputDataArea` | yes |
| `dpoutput.cxx`:217 | `ScDPOutputImpl::OutputBlockFrame` | yes |
| `dpoutput.cxx`:264-296 | `lcl_SetStyleById`, and the three styles that carry anything | yes |
| `dpoutput.cxx`:297 | `lcl_SetFrame` | yes |
| `dpoutput.cxx`:595-596 | `bSkip` drops an empty data-layout row dimension | yes |
| `dpoutput.cxx`:748 | `ScDPOutput::HeaderCell`, and its column-subtotal frame and styles | yes |
| `dpoutput.cxx`:788 | `HeaderCell`'s row-subtotal frame across the row headers | yes |
| `dpoutput.cxx`:854-868 | `GetColumnsForRowFields` | yes |
| `dpoutput.cxx`:871-923 | `CalcSizes` | yes |
| `dpoutput.cxx`:886 | `mnHeaderSize = 2` for a grid header layout | yes |
| `dpoutput.cxx`:892-907 | `nPageSize`, `mnTabStartRow`, `mnMemberStartRow` | yes |
| `dpoutput.cxx`:912-923 | the table's last row and column from the result's extent | yes |
| `dpoutput.cxx`:969 | `outputPageFields` | yes |
| `dpoutput.cxx`:1002 | `outputColumnHeaders` | yes |
| `dpoutput.cxx`:1075 | `outputRowHeader` | yes |
| `dpoutput.cxx`:1087-1090 | `FieldCell` vs `MultiFieldCell` | yes |
| `dpoutput.cxx`:1135-1137 | `bLast`, `nMinIndentLevel`, `o3tl::convert(13 * …, px, twip)` | **was :1138 in two places, corrected** |
| `dpoutput.cxx`:1205 | `bColumnFieldIsDataOnly` | yes |
| `dpoutput.cxx`:1226 | `DeleteAreaTab(…, InsertDeleteFlags::ALL)` | yes |
| `dptabres.cxx`:1425 | `ScDPResultMember::FillMemberResults` | yes |
| `dpobject.cxx`:531 | `bFilterButton = IsSheetData() && GetFilterButton()` | yes |
| `pivottablebuffer.cxx`:296 | `mbCompact = mbSubtotalTop && mbOutline && compact` | yes |
| `pivottablebuffer.cxx`:1331-1336 | `clearContents(… HARDATTR \| STYLES …)` over the stated range | yes |
| `pivottablebuffer.cxx`:1365-1366 | `SetFilterButton(false)`, `SetExpandCollapse(mbShowDrill)` | yes |
| `pivottablebuffer.cxx`:1368 | `SetHideHeader(mnFirstHeaderRow == 0)` | yes |
| `pivottablebuffer.cxx`:1421-1426 | the start moved up by `pageFields + 1`, clamped at row one | yes |
| `pivotcachebuffer.cxx`:1093-1114 | only `XML_worksheet` is finalized | **was a bare file name, now a range** |
| `MemberResultFlags.idl` | `HASMEMBER 1`, `SUBTOTAL 2`, `CONTINUE 4` | yes |

Two off-by-one citations and one that named no line. Nothing pointed at a different statement.

## What the inherited work got wrong

**The indent is the drill-down step and nothing else.** `nIndent = 13 * (bLast ? … :
nMinIndentLevel + …)` with `nMinIndentLevel = mbExpandCollapse ? 1 : 0` (`:1135-1137`), and the
OOXML import sets `mbExpandCollapse` from the pivot's own `showDrill`
(`pivottablebuffer.cxx`:1366). `DynamicBubbleChart.xlsx` states `showDrill="0"` and has five row
fields, four of them not innermost — the salvaged code indented every one of them by 195 twips,
and the reference's own `.fods` of that workbook writes no `fo:margin-left="0.1354in"` anywhere.

**A single compact row field is not a different layout.** The salvaged code declined any pivot
with a compact row field. With one row field `GetColumnsForRowFields` returns `0` non-compact
fields plus one for `maRowCompactFlags.back()` — the same single column Excel wrote — `bLast` is
true so the indent is zero, and `nNumRowFields == 1` takes `FieldCell` rather than
`MultiFieldCell`. Nothing differs. Declining it cost thirteen pivots in three workbooks; the
oracle below now says all thirteen agree on every edge.

**Excel's `ref` excludes the filter rows and Calc's does not.** `finalizeImport` moves the data
pilot's start up by `pageFields + 1`, clamped at row one, and `CalcSizes` moves the table back
down by the same, so the table lands on the stated range unless the clamp bites. Now modelled
including the clamp. Twelve of the sixteen accepted pivots have page fields — `alle einzeln` two,
each of `Keywords_Mapping`'s eleven one or two — and none of them clamps, so the clamp itself is
unwitnessed and only the offset is measured.

**A hidden header was tried and refused.** `SetHideHeader(mnFirstHeaderRow == 0)` makes
`mnHeaderSize` 0, which looked like a free extension to the two documents that state it. It is
not: with `firstHeaderRow="0"` accepted, `033_Event_planning_tracker` gets 24 of 131 edges right,
misses 26, invents 56 and draws 25 at the wrong width — its reference table ends a row above the
range Excel wrote and its top row takes an inner rule where a first row takes an outer one.
Where the reference's table actually starts there is **not settled**. The case is declined, and
`035_Project_plan_for_law_firms`, the same template, with it.

## The oracle: both directions, not one

`check-grid.py` as inherited scored only the edges the prediction stated. That is a precision: a
prediction of nothing scores 100 % on it, and it cannot see a line the reference draws and we do
not. It now walks the whole of each pivot's rectangle, page-field rows included, and counts an
edge the reference states and the prediction does not (MISS) as well as one the prediction
invents (EXTRA) and one drawn at the wrong width (WRONG). Generated cell styles — read off the
`Pivot_20_Table_20_*` parent the reference gives each automatic style — and the generated indent
are compared the same way. The indent is compared against the rectangle's own floor, because a
workbook can give every cell of a sheet a left margin of its own and Calc's `ScIndentItem`
replaces rather than adds (`049_Expenses_calculator` states 210 twips on all 54 cells of its
pivot's rectangle).

| Workbook | pivots accepted | edges | MISS | EXTRA | WRONG | styles | indents |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `alle einzeln.xlsx` | 1 | 30250 | 0 | 0 | 0 | 6060 ✓ | ✓ |
| `Keywords_Mapping_Graphs_and_Charts.xlsx` | 11 | 749 | 0 | 0 | 0 | 142 ✓ | ✓ |
| `DynamicBubbleChart.xlsx` | 1 | 296 | 0 | 0 | 0 | 60 ✓ | ✓ |
| `007_Contextures_chart_sample.xlsm` | 1 | 120 | 0 | 0 | 0 | 18 ✓ | ✓ |
| `049_Expenses_calculator.xlsx` | 1 | 94 | 0 | 0 | 0 | 23 ✓ | ✓ |
| `037_Personal_money_tracker.xlsx` | 1 of 2 | 87 | 0 | 0 | 0 | 21 ✓ | ✓ |
| **total** | **16** | **31596** | **0** | **0** | **0** | **6324 ✓** | **✓** |
| `features/sheet-pivot-grid.xlsx` (fixture) | 1 | 99 | 0 | 0 | 0 | 22 ✓ | ✓ |

On `alle einzeln.xlsx` that is all **9092** bordered cells and all **30250** stated edges, with
nothing missed and nothing invented.

The null this is against: the sixteen rectangles hold **38772** edge slots and the reference
states **31596** of them, 81.5 %. A predictor that stated every slot would agree on 31596 and
invent 7176; one that stated none would miss 31596. Neither would get the widths, and the widths
carry most of the shape — 2494 of the 31596 are the 40-twip outer rule and the rest the 20-twip
inner one, so a predictor that got every edge's presence right and guessed 20 everywhere would
still be wrong 2494 times. This score has 0 in all three columns.

`pivot-grid.py` is the probe's own transcription, not the shipped C#, so this table alone would
not settle that the shipped code is right. The fixture is where the two meet: the same 99 edges,
22 styles and 2 indents are asserted by `SheetPivotGridTests` against the shipped code and by
this script against the predictor, and both agree with the reference. The rendered ink below is
the measurement of the shipped code on the eleven documents.

## What is declined, and the census that says nil

`IsGeneratable` accepts a pivot only when the geometry Excel states is the geometry `CalcSizes`
would compute, and only when the cache reads a range of the workbook.

*The cache.* `PivotCache::finalizeImport` finalizes only `XML_worksheet`; the `XML_external`,
`XML_consolidation` and `XML_scenario` arms of its switch are empty
(`pivotcachebuffer.cxx`:1093-1114), so no `ScDPObject` is ever created. Measured over the eleven:
the three workbooks whose caches are all `type="external"` write **0** `table:data-pilot-table`
elements for their **9** pivot parts, and the other eight write exactly **one for each of their
19**. The base rate is not in doubt here — it is 19/19 against 0/9.

*The data-layout dimension on the row axis.* With one data field its member result is empty, so
`bSkip` drops it (`:595-596`) and Calc lays out one row-label column fewer than Excel wrote.
Declined. **No document in the corpus states it**: `pivot-census.py` reports `rowDataPH=False`
for all 28 pivot parts of the eleven workbooks, so this is a refusal to act where there is
nothing to check against, not a modelled case.

*A grid header layout* (`mnHeaderSize = 2`, `:886`) cannot arise: `SetHeaderLayout` is called
only from `sc/source/filter/excel/xipivot.cxx` and `sc/source/filter/xml/xmldpimp.cxx`, never
from `sc/source/filter/oox`.

Three of the 19 worksheet-cached pivots are declined: `033` and `035` for the hidden header
above, and `037_Personal_money_tracker`'s `Monthly summary` for stating `firstDataCol=1` against
two row fields, which is Calc's compact packing of more than one field and a layout this does not
model.

## Reach: the eleven pivot-bearing documents

Base is `d07a9ce75`, the commit the salvaged WIP sits on; head is this branch. Ink is `|ink|%`
from `pdf-image-diff.py` summed unsigned over the document's pages against the banked 26.2.4.2
reference in `/home/user/gate-orig-r83/ref`; MAJOR is that script's own verdict.

| document | pages | base ink | head ink | Δ | base MAJOR | head MAJOR |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `alle einzeln` | 186 | 225.44 | **0.07** | −225.37 | 36 | **0** |
| `Keywords_Mapping_Graphs_and_Charts` | 46 | 10.79 | **1.68** | −9.11 | 11 | **0** |
| `DynamicBubbleChart` | 5 | 4.42 | **0.94** | −3.48 | 2 | 1 |
| `049_Expenses_calculator` | 4 | 2.18 | **0.90** | −1.28 | 2 | **0** |
| `007_Contextures_chart_sample` | 7 | 0.95 | **0.13** | −0.82 | 1 | **0** |
| `037_Personal_money_tracker` | 5 | 3.45 | **2.88** | −0.57 | 1 | 1 |
| `033_Event_planning_tracker` | 3 | 12.65 | 12.65 | 0.00 | 1 | 1 |
| `035_Project_plan_for_law_firms` | 4 | 1.75 | 1.75 | 0.00 | 0 | 0 |
| `027_Simple_personal_cash_flow_statement` | 10 | 3.03 | 3.03 | 0.00 | 1 | 1 |
| `026_Monthly_cash_flow_statement` | 11 | 0.59 | 0.59 | 0.00 | 0 | 0 |
| `053_Personal_asset_inventory` | — | 0.00 | 0.00 | 0.00 | 0 | 0 |
| **total** | | **265.25** | **24.62** | **−240.63** | **55** | **4** |

`053_Personal_asset_inventory` is the unscoreable one: `pdf-image-diff.py` refuses a comparison
whose page counts differ and reports no pages, at base and at head alike, so its `0.00` is an
absence of measurement and not a score. It has one pivot and its cache is external, so nothing is
generated for it either way.

The five documents at Δ 0.00 are byte-identical at the two binaries (see the sweep). Three of
them — `053`, `026`, `027` — are r103's control: no pivot-parented style in their `.ods` twins,
and no generated grid here.

r103 sized the prize by stripping the pivot-parented styles out of each `.ods` twin: 234.20 on
`alle einzeln` and 16.54 between the other seven. That is a sizing on the ODF side and not a
budget for this one — `Keywords_Mapping` gave back 9.11 where its twin was sized at 6.71 — but it
does say which documents are still owed something, and they are `033` (sized 0.55), `035` (0.53)
and `037`'s declined second pivot (part of its 3.59).

`alle einzeln` at 0.07 over 186 pages is level with the reference's own `.ods` of the same
workbook, which scores 0.02 over the same pages — the floor this seat could reach.

## Regressions

`sweep.sh` rendered all **307** sheets-track documents (241 `.xlsx`, 64 `.xls`, 2 `.xlsm`) at
both binaries with `SOURCE_DATE_EPOCH` fixed, checked `%%EOF` before hashing, deleted each render
as it went, and would have stopped itself with 1 GiB left. It did not stop and no render failed
or was truncated, so all 614 hashes are of complete PDFs.

**Six documents moved**, and they are exactly the six with a pivot the grid is generated for:
`alle einzeln`, `Keywords_Mapping_Graphs_and_Charts`, `DynamicBubbleChart`,
`049_Expenses_calculator`, `007_Contextures_chart_sample` and `037_Personal_money_tracker`. Every
one of the six is an improvement in the table above; the other 301 are byte-identical. Full
hashes in `sheets-sweep-r107.tsv`.

The other two tracks cannot be reached from here — the change is inside `XlsxReader` and
`XlsxFile`, which only the OOXML spreadsheet path enters — and the 64 `.xls` in the sweep, which
go through `XRay.Office.MsBinary` instead, are the control that says so: none of them moved.

## Tests

All eleven projects were run again on the exact tree that is committed, one project at a time so
a truncated run cannot hide as a pass, and the totals below are read out of that run's own output
(`tests.log`).

Ten non-fidelity projects — Containers 109, Core 544, Markup 259, OpenDocument 160,
Presentations 1081, Rendering 164, Spreadsheets 1308, Text 728, Vector 309, WordProcessing 1938 —
**6600 passed, 0 failed**, and the run reached the last of them.

`XRay.Office.Fidelity.Tests`: **Failed: 10, Passed: 542, Total: 552**. The ten by name, which are
the known ten and no eleventh:

    PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt   paginated.rtf .fodt .doc .docx
    TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop      list-label-overrun.docx .fodt .doc .odt
    SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt   sheet-rich-text.xlsx
    JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt  justify-shrink-2013.docx

`SheetPivotGridTests` is seven tests over `features/sheet-pivot-grid.xlsx`, a fixture written by
`make-fixture.py` whose `styles.xml` states one border and it is empty and one `cellXf` with no
weight, alignment or indent — so every non-zero number in the test is unreachable from the file.
With the two new source files removed and the four modified ones taken back to `d07a9ce75`, six
of the seven fail; the one that passes is
`TheOtherSheetOfTheSameWorkbookIsUntouched`, which asserts an absence.

Its expected values were re-read out of 26.2.4.2's own `.fods` of the fixture during this round
and all of them hold: 36 cells × 4 edges, `0.99pt` for 20 twips and `2.01pt` for 40, the
`Pivot_20_Table_20_*` parent on each of the 22 styled cells, and `fo:margin-left="0.1354in"` on
exactly the two cells where the outer row field's members start. Two numbers in its own remarks
did not hold and are corrected: removing `HeaderCell`'s row-subtotal frame costs the fixture 10
of its 99 edges and `alle einzeln` 30 of its 30250, not "eighteen" in both places.

## What is left unsettled

- **Where the reference puts a hidden-header pivot.** `033`/`035` are declined on measurement,
  not on a model. 1.08 of reach.
- **Compact packing of more than one row field.** `037`'s `Monthly summary` and any pivot whose
  `firstDataCol` is less than its row-field count. The salvaged code's remark asserted that the
  reference's output there "starts two rows above the cells Excel wrote"; that was never
  measured, and it is removed rather than repeated.
- **Calc clears the pivot's range and this does not.** `clearContents(… HARDATTR | STYLES …)`
  (`pivottablebuffer.cxx`:1336) and `DeleteAreaTab(…, ALL)` (`dpoutput.cxx`:1226) empty the range
  before `ScDPOutput` writes, so the reference's behaviour is replacement and this is a merge.
  For borders the two are indistinguishable in this corpus: **0 of the 19 pivot ranges in the
  eight worksheet-cached workbooks contain a cell whose `cellXfs` entry states any border**
  (`statedborders.py`). For alignment and indent they are not — 18 of the 19 ranges state an
  alignment, 3 state an indent, one states a bold; `049_Expenses_calculator` has 38 aligned and
  31 indented cells inside its pivot — and clearing those is left undone and unmeasured. It is
  also a difference this change does not create: the base keeps them too. `alle einzeln`, the
  document this was written for, states none of the four on any of its pivot's 9090 cells.
- **`mnTabEndCol` is widened to `mnTabStartCol + 1` when a pivot with page fields is narrower
  than two columns** (`:916-918`). Not modelled, and it cannot bite under the acceptance rules:
  they require at least one row-label column and at least one column item, so the table is always
  at least two columns wide.
- **The pivot's own `<format>`/`dxf` records** reach `maFormatOutput` and can put formatting back
  after the generated styles. Not read here and not censused.
