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

Not yet: applying number formats (unnecessary while the cached display text is present, but
rendering will need it), print setup, and the sparse typed cell storage below, which
extraction does not need and layout will.

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
