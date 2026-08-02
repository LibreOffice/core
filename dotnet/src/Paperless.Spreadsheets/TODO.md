# Paperless.Spreadsheets — TODO

The Calc equivalent: XLSX/XLSB/XLS/ODS/CSV and variants.

Two things make spreadsheets unlike the other families, and both shape the design:

1. **Content and formatting are stored independently.** Formatting is a run-length structure
   keyed by row, separate from the cells. Keep them apart — merging into per-cell objects is
   vastly more expensive on sheets with large uniformly-formatted regions, which is most of
   them.
2. **There is no intrinsic pagination.** Pages exist only once print settings, scaling and
   breaks are applied. Print setup *is* the page geometry.

Reference: `research/03-calc.md`.

**Done: ODS extraction** (`ods`/`ots`/`fods`), via `Paperless.OpenDocument`. Each sheet
becomes a section holding one table; cells carry both their typed value and the cached text
the authoring application displayed, plus their formula in the file's own syntax. Hidden
sheets are extracted and flagged rather than skipped. Repeated rows and columns are expanded
only within the used range — a spreadsheet pads every row to the sheet's full width and the
sheet to its full height, so expanding naively would materialise billions of cells.

**Done: XLSX extraction** (`xlsx`/`xlsm`/`xltx`/`xltm`), in `Ooxml/`. The same content tree,
so a caller indexing a mixed corpus never branches on which of the two it opened —
`XlsxReaderTests` asserts the same facts about the same workbook as `OdsReaderTests`, and
asserts that `sheet-xlsx.xlsx` and `sheet-ods.ods` extract to identical text.

The difference that shaped the work: **SpreadsheetML caches no display text.** ODF writes the
rendered string beside the value; SpreadsheetML writes the value alone, so `£4.50` and
`30 July 2026` exist nowhere in the file and only appear once `styles.xml` has been resolved
and the format code applied. Producing the same shape of tree therefore meant implementing the
formatter, which is `Numbers/` — the Excel format-code language, deliberately outside `Ooxml/`
because XLS's `FORMAT` records and XLSB carry the same codes. ODF is the exception: it writes
a *structured* `number:number-style` instead, so nothing in `Numbers/` applies to it.

Measured against LibreOffice 24.2.7.2's own all-sheets CSV export of
`features/sheet-ooxml-features.xlsx` (tab-delimited, sheet index `-1`): token similarity
0.98, and the **only** difference across four sheets is the cell comment, which that filter
drops and Paperless deliberately keeps. Every displayed value was also checked against the
text layer of the reference PDF. LibreOffice omits the hidden sheet from both, and Paperless
extracts it flagged — the second deliberate difference.

Traps that cost time, recorded so they are not rediscovered:

- **A date format's separators tokenise as arithmetic.** `M/D/YYYY` lexes its slashes as
  fraction bars and `m,d` its comma as a thousands-scaling comma, because those tokens are
  shared with numeric formats. Dropping them silently turns `7/30/2026` into `7302026`.
- **A conditional subformat is usually text-only.** `[>100]"big";"small"` has no digit
  placeholders at all, so a selector that skips text subformats — reasonable, since the
  fourth subformat *is* for text — skips both branches and always returns the first. Exclude
  by *position* (index 3), never by kind.
- **`r` is optional on both `<row>` and `<c>`.** Without it a cell follows the previous one,
  which is what LibreOffice does too (`sc/source/filter/oox/sheetdatacontext.cxx:347`).
- **Diagnostics accumulate after the copy.** A worksheet part that will not parse is only
  discovered while walking the sheets, so a reader that snapshots the file's diagnostic list
  up front loses every one of them.

**Settled decisions.**

- *Cached results are trusted; nothing is recalculated.* Extraction reads `<v>` and keeps
  `<f>` as written. This closes off translating formulas into a common grammar: doing so
  would misreport what the document says.
- *Shared formulas are reconstructed by shifting relative references* rather than left empty.
  Excel writes a filled-down column as one group where only the master carries any text, so
  leaving followers empty drops the formula from most cells in a real workbook. The rewrite
  refuses anything glued to a neighbouring identifier character or followed by `(`, so
  `LOG10(A1)`, `Tax_2020` and an `A1` inside a string literal are left alone.
- *A time-only format yields a `TimeSpan`, not a time-of-day `DateTime`* — matching the ODS
  path, because the file genuinely does not distinguish 14:30 from an elapsed 14½ hours.
- *`SheetCount` stays null for an XLSX.* LibreOffice's exporter writes no `Sheets` extended
  property, and counting the sheets would make "the file does not say" indistinguishable from
  a file that does.

Not yet, and why:

- **Built-in number-format ids 0–49 are locale-dependent, and the two readers pick different
  rows.** The XLSX path uses the `en_US` row of LibreOffice's per-locale table
  (`sc/source/filter/oox/numberformatsbuffer.cxx:436`, `en_US` at `:798`); the XLS path uses
  its `DONTKNOW` fallback (`sc/source/filter/excel/xlstyle.cxx:819`). LibreOffice picks by the
  workbook's locale and neither reader can, so a German workbook using bare id 14 extracts as
  `M/D/YYYY` from an XLSX and `DD/MM/YYYY` from an XLS rather than `DD.MM.YYYY` from either.
  It only bites files that use a built-in id *without* declaring the code, which LibreOffice
  never writes and Excel does constantly. Fixing it needs locale infrastructure Paperless does
  not have — and only then would the two tables be worth merging.
- **`General` is fifteen significant digits, not column-width dependent.** Calc's `General`
  picks between fixed and scientific by how many characters fit in the column
  (`research/03-calc.md` §A.7), and so does the `###` a too-narrow numeric cell shows. Both
  need a width, which extraction has no way to know; they belong to layout.
- **Dates before March 1900 follow Excel, and LibreOffice's own two filters disagree about
  them.** Excel treats 1900 as a leap year — a Lotus 1-2-3 compatibility bug — so its serial
  59 is 28 February 1900 and 60 is a 29 February that never existed. LibreOffice's *BIFF*
  filter reproduces that, adding a day to anything landing before 1900-03-01
  (`XclRoot::GetDateTimeFromDouble`, `sc/source/filter/excel/xlroot.cxx:351`), which puts
  serial 60 on 1 March sharing the day with 61. Its *OOXML* filter does not, and says so
  (`sc/source/filter/oox/workbooksettings.cxx:295`: "LO never treats 1900 as a leap year (so
  we never match Excel's first two months of 1900)"). `SpreadsheetDate` follows the BIFF rule
  for both readers, because the file was written under it and because one rule shared between
  the two readers is worth more than matching each LibreOffice filter separately. The
  consequence: dates in January and February 1900 agree with an XLS render and are one day
  later than an XLSX one. Nothing real is affected; recorded so it is not mistaken for a bug.
- **Pivot caches and defined names.** Still not reached. Drawings are, for layout
  (`Ooxml/XlsxDrawings.cs`, `OpenDocument/OdsDrawings.cs`), and charts are, for both content and
  layout — see the two chart sections below.
- **The sparse typed cell storage below**, which extraction does not need and layout will.

## Done: print setup and pagination

`Layout/`. A spreadsheet has no pages until a print setup is applied, so this is not "apply a
page size to a document" — the print setup **is** the page geometry, and everything about where
the pages fall follows from it. Measured against LibreOffice 24.2.7.2: the three
`sheet-print-*` corpus workbooks paginate to **14 pages each**, from all three formats, and
every one of those forty-two pages starts at the cell LibreOffice's own PDF starts it at.
`sheet-features.ods` gives 2, `sheet-ooxml-features.xlsx` 3 and `xls-features.xls` 4, which are
the counts `pdfinfo` reports for LibreOffice's renderings of the same files.

**Page count is the assertion worth having.** It is the one number that catches a wrong scale, a
missed print area, a break honoured that should have been suppressed and a repeated band not
subtracted from the page, and no tolerance can hide any of them. The second assertion, in
`SheetPaginationComparisonTests`, is which cells land on which page: every cell of the print-setup
workbook names its own coordinates, so the first word LibreOffice drew on a page states where that
page starts. That is what catches the error a count cannot — a page order read backwards produces
the right number of pages holding the wrong cells.

**Two routines are ported, because LibreOffice splits the work in two and the split matters.**
`ScTable::UpdatePageBreaks` (`sc/source/core/data/table5.cxx:57`) walks the columns and rows
accumulating sizes against the page and records a break wherever the running total would
overflow; `sc::PrintPageRanges::calculate` (`sc/source/ui/view/printfun.cxx:3082`) turns that set
of breaks into page ranges. The second pass is where hidden rows and columns collapse — several
breaks inside one hidden block count as one, because a page holding nothing visible must not
exist. `ScPrintFunc::CalcZoom` (`:2816`) sits on top and bisects on the integer percentage when
the sheet has to fit a page count; it converges in about seven repaginations, and it has to be a
search rather than a formula because halving the scale does not halve the page count — a column
either fits a page or starts a new one.

**The arithmetic is done in whole twips, deliberately.** Calc accumulates `sal_uInt16` twips
against a page size derived by integer division (`… * 100 / nZoom`, `GetDocPageSize`,
`printfun.cxx:2987`), so a page that fills exactly is decided by those roundings rather than by
the exact measure. Working in EMUs and converting at the end puts the boundary somewhere else on
any sheet whose columns happen to fill the width. The `- 1` and `+ 1` in the port are not noise
either: the page rectangle is a VCL `tools::Rectangle`, whose right and bottom edges are
inclusive.

**The margin model is Calc's, not any one file format's**, because the reference rendering is
Calc's. `TopMargin` is the distance to the top of the *header* when a header is switched on, and
`HeaderHeight` is the whole band down to the first row — so the first row sits at
`TopMargin + HeaderHeight`. ODF states exactly that. SpreadsheetML and BIFF both state a top
margin measured to the first row and a header margin measured to the header, and both readers
swap them round the way LibreOffice does (`sc/source/filter/oox/pagesettings.cxx:1001-1040`,
`sc/source/filter/excel/xipage.cxx:296-315`). The invariant that falls out is what makes those two
readers cheap: with or without a header, the first row still starts at the file's own top margin.

Traps that cost time, recorded so they are not rediscovered:

- **An ODF page layout that states nothing is not a page of nothing.** `sheet-features.ods`'s
  `style:page-layout` holds a writing mode and two header/footer bands: no page size, no margin.
  LibreOffice renders it on A4 with two-centimetre margins, because those are what
  `ScStyleSheet::GetItemSet` puts in a page style *before any file is read*
  (`sc/source/core/data/stlsheet.cxx:170-200`). Reading an absent `fo:margin-left` as zero widens
  the page by four centimetres and loses a break. The same routine is where the 0.5 cm + 0.25 cm
  default header band comes from.
- **`fitToWidth` and `fitToHeight` mean nothing without `pageSetUpPr/@fitToPage`.** Every workbook
  LibreOffice writes carries `fitToWidth="1" fitToHeight="1"` on `pageSetup` whether or not
  anything is fitting to anything, and the flag that switches them on lives in a different element
  — LibreOffice calls the separation out as odd itself
  (`sc/source/filter/oox/worksheetfragment.cxx:650`). Reading them without the flag turns every
  ordinary sheet into a one-page sheet. BIFF has the same trap with `SETUP` and `WSBOOL`.
- **A sheet of long strings is wider than its cells.** `xls-features.xls` came out three pages
  against LibreOffice's four for half an hour before the cause turned up: a string too wide for
  its column spills into the empty cells beside it, and Calc *widens the print area* to cover all
  of it before paginating (`ScTable::ExtendPrintArea`, `sc/source/core/data/table1.cxx:2127`).
  This is the single place pagination depends on measuring text, and it is survivable that the
  measurement is approximate: the extension is by whole columns, so being within one column's
  width of LibreOffice's answer gives the same page. Its 183-character strings in a 64-point
  column extend to column N either way.
- **The used area starts at A1, not at the first cell that holds something.**
  `AdjustPrintArea(true)` sets the start to column zero and row zero and searches only for the end
  (`printfun.cxx:700`). A sheet whose data begins at C3 still prints columns A and B, blank.
- **A column width is not a length in either Excel format.** SpreadsheetML states it in *digits*
  of the workbook's default font and BIFF in 256ths of a character, so neither is a measurement
  until a font has been measured. LibreOffice asks its reference device for the widest digit's
  advance in whole twips (`worksheethelper.cxx:1212`, `xltools.cxx:304`). 111 twips is what
  10-point Liberation Sans gives, and it checks out: `sheet-ooxml-features.xlsx` writes
  `width="20.76"` and LibreOffice's rendering puts the columns 115.2 points — 2304 twips — apart,
  which is 20.76 × 111 rounded.
- **A whole-column print range covers a million rows.** `A:D` paginated literally gives a
  four-column sheet twenty thousand blank pages. Calc cuts it back by re-searching the axis the
  range spans entirely, and only that axis (`AdjustPrintArea(false)`, `printfun.cxx:707`).
- **An empty visible sheet prints nothing at all**, not a blank page — measured on a two-sheet
  document whose second sheet holds one empty cell, which converts to a one-page PDF. Worth
  measuring rather than assuming, because `AdjustPrintArea`'s early return is guarded by the
  skip-empty option and reads as though it would produce a page without it.
- **`Print_Titles` holds both repeated bands in one name and distinguishes them by shape**, not by
  order: the column band is a whole-column reference with no row digits and the row band a
  whole-row reference with no column letters. In BIFF that is stored against sheet limits of 255
  columns and 65 535 rows, so the "spans the whole sheet" test has to be made against *those*
  limits and then widened, not against Calc's.
- **A blank page is not printed, and that is a rule rather than a nicety.** The printed block runs
  from A1 to the far corner of whatever the sheet reaches, so a sheet whose only content sits five
  hundred rows down paginates to ten sheets of paper of which nine are white. Calc drops those:
  `ScPrintPageRangesProvider` discards a whole band of rows when `ScDocument::IsPrintEmpty` holds
  across it, and `lcl_SetHidden` then hides the individual pages inside a band that survived
  (`printfun.cxx:3174, :3138`), both asking the same question of the same kind of block — so one
  test per page gives the same answer as their two passes. `Layout/SheetEmptyPages.cs` is that
  port, applied between `SheetPagination.Paginate` and the page list.
  **Three things keep a page and only one of them is cells** (`documen9.cxx:449-484`): a cell with
  something in it; a border anywhere in the block, because "we want to print sheets with borders
  even if there is no cell content"; and any drawing whose bounding rectangle *overlaps* the
  block, through `HasAnyDraw`, which walks the whole drawing page rather than the objects anchored
  inside it. Measured on `sc/qa/unit/data/xlsx/singlecontrol.xlsx`, a sheet with no cells at all
  and one form control anchored at row 516: **10 pages before this, 1 after, and LibreOffice
  prints 1.** No corpus row moved — every corpus spreadsheet is small and dense, so not one of
  them has a blank page to drop, which is exactly why this went unnoticed until a `sc/qa` sheet
  turned up with 516 empty rows.

## What the third sheets sweep found

Measured at `ef1aac0c8`. `sheets/batch-001` was **6/10**, the weakest level-one batch of the three
tracks, and its four failures all had the right page count and the wrong word count. Three of them
turned out to be **one defect**, and it was not the one the handover named.

**An accounting format's `*` fill was dropped, not deferred.** `_("$"* #,##0.00_)` is what Excel
writes for every accounting cell, and `*c` means "repeat `c` until the column is full" — which is
what puts the currency symbol against the cell's left edge and the digits against its right. The
parser threw the directive away with a comment saying it is column-width dependent and therefore
contributes nothing to extracted text. That is true of extracted text and false of a rendering: the
symbol came out jammed against the digits at the right-hand end, and `wc -w` scored the missing gap
as a missing word on every currency cell in the corpus.

LibreOffice keeps the two apart with a flag rather than by dropping it, and the mechanism is worth
reproducing exactly because it is what makes the two paths one implementation. `SvNumberformat`
writes `U+001B` followed by the fill character into the output string when `bStarFlag` is set
(`lcl_appendStarFillChar`, `svl/source/numbers/zformat.cxx:2200`) and emits nothing when it is not;
Calc's cell output finds the escape, records the position and the character, removes the pair, and
pads at that point once it knows the column width (`ScDrawStringsVars::RepeatToFill`,
`sc/source/ui/view/output2.cxx:572`). `NumberFormatter.FillMarker` and its `keepFillMarkers`
parameter are that flag; `SheetTextLayout.RepeatToFill` is that padding, including both truncations
Calc marks in its own comments — the character's width is measured from a twenty-copy sample rather
than one, and both it and the count round towards zero.

Three of the batch's four failures were nothing else:
`REDAC_SCHEDULE_RPD_135.xls` **178 words against 202**, which is exactly its 24 currency cells;
`RPD 155 REDAC SCHEDULE 2014-04-02.xls` 133 against 151, of which 16; and
`einvoice-summary-worksheet.xlsx` **203 against 298**, the largest single gap in the batch.

**The handover's named cause for the two `.xls` files was wrong, and its measurement was right.**
It attributed their shortfall to the missing BIFF drawing layer and called `MSODRAWING` + `OBJ`/`TXO`
"the next contained win". Neither file has a drawing at all: `pdftotext -layout` on the two
renderings side by side shows the reference writing `$   400` where we wrote `$400`, once per
currency cell, and the counts fall out exactly. The Escher item is still real — `apron-area.xls` and
its 137 words stand — but it was not this batch's defect.

**A cell whose output area misses the page was drawn anyway.** Calc's string loop starts one column
*before* the block it prints, so a long string reaching in from the left lands on the page its tail
falls on (`output2.cxx:1541`); the port had that half. The other half is `bOutside` (`:2037`),
which skips any cell whose area — its own column, widened through the empty cells beside it — does
not overlap the block at all. Without it every band's nearest-left neighbour was drawn whether or
not a stroke of it reached the paper. `ExampleWhiteListData.xlsx` put twenty part numbers off the
left edge of its last two pages, at `x = -2.6 pt`: **838 words against 821, now 821**. This is the
one document in the batch where we drew *more* than the reference, and over-drawing is the harder
direction to notice.

**A merged heading anchored in a hidden column vanished.** A hidden column is not placed on the
page, so the cell that anchors a merge starting inside one is never reached — and the whole block
goes with it, however many visible columns it covers. Calc reaches it from the other end: every
covered cell asks `ScOutputData::GetMergeOrigin` (`:953`) for the block's origin, and that walk
gives up the moment it steps onto a column that is *not* hidden (`if (!bDoMerge && !bHidden) return
false;`, `:993`), because that column is either the origin or a nearer covered cell and one of them
will draw it. So exactly one cell of a block ever draws it: the leftmost whose path back is entirely
hidden. `RPD 155 REDAC SCHEDULE 2014-04-02.xls`'s `Funds ($000)` is a four-column merge anchored in
a collapsed column, and it was the last two words of that document's deficit.

`sheets/batch-001` is **10/10** with these three, and every one of the ten passes the ink-imbalance
image diff.

**Two fixtures were added rather than one, because each of the last two rules has a negative half
that a single sheet cannot state.** `features/sheet-lead-in.fods` holds two rows differing in
nothing but the length of one string: the short one must not be drawn on the second page and the
long one must. `features/sheet-hidden-merge.fods` holds a three-column merge anchored in a collapsed
column beside an ordinary cell in that same column: the merge must be drawn and the ordinary cell
must not. Both were checked against LibreOffice's own PDF word by word and position by position
before being committed, and both are flat ODF so the fixture is readable.

Still open, and found while doing the above:

- **`_c` reserves one space rather than the character's width.** LibreOffice pads with
  `cCharWidths[c - 32]` blanks — a table of approximate widths in units of a space
  (`SvNumberformat::InsertBlanks`, `zformat.cxx:90`) — where the parser here always writes one. It
  happens to be right for `_(` and `_)`, which are what the accounting formats use and what every
  corpus file reaches, and wrong by one space for a digit or a capital. Porting the table would
  change extracted text as well as drawn text, so it wants its own measurement.
- **ODF states no fill directive at all.** `OdfNumberFormat` compiles a `number:*-style` tree into
  a format code and emits no `*`, so `SheetCellFormat.NumberFormat` is left null on the ODS path
  and an accounting-formatted ODS draws no fill. Nothing in the corpus reaches it.
- **XLSB likewise**, for the older reason: `Xlsb/XlsbStyles.cs` reads `styles.bin` only as far as
  the number format's *code*, and nothing there builds a `SheetCellFormat`.
- **A merge anchored in a hidden *row* is still lost.** `GetMergeOrigin` walks up as well as left
  and the port only walks left. No corpus document reaches it, and the shape of the fix is the same
  loop over rows.

## What the second sheets sweep found

Measured at `306f86e65` over `sheets/batch-001 … batch-018`, 171 documents: **82 matching, total
absolute page error 1120**, `xls` 34/62 and `xlsx` 48/109. The near-equal failure rates across the
two formats say the residue is below both readers, which is where both of this round's causes were.

**A repeated title row was keeping every blank page.** `SheetEmptyPages.IsBlank` returned "not
blank" for any page carrying a repeated band, on the reasoning that the band does print there. Calc
does not count it: `IsPrintEmpty` is asked for the page's own block alone —
`IsPrintEmpty(getStartColumn(), nPageStartRow, getEndColumn(), nRow-1, …)` and the same range
through `lcl_SetHidden` (`sc/source/ui/view/printfun.cxx:3174, :3053`) — and `PrintPage` adds the
repeated band afterwards, so it never enters the question. One `if` therefore disabled the whole
empty-page class for **every sheet declaring `_xlnm.Print_Titles`**. `fy20-may20-sep20.xlsx` repeats
row 1 and its column F reaches only row 76, so its second column band is two pages to Calc and was
103 blank ones here: **233 pages against 96, now 118**.

The residue on that document is the row heights, and it is the item already recorded above under
*the row height a rotated cell asks for*: its rows state `ht` without `customHeight`, so Excel's
cached heights are stale caches and Calc recomputes every one of them on load. Ours are 26.45 pt
where Calc makes 23.84, and the default row is 13.15 against 12.73 — about 11% too tall, hence 118
pages against 96. Nothing here is wrong about pagination; the measurement feeding it is Excel's
rather than Calc's, and closing it needs Calc's own coarse row-height measurement, which is the
same missing piece the rotated-cell section describes.

**A sheet's text boxes were read for their anchor and never drawn.** `XlsxDrawings` took `xdr:sp`
so the print area would be right and dropped its `xdr:txBody` on the floor, so a text box put
nothing on the paper — and a text box is the one thing on a sheet that no walk of the cells can
find, so no check built on the grid could see it either. Ten of the corpus's `xlsx` workbooks carry
shape text and 1083 words of it between them; on
`SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.xlsx`, whose entire methodology note is one text
box, that was **163 words against 550, now 386**. `Layout/SheetShapeText.cs` models it,
`Layout/SheetShapePainter.cs` draws it.

**What that shape text still gets wrong, and it is one thing.** The runs name `+mn-lt` — the
theme's minor Latin face, Calibri, so Carlito — and the sheet path can shape in one face only, so
they are set in Liberation Sans. Carlito is about 9% narrower at the same size, so our lines run
further right and more of them fall off the paper, which is the whole of the remaining 386-against-
550. That is the module's existing single-face limitation rather than a new one; it is the same gap
the cell engine has, and it will close with the same work. Two smaller ones beside it: `a:br` is
treated as a paragraph end, which gives the same lines; and `vertOverflow="clip"` is not honoured,
so a body too tall for its box is drawn over the cells below instead of being cut. Honouring the
clip **before** the face is fixed would make the corpus word count worse while making the rendering
better, which is the trap the corpus-batches skill names — so it is left until the face is right.

**The XLS half of the same defect is not fixed.** `apron-area.xls` has 137 words in eleven Escher
custom shapes and a deficit of exactly 137; the BIFF reader has no drawing layer at all
(`MsBinary/` holds no Escher reader, though `Paperless.MsBinary/Escher` exists and DOC and PPT use
it). Reaching it needs `MSODRAWING` walked in the sheet substream and the `OBJ`/`TXO` pair that
carries a text box's string. That is the next contained win on this track.

## What the first sheets sweep found

The first whole-track sweep of `sheets/batch-001 … batch-018` (174 documents, `xls` and `xlsx`)
put the paragraph above right on the mechanism and wrong on the scale. "No corpus row moved" was
true of the batches that existed then; measured across the whole track, **blank pages were the
single largest defect on the track** — one document produced 1170 pages of which 949 were blank
against LibreOffice's 220. The three causes, all now fixed, are worth stating because none of them
is where a page-count problem is usually looked for:

- **A wrapping cell was widening the print area.** `SheetTextOverflow` measured every text cell
  against its column and extended the printed block to cover the spill, without asking whether the
  cell wraps. `ScColumn::GetNeededSize` refuses in one line — `if ( bWidth && bBreak ) return 0;`
  (`sc/source/core/data/column2.cxx:226`) — and it is the whole difference on a sheet with a wide
  prose column, because those strings measure to thousands of points and every point of that
  became empty columns, and every band of empty columns became a band of blank pages.
  `AFS-400_Contacts.xlsx`: **340 pages against 48**, 289 of ours blank and none of LibreOffice's.
- **"Is there any cell to the left" was standing in for a measurement.** The recorded reason was
  that widening it means measuring and measuring is what the sweep is sensitive to. That was the
  right trade while the measurement above was wrong; with it fixed, Calc's own fourth test is
  portable as written — re-run `ExtendPrintArea` over the block's rows from column zero and keep
  the page when the extension reaches it (`documen9.cxx:486-500`). The conservative version keeps
  *every* column band of *every* row band that has anything in column A, which on a wide sheet is
  most of the paper: `RCO_VOR_Master_List_082824.xlsx` at **183 pages against 80**, 103 blank.
- **A background was being treated as a border.** `HasAttrFlags::Lines` tests the four edges of
  `ATTR_BORDER` and nothing else (`attarray.cxx:1279-1284`); a fill is not a border and does not
  keep a page. `grants-2005.xls`, whose shading reaches far past its cells: **1170 pages against
  220**, 949 blank.

And one that is not about blank pages at all:

- **A BIFF `SETUP` marked invalid loses its scale, not only its paper.** `EXC_SETUP_INVALID` is
  described everywhere as covering the paper size and the orientation, and LibreOffice reads it as
  covering the scale too: one assignment sets both flags (`xipage.cxx:68`) and `ATTR_PAGE_SCALE` is
  written only under `else if (maData.mbValid)` (`:274-276`). **Twenty-three of the corpus's 87
  `.xls` files set the bit**, and the scale sitting beside it is arbitrary — 255, 285, 300, once
  20480 — so honouring it multiplies the sheet by two or three in each direction. `P1636e.xls`
  states 285 and came out on twelve pages against LibreOffice's two.

Deliberate deviations from the port, both narrow:

- **A fit-to-pages search that bottoms out is clamped rather than reproduced.** LibreOffice leaves
  its loop with the page split from the *previous* scale and the zoom from this one
  (`printfun.cxx:2862`), which is self-inconsistent; the port clamps to `ZOOM_MIN` and re-splits.
  It shows only on a sheet with a column too wide to fit at any scale.
- **A page break inside a repeated band steps past the band.** Calc's own loop clears the breaks
  inside the band by pre-incrementing the loop variable, so those columns are never measured
  against the page either (`table5.cxx:180`). It reads like an off-by-one and it is what the
  reference renderer does, so it is reproduced rather than corrected. It only bites a repeated
  band that does not start at the print range's own first column.

Not yet, and why:

- **A header or footer taller than its declared height is under-measured.** Calc recomputes a
  dynamic band's height from the text in it and floors the result at the declared height
  (`UpdateHFHeight`, `printfun.cxx:846`). Every file LibreOffice writes declares 0.75 cm and puts
  one line of ten-point text in it, which measures well under that — so the declared value is the
  answer for all of them, and a header of several lines is the case this gets wrong. Fixing it
  needs the header's field language parsed and its text laid out, which is the same work as
  drawing it.
- **The paper size default is locale-dependent and A4 is assumed.** Calc's is
  `SvxPaperInfo::GetDefaultPaperSize()`, which is Letter in an American locale; the same missing
  locale infrastructure that keeps the two built-in number-format tables apart is what keeps this
  from being answered properly.
- ~~**`SkipEmpty` is not implemented.**~~ **This was wrong twice over and is now done.** The claim
  was that the option is off unless a caller passes it "and its PDF export does not". Both halves
  are false: `ScPrintOptions::SetDefaults` sets `bSkipEmpty = true`
  (`sc/source/core/tool/printopt.cxx:38`) and the render path reads the module's options straight
  out of it (`ScPrintUIOptions::ScPrintUIOptions`, `sc/source/ui/unoobj/docuno.cxx:266-268`), so
  **Calc drops empty pages on export by default**. `Layout/SheetEmptyPages.cs` implements it. The
  general lesson is the one the render-comparison skill states: the source says what mechanism
  exists, the binary says what it does, and the reference PDF settles it — `fy20-may20-sep20.xlsx`
  has a second column band whose content stops at row 76, and LibreOffice prints two pages of that
  band against a full 94.
- **The used area counts cells with content only.** Calc's own search also counts a cell carrying
  nothing but a style, because its attribute array knows about it. The content tree records no
  formatting, so a sheet whose last two columns are empty-but-shaded comes out narrower here.
  **This now costs pages as well as columns**, because the empty-page test above asks the same
  question: a page whose block holds only styled-but-empty cells is blank here and is not to Calc.
  Measured on `FAA-2019-0995-0002_attachment_2.xlsx`, whose `ADSB` sheet loses the last row band of
  each of its two column bands: 31 pages against 33, and it was a match before. That is the whole
  cost of the repeated-band fix below, against 115 pages saved on one document.
- **Multiple print ranges are paginated but not merged.** Calc paginates each in turn with a zoom
  of its own, which is what the port does; what it does not do is Calc's own oddity of hiding all
  breaks when a sheet has more than one range (`table5.cxx:97`).
- **Cell text layout.** Alignment, wrap, shrink-to-fit, rotation, indent, rich text inside a cell,
  the `###` a too-narrow numeric cell shows and the overflow of a long string *as drawn* are all
  still the separate item below. Pagination needed only as much of it as the print-area extension
  above, because a row's height and a column's width come from the file and never from the text.
  What `SheetPage.Draw` does today is one glyph run per cell at a baseline, in one face for the
  whole workbook, left for text and right for numbers — enough to see the pages, not enough to
  compare them against a rendering.

**Done: XLS (BIFF8 and BIFF5) extraction and CSV**, in `MsBinary/` and `Csv/`. The XLS reader
produces the same content-tree shape as the ODS one — a section per sheet holding one table,
hidden sheets flagged rather than skipped, cells carrying a typed value and the displayed
text — with one deliberate difference: `Formula` is null, because BIFF formulas are RPN token
arrays and the cached result is what a reference renderer shows.

**Done: number formats**, in `Numbers/`. Not optional for BIFF the way it is for ODF: a date
is a serial number and nothing caches the text, so without the format a workbook of dates
extracts as five-digit integers. One engine serves XLSX, XLS and XLSB alike, because the
`numFmt` codes and the `FORMAT` records are the same language.

### One number-format engine, not two

XLSX and XLS were implemented concurrently, and both discovered the same thing — neither
format caches the displayed text the way ODF does — so both wrote an Excel format-code engine
in `Numbers/`. The decomposed one (`NumberFormatCode` → `NumberFormatSection` → `FormatToken`,
rendered by `NumberFormatter`) survived; the self-contained 1040-line one was dropped. The
evidence, not the merge order:

- **Conditions.** `[>=100]#,##0;[RED]-#,##0` selects a subformat by comparing the value. The
  survivor evaluates them and falls back to the first unconditional subformat as the
  else-branch; the other parsed them only far enough to set `IsUnderstood = false` and hand
  the caller a number it could not justify. Built-in ids 5–8 and 41–44 are conditional-shaped
  accounting formats, so this is not an exotic path.
- **Fixed denominators.** `# ?/8` — eighths, the fraction format a price sheet uses — needs
  the literal `8` after the bar read as a denominator. The survivor does; the other required a
  digit placeholder on both sides of `/` and lexed `?/8` as a placeholder, a literal slash and
  a literal 8.
- **`?` versus `#`.** Both mean "no digit here", but `?` writes a space so a column lines up on
  its decimal point and `#` writes nothing. The survivor distinguishes them in both the
  integer and the fraction parts; the other only in the fraction.
- **Sub-second times.** `mm:ss.0` is built-in id 47, and only the survivor renders the digit
  after the point.

Structurally the decomposed design was the right one to keep for a second reason: the parse
and the render are separable, so `NumberFormatSection` can gain a token kind without the
renderer's 700 lines being in the same file as the lexer's. The self-contained one is easier
to read end to end, and that is a real cost being paid here — a reader who wants to know what
`# ??/??` does now opens three files.

**Three things the dropped engine did better, and all three are now in the survivor**, found
by re-measuring rather than by reading:

- **A half rounds away from zero.** 4.5 under `0` shows 5. LibreOffice's
  `rtl_math_RoundingMode_Corrected` is literally `approxFloor(magnitude + 0.5)`
  (`sal/rtl/math.cxx:483`); .NET's own `"F0"` is IEEE-correct and rounds a half to *even*, so
  it gave 4. Wrong one time in two on any column of prices shown without decimals, and not the
  kind of error a spot check finds.
- **A clock field truncates rather than rounds.** 05:35:31 under `hh:mm` reads 05:35.
  LibreOffice says so in as many words — "do not round values (specifically not up), but
  truncate to the next magnitude, so 23:59:59.99 is still 23:59:59 and not 24:00:00 (or even
  00:00:00 which Excel does)", `tools/source/datetime/ttime.cxx:217`. The survivor rounded to
  the minute, which is Excel's rule; since the comparison is against LibreOffice's render, and
  since the two disagree only in the last displayed digit of a value the file did not round
  either, LibreOffice's is now the one reproduced. This is the single difference the
  reconciliation caught by measuring: `formats.xls` Sheet3 holds 05:35:31.2 and showed 05:36.
- **The denominator of a fraction pads on the right.** `# ??/??` over 1.25 is `1  1/4 `, not
  `1  1/ 4`: LibreOffice passes `bInsertRightBlank` for the denominator alone and calls it
  "left alignment of denominator" (`svl/source/numbers/zformat.cxx`, `ImpNumberFill`). That is
  what lines a column of fractions up on its bars.

Its BIFF built-in table came across untouched as `Numbers/BuiltInNumberFormats.cs` (see
below), and none of its 46 number-format test cases was dropped for failing to compile. They
split along the same seam the code does: `NumberFormatterTests` asserts the text a code
produces and `NumberFormatCodeTests` what a parsed code *says* — whether a stored double is a
date, a duration or a number, and which built-in code an index stands for, which are the
decisions that change a cell's type before any text exists. Cases asserting a behaviour the
other file already covered through a different API were folded into it rather than kept twice;
the ones that reached somewhere new — the fractions, `dddd`, an empty middle section, an `mm`
that is a month and an `mm` that is a minute in one code — were rewritten and kept. The one
assertion that had to change meaning is `IsUnderstood` on a conditional code: it is now a test
that the condition *selects correctly*, plus a second on `IsFullyReproduced` for the
numeral-system directives that genuinely are not reproduced.

**Re-measured after the reconciliation**, against LibreOffice 24.2.7.2's *rendering* — not its
CSV export, for the reason recorded further down:

- `sc/qa/unit/data/xls/formats.xls`, LibreOffice's own number-format torture sheet, now comes
  out **identical on every row of every sheet bar one**: decimals, percent with a shorter
  negative section, currency, three-digit-exponent scientific, both fractions (`25 31/82` and
  `  7/18`) and the `hh:mm` time on Sheet3. The exception is the boolean row, which is the
  documented `TRUE`-versus-`1` difference. Before the clock fix it was two rows.
- `[h]:mm:ss` on 1.5208333 gives `36:30:00`, keeping the whole day; `[mm]:ss` gives `2190:00`;
  `h:mm:ss` on the same value gives `12:30:00`. Elapsed time survives intact.
- `features/sheet-ooxml-features.xlsx` still matches the reference PDF's text layer on every
  displayed value across all four sheets — `£4.50`, `£85.50`, `63.5%`, `#DIV/0!`, `1.23E+04`,
  `72.5 kg`, `-1,234.50`, `2 1/4`, `30 July 2026` — plus the hidden sheet and the comment,
  which LibreOffice's export drops and Paperless deliberately keeps.
- `features/xls-features.xls` likewise, with the same two documented exceptions: the boolean,
  and the `###` LibreOffice draws for a timestamp too wide for its column.

**Two built-in tables, deliberately.** `BuiltInNumberFormats` (BIFF) and
`XlsxStyles.BuiltinCode` (OOXML) look like duplication and are not: LibreOffice uses different
tables for the two filters. BIFF falls back to `spBuiltInFormats_DONTKNOW`
(`sc/source/filter/excel/xlstyle.cxx:819`), where id 14 is `DD/MM/YYYY` and id 37 is
`#,##0;-#,##0`; OOXML uses the per-locale table in
`sc/source/filter/oox/numberformatsbuffer.cxx:436`, whose `en_US` row makes id 14 `M/D/YYYY`
and id 37 the parenthesised `#,##0_);(#,##0)`. Merging them would change what one of the two
readers extracts. Both are locale-dependent in the same way and both are on the list below.

## Document model

- [ ] Sparse column-oriented cell storage; typed blocks rather than boxed per-cell objects
- [ ] Formatting as a separate run-length structure keyed by row, with pooled pattern
      objects
- [ ] Three-level attribute resolution: direct formatting → named cell style → default
- [x] Row heights and column widths as run-length segments; hidden rows/columns — `Layout/SheetGrid.cs`.
      Run-length in all three formats and kept that way: a `<col>` spans `min`..`max`, ODF repeats a
      column element, and a sheet has 1 048 576 rows almost all at the default height
- [ ] Merged ranges; cell borders (resolving the shared-edge conflicts between neighbours)
- [ ] Number formats, including custom codes and locale-dependent behaviour
- [ ] Conditional formatting; data validation
- [ ] Defined names; sheet-local and workbook-global scopes
- [ ] Comments/notes; drawing objects; charts
- [x] Print setup: print areas, repeated rows/columns, scale-to-pages, page order, headers
      and footers — `Layout/SheetPrintSetup.cs`, read by all three formats. Header and footer
      *text* is carried as written and not yet parsed or drawn

## Formula engine

- [ ] Tokeniser and compiler to an RPN token array
- [ ] Multiple grammars: ODFF, OOXML A1, Excel R1C1, and the native display form
- [ ] References: relative/absolute, 3-D, whole-row/column, external
- [ ] Interpreter as a stack machine, with matrix values and error propagation
- [ ] Error values: `#DIV/0!`, `#VALUE!`, `#REF!`, `#NAME?`, `#NUM!`, `#N/A`, `#NULL!`
- [ ] The function catalogue — start with the common few dozen, not all ~500
- [ ] Array formulas; implicit intersection
- [ ] **Recalculation policy.** Cached results are what a reference renderer displays, so
      default to trusting them (`LayoutOptions.RecalculateFormulas` opts out). Note that a
      one-shot extract/render tool needs no dependency-tracking broadcast machinery — a
      single topological recalculation suffices, so skip the complexity LibreOffice needs for
      interactive editing.

## Importers

### ODS — first
- [ ] `table:table`/`table-row`/`table-cell`; `office:value-type`
- [ ] `table:number-columns-repeated` / `number-rows-repeated` — the compression scheme;
      expanding it naively will blow up memory on wide sheets
- [ ] ODFF formulas with namespace prefixes

### XLSX
- [x] `workbook.xml`, `worksheets/*.xml`, `sharedStrings.xml`, `styles.xml` — all located by
      relationship, with the conventional name only as a fallback. A sheet's `r:id` is the
      only thing that says which part is which sheet; `sheet1.xml` being the first sheet is a
      coincidence any workbook with a deleted sheet breaks.
- [x] Cell types: `t="s"` (shared index), `inlineStr`, `str`, `b`, `e`, `d`, and the bare
      numeric default — six genuinely different meanings, and reading any of them as the
      default reads a string as zero
- [x] Formulas kept as written; shared-formula groups reconstructed; array masters keep theirs
- [x] Styles far enough for number formats: `cellXfs` → `numFmtId` → `numFmts` or the built-in
      table. Fonts, fills and borders are left for rendering, which is what discards them.
- [x] `mergeCells` → spans on the anchor, covered cells not invented
- [x] Legacy `comments` parts, resolved against the *worksheet* part, with their author list
- [x] Hidden and `veryHidden` sheets extracted and flagged
- [x] The 1904 epoch (`workbookPr/@date1904`), which shifts every date by 1462 days
- [ ] `theme1.xml` — needed for themed fills and fonts, i.e. for rendering, not extraction
- [ ] Styles: `fonts`, `fills`, `borders`, `dxfs`
- [ ] Threaded comments (`threadedComments`). Excel writes them *in addition* to the legacy
      part rather than instead of it, so reading the legacy one alone loses nothing today —
      but it loses the reply threading and the resolved-state flag.
- [x] Drawing anchors: `oneCellAnchor`, `twoCellAnchor`, `absoluteAnchor` — `Ooxml/XlsxDrawings.cs`,
      reached through the *worksheet's* own `drawing` relationship and never by part name.
      `editAs` is read past deliberately: it says how a drawing behaves when the sheet is
      **edited**, and the rectangle it occupies on a printed page is the same either way
- [ ] Tables, autofilters, pivot caches (extraction only)

### XLSB
- [x] BIFF12 records inside an OPC package, in `Xlsb/`. Same logical model as XLSX, binary
      encoding — and the split is worth stating precisely, because half of an XLSB is not binary
      at all. The **package** is OPC and identical: parts, content types, a workbook part naming
      every other part by relationship. The **spreadsheet** parts are BIFF12: `workbook.bin`,
      `sheet1.bin`, `sharedStrings.bin`, `styles.bin`. Everything DrawingML — the drawing part,
      the chart space, the theme, the images — is *the same XML an XLSX holds*, because DrawingML
      has no binary encoding, so `Ooxml/XlsxDrawings.cs` and `Ooxml/XlsxCharts.cs` serve both
      paths unchanged and only the part name comes out of BIFF12.
- [x] Cells: all three families — `CELL_*` naming its column, `MULTCELL_*` continuing from the
      previous one, `FORMULA_*` naming it and carrying a token array after the cached result. The
      cached result is read and the tokens are not decoded, so an XLSB cell carries a null
      `Formula`, like an XLS cell and unlike an XLSX one.
- [x] `SST` and rich strings, `NUMFMT`/`CELLXFS`, `MERGECELLS`, `SHEETFORMATPR`, `COL`, `ROW`,
      `PAGEMARGINS`, `PAGESETUP`, `PRINTOPTIONS`, `HEADERFOOTER`, `BRK`, the 1904 epoch,
      hidden sheets
- [ ] Cell fonts, fills and borders. `Xlsb/XlsbStyles.cs` reads `styles.bin` only as far as the
      number format each `CELLXFS` entry names; the fonts-and-alignment half that
      `Ooxml/XlsxCellFormats.cs` and `XlsxCellDecoration` take from `styles.xml` has no binary
      counterpart yet, so an XLSB draws in the default face with no fills and no borders.
- [ ] Comments (`comments1.bin`), which the XLSX path already reads from XML
- [x] **Import only** — LibreOffice cannot write XLSB, so test files must come from Excel. The
      ten in `sc/qa/unit/data/xlsb/` are the whole supply on this machine, and they are what the
      reader was measured against; the record-level cover is `XlsbReaderTests`, which assembles
      workbooks byte by byte, because a real file cannot say *which* record a regression broke.

**Measured, `paperless render` against `soffice --convert-to pdf`, page counts and `pdftotext`
word counts, over all ten files:** **eight match exactly.** The two that do not are understood:

| file | ours | LibreOffice |
| --- | --- | --- |
| `pivot-table/calcfields.xlsb` | 4 pages, 151 words | 2 pages, 108 words |
| `pivottable_error_item_filter.xlsb` | 1 page, 24 words | 1 page, 23 words |

`calcfields` is not a reader defect, and that is worth knowing before someone "fixes" it:
LibreOffice **rebuilds** a pivot table from its cache on import and writes its own result over the
cells, dropping the five calculated fields Excel had already written into them. We read the cells
as the file states them, which is what Excel shows; the extra pages follow from the extra columns.
The single remaining word is the same thing on a smaller table.

**Traps. Every one of these desynchronises the rest of the part rather than spoiling one field,
so the symptom appears a long way from the cause:**

- **A record identifier is one byte below 0x80 and two above it.** The identifier and the size
  share one variable-length encoding (`lclReadRecordHeader`,
  `oox/source/core/recordparser.cxx:255`), so a reader assuming a fixed width reads the second
  byte of the first wide identifier as a length and never finds a cell again. `SHEETFORMATPR` is
  0x01E5 and sits near the top of every worksheet part.
- **A string count of −1 means "no string", not "a string of length −1"**
  (`BiffHelper::readString`, `sc/source/filter/oox/biffhelper.cxx:86`). Read unsigned it asks for
  four billion characters.
- **`XF` is one identifier used inside both `CELLSTYLEXFS` and `CELLXFS`**, and only the container
  distinguishes them (`stylesfragment.cxx:302`). A flat walk shifts every cell format's index by
  however many named styles the workbook has — which reads as a number-format bug and is a
  parsing one.
- **The flag byte before a string is present for `SI` and `CELL_RSTRING` and absent for
  `CELL_STRING`**: `importCellString` passes `bRich = false` and `importCellRString` passes true
  (`sheetdatacontext.cxx:551, :574`). This cost time. One byte out of phase and every character
  after it comes back a CJK ideograph, because the halves of each UTF-16 unit swap — `cached`
  reads as `挀愀挀栀攀搀`, which looks like an encoding bug and is an offset bug.
- **A `BinRange` is the row pair before the column pair** (`addressconverter.cxx:59`), the reverse
  of how a range is spoken. Read in the spoken order it gives a plausible range that is wrong on
  every block that is not square.
- **Widths are 256ths of a digit and heights are twips**, where the XML states a fraction of a
  digit and points (`worksheetfragment.cxx:800, :827`). The XML's scales give columns 256 times
  too wide — one column to a page — and rows twenty times too tall.
- **`BIFF12_PAGESETUP_INVALID` is the flag that makes `paperSize` count.** `mbValidSettings` is
  its negation, and the paper size is applied only when `mbValidSettings` is *false*
  (`pagesettings.cxx:271, :935`) — so the flag whose name says the settings are invalid is the
  one that says to use them, and a sheet stating no `PAGESETUP` at all keeps the application's
  own paper because `mbValidSettings` is constructed true.

### XLS (BIFF8)
- [x] Substreams; the `BOF`/`EOF` structure. A sheet is found by the offset its `BOUNDSHEET`
      states, and that offset is wrong often enough that LibreOffice carries a fallback for it
      (`read.cxx:52-66`, i#115255): when it does not land on a `BOF`, scan forward until one
      does. Nested `BOF`/`EOF` pairs — an embedded chart inside a sheet — are counted, or the
      inner `EOF` ends the sheet three records in.
- [x] `SST`, `XF`, `FORMAT`, `DIMENSIONS`, `MERGEDCELLS`, `CODEPAGE`, `1904`
- [x] Cell records: `LABELSST`, `LABEL`, `NUMBER`, `RK`, `MULRK`, `BLANK`, `MULBLANK`,
      `FORMULA`, `STRING`, `BOOLERR`, `INTEGER`
- [x] BIFF5 as well. Measured against `sc/qa/unit/data/xls/shared-formula/biff5.xls`: all 376
      rows match LibreOffice's own CSV export cell for cell, Greek text included — that file
      declares code page 1253 and its strings are byte strings, so it exercises the
      `CODEPAGE` path that BIFF8 almost never needs. **It is not in the corpus**, because
      LibreOffice cannot write BIFF5: its filter is import-only
      (`filter/source/config/fragments/filters/MS_Excel_5_0_95.xcu:19`, `Flags: IMPORT ALIEN
      PREFERRED`), so a committed BIFF5 file would have to come from Excel 95 or be built by
      hand. BIFF2–BIFF4 are read on the BIFF5 path with a diagnostic; their record layouts
      differ enough that this recovers cells rather than reading the file properly.
- [ ] `FONT`, `ROW`, `COLINFO` — read past, not read. Nothing in the content tree records a
      font, a row height or a column width; rendering will need all three.
- [ ] BIFF formula token decoding. Out of scope for extraction on purpose: the cached result
      in the `FORMULA` record is what a reference renderer displays, so decoding the tokens
      would buy a `Formula` string and nothing else. Note the class-selection problem before
      starting — Excel encodes the reference, value and array forms of one operator as three
      different opcodes, and LibreOffice resolves them through the tables in
      `sc/source/filter/excel/xlformula.cxx`, which is most of the work.
- [ ] `ARRAY`, `SHRFMLA`, `TABLE`. A shared-formula cell's own `FORMULA` record still carries
      its cached result, so those cells extract correctly today; only the expression is
      missing, and it is missing for every formula anyway.
- [ ] RC4/XOR decryption. A `FILEPASS` record raises `PasswordRequiredException` rather than
      producing garbled cells. Both schemes are in `xistream.cxx:37-195`; the XOR one is
      twenty lines, the RC4 one needs the Office 97 key derivation shared with DOC and PPT,
      so it belongs in `Paperless.MsBinary` rather than here.
- [ ] `NOTE` (cell comments) and `HLINK`. A BIFF8 `NOTE` is an Escher object and the text
      lives in the drawing layer's `TXO` records, so it needs the MS-ODRAW reader that the
      PPT work owns — this is the one piece of cell extraction that is blocked on somebody
      else's module rather than on effort here.
- [x] Rich-text runs inside a cell, for **rendering** — `BiffRecordReader.ReadUnicodeString`
      returns them and `XlsWorkbookReader.BuildRichText` pairs them into portions. A BIFF run
      states a start and no length, so a portion reaches to the next run's start and the
      characters before the first keep the cell's own font. Extraction is unchanged: splitting a
      cell into several `ContentRun`s is a different question and the tree records no font

**Two differences from LibreOffice's rendering, both deliberate.** A boolean cell shows as
`TRUE` here and as `1` in Calc, which has no boolean cell type and imports the cell with the
General format; Excel, which wrote the file, shows `TRUE`, and so does the ODS reader, and
`Value` carries the boolean either way. And a cell too narrow for its text renders as `###` in
Calc; extraction reports the whole text, because `###` is a function of the column width and
there is no column width in extracted text.

### Number formats
- [x] The code language: sections, digit placeholders, date and time parts, quoted literals,
      `[$symbol-locale]` currency, `[h]` elapsed time, `E+00` scientific.
- [x] The built-in indices below 164, from LibreOffice's own `DONTKNOW` table
      (`xlstyle.cxx:820-905`). **Which table is a real ambiguity**: index 14 is `DD/MM/YYYY`
      there and `M/D/YYYY` in the US table, and LibreOffice picks by the *reading* machine's
      locale, not by anything in the file. Files LibreOffice writes sidestep it by emitting an
      explicit `FORMAT` record for every format they use.
- [x] Fractions, both `# ?/?` with a placeholder denominator and `# ?/8` with a fixed one. The
      placeholder form gives the denominator's *width*, not its value, so it is a
      continued-fraction expansion for the closest fraction with a bounded denominator rather
      than a digit walk. Measured against LibreOffice's rendering of
      `sc/qa/unit/data/xls/formats.xls`: 25.378 under `# ??/??` shows `25 31/82` in both, and
      0.389 shows `  7/18`.
- [x] Conditions (`[>100]`), which select a subformat by comparing the value, with the first
      unconditional subformat as the else-branch.
- [ ] Month and weekday names in a locale other than English. The name a spreadsheet shows
      depends on the reading application's locale, so there is no right answer available from
      the file alone, and the English names are what both readers emit.
- [ ] `[NatNum…]` and `[DBNum…]` numeral systems, and `[~calendar]` era substitution. These
      change the *characters*, so silently ignoring one produces plausible Western digits that
      are not what the cell shows. `IsFullyReproduced` reports false and the XLS reader raises
      `PL2324` rather than presenting a guess. (This is the narrowed remains of the dropped
      engine's `IsUnderstood`, which also covered conditions; those are reproduced now.)

`formats.xls` is worth knowing about: it is LibreOffice's own number-format torture sheet, and
every row of every sheet now comes out identical to LibreOffice's rendering character for
character — decimals, percent with a shorter negative section, currency, scientific with a
three-digit exponent, both fractions, and Sheet3's `hh:mm` time — except the boolean row, which
is the deliberate `TRUE`-versus-`1` difference recorded above.

**The trap that cost the most time here**: LibreOffice's CSV export is *not* a reference for
number formats. Exporting the feature workbook writes `4.5` where its own PDF rendering of the
same cell shows `£4.50` — and it does the same to `sheet-features.ods`, whose cached display
text says `£4.50` in the file. Percentages survive the round trip and currencies do not. Half
an hour went into looking for the bug in the XF resolution before rendering the file and
finding the reader was right all along. Compare against the PDF text layer for anything
number-formatted.

### CSV
- [x] Separator, quoting and encoding detection, in `Csv/`. Every decision is an Information
      diagnostic naming its evidence (`PL2340`–`PL2342`), because the TODO's warning is
      right — a mismatch here is usually a different reading of an ambiguous file rather than
      a defect, and that is only arguable when the reading is visible.
- [x] Separator chosen by *consistency* rather than frequency. Frequency loses badly on
      prose: a one-column file of English sentences holds more commas than a three-column
      semicolon file holds semicolons, while a real separator appears the same number of times
      in every line.
- [x] Encoding by byte-order mark, then a strict UTF-8 validation, then Windows-1252. The
      validation is what makes this reliable rather than a coin toss: Windows-1252 text is
      almost always invalid UTF-8 the moment it uses an accent.
- [ ] A decimal comma is not inferred. `4,50` in a semicolon file extracts as the text `4,50`
      rather than as 4.5, because deciding otherwise means guessing a locale from a file that
      states none — and the displayed text would be the same either way.
- [ ] Nothing is interpreted: `=B2*C2` extracts as six characters where Calc's import compiles
      it and shows the result. That is deliberate for extraction and wrong for rendering, so
      the CSV path will need an import-options record before it can be laid out.
- [ ] LibreOffice is not an oracle here at all: its CSV *import* uses the filter options its
      caller passed rather than detecting anything, so a headless conversion of
      `csv-semicolon.csv` reads it as one column. Comparing against it measures the options,
      not the file.

## Rendering

- [x] Cell text: fonts, alignment, wrap, shrink-to-fit, rotation, indent — `Layout/SheetTextLayout.cs`,
      with the formats read in `Ooxml/XlsxCellFormats.cs`, `MsBinary/XlsCellFormats.cs` and
      `OpenDocument/OdsCellFormats.cs`. See the section below
- [x] Overflow into adjacent empty cells; `###` when a number does not fit, and the
      `General`-shrunk form that is drawn *instead* of hashes when the format allows it
- [x] **Rich text within a cell** — the portions are `Layout/SheetRichText.cs`, read by
      `Ooxml/XlsxRichRuns.cs`, `OpenDocument/OdsCellFormats.cs` and `MsBinary/XlsWorkbookReader.cs`,
      and drawn one glyph run per portion by `Layout/SheetTextLayout.cs`. See the section below
- [x] **Cell backgrounds, cell borders, the printed grid and the row and column headings** —
      in `Layout/SheetPageDecoration.cs`, read into `Layout/SheetDecoration.cs` by
      `Ooxml/XlsxCellDecoration.cs`, `OpenDocument/OdsCellDecoration.cs` and
      `MsBinary/XlsCellDecoration.cs`
- [x] **Print pagination** — ported, in `Layout/SheetPagination.cs`
- [x] Repeated rows/columns; scale-to-pages; page order
- [x] **Header and footer text**: the field language is `Layout/SheetHeaderFooter.cs` and the
      placement is `SheetPageDecoration.DrawHeaderAndFooter`
- [x] **Pictures anchored to cells** — `Layout/SheetDrawings.cs` and `Layout/SheetPageGraphics.cs`,
      read by `Ooxml/XlsxDrawings.cs` and `OpenDocument/OdsDrawings.cs`. A chart is read into a
      `ChartPlot` and drawn by `Layout/SheetChart.cs`; see the chart section. BIFF is not read yet

### What the decoration path draws, and the rules it draws by

Measured against LibreOffice's own PDF of `sheet-decor-{ods,xlsx,xls}` with
`SheetDecorationComparisonTests`: four fills and twenty-eight lines on each, matching operator
for operator — same count, same order, same colours, same pen widths, positions within half a
point.

**Why a shared edge goes where it does.** When two neighbours both state a border on the edge
between them, Calc takes `std::max` of the two under `svx::frame::Style::operator<`
(`svx/source/dialog/framelinkarray.cxx:796-799`, `svx/source/dialog/framelink.cxx:306-334`).
The ordering is: wider wins; then double beats single; then the double with the *narrower* gap;
and only two single rules **one twip wide** are settled by their pattern, the lower
`SvxBorderLineStyle` enumerator winning. Colour is not in the comparison at all, so two
equal-width borders of different colours are *equal* and `std::max` returns its first argument —
which is always the cell being asked about. Both neighbours are consulted even at the page's own
edges, because `ScDocument::FillInfo` builds the array one column and one row wider than the
page on every side (`sc/source/core/data/fillinfo.cxx:1019`) and printing sets no clip range.

**Why it is not the Writer rule.** A Writer table consolidates its borders into one stroke per
grid line and overshoots each end by half its *own* width. Calc does neither.
`Array::CreateB2DPrimitiveRange` emits one primitive per cell's top and left edge, and the
bottom and right only for the last row and column
(`svx/source/dialog/framelinkarray.cxx:1490-1537`), so two adjacent cells agreeing about a
border produce two strokes. And an end is extended by half the width of the **perpendicular**
border it meets, or by nothing where it meets none — the simple case of `getExtends`
(`svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx:310`). Measured on
`sheet-decor-ods.ods`: the red 2.5 pt vertical in row 2 meets no horizontal and runs 12.784 pt
against a row 12.813 pt tall, while the blue one in row 3 ends on a one-point box and overshoots
it by 0.509 pt. The Writer rule would have made the red one 2.5 pt too long.

**The printed grid is black and sits at the far edge.** `ScPrintFunc::PrintPage` starts from
`Color aGridColor(COL_BLACK)` and only takes the screen's colours when the printout asks for
them, which a PDF export does not (`printfun.cxx:1662`, `:2340`) — every gridline in the
reference PDF is written under `0 0 0 RG`, not the pale grey the screen shows.
`ScOutputData::DrawGrid` advances the pen by a column's width *before* drawing
(`output.cxx:420-424`), so there is no line down the left of the first column and none along the
top of the first row; the block's own left and top edges come from the single rectangle Calc
draws round the whole printed area whenever *either* the grid or the headings print
(`printfun.cxx:2384`). Widths: LibreOffice writes them as a hairline, which its export emits as
`0.1 w`.

**Border widths are twips, and the names lie.** `hair` is 1 twip, `thin` 15, `medium` 35 and
`thick` 50 (`sc/source/filter/inc/xlconst.hxx:250-253`, and the same four numbers under
different names in `stylesbuffer.hxx:63-67`). So `thin` strokes at 0.75 pt and `hair` at a
twentieth of a point. ODF states its widths as lengths and they are snapped to whole twips,
because that is what `SvxBorderLine` stores: a 2.5 pt border round-trips through 1/100 mm and
comes back as `"2.49pt"`, which taken literally strokes 0.01 pt thin.

**A named trap, and it cost an hour.** Saving the corpus source as ODS, LibreOffice *moved* the
one blue cell's fill onto its whole column as `table:default-cell-style-name="ce12"` and then
cancelled it seven rows down with `table:style-name="Default"` on the single cell that must stay
white. A reader that treats "names the default style" as "names no style" paints a cell
LibreOffice leaves blank — so `SheetFormatting` keeps an explicit zero, meaning "this cell states
a style and it paints nothing", distinct from having no entry at all. The same shape appears in
SpreadsheetML as `s="0"` against a `<col style="…">` and in BIFF as a cell XF of 0 against a
`COLINFO`.

**Two reader fixes the measurements forced.** An ODF header band's declared height *already
includes* its gap to the body: Calc's `aHdr.nHeight` is `ATTR_PAGE_SIZE`'s height and
`aHdr.nDistance` is subtracted from it to get the rectangle the text is laid out in
(`lcl_FillHFParam`, `printfun.cxx:664`; `PrintHF`, `:1808`), so adding the gap double-counted it
by 0.25 cm — 7.09 pt — on every page of every file LibreOffice writes. And a BIFF header band has
a floor an OOXML one does not, because `XclImpPageSettings::Finalize` never puts an
`ATTR_PAGE_SIZE` on the header's item set (`sc/source/filter/excel/xipage.cxx:310-331`): Calc's
own 0.75 cm default stands as `nManHeight`, which on `sheet-decor-xls.xls` is 4 pt more than the
file's margins alone would give and matches LibreOffice's 21.11 pt to within 0.15.

### What is left, and why

- [ ] **A dynamic header band, measured the way Calc measures it.** `UpdateHFHeight` recomputes
      the band from the header's own text and takes the larger of that and the declared height
      (`printfun.cxx:846-856`). Reproducing it needs the header font's metrics inside the
      *reader*, which has none — the readers produce a `SheetPrintSetup` before any font is
      resolved. Measured cost: on `sheet-decor-xlsx.xlsx` LibreOffice's band is 18.13 pt against
      the 17.10 pt the file's margins give, because the OOXML filter's own text measurement and
      the EditEngine's disagree by that much; every line on the page therefore sits 1.03 pt
      lower than ours. `SheetDecorationComparisonTests` allows for it once per page rather than
      per stroke, so everything *within* a page is still compared to half a point
- [x] **1/100 mm quantisation** — reproduced rather than tolerated now, in
      `Layout/SheetDeviceUnits.cs`, which cell text brought and which the decoration path was
      routed through when the two halves were merged. It is worth less to decoration than to
      text, but it is worth something: see the reconciliation note at the end
- [ ] **Hatch patterns.** SpreadsheetML has eighteen and BIFF the same, each a foreground drawn
      over a background. One colour cannot stand for them, so a hatched cell is painted with its
      *background* colour, which is Calc's own fallback
      (`XclImpCellArea`, `sc/source/filter/excel/xistyle.cxx:1075`). Drawing the hatch needs a
      tiling paint the drawing IR does not have
- [ ] **Diagonal borders.** `ATTR_BORDER_TLBR` and `ATTR_BORDER_BLTR` are read by all three
      filters and by none of the three readers here. They are the one part of a cell's border
      set that is not an edge, so they need their own geometry rather than fitting into
      `SheetCellBorders`
- [ ] **Double rules** are modelled — `SheetBorder` carries a primary, a gap and a secondary,
      which is what `svx::frame::Style` carries and what the shared-edge ordering needs — but
      the widths of the three parts are guessed as thirds rather than derived the way
      `SvxBorderLine::GuessLinesWidths` does. Nothing in the corpus states one
- [ ] **Conditional formatting** changes a cell's fill and border and is read by none of the
      three readers. It has to be resolved per cell against the cell's *value*, so it cannot go
      in the run-length store as it stands
- [ ] **A multi-line header.** Every format can write a line break into one part of a band;
      only the first line is drawn, because laying out the rest means growing the band, and the
      band's height is what pagination already decided
- [ ] **`&D` and `&T` resolve against the clock**, so a document holding them cannot be compared
      against a reference rendering made at another moment. `SheetHeaderContext.Printed` exists
      so a caller can pin it; the corpus file deliberately uses `&A`, `&P`, `&N` and `&F` instead
- [ ] **A page's own background and border.** `ATTR_PAGE_BACKGROUND` and `ATTR_PAGE_BORDER` are
      drawn by `ScPrintFunc::DrawBorder` round the whole printed block (`printfun.cxx:2295`) and
      are read by none of the three readers
- [ ] **Headings are placed unscaled by pagination and scaled by drawing.** Calc does both:
      `nHeaderWidth = PRINT_HEADER_WIDTH * nScaleX` on the paper (`printfun.cxx:2205`) while
      `CalcPages` subtracts the unscaled constant in document twips. The two agree; it is worth
      knowing they are different numbers before changing either
- [ ] **A cell's overflow stops at a horizontal page break**, so the second page of a
      horizontally-split sheet draws nothing at all when everything on it is spill. Measured on
      `xls-features.xls`, whose `Strings` sheet is one column of 180-character strings over 48
      rows and which splits into two horizontal pages: page 3 is **1213 words in both**, and page
      4 is **3 in ours against 1011 in the reference** — the three being the `&A` header and the
      `&P` footer, so not one cell reaches it. It is not the reader (`paperless extract` returns
      all 48 rows) and not pagination (four A4 pages either way). It is that
      `SpreadsheetPages.DrawCell` is driven by the *placed columns of the page*: a cell in column
      A is drawn only on the page whose column band contains A, so the part of its string that
      spills into the next band is never drawn there. `SheetTextContext` already measures the
      spill against the document grid rather than against the page — see the remark on it, which
      exists for exactly this — which is why page 3's clipped string is right. The missing half is
      that Calc *also* draws, on each page, the cells left of the band whose text reaches into it:
      `ScOutputData::LayoutStringsImpl` walks back over the left neighbours before it decides an
      output area (`sc/source/ui/view/output2.cxx:1595-2290`). The fix is a per-page lead-in of
      the columns left of the band, drawn for their overflow alone. **Not chart-related**: that
      sheet holds no drawing at all
- [ ] **Two smaller word-count differences from the same whole-corpus sweep**, neither a cascade:
      `sheet-features.ods` renders **46 words against the reference's 45** — one *more*, the
      direction that usually means a cell the reference suppresses rather than one we invent — and
      `sheet-rich-text.xlsx` and `.xls` render **47 and 46 against 49**. The `.ods` of that same
      rich-text document matches exactly, so those two are in the importers rather than in the
      layout all three share

## Done: cell text

`Layout/SheetTextLayout.cs`, a port of `ScOutputData::LayoutStringsImpl`
(`sc/source/ui/view/output2.cxx:1595-2290`), with the formats read per format family in
`Ooxml/XlsxCellFormats.cs`, `MsBinary/XlsCellFormats.cs` and `OpenDocument/OdsCellFormats.cs` and
pooled per sheet by `Layout/SheetCellFormats.cs`.

**What is measured.** `sheet-print-xlsx.xlsx` and `sheet-print-ods.ods` now agree with
LibreOffice 24.2.7.2's own PDF on **all 2 281 positioned text runs across fourteen pages**, within
**0.006 pt** across and **0.024 pt** down — every run, not a sample, compared operator for operator
out of the two content streams. The new `sheet-cell-text.{fods,xlsx}` agrees on all 24 runs of its
alignment sheet, including the glyph counts, the em sizes and the fill colours, within 0.075 pt.
Page counts are unchanged: `SheetPaginationComparisonTests` still passes on all six workbooks. The
suite is 1 808 passing, 0 failed, 0 skipped — 29 more than before, in
`SheetTextComparisonTests` (twelve, against `soffice`) and `SheetCellTextTests` (seventeen, which
need no LibreOffice).

**Two roundings, not one, and only one of them matters.** Calc stores geometry in twips and draws
through a device whose unit is a hundredth of a millimetre, so every length crosses a lossy
conversion — and which way it loses differs by what is being converted. A **position or a size
truncates**: a 12.8 pt row is 451 hundredths, not 451.6, so it draws 12.7843 pt tall. A **font
height rounds**: ten point is 353 hundredths, so text is emitted at 10.0063 pt. The font size is a
fixed six thousandths of a point and would never matter; the row height is a sixty-fourth of a
point *per row*, so by the sixty-seventh row of a page the baseline is 0.86 pt out — eight times
the bound this project holds itself to, and it reads as a wrong row height rather than as rounding.
Both live in `Layout/SheetDeviceUnits.cs`.

**And the quantisation is two steps, not one.** A file may state a length in any unit, and Calc's
own storage is twips, so a length is quantised twice on its way to the page. An ODF row of
`0.178in` is 452.1 hundredths directly, 256 twips once Calc has it, and 451 hundredths when drawn.
Snapping straight to the device unit gives 452 and `sheet-print-ods.ods` drifts 1.5 pt down an
eighty-row page; going through twips first makes it exact.

**Snap before scaling, not after.** Calc hands its device coordinates *unscaled* and lets the map
mode's fraction apply the print zoom, so a 72 pt column at 66% comes out at exactly 47.52 pt.
Snapping the scaled value instead gives 47.5087 — 0.2 pt of accumulated error across the scaled
sheet of `sheet-print-xlsx.xlsx`, and the last of the differences to be found.

Rules ported, each with the citation that states it:

- **The general alignment is the cell's *type*, not a constant.** `getAlignmentFromContext`
  (`output2.cxx:1443`): a value goes right and everything else left. This is the rule that makes a
  spreadsheet's alignment enumeration different from a word processor's, where the default is a
  constant and can be resolved when the style is read.
- **Vertical `Standard` is bottom**, settled in one line before any drawing happens
  (`output2.cxx:348`). The baseline is then `rowBottom − bottomMargin − descent`, and the text
  height is `ascent + descent` from the font metric.
- **A cell's line height is not the word processor's.** Calc builds it from the metric alone, with
  no external leading (`aTextSize.setHeight(aMetric.GetAscent() + aMetric.GetDescent())`,
  `output2.cxx:734`), where Writer adds the line gap. Ten-point Liberation Sans wraps at a pitch of
  11.17 pt in a cell and 11.50 pt in a paragraph.
- **All four margins are 20 twips** (`SvxMarginItem(20, 20, 20, 20)`, `svx/source/items/algitem.cxx:123`),
  and all four are measurable: text starts 0.9921 pt inside its column and its baseline sits that
  far above the row's bottom.
- **Overflow is asymmetric.** A left-aligned string spills right into cells that are *visually*
  empty and stops at the first that is not; a right-aligned one spills left; a centred one both
  ways; and a **value never spills at all** — it shows `###` instead (`GetOutputArea`,
  `output2.cxx:1204`). Wrapping, filling and shrinking all suppress it too, because Calc passes
  `bCellIsValue || bRepeat || bShrink` for the same parameter.
- **A clipped string is shortened before it is drawn**, by the ratio of visible width to total
  width (`output2.cxx:2202`). LibreOffice does it for speed; it is reproduced because it is visible
  in the output — the blocked cell of `sheet-cell-text` holds 31 characters and both PDFs draw 23.
- **Shrink-to-fit is a measure-and-retry, and the first guess is an integer division.** The scale
  is `available × 100 / textWidth` truncated, then cut by a tenth up to seven times
  (`output2.cxx:1864`). The truncation is what makes it reproducible: the corpus cell comes out at
  87% of ten point in both renderers, which is 8.70 pt rather than the 8.74 an exact proportion
  gives.
- **An indent counts only when the cell states left or right alignment outright.** Calc reads
  `ATTR_INDENT` in that case alone (`output2.cxx:445`), so a General-aligned cell carrying an
  indent draws without one. It looks like a port bug until the reference renderer is measured.
- **Cell text is not kerned.** "There is no cell attribute for kerning, default is kerning OFF"
  (`output2.cxx:405-409`). HarfBuzz kerns `1.2E+11` by 152 design units, which is 0.74 pt of
  right-aligned cell — a difference no metric or margin would explain.

The trap that cost the most time, recorded so it is not rediscovered:

- **`###` is not what a too-narrow number usually shows.** `SetTextToWidthOrHash`
  (`output2.cxx:610`) hashes a numeric cell only when its format is *not* `General`. A `General`
  cell is re-rendered with as many characters as the column has **widest-digit widths** — not a
  measurement of the text, a count of digit widths — and falls back to scientific notation from
  there. So 123 456 789 012 in a 43 pt column draws as `1.2E+11` in Calc and not as `###`, and a
  port that hashes every value that does not fit produces entirely plausible output that disagrees
  with the reference on every wide number. The width-dependent `General` rendering is
  `Layout/SheetGeneralWidth.cs`, a port of `SvNumberformat::GetOutputString(double, nCharCount, …)`
  (`svl/source/numbers/zformat.cxx:2429`). **It is rendering-only**: `paperless extract` still
  reports the cell's full text, which is the recorded decision — hashes are a function of a column
  width, and extracted text has no column width.

**A defect found by rendering rather than by testing, and it was ours.** The sheet path built its
`FontReference` from the face's own family name instead of from the resolver's key. The key a
`SystemFontResolver` produces is the font *file's* path, and the PDF backend loads and embeds the
face from it — so a hand-built reference embedded nothing, left the backend with no `/Widths` to
advance the pen with, and made it correct every glyph with an explicit `TJ` adjustment of about
-700 thousandths of an em. The output looked right and extracted as loose characters: `pdftotext`
reads an adjustment that large as a word break, so the fourteen-page print workbook came out as
13 255 one-character "words" against LibreOffice's 2 281 real ones. It is a searchability bug
rather than a rendering one, which is why nothing that looked at pixels or positions would ever
have caught it, and "text stays glyph runs so PDF output can be real searchable text" is the stated
reason the drawing IR is shaped the way it is. Both sides now give 2 281.

**And the other half of that defect, which the widths fix hid for a while.** Recovering the
`/Widths` from the run made the spacing right and the extraction right, and it did nothing at all
about the embedding: a face whose file never loaded is still a face the PDF has no bytes for.
`pdffonts` on `sheet-features.ods` reported the two cell faces `emb yes` and the header's third
face `emb no`, in a file whose every word extracted correctly — which is exactly why the
page-and-word sweep could not see it and neither could any operator comparison, since a reference
and an embedding are not a pen position.

The furniture path is where it lived. `SheetFace` had carried the resolver's own reference from
the beginning, so **cell text was never affected**; `SheetBandText` resolved its face and then
rebuilt the reference with `FaceKey = face.FamilyName`, which is a family name where
`FileFontProvider` expects a path. That helper draws every header, every footer and — through
`SheetChart` — **every chart label**, so one unembedded reference covered the furniture of every
sheet in the corpus and the labels of every spreadsheet chart.

Why it could not be built from what the helper had: an `OpenTypeFace` is a parsed table directory
and does not know the file it came out of. `Load()` now returns the face and the reference it
resolved through as one `Lazy<(OpenTypeFace?, FontReference?)>`, which is the same shape
`SheetFace` uses and for the same reason. Eight spreadsheet files went from one unembedded face
each to none, with no page count and no word count moving.
`tests/Paperless.Rendering.Tests/PdfFontEmbeddingTests.cs` holds it, over `sheet-features.ods`,
`sheet-ooxml-features.xlsx`, `xls-features.xls` and `chart-bar-sheet.xlsx`.

**Deliberate deviations, all narrow.**

- **A rotated cell is drawn but not compared.** Calc turns the text about the cell's bottom-left
  corner and writes it with a PDF text matrix; Paperless writes a transform around an ordinary pen,
  which `PdfTextRuns` cannot read. So the corpus document keeps its turned cells on a second sheet
  and the reading of the angle is asserted without rendering. Stacked text (Excel's rotation 255,
  ODF's `style:direction="ttb"`) draws one character per line, centred.
- **A rotated cell does not make its row taller**, and rotated text is not clipped against its
  neighbours. The row-height half was implemented, measured and backed out; the measurement is
  below under "the row height a rotated cell asks for". The clip is Calc's own — its PDF carries a
  `re W* n` round the printed block before the turned glyphs — but it changes no glyph in the
  content stream, only which of them are visible, so nothing any comparison here reads would move.
- **Justified and distributed alignment place from the left and are not stretched.** They force
  wrapping, which is the part that changes where the lines fall; the stretch would need the
  space-distribution the word processor's justification already does, through a `MeasuredParagraph`
  the cell path does not build.
- **`Fill` does not repeat its text.** `RepeatToFill` (`output2.cxx:573`) pads the string to the
  column with copies of itself, sized from a twenty-character sample. Nothing in the corpus uses it.
- **The right-to-left branch of the general alignment is not reproduced.** It turns the rule round
  when the text begins with a right-to-left character, and needs the cell's writing direction,
  which no reader carries yet.

Not yet, and why:

- **A wrapping cell does not make its row taller.** Calc's rows are as tall as the file says unless
  the row asks for an optimal height, in which case Calc recomputes it from the text
  (`ScColumn::GetNeededSize`). Every file LibreOffice writes stores the computed height, so this
  bites only a file whose stored height disagrees with its content — and fixing it means moving row
  height out of the grid and into layout, which would make pagination depend on text measurement in
  a second place.
- **The clip mark is not drawn.** Calc puts a small triangle at the edge of a cell whose text is cut
  off (`SetClipMarks`, `output2.cxx:3371`) and reserves room for it in the clip rectangle. It is a
  screen decoration that its PDF export does not emit, so reproducing it would be a difference.

Two measured differences that are **LibreOffice reading the file**, not Paperless drawing it, and
both are recorded here because they look like rendering bugs:

- **LibreOffice's own BIFF import gives a workbook a different geometry from its ODF and
  SpreadsheetML forms.** Converting `sheet-cell-text.fods` to all three and rendering each with
  `soffice`: the ODS and XLSX put the first column's text at 57.685 pt and the XLS at 58.677 pt — a
  page margin 21 twips further right — and the XLS's first four columns span 230.17 pt against
  230.40. Paperless agrees with the ODS and XLSX in both. It has a knock-on worth naming: a
  shrink-to-fit scale is an integer percentage of the available width, so LibreOffice shrinks the
  XLS's cell to 85% where it shrinks the other two to 87%, and clips one character more. Page
  counts are unaffected, which is why `SheetPaginationComparisonTests` never saw it.
- **A header band taller than its declared height shifts every row.** Already on the list above;
  the measurement is 1.3 pt on `sheet-cell-text.xlsx`, whose header margin and top margin differ by
  19.1 pt while Calc computes 20.4 pt from the twelve-point serif line it puts there. The corpus
  document switches its header and footer off so that the cell-text comparison is about cell text.

## Reconciling the two halves: what it cost, and what had to give

Cell text and cell decoration were built side by side on the same base, and they collided in nine
files. Most of it was addition — each half had grown the same three readers to reach a different
part of the same record — but four decisions were not, and they are recorded here because each
changes behaviour rather than just moving code.

**One BIFF walk, not two.** Both halves extended `ReadXf`, and neither was a superset: text wanted
the font index and the alignment word, decoration wanted the two border dwords and the fill word
that follow them in the *same* record. They are now read in one pass, in stream order, which is the
only way they can be read at all — the fields are sequential, so a second walk would have to reparse
every record to reach byte 10. `PALETTE` and `ROW`'s trailing `ixfe` are likewise read once and
handed to both tables. The `ixfe` mask is decoration's: the low twelve bits are the XF index and the
top four are flags, and text was reading all sixteen — harmless on every file in the corpus, wrong
on any row above XF 4095.

**One `styles.xml` parse.** Decoration was loading the part as `StylesRoot` and text as
`StyleSheet`; they were the same single load under two names, so the property is now `StyleSheet`
and `XlsxCellDecoration` reads the `fills` and `borders` off it while `XlsxCellFormats` reads the
`fonts` and the `xf` alignment. Nothing was given up; the name went to text's because
`SheetCellFormats`/`XlsxCellFormats` already spell it that way.

**A name had to move.** Both halves defined `SheetCellFormat` in `Paperless.Spreadsheets.Layout` —
decoration's was a fill and four borders, text's is a font, an alignment and a number format. The
decoration one is now `SheetCellDecoration`, which is what its container `SheetFormatting` and its
three readers were already called; the text one keeps the name because it is the one a caller sees,
through `SheetLayout.Formats`.

**The device unit, and it had to be measured.** Cell text brought `Layout/SheetDeviceUnits.cs` with
a warning that gridlines and borders must go through it or they will not land on the edges the text
aligns to. Decoration predates it and drew from raw EMUs. Both now share one `Columns()`/`Rows()`,
so the question was only which rounding that shared geometry should use — and the two halves
disagreed about what LibreOffice does. Text measured 451 hundredths for a 0.178 in row on
`sheet-print-ods.ods`; decoration measured 452 for the same row height on `sheet-decor-ods.ods`,
because Calc's `SetSnapPixel` path rounds a printed *background* differently from the way
`GetScaledRowHeight` truncates a printed *string*. So there is no rounding that is right for both,
and the choice was made on the worst error each gives, comparing every rectangle and every line
against LibreOffice's own PDF:

| file | snapped (kept) | unsnapped (decoration's own) |
| --- | --- | --- |
| `sheet-decor-ods.ods` | **0.114 pt** | 0.381 pt |
| `sheet-decor-xls.xls` | **0.114 pt** | 0.254 pt |
| `sheet-decor-xlsx.xlsx` | 1.173 pt | 1.143 pt |

Snapping wins on two of the three and loses 0.03 pt on the third, where the number is the known
header-band offset (1.03 pt) rather than geometry — sideways, not backwards. Against that,
*not* snapping costs cell text 1.5 pt down an eighty-row page. So the shared geometry snaps, and
decoration inherits it: the residual 0.114 pt on a three-column sheet is four hundredths of a
millimetre of disagreement about how LibreOffice rounds its own backgrounds, which is a quarter of
the half-point the comparison tests hold to.

The heading strip goes through the same rounding for the same reason — it is a centimetre, which is
1000 hundredths through whole twips and 1000.06 direct — and it is snapped *before* the print zoom
multiplies it, because Calc computes `PRINT_HEADER_WIDTH * nScaleX` in unscaled hundredths
(`printfun.cxx:2204`) and lets the map mode's fraction do the scaling afterwards. The page margin
is snapped and not scaled, which is the same rule seen from the other end: `aPageRect` divides the
margin by the zoom before the device multiplies it back (`printfun.cxx:2104`).

**What it cost, end to end.** The suite is 1 865 passing, 0 failed, 0 skipped — mainline's 1 836
plus cell text's 29, with nothing dropped from either half. `sheet-print-xlsx.xlsx` still renders to
14 pages against LibreOffice's 14, and `pdftotext` still reads 2 281 words from both, which is the
check that catches a PDF whose glyphs land correctly but whose text will not come back out. (The
character count recorded here as 13 269 measures 13 255 on both sides today; see the rich-text
section below.)

## Done: rich text in a cell

`Layout/SheetRichText.cs` holds the portions a cell's text is split into; `Layout/SheetText.cs`
shapes one segment per portion and `Layout/SheetTextLayout.cs` draws one glyph run per segment. The
readers are `Ooxml/XlsxRichRuns.cs` (with `XlsxSheetFormats`), `OpenDocument/OdsCellFormats.cs` and
`MsBinary/XlsWorkbookReader.cs` over `BiffRecordReader.ReadUnicodeString`.

**What is measured.** `sheet-rich-text.{fods,xlsx}` agrees with LibreOffice 24.2.7.2's own PDF on
all **24 portions** of its rich sheet: same count, same glyph counts, same em sizes, same colours,
positions within **0.25 pt**. `sheet-rich-text.xls` agrees on the count and the em sizes.
`sheet-print-xlsx.xlsx` is unchanged at 14/14 pages, 2 281/2 281 words and 13 255/13 255
non-whitespace characters against `soffice` — that last number is 13 255 rather than the 13 269
written down earlier, and *both* sides give 13 255 today, so the earlier figure was counted
differently rather than lost.

**The three formats state a run three different ways, and each is read as it is stated.** This is
the whole of the reader work and it is not a detail:

- **ODF's `text:span` is a delta over the cell's own text properties.** A span stating only
  `fo:font-weight` keeps the cell's family, size and colour, because that is what ODF style
  inheritance means. The spans are flattened in the same walk that counts the columns, mirroring
  `OdfContentReader`'s whitespace collapsing exactly — a mismatch there shifts every offset in the
  cell silently, so the text this counts is handed to `SheetRichText` and compared against what is
  drawn.
- **BIFF's formatting run names a whole `FONT` record**, so it restates the family, size, weight,
  posture and colour whether or not it changes any of them. It states a *start* and no length, so a
  portion reaches to the next run's start and the characters before the first keep the cell's own
  font.
- **SpreadsheetML's `rPr` is a complete font over the workbook's default, not a delta over the
  cell's** — and this one is measured rather than read off the schema. Saving a cell whose first
  word is bold, LibreOffice writes the *cell's* `fontId` as the bold one and then writes the second
  run with an `rPr` that states a size and a name and no `b`; its own rendering draws that run
  **regular**. Its importer says why: a portion's font is built from the theme's default font model
  with every "used" flag already set (`Font::Font(rHelper, bDxf=false)`,
  `sc/source/filter/oox/stylesbuffer.cxx:584`) and the `rPr` overwrites what it names, so the
  cell's font never enters the portion (`RichStringPortion::convert`,
  `sc/source/filter/oox/richstring.cxx:109-118`). **This is the trap that cost the most time here.**
  Reading `rPr` as a delta is the obvious reading, it is what the other two formats do, and it
  leaves the whole cell bold — which looks like a font-resolution bug and is not one.

Rules the layouter gained, each measurable in the reference:

- **A line is as tall as its tallest portion**, and the block's height is the sum of its lines
  rather than a pitch times a count. A fourteen-point word in a ten-point cell pushes the line's
  baseline down; for a cell in one face the two arithmetics give the same number, which is why
  nothing already measured moved.
- **A wrapping rich cell breaks against its own runs**, through `ParagraphLayouter`'s run-aware
  overload over a `MeasuredParagraph`. A bold word is wider than the same characters set regular,
  so breaking against the cell's font alone moves the break by a word — the corpus cell breaks
  after "breaks" in both renderers and after "whose" if measured in one face.
- **A shortened or shrunk rich cell keeps its portions lined up with its characters**, because
  every re-shape is a *range of the cell's text at a percentage of its size* rather than a
  substring handed round. Threading that percentage through is not cosmetic: without it a cell that
  shrinks and is then clipped comes back at full size and keeps a different number of characters.

**Concatenating separately shaped portions is exactly right here and would not be in a word
processor.** Cell text is unkerned (`output2.cxx:405-409`), so there is no pair adjustment to lose
across a portion boundary — checked on Liberation Sans, whose `GPOS` and legacy `kern` tables have
no pair for any boundary in the corpus document, so shaping the portions together would give the
same widths.

Not yet, and why:

- **A rich cell's portion widths are LibreOffice's to within 0.25 pt and not 0.1 pt.** A rich cell
  goes through EditEngine in Calc and through the same shaper as any other cell here, and the two
  measure a *portion* differently. LibreOffice's portion widths are always a whole hundredth of a
  millimetre — `One ` in ten-point Liberation Sans is exactly 762 of them, 21.600 pt, against the
  765.29 the font's own advances give — so the pen drifts, 0.09 pt after one portion and 0.21 pt
  after four. Four models of that quantisation were measured against ten portions of the corpus
  document and none reproduces it: per-character truncation to whole hundredths is the closest at
  0.057 pt against our 0.09, per-character truncation to whole twips reproduces `One ` exactly and
  nothing else, and rounding in either unit is worse than truncating. One portion comes out
  *wider* in LibreOffice than the font's advances allow, which no rounding rule explains and
  hinting would. The plain path has no such difference and still agrees to 0.006 pt.
- **A rich cell's underline, strikethrough and escapement are read past.** All three formats state
  them per run and the drawing IR has no underline, so a superscript footnote marker inside a cell
  draws on the baseline at full size.
- **Extraction still reports a rich cell as one `ContentRun`.** The portions are a rendering
  structure; splitting the content tree would need `ContentRun` to carry a font, which it
  deliberately does not.

## Done: pictures anchored to a sheet

`Layout/SheetDrawings.cs` is the model and `Layout/SheetPageGraphics.cs` places and paints;
`Ooxml/XlsxDrawings.cs` and `OpenDocument/OdsDrawings.cs` read the two formats.

**Measured**: the corpus picture lands within **0.028 pt** across, **0.011 pt** down and
**0.028 pt** of width, and its height is exact, against LibreOffice's own PDF — from the flat ODF
source and from the SpreadsheetML export alike, compared as image-XObject placements with
`PdfPaints.ReadImageDraws`.

**The anchor is the whole of the work.** A drawing is fastened to the grid by a cell and an offset,
so it cannot be placed until the column widths are known, and a two-cell anchor's size is the span
between its corners rather than anything the file states as a length. That is measurable rather
than theoretical: the corpus frame states `svg:width="1.28in"` and ends at C3, and LibreOffice
draws 1.3201 in — the two columns it crosses less its own start offset — and rewrites the attribute
to match when it saves. So an ODF frame carrying a `table:end-cell-address` is read as a two-cell
anchor and one without it as a one-cell anchor, which is the distinction the attribute exists to
make. The span goes through `SheetDeviceUnits` per column, like everything else on the page, or a
picture two columns wide would not line up with the column it ends in.

**Drawn after the grid**, which is Calc's own order: the front drawing layer runs after
`DrawGrid` (`printfun.cxx:1695-1703`), so a logo covers the gridlines under it rather than being
crossed by them.

**Nothing is decoded.** `RasterImage.Encoded` carries the bytes the file stored and whichever
backend wants pixels calls `RasterImageDecoder.Ensure`. That is the layering rule rather than a
convenience — a reader that decoded would put a codec in the extraction path.

**A metafile draws too, and it is deferred by a different mechanism for the same reason.**
`SheetDrawing` gained a `Lazy<VectorImage>?` beside its `RasterImage?`, and both readers sniff the
bytes with `VectorImages.For` before wrapping them. It needed nothing in `Paperless.Core`:
`VectorImage` is already `Draw(IDrawingSink, DocRect)` plus an intrinsic size, and this library
already referenced `Paperless.Vector`. A `Lazy` rather than an eager decode because the decoder
*is* reachable from here — measured on this tree, the first `VectorImages.Decode` in a process
costs **1044 ms** for a WMF with one text run against 0.21 ms once warm, nearly all of it resolving
faces through `Paperless.Text`, and a caller after cell values must not pay it.
`SheetVectorPictureTests.NothingIsDecodedUntilSomethingAsksForThePicture` asserts `IsValueCreated`
is false after a full layout.

**The part name is a lie and the bytes are not.** `vector-picture-sheet.xlsx` is LibreOffice's own
export of `vector-picture-sheet.ods` and it writes the EMF into **`xl/media/image2.wmf`**, with
`[Content_Types].xml` declaring nothing useful for either extension. An EMF+ would be less
distinguishable still — it *is* an EMF, same `EMR_HEADER`, same signature, with no signature of its
own anywhere. So `VectorImages.For` sniffs and the declared type is not consulted at all.

**The `svgBlip` extension reaches spreadsheets too**, which was not obvious: the same
`{96DAC541-7B7A-43D3-8B79-37D633B846F1}` extension Word and PowerPoint write appears on an
`xdr:pic`'s `a:blip`, naming an SVG beside the PNG on `r:embed`. `BlipReference.Choose` is what
`XlsxDrawings` now calls, and the raster is kept beside the vector so an empty decode falls back to
it rather than to nothing. ODF needs no such step — a `draw:frame` lists alternatives as sibling
`draw:image` children and the first drawable one wins.

**Measured**, both PDFs rasterised at `pdftoppm -r 150`: `vector-picture-sheet.ods` `mae 0.0059`
and `.xlsx` `mae 0.0104`, 10/10 words and 1/1 pages on both. The residual ink is `emf-shapes.emf`'s
gradient bar, which LibreOffice's own EMF import does not draw — verified by converting the bare
`.emf` with `soffice`.

**A named trap, and it cost an hour of looking in the wrong place.** The anchor arithmetic was
right on the first run and the page came out blank, because four guards in `Paperless.Rendering`
asked `image.Width <= 0` before drawing — the right question only for an image that has *already*
been decoded, and an encoded one has zero for both dimensions until a codec has seen it. The
display list held the picture, `Ensure` would have decoded it, and nothing reached the point of
calling `Ensure`. Fixed on the slide-pictures branch (`fa666554d`) rather than here; the symptom to
recognise is a placement that measures correctly and paints nothing at all.

**A shape is not drawn and is still an object, and leaving it out cost whole files.** An
`xdr:sp`, `xdr:cxnSp` or `xdr:grpSp` was dropped by `XlsxDrawings` on the reasonable ground that
nothing here can paint one. But Calc's print area is the bounding box of *every object on the
drawing layer* and a shape is an object like any other — `GroupShapeContext::createShapeContext`
takes `sp`, `cxnSp`, `grpSp`, `graphicFrame` and `pic` alike
(`sc/source/filter/oox/drawingfragment.cxx:198`) — so dropping them meant a sheet whose only
content was a shape had **no printed block at all and produced no pages**: `paperless render`
failed outright with *"the page range selects none of the 0 pages"*. Measured over the **55**
workbooks in `sc/qa/unit/data/xlsx/` and `chart2/qa/extras/data/xlsx/` that hold a sheet shape:
**27 rendered nothing, now 1** (`forcepoint107.xlsx`, which LibreOffice also declines), and exact
page-and-word matches went **7 → 15**. Twelve of the chart workbooks went from one page to the two
LibreOffice prints, or two to four. The anchor is read and the `cNvPr` is read; nothing else is,
and `SheetPageGraphics` skips any drawing carrying neither picture nor chart, so a shape reaches
the print area and stops there.

**"Hidden" means two unrelated things and only one of them is a shape.** The print area skips an
object only when it sits on `SC_LAYER_HIDDEN` (`ScDrawLayer::GetPrintArea`, `drwlayer.cxx:1408`),
and that layer holds exactly one kind of thing: the caption of a comment nobody has pinned open
(`sc/source/core/data/postit.cxx:84`). A shape whose `cNvPr` says `hidden="1"` is *not* on it —
`oox` gives that shape `Visible = false` and `Printable = false` and leaves it on the standard
layer (`oox/source/drawingml/shape.cxx:1436-1442`) — and the line immediately above the layer test
admits as much: `//TODO: test Flags (hidden?)`. So the flag is read, the painter honours it, and
the print area does not. Reading `#i104716# don't include hidden note objects` as though it were
about the flag rather than about the layer is the mistake, and it is worth `sc/qa/unit/data/xlsb/
universal-content.xlsb`: its only drawing is a hidden comment shape reaching column 12 and row 50,
**1 page and 11 words here against LibreOffice's 4 and 20**, now 4/4 and 20/20.

Not yet, and why:

- **BIFF drawings are not read.** The route is the same shared Escher reader the DOC and PPT work
  use — `MSODRAWINGGROUP` (0x00EB) in the workbook globals holds the BLIP store and `MSODRAWING`
  (0x00EC) in each sheet substream holds the shapes, with the client anchor in the `OBJ` record
  (`sc/source/filter/excel/xiescher.cxx`). What is missing is not the anchor arithmetic, which is
  shared with the two formats above, but a **BLIP store reader**. That is now half-written and in
  the wrong library: `Paperless.WordProcessing/Ww8/Ww8Blips.cs` reads an `F007` entry, follows its
  `foDelay`, tolerates the one-or-two-checksum rule and — since the vector wiring — inflates a
  metafile out of its `OfficeArtMetafileHeader`. None of that is DOC-specific. Moving it into
  `Paperless.MsBinary/Escher/` is what buys XLS and PPT pictures at once, and the metafile half is
  the part that would be most annoying to write twice.
- **A drawing belongs to the page holding its top-left cell.** Calc positions the drawing layer in
  document coordinates and clips it per page, so a picture straddling a page break appears on both
  pages, cut. Anchoring it to one page is the same answer for everything that does not straddle.
- **A rotated picture is not expressible.** `IDrawingSink.DrawImage` takes a rectangle rather than
  a matrix, so `xdr:spPr/a:xfrm/@rot` and ODF's `draw:transform` are read past. Recorded rather
  than fixed: four agents are building against that IR at once.
- **A crop is not applied.** SpreadsheetML states one as `a:srcRect` fractions and ODF as
  `fo:clip`. The drawing model has clipping and no crop, so the shape is a larger destination
  rectangle clipped to the frame's outline rather than a new IR primitive.
- **A chart's tick labels are formatted now, and the formatter is no longer this family's.** The
  number-format engine moved from `Paperless.Spreadsheets/Numbers/` down into
  `Paperless.Core/Numbers/`, beside `Core/Charts`, which is what a chart composed in Core needed to
  write `1,200,000` on an axis instead of `1200000`. Nothing about this family's use of it changed
  but the namespace: it is pure computation over a format code with no external dependencies, so
  Core keeps its zero-dependency rule. `NumberFormatterTests` went with it to
  `Paperless.Core.Tests`; `NumberFormatCodeTests` stayed here, because half of what it asserts is
  which built-in code an XLSX style index stands for. ODF reaches the same engine through
  `Paperless.OpenDocument/Styles/OdfNumberFormat.cs`, which compiles a `number:*-style` tree into a
  format code exactly as `xmloff`'s own import does. See `dotnet/TODO.md` Phase 3.5.
- **A rotated label in a chart is no longer only the value-axis title.** A crowded category axis now
  turns its labels 45° — `ChartAxisLabels` in `Paperless.Core.Charts`, a port of
  `VCartesianAxis::createTextShapes` — and `Layout/SheetChart.cs` needed no change for it: the sink
  transform it already built for the axis title carries any angle. Chart-wide rather than
  spreadsheet-specific, and this is the second time putting rotation on `ChartLabel` rather than on
  the one caller that had it has cost the consumers nothing.
- **Trendlines draw, and `trendline.ods` is still not an oracle.** `chart:regression-curve` and
  `chart:mean-value` are read in `Paperless.OpenDocument/OdfChartPlot.cs` and fitted by
  `Paperless.Core/Charts/ChartRegression.cs`, a port of `chart2/source/tools/`'s seven calculators.
  ODF states more than OOXML does and one thing it states is free: `chart:equation/@svg:x` and
  `@svg:y` give the equation's position outright, in the same coordinate space
  `chart:coordinate-region` uses, so an ODF chart needs no equivalent of `VSeriesPlotter`'s default
  placement at the curve's own corner. The 59-workbook error is unchanged at ~2176 and still is not
  a chart measurement: it is the *spreadsheet's* uncomputed formulas, which is why the deck set in
  `chart2/qa/extras/data/pptx/` is the honest one. See `dotnet/TODO.md` Phase 3.5.
- **A data table under a chart is OOXML-only.** `c:dTable` has no ODF counterpart at all, so an ODS
  chart never draws one and `ChartPlot.DataTable` is always null on this family's ODF path.
- **VML shapes are not read.** A legacy cell comment's box and Excel's form controls live in
  `xl/drawings/vmlDrawing*.vml`, a different vocabulary reached by a different relationship.
- **A shape's own text is not drawn**, which is now the largest remaining gap on any sheet holding
  one. A shape reaches its page and paints nothing, so `sc/qa/unit/data/xlsx/tdf119565.xlsx`
  renders its one correct page with 0 words against LibreOffice's 29 — all 29 of them inside
  `xdr:sp/xdr:txBody`. The body is DrawingML's ordinary `a:p`/`a:r`, the same vocabulary the deck
  path already lays out, so this is a wiring job rather than a new reader; what it needs is a
  laid-out text block inside a `DocRect` that this family does not yet have a route to.
- **`FormatCatalogue` still reports XLSB as not readable.** `IsReadSupported` is what
  `paperless identify` prints and the flag was deliberately left alone, because that list is in
  `Paperless.Core`, another agent is working there, and it is already stale for XLS, CSV, PPT and
  PPTX besides — so it wants one deliberate pass rather than one format bolted on.

## The row height a rotated cell asks for, and why it is not recomputed

All three formats say whether a row's height was set by a user or computed by the writer — ODF's
`style:use-optimal-row-height`, SpreadsheetML's `customHeight` and BIFF's `fUnsynced` — and Calc
honours it on load, recomputing every non-manual row before anything is drawn
(`ScDocRowHeightUpdater`). The flag **is** read, into `SheetSizeRun.IsOptimalSize`, by all three
readers. The recomputation is not shipped, and the reason is a measurement rather than an estimate.

The formula is two lines: `height = textHeight × |cos θ| + textWidth × |sin θ|`, then the cell's top
and bottom margins (`ScColumn::GetNeededSize`, `sc/source/core/data/column2.cxx:517-546`).
Reproducing it exactly still gives the wrong answer, because the measurement it consumes is not the
one Calc draws with. Five probe documents, each one rotated cell in a row asking for an optimal
height, read back out of the ODS LibreOffice saved:

| cell | LibreOffice | from the font's advances |
| --- | --- | --- |
| ten capital `X` at 90° | 68.65 pt | 68.70 pt |
| `Upright heading` at 90° | 68.65 pt | 72.65 pt |
| `Upright heading` at 45° | 56.69 pt | — |
| the same text twice over at 90° | 135.81 pt | — |

The first row says the formula and the margins are right. The second says the *measurement* is not:
Calc's row-height path collapses two strings 4 pt apart onto the same height, while its printed
output advances the pen by the font's exact widths — checked against the per-character `Tm`
matrices in its own PDF, where `S` in `Slanted heading` advances 7.229 pt against the font's 7.227.

So recomputing would replace a correct number with one 5.8% too large on **every file LibreOffice
wrote**, because there the stored height *is* the height Calc computed, and it would move every row
below it. It is only an improvement on a hand-written sheet whose stated height is a stale cache,
which is the corpus's flat ODF sources and nothing a user has. Shipping it needs Calc's own coarse
measurement reproduced first, and that is the same missing piece as the dynamic header band's:
`UpdateHFHeight` measures through the same kind of reference device.

The premise this was checked against is worth writing down because it does not survive: pagination
already depends on measuring text once, in `ScTable::ExtendPrintArea`, and that is survivable
because the extension is **by whole columns** — being within a column's width of LibreOffice's
answer gives the same page. A row height is a length, so a 5.8% error is a 5.8% error, and there is
no quantum to hide it in.

**The XLSB reader reads the flag too**, so all four readers now fill `SheetSizeRun.IsOptimalSize`
and the decision above covers the whole family: BIFF12's `ROW` states it as
`BIFF12_ROW_CUSTOMHEIGHT` (0x2000), whose *absence* is the optimal-height case
(`sheetdatacontext.cxx:432`). That is the same polarity as SpreadsheetML's `customHeight` and the
opposite of BIFF8's `fUnsynced`, which is worth checking against the source rather than inferring
from the neighbouring format. Nothing else about the decision changed: what unblocks it is still
Calc's own coarse measurement, and nothing here has it yet.

## Done: a chart's content, out of the cache

`Ooxml/XlsxCharts.cs` for SpreadsheetML and `Paperless.OpenDocument/OdfChart.cs` for ODF, both
producing the section shape `Paperless.Ooxml/DrawingML/DrawingChart.cs` documents: a
`SectionKind.Frame` section whose `Name` is the chart's title, the title and each titled axis as
paragraphs, then one `ContentTable` with the series across the header row and the categories down
the first column. Read into `SheetChartTests` against `chart-bar-sheet.{fods,ods,xlsx}`.

**A chart follows its sheet, it does not sit inside it**, and the reason is the anchor rather than
taste. SpreadsheetML anchors a drawing by address, so a chart could have gone anywhere; ODF anchors
it by *containment* — the `draw:frame` is a child of the `table:table-cell` it is fastened to — so
placing the chart where it was found would put a whole `ContentTable` inside a `ContentTableCell`,
a shape nothing else in the tree produces. Both readers therefore hoist it to a sibling section
immediately after the sheet, which is exactly where a cell comment already goes, and the two
families come out identical. Measured on LibreOffice's own `chart2/qa/extras/data/xlsx`: **151 →
153** of its 154 workbooks extract to something. A small movement, and expected — a workbook
carrying a chart also carries the cells behind it, so unlike a deck it was never empty. The
comparison worth having on this family is the one `SheetChartTests` makes instead: the cache
against the range it names.

**Three relationship hops, and the middle one is easy to get wrong.** The sheet declares
`…/drawing`; the drawing part declares `…/chart` per graphic frame; and `c:chart/@r:id` inside the
frame resolves against the **drawing** part, not the sheet. Resolving it against the sheet finds
nothing in most workbooks and finds the wrong part in one whose sheet happens to declare an `rId1`
of its own.

**The cache is preferred even where the live range is right there.** `chart-bar-sheet.xlsx` states
`c:f = Revenue!$B$2:$B$5` over cells in the same workbook, and its `c:numCache` repeats those eight
numbers; `SheetChartTests.TheCacheAgreesWithTheSheetItReferences` reads both and asserts they
match, so a divergence fails a test instead of producing a plausible chart. Resolving the range
instead would need the formula engine and the number formatter — and would disagree with what Calc
draws the moment a cache is stale, which is the case the rule exists for.

## Done: a sheet's chart is drawn

**All three forms match the reference now.** `chart-bar-sheet.{ods,fods,xlsx}` went from **14 words
against 34, 34 and 29** to **34/34, 34/34 and 29/29**. `SheetDrawing` carries a `ChartPlot` beside
its `Image`; `Ooxml/XlsxDrawings.cs` and `OpenDocument/OdsDrawings.cs` read it in the same pass as
the anchor, because the anchor is what gives the chart a rectangle and the rendering path walks the
drawing part exactly once; and `Layout/SheetChart.cs` paints a laid-out chart straight into the
`IDrawingSink`. `XlsxCharts` still walks the part a second time for the content tree, which is
deliberate — extraction must not pay for the anchors.

**The Core move is what made it possible, and it is done.** `ChartPlot`, `ChartScale` and
`ChartLayout` are in `Paperless.Core.Charts`; `OdfChartPlot` and `OdfChartStyles` came down into
`Paperless.OpenDocument`, so one ODF reader serves ODP, ODS and ODT and this family needed no copy
of it. Only the geometry and the model moved — nothing that parses XML — so Core still has zero
external dependencies.

**And the number formatter followed it down, for the same reason and with the same test.**
`Numbers/` — `NumberFormatCode`, `NumberFormatSection`, `NumberFormatter`, `BuiltInNumberFormats`,
`SpreadsheetDate` — is now `Paperless.Core/Numbers/`. Every one of those files imported
`System.Globalization` and `System.Text` and nothing else, so the move costs Core nothing; what it
buys is that a chart composed in Core can write `1,200,000` on an axis. This family's readers
changed by one `using` each.

**A sheet's chart type is stretched properly now.** `SheetChart.Text` folds `ChartLabel.Stretch`
into a sink transform, so a chart composed at its stated 12 × 7 cm and drawn into a frame 0.625 as
wide and 0.709 as tall gets type at the right width rather than `sx/sy` too wide. Measured against
LibreOffice's PDF for `chart-bar-sheet.ods`: the title measured **70.4 pt against a reference 62.1**
and now measures **63.7**.

**Three defects a deck could never have found, and each looked like something else.**

- **The replacement picture wins if you look for it first.** An ODS `draw:frame` holding a chart
  carries `draw:image xlink:href="./ObjectReplacements/Object 1"` *beside* the `draw:object`.
  `OdsDrawings` tested for the image first, so every chart in every ODS LibreOffice has ever
  written was recorded as a plain picture — and then painted as nothing, because all 82 of those
  streams are `VCLMTF` and no decoder here reads StarView metafiles. An ODP frame carries the
  object alone, so the deck path never saw it. **This is the named trap**: the symptom was "a
  spreadsheet chart draws nothing", which reads as a missing feature and was a wrong `if` order.
- **`a:graphic` is DrawingML's, not the spreadsheet drawing's.** `XlsxDrawings` looked the element
  up in the `spreadsheetDrawing` namespace, found nothing, and therefore never set `IsChart` on any
  XLSX graphic frame at all. Invisible until something downstream needed the flag.
- **A chart is anchored below the used range and Calc prints it anyway.** `chart-bar-sheet.ods` has
  four rows of data in `A1:C5` and anchors its chart in row 7. The print area is computed from the
  *cells* — `ScTable::GetPrintArea` (`sc/source/core/data/table1.cxx:657`) tests data, notes,
  sparklines and attributes and never asks the drawing layer — but the drawing layer is then painted
  in document coordinates and clipped to the paper, not to the used range
  (`PrintDrawingLayer(SC_LAYER_FRONT)`, `printfun.cxx:1699`). `SheetPageGraphics` now continues
  through the grid past the last printed row or column, snapped per index as the two-cell anchor's
  own edge already was.

**The chart is composed at its own stated size and stretched into the frame.** Measured: the chart
document states `svg:width="12cm" svg:height="7cm"` and the frame is 2.952 in by 1.9547 in, and in
LibreOffice's PDF the chart's 13 pt title measures **62.1 pt wide against 99.4 pt for the same title
in the same chart's `.xlsx` form** — 0.625, the width ratio exactly — with a height ratio of 0.708.
Composing in the frame instead would give an axis 77 pt long, room for six tick intervals, and
`0 50 … 200`; composing at 12 × 7 cm gives 108.8 pt, room for nine, and `0 20 … 180`, which is what
the reference draws.

**Text is measured and drawn by `SheetBandText`, not by the cell engine.** A chart's line height is
the face's own — ascent plus descent plus the gap, 1.1499 em in Liberation Sans — where a cell's
drops the gap (`ScDrawStringsVars`, `output2.cxx:734`). `ChartLineHeightAt` is the one method added
for it. The print zoom is applied to the *type* as well as the rectangle, because a chart laid out
at 100% type in a 50% rectangle reserves twice the room its labels need.

**The workbook's theme reaches a chart part now, and it was the difference between a picture and an
empty plot area.** `XlsxDrawings.Plot` called `DrawingChartPlot.Read` with no theme, and the comment
beside it recorded that as harmless because "every chart in the corpus states its fills as
`a:srgbClr`". That is true of every chart LibreOffice wrote and false of charts Excel wrote.
Measured on `chart2/qa/extras/data/xlsx/bubble_chart_simple.xlsx`, whose three series state
`a:schemeClr val="accent1|2|3"` with `a:ln/a:noFill`: every bubble resolved to no fill and no
outline, so the plot area came out with its axes, its legend and not one mark on it — which reads
as an unimplemented chart type and was an unresolved colour. `XlsxFile.ThemeRoot` was already being
loaded for `XlsxCellDecoration`; threading it through `XlsxDrawings.Read` is four lines, and it took
`barOfPieChart.xlsx` from 11 drawn marks to 20. Over the whole 154-file `xlsx/` set the total
absolute word error went **2599 → 2547** with exact matches **8 → 9**.

**A sheet-anchored ODS drawing is still not read, and it hides a whole chart.** `OdsDrawings` walks
`draw:frame` inside `table:table-cell` only, so a frame in `table:shapes` — the sheet-level shapes
container, positioned by `svg:x`/`svg:y` against the sheet rather than by a cell — never becomes a
`SheetDrawing`. `chart2/qa/extras/data/ods/tdf166428_Low_High_StockChart_LO248.ods` writes its stock
chart exactly that way: the chart is read into a correct `ChartPlot` and then has nowhere to go, and
the file measures **24 words against LibreOffice's 60**, all of the difference being the chart. It
is the same shape of defect as the `ObjectReplacements` one above — the chart engine is not
involved, the drawing never arrives — which is why it is recorded here and not in the chart section.

**A chart anchored past the right-hand page break lands on a page we do not produce.** Every one of
the five OOXML chart files measured for the plot-type work renders one page against LibreOffice's
two, and `xlsx/bubble_chart_simple.xlsx` — anchored at column 11, well right of its four cells —
still measures 5 words against 26 with a complete chart composed behind it. Sheet pagination, not
charts.

Left open here:

- **A glyph run cannot be stretched non-uniformly.** The frame-to-chart mapping scales positions by
  `sx` and `sy` and the em size by `sy` alone, so text comes out `sx/sy` too wide — 12% on
  `chart-bar-sheet.ods`. Fixing it means a transform round each run.
- **BIFF charts** are still not read; see below.

Not yet, and why:

- **BIFF charts.** A chart in an XLS lives in a `Chart` substream of its own, reached from the
  `OBJ`/`MSODRAWING` pair, with its series in `SERIES` (0x1003), `AI` (0x1051) and the `SIINDEX`
  block rather than in anything resembling a cache. It needs the Escher work the pictures item
  above is already waiting on, and then a fourth chart reader rather than a third: the BIFF chart
  records are not a serialisation of the same model.
- **`xl/charts/chartN.xml` is opened a second time.** `XlsxDrawings` walks the drawing part for
  layout and `XlsxCharts` walks it again for content, because extraction must not pay for the
  anchor arithmetic and a caller that never asks for content never opens a chart part. The parts
  are cached by the package, so the cost is a second parse of one small XML document per chart.
