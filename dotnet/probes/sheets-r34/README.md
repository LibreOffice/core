# Sheets round thirty-four — probe data

Base commit `a115b723b`, checked with `git log --oneline -1` in the worktree before anything was
measured. Reference renderer LibreOffice 24.2.7.2. Both whole-track sweeps ran against a
checksummed copy of the CLI whose `Paperless.Spreadsheets.dll` and `Paperless.Core.dll` hashes
were diffed against the tree's before the run started, and the copy was proved to hold the change
by rendering the one document whose page count it moves.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep at `a115b723b`, before any change |
| `final-whole-track.tsv` | the same on the final tree |
| `reach.tsv` | the byte-level reach of the two changes over all 171 documents, clock pinned |
| `rowdiff.py` | our resolved row heights against LibreOffice's own `style:row-height`, per sheet, per row |
| `RowProbe/` | dumps what a row's cells look like to the height computation — text as the reader produced it, wrap flag, face, size, column width |
| `make-break-fixture.py` | builds `tests/corpus/features/sheet-row-height-break.xlsx` |
| `reach.sh` | renders the track with two CLIs and diffs the bytes; unchanged from r33 |
| `score.py` | turns a `rows.tsv` into matches, page error, exact counts, word error |

## The baseline reproduces the brief, with one figure four out

```
150/171 matches   abs page error 85   exact page counts 156   abs word error 32678
001 10/10  002 10/10  003 10/10  004 10/10  005 10/10  006 10/10  007 10/10  008 10/10
009 9/9    010 8/10   011 8/10   012 8/10   013 8/10   014 9/10  015 6/9   016 5/9
017 6/10   018 3/4
```

Matches, page error and exact page counts are the brief's to the digit. The word error is 32678
against the brief's 32673 — five out, on a track whose reference has been shown to move a page
between sweeps. 171 rows, no duplicate path, no `ref-failed`, no `ours-failed`.

## The cluster is a hard break, and it is two mechanisms rather than one

The brief's reading was **row heights differing by a whole multiple of one line of the cell's own
text, both signs**. That measurement is right and there is no single cause behind it. Comparing
our resolved heights against LibreOffice's own `style:row-height` (`rowdiff.py`) and then dumping
the cells that decide them (`RowProbe`) splits it into three:

### (a) A break in a cell that does not wrap is not a break at all

`Capability_List…unsorted.xlsx` A452 is `19090-105 (SCD\n604-85001-23)` in a cell whose format
does **not** wrap. LibreOffice states **285.1 twips** for that row — one line — where we gave 567,
and its own PDF holds `19090-105 (SCD604-85001-23)` as a single run with no space where the break
is.

Three citations, all saying the same thing from different ends, and the two importers say it in
the same words:

- `SheetDataBuffer::setStringCell` computes `bSingleLine = !pXf->getAlignment().getModel().mbWrapText`
  and passes it to `putRichString`, which is `rEE.SetSingleLine(bSingleLine)`
  (`sc/source/filter/oox/sheetdatabuffer.cxx:125-133`, `worksheethelper.cxx:1607-1611`).
- `XclImpStringHelper::SetToDocument` computes `bSingleLine = !pXF->GetLineBreak()` and calls the
  same thing (`sc/source/filter/excel/xihelper.cxx:246-256`).
- The height agrees independently: `bStdOnly = !bBreak`, `sc/source/core/data/column2.cxx:930-935`.
  So does the drawing: `ScDrawStringsVars::HasEditCharacters` (`output2.cxx:823-847`) lists seven
  code points and U+000A is not one of them.

Neither importer asks whether the string is rich, and a plain string never reaches the other
branch either — `RichString::extractPlainString` refuses a string holding U+000A
(`richstring.cxx:375`). **The first version of the fix carried a `rich &&` qualifier and it was
wrong**; removing it changed nothing on this corpus (see the reach below), which is why the sweep
below was not repeated.

LibreOffice's own flat-ODF export of the round's fixture shows the two branches side by side:
the non-wrapping cell comes back as one `<text:p>` holding a raw newline, the wrapping one as
three `<text:p>`.

### (b) A trailing break is an empty paragraph, and Calc reserves a line for it

`flightstandards-doc-Cross-reference-table_version02.xlsx` D936 is
`Contracted activities \r\nCONTRACTING - GENERAL\r\n`. LibreOffice states **700.7 twips** — three
lines — where we gave 477.

This one is ours rather than Calc's. `ContentTableCell.AppendText` stripped **every** trailing
newline where its own comment says it takes the one the last paragraph contributes; each
paragraph terminates itself, so a cell whose last paragraph is empty ends in two, and taking both
erases the paragraph rather than the terminator.

### (c) A ±1 line difference in the 96 dpi measurement — not addressed

What is left after (a) and (b), measured the same way:

| document | rows differing before | after |
|---|---:|---:|
| `Capability_List…unsorted.xlsx` | 16 of 2225 | 1 |
| `flightstandards-doc-Cross-reference-table_version02.xlsx` | 45 of 2201 | 7 |
| `tk-syllabus-comparison-document-v5.xlsx` | 20 of 6520 | 19 |
| `seihon_zassi_kikou_20221215.xlsx` | 129 (the brief's figure) | 0 of 4000 |

The residue is almost all ±224 twips — one line at 10 pt — in **both** directions on the same
document, which is what a per-line rounding looks like rather than a ratio.

**A warning for whoever takes it: the reference PDF cannot arbitrate this.** Calc measures a row
against a 96 dpi `VirtualDevice` and draws it against the export device, and the two can break
the same cell differently. Measured on the header row of `flightstandards`' Cross-Reference
Table: LibreOffice reserves **1819.9 twips — eight lines** — and its PDF draws that row's cells on
**four**, which is exactly what we both draw and what we also reserve. The flat-ODF row height is
the only oracle for (c).

## Reach, predicted before the sweep

Predicted **25–40 documents changed and 2–4 verdicts moved**. Measured, rendering all 171 with
both CLIs under `SOURCE_DATE_EPOCH=1700000000`: **24 differ, 147 byte-identical**, and three
verdicts moved. The prediction was just high.

Three of the 24 are `.xls`. A census can only look at the zip half: 43 of the 118 documents it can
read hold a cell string with a break and 32 of those a trailing one, over a track whose other 53
documents it cannot read at all — so on the half it can see it over-stated by about 1.8×, and on
the other half it would have reported nothing where three documents moved.

The second change — dropping the `rich &&` qualifier — was measured the same way against the
first: **171 of 171 byte-identical**, so the whole-track sweep taken at `3a6dc3884` describes
`63210aa11` exactly.

## The whole-track sweep

```
153/171 matches   abs page error 77   exact page counts 159   abs word error 32630
001 10/10  002 10/10  003 10/10  004 10/10  005 10/10  006 10/10  007 10/10  008 10/10
009 9/9    010 8/10   011 9/10   012 9/10   013 8/10   014 9/10  015 6/9   016 6/9
017 6/10   018 3/4
```

| document | before | after |
|---|---|---|
| `Capability_List…unsorted.xlsx` | 150/147 `pages` | **147/147 `match`** |
| `sectors-defense-and-aerospace.xlsx` | 225/227 `pages` | **227/227 `match`** |
| `flightstandards-doc-Cross-reference-table_version02.xlsx` | 461/464 `pages` | **464/464 `match`** |

Nothing moved the wrong way. Two documents in the reach changed without changing a verdict and
are worth naming:

- `ans_mappings_of_eccairs_terms.xlsx` went 192/191 to 190/191. Its absolute page error is
  unchanged at one, and it is the document whose *reference* returned 191 then 190 on successive
  sweeps in round 33.
- `seihon_zassi_kikou_20221215.xlsx` stays 83/84 although its first 4000 rows now agree with
  LibreOffice's own row heights exactly. The sheet has 5159.

## The +3 pages and the taller rows were one finding

The brief asked whether the human review's *"some cells are taller on paperless"* on
`Capability_List…unsorted.xlsx` and its +3 pages were the same thing. They are: the sixteen rows
that differ are the taller cells, and with them corrected the workbook renders 147 pages against
the reference's 147.

## Tests

`SheetBreakRowHeightTests` (seven) over `tests/corpus/features/sheet-row-height-break.xlsx`, and
`ContentTableCellTextTests` (six) on the Core half.

The fixture is written as SpreadsheetML **directly** rather than authored as flat ODF and
converted, and that is the point of it: the two importers genuinely disagree about a string
holding U+000A, so a fixture converted through ODF would measure the other importer and pass
whatever the code did. Its face is stated — Liberation Sans 10 pt — because a fixture naming Arial
resolves to the unwired default and would pass whatever the code did for a second reason.

Its five rows differ only in `wrapText` and in where the breaks fall. LibreOffice's own flat-ODF
export gives **252.9 / 700.7 / 700.7 / 476.8 / 256.3** twips; we give 256 / 701 / 701 / 477 / 256.

Both halves were verified by reintroduction, through `verify-test.sh`:

| mutation | detected by |
|---|---|
| `while` back in `ContentTableCell.AppendText` | Core 2 of 284, Spreadsheets 2 of 605 |
| `\|\| text holds a break` back in `SheetOptimalRowHeights` | Spreadsheets 2 of 605 |

## One of the brief's review items, refuted by measurement

The brief asked whether `T0A0D0000090006XLSE.xls`'s *"minor text sizing causes different wrapping
in some cells"* is the same root as the row-height cluster, before treating it as separate. It is
not: `rowdiff.py` compares its only sheet against LibreOffice's own `style:row-height` and
**0 of 360 rows differ**. Whatever is wrapping differently there does not reach the reserved
height, which puts it on the drawing side — consistent with the document being page-exact
(162/162) while over-drawing 2098 words.

## Left open, and labelled

- **The 3.4 twips on the fixture's row 1.** Calc keeps a multi-line string as an `EditTextObject`
  even in single-line mode, so `HasEditCells` sends that row through `GetNeededSize`, which
  measures one line and applies **no** floor; our arithmetic path floors it at the sheet's optimal
  minimum. LibreOffice writes 252.9 for that row and 256.3 for the plain-string row 5; we write
  256 for both. A fifth of a point on a row.
- **ODF.** `ScXMLImport` makes a cell of several `text:p` a multi-paragraph edit cell whatever the
  wrap says, so the rule shipped here is wrong for `.ods`. The sheets track holds none, so this is
  unmeasured rather than measured; `SheetHardBreakTests` already records the same gap on the
  drawing side.
