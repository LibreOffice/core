# round 80 — the `.ods` column: what an over-wide print area was measuring, and the 200-row rule

`.ods` **234 → 255 of 307**, measured against LibreOffice **26.2.4.2** on the converted corpus.

## Environment

| | |
|---|---|
| tree | `/home/user/wt-odsresid2`, branch `agent/odsresid2`, base `5c003d58c` |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything here** |
| corpus | `/home/user/corpus-odf`, 26.2.4.2's own conversion of the 947-document corpus; the `.ods` column is 307 files |
| reference bank | `/home/user/gate-odf-r78/ref`, rendered at 26.2.4.2 |
| original track | `/home/user/sample-files/sheets`, 307 `.xls`/`.xlsx`/`.xlsm` |
| C++ read-only | `/home/user/libreoffice-core` |
| fonts | system fontconfig; all five tarball confound families aside |
| load | a whole-converted-corpus gate ran in the main checkout throughout. This round used two workers. Load average 8–23 on four cores. |
| date | 2026-09-08 |

`sweep-ours.sh` re-renders our half alone and applies `batch-check.sh`'s verdict rule — the same
`awk` band, the same floor of 15 alphanumeric characters — to the banked reference bytes. The reuse
is sound because the diff is confined to `dotnet/src` and cannot reach `soffice`.

**The base was re-derived rather than trusted, and it reproduces.** `before.tsv` is this round's
base at `5c003d58c`, rendered fresh; against `dotnet/probes/odf-gate-r78/rows.tsv`'s `.ods` half it
agrees on page count and verdict for **307 of 307**. Worth running: four commits between
`873c766f9` — the gate the brief quotes — and `5c003d58c` touch `XRay.Office.OpenDocument` and
`XRay.Office.Rendering`, which an `.ods` does go through.

## The column

| | match | pages | pages,words | words | ours-failed |
|---|---:|---:|---:|---:|---:|
| before, `5c003d58c` | **233** | 27 | 10 | 36 | 1 |
| after | **254** | 6 | 9 | 37 | 1 |
| after, the timeout scored on its own | **255** | 6 | 9 | 37 | 0 |

**Twenty-one verdicts move and every one of them is a gain; none is lost.** All 21 are pagination
rows, and after them **6 `pages` and 9 `pages,words` remain of the 37 the round opened with**.

The `ours-failed` row is the same one on both sides and it is the 240 s gate bound rather than a
defect: `Global_Market_Forecast_2016-2035_Airbus_Data_Set.ods` rendered on its own takes **215 s**
and scores 567 pages of 567, 305 549 glyphs of 305 549 and no unembedded font — a `match`. It is a
`match` in the banked r78 gate too, which is why the base is 234 rather than 233.

| document | before | after | reference |
|---|---:|---:|---:|
| `CIS_Debian_Linux_8_Benchmark_v1.0.0` | 88 | **61** | 61 |
| `Laser Report 2024 FOIA __Oct (1)` | 486 | **506** | 506 |
| `afn-afn-20250801-fy25-jan25-mar25` | 282 | **270** | 270 |
| `EASA-IFP-147Scope_WEB…` | 83 | **94** | 94 |
| `Aviation_Abbreviations` | 86 | **94** | 94 |
| `seihon_zassi_kikou_20221215` | 76 | **84** | 84 |
| `ans_mappings_of_eccairs_terms` | 170 | **179** | 179 |
| `FY2021-AIP-grants` | 51 | **57** | 57 |
| `EASA-IFP-145Scope(WEB)…` | 111 | **114** | 114 |
| `SIL_TDB648` | 92 | **88** | 88 |
| `SIL_TDB609` | 54 | **50** | 50 |
| `SIL_TDB605` | 48 | **44** | 44 |
| `aircraft_analysis-2018-06` | 38 | **42** | 42 |
| `FY2023-AIP-grants` | 30 | **33** | 33 |
| `National-Finalists-2024-1` | 24 | **27** | 27 |
| `wiley-cancelled-title-list` | 58 | **60** | 60 |
| `CSJU List of Recipients of funds 2013-2020` | 93 | **95** | 95 |
| `tk-syllabus-comparison-document-v5` | 852 | **855** | 855 |
| `airports_6` | 18 | **17** | 17 |
| `june_2025_published` | 14 | **15** | 15 |
| `EASA-IFP-WEBLISTNOODATA-ListsofForeignapprovals(WEB)…` | 18 | **19** | 19 |

The three `SIL_TDB` documents are worth pointing at: round 77 left them as *"which of the
watermark's bounding boxes reach past the last cell"*. They were not a drawing question at all —
their rows past 200 were being recomputed, and with the stored height back they are page-exact.

## 1. A multi-paragraph cell spills as far as its widest paragraph, and that is the whole of `CIS_Debian`

**Rule.** A string too wide for its column spills into the empty cells beside it and Calc widens
the print area to cover all of it — `ScTable::ExtendPrintArea`
(`sc/source/core/data/table1.cxx`:2127), per cell in `MaybeAddExtraColumn` (`:2218`). *How wide*
"all of it" is comes from `ScColumn::GetNeededSize`, and for a cell the importer stored as an
`EditTextObject` that is not the length of the string:

- `bEditEngine` is set for `CELLTYPE_EDIT` (`sc/source/core/data/column2.cxx`:297-300);
- the cell is formatted against `Size aPaper(1000000, 1000000)` (`:447`), so nothing is broken for
  width;
- and the width is `pEngine->CalcTextWidth()` (`:565`), which is
  `std::max` over the paragraphs of `CalcParaWidth`
  (`editeng/source/editeng/impedit2.cxx`:3507-3525).

So a cell of thirty short paragraphs spills as far as its **longest line**, not as far as their
sum. Which cells have several paragraphs is the *importer's* answer and not the string's: Calc's
ODF filter never calls `SetSingleLine`, so a raw newline makes a paragraph, while the BIFF and
SpreadsheetML filters do call it for a cell that does not wrap and the break stays a character
inside one line. That is the same distinction `SheetLayout.CellBreaksStartLines` already carries
for the drawing, and it is the gate the fix uses.

**Established on the reference, with no free parameter.** `cis-variants.py` rewrites one thing at a
time in `CIS_Debian_Linux_8_Benchmark_v1.0.0.ods` and `refpages.sh` renders each through 26.2.4.2;
`cis-variants.tsv` is the table:

| variant | 26.2.4.2 | ours at `5c003d58c` |
|---|---:|---:|
| `copy` (control: repackaged, unchanged) | 61 | 88 |
| `only_lic` — the License sheet alone | 1 | 1 |
| `only_l1` — Level 1 alone | 39 | **51** |
| `only_l2` — Level 2 alone | 21 | **36** |
| Level 1 with every newline rewritten as a space | **51** | 51 |
| Level 2 with every newline rewritten as a space | **36** | 36 |
| Level 2 with only each cell's longest paragraph kept | **21** | 21 |

The last three rows are the argument. Making the cells single-paragraph — the same characters, one
line — makes **26.2.4.2 itself produce our two counts exactly**; keeping only the longest paragraph
makes **us produce its two counts exactly**. Nothing else in the file changed.

**Reach: 33 of the 307 `.ods` hold a non-wrapping cell the ODF filter makes several paragraphs
of** — 649 cells whose `text:p` carries a raw newline (361 of them in `CIS_Debian`) and 1246 cells
with more than one `text:p` of their own (`census.py`, `census-breaks.txt`). A further 15 243 such
cells wrap, and neither renderer extends the print area for those. The ratio of a cell's whole
string to its longest paragraph runs to **23.1** on `sectors-defense-and-aerospace`, 17.3 on
`CIS_Debian` and 16.1 on `AFS-400_Contacts`.

`SheetTextOverflow.WidestParagraph` is the seat.

## 2. An ODF sheet recalculates its automatic row heights for its first 200 rows only

**Rule.** `ScXMLTableRowContext::endFastElement` (`sc/source/filter/xml/xmlrowi.cxx`:215-244) adds
every row block to the height-recalculation ranges and then takes it straight back out again:

```cpp
if (nCurrentRow > 200 && ptmpStyle && !ptmpStyle->FindProperty(CTF_SC_ROWHEIGHT))   // :228
{
    XMLPropertyState* pOptimalHeight = ptmpStyle->FindProperty(CTF_SC_ROWOPTIMALHEIGHT);
    if (pOptimalHeight && ::cppu::any2bool(pOptimalHeight->maValue))
        rRecalcRanges.at(nSheet).maRanges.setFalse(nFirstRow, nCurrentRow);
```

under the comment *"recalc only the first 200 row in case of optimal document loading"*.

**Two things about that test are not what it looks like.** *"A style with no `CTF_SC_ROWHEIGHT`"*
means a style that states **both** a height and the optimal flag:
`ScXMLRowImportPropertyMapper::finished` (`sc/source/filter/xml/xmlstyli.cxx`:245-258) moves the
height *into* the optimal-height property and clears the height one whenever
`style:use-optimal-row-height` is true, so `FindProperty(CTF_SC_ROWHEIGHT)` is null for exactly
those. And the `any2bool` that follows is then reading the **height**, which is true whenever it is
non-zero. A style stating the flag and no height has both properties cleared, is not found either,
and falls through to `setTrue` — recalculated wherever it sits.

**The boundary is exactly where the source says, measured rather than assumed.**
`dotnet/tests/corpus/features/sheet-row-height-limit.fods` is 206 rows, each its own
`table:table-row`, each stating `style:row-height="0.3in"` with the optimal flag, on a 200 cm page
so that all of them are on one page. 26.2.4.2's baselines (a cell is bottom-aligned by default, so
the gap after a row's text is the *next* row's height):

| gap | pt |
|---|---:|
| r0 → r1 … r199 → r200 | 12.78 |
| **r200 → r201** | **21.60** |
| r201 → r205 | 21.60 |

So rows 0 to 200 inclusive are recalculated and 201 onwards is not, which is `nCurrentRow > 200` on
a zero-based index exactly — `ScMyTables::GetCurrentRow` is `maCurrentCellPos.Row()`
(`sc/source/filter/xml/xmlsubti.hxx`:88).

**It reaches 166 of the 307 `.ods` and 182 744 rows** (`rowlimit-census.py`,
`census-rowlimit.txt`), against 241 539 rows those files state a block for — so two rows in five of
the whole column were being recomputed when the reference keeps them.

**And it is visible in the reference's own PDF on the largest pagination row in the column.**
`Laser Report 2024 FOIA __Oct (1).ods` has one row style: `style:row-height="0.2189in"` with the
optimal flag, on all 11 608 of its rows. 26.2.4.2 draws pages 1–4 at 48 lines on a 15.0 pt pitch —
the recomputed height — and from row 201 on a **15.75** pt pitch, 46 to a page. We recomputed every
row and printed **486 pages against 506**. The same signature is on `EASA-IFP-147Scope` (reference
15.0 against our 13.8, 83 pages against 94) and on `Aviation_Abbreviations` (the same two numbers,
86 against 94).

**It cuts both ways, which is what says it is a rule and not a fudge.** On
`afn-afn-20250801-fy25-jan25-mar25.ods` the stated height is `0.1457in` = 10.49 pt and our
recomputed one is 12.8, so *we* were the long one: page 15 has the reference on a 10.5 pt pitch and
us on 12.8, and the document is 282 pages against 270.

`OdsPrintSetup.RecalculatedRowLimit` is the seat, and it is the **importer's** rule: it reaches
`SheetOptimalRowHeights` only as those rows' optimal flag being false, so the two Excel families —
which have no such limit — cannot see it.

## 3. A multi-paragraph cell's row is as tall as all of its paragraphs, in ODF

The companion to §1 on the other axis, and the half round 77 measured and deliberately left.
`GetNeededSize`'s height branch is `pEngine->GetTextHeight()` (`column2.cxx`:571-577), which for N
paragraphs on an unbounded paper is N lines. `SheetOptimalRowHeights.StandingEditLine` took one
line for every edit cell; it now takes one per paragraph, gated on the same
`CellBreaksStartLines`.

**It had to land with §2 and that is why round 77 left it.** On its own it moved
`Capability_List_9-14-2022_Dallas_Combined-Aircraft_Manuf_unsorted.ods` from the reference's 147
pages to 150, because 26.2.4.2 draws that document's fifteen two-paragraph cells as two lines each
*inside rows it leaves 14.23 pt tall* — the second line painted over the row beneath. Those rows
are past row 200 and keep their stored height, which is §2.

Measured on `dotnet/tests/corpus/features/sheet-cell-break-height.fods`, whose row 2 holds three
paragraphs in a 6 cm non-wrapping cell: 26.2.4.2 starts row 3 **34.61 pt** below row 2 — three
lines of 11.197 pt and a margin — against the 12.39 pt a single line gives.
`SheetVerbatimCellTextTests.TheParagraphsAreDrawnALineApartAndTheRowIsAsTallAsAllOfThem` now
asserts the reference's number rather than ours.

## 4. What this brief got wrong

**(a) "37 pagination rows — 27 long, 10 short".** The 27 and the 10 are the *verdict* counts —
`pages` and `pages,words` — not a long/short split. Counted by sign on the same file, the 37 are
**25 short and 12 long** (ours fewer pages than the reference, and more). The convention checks
out against round 77's own figure: `odf-gate-r76`'s 39 pagination rows split 28 short and 11 long,
which is what its write-up says.

**(b) "36 character-count rows".** 36 is the `words`-only verdict count; **46** rows fail on
characters once the 10 `pages,words` rows are included.

**(c) "`CIS_Debian` … a column-band question over 246 styled-but-empty columns".** Refuted at the
reference. Deleting those 246 columns from the file leaves 26.2.4.2 at **61 pages**; so does
stripping their `table:default-cell-style-name`, and so does deleting the padding rows or
replacing them with 1, 100 or 400. The columns cannot matter: their `ce3` states
`fo:background-color="transparent"` and no border at all, so `ScAttrArray::GetLastVisibleAttr`
finds nothing in them and `ScTable::GetPrintArea`'s attribute pass never widens the block —
`SC_COLUMNS_STOP` (`sc/source/core/data/table1.cxx`:655), which this tree already implements as
`SheetDecorationArea.StopAtEqualColumns`, is not even reached. What *does* widen it is the text
overflow of §1.

**(d) "The characters are now nearly exact — what is left is purely that we spend 27 more pages on
the same text."** True as far as it goes, and the two halves are the same defect seen twice: the
text that made the character count right is the text whose *width* was deciding the page count.

**(e) "`EASA-IFP-147Scope` and `ans_mappings_of_eccairs_terms` … page 1 identical in every
measurable quantity; divergence is later."** Correct, and the reason is now named: page 1 is inside
the first 200 rows, where both renderers recompute the row height. The divergence begins at row
201 and is visible as a pitch change in the reference's own PDF.

**(f) "A pagination defect can be a font defect — rule out the face before blaming the geometry."**
Ruled out rather than assumed here: on `Laser Report` the two renderings agree line for line to
0.1 pt on pages 1 to 4 and diverge only where the reference's *row pitch* steps from 15.0 to 15.75,
which no face can do.

**And one thing this round got wrong about itself, recorded because it cost a surprise.** The first
cut of `census.py` counted only a raw newline *inside* a `text:p` and so found 13 documents. A cell
with several `text:p` children is equally a multi-paragraph edit cell, and counting both gives
**33 documents, 649 raw-newline cells and 1246 several-paragraph ones**. Two of the documents that
moved for the worse (§5) hold no raw newline at all and were invisible to the first census.

## 5. Eight renderings moved without gaining a verdict, and two of them are worse

| document | before | after | reference |
|---|---|---|---|
| `Part_375_Operators` | `pages,words` 17 | `words` **19** | 19 |
| `Part_129_Operators` | 20 | **22** | 23 |
| `Special-Procedures_2025-07-10` | 20 | **21** | 22 |
| `arp-sop-300-Exhibit-A-Table-Templates` | 17 | **15** | 11 |
| `070_Equipment_inventory_list` | 1211 glyphs | **1175** | 1028 |
| `047_Date_tracker_Gantt_chart` | 3551 glyphs | 3556 | 3538 (still a match) |
| **`SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA`** | 7 | **11** | 4 |
| **`EHEST-Pre-departure-checklist-Rev.-1-06-12-2016`** | 22 | **27** | 24 |

**The last two are the cost of §3 and the cause is a reader defect this round did not create.**
Neither holds a raw newline, a `text:line-break` or a row style stating both a height and the
optimal flag — so none of the three rules can reach either of them through a *cell*. What they hold
is a `draw:custom-shape` text box anchored in a cell, whose paragraphs
`OdfContentReader.ReadShape` appends to the cell it is anchored in; `ContentTableCell.GetText()`
then joins them with newlines and `SheetOptimalRowHeights` measures a fourteen-paragraph text box
as a fourteen-line cell. Confirmed by re-rendering each with both binaries: 22 → 27 and 7 → 11,
reproducibly.

**Reach of that amplification: 253 such cells in 46 of the 307 `.ods`**
(`census.py`'s `shapecells` column, and `census-shapetext.txt`), of which exactly two documents
change a page count. A shape is not a cell in Calc — `ScColumn::GetOptimalHeight` walks the
column's cell storage and a `draw:` object is in the drawing layer — so the fix is to stop the
shape's text reaching the cell's, and it belongs in the reader rather than here. **Left, with the
seat named**: `OdfContentReader.ReadShape` (`dotnet/src/XRay.Office.OpenDocument/OdfContentReader.cs`)
appends into `target` with no marker a layout could filter on, and extraction wants that text
where it is.

## 6. The original `.xlsx`/`.xls` track does not move

`render-hash.sh` renders a track with one binary and records pages, alphanumeric characters and an
md5 of the PDF with its `/CreationDate` and XMP `dc:date` masked, under a fixed
`SOURCE_DATE_EPOCH`, deleting each PDF as it goes. Run over `/home/user/sample-files/sheets` — 307
`.xls`/`.xlsx`/`.xlsm` — with a copy of the binary built at `5c003d58c` and with this round's:
`orig-base.tsv` and `orig-after.tsv` are **identical on all 307 rows**, with no failures on either
side.

That is what the gating predicts. All three seats are behind the ODF reader:
`OdsPrintSetup.RecalculatedRowLimit` is in the ODF print-setup reader, and both layout changes are
gated on `SheetLayout.CellBreaksStartLines`, which only `OdsSpreadsheetDocument` sets — with it
false, `WidestParagraph` measures the whole string exactly as before and `ParagraphsOf` answers
one.

## 7. What is left, with its seat

- **A shape's text reaches the cell it is anchored in** (§5). 253 cells in 46 documents; two page
  counts. `OdfContentReader.ReadShape`.
- **`SSRO` 11 against 4 and `EHEST` 27 against 24** are the two witnesses for it and are the only
  rows this round made worse.
- **The remaining 15 pagination rows.** Six `pages` and nine `pages,words`. The largest are
  `EHEST` (27/24), `arp-sop-300` (15/11), `SSRO` (11/4), `Part_129_Operators` (22/23) and
  `RMP 2011-2014 and Inventory` (36/38) — the last of which is a *one-page* difference of a
  different kind: the two renderings agree line for line on every page, and the reference simply
  emits an extra 18-line page 22 before its second sheet where we do not.
- **The sheet reader still reads only `draw:frame`.** 380 `draw:custom-shape`, 148 `draw:a`,
  78 `draw:control`, 31 `draw:connector` and 17 `draw:line` sit directly in cells of this column
  and none of them is drawn — no fill, no outline, no text. Round 77 left it and it is still open;
  §5 shows that the *extraction* side of the same shapes is read, which is why the height defect
  above is reachable at all.
- **The 46 character-count rows.** Untouched by this round except incidentally. The extremes are
  `TICAPCapability_Final.ods` at 21 721 of 27 777 and
  `underlying-holdings-…-state-street-emu-esg-screened-index-equity-fund.ods` at 24 660 of 23 820;
  the middle of the table is dominated by `chartset-*` documents, which is a chart question rather
  than a sheet one.
- **The performance tail.** `Global_Market_Forecast` takes 215 s against the gate's 240 s bound and
  is a `match` when it finishes. Round 75 recorded eight of 307 over 100 s; that is unchanged.

## Files

| | |
|---|---|
| `sweep-ours.sh` | render our half of the `.ods` column and score against a banked reference (round 77's, unchanged) |
| `score.sh` | score two directories of already-rendered PDFs with the gate's own rule |
| `render-hash.sh` | render a track and hash it, one PDF at a time, for a two-binary comparison |
| `compare.py`, `moved.txt` | diff two row files: verdict counts and every row that moved |
| `cis-variants.py`, `refpages.sh` | author one-attribute variants of `CIS_Debian` and render them at 26.2.4.2 |
| `cis-variants.tsv` | the variant table §1 is established on |
| `census.py`, `census-breaks.txt` | multi-paragraph cells per document, by spelling, wrap and whether a shape produced them |
| `census-shapetext.txt` | cells holding a `draw:` shape of more than one paragraph (§5) |
| `rowlimit-census.py`, `census-rowlimit.txt` | rows past the 200-row limit whose stated height Calc keeps |
| `before.tsv`, `after.tsv` | the column at `5c003d58c` and at this round's HEAD |
| `orig-base.tsv`, `orig-after.tsv` | the original sheets track, both binaries |
