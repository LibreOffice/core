# Round 100 — the last three sheets seats: a merged fill's width, a BIFF run's colour, and what `alle einzeln` actually is

    ours   = XRay.Office.Cli @ 18f9da973 (base) and @ 18f9da973 + this round's diff (head)
    ref    = /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2 — and the banked
             reference PDFs at /home/user/gate-orig-r83/ref (the 947 originals) and
             /home/user/gate-odf-r80/ref (the 307 `.ods`)
    corpus = /home/user/sample-files/sheets — 243 `.xlsx`/`.xlsm` and 64 `.xls`;
             /home/user/corpus-odf/sheets — 307 `.ods`
    rule   = ink is `|ink|%` from `pdf-image-diff.py`, summed unsigned over the document's pages
    date   = 2026-09-11

## 0. The headline

| seat | what it was | what it is |
|---|---|---|
| **O32** merged fill width | characterised at r99, not fixed | **fixed.** `TOGAF9-Tool-ConfReqts-CSQ.xls` **16.75 → 12.92** and MAJOR **4 → 1**; pages 1, 4, 24 and 26 go to **0.00** and no page worsens. The rule is not *"the whole merge, clipped by the paper"* — the fill stops **one column past the block's last**, measured at 26.2.4.2 on a fixture built to separate the two. Corpus reach **84 of 614 sheets renderings, 0 failures**, summed unsigned ink over them **783.39 → 700.41** and MAJOR pages **346 → 275**, 51 improving and 6 worsening |
| **O33** BIFF shape run colour | 119 spans in 2 of 64, unimplemented | **implemented, and the seat's arithmetic is corrected.** `PC1000.xls` 1.92 → **1.59** with its non-black span profile now the reference's **exactly** (43 of 43), and `TOGAF9` gains the one red span the reference draws. **The 43-vs-119 disagreement was not the `CONTINUE` assumption**: fixing that moves the record count 43 → **48**, and the rest of the gap is that **113 of the 119 are EHEST's, whose boxes this tree does not draw at all** — a dropped-shape defect, not a colour one, and it is left as a new seat |
| **O17** `alle einzeln.xlsx` 225.44 | *"cell presentation or grid geometry"* | **reached, and named to one mechanism.** 150 of its 186 pages are **0.00 exact**; all 36 defective pages are the `Pivot` sheet. **26.2.4.2 generates `Pivot Table *` cell styles when it imports a SpreadsheetML pivot table and this tree generates none** — the workbook's own `styles.xml` states **one border and it is empty**. The `.ods` twin of the identical workbook renders at **0.02 over the same 186 pages**, and stripping only the pivot borders out of that `.ods` puts it back at **229.12**. Not fixed |

**Two of r99's readings are corrected by measurement rather than by argument** and both corrections
are in §2 and §3: the merged fill is *capped*, not paper-clipped, and O33's EHEST half is not a
colour defect.

## 1. O17 — `alle einzeln.xlsx` is a pivot table's generated cell formatting, and the `.ods` twin proves it

Taken first, because two rounds listed it last and did not reach it.

### 1.1 Where the 225.44 is

Rendered whole under `SOURCE_DATE_EPOCH` and scored page by page against the banked reference
(`ink-o17-xlsx.txt`): **186 pages, 225.44 summed unsigned ink, 36 MAJOR** — which reproduces the
seated figure exactly and is the control on the instrument.

**Pages 1 to 36 are every one of the MAJOR pages and pages 37 to 186 are 0.00 to two decimals.**
That is 150 of 186 pages already pixel-equal. Pages 1-36 are the `Pivot` sheet and 37-186 the
`alle einzeln` sheet — read off the renderings rather than assumed: page 36 opens `TÜV-Nord` and
page 37 opens the second sheet's header row, `Ort  Strecke  m/w`.

Every one of the 36 carries the same region: *"a fill or background shading the reference has and
we do not"* over 47 % of the page.

### 1.2 What the region is, per operator

`get_drawings` on page 5, ours beside the reference's:

| | drawings |
|---|---|
| ours | **none at all** |
| reference | **65 strokes**, black, 1.0 pt with a 2.0 pt left edge, at x 137.57 / 246.95 / 360.28 / 466.53 and a horizontal every ~13 pt |

So it is not a fill: it is a **cell border grid**, and we draw none of it. The text differs with
it — ours puts `342` at x 229.30 (right-aligned in its column) and the reference at **148.34**
(left-aligned, and indented).

### 1.3 The workbook states no border at all, and the reference invents them

`xl/styles.xml` holds `<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>`.
There is nothing to read.

26.2.4.2's own `--convert-to fods` of the same file holds **44 cell styles carrying an
`fo:border`**, of which **41** are parented to a `Pivot_20_Table_20_*` style — `Corner`,
`Field`, `Category`, `Title`, `Value`, `Result`. Those are Calc's own DataPilot output styles:
`lcl_SetStyleById` creates each one on demand and gives `Result` and `Title` a bold weight and
`Category` and `Title` a left justification (`sc/source/core/data/dpoutput.cxx`:264-294 **in this
tree**, which declares 27.2.0.0.alpha0+ and is not the reference binary's source), and
`lcl_SetFrame`/`ApplyFrameAreaTab` hard-format the frames beside them.

The geometry the reference lays them over is the one the file itself states —
`<location ref="A4:I1013" firstHeaderRow="1" firstDataRow="2" firstDataCol="6" rowPageCount="2"
colPageCount="1"/>` — and the `.ods` twin's grid matches it row for row: rows 1-2 page fields,
row 3 blank, row 4 `Corner`/`Field`, row 5 `Field`/`Category`, rows 6-1011 `Category`/`Value`,
rows 1012-1013 `Title`/`Result`.

### 1.4 The `.ods` twin is the control, and it is decisive

`/home/user/corpus-odf/sheets/done-013/ods/alle einzeln.ods` is 26.2.4.2's own conversion of the
same workbook, so it carries those generated styles as ordinary markup. This tree renders it at:

| | pages | sum `\|ink\|%` | MAJOR |
|---|---:|---:|---:|
| `alle einzeln.ods` | 186 | **0.02** | **0** |
| `alle einzeln.xlsx` | 186 | **225.44** | 36 |

**Identical content, identical page count, and the whole of the difference is which spelling
states the pivot's formatting.** Our painter, our borders, our alignment, our indent and our bold
are all already right; what is missing is that nothing generates the styles on the SpreadsheetML
path.

### 1.5 Which half of the generated formatting costs the ink

`o17-variants.py` strips one half out of the correct `.ods` and renders that (scored against the
same banked `.ods` reference):

| variant | pages | sum `\|ink\|%` | MAJOR |
|---|---:|---:|---:|
| the `.ods` as it stands | 186 | **0.02** | 0 |
| less every `fo:border*` | 186 | **229.12** | **36** |
| less the `Category` indent and the `Category`/`Title` left justification | 186 | **19.85** | 4 |
| the `.xlsx`, which has neither | 186 | 225.44 | 36 |

**The borders alone reproduce the whole seat** — 229.12 against 225.44, and the same 36 MAJOR
pages — and the alignment is worth about 20 on top of a scale where regions saturate. An
implementation should start with the grid and the outer frame; the justification and indent are
the second order.

### 1.6 Reach, and what it is not

**21 of the 550 `.xlsx`, `.xlsm` and `.ods` of the sheets corpora state a pivot table**
(`pivot-census.py`, `pivot-census.txt`): 11
`.xlsx`/`.xlsm` and 10 `.ods`. The `.ods` ten cannot be affected — they already carry the styles,
which is the whole of §1.4 — so the reachable set is the **11**.

Of the eleven, 26.2.4.2's own `fods` gives **8** at least one cell parented to a
`Pivot Table *` style and **3** none:

| document | pivot-parented automatic styles | style-name occurrences | of them bordered |
|---|---:|---:|---:|
| `037_Personal_money_tracker` | 35 | 133 | 125 |
| `Keywords_Mapping_Graphs_and_Charts` | 9 | 111 | 111 |
| `033_Event_planning_tracker` | 19 | 85 | 85 |
| `035_Project_plan_for_law_firms` | 23 | 79 | 57 |
| `DynamicBubbleChart` | 11 | 78 | 78 |
| `007_Contextures_chart_sample` | 21 | 33 | 33 |
| `049_Expenses_calculator` | 27 | 27 | 27 |
| `alle einzeln` | 41 | 56 | 56 |
| `026_Monthly_cash_flow_statement`, `027_Simple_personal_cash_flow_statement`, `053_Personal_asset_inventory` | 0 | — | — |

*The occurrence column is `table:style-name="ceN"` counted in the markup and is **not** a cell
count — ODF's `number-columns-repeated` compresses a run of identical cells into one attribute,
which is why `alle einzeln`'s 1013-row pivot shows 56. It is a presence bound, not a size.*

**And a construct count is not a reach figure — N11.** The seven other pivot-styled documents'
whole-document ink, pivot or not, is 0.95, 12.65, 1.75, 3.45, 2.19, 4.55 and 10.79 — **36.33
between them** against `alle einzeln`'s 225.44 — and no measurement here says how much of that
36.33 is the pivot: several of them carry charts, which is a different seat. The honest statement
is that **one document is worth 225.44 and the other seven are worth at most 36.33 between them**,
and the three with no pivot-parented cell at all (0.59, 3.03 and 0.00) cannot be this.

### 1.7 What is left

Not fixed. The implementation needs Calc's own region assignment — which cells of the output range
are `Corner`, `Field`, `Category`, `Title`, `Value`, `Result` — plus the per-cell border edges,
which vary with a category group's first, middle and last row (`ce3`/`ce4`/`ce5` in the twin).
Every one of those answers is readable off the `.ods` twin cell by cell, so the fixture problem is
solved before the work starts; what remains is the rule that produces them from
`pivotTableDefinition`'s `location`, `firstHeaderRow`, `firstDataRow` and `firstDataCol` plus the
page-field rows above.

*And the `.xls` half of that census is not zero and was not counted here.* `pivot-census.py`
scans zips, so the 64 BIFF workbooks are outside it — but two of the ten `.ods` in the list are
26.2.4.2's own conversions of `TOGAF9-Tool-ConfReqts-CSQ.xls` and `orbus_togaf_tool_csq.xls`, so
**at least two `.xls` state a pivot table as well**. Neither is a witness for this seat:
`TOGAF9`'s `.ods` twin holds **3** pivot-parented styles against `alle einzeln`'s 41 and renders
at **9.04 over 28 pages with 0 MAJOR** where its `.xls` is at 12.92, so whatever is left there is
not the pivot's formatting.

## 2. O32 — a merged fill stops one column past the block, and that is not what the seat said

### 2.1 The rule, measured before it was implemented

r99 read `ScOutputData::DrawBackground`'s column loop as *"paints the fill across the whole merge
and lets the paper clip it"*. The loop's own break says otherwise —

    for (SCCOL nMerged = 0; nMerged < nMergedCols; ++nMerged) {
        SCCOL nCol = nX + nOldMerged + nMerged;
        if (nCol > mnX2 + 2) break;
        nNewPosX += mpRowInfo[0].basicCellInfo(nCol - 1).nWidth * nLayoutSign;
    }

— because the column whose width is added is `nCol - 1`, so the last one that can be added is
`mnX2 + 1`: **one column past the block's last, and no further** (`sc/source/ui/view/output.cxx`
:1148-1172 **in this tree**, 27.2.0.0.alpha0+, which is not the reference binary's source).

r99's witness could not tell the two apart — `TOGAF9`'s merge is **two** columns wide on a
one-column block, where "the whole merge" and "one past the block" are the same answer.
`tests/corpus/features/sheet-merge-fill-overflow.fods` was built to separate them: 2 cm column A
alone on page 1 with a merge of **A:D (11 cm)** and one of **A:C (8 cm)**, both well inside the
17 cm of printable width so the paper cannot be doing the clipping. 26.2.4.2's own PDF:

| | reference draws | width |
|---|---|---:|
| `ZMERGE4`, A1:D1 | 56.64 – 198.43 | **141.79 pt = 5 cm = A + B** |
| `ZMERGE3`, A2:C2 | 56.64 – 198.43 | **141.79 pt = 5 cm = A + B** |
| `ZPLAIN`, unmerged | 56.64 – 113.39 | 56.75 pt = 2 cm, the control |
| `ZMERGE32`, A4:C5 | 56.64 – 198.43 over **both** rows | 141.79 pt |

**The four-column merge and the three-column merge are drawn at the same width.** A rule painting
the whole merge would draw 311.81 and 226.77 there. So the cap is real and it is one column.

Two more things the fixture settles. **A merge spanning rows is extended on every row it covers**,
not only on the origin's — the reference coalesces rows 4 and 5 into one rectangle 141.79 wide.
And **page 2 is the control for the other direction**: its block is B to E, the origins are off to
the left, and the reference draws B+C+D (255.18) and B+C (170.13) — the covered columns' own
widths, with no overflow into E. Only the origin carries `ATTR_MERGE` and therefore a column
count; a covered cell carries `ATTR_MERGE_FLAG` and extends nothing.

**That second half is not decoration — getting it wrong costs ink.** The first cut of this fix
widened any covered cell whose merge crossed the block edge, and on `TOGAF9` that put page 27 from
**0.00 to 0.65** and page 2 from 0.00 to 0.01, because both pages carry merges whose origin is on
an earlier page column. Restricting the extension to the origin took both back to 0.00.

Ours reproduces the fixture's every right edge: 198.43, 198.43, 113.39 and 198.43 on page 1 (four
rectangles where the reference coalesces rows 4-5 into three), maximum 311.81 on page 2.

**And a third guard, which the corpus cannot witness and the code must have anyway.** The placed
columns handed to `DrawBackgrounds` include the repeated print-title columns, at their own band's
x and with their own fixed width, so only a column of the page's *own* block may be widened.
`guardcheck.py` is the check that adding it changed nothing measurable: of the 550 `.xlsx`,
`.xlsm` and `.ods` exactly **one** declares repeated print *columns*
(`037_Personal_money_tracker`) and no `.ods` states a `table:title-columns` at all, so the guard
was tested by rendering that document and all 64 `.xls` at the binary with it and the binary
without — **64 checked, 0 differing**. The sweep below was taken with the unguarded binary and
therefore stands for the committed tree.

### 2.2 What it is worth on the witness

`TOGAF9-Tool-ConfReqts-CSQ.xls`, whole document, against the banked 26.2.4.2 reference
(`ink-togaf-base.txt`, `ink-togaf-head.txt`):

| | pages | sum `\|ink\|%` | MAJOR |
|---|---:|---:|---:|
| base | 28 | **16.75** | 4 |
| head | 28 | **12.92** | **1** |

Five pages move and **none worsens**: page 1 **1.80 → 0.00**, page 4 **1.22 → 0.00**, page 24
**0.14 → 0.00**, page 26 **0.52 → 0.00**, page 21 2.67 → 2.52. The other 23 pages are identical to
two decimals.

*r99 predicted 3.54 from pages 1, 4 and 26. The measured figure is **3.83**, because page 24 was
not in that list and page 21 — the displaced-marks page the brief said explicitly was **not** this
— gives back 0.15 of its own.*

## 3. O33 — the colour is implemented, and 113 of the seat's 119 spans are a different defect

### 3.1 The record-side count, with r99's `CONTINUE` assumption corrected

r99 left *"its two legs disagreed on the colour count, 43 from the records against 119 from the
reference"* explicitly unresolved, and named the cause: its census assumed a single `CONTINUE` of
characters. **That is the right cause and it is worth 5 runs, not 76.**

`txo-runs-r100.py` concatenates every `CONTINUE` after a `TXO` and takes the run array as the last
`formatSize` bytes of it, which needs no knowledge of how the string was split. Over the 64 `.xls`:

| | boxes | applied runs | stating a non-black colour | stating an italic | parsing plausibly |
|---|---:|---:|---:|---:|---:|
| text boxes and buttons (`ftCmo` 6, 7) | 108 | **263** | **48** | **0** | **108 of 108** |
| cell comments (type 25) | 47 | 104 | 104, and not this seat's | **0** | 47 of 47 |

*"Plausibly" is the array opening at character 0 and never going backwards, which character bytes
misread as run entries are not. A cell comment's text never goes through the shape path —
`XclImpNoteObj` calls `SetInsertSdrObj(false)` and `XlsDrawing.Build` skips `ftCmo` type 25 — so
its 104 colour-stating runs are counted here only to show the parse is sound on them too.* r99's figures were 263 runs and **43** colours; the five it lost
are in the ten boxes `txo-continues.py` finds needing more than one character `CONTINUE` — **all
ten in `EHEST-Pre-departure-checklist`, which uses four `CONTINUE`s where the other 98 boxes of
the corpus use two.**

So the two legs are 48 and 119, and **the remaining gap is not a parse at all — a formatting run
is not a span.** One run covers a stretch of characters that the reference resolves into one span
per paragraph, and 26.2.4.2's own `fods` counts the second.

**This reader was never the one with the bug.** `XlsDrawing.ReadText` reads through joined
continuations and its run array is correct on all 108 boxes; it was r99's standalone census script
that assumed one.

### 3.2 The colour, implemented

`SheetShapeRun` gains a `Colour?`; `XlsCellFormats.StatedColour` resolves a `FONT`'s `icv` through
the workbook's palette and answers null for `0x7FFF` and for an index outside it;
`SheetShapePainter` carries it beside each shaped stretch and draws
`Paint.Solid(piece.Colour ?? Colour.Black)`. The slant stays uncarried on N23's census.

`tests/corpus/features/sheet-shape-run-colour.xls` is `…​.fods` put through 26.2.4.2's own
*MS Excel 97* filter, so its `TXO` run array and `FONT` records are the reference's own writing.
26.2.4.2 draws four spans for it and this tree now draws the same four, each within **0.35 pt** of
the reference's x:

| span | reference | ours |
|---|---|---|
| `ZCELL`, cell text | `#000000` at x 58.68 | `#000000` at x 58.68 |
| `ZREDRUN` | `#ff0000` at x 62.28 | `#ff0000` at x 62.56 |
| `ZPLAINRUN`, whose `FONT` states automatic | `#000000` at x 118.91 | `#000000` at x 119.23 |
| `ZBLUERUN` | `#0000ff` at x 192.87 | `#0000ff` at x 193.22 |

The two black spans are the controls and neither is decoration: `ZCELL` never went through this
path at all, and `ZPLAINRUN` is a run of the same text box whose `FONT` states automatic — so a
rule that painted every shape run in the first run's colour would fail on it.
`SheetShapeRunColourTests` asserts all four and fails at the base.

### 3.3 Reach, measured over the 64 `.xls`

Only the BIFF reader sets the new field, so no `.xlsx` or `.ods` can move; the sweep is the 64
`.xls` at the merge-fix binary against the merge-fix-plus-colour binary, hashed
(`colour-sweep.tsv`). **2 of 64 move and 62 are byte-identical, with 0 failures.**

| document | pages | base `\|ink\|%` | head | non-black spans, base → head | reference's |
|---|---:|---:|---:|---:|---:|
| `PC1000.xls` | 13 | 1.92 | **1.59** | 36 → **43** | **43** |
| `TOGAF9-Tool-ConfReqts-CSQ.xls` | 28 | 12.92 | 12.92 | 6 → **7** | **7** |

`PC1000` lands on the reference's colour profile exactly. `TOGAF9`'s ink does not move to two
decimals on any page, and what it gains is the one span the reference draws in `#ff0000` —
`Indicative only`, page 6, 16.3 pt, at x 532.60 against our 526.42, which is a
placement question and not this one.

*That corrects r99's other side note.* It read 26.2.4.2's `fods` of `TOGAF9` as holding **0**
non-black shape spans and concluded "O32 is not O33". The rendered reference holds **one**, and
this tree now draws it.

### 3.4 EHEST is not a colour defect — its boxes are not drawn at all

`EHEST-Pre-departure-checklist` does **not** move, and that is the round's substantive correction
to the seat. Its 35 colour-stating runs name palette indices 11, 51 and 10 — `#00ff00`, `#ffcc00`
and `#ff0000` in the BIFF8 default palette, which is what the workbook uses because it states no
`PALETTE` record — and those are exactly the three colours 26.2.4.2 draws. But:

| | ours | reference |
|---|---:|---:|
| spans reading `ACCEPTABLE`, `CAUTION` or `HIGH RISK` | **18** | **49** |
| of them at 13.0 pt in green, amber or red | **0** | **31** |
| filled paths on page 8 | 5 | **7** |

**We draw none of those boxes**, in any colour, and their fills are missing too. `xray-office
extract` settles which side of the pipeline it is on: **60 of the workbook's 75 shape-path text
boxes are absent from the extracted text**, so the loss is in the reader and not in the painter.

The pattern says where to look: the missing set is a **per-sheet tail** — `HIGH RISK`,
`RESET SCORES`, `INSERT QUESTION`, `DELETE QUESTION`, `ADD NEW SHEET`, `DELETE SINGLE LINE`, six
or seven on each of the ten data sheets — which is the shape-to-`OBJ` pairing in `XlsDrawing.Build`
running out (`if (at >= _objects.Count) break;`) or drifting. The workbook holds **105 `OBJ`** and
**85 `TXO`** records.

**So O33's colour half is closed and 113 of its 119 spans were never colour.** The seat that
remains is *a BIFF sheet's last few shapes are dropped*, and it is worth 8.65 of ink on this one
document.

## 4. O32's confinement over the whole sheets corpus

`sweep-merge.py` renders every `.xlsx`, `.xlsm`, `.xls` and `.ods` of the two sheets corpora at
the base binary and at the merge-fix binary under `SOURCE_DATE_EPOCH`, one temporary directory per
render, checks each PDF for `%%EOF` before hashing it and deletes it again:

| | |
|---|---:|
| documents rendered, twice each | **614** |
| **renderings that move** | **84** |
| failures or truncations on either leg | **0** |

By spelling: **42 `.ods`, 34 `.xlsx`, 8 `.xls`, 0 `.xlsm`** — and they come in **pairs**. Every one
of the 42 `.ods` is the converted twin of one of the 42 originals, which is the check that the
change is a property of the document and not of the reader: the same merge crossing the same page
boundary is found by the ODF path and by the SpreadsheetML and BIFF paths alike.

**No gate column can move and the check is not only the argument.** `DrawBackgrounds` emits fills
and nothing else, so no glyph is added or removed; read out of the two renderings anyway,
`TOGAF9-Tool-ConfReqts-CSQ.xls` is **28 pages and 156085 alphanumeric characters at both binaries**
and `036_Simple_to-do_list` is **4 and 341 at both**.

**And the ink, each mover against its own banked reference** (`score-movers.sh`,
`score-movers.tsv`; one output directory per document, `%%EOF`-checked, the whole document scored
page by page at both binaries):

| | |
|---|---:|
| renderings scored | **84** |
| pages | **4487** |
| sum `\|ink\|%`, base → head | **783.39 → 700.41** |
| MAJOR pages, base → head | **346 → 275** |
| improve / worsen / level | **51 / 6 / 27** |

The ten largest movements:

| document | pages | base `\|ink\|%` | head | base MAJOR | head MAJOR |
|---|---:|---:|---:|---:|---:|
| `2023-qhp-form-and-rate-combined-checklist-final__xlsx` | 100 | 8.77 | **3.09** | 13 | 3 |
| `2023-qhp-form-and-rate-combined-checklist-final__ods` | 95 | 8.90 | **3.57** | 17 | 7 |
| `orbus_togaf_tool_csq__xls` | 75 | 50.90 | **45.72** | 24 | 20 |
| `orbus_togaf_tool_csq__ods` | 75 | 65.43 | **60.26** | 27 | 22 |
| `DOE-C2M2-V1.1-to-DOE-C2M2-V2.1 (1.1.0)__xlsx` | 91 | 6.84 | **2.19** | 2 | 1 |
| `DOE-C2M2-V1.1-to-DOE-C2M2-V2.1 (1.1.0)__ods` | 91 | 6.83 | **2.18** | 2 | 1 |
| `7-memento-2015-transports-aeriens-b__ods` | 190 | 13.84 | **9.57** | 4 | 4 |
| `TOGAF9-Tool-ConfReqts-CSQ__ods` | 28 | 13.05 | **9.04** | 3 | 0 |
| `TOGAF9-Tool-ConfReqts-CSQ__xls` | 28 | 16.75 | **12.92** | 4 | 1 |
| `ODs-February-2022-Airbus-Commercial-Aircraft__xlsx` | 175 | 4.62 | **1.03** | 4 | 3 |

**The six that worsen are three documents in both spellings**, and they matter more than their
size: `Global_Market_Forecast` 0.08 → 1.41 and 0.19 → 1.56 over 567 pages,
`036_Simple_to-do_list` 0.44 → 0.55 and 12.86 → 12.46 (the `.ods` twin of that one *improves*),
`fse_identification_form` 0.38 → 0.45 and 0.25 → 0.32, and `DynamicBubbleChart` 4.55 → 4.42 /
3.20 → 3.45. In each the region is *"a fill or background shading we draw and the reference does
not"* against the right edge — the reference stopped at the block and we went one column past.

### 4.1 The narrower rule was tried and is measurably worse

The obvious response is to restrict the extension to a merge whose origin is the block's *first*
column, which is the only shape the fixture witnesses. **Measured rather than assumed**: the
restricted binary was built and the six worsening renderings plus the ten largest improvements
were re-scored (`check-narrow.sh`, `narrow-rule.txt`). Over those 22 renderings —

| | base | as committed | restricted |
|---|---:|---:|---:|
| sum `\|ink\|%` | 237.08 | **182.19** | 197.92 |

— the restriction buys back the three worsening documents (1.37, 0.11 and 0.07) and gives up far
more: `043_Buy_a_car` 0.09 → 2.95, `ODs-February` 1.03 → 4.62, `DOE-C2M2` 2.19 → 5.19, all the way
back to the base. So the extension is right for a merge that starts inside the block on most
documents and wrong on three, and **what separates them is not established** — that is the
residual §6 records rather than a rule this round could pick by argument.

## 5. Tests

Ten non-fidelity projects, run individually and totalled here rather than through the solution —
every number read out of this round's own output: Core **528**, Containers **109**, Markup **259**,
Text **728**, OpenDocument **146**, Presentations **1047**, Rendering **164**, Vector **309**,
WordProcessing **1938**, Spreadsheets **1295**. **0 failed and 0 skipped in every one.** Solution
build 0 warnings, 0 errors.

`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed of 552**, and the 552 is
`dotnet test --list-tests`' own count rather than a carried number. The ten are exactly the known
names: `TabStopComparisonTests` x4 (`list-label-overrun.doc`/`.fodt`/`.odt`/`.docx`),
`PageDrawingComparisonTests` x4 (`paginated.docx`/`.fodt`/`.rtf`/`.doc`),
`JustificationShrinkComparisonTests` (`justify-shrink-2013.docx`) and
`SheetDrawingComparisonTests` (`sheet-rich-text.xlsx`).

**Two new classes, three tests, and each class has a test that fails at the base.** Reverting
`SheetPageDecoration.cs` to `18f9da973` and rebuilding gives
`SheetMergeFillOverflowTests` **1 failed, 1 passed** — the one that fails is the overflow and the
one that passes is the page-2 control, which is the point: the origin-off-the-block case must not
move. Reverting `SheetShapePainter.cs` gives `SheetShapeRunColourTests` **1 failed, 0 passed**.

Their fixtures are `tests/corpus/features/sheet-merge-fill-overflow.fods`, authored for this and
with 26.2.4.2's own numbers in its header, and `sheet-shape-run-colour.xls`, which is the `.fods`
beside it put through 26.2.4.2's own *MS Excel 97* filter so that its `TXO` and `FONT` records are
the reference's writing rather than hand-assembled.

## 6. What this round could not settle

- **O17 is characterised and not fixed**, §1. The mechanism is named to one sentence and the
  ground truth for every cell of it is in the `.ods` twin, but generating Calc's DataPilot output
  formatting is a subsystem rather than a hunk: it needs the region assignment (`Corner`, `Field`,
  `Category`, `Title`, `Value`, `Result`) and the per-cell border edges, which vary with a
  category group's first, middle and last row. Start with the borders — they are 229.12 of the
  225.44 and the justification is 19.85.
- **A BIFF sheet's last few shapes are dropped**, §3.4. 60 of `EHEST-Pre-departure-checklist`'s 75
  shape-path text boxes never reach the content tree, six or seven per sheet and always the tail.
  Worth 8.65 of ink on that one document and unmeasured elsewhere; the workbook holds 105 `OBJ`
  against 85 `TXO`, and `XlsDrawing.Build`'s shape-to-object pairing is where to start.
- **The one mover that worsens.** `036_Simple_to-do_list` goes 0.44 → 0.55 over four pages, and
  the region is *"a fill or background shading we draw and the reference does not"* at
  x 0.94-0.99 of page 1: its `B2:L2` merge is painted to 777.32 where the reference stops at the
  block's own right edge, 750.05. The reference extends a merge one column past the block on the
  fixture and does not here, and **what separates the two cases is not established** — the
  candidates are that the reference's printed block ends one column further right than ours on
  that sheet, and that the outer loop's `nX + nMergedCols <= mnX2 + 1` guard reaches the origin
  differently when the merge does not begin at `mnX1`. The same shape is
  `Global_Market_Forecast` (0.08 → 1.41 over 567 pages) and `fse_identification_form`, so it is
  three documents in six renderings, and §4.1 shows the obvious restriction costs more than it
  saves. This is the one place in the round where a rule was chosen on a net figure rather than on
  a mechanism.
- **`TOGAF9`'s remaining 12.92** is 23 pages of diffuse displaced glyphs plus page 21's 2.52,
  which r99 characterised as a displaced-marks region and which is still not characterised. Its
  `.ods` twin is at **9.04 with 0 MAJOR**, so about three points of it are the BIFF path's own.
- **`alle einzeln`'s four alphanumeric characters** — 278872 against the reference's 278868 over
  186 pages — are untouched and were not looked for. They are one part in seventy thousand.
- **The `.xls` pivot census was not done.** `pivot-census.py` walks zips, so the 64 BIFF workbooks
  are outside its 21; two of them are known to hold a pivot only because their `.ods` twins do.
  A `SXVIEW` scan would settle it and costs one pass.

## 7. Files

| file | what it is |
|---|---|
| `results.md` | this |
| `pivot-census.py`, `pivot-census.txt`, `pivot.list` | which corpus documents state a pivot table, and how much `Pivot Table *` formatting 26.2.4.2 generates for the OOXML ones |
| `o17-variants.py` | the decomposition of O17: the `.ods` twin with only its pivot borders, or only its pivot alignment, stripped out |
| `ink-o17-xlsx.txt` | `alle einzeln.xlsx` per page against the banked reference — the 225.44, and the 150 pages at 0.00 |
| `sweep-merge.py`, `merge-sweep.tsv` | O32's confinement: every sheets document rendered at both binaries, `%%EOF`-checked and hashed |
| `score-movers.sh`, `score-movers.tsv` | the movers rendered again and scored on ink at both binaries |
| `ink-togaf-base.txt`, `ink-togaf-head.txt`, `ink-togaf-colour.txt` | `TOGAF9-Tool-ConfReqts-CSQ.xls` per page at the base, with the merge fix, and with the colour on top |
| `txo-runs-r100.py`, `txo-runs-r100.tsv`, `txo-runs-r100.txt` | r99's `TXO` census with its `CONTINUE` assumption corrected, and a plausibility column |
| `txo-continues.py` | how many `CONTINUE` records each shape-path `TXO` uses — the ten that broke the old parse |
| `sweep-colour.py`, `colour-sweep.tsv` | O33's confinement over the 64 `.xls` |
| `spancolours.py` | every text span's colour in a PDF, tallied — the instrument §3 is built on |
| `check-narrow.sh`, `narrow-rule.txt` | the restricted rule of §4.1, built and scored over 22 renderings |
| `guardcheck.py` | whether the repeat-print-column guard changes any rendering: the 64 `.xls` and the one document in 550 that declares repeated print columns, at the binary with it and without |
