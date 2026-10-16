# round 77 — the `.ods` column: what a Calc cell's own text is, and where its pictures were

`.ods` **229 → 234 of 307**, measured against LibreOffice **26.2.4.2** on the converted corpus.

## Environment

| | |
|---|---|
| tree | `/home/user/wt-odspage`, branch `agent/odspage`, base `584ba83f9` |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything here** |
| corpus | `/home/user/corpus-odf`, 26.2.4.2's own conversion of the 947-document corpus; the `.ods` column is 307 files |
| reference bank | `/home/user/gate-odf-r76/ref`, rendered at 26.2.4.2 |
| original track | `/home/user/sample-files/sheets`, 307 `.xls`/`.xlsx`/`.xlsm` |
| C++ read-only | `/home/user/libreoffice-core` |

`sweep-ours.sh` re-renders our half alone and applies `batch-check.sh`'s verdict rule — the
same `awk` band, the same floor of 15 alphanumeric characters — to the banked reference bytes.
`score.sh` does the same over two directories of PDFs that already exist.

**The base was re-derived rather than trusted.** `score.sh` over `/home/user/gate-odf-r76/ours`
and `.../ref` reproduces `dotnet/probes/odf-gate-r76/rows.tsv`'s `.ods` half **row for row, 307
of 307, with a zero-line diff**: 229 match, 39 pagination rows (28 short, 11 long), 39
character-count rows. Every figure the brief quotes about the column's shape is confirmed;
the two per-document counts it quotes are `rawwords` (column 8) rather than `glyphs` (column 9),
which is why `CIS_Debian` reads 7581/13492 there and 38941/76210 here.

`before.tsv` is that base. `after.tsv` is the same 307 documents rendered at this round's HEAD
and scored against the same reference bytes.

## The column

| | match | pages | pages,words | words | ours-failed |
|---|---:|---:|---:|---:|---:|
| before, `584ba83f9` | **229** | 25 | 14 | 39 | 0 |
| after | **233** | 27 | 10 | 36 | 1 |
| after, the timeout scored on its own | **234** | 27 | 10 | 36 | 0 |

Ten verdicts moved and nine of them improved:

| | before | after | |
|---|---|---|---|
| `DynamicBubbleChart.ods` | words | **match** | 1662 → 1720 of 1728 glyphs |
| `047_Date_tracker_Gantt_chart` | words | **match** | 3193 → 3551 of 3538 |
| `049_Expenses_calculator` | words | **match** | 1487 → 1526 of 1548 |
| `016_Free_Organizational_Chart_Template` | pages | **match** | 3 → 4 pages of 4 |
| `TK-Syllabus-Comparison-Document-v2` | pages | **match** | 1231 → 1235 pages of 1235 |
| `CIS_Debian_Linux_8_Benchmark_v1.0.0` | pages,words | pages | **38 941 → 76 633 of 76 210 glyphs** |
| `SIL_TDB648` | pages,words | pages | 60 → **92** pages of 88; 30 069 → 30 809 of 30 888 |
| `SIL_TDB605` | pages,words | pages | 18 070 → 18 616 of 18 693 |
| `SIL_TDB609` | pages,words | pages | 21 995 → 22 541 of 22 688 |
| `Global_Market_Forecast_2016-2035_Airbus_Data_Set` | match | ours-failed | **a timeout, not a defect — see below** |

Eighteen more renderings moved without changing a verdict — **11 towards the reference and 7
away**. The seven lose 1, 1, 2, 8, 12, 133 and 445 alphanumeric characters of agreement against
gains of 1, 4, 9, 56, 111, 191, 299, 301, 426, 460 and 516; `after.tsv` beside `before.tsv` has
every one. The two large losers are `orbus_togaf_tool_csq.ods`, already 1418 characters *over*
the reference and now 1863 over, and `ans_mappings_of_eccairs_terms.ods`, 23 over and now 156
under.

**The `ours-failed` row is the 240 s gate bound and not this round.** Rendered on its own the
document takes **210 s** and scores 567 pages of 567, 305 549 glyphs of 305 549, no unembedded
font — a `match`. Timed with the change backed out it takes **223 s**, so the change made it
*faster*, not slower, and it is simply a document that sits inside a fifth of the bound with
two other rounds' sweeps running beside it. `dotnet/CLAUDE.md` already records that eight of
the 307 render in over 100 s; this is the first one to cross.

## 1. A Calc cell's `text:p` is taken exactly as written, and the other two ODF applications' is not

**Rule.** ODF's text content model is shared by Writer, Calc and Impress; its *importer* is not.
A spreadsheet cell's paragraphs are read by Calc's own filter rather than by `xmloff`'s text
import, and `ScXMLCellTextParaContext::characters` appends the characters it is handed with **no
normalisation at all** — `maContent += rChars;`, `sc/source/filter/xml/celltextparacontext.cxx`:36-39.
So leading, trailing and repeated spaces survive, and a raw `U+000A` inside a `text:p` is a
**paragraph break**: `ScXMLTableRowCellContext::PushParagraphEnd` hands such a string to the
EditEngine because `ScStringUtil::isMultiline` is a search for `\n` or `\r`
(`sc/source/core/tool/stringutil.cxx`:426-429) and that is one of the two conditions that make a
string cell an edit cell (`sc/source/filter/xml/xmlcelli.cxx`:625-629).

**Measured**, one flat ODF sheet of eight one-cell rows, none of them wrapping, 26.2.4.2's own
PDF read with PyMuPDF (`ws.fods`, kept here; the fixture
`dotnet/tests/corpus/features/sheet-cell-verbatim-text.fods` is the trimmed form):

| cell | 26.2.4.2 draws |
|---|---|
| `ABC` | one line, 20.55 pt wide |
| `  ABC` | one line, **26.10 pt** — the two spaces are in the ink |
| `ABC  ` | one line, **26.10 pt** — so is the trailing pair |
| `A   B` | one line, 21.64 pt |
| `ABC\nDEF` | **two lines**, 11.20 pt apart |
| `ABC\r\nDEF` | **two lines** — CRLF is one break |
| `ABC\n  DEF` | two lines, the second keeping its two spaces |

This tree collapsed all of it. `OdfContentReader.CellTextIsVerbatim` is the switch; only
`OdsReader` sets it, and `ReadCell` turns it on around the cell's own blocks alone, so a shape
anchored in a cell still goes through `xmloff`'s rule. After the change our rendering of that
probe agrees with 26.2.4.2 on all seventeen drawn lines, to 0.01 pt in position and 0.05 pt in
width.

**Reach: 659 paragraphs in 16 of the 307 `.ods` hold a raw newline** (`census.py … newline`,
`census-newline.txt`), 361 of them in `CIS_Debian_Linux_8_Benchmark_v1.0.0.ods`. The whole
column holds **40** `text:line-break` elements against those 659, so this is the spelling
LibreOffice's own conversion actually writes for a multi-line cell and the ODF-namespace one is
the rarity. That is the same trap `dotnet/CLAUDE.md` records for `drawooo:display` and
`text:line-break`, arriving through a third door: not a wrong prefix and not a missing
attribute, but **a plain text node that means something different in one of the three
applications**.

**Cost, and it is the other half of the reach figure**: 160 422 paragraphs in **210 of the 307**
differ between the collapsed and the verbatim readings by ordinary white space rather than by a
newline. That is a wide blast radius for one line of reader code, and it is why the sweep was
run whole rather than over the sixteen.

### 1b. A hard break starts a line in a cell that does not wrap — in ODF, and only in ODF

`SheetTextLayout` already sent a *wrapping* cell's text through the breaker and `LineCount`
already split on `\n` first. A cell that does not wrap took neither path, which is right for the
two Excel families and wrong for ODF, and this tree said so: `SheetOptimalRowHeights` carried
*"ODF is the one importer that disagrees … the sheets track holds no `.ods`, so this is an
unmeasured deviation"*, and `SheetHardBreakTests.ANonWrappingCellStillLosesItsBreaks` asserted
the gap and named itself *"the assertion to delete rather than the one to keep passing"*.

It is now measured and deleted. The rule is the **importer's** and not the cell's: BIFF and
SpreadsheetML put the EditEngine into single-line mode for a cell whose format does not wrap
(`bSingleLine = !pXF->GetLineBreak()`, `sc/source/filter/excel/xihelper.cxx`:246-256;
`rEE.SetSingleLine(bSingleLine)`, `sc/source/filter/oox/worksheethelper.cxx`:1607-1611), and
under `EEControlBits::SINGLELINE` `ImpEditEngine::ImpInsertText` never looks for a separator —
`nEnd = !maStatus.IsSingleLine() ? aText.indexOf(LINE_SEP, nStart) : -1`,
`editeng/source/editeng/impedit2.cxx`:2876-2877, so the break stays in the string as a character
and starts nothing. **Calc's ODF filter never calls `SetSingleLine`.**

`SheetLayout.CellBreaksStartLines` carries it, `SheetTextContext.BreaksStartLines` takes it to
the draw, and such a cell is broken against an unbounded measure so that only the paragraph ends
a line — Calc reaches the same place by giving it a paper it can never fill. On
`sheet-cell-hard-break.fods`, re-rendered at 26.2.4.2 for this round, **all 22 of its lines are
where the reference puts them, to 0.03 pt**, row 2's three included; 26.2.4.2 and 24.2.7.2 agree
on that file line for line, so the figures the fixture has carried since it was written stand.

`CIS_Debian_Linux_8_Benchmark_v1.0.0.ods` is what this is worth: **38 941 → 76 633 alphanumeric
characters against the reference's 76 210**, inside the band from 49 % outside it. Its page
count is unchanged at 88 against 61 and is a different defect — see §5.

## 2. A sheet's pictures were behind two wrappers, and the brief named only the second

The brief's item 1 says a framed picture vanishes because `draw:transform` is not parsed.
**That is true and it is not the first gate.** `OdsDrawings` walked a cell's own
`draw:frame` children, and on `SIL_TDB648.ods` **72 of the 74 frames are inside a `draw:g`** —
so a grouped frame stating a plain `svg:x` was missed too, and the transform never came into it.

Censused over the column (`census.py … frames`, `census-frames.txt`):

| | frames | documents |
|---|---:|---:|
| a cell's or `table:shapes`' own child | 386 | — |
| inside a `draw:g` | **89** | 3 |
| inside a `draw:a` | **33** | 13 |
| total | 504 | 147 |

`draw:a` is ODF's spelling for *this shape is a hyperlink* — a wrapper element rather than an
attribute — and it stands between a cell and 33 more frames in thirteen documents, one of them
`Application_Compliance_Checklist_5_Apr_2021.ods`. Neither wrapper has a rectangle of its own,
so a child's coordinates are the anchor cell's either way; `OdsDrawings.Frames` walks through
both. The group is deliberately **not** read as a drawing: ODF gives it no rectangle, its
children carry absolute coordinates in the anchor's space, and the `table:end-cell-address`
LibreOffice writes on one is its cached bounding anchor for the union that
`SheetDrawingArea` already takes from the children themselves.

The transform is the second gate and is real: **81 frames in 5 documents state
`draw:transform` and no `svg:x`** — and, counting every `draw:` element rather than frames
alone, **336 in 13 documents**, so the shapes this reader does not read at all (380
`draw:custom-shape`, 78 `draw:control`, 31 `draw:connector`, 17 `draw:line` at the top level of
a cell) carry most of the remainder. `OdfTransform` is the parser, moved out of
`OdpSlideLayout` into `XRay.Office.OpenDocument` verbatim for the reason `dotnet/CLAUDE.md` gives
for `XRay.Office.Ooxml/DrawingML`: it reads markup, depends on nothing above Core, and now serves
two families instead of one. `XRay.Office.Presentations.Tests` is 982 of 982 before and after the
move.

**What a turned frame's rectangle is, and why it is the box rather than the shape.** The
transform maps the frame's own `svg:width` × `svg:height` rectangle; the drawing takes the
axis-aligned **bounding box** of that, because both of Calc's page-deciding questions ask for
it — `ScDrawLayer::GetPrintArea` widens the printed block to cover every object
(`sc/source/core/data/drwlayer.cxx`:1400-1424) and `ScDocument::HasAnyDraw` keeps a page an
object overlaps (`documen9.cxx`:382-404), and both go through `GetCurrentBoundRect`. The
picture's own size and angle ride inside it as a `SheetDrawingPart`, which is the shape the
SpreadsheetML reader has produced for a grouped, turned watermark since the round that wrote
`SheetDrawingBounds` — so nothing new was needed downstream.

Checked twice against 26.2.4.2:

- `sheet-grouped-frames.fods` (the fixture): three pictures at (74.69, 74.67), (74.69, 182.67)
  and (128.69, 272.67)–(289.39, 407.03). Ours are within **0.02 pt** on all three, the turned
  one's box included — 160.70 × 134.36 pt, which is `2·cos30 + 1·sin30` and `2·sin30 + 1·cos30`
  inches for a 2 × 1 in frame at `rotate (0.5235987755982988)`.
- `SIL_TDB648.ods` itself: page 4 of the reference draws three copies of the watermark
  439.2 × 283.3 pt, ours 440.8 × 282.5, and the vertical distance between two of them is
  **325.4 pt on both sides** — the difference between their two `translate` pairs exactly.

**And the reference does not turn the picture at draw time; it turns the pixels.** 26.2.4.2's
content stream places the fixture's turned picture with `160.696 0 0 134.362 128.693 434.862 cm`
— an axis-aligned matrix over a bitmap it has already rotated into the box — where we emit a
rotation and the shape's own 143.99 × 71.99 box inside it. Same ink, same box, different
representation; worth knowing before anyone compares content streams.

`SIL_TDB648.ods` goes **60 → 92 pages against 88**, from 28 pages out to 4.

## 3. What the brief got wrong

1. **"`draw:transform` is not parsed by the sheet reader, so a framed picture vanishes … 72 of
   its 74 `draw:frame` state `draw:transform` and no `svg:x`."** The first clause is true and
   the second is the wrong cause for that document: those 72 frames are inside a `draw:g`, and a
   walk that reads only a cell's own children misses them whatever they state. Both gates had to
   go; the group is the one that closes `SIL_TDB648`, and `draw:a` — which the brief does not
   mention at all — is worth 33 more frames in thirteen documents.
2. **"Reach: 5 documents, 81 frames of 504."** Right for `draw:frame` and short for the
   question: **336 shapes in 13 documents** state a transform once `draw:custom-shape`,
   `draw:g`, `draw:line`, `draw:connector` and `draw:control` are counted, and the sheet reader
   reads none of those element kinds at all.
3. **"`CIS_Debian…` 7581 characters against 13492, so we are also losing more than a third of
   the text. Two defects or one."** The figures are the `rawwords` column; on the column the
   gate scores it is 38 941 against 76 210, so the loss was **49 %** rather than a third. And it
   is **two** defects: the text was the cell reader (§1, now 76 633) and the pagination is
   untouched at 88 against 61 (§5).
4. **"`SIL_TDB648.ods` … is not a cell-overflow document: page 1 is identical on both sides."**
   Confirmed, and worth keeping — the previous round's refutation stands.
5. **The residue counts and the split — 39 pagination rows, 28 short and 11 long, 39
   character-count rows — are exact.** Re-derived from the bank with a zero-line diff.

## 4. What was refuted inside this round, and cost a rebuild to find

**A multi-paragraph cell does not make its row taller, and inferring that it does is wrong on
real documents.** The natural companion to §1b is that if the drawing takes N lines the optimal
height should too — `ScColumn::GetNeededSize` measures an edit cell through
`pEngine->GetTextHeight()` with a paper a million units wide (`column2.cxx`:487-491, 519), which
for N paragraphs is N lines. Implementing that moved
`Capability_List_9-14-2022_Dallas_Combined-Aircraft_Manuf_unsorted.ods` from the reference's
**147 pages to 150**, and the reference's own page 11 says why: it draws
`19090-105 (SCD` at 239.26 and `604-85001-23)` at 252.75 inside a row that spans **238.52 to
252.75**, so the second line is painted over the row beneath and the row kept its stored
14.23 pt.

The cause is a rule this tree does not model and should: **an ODF sheet's automatic row heights
are recalculated for its first 200 rows only.** `ScXMLTableRowContext` excludes a block ending
past row 200 from the recalc ranges whenever its style carries a stored height and the optimal
flag —

```cpp
if (nCurrentRow > 200 && ptmpStyle && !ptmpStyle->FindProperty(CTF_SC_ROWHEIGHT))
{
    XMLPropertyState* pOptimalHeight = ptmpStyle->FindProperty(CTF_SC_ROWOPTIMALHEIGHT);
    if (pOptimalHeight && ::cppu::any2bool(pOptimalHeight->maValue))
        rRecalcRanges.at(nSheet).maRanges.setFalse(nFirstRow, nCurrentRow);
    else
        rRecalcRanges.at(nSheet).maRanges.setTrue(nFirstRow, nCurrentRow);
}
```

`sc/source/filter/xml/xmlrowi.cxx`:218-243, with the comment *"recalc only the first 200 row in
case of optimal document loading"*. The test fires for exactly the style Capability_List's rows
use, because `ScXMLRowImportPropertyMapper::finished` removes `CTF_SC_ROWHEIGHT` from a style
that states a height **and** the optimal flag, passing the height through as the optimal one
(`sc/source/filter/xml/xmlstyli.cxx`:245-258) — so `FindProperty(CTF_SC_ROWHEIGHT)` is null and
the exclusion applies.

Established rather than assumed, on four probes: a four-row `.fods` whose second row holds three
paragraphs gets **all three lines** from 26.2.4.2 (row 3 starts at 105.15 rather than 70.54 +
one pitch); adding `style:row-height="0.198in"` to it changes nothing; hand-packaging the same
content as a real `.ods` changes nothing; and Capability_List, whose identical rows sit past row
200, keeps its stored height. So it is the row *index* and not the packaging, the stated height
or the flag.

**Left, deliberately.** `SheetOptimalRowHeights` recalculates every optimal row, so the two
rules have to land together or not at all, and the second one changes the height of rows on most
of the column. `SheetVerbatimCellTextTests.TheParagraphsAreDrawnALineApartAndTheRowKeepsItsHeight`
pins both halves — the three paragraphs where 26.2.4.2 draws them, and our row height as the
measurement it is, with the seat and the figure to assert instead when the rule lands.
`dotnet/tests/corpus/features/sheet-cell-break-height.fods` is the fixture.

**A second suspect was refuted on the way and should not be re-derived.** *"Our wrap treats a
trailing space as content where Calc's does not, which is why the taller rows appeared."* It
does not: five wrapping cells — plain, three trailing spaces, eight trailing spaces, three
leading spaces, three inner spaces — in a column too narrow for them agree with 26.2.4.2 on
**all five**, to 0.01 pt in every drawn line's position.

## 5. What is left, with its seat

- **`CIS_Debian_Linux_8_Benchmark_v1.0.0.ods`, 88 pages against 61.** The text is closed; the
  pagination is a *column band* question and untouched. Both sides emit long runs of pages with
  **zero ink** — the reference 40 of its 61 and we 67 of our 88 — and both put the same 21
  pages of content in the same order, 15 then 6. The sheet declares 11 used columns and then 246
  more carrying a default cell style and nothing else, so the difference is how far right the
  used area reaches over styled-but-empty columns and how many of the resulting empty pages
  survive `SkipEmpty`. `dotnet/src/XRay.Office.Spreadsheets/TODO.md`'s *"the used area counts
  cells with content only"* is the seat.
- **`SIL_TDB648.ods`, 92 against 88.** Four pages long where it was 28 short. The watermark's
  bounding boxes agree with the reference to about 1.5 pt each, so the residue is which of them
  reach past the last cell rather than whether they are there.
- **`afn-afn-20250801` 282/270, `ans_mappings_of_eccairs_terms` 170/179,
  `EASA-IFP-147Scope_WEB…` 83/94** and the rest of the 27 pagination rows are untouched by this
  round.
- **The 200-row recalculation rule**, §4.
- **The sheet reader reads only `draw:frame`.** 380 `draw:custom-shape`, 148 `draw:a`, 78
  `draw:control`, 31 `draw:connector` and 17 `draw:line` sit directly in cells of this column and
  none of them is read — no fill, no outline, no text. That is the ODF twin of *"a worksheet
  shape's fill and outline are not read at all"* in `dotnet/CLAUDE.md`, and no gate column can
  see it.

## The original track does not move

`render-hash.sh` renders a track with one binary and records pages, alphanumeric characters and
an md5 of the PDF with its `/CreationDate` and XMP `dc:date` masked, deleting each PDF as it
goes so the disk cost is one file rather than a track's worth. Run over
`/home/user/sample-files/sheets` — 307 `.xls`/`.xlsx`/`.xlsm` — with the binary built at
`584ba83f9` and with this round's, `orig-base.tsv` and `orig-after.tsv` are **identical on all
307 rows**.

That is what the gating predicts and the reason it is worth stating: every seat this round
touches is behind `SheetLayout.CellBreaksStartLines`, which only `OdsSpreadsheetDocument` sets,
or inside `OdsDrawings` and `OdfContentReader.CellTextIsVerbatim`, which only `OdsReader`
switches on. `SheetTextLayout.Place`'s new branch is dead when `BreaksStartLines` is false, and
the measure it passes to `Wrap` is then the one it passed before, character for character.

## Files

| | |
|---|---|
| `sweep-ours.sh` | render our half of the `.ods` column and score against a banked reference |
| `score.sh` | score two directories of already-rendered PDFs with the gate's own rule |
| `render-hash.sh` | render a track and hash it, one PDF at a time, for a two-binary comparison |
| `census.py` | the three censuses: raw newlines, how a frame is wrapped, `draw:transform` |
| `before.tsv`, `after.tsv` | the column at `584ba83f9` and at this round's HEAD |
| `census-newline.txt`, `census-frames.txt`, `census-transform.txt` | their output |
| `orig-base.tsv`, `orig-after.tsv` | the original sheets track, both binaries |
| `ws.fods` | the eight-row white-space probe §1 was established on |
