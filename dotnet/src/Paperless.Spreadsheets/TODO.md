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
  which is 20.76 × 111 rounded. **The font is now read and the digit measured** — see the
  column-width section below for where the measurement lands and why it could not go in the
  reader.
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

## What the tenth sheets sweep found: a paragraph is not one size, and a note is not on its cell

Swept whole at `5ec407cf3` before anything was changed: **127 of 171**, page error 117, 142 exact
page counts. The brief said 132, 119 and 141, measured at `6b6d54d37` plus its fix; the two
continuous quantities reproduce within two and the match count is five lower. Ten of the fourteen
per-batch figures reproduce exactly and 007, 008, 014 and 018 are each one lower — every one of
the four extra failures a word-gate verdict on a page-exact document, and two of them measurably
the `Tj`-splitting artefact the brief names (see below).

### An unsized DrawingML run is twelve point, and the paragraph around it keeps its own

`SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.xlsx` was page-exact and 31 words short, and
the missing words were in the two paragraphs of its methodology box that end with a run stating no
`sz`:

```xml
<a:r><a:rPr lang="en-GB"><a:effectLst/></a:rPr><a:t> </a:t></a:r>
```

Two bugs met on that one character. `SheetShapeText.DefaultSize` was 18 pt, cited as DrawingML's
own default — which it is, for the *shape*: `Shape::setDefaults` puts 18 pt on it
(`oox/source/drawingml/shape.cxx:334`) and LibreOffice's flat-ODS export writes exactly that as
every text box's default paragraph style. It is not what a **run** inherits. `TextBody::insertAt`
reads the text cursor's `CharHeight` before any of the body is inserted
(`oox/source/drawingml/textbody.cxx:62`) and `TextRun::insertAt` puts that on a run whose own
`moHeight` is unset (`textrun.cxx:82-85`); on a fresh Calc drawing object that is the EditEngine
pool's 240 twips. And `SheetShapeParagraph` collapsed a paragraph to the largest size any run
states, so one stray character re-measured 440 characters of body text.

**Measured against the binary, not derived, because both candidates are in the file.** A probe
workbook with three text boxes, round-tripped through the flat-ODS export:

| box | runs | export says | breaks after |
|---|---|---|---|
| one run, no `sz` | — | `fo:font-size="12pt"` | "decides" |
| `sz="1100"` body + unsized space | two spans | 11 pt and 12 pt | "every" |
| `sz="1100"` body + `sz="1800"` run | two spans | 11 pt and 18 pt | "every" |

All three shapes' default paragraph style says 18 pt and none of their runs does. The third row is
the clean statement of the second rule: a large run that is real, stated and drawn still does not
reach the words in front of it.

**A word may span a run boundary**, so wrapping run by run would not have worked either — SSRO
splits "either" as `" e"` + `"ither the date…"`, which is what an authoring tool leaves behind
when a character property is applied and undone. The painter now flattens a paragraph to text with
a per-character format beside it, wraps the whole string once, and cuts each line back into the
maximal stretches that share a format; a line's height and ascent are the largest its stretches
need, and its pieces are drawn end to end on one baseline. `a:endParaRPr` also reaches a blank
paragraph now, so the gap between two blocks is reserved at the body's own size.

Reach, measured by rendering rather than by grepping: **eleven of the track's documents have shape
text with runs at all**, all eleven were rendered before and after, and **two moved** — SSRO 519
words to 548 of 550, and `Foreign_SA-CAT-I_and_CAT-II-III_Pub_0.xlsx` 1530 to **1505 of 1504**,
which is its `vertOverflow="clip"` notes box now cutting in the right place. No page count moved
on any of them.

### A sheet's notes are printed on pages of their own, and nothing read the flag

`Hazard Analysis Template.xls` was 2 pages against 3 and 460 words against 682, and the missing
page is a list of its cell comments. Excel's "Comments: at end of sheet" —
`EXC_SETUP_PRINTNOTES` → `ATTR_PAGE_NOTES` (`sc/source/filter/excel/xipage.cxx:84`, `:257`),
`ScPrintFunc::PrintNotes` and `DoNotes` (`printfun.cxx:1930-2066`).

The rules are all in `DoNotes` and short enough to state whole: the page is divided by the width
of `"GW99999:"` in the default cell font, capped at half the printable width; the marks are flush
left and the note text wraps in what is left; a note is placed only while
`nPosY + nTextHeight < aPageRect.Bottom()`, and the pen then advances by its height plus 200
twips. Nothing is scaled — `DoNotes` sets `aTwipMode` rather than the page's zoomed map mode. The
order is **column-major** (`CountNotePages`, `printfun.cxx:2591-2600`), which is visible in the
reference: `Hazard Analysis Template.xls`'s note page runs D1, F2, H2, J2, L1, N2, P2, R2.

Reading it needed a join neither record holds. BIFF8's `NOTE` carries the cell and *names* an
object; the characters are in that object's `TXO`. `ftCmo`'s identifier was being skipped, and is
now kept and joined after the sheet is read, since a `NOTE` may precede or follow its `OBJ`.

Reach: **two of the 171 documents ask for this**, and 15 more hold comments without asking.

| | before | after |
|---|---|---|
| `Hazard Analysis Template.xls` | 2 pages / 460 words against 3 / 682 | **3 / 682**, exact |
| `RMP 2011-2014 and Inventory.xls` | 37 pages against 38 | 39 against 38 |

RMP now draws **both** of the note pages the reference draws — its pages 22 and 38, eight marks
and four, same order, same text. Its remaining page is not this: our first sheet takes 22 pages of
cells where the reference takes 21, and that predates this round untouched. It cost a while to see
because the reference's note pages are *not* at the end of the file: they follow each sheet, so
sheet 0's sits at page 22 in the middle of the document and reads as content at a glance.

**SpreadsheetML's `cellComments` is read as `asDisplayed`, not `atEnd`.** That looks backwards and
is what the binary does: Calc has one mode, so its OOXML filter has to choose which of the two
values turns it on, and `PROP_PrintAnnotations` comes from `mnCellComments == XML_asDisplayed`
(`sc/source/filter/oox/pagesettings.cxx:968`) where the BIFF12 path maps *both* non-`none` values
onto it (`:270`). No corpus XLSX states either value, so that half follows the source and is
unmeasured. **ODS is not wired at all** — `style:print="… annotations …"` on the page layout plus
`office:annotation` on the cells would be the two halves, and no corpus spreadsheet asks for it
there. Worth doing for symmetry, not for a number.

One instrument note for the next agent: **LibreOffice's flat-ODS export drops cell annotations
entirely.** Measured on two documents whose notes demonstrably print — `office:annotation` appears
zero times in the `.fods` and twenty-four times in the `.ods` of the same workbook. So the
`--convert-to fods` trick, which has settled several questions on this track, is the wrong
instrument for anything about notes; use `--convert-to ods` and read `content.xml`.

### The `Tj`-splitting artefact costs this track three matches, not one

> **Corrected, and the correction is the more useful half.** The histogram below reproduces —
> our text layer really does have a ceiling the reference's does not — but on all three
> documents it was a *symptom of the in-cell hard break*, not the cause of the miss. Once the
> break was honoured, `Praktikastellen_…xls` went 2019 → **1828 of 1828**, `FY2021-AIP-grants.xlsx`
> 161982 → **156679 of 156662** and `STC_WebList.xlsx` 1354164 → **1297910 of 1293910**; all three
> now match, and the `Tj` split costs this track **nothing**. Nothing about the sink changed.
>
> Read it as the standing warning about a plausible signature: a concatenated cell is broken
> mid-token by the wrap, and mid-token fragments are exactly what an operator-granularity
> defect also produces. The histogram cannot tell the two apart, and neither of the two agents
> that ran it asked whether the long token should have been there at all. Compare the two
> renderings' *lines* before believing a token-length histogram.

The brief names `Praktikastellen_…xls` as the one gate miss and says the same artefact inflates
word counts wherever a workbook holds long tokens. Measured, with a token-length histogram of both
text layers — the signature is a hard ceiling on our side that the reference does not have:

| document | ours | reference | tokens over 28 chars |
|---|---|---|---|
| `Praktikastellen_…xls` (005) | max 33 | max 55 | 1 against 19 |
| `FY2021-AIP-grants.xlsx` (014) | max 37 | max 49 | 15 against 92 |
| `STC_WebList.xlsx` (018) | max 89 | max 107 | 734 against 1348 |

Those three are `words`-only failures on page-exact documents and are the same defect. The other
two word-gate failures in the four batches that came in below the brief are **not** this:
`CSA_CCM_v1.2.xls` (007) over-counts by 1227 with the same 36-character maximum on both sides, and
`SLSA_Directory_031423.xlsx` (008) is 187 words *short*, which splitting cannot cause.

### The next lead: a hard break inside a cell never reaches us

**`CSA_CCM_v1.2.xls` (007) is 13 pages against 13 and 17079 words against 15852**, and the excess
is spread evenly — every page is 50 to 110 words over, with one at 254. It is not a page-count
problem and it is not the `Tj` artefact.

What the two renderings draw is the difference. The reference puts `ME 2.1`, `ME 2.2`, `PO 9.5`
and `PO 9.6` on four lines of one cell; we draw `ME 2.1ME 2.2PO 9.5` as one string, which then
overruns its column and is broken **mid-token** by the layouter — `NIST SP800-5` / `3 R3 CA-2`,
`Clause 5` / `.1 g`, `L.1, L.2, L.7` / `, L.9, L.11`. That is where the extra tokens come from: a
dropped break makes one long string, and the wrap of a long string that has no spaces in it makes
several short ones.

**The file really does hold the breaks.** LibreOffice's own `.ods` conversion of this workbook
writes that cell as a single `table:table-cell` holding **four `<text:p>` elements**, and it holds
**1403 multi-paragraph cells** in all. Our extraction of the same workbook yields **zero** strings
containing a newline — checked on `ECA Sinters.xls` too, also zero — so the break is lost in the
BIFF reader rather than in the layout.

> **Disproved, and both halves of it.** The BIFF reader keeps every break. Counted on the
> same two exports rather than argued: LibreOffice's `.ods` of this workbook holds **578**
> multi-paragraph `table:table-cell` elements out of 1505 cells, and our own XHTML extraction
> of it holds **578** `<br/>` — the same number, cell for cell, and `<p>ME 2.1<br />ME 2.2<br
> />PO 9.5<br />PO 9.6</p>` is line 158 of it. `BiffRecordReader.ReadRawUnicodeString` appends
> U+000A like any other code point and nothing downstream removes it; `ContentTableCell`
> strips only a *trailing* newline. **1403 is not a count of multi-paragraph cells in that
> file** and no measurement in the tree reproduces it.
>
> This is render-comparison's rule 7 exactly — extraction is right and rendering is wrong, so
> the defect is in the value only rendering resolves. Everything below the line about
> `SheetTextLayout` stands; everything above it about the reader does not.

Two things to hold onto before starting:

- **`SheetTextLayout.Wrap` would drop them anyway**, even once the reader keeps them. Its first
  line shapes the whole text and returns a single line when it fits the column
  (`if (available <= Length.Zero || whole.Width <= available) return [whole]`), so a cell whose
  hard-broken text is *narrower* than its column is drawn on one line whatever its breaks say.
  `LineCount`, right beside it, splits on `\n` before wrapping — so the row height and the drawn
  lines are computed by two rules that disagree. Fix both or the symptom only moves.
- **Estimate the reach by rendering, not by grepping.** A byte scan of the `.xls` shared-string
  tables for `0a 00` flags 53 of the track's 171 documents, but a compressed BIFF8 string stores
  one byte per character, so that pattern is a false positive there and the figure is an upper
  bound. No corpus `.xlsx` writes `&#10;` at all, so the SpreadsheetML side of this is unmeasured
  and may not be affected. `ZenithAviation_AuctionList.xls` matches exactly at 6626 words while
  carrying the same byte pattern 158 times, which is the warning: the pattern is not the defect.

## The seventeenth sweep: the workbook's default font is not the one the chain names

Swept whole at `196774051` before anything was changed, 171 documents: **136 of 171, page error
113, 145 exact page counts**, batches 001 to 008 at 80/80 — the handover to the digit. After the
one change, **137 of 171, page error 112, 146 exact**, batches 001 to 008 unchanged at 80/80 and
`batch-009` 6/9 → 7/9. Two rows moved in 171 and neither regressed.

The fix is in `Paperless.Text` and is written up in `dotnet/TODO.batches.md`: `Helv` resolves to
DejaVu Sans, not to the Liberation Sans `VCL.xcu` names, because fontconfig's pre-match hook runs
before the substitution chain and always answers. What matters here is *why a spreadsheet felt it*
— a column width in SpreadsheetML is a count of digits of the workbook's default font, so a wrong
face is a wrong grid, and `airports_6.xlsx`'s columns came out at 111 twips a character unit
against the 127 LibreOffice's own flat-ODF export states for all eight of them. Column C was
12.6% narrow, wrapped to two lines where the reference draws one, and the extra line height
accumulated into an eighteenth page. **The handover's measurement reproduced and its explanation
did not**: it read as cumulative row height, and the drawn row pitch is 8.99 pt on both sides.

### `Company_Seniority_Date_Calculator.xlsx`: measured to a tdf#103516 nudge that does not fire

13 pages against 12, and the extra page is `Bulletin Clarification`, sheet 4 of 7. It states
`<pageSetUpPr fitToPage="1"/>` with `fitToWidth` absent (so 1) and `fitToHeight="0"`, on Legal
landscape with quarter-inch margins — 972 pt of printable width and 576 pt of printable height —
and a print area of `A1:Y49`, 25 columns of 963.2 twips.

Measured off both PDFs, page 8: the reference draws a 300-twip row at a pitch of **11.67 pt** and
we draw it at **12.0**, and one line of body text spans 323.01 pt there against 331.38 pt here.
Both say the same thing — **the reference prints this sheet at zoom 78 and we print it at 80** —
and at 80 the last row does not fit, so we start a thirteenth page holding one row and 35 words.

Zoom 80 is what a fit-to-width search gives: 25 × 963.2 = 24080 twips against 19440 of paper, and
80 is the largest whole percent that fits. **78 is what `tdf#103516` gives** — `CalcZoom`'s tail at
`sc/source/ui/view/printfun.cxx:2988` multiplies a width-only fit by 0.98 and keeps the result when
the *vertical* page count drops. `SheetPagination.CalcZoom` already implements that rule and it
does not fire here, so either its `before` or its `Count(nudged).Rows` disagrees with Calc's
`m_nPagesY`.

**What is not yet reconciled, stated plainly so the next reader does not trust the arithmetic.**
LibreOffice's own row heights for this sheet, out of its flat-ODF export, sum to **14750.6 twips**
over rows 1 to 48, and the print area's row 49 is empty and takes the default 300 more.
`AdjustPrintArea(false)` does *not* crop it — the `#i53558#` fuzziness at `printfun.cxx:717-727`
only crops when more than `23*42` empty rows follow the data, and one does not. With 49 rows the
band count is 2 at both 80 and 78, so on this reading Calc would revert the nudge and print at 80,
which it demonstrably does not. With 48 rows it is 2 at 80 and 1 at 78 and everything agrees. So
one of the three inputs — the row sum, the printable height, or which row the band ends at — is
not what it is assumed to be here, and that is the thing to measure first rather than adjusting
the nudge to fit. Our own row heights are within 35 twips of LibreOffice's across the whole sheet
(measured span by span off the two PDFs), so the row model is not the suspect.

## The sixteenth sweep: the attribute scan is asked per column, and it stops twice

Swept whole at `22ed440e0` before anything was changed, 171 documents: **134 of 171, page error
116, 143 exact page counts** — the brief's three headline figures to the digit, and batches 001 to
006 at full parity. `batch-007` and `batch-008` were each 9/10 and their single misses were the
two documents the handover named. Both were taken, and the round ends at **136 of 171, page error
113, 145 exact**, with batches 001 to 008 at full parity. Seven documents moved and every one
improved.

### An empty column's scan starts at its own last data row, not at the sheet's

`SheetDecorationArea.Extend` started every column's attribute scan at the *sheet's* last data row.
Calc asks the question per column and answers it per column: `ScColumn::GetLastVisibleAttr` passes
that column's own `GetLastDataPos()`, documented "always including notes, **0 if none**"
(`sc/inc/column.hxx:892-897`), into `ScAttrArray::GetLastVisibleAttr`.

Measured on `Computer and Software Services_50 State Comparison.xlsx`. Columns I to O carry a
solid fill on all 129 rows and hold no data; the sheet's data stops at row 42. Below row 42 the
fill is one run of 112 visually equal rows, which is past `SC_VISATTR_STOP` and ends the scan with
nothing found — so the old start reached nothing at all. The run Calc actually takes is the
**header row above the data**: `nAttrStartRow` is clamped to `nLastData + 1` (`attarray.cxx:1961`)
and a run that is nothing but row 1 therefore measures **zero** rows, which is inside the limit.
The print area stopped at column H and Calc's reaches O, a whole third column band.
**26 pages now against 26, 2818 words against 2819.**

Three details of the port are load-bearing and each was checked by breaking it:

- the start is the column's own last data row, and a column with no data uses Calc's **0**, not
  "no data at all" — one row apart, and worth a column of print area;
- the row at the start is *inside* the scan, because Calc processes the run containing
  `nLastData` rather than the run after it;
- a run's length is measured from `nLastData + 1` however far above that the run begins.

### The same change cost nine pages elsewhere, and the sideways stop is why

The whole track was swept again and **exactly four rows moved**. Two were the reference's own word
count drifting — `PBN Matrix NAAs (V01).xlsx` 5554 → 5557 and `SIL_TDB648.xlsx` 7679 → 7678, both
in the LibreOffice column. The other two were the target above, and
`environment-edb-docs-edb-emissions-databank.xls` going **195 pages to 204**, out of `match`.

Probed rather than guessed: its `ICAO databank` sheet holds data to column 104 and per-cell
formatting to column 228, and the printed range came back `0..228` where it had been `0..104`.
124 empty ruled columns that the old start had never been able to reach.

Calc drops them by `SC_COLUMNS_STOP` = 30 (`table1.cxx:655`, `737-757`), the sideways twin of
`SC_VISATTR_STOP` and previously not implemented at all. Walking right from the last data column
it groups columns that are visually equal **over every row**, and the first group of thirty or
more ends the block before it; it then walks back over any column whose own scan found nothing.
The cut is sideways only — `nMaxY` keeps what the dropped columns reached. On an ordinary sheet
the run past the last formatted column is unbounded and equal to itself, so the walk stops exactly
where it did before and nothing changes; that is why this had never been needed.

The document is back to 195 pages and 65593 words.

### `dragon-175066A.xlsx` is one twip of digit width, and the twip is already recorded

The other named lead, **14 pages against 13 with the words exact**. Measured rather than inferred,
off both PDFs with `pdftotext -bbox`:

| | reference | ours |
| --- | --- | --- |
| column B's left edge | 80.528 pt | 81.153 pt |
| column C's left edge | 357.0 pt | 363.1 pt |
| column B's drawn width | 276.47 pt | 281.95 pt |

Column B is `10.3783in` = 747.24 pt in LibreOffice's own flat-ODF export of the file, so the
scales are **0.3700 and 0.3773** — LibreOffice prints at zoom **37** and we print at **38**. The
sheet is `<pageSetUpPr fitToPage="1"/>` with `fitToWidth` 1 and `fitToHeight` 0, and `CalcZoom`
bisects on **integer** percentages, so the exact fit ratio only has to cross a whole percent for
the answer to move by 2.7%.

It crosses it because our columns are 0.7% narrower. The workbook's default font is 宋体 at 11 pt,
which fontconfig resolves to DejaVu Sans on this machine — LibreOffice's too, since its export
gives exactly 140.0 twips per character unit for all three columns. Our own exact metric is
**139.9701**, and `SheetFonts.DigitWidthTwips` truncates it to 139.

That truncation is not a bug found here; it is the rule that section already documents, chosen
because Carlito 11 pt measures 111.50 and LibreOffice writes 111, and Carlito is the default of 65
of the 171 corpus spreadsheets. What is new is **what the twip costs**: forcing 140 for this
workbook alone renders it at **13 pages and 8143 words against 8142**, which is a match. A one-twip
column error is normally invisible; on a fit-to-width sheet the integer zoom amplifies it into a
whole page.

The experiment was then run rather than left for the next round. Every one of the nine measured
faces is satisfied by truncating unless the fraction carries past a threshold in (0.64, 0.70] —
Carlito 12 pt is 121.64 → 121 and DejaVu Sans 12 pt is 152.70 → 153 — and a threshold fitted to
nine points, two of which are the only ones that discriminate, is a fudge until a corpus says
otherwise. So the corpus was asked, and cheaply: **the reference side does not move for a
Paperless-only change**, so rendering our side alone over all 171 and joining the reference column
of an existing sweep answers it in one pass. The harness was validated first by reproducing that
sweep's 135/114/144 with zero rows differing.

At 0.67 the whole track goes **135 → 136, page error 114 → 113, page-exact 144 → 145**, and *six
documents moved with every one improving*: `dragon-175066A.xlsx` 14 pages to 13 and into parity,
and five word counts closer — 345 → 344 of 344, 799 → 798 of 798, 4176 → 4184 of 4184,
6257 → 6246 of 6245, 73991 → 73750 of 73542, four of them exact. No page count moved anywhere
else. A confirming full sweep against a freshly generated reference gave the same three numbers.

The constant is still fitted and `DigitWidthCarry`'s remarks say so in those words; what changed
is that it is now fitted to two independent bodies of evidence rather than to two data points.

## The fifteenth sweep: the break was in the drawing, and it was worth five documents

Swept whole at `1aefcdfdb`, 171 documents, before and after. The baseline reproduced the brief's
three headline figures to the digit — **129 of 171, page error 116, 143 exact page counts** — and
after the fix: **134 of 171, page error 116, 143 exact.**

| | before | after |
| --- | --- | --- |
| documents matching | 129 | **134** |
| documents with an exactly correct page count | 143 | 143 |
| total absolute page error | 116 | 116 |
| **total absolute word error** | 107 780 | **44 496** |

**No page count moved anywhere on the track**, which is the tell that the diagnosis was right:
`LineCount` had always split on the break, so the row heights were already correct and only the
drawing was not. Five documents went to `match` and none left it; the one other verdict change is
`dragon-175066A.xlsx`, which went from failing pages *and* words to failing pages alone at 8143
words against 8142.

Fourteen documents' word error rose, twelve of them by five or fewer; the two that are not are
`CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` (a 1227-word shortfall became 1316, still a `words`
failure) and `afn-afn-20250801-fy25-jan25-mar25.xlsx` (407 to 449 on 73 542, still a match).

### What it was

Not the reader. `SheetTextLayout.Wrap` shaped the cell's whole text and returned it as one line
whenever that fitted the column — `if (available <= Length.Zero || whole.Width <= available)
return [whole]` — so a break never reached the line breaker unless the concatenation happened to
overrun. `LineCount` beside it split on the break first, so the two disagreed by exactly the
cells that fitted.

Calc does not have the choice to make: a BIFF or SpreadsheetML string holding U+000A becomes a
`CELLTYPE_EDIT` cell (`XclImpStringHelper::SetToDocument`,
`sc/source/filter/excel/xihelper.cxx:245-258`; `SheetDataBuffer::setStringCell`,
`sc/source/filter/oox/sheetdatabuffer.cxx:120-135`, which every such string reaches because
`RichString::extractPlainString` refuses one at `richstring.cxx:375`), and
`ScOutputData::LayoutStringsImpl` sends every edit cell to `DrawEdit` without asking anything
else about it (`output2.cxx:1711-1712`). One paragraph is one line; the column width decides
where a paragraph is broken and never whether it is.

Three changes, all in `SheetTextLayout`, all narrow:

- the shortcut asks whether the text holds a break before taking itself, so a cell with none
  measures and draws exactly as it did;
- a line's shaped range backs off over the break that ends it — Writer's break portion is "zero
  width, and no glyph", and keeping it measured the character's advance into a centred line and
  put a U+000A in the text layer;
- an empty paragraph is shaped from its break for the metrics and then emptied, so it takes its
  pitch without drawing a `.notdef` box.

`sheet-cell-hard-break.fods` and `SheetHardBreakTests` are the fixture and the ten tests, every
asserted figure read out of LibreOffice 24.2.7.2's own PDF of the fixture with `pdftotext -bbox`.
Each half was verified by reintroducing it: restoring the unconditional shortcut fails 4 of the
10, restoring the shaped range to the line's full end fails the same 4, and dropping only the
empty-paragraph line fails exactly 1.

### One case is deliberately not implemented, and it is ODF's alone

A cell that holds a break and does **not** wrap. ODF's importer makes a multi-paragraph edit cell
whatever the wrap option says, so LibreOffice draws three lines for row 2 of the fixture; BIFF and
SpreadsheetML both fold the same content onto one paragraph (`SetSingleLine(!GetLineBreak())`,
`xihelper.cxx:250-256`; `bSingleLine = !mbWrapText`, `sheetdatabuffer.cxx:120-135`). The sheets
corpus is entirely `.xls` and `.xlsx`, so the one line we draw is what its two importers ask for.
`ANonWrappingCellStillLosesItsBreaks` states the gap rather than blessing it, and is the
assertion to delete when the ODF side is done.

### The next lead: an empty column's attribute scan starts from its own last data row

`Computer and Software Services_50 State Comparison.xlsx` (008) is **24 pages against 26** with
the words all but matching, 2816 against 2819. The two extra pages are pages 5 and 6 of the
reference and their entire text is `5` and `6` — the footer's page number on a blank body. The
sheet is `pageOrder="downThenOver"` over two row bands, so those two are a **third column band**
that we do not produce at all: our print area stops at column H and LibreOffice's reaches O.

Columns I to O of that sheet hold **129 rows of cells that carry a visible fill and no data
whatever**. `SheetDecorationArea.Extend` skips every row at or before the *sheet's* last data row
— 42 here — and then applies `SC_VISATTR_STOP`, so it sees one run of 87 equal rows, gives up,
and never widens. Calc asks the question per column: `ScColumn::GetLastVisibleAttr`
(`sc/inc/column.hxx:892-897`) passes `GetLastDataPos()`, **that column's own** last data row,
"0 if none", into `ScAttrArray::GetLastVisibleAttr` (`attarray.cxx:1922`). For a column with no
data the scan therefore starts at the top of the sheet rather than 43 rows down, and the run
arithmetic that follows is a different sum.

That difference is stated and cited; it is **not yet measured to be the cause here**, because the
run lengths still have to come out under 84 for Calc to keep the columns and I did not get as far
as reproducing its `IsVisibleEqual` grouping. Start by making that one change and rendering this
document: it is a one-line change to which row the scan starts from, and the whole track has to be
swept behind it because `SheetDecorationArea` decides the print area of every sheet.

## What the ninth sweep found: a row is measured on a device Calc never draws with

Swept whole at `6b6d54d37`: **125 of 171**, page error 192, 134 exact page counts — the briefed
figures to the digit. After the fix below, at the same commit plus it: **132 of 171, page error
119, 141 exact**. Thirteen documents improved and none regressed; batches 001–005 stay at 50/50.

### The instrument, first, because it is what made the rest quick

Export the workbook to flat ODS and read `style:row-height` out of it. That is LibreOffice's own
recomputed height for every row, so ours can be joined against it row by row — 7909 comparable
rows on `TK-Syllabus-Comparison-Document-v2.xlsx`. The scripts and a description of each are in
`scratchpad/sheets-ad4b/`; `RowHeightProbe.cs.txt` there is the test class that dumps our side,
kept out of the test project because it no-ops without an environment variable.

It reframed the problem in one run. **7133 of 7909 rows were already exact and every one of the
763 that were not was short by a whole number of lines** — one line is 18 device pixels at 11 pt,
268.7 twips, and LibreOffice's own heights are 300.0, 566.9, 835.1, 1103.8 and 1373.0, which are
exactly `(lines × 18 + 2) / 0.067`. So the vertical model was right to the twip and the *line
count* was wrong. Column widths were ruled out the same way: the export gives 490.9, 5834.9,
1649.2, 1632.8 and 6215.8 twips where we had 491, 5835, 1649, 1633 and 6216.

### Two quantisations, both horizontal, both absent

**The em is rounded to whole device pixels before a single advance is measured.**
`ScColumn::GetNeededSize` formats through a `ScFieldEditEngine` whose reference device is a
headless 96 dpi `VirtualDevice` (`column2.cxx:410-424`), and a device instantiates a font at an
integer pixel size. Eleven point is 15 pixels rather than 14.667, so its advances come out 2.3%
**wide**; ten point is 13 rather than 13.333 and comes out 2.5% **narrow**; twelve point is 16
exactly and is unchanged. `MetricGrid.ToEmSize` is that rounding, and it is the same grid the
ascent and descent already went through.

**The paper is narrower than the column by the document's print-to-screen factor.**
`ScSizeDeviceProvider` derives both resolutions from its virtual device and then divides **one**
of them: `nPPTX /= rDocSh.GetOutputFactor()` (`sc/source/ui/docshell/sizedev.cxx:52`). `nPPTY` is
left alone. And `nPPTX` is exactly what the column width is multiplied by to get the EditEngine's
paper (`nDocWidth = GetOriginalWidth(nCol) * nPPTX`, `column2.cxx:463`). That asymmetry is the
whole shape of the symptom — the pitch was exact and the width was 6% out.
`ScDocShell::CalcOutputFactor` (`docsh3.cxx:380-426`) sets the factor once per document, before
the import, from one fixed 72-character test string measured on two devices, so it is a property
of the machine and the application's default cell font rather than of the workbook. **1.0345
here**, alongside the 96 dpi and the 0.067 and fitted the same way.

Two more corrections came with it, both small and both real: the paper is turned back into a
length by `pDev->PixelToLogic` at the device's true resolution — 15 twips a pixel — and not by
dividing by the rounded 0.067, which is only how the *height* comes back; and the cell's
left/right margins come off in pixels computed from the divided `nPPTX`.

### Measured against the binary, not derived

The derivation would have been wrong. Reading `CalcOutputFactor` alone gives 1.005, because in
headless both of the devices it compares are 96 dpi virtual devices; the binary says 1.0345.
Five hand-authored flat-ODS probes, each holding unbreakable strings against 52 column widths and
round-tripped through `soffice` so its own recomputed heights say where it broke each line:

| probe | rows | exact |
|---|---|---|
| Calibri 11 pt, 8 strings × 52 widths | 416 | 416 |
| Calibri 12 pt | 416 | 416 |
| Calibri 10 pt | 416 | 416 |
| Liberation Sans 10 pt | 416 | 416 |
| Calibri 8/10/11/12/14 pt, one word per row | 450 | 450 |

2114 rows, every one to the twip, on one constant. Fitting the same constant independently
against the TK document's 7909 rows lands on 1.0345 as well, and takes it from 7079 exact rows to
7804.

**It is the measuring device and not the face.** A PDF of the same strings has LibreOffice
drawing Carlito at our advances — 206.95 pt against our 207.20 for a 42-character word, 394.48
against 395.00 for 42 M's, 105.78 against 106.05 for 42 i's. Only the row it reserves disagrees.
`sheet-row-height-device.fods` and `SheetRowHeightDeviceTests` hold the assertions; each pair of
its sheets sits fifty twips either side of a break, and at ten point the two quantisations act
against each other, so an implementation carrying one alone puts a pair on the wrong side.

### What is left on that document

74 rows short and 31 tall out of 7909, almost all by exactly one line, and undiagnosed. The
next thing to look at is the rich cells: the residue is concentrated in cells that carry both a
hard break and more than one format, and `SheetTextLayout.RichLineRanges` hands the whole text
to one `ParagraphLayouter.Layout` call where `LineCount` splits paragraphs itself first.

## What the eighth sheets sweep found: a merge has two axes, and a shape has a face

Swept whole at `7049756d9`: **125 of 171**, page error 222, exact page counts 134 — the briefed
figures to the digit, for the second handover running.

### A merged block reaches back on two axes, and only one was walked

`DrawCoveredMerge` walked left to a merge's origin and never up, so a block anchored on the page
above vanished from the page its tail falls on. It is one function in Calc, not two:
`ScOutputData::GetMergeOrigin` (`sc/source/ui/view/output2.cxx:953`) walks left while the position
is `bHOverlapped` (`:989`) and then up while it is `bVOverlapped` (`:1008`), both gated by the same
`bDoMerge` — `bIsLeft = (nX == mnVisX1)` for a horizontally overlapped cell, `bIsTop =
(nY == mnVisY1)` for a vertically overlapped one, and both together for a cell covered on both
axes (`:958-983`). `GetOutputArea` subtracts the rows between (`:1237-1254`), which puts the anchor
above the top of the paper and leaves the tail on it.

**The previous round's diagnosis of `RegChangeReport.xlsx` was right about the row and wrong about
the cell.** Row 24 really is 12.75 pt and `customHeight` — that is why a probe built to that shape
rendered on one page — but the 3 278-character description in it is `B24:B58`, a merge **thirty-five
rows tall**, and `B59:B84`, `B85:B97`, `B103:B112` and `B113:B135` are four more of the same. The
row was never the unit; the block was.

Measured on the document: **2314 extractable words against 3137, and 3060 now**, its pages 2, 3 and
4 going from 5, 294 and 249 words to 505, 446 and 343 against the reference's 506, 434 and 344.
Page 6 is the residue and is a different question — its block's tail lands there because the
reference's page 6 starts one row earlier than ours, so nothing on our page 6 is covered by it.

Whole-track before and after, 171 documents each time: **no page count and no verdict changed**;
twelve documents' word counts moved, eight of them closer, one to exact
(`Aircraft_Database.xlsx` 16504 → 16520 of 16520), and three further by 8, 15 and 1064 words on
totals of 2372, 68113 and 1 293 910 — all three still matches. `features/sheet-merge-down-break.fods`
states both halves and was checked against LibreOffice 24.2.7.2's own PDF for it.

### A shape's text is set in the face its runs name

`SheetShapeRun` carried a size and no typeface, and `SheetShapePainter` shaped every box in the
furniture face. The face is not only the ink: it sets the line height, it sets the advance widths
the wrap is decided by, and it sets the ascent the baseline sits at, so a body drawn in one face
and measured in another breaks every line in a place its own metrics did not pick.

**`+mn-lt` is the common case and is not a family name.** It means "the theme's minor Latin face"
and is resolved through `a:fontScheme` (`Theme::resolveFont`, `oox/source/drawingml/theme.cxx:71`);
handed literally to a resolver it asks for a family that exists nowhere. `DrawingFontScheme` in
`Paperless.Ooxml` already did this for the other two families, so the fix is a read and a thread
rather than new machinery.

Measured on `SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.xlsx`, whose methodology note is one
text box stating `+mn-lt` against a Calibri theme: its drawn line pitch was **12.65 pt against the
reference's 13.43 and is 13.43 now**, and its extractable words went **479 to 519 of 550**. Seven of
the track's 109 package spreadsheets state a typeface on shape text — four `+mn-lt`, two Arial, one
Webdings — and all seven were rendered before and after: SSRO and `SIL_TDB648.xlsx` (7668 → 7680 of
7678) moved closer, `arp-sop-300-Exhibit-A-Table-Templates.xlsx` moved 2 words further on 3740, and
four did not change at all. **None changed verdict**, which is the honest headline: this is a
correctness fix with no match to show for it on this corpus.

`features/sheet-shape-theme-font.xlsx` is LibreOffice's own `sheet-shape-text.xlsx` with the
theme's minor Latin face changed to Calibri and two text boxes in place of one, the first naming
its face indirectly and the second naming Times New Roman. LibreOffice's own PDF for it embeds
`Carlito-Regular`, `LiberationSerif` and `LiberationSans` and wraps each box after "the" and "run";
we now produce the same three faces and the same two breaks.

### `orbus_togaf_tool_csq.xls`: the reference invents a sheet, and it is 42 pages of it

**33 pages against 75, and the 42 missing ones are a sheet the file does not contain.** The
reference's pages 34 to 75 are headed `DPCache`; the workbook's BOUNDSHEET records name six sheets
— `Instructions`, `Vendor Product Information`, `KTCs & TCRs`, `Evaluation Summary`,
`Audit Checklist`, `Additional Features ` — and none of them is it.

`XclImpPivotCache::ReadPivotCacheStream` creates it: a pivot cache whose source is an external or
deleted sheet has no range to point at, so the filter calls `rDoc.MakeTable`, names the new sheet
`DPCache` and redirects the cache's source range into it
(`sc/source/filter/excel/xipivot.cxx:717-733`). LibreOffice then prints it, and it is 42 pages of
cache rows. Nothing in the file asks for those pages and nothing we could read would produce them.

This is worth stating precisely because it is **19% of the track's whole page error on one
document**, and because "the reference is wrong" needs a higher bar than it usually gets: the
mechanism is named, the sheet is demonstrably absent from the file, and the reach was measured
rather than assumed — `DPCache` appears in **exactly one** of the track's 171 reference PDFs.

### Fixed: a legacy workbook's pictures never reached its sheets

`XlsDrawingCollector` emitted a drawing only for a shape carrying `TXO` text — it walked a picture
shape, consumed its `OBJ` record and dropped it — and `MSODRAWINGGROUP` (0x00EB) was declared in
`BiffRecords` and read nowhere. So no `.xls` in any corpus had ever had a picture on its page.

Excel keeps the `OfficeArtDggContainer` **inline in the workbook globals**, where Word keeps it in
the table stream and PowerPoint in a `PPDrawingGroup`, so `EscherBlips.Read` already knew the
structure and needed only the bytes and no delay stream. A shape names its picture with `pib`
(property 260), one-based into that store.

Reach, measured by walking the globals substream rather than by grepping: **8 of the sheets track's
61 `.xls` carry an `OfficeArtBStoreContainer`**, 27 `msofbtBSE` between them —
`TICAPCapability_Final.xls` 12, `apron-area.xls` 6, `INDEX_Digital_Transformation_Toolkits.xls` 4,
and five with one each.

Two things worth carrying forward:

- **Most of them are metafiles.** Every blip in `TICAPCapability_Final.xls` and the one every one of
  `apron-area.xls`'s seven shapes points at are `0xF01A` (EMF), so they draw as vector content and a
  count of `/Subtype /Image` in the output PDF sees none of them. The earlier census that read
  `apron-area.xls` as "0 images against 1670" was counting placements of something the two sides
  represent differently, not a picture we failed to draw — its page-1 unaccounted ink is 1.09% before
  the fix and 1.00% after, and it was a full match throughout.
- **A missing picture costs pages, not only ink.** `SheetEmptyPages.TouchedByADrawing` keeps a page
  holding no cells but holding a drawing.

### Vertical clipping: the rule is real, the document that named it is not explained

`SheetTextLayout` says "the clip never cuts the text vertically" and cites
`output2.cxx:2093`. **Half of that is wrong.** Calc asks one question in four places with the same
three lines —

```
if ( meType != OUTTYPE_PRINTER ||
     ( mpDoc->GetRowFlags( nCellY, mnTab ) & CRFlags::ManualSize ) ||
     ( aVars.HasCondHeight() ) )
    bVClip = true;
```

— at `output2.cxx:2104`, `:3256`, `:4132` and `:4419`, and the comment beside the third says why:
*"Don't clip for text height when printing rows with optimal height."* A row whose file merely
states `ht` is Calc's own measurement of its content, so nothing can overflow it and clipping would
only cut its own answer. **A row the user sized is a statement, and a wrapping cell taller than it
loses everything past its bottom edge.** The slack is one reference-device pixel —
`nEngineHeight >= aCellSize.Height() + aRefOne.Height()` (`:3248`), 15 twips at 96 dpi.

The second half matters as much and is easy to miss: `EnableSkipOutsideFormat` is turned on for
every top- or standard-aligned cell (`:3115`) and the engine is given the *cell* as its paper, so a
line whose top falls past the bottom is **never formatted at all**. That is what makes the
difference visible to `wc -w` rather than only to the eye — a clipped glyph is still an extractable
word and a glyph never drawn is not.

**Both were implemented, built clean, and measured to move nothing.**
`Application_Compliance_Checklist_5_Apr_2021.xlsx` — 18 pages against 14 and **26 353 words against
17 718**, the reference drawing its checklist in six content pages where we take ten — came back
byte-for-byte the same page and word counts with the clip in, and again with the skip on top. So
the rule is right and the diagnosis of this document is not: its overflowing cells are not reaching
the branch. Ruled out along the way:

- **The row flags are correct.** Probed: `App. Compliance Checklist` rows 29 onward arrive as
  manual-height at 450–765 twips, matching the file's `customHeight` on 715 of 858 `ht` rows.
- **It is not hidden sheets.** Five of its nine are `hidden` or `veryHidden` and
  `SpreadsheetPages.IsPrinted` already drops them.

The next thing to check is whether those cells wrap at all: a cell that does not wrap is one long
line taking the horizontal path, where Calc *shortens* the string — dropping characters — and we
clip it instead, which keeps every glyph in the text layer. That is the same defect in the other
axis and would explain an over-count with the row heights agreeing.

### `INDEX_Digital_Transformation_Toolkits.xls`: six pages of pictures we never draw

18 pages against 24 with the words matching **exactly** (1982 against 1982), which reads as six
blank pages the reference keeps and is not. The last sheet's two columns are 0.2374 in and
7.0555 in against a 7.1 in printable width, so they do not fit side by side and the sheet takes two
column bands; the reference's pages 13 to 18 are the narrow first band, and they carry **30, 42, 38,
32, 36 and 16 images each** — the small icons anchored at `svg:x="0in"`. `HasAnyDraw` keeps them.

Our `SheetEmptyPages.TouchedByADrawing` would keep them too. It does not, because **we draw no
images from this workbook at all**: `pdfimages -list` reports images on every one of the reference's
24 pages and none anywhere in ours. So this is not an empty-page rule at all — it is the Escher
picture path on this one `.xls`, and the six pages are a symptom.

**Half diagnosed, half fixed.** With the picture path in (see above) this workbook now draws 74
image objects and renders **20 pages against 24**, so two of the six came back and four did not. The
four remaining are still an empty-page question and are now worth asking as one: the icons are
anchored at `svg:x="0in"`, so their rectangles begin exactly on the band's left edge, and
`TouchedByADrawing` compares a drawing's bounds against the block's with `>=`/`<=` on both sides.
Whether every icon on those four pages is inside the *printed* range at all is the thing to measure
next — `HasAnyDraw` walks the whole drawing page rather than the objects anchored in the range.

### `TK-Syllabus-Comparison-Document-v2.xlsx`: rows 10.4% too tall, lines exactly right

**Settled by the ninth sweep — see the top of this file.** Keep the measurement below and drop
its last paragraph's conclusion: `WrappedHeight`'s *vertical* quantisation was never the problem.
The row heights reserved and the lines drawn disagreed because the width the text was broken at
was measured on a device 6% narrower than the column, and the document is now an exact match at
1235 pages.

**1314 pages against 1235 — 36% of the track's page error on one document**, and its sibling
`tk-syllabus-comparison-document-v5.xlsx` is 849 against 855 in the other direction. The words agree
(258 720 against 258 369), so this is pagination alone.

The first divergence is page 6 and it is cumulative: on page 5 both renderings agree to 0.02 pt down
to the fifth row and drift from there. What is drifting is measured and narrow:

- **The wrapped lines are identical.** Between the same two markers on page 5 both draw 25 lines,
  breaking at the same words, at a pitch of **13.45 pt in both**.
- **The rows those lines sit in are not.** Three consecutive rows measure 95.5, 135.8 and 122.3 pt
  in the reference and 105.0, 149.9 and 135.0 pt here — **1.0995, 1.1038 and 1.1038**.

So the row height reserved and the line height drawn disagree with each other, by a ratio that is
the same for rows of three different line counts. Both renderings set the body in Carlito and both
PDFs embed it, so this is not a substitution. The sheets state `ht` on 634 to 719 rows **without**
`customHeight`, which is the case `SheetOptimalRowHeights` recomputes — so the number in question is
ours, and `WrappedHeight`'s pixel quantisation (`MetricGrid.ToPixels`, 96 dpi) is where to look.
That figure was fitted to thirty probe rows in one face; Carlito's `OS/2` 1946/558 over 2048 units
is not among them. Note the caveat that comes with it: the row here is 21 columns wide and its
height is set by whichever cell needs most, so the tallest cell was not the one whose lines were
counted above. Confirm which column sets each of the three rows before trusting the ratio as a
per-line figure.

## What the seventh sheets sweep found: three ways a cell escapes `DrawStrings`

Swept whole at `e2e0bdee3`: **122 of 171**, page error 222, exact page counts 134 — the briefed
figures to the digit, which is the first time this track's handover has reproduced exactly.

All three fixes this round are the same shape, and it is worth stating as one finding rather than
three. **`ScOutputData::DrawStrings` is not the only thing that draws a cell**, and the cells that
leave it behave differently in a way the gate can see: the plain path *shortens* a string that
will not fit, and the EditEngine path clips it instead. Every defect below is a cell taking the
EditEngine path in Calc and the plain one here.

### A no-break space sends a cell to the EditEngine

`ScDrawStringsVars::HasEditCharacters` (`sc/source/ui/view/output2.cxx:823-847`) is consulted at
`output2.cxx:1812`, before anything about the output area is decided. Seven code points force it —
`CHAR_NBSP`, `CHAR_SHY`, `CHAR_ZWSP`, the two bidi marks, `CHAR_NBHY` and `CHAR_WJ` — and
`DrawStrings` then skips the cell entirely for `DrawEditStandard`, which sets a clip of the cell
and draws the whole string behind it. The no-break space alone is excused by a repeat directive,
which is tdf#122676 stated in the comment beside the case: it is a thousands separator in half of
Europe.

Measured on `esurf-12-135-2024-t01.xlsx`, whose date column is written `28<NBSP>Oct<NBSP>2012` in a
column one character too narrow: the reference's text layer holds all eleven characters on eighteen
of its twenty-three rows and ours held ten. **113 extractable words against 124, and 123 now.**
The tell in the reference PDF is unmistakable once seen — those cells are drawn in a second,
`/P<</MCID n>>BDC`-tagged pass at the end of the page's content stream, each behind a clip one row
tall, while every other cell is drawn inline behind a clip the whole band's height.

### A hyperlink replaces a cell's content with one field, and a field never wraps

Calc does not decorate a cell with a hyperlink. `WorksheetGlobals::insertHyperlink`
(`sc/source/filter/oox/worksheethelper.cxx:1062-1080`) and `lclInsertUrl`
(`sc/source/filter/excel/xicontent.cxx:157-215`) both replace a **string or edit** cell's content
with a single `SvxURLField` whose representation is the string it held, and leave every other cell
type carrying a plain `ATTR_HYPERLINK` that changes nothing. A field is one indivisible portion:
"Fields aren't wrapped, so clipping is enabled to prevent a field from being drawn beyond the cell
size" (`DrawEditParam::readCellContent`, `output2.cxx:2560-2567`, consulted at `:3239`).

That reaches pagination and not only the word count, because a URL is exactly the string a line
breaker splits at every solidus: a wrapping column of links measured four or five lines a row
instead of one, and a row height is a page count. `SheetLayout.HyperlinkRanges` is filled by all
four readers — `hyperlinks` in a SpreadsheetML worksheet, BIFF8 `HLINK`, BIFF12 `BrtHLink`, ODF
`text:a`. Only *whether* a URL results is read, never what it is; for BIFF that is the flag word
(`EXC_HLINK_BODY|MARK|UNC`) rather than a walk of the monikers, because a link resolving to an
empty string never reaches a cell at all (`XclImpHyperlink::ReadHlink`'s `if (!aString.isEmpty())`).

Measured on `Published_Issuances_2024.xlsx`, whose last column is a wrapping column of links:
**482 extractable words against 458, and 458 now. 33 of the track's 171 documents carry cell
hyperlinks**; the other 32 were rendered before and after and none changed verdict.

### A clipped string's surviving glyphs do not move

Not an EditEngine case, but found by the same reading. `Shorten` drops the characters a clipped
cell cannot show — Calc does the same, "if the string is clipped, make it shorter for better
performance since drawing by HarfBuzz is quite expensive" (`output2.cxx:2202`), and it is visible
rather than merely faster because the PDF holds what was drawn. A right-aligned string loses its
*head*, and the glyphs that remain were already standing at `right − margin − shortened`, which is
what `Horizontal` returns when handed the shortened run's own width. It was then shifted right
again by the width dropped.

Measured on `RVSM_Non_approved_list_2025_84c0b3f4ac.xlsx`, whose left-clipped dates ran flush into
the next column with no gap: the reference draws `2-10-2022` from 51.82 pt to 97.50 pt and we drew
it from 57.36 pt — **5.54 pt right, the width of the one digit dropped.** `pdftotext` then read
each date and its neighbour as one token: **419 extractable words against 445, and 445 now.**

Fixtures: `features/sheet-edit-characters.fods`, `features/sheet-hyperlink-field.xlsx` and
`features/sheet-clipped-alignment.fods`, each checked against LibreOffice 24.2.7.2's own PDF and
each carrying its negative half — plain text in the same column that must still be shortened, the
same URL without a link on it that must still wrap into five lines, and a left-aligned cell that
must still lose its tail. All five tests were confirmed to fail with their defect put back.

### `RegChangeReport.xlsx`: the lead is measured further and still not caused

The previous round recorded page 2 as 5 words against 507 and refuted the row-split theory from
`UpdatePageBreaks`. The measurement reproduces (2302 against 3137, and 2314 now), and this is what
the reference's geometry says, which narrows it without settling it.

Row 24 holds the 3 264-character "Liability Management Framework" description, wraps to about 57
lines, and its row is **12.75 pt tall and marked `customHeight`** — LibreOffice's own flat-ODF
export writes it `style:use-optimal-row-height="false"`. The reference draws that one cell on
**three consecutive pages**, each time at a different vertical offset: page 1 from the row's own
position near the bottom, running off the paper at y=774 of 792; page 2 with line 1 just above the
sheet at y=−6, so lines 2 to 57 land; page 3 from line ~41 at y=1. Nothing on page 2 but that cell.
We draw it once, on page 1, and leave page 2 holding only the security-classification band.

**A probe does not reproduce it.** A flat-ODS sheet with one 12.75 pt manual-height wrapping cell
holding 420 words, followed by eleven ordinary rows, renders on **one** page with the text cut off
after four lines — so "a manual-height row's overflow is redrawn on the following pages" is *not*
the rule as stated. Something about this document puts the cell on three pages and the probe's on
one; whatever that is, it is worth finding, because it is 835 words on a single document and the
same shape as the horizontal lead-in already implemented.

### Two diagnosed and not fixed

- **`Hazard Analysis Template.xls` prints its cell notes on a page of their own.** 2 pages against
  3, 461 words against 682, and the missing page is a list of `D1:`, `F2:`, `H2:` … labels
  followed by note text, which is Excel's "Comments: at end of sheet". LibreOffice's flat-ODS
  export of the file says `style:print="annotations …"`, and `XclImpPageSettings::ReadSetup` reads
  it from `EXC_SETUP_PRINTNOTES` (`sc/source/filter/excel/xipage.cxx:84`) into `ATTR_PAGE_NOTES`
  (`:257`). Nothing here reads cell comments for layout in any format, so this is a feature rather
  than a wiring change, and no other corpus document was shown to need it.
- **A sheet shape's text is drawn in one face whatever it states**, which `SheetShapeText` already
  records as a limitation. *(Fixed in the eighth round — see above. The measurement below stands and
  the pitch is 13.43 pt on both sides now; the document still fails the word gate, at 519 of 550.)*
  Measured on `SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.xlsx`,
  whose 806 pt notes box states `<a:latin typeface="+mn-lt"/>` against a theme whose minor Latin
  face is Calibri: the reference's line pitch is 13.5 pt and ours 12.5, which inverts to Carlito's
  line box against Liberation Sans's. Every line therefore breaks in a different place and the
  page-edge clip cuts a different amount — 479 words against 550. The reach is small and was
  measured rather than assumed: **7 of the track's 109 XLSX have shape text naming a typeface**,
  three of them through `+mn-lt`.

## What the fifth sheets sweep found

`sheets/batch-003` measured **8/10** at `86ce2dc9b`, reproducing the briefed baseline exactly, and
is **10/10** after this round with `batch-001` and `batch-002` re-proved at 10/10 each. Both
failures were `.xlsx` and neither was in the reader: the two documents' cells extract correctly and
the defects are in what layout does with cells and drawings that hold nothing.

**"A cell exists here" was standing in for "there is content here", and the two differ by a whole
column band.** `SheetEmptyPages` dropped a page only when the block held no `ContentTableCell` at
all. `ScTable::IsBlockEmpty` (`sc/source/core/data/table2.cxx:2432-2452`) asks each column for
`IsEmptyData` — the cell *store*, which a cell carrying nothing but a style index is not in; its
attributes reach the question only through the separate `HasAttrFlags::Lines` test that follows.
Every format writes such cells in quantity: SpreadsheetML a bare `<c r="I1" s="13"/>`, BIFF a
`MULBLANK` across a whole formatted row. Measured on
`Bulletin-37-Appendix-2-immediate-detriment-data-request.xlsx`, whose columns I to P carry a style
on every cell of rows 1 to 15 and nothing else: **6 pages against LibreOffice's 5**, the extra one
being that band. The same test governs `ReachedFromTheLeft`'s short-circuit and moved with it,
which is what `bLeftIsEmpty` is — `IsPrintEmpty`'s own verdict on the band to the left, and that
verdict starts at `IsBlockEmpty`.

**A merge that holds nothing was reaching layout as no merge at all.** Every format states its
merges once, as a list of ranges, and every reader then puts the span on the block's top-left cell
and drops the cells it covers — which recovers the merge only when that top-left cell survives. An
empty cell past the last filled one in its row is trailing padding that all four readers drop, so
the anchor of an empty merge went with it and `SheetLayout` derived no merge from the tree. That is
exactly the merge that matters, because `ScOutputData::IsAvailable` stops a neighbour's long string
at a merged or overlapped cell **whether or not it holds anything**
(`sc/source/ui/view/output2.cxx:1178-1191`). `SheetLayout.StatedMerges` now carries the ranges
themselves and every reader fills it — `mergeCells`, BIFF12 `MERGECELL`, BIFF8 `MERGEDCELLS`, and
ODF's `table:number-columns-spanned`. Measured on the same workbook, whose A1 title ran through its
empty `B1:D1` merge and onto the next column band: **554 words against 541, and 541 now.**
`DrawLeadIn`'s own merge test moved to `IsMerged` at the same time; it was asking the cells for
spans and had the same blind spot.

**A drawing belongs to the sheet, not to a page.** `SheetPageGraphics` anchored one to the page
holding its top-left cell, which is the same answer for everything that does not straddle a break
and loses half of everything that does. `ScOutputData::PrePrintDrawingLayer`
(`sc/source/ui/view/output3.cxx:40-104`) sets a map-mode offset of minus the width of the columns
and the height of the rows *before* the page's first, and `PrintDrawingLayer` (`:138`) paints the
whole drawing page through it. `ColumnX` and `RowY` now walk the grid in **either** direction, so a
drawing anchored left of the band sits at a negative offset and shows its right-hand part, and an
absolute anchor resolves through the same walk instead of only landing on the page holding A1.
A rectangle that misses the paper is dropped. Measured on `Air_Boss_Master_List.xlsx`, whose note
box is anchored in column E and straddles the column break: LibreOffice prints its left half on
page 1 and its right half on page 3, so **514 words against 527, and 530 now.**

Fixtures: `features/sheet-empty-merge.fods` and `features/sheet-drawing-across-break.xlsx`, each
checked against LibreOffice 24.2.7.2's own PDF and each carrying its negative half — an unmerged
string that must still spill, and cells that must still be split between the two pages. All three
tests were confirmed to fail with their defect put back.

`batch-004` measured **8/10** at the same commit and is **10/10** now, with `batch-001` to
`batch-003` re-proved: **40 of 40**. Its two failures were both `.xlsx` over-counting words by
about fifty with the page count already right, and both were shapes drawn where the reference does
not draw them.

**A shape is bounded by the page's own cell block, not by the paper.** This is the correction to
the paragraph above: the first attempt dropped a drawing only once its rectangle missed the sheet
of paper, and the region `PrePrintDrawingLayer` hands to `BeginDrawLayers` runs from the page's
first printed column to its last (`sc/source/ui/view/output3.cxx:41-95`). The two differ wherever a
column band stops short of the right margin, which is most bands of a wide sheet. Measured on
`Part_375_Operators.xlsx`, whose two table slicers sit in the third of its three column bands and
fit comfortably on the paper of the other two: LibreOffice draws them once, on page 19, and we drew
them on pages 1, 10 and 19 — 2251 words against 2197, and 2197 now. The edges are inclusive because
Calc's `tools::Rectangle` is: probed against the binary with a box anchored exactly on a band's
right edge (drawn on both pages) and one half an inch past it (drawn on neither but its own), which
is `features/sheet-shape-clip.xlsx`.

**`vertOverflow="clip"` removes lines rather than masking them.**
`SdrTextObj::impDecomposeBlockTextPrimitive` builds a clip range of the box's height
(`svx/source/svdraw/svdotextdecomposition.cxx:581-624`) and hands it to
`TextHierarchyBreakupBlockText`, whose comment states the rule outright: "only text portions
completely inside are to be accepted, so this is different from geometric clipping (which would
allow e.g. upper parts of portions to remain)" (`include/svx/svdoutl.hxx:56-59`). So the overflow
never reaches the text layer, which is why it shows in a word count at all. `oox` sets the flag for
both `clip` and `ellipsis` (`textbodypropertiescontext.cxx:85-97`), and the vertical adjustment is
suppressed while it applies. Measured on `Foreign_SA-CAT-I_and_CAT-II-III_Pub_0.xlsx`, whose notes
box is 1.37 inches tall and holds five paragraphs of caveats: 1556 words against 1504, and 1530 now
— inside the gate's 2% band rather than exactly equal.

**The residue is not the box drawn twice, and the previous round marked that guess unverified.**
It is measured now, and it is the *page-edge clip*. LibreOffice's first page shows the notes block
cut off at the right edge of the column band, mid-word — its own text layer holds `Ai` and `r` as
separate tokens, and `the Flight Procedures an` ends there — with the tails picked up on the band
to the right, because Calc draws the string once per page positioned at its cell and clipped to the
page's output rectangle. We draw each line whole on the first band and none of it on the second, so
we are 35 words up and 9 down for a net 26. Implementing it needs a clip that drops the glyph runs
it excludes, which is what `PdfContentSink` is being taught elsewhere; nothing here should build a
second one.

### Where `batch-005` stands, and the largest lead in the track

`batch-005` measured **7/10** after the two rounds above — the scoreboard recorded 4/10, and the
three it gained were gained by the empty-page and merge work rather than by anything aimed at it.
Its three remaining failures were three different things, and one of them was the biggest
systematic defect this track had left. Two of the three are fixed below; the batch is **9/10**.

**A row height stated without `customHeight` is a hint, and Calc recomputes it.** `ht` on a `row`
is what the *writer* measured; `customHeight` is the flag that says a user chose it.
`WorksheetGlobals::convertRows` (`sc/source/filter/oox/worksheethelper.cxx:1268-1286`) imports the
stated height either way — "always import the row height, ensures better layout" — but calls
`SetManualHeight` only when `mbCustomHeight`, so every other row stays on optimal height and Calc
re-derives it from what the row holds. Measured on `National-Reports.xlsx`, whose 117 rows all
state `ht="15.75"` and none states `customHeight`: **our row pitch was 15.735 pt and LibreOffice's
is 15.0**, and the 4.9% compounded into 8 pages against 6.

**This is now done — see "Done: the row height a file only guessed at" below.** The document
matches exactly, 6 pages and 1276 words against 1276.

The remaining failure of the three: `esurf-12-135-2024-t01.xlsx` draws four-digit years as `201`
and `202`, three characters wide, where LibreOffice draws them whole — a number clipped to its
column instead of overflowing or becoming `###`, which is F.3's two-way coupling between the
`General` format and the column width.

`Background_Declaration_Template.xls`'s duplication was **not** what the previous round guessed.
It was recorded as "the column-band duplication the round above fixed for SpreadsheetML arriving
in the BIFF path", and that is wrong twice over. LibreOffice draws the text **zero** times, not
once: "W. Post: need to adapt to logic rules per record" appears on none of the reference's 25
pages. It is a *cell comment*, and a comment is not a shape — `ftCmo` type 25
(`EXC_OBJTYPE_NOTE`, `sc/source/filter/inc/xlescher.hxx:69`), whose importer calls
`SetInsertSdrObj(false)` in its constructor and turns the text into a `ScPostIt` instead
(`sc/source/filter/excel/xiescher.cxx:1852-1883`). Drawing it twice was not a defect on its own —
a shape really is drawn on every page it reaches — and the fourth embedded font was its.

## What the fourth sheets sweep found

Measured at `161d62fb9`. `sheets/batch-002` was **7/10** and the briefed baseline reproduced to the
digit — the same three failures, the same page and word counts. All three are `.xls`, and the
standing lead this track has carried for two rounds was **half right**: the two GA Survey files do
carry `MSODRAWING` + `OBJ` + `TXO`, so the missing drawing layer is real, and it accounts for six of
their twenty-eight and thirty missing words. The rest of both, and the missing page, is something
else entirely — and `P1636e.xls` has no drawing at all.

**A merged block straddling a column break is drawn on both pages, and only one of ours drew it.**
`P1636e.xls` merges its title and its eight footnotes across all six of its columns and breaks after
the third, so its second page holds nothing but covered cells. Calc reaches the block's anchor from
the covered end — `ScOutputData::GetMergeOrigin` (`output2.cxx:953`) — and one flag decides which
covered cell may reach: `bDoMerge`, which for a horizontally overlapped cell is
`bIsLeft = (nX == mnVisX1)` (`:957`), the block's first **visible** column. Set, the walk runs back
to the origin through anything; clear, it gives up at the first column that is not hidden, because a
nearer cell will draw the block instead. The port had only the second half — the hidden-column case
the previous round added — so a merge whose anchor was simply on the *previous page* vanished.
`ColumnBand` now carries `FirstVisible` (Calc's `mnVisX1`, which is `mnX1` after `StripHidden`) and
`DrawCoveredMerge` walks with it. The block is placed at its true position, off the left of the
page, and the part that lands on the paper is what shows: **304 words against 344, and 345 now**.

**A chart sheet is a sheet, and the BIFF reader dropped it.** `BOUNDSHEET` type 2 opens a substream
headed by a `BOF` of type `0x0020` that carries its own page setup and a chart, and
`XlsWorkbookReader` skipped it with the macro and Visual Basic substreams — so both GA files came out
a page short. `MsBinary/XlsChartReader.cs` reads the substream into the same `ChartPlot` the other
two families use, and `ReadChartSheet` gives it a `SheetLayout` whose only content is one absolutely
anchored drawing. Everything downstream then works unchanged: `SheetDrawingArea` widens the printed
range to cover it, `SheetEmptyPages` keeps the page it overlaps, and `SheetPageGraphics` paints it
through `SheetChart`.

Three things about that reader are worth stating because each was a decision rather than a
transcription:

- **The printed rectangle is computed from the paper, not read.** `CHCHART` does state a rectangle
  and it is the one Excel showed the chart at on screen; using it puts the chart off the paper.
  `XclImpChartObj::FinalizeTabChart` (`sc/source/filter/excel/xiescher.cxx`) derives the printed one
  instead — the paper less the margins, less two centimetres of width and one of height, less
  another two and one when the sheet prints its headings, offset a centimetre from the left of the
  sheet and half a centimetre from the top.
- **The series data is deliberately not resolved, and the reference agrees.** Both GA charts state
  their source links as `EXC_CHSRCLINK_DIRECTLY`, which names no range; LibreOffice produces a series
  with an empty range and draws no marks, and its flat-ODF export says so outright
  (`chart:values-cell-range-address=""`). The `LABEL` and `NUMBER` records that trail the chart
  substream *do* hold eight categories and eight numbers, and reading them would draw eight bars the
  reference does not.
- **A chart substream carries drawing objects too**, positioned in quarter-thousandths of the chart
  area with the fractions in the *cell* fields of the client anchor rather than in its offsets
  (`XclImpChartDrawing::CalcAnchorRect`, `xichart.cxx:4274`). The same eighteen bytes mean something
  else on a worksheet. That is where the GA files' `Source: 2012 GA Survey Table 5.1` lives.

**The BIFF drawing layer exists now, and it is the item this track has carried longest.**
`MsBinary/XlsDrawing.cs` concatenates every `MSODRAWING` payload in a substream into one Escher
stream — the split between records is arbitrary and a container routinely straddles it, so reading
them one at a time yields truncated containers — walks it with the existing
`Paperless.MsBinary/Escher` reader that DOC and PPT already use, and pairs each shape carrying
client data with the `OBJ` that follows it. A `TXO` and its `CONTINUE` records carry the string.
Nothing in `Paperless.MsBinary` changed. **`apron-area.xls` was 294 words against 431 and is
431/431, exactly, page for page.** Twenty-six of the sixty-two corpus `.xls` files carry a drawing
and twenty carry a `TXO`.

Only the text is drawn, not the shape's fill or its outline — which is the SpreadsheetML path's
limit too, so the two formats produce the same page from the same document.

**A ruled but empty cell is inside the printed area, and a workbook of forms is mostly that.**
`ScTable::GetPrintArea` runs two passes over the same columns: the first finds the last row and
column holding *data*, and the second — headed `// Test attribute` (`table1.cxx:710`) — asks each
column for its last *visibly* attributed row, where visible means a non-transparent background, any
of the four border edges, a diagonal or a shadow (`ScPatternAttr::CalcVisible`, `patattr.cxx:1584`).
`e-pass-contact-details-template.xlsx` has nine values and a ruled box on row 14, so the box was
outside the block, never placed, and never drawn: it passed the page and word gates and differed
from LibreOffice by 0.21% of its second page's ink. `Layout/SheetDecorationArea.cs` is that pass.

**Getting the stop condition wrong costs more than the rule gains, and it was measured both ways.**
Formatting reaches the end of a sheet far more often than data does, so the scan has to stop: Calc
walks runs of visually equal rows below the last data row and breaks on the first run of
`SC_VISATTR_STOP` = 84 rows or more (`ScAttrArray::GetLastVisibleAttr`, `attarray.cxx:1922`). **Both
kinds of run count**, and the first implementation here only broke on *gaps*. Measured over the
whole track: breaking on gaps alone took the page error from 885 to **957**, because
`edb-emissions-databank v27`'s third sheet rules 46172 cells down to row 1001 in one unbroken run and
that took its 368 pages to 460. Breaking on equal runs as well — one is one run far longer than the
limit, so Calc takes nothing from it — gives **862**, and `links-2026.xlsx` goes from 21 pages
against 45 to 45 exactly.

**Two defects in `Paperless.Core.Charts` that only a spreadsheet found, both about a title being in
the band already reserved for it.** They are in shared code and are in their own commit.

- **A title states its own line breaks.** `'Chart 8\n2012 Average Fuel Consumption Rates by Aircraft
  Type'` is one `CHSTRING`, and DrawingML writes the same thing as a second `a:p`. Measured as one
  line, it reserves half the room it needs and the second line is drawn over the first. `LinesOf`
  and `MeasureLines` are the fix, and putting them inside `Shape` means every reservation follows.
- **A bar chart's axis titles swap places, because its axes do.** The room was already reserved that
  way — `PlotAreaOf` picks `beside` and `below` from `plot.Direction` — and `AddTitles` drew them by
  *role*, so each title landed in the other's band. `TITLE_AT_STANDARD_X_AXIS_POSITION` is always
  `ALIGN_BOTTOM`: what runs horizontally is titled underneath, whichever axis that is.

Both GA files now match page for page and word for word — **509/537 → 537/537 and 613/643 →
643/643** — and neither has a major image difference. `sheets/batch-002` is **10/10 on the word gate
and 10/10 on the ink-imbalance image diff**, and `sheets/batch-001` stays 10/10 on both.

**Whole track, `sheets/batch-001 … batch-018`, 171 documents: 97 matching → 108, total absolute page
error 881 → 860, exactly-correct page counts 118 → 121**; `xls` 40/62 → **45/62** and `xlsx` 57/109 →
**63/109**. The worst pages by ink imbalance across the two level-one batches are 0.68% and 0.60%,
neither of them a major region.

**A word of caution about the last digit of any of those.** Re-running the sweep at the same commit
gave a *reference* page count of 191 for `ans_mappings_of_eccairs_terms.xlsx` where the previous run
gave 193, so `soffice` is not perfectly reproducible on every document and a difference of one or
two in the page error is noise rather than signal. The A/B figures above — 885, 957, 862 — are
sharper than the sweep totals because they re-rendered only our side against one fixed set of
reference PDFs.

Left open, and found while doing the above:

- **A BIFF chart's series is never resolved.** A `CHSOURCELINK` of type `WORKSHEET` carries a
  formula token array naming the range, which needs the formula engine. Both corpus chart sheets
  state `DIRECTLY` and want no series, so nothing here reaches it — but an embedded worksheet chart
  would, and those are not read at all yet either (they arrive through `OBJ` type 5 and the chart
  substream that follows it, which the drawing collector now sees and ignores).
- **A shape's fill and outline are not drawn**, on either Excel path.
- **`CHLINEFORMAT`'s palette colour is not read**, so a gridline is always black.
- **A merge anchored in a hidden *row* is still lost.** `GetMergeOrigin` walks up as well as left
  and the port still only walks left. No corpus document reaches it.
- **`XlsChartBuilder`'s `Inside(CHTEXT)` guard is defensive and untested.** A test written for it
  passed with the guard removed — `_pendingText` is reset at every `CHTEXT` and consumed at its
  `CHEND`, so a stray `CHSTRING` cannot leak into a title — and was deleted rather than shipped.

## What the third sheets sweep found

Measured at `ef1aac0c8`. `sheets/batch-001` was **6/10**, the weakest level-one batch of the three
tracks, and its four failures all had the right page count and the wrong word count. Three of them
turned out to be **one defect**, and it was not the one the handover named.

**Whole track, `sheets/batch-001 … batch-018`, 171 documents: 92 matching against the previous
round's 84, with total absolute page error unchanged at 1007 and 109 documents on an exactly
correct page count.** `xls` is 39/62 and `xlsx` 53/109. The page error not moving at all is the
right result rather than a disappointing one: every fix this round is a word-level fix, none of
them touches a column width or a row height, and a page count that moved would have meant one of
them had a side effect nobody asked for. **Page error is where this track's remaining value is**,
and the item that would move it most is the column-width measurement below.

The 84 is the predecessor's number and was not independently re-measured at `ef1aac0c8`; what was
re-measured is `sheets/batch-001`, which reproduced the briefed 6/10 and all four of its word
counts to the digit, and the whole-track page error, which came out at the briefed 1007 exactly.

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

`sheets/batch-001` is **10/10** with these three, and nine of the ten pass the ink-imbalance image
diff. Two of them did not before, and the tenth still does not — both are below.

**A cell's underline was neither read nor drawn, by any of the three readers.** It is the commonest
decoration a spreadsheet has, because the hyperlink style *is* an underlined blue font, and a column
heading ruled off from its data is the other half. `FMMO_NMPF_37C.xlsx` passed the word gate — 216
against 216 — and its ink imbalance against LibreOffice's rendering was 0.45%, all of it the lines
under its three source links and every one of its headings. It is 0.09% now, which is the image
comparison's difference between `MAJOR` and `shifted`. `SheetCellFormat` carries
`Underline` and `IsStruckThrough`, all three readers fill them — SpreadsheetML's `<u val="…"/>`,
BIFF's `FONT` underline byte and `fStrikeOut` flag, ODF's `style:text-underline-style` plus its
separate `-type` — and `SheetTextLayout.Decorate` draws them as filled rules from the face's own
`post` and `OS/2` metrics, through the same `LineSpacing.ResolveDecorations` the rest of the
project uses. Excel's two *accounting* underline styles run the width of the cell rather than of the
text and are folded onto their plain counterparts; a dotted or wavy ODF underline draws solid. Both
are recorded on `SheetUnderline` rather than silently lost.

Not reproduced, and it is one thing: a rich cell mixing an underlined portion with a plain one
underlines the whole line, because the rule is drawn per line from the cell's own format. The
portions carry the format that would answer properly and the per-portion run geometry to place a
partial rule with does not exist yet.

## Done: a column width is measured in the workbook's own font

`DigitWidthTwips` was the constant **111** in all three of `Ooxml/XlsxPrintSetup.cs`,
`MsBinary/XlsPrintSetup.cs` and `Xlsb/XlsbPrintSetup.cs` — the widest digit of ten-point Liberation
Sans — and every workbook whose default font is anything else therefore had proportionally wrong
columns. `Patent Index 2024 - Top 100 applicants 2024.xlsx` defaults to twelve-point Arial, whose
digit is 133 twips, and it was the last major image difference on `sheets/batch-001`.

**The 133 was confirmed against LibreOffice rather than against arithmetic, and the arithmetic
alone would have been misread.** Measuring the two renderings' column pitch gives 137 twips per
digit, not 133; the extra 3% is that LibreOffice paginates that sheet at a **34%** fit-to-page zoom
where we pick 33%, and the same 3% is the whole of the "reference draws the same text at an em 3.5%
larger" note this section used to carry. That was recorded as an undiagnosed second defect and a
possible `mpRefDevice`/`mpDev` split; it is neither. **The instrument that settles it is
`--convert-to fods`**: LibreOffice writes the width it computed into `style:column-width`, so a
round trip states the answer outright — 886.2, 6591.2, 1389.0 and 1182.1 twips against digit counts
of 6.6640625, 49.5546875, 10.44140625 and 8.88671875, which is 133.0 four times over. Use it for
anything stated in a unit the file does not carry; it is far sharper than fitting a pitch.

**The claim that almost every real `.xlsx` is affected is false, and the survey is worth keeping.**
Of the 109 corpus `.xlsx` files, 51 default to Calibri 11 and 29 to Arial 10, whose digits measure
111.50 and 111.23 twips — both of which LibreOffice reports as **111**, the constant that was
already there. So 80 of 109 were already right to within half a percent, and the same holds for
49 of 61 `.xls`. The 41 documents that were wrong were wrong by 9% to 38% (Aptos Narrow 11 and
Verdana 11 at 1.26, Arial Black 12 at 1.38), which is what makes it worth doing.

**The digit is truncated, not rounded**, and that was measured too: one-column probe workbooks
round-tripped through LibreOffice 24.2.7.2 give 111.23 → 111, 122.35 → 122, 133.48 → 133,
111.50 → **111**, 121.64 → **121**, 100.00 → 100, 120.02 → 120, 139.97 → **140**, 152.70 → **153**.
Eight are exact either way and the two Carlito rows disagree with the two DejaVu ones, so a device's
quantisation decides them rather than a rounding rule. Truncation is the half that matters: Carlito
11 is what Excel's own default resolves to and is the default of 65 of the 171 corpus spreadsheets,
all of which were already correct at 111 and which rounding would move to 112.

**Where the measurement lives, and why not in the reader.** `Layout/SheetColumnDigits.cs` holds a
width as a count of digits plus a fixed part that does not scale (`baseColWidth`'s five screen
pixels, BIFF's deliberate half-twip bias, and Calc's own 64-point standard column, which is a
length rather than a count). The readers build that and the default font's *name* — both free — and
`SheetLayout.Grid` resolves the face and converts, once per sheet, the first time anything asks for
the geometry. Nothing on the extraction path asks, so `paperless extract` still pays for no font,
which is the rule in `dotnet/CLAUDE.md` that made this look bigger than it is. LibreOffice resolves
early instead (`UnitConverter::finalizeImport`, `sc/source/filter/oox/unitconverter.cxx:113`) because
it has a document with a reference device attached where Paperless has a reader with a stream.
The grid is still materialised eagerly at the 111 fallback, so a caller that never resolves gets
exactly what it got before.

Measured over `sheets/batch-001 … batch-018`, 171 documents, this change alone: **92 matching → 97,
total absolute page error 1007 → 885, exactly-correct page counts 109 → 117**; `xls` 39/62 → 40/62
and `xlsx` 53/109 → 57/109. `sheets/batch-001` stays 10/10 on the word gate and is now **10/10 on
the ink-imbalance image diff as well** — the Patent workbook was 6.20% differing at 2.29% ink
imbalance and now has no major region at all.

**`DEFCOLWIDTH` carries `#i3006#`'s font-dependent correction, and it could not be applied before
this** — it depends on the default font's size, which was not read. `ImportExcel::DefColWidth`
(`sc/source/filter/excel/impop.cxx:640-657`) adds `40960 / max(fontHeightTwips − 15, 60) + 50` in
256ths of a digit before converting, with the comment "additional space for default width — Excel
adds space depending on font size"; `XclTools::GetXclDefColWidthCorrection` (`xltools.cxx:318`)
reconstructs the formula as five screen pixels expressed in digit-widths and admits that three of
its constants are of unknown origin, so it is reproduced as written. It is worth 110 twips — nine
per cent of the column — on a twelve-point Calibri sheet: `aircraft_analysis_2016-04-27.xls` states
`DEFCOLWIDTH` 10, LibreOffice's own flat-ODF export of it puts the default column at 1319 twips,
and ten digits alone give 1209. Over the 62 corpus `.xls` documents it takes the page error from
90 to **86** and the exactly-correct page counts from 46 to **47**; three documents move and no
match is lost, of which `underlying-holdings-emea-en_gb-monthly-holdings-state-street.xls` goes
from 11 pages against 15 to 15 against 15.

**Both changes together, over the whole track: 92 matching → 97, total absolute page error
1007 → 881, exactly-correct page counts 109 → 118**; `xls` 39/62 → 40/62 with its page error
106 → 86, and `xlsx` 53/109 → 57/109 with its page error 901 → 795. The remaining 881 is
overwhelmingly `.xlsx` and overwhelmingly a handful of documents: eight of them carry 648 of it,
led by `sectors-defense-and-aerospace.xlsx` at 41 pages against 227 and
`TK-Syllabus-Comparison-Document-v2.xlsx` at 1391 against 1235. **The next sweep's leverage is
there rather than in another systematic width or height**: 118 of 171 documents already have the
page count exactly right, and the tail is a few documents wrong by a lot.

Two things the corrected widths exposed rather than caused, both left:

- **`aircraft_analysis_2016-04-27.xls` paginates to 65 pages against LibreOffice's 46**, and its
  column widths are now provably right — its two `COLINFO` columns come out 5424 and 6231 twips
  against LibreOffice's own 5423.8 and 6231.2, and its default column matches too. So whatever is
  left is not a width. It was 44 before, which was nearer by accident: two errors were cancelling.
- **A fit-to-page zoom is picked one per cent low.** The Patent workbook sets
  `<pageSetUpPr fitToPage="1"/>` with `fitToWidth="0"`, and LibreOffice fits its 103 rows to one
  page at 34% where the search here lands on 33%. It costs nothing in pages on that document and
  three per cent in every position on it — and it is the whole of what used to be recorded here as
  an unexplained "the reference draws the same text at an em 3.5% larger".

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

**What that shape text still gets wrong, and it is one thing.** *(Closed in the eighth round: a run
carries its typeface now and `+mn-lt` is resolved through the theme's font scheme before it is
stored. What follows is the diagnosis as it stood.)* The runs name `+mn-lt` — the
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
- [x] **A drawing belongs to the sheet, not to a page.** It used to be anchored to the page holding
  its top-left cell, which is the same answer for everything that does not straddle a break and
  loses half of everything that does. `ScOutputData::PrePrintDrawingLayer`
  (`sc/source/ui/view/output3.cxx:40-104`) sets a map-mode offset of minus the width of the columns
  and the height of the rows *before* the page's first and `PrintDrawingLayer` (`:138`) paints the
  whole drawing page through it, so the anchor is now resolved against the page's own columns
  wherever it can be and walked out through the grid in **either** direction where it cannot — a
  drawing anchored left of the band sits at a negative offset and shows its right-hand part.
  A rectangle that misses the paper is dropped rather than drawn off the edge. Measured on
  `Air_Boss_Master_List.xlsx`, whose note box is anchored in column E and straddles the column
  break: its right half is on LibreOffice's page 3 and was on none of ours, 514 words against 527.
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

## Done: the row height a file only guessed at

All four formats say whether a row's height was set by a user or computed by the writer — ODF's
`style:use-optimal-row-height`, SpreadsheetML's and BIFF12's `customHeight`, BIFF8's `fUnsynced` —
and Calc honours it on load, recomputing every non-manual row before anything is drawn. Each filter
does it in its own place: `WorkbookGlobals::finalize` for the OOXML family
(`sc/source/filter/oox/workbookhelper.cxx:659`), `ImportExcel::AdjustRowHeight` for BIFF
(`sc/source/filter/excel/impop.cxx:1285`), `ScXMLImport`'s recalc ranges for ODF
(`sc/source/filter/xml/xmlimprt.cxx:1438`). `Layout/SheetOptimalRowHeights.cs` reproduces it, and
`SheetLayout.Grid` applies it once per sheet on first read, so extraction still pays for none of it.

**Two computations, and only the first is a measurement.** `ScColumn::GetOptimalHeight`
(`column2.cxx:898-1100`) walks each column's attribute ranges and asks two different questions.
Every cell contributes `lcl_GetAttribHeight` (`column2.cxx:866-892`), which is arithmetic on the
font's *size* and nothing else: `trunc(sizeTwips × 1.18) + topMargin + bottomMargin − 23`, floored
at the sheet's minimum. A cell that also wraps, rotates, stacks or holds a second line contributes
`GetNeededSize` as well, and the row takes the larger. So a plain sheet needs no glyph measured at
all — which is why the arithmetic shipped first and on its own.

**The measurement is coarse, and reproducing the coarseness is what makes it exact.** The previous
round measured five rotated probes, found an accurate reproduction 5.8% too large, and stopped —
the right call on that evidence, and the evidence was incomplete. `GetNeededSize`'s EditEngine
branch formats against a headless `VirtualDevice` and quantises to whole device pixels three times:
the em size, the ascent and the descent. The two cell margins truncate to one pixel each. The
pixel total is turned back into twips by dividing by `ScSizeDeviceProvider`'s `nPPTY`, which is
**0.067 and not 1/15** — it is computed as `LogicToPixel(Point(1000,1000), MapTwip).Y() / 1000.0`
and that conversion returns whole pixels, so 666.67 becomes 667
(`sc/source/ui/docshell/sizedev.cxx:48-50`). Dividing by 1/15 instead puts a three-line twelve-point
row at 795 twips where LibreOffice writes 791.

The quantisation is exactly `Paperless.Text.Fonts.MetricGrid`, which Writer already had for
`fUsePrinterMetrics` — the same rounding on a different device, arrived at independently, which is
the strongest evidence the model is right.

Fitted to thirty probe rows — six font sizes against five wrapped line counts, read out of
LibreOffice's own flat-ODF export — and reproduces **all thirty**, including the eighteen-point row
whose single word is wider than its column and therefore takes two lines. `sheet-row-height-hint.xlsx`
and `sheet-row-height-wrap.xlsx` hold the assertions.

**What is still not measured**, and falls back to the larger of the arithmetic height and what the
file states: a turned or stacked cell, whose size is its text's *width* put through an angle, and a
cell in several faces, whose lines are each as tall as the tallest portion on them. The fallback
cannot lose text, because the arithmetic height is a lower bound in Calc too — `bStdAllowed` stays
true for such a cell and its attribute height is written into the array before any measurement is
compared against it.

**Calc will not replace a height that rounds to the same pixel.** `lcl_pixelSizeChanged`
(`sc/source/core/data/table2.cxx:3388`) compares `trunc(height × nPPTY)` and leaves the file's value
alone when they agree, so a document LibreOffice wrote round-trips unchanged even where the
arithmetic here lands a twip or two away. Calc applies that guard on the OOXML and ODF paths and not
on the BIFF one, which reaches rows through `SetOptimalHeightOnly`; it is applied to all three here,
because the difference can only ever leave a file's own height in place.

**The floor is per sheet, and only one filter sets it.** `ScTable::GetOptimalMinRowHeight`
(`sc/inc/table.hxx:882-887`) answers `ScGlobal::nStdRowHeight`, 256 twips, unless something set it,
and the only thing that ever does is the OOXML filter, from the sheet's own `defaultRowHeight`
(`worksheethelper.cxx:965`). A BIFF or ODF sheet is floored at 256 whatever its file says its
default row is — which is why `SheetGrid.OptimalMinimumRowHeight` is a property of its own rather
than `SheetAxis.DefaultSize`, whose value it happens to share for SpreadsheetML and not for the
others.

**A BIFF sheet can declare every row manual in the record about its default height**, and missing
that cost eight `.xls` documents their page count on the sweep that found it. `DEFAULTROWHEIGHT`
carries its own `fUnsynced` (`EXC_DEFROW_UNSYNCED`, `sc/source/filter/inc/xltable.hxx:114`), and
`XclImpColRowSettings::Convert` answers it by marking every row of the sheet `ManualSize` before it
reads a single `ROW` record — "first access to row flags, do not ask for old flags",
`sc/source/filter/excel/colrowst.cxx:212-215`. Nothing later clears it: the per-row loop only ever
sets the bit. `SheetGrid.RowHeightsAreManual` carries it, and the reader was previously skipping
that word to get at the height behind it. BIFF2's two-byte record has no flags field at all and
Calc passes the bit unconditionally (`impop.cxx:598-604`).

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
