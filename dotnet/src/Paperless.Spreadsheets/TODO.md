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

Not yet: print setup, and the sparse typed cell storage below, which extraction does not need
and layout will.

**Done: XLS (BIFF8 and BIFF5) extraction and CSV**, in `MsBinary/` and `Csv/`. The XLS reader
produces the same content-tree shape as the ODS one — a section per sheet holding one table,
hidden sheets flagged rather than skipped, cells carrying a typed value and the displayed
text — with one deliberate difference: `Formula` is null, because BIFF formulas are RPN token
arrays and the cached result is what a reference renderer shows.

**Done: number formats**, in `Numbers/`. Not optional for BIFF the way it is for ODF: a date
is a serial number and nothing caches the text, so without the format a workbook of dates
extracts as five-digit integers. The same code will serve XLSX, whose `numFmt` codes are the
same language.

## Document model

- [ ] Sparse column-oriented cell storage; typed blocks rather than boxed per-cell objects
- [ ] Formatting as a separate run-length structure keyed by row, with pooled pattern
      objects
- [ ] Three-level attribute resolution: direct formatting → named cell style → default
- [ ] Row heights and column widths as run-length segments; hidden rows/columns
- [ ] Merged ranges; cell borders (resolving the shared-edge conflicts between neighbours)
- [ ] Number formats, including custom codes and locale-dependent behaviour
- [ ] Conditional formatting; data validation
- [ ] Defined names; sheet-local and workbook-global scopes
- [ ] Comments/notes; drawing objects; charts
- [ ] Print setup: print areas, repeated rows/columns, scale-to-pages, page order, headers
      and footers

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
- [ ] `workbook.xml`, `worksheets/*.xml`, `sharedStrings.xml`, `styles.xml`, `theme1.xml`
- [ ] Styles: `cellXfs`, `numFmts`, `fonts`, `fills`, `borders`, `dxfs`
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
- [ ] Fractions (`# ?/?`). `NumberFormatCode.IsUnderstood` reports false for them and the
      reader records a diagnostic rather than showing a wrong number; they need a
      continued-fraction search rather than a digit walk.
- [ ] Conditions (`[>100]`), and month and weekday names in a locale other than English. The
      name a spreadsheet shows depends on the reading application's locale, so there is no
      right answer available from the file alone.

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
- [ ] **Print pagination** — port this faithfully; it is the page geometry
- [ ] Repeated rows/columns; scale-to-pages; headers and footers
- [ ] Drawing objects and charts anchored to cells
