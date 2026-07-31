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

- **Built-in number-format ids 0–49 are the `en_US` table.** They are locale-dependent —
  LibreOffice keeps one table per locale (`sc/source/filter/oox/numberformatsbuffer.cxx:436`,
  `en_US` at `:798`) and picks by the workbook's locale — so a German workbook using bare id
  14 extracts its dates as `M/D/YYYY` rather than `DD.MM.YYYY`. It only bites files that use
  a built-in id *without* declaring a `numFmt`, which LibreOffice never writes and Excel does
  constantly. Fixing it needs locale infrastructure Paperless does not have.
- **`General` is fifteen significant digits, not column-width dependent.** Calc's `General`
  picks between fixed and scientific by how many characters fit in the column
  (`research/03-calc.md` §A.7), and so does the `###` a too-narrow numeric cell shows. Both
  need a width, which extraction has no way to know; they belong to layout.
- **Dates before March 1900 follow Excel, not LibreOffice.** Excel treats 1900 as a leap year
  — a Lotus 1-2-3 compatibility bug — so its serial 59 is 28 February 1900 and 60 is a
  29 February that never existed. Paperless reproduces that, because the file was written
  under it. LibreOffice deliberately does not
  (`sc/source/filter/oox/workbooksettings.cxx:295`: "LO never treats 1900 as a leap year"),
  so a comparison of dates in the first two months of 1900 will differ by one day. Nothing
  real is affected; recorded so the difference is not mistaken for a bug.
- **Drawings, charts, pivot caches and defined names.** None is reached yet;
  `oneCellAnchor`/`twoCellAnchor` will want the shared DrawingML text-body reader in
  `Paperless.Ooxml` that the PPTX work is building.
- **Print setup**, which is the page geometry and therefore a rendering prerequisite.
- **The sparse typed cell storage below**, which extraction does not need and layout will.

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
- [ ] Substreams; the `BOF`/`EOF` structure
- [ ] `SST`, `XF`, `FONT`, `FORMAT`, `DIMENSIONS`, `ROW`, `COLINFO`, `MERGEDCELLS`
- [ ] Cell records: `LABELSST`, `NUMBER`, `RK`, `MULRK`, `BLANK`, `MULBLANK`, `FORMULA`,
      `ARRAY`, `SHRFMLA`, `STRING`, `BOOLERR`
- [ ] BIFF formula token decoding
- [ ] RC4/XOR decryption; `CODEPAGE`
- [ ] BIFF5 where cheap; BIFF2 probably not worth it

### CSV
- [ ] Separator, quoting and encoding detection. Genuinely ambiguous — a mismatch here may
      be expected rather than a bug.

## Rendering

- [ ] Cell text: alignment, wrap, shrink-to-fit, rotation, indent
- [ ] Overflow into adjacent empty cells; `###` when a number does not fit — note the
      displayed text is **column-width dependent**, so it cannot be computed before layout
- [ ] Rich text within a cell
- [ ] Grid lines, backgrounds, and border resolution between neighbouring cells
- [ ] **Print pagination** — port this faithfully; it is the page geometry
- [ ] Repeated rows/columns; scale-to-pages; headers and footers
- [ ] Drawing objects and charts anchored to cells
