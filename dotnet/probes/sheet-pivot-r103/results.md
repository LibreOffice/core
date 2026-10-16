# Round 103 — a BIFF sheet's dropped shapes, and what a pivot table's generated grid is worth

    ours   = XRay.Office.Cli @ e58567aea (base) and @ e58567aea + this round's diff (head)
    ref    = /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2 — and the banked
             reference PDFs at /home/user/gate-orig-r83/ref (the 947 originals) and
             /home/user/gate-odf-r80/ref (the 307 `.ods`)
    corpus = /home/user/sample-files/sheets — 240 `.xlsx`, 2 `.xlsm` and 64 `.xls`;
             /home/user/corpus-odf/sheets — 307 `.ods`
    rule   = ink is `|ink|%` from `pdf-image-diff.py`, summed unsigned over the document's pages
    date   = 2026-09-11

## 0. The headline

| seat | what it was | what it is |
|---|---|---|
| **O37** a BIFF sheet's last few shapes are dropped | 31 spans the reference draws and we draw 0 of; *"`XlsDrawing.Build`'s shape-to-`OBJ` pairing running out"* | **fixed, and it was three defects and not one — none of them the pairing.** `EHEST-Pre-departure-checklist.xls` draws **33 of the reference's 33** colour-stating shape spans against **0** at the base, each within **0.13 pt in x and 0.92 pt in y** of 26.2.4.2's own. Ink **8.65 → 8.06**. Corpus reach **9 of 64 `.xls`, 0 failures**, 60 non-`.xls` renderings byte-identical. Summed over the nine the ranking column moves **31.44 → 31.58**, and §3.4 shows why: the change is measurably *right* on every document it touches and exposes two defects underneath it |
| **O17** a pivot table's generated cell formatting | characterised at r100, not fixed; *"start with the grid"* | **not fixed, and the reach is now measured on all eleven rather than bounded.** Stripping *only* the pivot-parented borders out of each document's `.ods` twin says what the generated grid is worth there: **234.20 on `alle einzeln` and 16.54 between the other seven** — 6.71 of it on `Keywords_Mapping_Graphs_and_Charts`, which r100's whole-document bound could not separate out — and **0.00 on all three that carry no pivot-parented style**, which is the control. §5 writes the algorithm out to `file:line` |

**Two things this round takes back from earlier rounds.** r100 §3.1 said EHEST's ten boxes *"use four `CONTINUE`s where the other 98 boxes of the corpus use two"*. They do not: the third and fourth `CONTINUE` after those ten `TXO` records are **Escher shape containers**, not string continuations, and reading them as text is exactly the defect (§1.1). And O19's entry in `OPEN-ISSUES.md` says a `TXO`'s runs are in the *last* `CONTINUE` after the record — for those ten boxes the last `CONTINUE` is a shape.

## 1. O37 — three defects, and the seat named none of them

The seat said the loss was `XlsDrawing.Build`'s `if (at >= _objects.Count) break;`. **It is not**:
that guard cannot fire on this workbook. `escher-walk.py` assembles each sheet's Escher stream
exactly as the base reader did — `MSODRAWING` records only, no `CONTINUE` after an `OBJ` or a
`TXO` — and walks it (`ehest-escher.txt`): the five sheets that hold a group carry **11 `OBJ`
records against 10 shapes with client data**, and the other four carry 7 against 7. The shape list
runs *short* of the object list on every sheet, so the pairing never runs out; what it does is
finish early. The three real causes are below, each measured separately.

### 1.1 A `TXO` swallows the drawing's own `CONTINUE` records

Once a sheet's Escher stream passes the 8224-byte record ceiling, Excel stops writing `MSODRAWING`
records and writes the rest of the drawing as bare `CONTINUE` records, **interleaved between the
`OBJ` and `TXO` records that follow**. `EHEST`'s first data sheet is the case: 8129 bytes of
`MSODRAWING` and two further shape containers, 126 and 8 bytes, arriving as `CONTINUE`s after a
`TXO`'s own two.

`BiffRecordReader.MoveNext` joined every `CONTINUE` behind a record into it, `OBJ` excepted. So the
`TXO` at that boundary ate the 126-byte `SpContainer`, the drawing lost its last shape, and the
`DgContainer`'s declared length — **8255 body bytes against the 8201 the reader had** — overran by
exactly the 54 bytes of the two swallowed records.

LibreOffice does not have the problem because it turns continuation joining **off** for the whole
drawing block: `XclImpDrawing::ReadMsoDrawing` calls `rStrm.ResetRecord(false)` and then loops over
`MSODRAWING`/`MSODRAWINGSEL`/`CONT` → Escher, `OBJ` → `ReadObj8`, `TXO` → `ReadTxo`, re-enabling it
only at the end (`sc/source/filter/excel/xiescher.cxx`:4021-4052 **in this tree**, which declares
27.2.0.0.alpha0+ and is not the reference binary's source). `ReadTxo` then takes exactly one
`CONTINUE` for the characters and one for the formatting runs, each by name
(`:4242-4269`), and anything further goes back to the block loop as drawing bytes.

So `TXO` joins `OBJ` as a record the stream does not absorb continuations into, and
`XlsDrawingCollector.ReadText` asks for its two by name through the new
`BiffRecordReader.StartContinuation`. `TXO` is only ever read from inside that block —
`git grep EXC_ID_TXO sc/` finds one reader and it is `ReadMsoDrawing`'s — so the change needs no
version or context test.

**On this workbook that alone is worth no ink**: the recovered tail shape is the sheet's last
form-control button, and 26.2.4.2 does not print the buttons (`RESET SCORES` appears in the
reference's 24 pages exactly once, inside an instruction sentence). It is a correctness fix whose
value is that the drawing stream is no longer truncated.

### 1.2 A shape inside a group states no client anchor, and every one of them was dropped

`EHEST`'s three colour boxes are a **group**. A grouped shape carries `msofbtChildAnchor` — a
rectangle in the group's own coordinate space — and only the group's own shape carries the
`msofbtClientAnchor` that says where that space lands on the sheet. `Build` asked every shape for a
client anchor and `continue`d when there was none, so **no grouped shape has ever been drawn from a
`.xls`**.

The map is `SvxMSDffManager::ImportShape`'s
(`filter/source/msfilter/msdffimp.cxx`:4318-4340 **in this tree**):

    fXScale = clientRect.Width / globalChildRect.Width
    x       = (l - globalChildRect.Left) * fXScale + clientRect.Left

with `globalChildRect` the **union of the direct children's own child anchors**, which is what
`GetGlobalChildAnchor` computes at `:5029-5045` — not the group's `msofbtSpgr`. `Build` now keeps
the placed box of every shape, group shapes included, and lays a child inside its parent's by that
map; `WithinCells` does it for a sheet's two-cell anchor and `WithinBox` for a chart's absolute
one. The two-cell case turns both corners into distances from the sheet's origin before taking the
fraction and turns the result back into cells, because interpolating in column *indices* is wrong
on any sheet whose columns differ in width — the fixture's do, 1 cm and 6 cm alternating.

**A group's own shape is not drawn.** `ImportShape` branches on `ShapeFlag::Group` *before* the
branch that builds an item set, so a group object never reaches `ApplyAttributes` and its stated
fill and line reach nothing (`msdffimp.cxx`:4376-4386 in this tree). Measured at the binary:
`EHEST` page 8's group states a white fill and a black outline over 391 × 33 pt and 26.2.4.2 draws
**no rectangle there at all**, while this tree drew one at both r100 and the base of this round.

### 1.3 An embedded chart's substream carries a drawing of its own, and it was stepped over

Five of `EHEST`'s ten triples are at worksheet level; the other five are inside the **embedded
chart's substream**, and `ReadEmbeddedChart` took only the chart records and stepped over
everything else, by design and with a comment saying so. Calc does not: an embedded chart gets its
own `XclImpChartDrawing`, whose `ConvertObjects` puts the objects on the chart's model and whose
`CalcAnchorRect` reads the anchor's *cell* fields as quarter-thousandths of the chart's rectangle
(`sc/source/filter/excel/xichart.cxx`:4242-4290 in this tree). The substream's drawing now goes
into a collector of its own, is attached to the chart's `OBJ`, and is expanded against the
rectangle the chart shape was placed in — the same `within` map §1.2 uses, with the fractions
coming from `ChartTotalUnits` instead of from a child anchor.

It has to be a separate collector. Appending those bytes to the sheet's Escher stream puts a second
`DgContainer` inside the sheet's own `SpgrContainer`, which no walk of the sheet's drawing reaches,
and shifts the sheet's shape-to-`OBJ` pairing by however many objects the chart carries.

### 1.4 A shape's text margin, which is why the recovered boxes landed 1.6 pt out

With §1.2 and §1.3 in, all 33 spans were drawn and every one of them sat about 1.4 pt left of and
1.6 pt above the reference's. The same offset was already on every *ungrouped* BIFF shape in the
tree and nobody had measured it: `EHEST` page 1's `INSTRUCTIONS` box started at 87.11 against
26.2.4.2's 88.44.

It is the automatic text margin. A shape that sets `fAutoTextMargin` (Escher property 188, a
boolean of the text group and therefore bit 3 of property 191) states no lengths and the **host**
answers with a constant; Excel's is `EXC_OBJ_TEXT_MARGIN`, **20000 EMU = 1.5748 pt**, on all four
sides (`sc/source/filter/inc/xlescher.hxx`:140, applied by
`XclImpDrawObjBase::PreProcessSdrObject` at `sc/source/filter/excel/xiescher.cxx`:546-553 in this
tree). A shape that does not set it states `dxTextLeft` and its three siblings itself, in EMUs, and
26.2.4.2's own *MS Excel 97* filter writes all four as **zero**. This tree used a hard-coded tenth
of a millimetre either side and nothing top or bottom, from neither rule.

Both branches are now read. `INSTRUCTIONS` goes 87.11 → **88.40** against 88.44, and the 33 boxes
land within 0.13 pt in x (`ehest-spans.txt`).

*What a shape stating neither should get is not settled here.* The C++ carries two different
defaults — 91440/45720 EMU at `msdffimp.cxx`:5280-5283 and 0.25 cm/0.13 cm at `:1474-1477` — and
**489 of the 489 shape containers in the 64 `.xls` state all four** (`autotm.py`,
`shape-census.txt`), so no document here exercises the case. Such a shape keeps the tenth of a
millimetre it had.

### 1.5 The census the four rules reach

`autotm.py` walks the Escher stream of every one of the 64 `.xls` and counts, per shape container,
the auto-margin bit, the group flag and the child anchor (`shape-census.txt`):

| | |
|---|---:|
| `.xls` stating a drawing at all | **24 of 64** |
| shape containers in them | **489** |
| setting `fAutoTextMargin` | **299** |
| carrying `msofbtChildAnchor` — the shapes §1.2 recovers | **44**, in **2** documents (`apron-area` 31, `EHEST` 13) |
| carrying the group flag | 138, which counts each sheet's patriarch |

*The 13 for `EHEST` is a floor, not a count: the census reads only the `MSODRAWING` records and
therefore drops the same shape containers §1.1 is about.*

## 2. The witness

`EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls`, whole document, against the banked
26.2.4.2 reference (`ehest-ink.txt`, `ehest-spans.txt`):

| | pages | sum `\|ink\|%` | MAJOR | 13 pt colour-stating shape spans |
|---|---:|---:|---:|---:|
| base | 24 | **8.65** | 0 | **0** |
| head | 24 | **8.06** | 0 | **33** |
| 26.2.4.2 | 24 | — | — | **33** |

Every one of the 33 is on the reference's page, in the reference's colour, within **0.13 pt** in x
and **0.92 pt** in y. Eleven pages move and the four that improve most are 22 (**0.78 → 0.12**),
10 (0.82 → 0.59), 11 (0.23 → 0.12) and 1 (0.41 → 0.31).

**Three pages worsen and both reasons are defects the change uncovers rather than causes**:
pages 13 (+0.20) and 15 (+0.21) lose the white group rectangle §1.2 stopped drawing, and what was
underneath it is our own row-grid rules at 267.5 / 289.0 / 310.5 pt where 26.2.4.2 draws three grey
ones at 264.8 / 287.7 / 310.6 — a row-height difference the white panel had been hiding. Page 8
(+0.05) is the same thing plus the recovered text.

## 3. Corpus reach, and the one place the ranking column disagrees with a direct measurement

### 3.1 The sweep

`sweep.sh` renders every one of the 64 `.xls` at both binaries under `SOURCE_DATE_EPOCH`, one
temporary directory per render, checks each PDF for `%%EOF` before hashing it and deletes it again
(`xls-sweep.tsv`): **64 documents, 9 movers, 0 failures.**

### 3.2 Confinement

Only `XRay.Office.Spreadsheets/MsBinary` changed behaviour — `XlsDrawing.cs`,
`BiffRecordReader.cs`, `XlsWorkbookReader.cs` — plus one new `const` in
`XRay.Office.MsBinary/Escher/EscherRecordTypes.cs`, which adds no code path. Checked anyway over 40
`.xlsx`/`.xlsm`, 10 `.ppt` and 10 `.doc` at both binaries (`confine.tsv`, `confine.list`): **60
documents, 0 movers, 0 failures.**

### 3.3 The movers scored

`score.sh`, `movers.tsv`; each mover rendered again at both binaries and scored page by page
against its own banked reference:

| document | pages | base `\|ink\|%` | head | base MAJOR | head MAJOR |
|---|---:|---:|---:|---:|---:|
| `EHEST-Pre-departure-checklist` | 24 | 8.65 | **8.06** | 0 | 0 |
| `apron-area` | 3 | 0.88 | **1.51** | 1 | 1 |
| `2012-GA-Survey-Chapter-6-Tables` | 5 | 0.13 | **0.25** | 0 | 0 |
| `SIL_TDB605` | 44 | 0.94 | 0.96 | 1 | 1 |
| `TOGAF9-Tool-ConfReqts-CSQ` | 28 | 12.92 | 12.93 | 1 | 1 |
| `2012-GA-Survey-Chapter-5-Tables` | 3 | 0.31 | 0.30 | 0 | 0 |
| `SIL_TDB609` | 50 | 1.08 | 1.06 | 1 | 1 |
| `TICAPCapability_Final` | 17 | 4.93 | 4.91 | 2 | 2 |
| `PC1000` | 13 | 1.60 | 1.60 | 0 | 0 |
| **total** | **187** | **31.44** | **31.58** | **6** | **6** |

**N11: nine documents move a rendering, and three of them move by more than a hundredth.**

*The whole sweep and this table were run twice, because the first run's head binary was replaced
under it mid-sweep — the very trap `dotnet/CLAUDE.md` records. The second run reproduces the same
nine movers and every one of these eighteen ink figures to the hundredth, which is the control on
the instrument as well as on the accident.*

### 3.4 The ranking column is +0.14 and the direct measurements are unanimous

This is the uncomfortable part of the round and it is stated rather than dressed up. Summed
unsigned ink over the nine movers rises by 0.14. Every direct measurement of the same change says
it is right:

- **`EHEST`**: 0 of 33 spans drawn → **33 of 33**, at the reference's own positions. −0.59.
- **`2012-GA-Survey-Chapter-6`**: exactly one span moves between base and head — `Source: 2012 GA
  Survey Table 6.1`, from x **87.02 to 86.74**, and 26.2.4.2 draws it at **86.74**. Every other one
  of the document's 448 spans is unchanged. `|ink|%` **rises** 0.13 → 0.25. A 0.28 pt correction
  onto the reference's own x, scored as a regression by a metric that samples the page at 512
  pixels on its long edge.
- **`apron-area`**: +0.63, and it is the pattern fill. The 31 grouped shapes this recovers include
  four rectangles that 26.2.4.2's own `fods` gives `draw:fill="bitmap"` — a hatch — which
  `EscherInk` paints as its solid foreground grey, a shortcut its own comment already records
  ("*a gradient or a bitmap fill is painted here as its foreground colour*"). The page's largest
  region goes from *"marks displaced or reshaped, 7.40 % of page"* at the base — our text in the
  wrong place — to three fill regions; the text defect is gone and a fill defect is exposed. That
  is a new seat, §6.

So the honest statement is: **three documents' renderings are measurably closer to 26.2.4.2 and
one is measurably further, the further one for a reason that is not this change, and the column the
round is ranked on nets +0.14.** No page's verdict moves in either direction.

## 4. Tests

Ten non-fidelity projects, run one at a time and each figure read out of this round's own output:
Core **528**, Containers **109**, Markup **259**, Text **728**, OpenDocument **146**,
Presentations **1054**, Rendering **164**, Vector **309**, WordProcessing **1938**,
Spreadsheets **1297** — **6532 passed, 0 failed and 0 skipped in every one**. Solution build
0 warnings, 0 errors.

`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed of 552**, and the ten are exactly the known
names: `TabStopComparisonTests` ×4 (`list-label-overrun.fodt`/`.odt`/`.docx`/`.doc`),
`PageDrawingComparisonTests` ×4 (`paginated.fodt`/`.docx`/`.doc`/`.rtf`),
`SheetDrawingComparisonTests` (`sheet-rich-text.xlsx`) and `JustificationShrinkComparisonTests`
(`justify-shrink-2013.docx`).

Two tests in one new class, `SheetShapeGroupAnchorTests`, both failing at the base
(`dotnet test --filter SheetShapeGroupAnchorTests` on `e58567aea`: **2 failed, 0 passed**; after:
**2 passed**). Spreadsheets' 1297 is r100's 1295 plus these two.

Fixtures, all three in `tests/corpus/features`:

- `sheet-shape-group-anchor.fods` — authored here. Four columns alternating 1 cm and 6 cm and rows
  alternating 0.5 cm and 3 cm, so a child mapped by interpolating column *indices* lands nowhere
  near the answer; one ungrouped text box `ZLONE` as the control and a `draw:g` of `ZGROUPA` and
  `ZGROUPB`.
- `sheet-shape-group-anchor.xls` — that file put through 26.2.4.2's own *MS Excel 97* filter, so
  its Escher group, its child anchors and its stated text margins are the reference's writing.
- `sheet-shape-group-margin.xls` — the same file with **two bits** changed. LibreOffice's filter
  never writes `DFF_Prop_AutoTextMargin`, so no round-tripped fixture can carry it; it does write
  property 191, the boolean group the flag lives in, so `patch-automargin.py` sets bit 3 of the
  value half and bit 3 of the "was stated" half of a value that is already there. Six bytes are
  rewritten and no length anywhere changes.

Every expected number is the baseline origin 26.2.4.2 writes into its own PDF of the same file:

| span | reference | head | base |
|---|---|---|---|
| `ZGROUPA` | 84.95, 195.00 | 84.98, 195.00 | **not drawn** |
| `ZGROUPB` | 311.70, 251.77 | 311.58, 251.74 | **not drawn** |
| `ZLONE`, margins stated zero | 70.81, 132.58 | 70.78, 132.55 | 71.06, 132.55 |
| `ZLONE`, `fAutoTextMargin` set | 72.40, 134.16 | 72.36, 134.12 | 71.06, 132.55 |
| `ZCELL`, a cell and not a shape | 58.68, 123.45 | 58.68, 123.45 | 58.68, 123.45 |

`ZCELL` is the control that never goes through the shape path, and the second `ZLONE` row is the
control on the margin: only that one shape carries the flag and the other three must not move.

## 5. O17 — the eleven measured, and the algorithm written out

Not fixed. What this round adds to r100 is the reach figure the brief asked for and the algorithm
in a form the next round can implement from.

### 5.1 What the generated grid is worth, document by document

r100's decomposition was one document deep: strip every `fo:border*` from `alle einzeln`'s `.ods`
twin and it goes from 0.02 to 229.12. That works there because the workbook states one border and
it is empty, and it does not generalise — the other ten documents are full of ordinary borders.

`pivot-strip.py` removes `fo:border*` from **exactly** the automatic cell styles whose parent is a
`Pivot_20_Table_20_*` style, leaving every other border alone, and `pivot-reach.sh` renders the
result against the banked `.ods` reference (`pivot-reach.tsv`):

| document | pages | `.xlsx` `\|ink\|%` | its `.ods` twin | twin less the pivot's borders | **the grid is worth** | border attributes removed |
|---|---:|---:|---:|---:|---:|---:|
| `alle einzeln` | 186 | **225.44** | **0.02** | **234.22** | **+234.20** | 155 |
| `Keywords_Mapping_Graphs_and_Charts` | 46 | 10.79 | 6.97 | 13.68 | **+6.71** | 36 |
| `037_Personal_money_tracker` | 5 | 3.45 | 1.29 | 4.88 | **+3.59** | 136 |
| `DynamicBubbleChart` | 5 | 4.42 | 3.45 | 6.77 | **+3.32** | 41 |
| `007_Contextures_chart_sample` | 7 | 0.95 | 0.00 | 0.98 | **+0.98** | 84 |
| `049_Expenses_calculator` | 4 | 2.18 | 0.94 | 1.80 | **+0.86** | 104 |
| `033_Event_planning_tracker` | 3 | 12.65 | 1.87 | 2.42 | **+0.55** | 73 |
| `035_Project_plan_for_law_firms` | 4 | 1.75 | 1.43 | 1.96 | **+0.53** | 84 |
| `026_Monthly_cash_flow_statement` | 11 | 0.59 | 0.73 | 0.73 | 0 | **0** |
| `027_Simple_personal_cash_flow_statement` | 10 | 3.03 | 9.38 | 9.38 | 0 | **0** |
| `053_Personal_asset_inventory` | — | *unscoreable* | *unscoreable* | — | — | **0** |

**`alle einzeln` is worth 234.20 of it and the other seven are worth 16.54 between them.** That
sharpens r100's *"at most 36.33 between them"*, which was those documents' whole-document ink and
not the pivot's share of it. The three with no pivot-parented cell style — the same three r100
named — move by nothing at all, which is the control on the instrument: stripping a style that is
not there changes no pixel. `alle einzeln`'s 225.44 reproduces r100's figure exactly, which is the
control on the scoring.

*Two footnotes on the table.* `053_Personal_asset_inventory` cannot be scored on ink in either
spelling — this tree paginates it in **4** pages against the reference's **2**, so
`pdf-image-diff.py` refuses the comparison; r100's 0.00 for it was that refusal and not a
measurement. And `Keywords_Mapping_Graphs_and_Charts` has no `.ods` in the r80 bank, so its two
`.ods` figures are against a reference PDF rendered for this table by 26.2.4.2 rather than a banked
one.

The **stripped-minus-twin** column is what the pivot's generated grid is worth in that document,
measured on the spelling that has it. It is an upper bound on what implementing §5.2 recovers,
because the `.xlsx` differs from its twin in other ways too — visible in the rows where the
`.xlsx` column is *below* the sum of the twin and the grid.

### 5.2 The algorithm, to `file:line`

All of `sc/source/core/data/dpoutput.cxx`, **in this tree** (27.2.0.0.alpha0+, not the reference
binary's source); every line number is that file's.

1. **The frame is a colour and two widths.** `OutputBlockFrame` (`:217-258`) draws
   `SC_DP_FRAME_COLOR` — plain black, `:79` — at `SC_DP_FRAME_INNER_BOLD` (20 twips, `:76`) on any edge inside the table and at
   `SC_DP_FRAME_OUTER_BOLD` (40 twips, `:77`) on `mnTabStartCol`, `mnTabStartRow`, `mnTabEndCol` and `mnTabEndRow`.
   Its `bHori` argument, false everywhere but one call, decides whether the block's *inner*
   horizontal lines are drawn as well.
2. **A field name cell gets a hairline box of its own**, `lcl_SetFrame(..., 20)` at `:825` and
   `:998` — 20 twips, and nothing to do with the frame colour.
3. **The row headers decide where the blocks start.** `outputRowHeader` (`:1097-1152`) walks each
   row field's members; at a member that is not a subtotal and is not in the *last* row field it
   calls `AddRow(nRowPos)` and draws `OutputBlockFrame(nColPos, nRowPos, mnTabEndCol, nEndRowPos)`
   across the whole table and `OutputBlockFrame(nColPos, nRowPos, nColPos, nEndRowPos)` down its
   own column, where `nEndRowPos` is the last row the member's `CONTINUE` flags run to. The last
   row field draws no frame at all. A subtotal row calls `AddRow` and nothing else.
4. **The column headers do the same on the other axis**, `:1045-1071`.
5. **`OutputDataArea` (`:127-168`) then rules the data block** from the added rows and columns,
   with `mnTabEndCol+1` and `mnTabEndRow+1` appended and both lists sorted. When every data row was
   added (`bAllRows`) each column strip is one block with `bHori` **true**; when not, the loop at
   `:143-150` steps `i` by two starting at `nCol % 2`, which is where the checkerboard in a
   part-ruled pivot comes from. Then the row-header block and the column-header block are framed
   at `:159-168`.
6. **The four styles come with it.** `lcl_SetStyleById` (`:264-295`) creates each
   `Pivot Table *` style on demand: `Result` and `Title` bold, `Category` and `Title` left
   justified, and nothing else. The indent is separate — `ApplyAttr(..., ScIndentItem(nIndent))`
   at `:1140`, `13 px` per level converted to twips.

**And the ground truth for every cell of it is already written out.** The `.ods` twin of each of
these documents is 26.2.4.2's own conversion, so it carries the generated styles as ordinary
markup and can be read cell by cell rather than derived. `pivot-strip.py` is also the instrument
for checking an implementation: put the generated borders in and the `.xlsx` should score what its
twin scores.

## 6. What this round could not settle

- **O17 is not fixed**, §5. The algorithm is written out and the reach is measured; what remains is
  the implementation, and it is a subsystem rather than a hunk — it needs the pivot's member tree
  (which rows a row field's group covers) before it can place a single line. The member runs are
  derivable from the laid-out cells Excel already wrote into the sheet, which is what
  `XlsxPivotLabels.ReadKeys` does for the repeated-label rule, so the input is in reach.
- **A pattern fill is painted as its foreground colour**, §3.4. `EscherInk.Read` paints
  `mso_fillPattern`, `mso_fillTexture` and `mso_fillPicture` as `fillColor`; 26.2.4.2 builds the
  bitmap. It was worth almost nothing while grouped shapes were dropped and is worth **+0.63 on
  `apron-area`** now that they are not — four rectangles on its page 1. This is a new seat and it
  is the largest single thing this round's change exposes.
- **`EHEST`'s row-grid rules are about 2.5 pt out on some sheets**, §2. Pages 13 and 15 draw three
  horizontal rules at 267.5 / 289.0 / 310.5 pt against 26.2.4.2's 264.8 / 287.7 / 310.6 — a
  row-height question, not a drawing one, and it is what pages 13 and 15 regress by.
- **A shape stating neither `fAutoTextMargin` nor `dxTextLeft` keeps a tenth of a millimetre**,
  §1.4. 489 of 489 shape containers in the 64 `.xls` state the four lengths, so the corpus has no
  witness and the two candidate defaults in the C++ disagree with each other.
- **The ranking column and the direct measurements disagree by 0.14**, §3.4, and this round did not
  reconcile them. The two are measuring different things — `|ink|%` at 512 px on the long edge
  against a span's x to two decimals — and where they disagree here the span position is checked
  against 26.2.4.2 and the ink is not checked against anything.
- **`EHEST`'s form-control buttons are drawn by neither renderer** and this round did not establish
  why 26.2.4.2 does not print them. Our reason is `ftCmo`'s printable flag; whether that is the
  reference's is unmeasured.

## 7. Files

| file | what it is |
|---|---|
| `results.md` | this |
| `autotm.py`, `shape-census.txt` | every Escher shape container in the 64 `.xls`, with its auto-margin bit, group flag and child anchor |
| `escher-walk.py`, `ehest-escher.txt` | each sheet's Escher stream assembled the way the *base* reader assembled it, walked as a record tree — the 11-against-10 and the 54-byte overrun of §1 |
| `patch-automargin.py` | sets `fAutoTextMargin` on one shape of a workbook 26.2.4.2 wrote, by changing two bits of a property that is already there |
| `sweep.sh`, `xls-sweep.tsv` | the 64 `.xls` rendered at both binaries, `%%EOF`-checked and hashed |
| `confine.list`, `confine.tsv` | the same for 40 `.xlsx`/`.xlsm`, 10 `.ppt` and 10 `.doc` |
| `score.sh`, `movers.tsv` | the nine movers rendered again and scored on ink at both binaries |
| `ehest-ink.txt` | `EHEST` per page at both binaries against the banked reference |
| `ehest-spans.txt` | its 33 colour-stating shape spans, ours beside 26.2.4.2's |
| `pivot-strip.py` | removes `fo:border*` from only the pivot-parented cell styles of an `.ods` |
| `pivot-reach.sh`, `pivot.list`, `pivot-reach.tsv` | O17's reach: the eleven scored, and what the generated grid is worth on each twin |
| `tests-nonfidelity.log`, `tests-fidelity.log` | the two test runs §4 quotes, as they came out |
| `xls.list` | the 64 `.xls` |
