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
- **Drawings, charts, pivot caches and defined names.** None is reached yet;
  `oneCellAnchor`/`twoCellAnchor` will want the shared DrawingML text-body reader in
  `Paperless.Ooxml` that the PPTX work is building.
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
- **`Print_Titles` holds both repeated bands in one name and distinguishes them by shape**, not by
  order: the column band is a whole-column reference with no row digits and the row band a
  whole-row reference with no column letters. In BIFF that is stored against sheet limits of 255
  columns and 65 535 rows, so the "spans the whole sheet" test has to be made against *those*
  limits and then widened, not against Calc's.

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
- **`SkipEmpty` is not implemented.** Calc drops a page whose whole block is blank, but only when
  the caller passes the option (`ScPrintOptions::GetSkipEmpty`), and its PDF export does not — so
  reproducing the reference means not implementing it.
- **The used area counts cells with content only.** Calc's own search also counts a cell carrying
  nothing but a style, because its attribute array knows about it. The content tree records no
  formatting, so a sheet whose last two columns are empty-but-shaded comes out narrower here.
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
- [ ] Drawing anchors: `oneCellAnchor`, `twoCellAnchor`, `absoluteAnchor`
- [ ] Tables, autofilters, pivot caches (extraction only)

### XLSB
- [ ] BIFF12 records inside an OPC package. Same logical model as XLSX, binary encoding.
- [ ] **Import only** — LibreOffice cannot write XLSB, so test files must come from Excel.

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
- [ ] Rich-text runs inside a cell. The `SST` string's formatting runs are read far enough to
      skip past them; splitting a cell into several `ContentRun`s needs the `FONT` table.

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

- [ ] Cell text: alignment, wrap, shrink-to-fit, rotation, indent
- [ ] Overflow into adjacent empty cells; `###` when a number does not fit — note the
      displayed text is **column-width dependent**, so it cannot be computed before layout
- [ ] Rich text within a cell
- [ ] Grid lines, backgrounds, and border resolution between neighbouring cells
- [x] **Print pagination** — ported, in `Layout/SheetPagination.cs`
- [x] Repeated rows/columns; scale-to-pages; page order
- [ ] Header and footer text: the `&P`/`&D`/`&A` field language, and laying it out. The band's
      *height* is read and reserved, which is what pagination needs; nothing draws in it yet
- [ ] Drawing objects and charts anchored to cells
