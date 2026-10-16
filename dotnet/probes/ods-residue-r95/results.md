# Round 95 — the four seats round 92 left, and two of the four premises were wrong

    ours   = XRay.Office.Cli @ 9814327c8 (base) and @ 9814327c8 + this round's diff (head)
    ref    = /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2 0229ac93fcf0…
             plus the banked reference PDFs at /home/user/gate-odf-r80/ref (the 307 `.ods`)
             and /home/user/gate-orig-r83/ref (the 947 originals)
    corpus = /home/user/corpus-odf/sheets — 307 `.ods`, 26.2.4.2's own conversion of the corpus
             /home/user/sample-files/sheets — the same 307 documents as `.xlsx`/`.xls`/`.xlsm`
    fonts  = the tarball confounds as `probes/ods-track-r88` left them; none moved this round
    rule   = batch-check.sh of 2026-09-05 (column 9, `glyphs`, within max(2 %, 15)), transcribed
             into `sweep-ours.py` so our half alone can be scored against a banked reference
    load   = 5-minute load average 0.3–0.5 through every sweep quoted here; no other round was
             building or sweeping in this container while they ran
    date   = 2026-09-11

## 0. The headline

| seat | round 92 called it | what it is |
|---|---|---|
| **O3** ODF 1.2 `style:map` | *"reaches 1 of 307"* | **nil reach, 0 of 307** — the census counted number-format maps |
| **O4** BIFF `CONDFMT` | *"the two spellings disagree about a row height"* | **nil reach** — BIFF8 row heights are never recomputed, and the named witness is not BIFF |
| **O5** `017_Timeline_Templates` | *"a print-area extent"* — right, and now closed | **fixed**: a `draw:connector`'s rectangle is its two endpoints |
| **O6** `sistem-rekod-markah-srm` | *"a page-column question, not a vertical one"* | **re-seated, not closed** — it is a row height after all, 276 against 298 on exactly the conditional range's rows |

One document closes. **O6 does not reach either terminal state and this write-up does not pretend
it does**: what it has is a refuted seat, a mechanism narrowed to one contradiction, and the
experiment that would settle it (§4).

## 1. O4 — BIFF `CONDFMT` is unread, and it could not matter

**The record is real and this tree does not read it.** Walking every worksheet substream of the
corpus's 64 `.xls` (`condfmt-census.py`, which parses the record exactly as
`XclImpCondFormat::ReadCondfmt` does — `sc/source/filter/excel/xicontent.cxx`:516-524, a `ccf`
count, ten ignored bytes, then a `SqRef` of four 16-bit fields per range):

| document | CONDFMT records | ranges |
|---|---:|---:|
| `NPA_21_21_Sentenced_Comments.xls` | 3 | 6 |
| `Background_Declaration_Template.xls` | 23 | 23 |
| `Hazard Analysis Template.xls` | 1 | 2 |
| `TICAPCapability_Final.xls` | 2 | 4 |

**4 of 64, 29 records, 35 ranges** — and the parse is checked rather than asserted: 26.2.4.2's own
`--convert-to ods` of the same four workbooks writes those 35 ranges back out as
`calcext:target-range-address`, and the two lists agree **range for range, sheet for sheet, in
order, 35 of 35**.

**But a conditional format cannot move a BIFF row height, because BIFF8 row heights are never
recomputed.** `ImportExcel8::Read` holds its `AdjustRowHeight()` call inside an `#if 0` whose
comment is the whole rule — *"Excel documents look much better without this call; better in the
sense that the row heights are identical to the original heights in Excel"*
(`sc/source/filter/excel/read.cxx`:1284-1288) — while `ImportExcel::Read`, which is BIFF2 through
BIFF7, calls it unguarded (`:779-780`). So neither `EXC_DEFROW_UNSYNCED` nor a `ROW` record's own
`fUnsynced` decides anything on a BIFF8 sheet: nothing asks.

Measured at the reference rather than argued (`xls-rowflags.py`). A four-row BIFF8 workbook
26.2.4.2 itself wrote, whose `DEFAULTROWHEIGHT` `grbit` is zero and whose four `ROW` records
already carry `fUnsynced` clear, comes back from `--convert-to fods` at whatever its records
state:

| the file's `ROW` heights | 26.2.4.2 answers |
|---|---|
| 276 / 298 / 298 / 298 (as written) | 276 / 298 / 298 / 298 |
| patched to a uniform 255 | 255 / 255 / 255 / 255 |
| patched to a uniform **100** | 100 / 100 / 100 / 100 |

A hundred twips is a height no content could ask for. This tree already models it —
`SheetGrid.RowHeightsAreManual` is set for every BIFF8 workbook — and the remark on it now carries
this citation in place of the fitted *"recomputing without this cost eight `.xls` their page
count"* that stood there, which was the right answer for the wrong reason.

> **Correction, round 97.** The paragraph below is withdrawn. The reader it describes parses `CONDFMT` — a count and a `SqRef` of ranges — and never reads the `CF` records that hold the rules, so 64 of 64 byte-identical PDFs was a property of the instrument rather than a fact about the corpus. Asked of the reference instead, `--convert-to fods` at 26.2.4.2 resolves **29 conditional formats and 43 conditions across these four workbooks, 43 of 43 naming a style that carries a fill, a font colour or bold**. The seat is live again as O25; see `probes/biff-condfmt-r97`. The row-height half of this section — the `#if 0` around `AdjustRowHeight()` — is unaffected and stands.

~~**Reading the record anyway moves nothing, and that is measured.**~~ The reader was written,
verified against the reference's export, and then reverted: rendering all 64 `.xls` at the round's
base and with `CONDFMT` read, under `SOURCE_DATE_EPOCH`, one output directory per document, gives
**64 of 64 byte-identical PDFs**. Dead code with a proven reach of zero is not worth carrying, so
what the tree gained is the rule written down where the next round will meet it.

**And the witness round 92 named is not a BIFF file.** `Special-Procedures_2025-07-10.xls` opens
`PK\x03\x04`: it is an OPC zip wearing a `.xls` name — rule 4, *detect formats by content* —
so it goes through the SpreadsheetML reader, which has read `conditionalFormatting` since round 92.
Its `.xlsx`-path rows state no `ht` at all past row 6 and it was **22 pages of 22 before that round
as well as after**, which is why it did not move. There is no workbook in this corpus whose two
spellings disagree about a row height for want of `CONDFMT`.

## 2. O3 — the ODF 1.2 `style:map` spelling reaches nothing

**The figure that seated this counted the wrong element.** `style:map` is the conditional cell
format's element *and* the element a `number:*-style` uses for its positive, negative and zero
sub-formats. The discriminator is the parent and one attribute: a conditional cell format sits on a
`style:style style:family="table-cell"` and carries `style:base-cell-address`.

Censused that way over the 307 converted `.ods` (`stylemap-census.py`):

| | documents |
|---|---:|
| state both spellings | **53** |
| state a table-cell `style:map` and no `calcext:` | **0** |
| state `calcext:` alone | 42 |
| neither | 212 |

53 + 42 = 95, which reproduces round 92's `calcext:` count exactly, so the two censuses differ only
in what they call a `style:map`. **The document round 92 named,
`2025_Active_Civil_Airmen_Statistics_FINAL.ods`, states 23 number-format maps and no conditional
format at all** — which is also why the rule would not have closed its 39-pages-against-36.

**And where both spellings are present they are the same set of cells, cell for cell.**
`stylemap-cover.py` resolves every cell style carrying a conditional `style:map`, walks each table
honouring `number-columns-repeated` and `number-rows-repeated`, takes the effective style of every
cell (its own, else its row's default, else its column's) and checks the address against the union
of the `calcext:target-range-address` ranges: **53 documents, 0 cells outside**.

That is not an accident, and the exporter says why: `ScXMLExport::ExportConditionalFormat` walks
the sheet's one `ScConditionalFormatList` into `calcext:conditional-formats`
(`sc/source/filter/xml/xmlexprt.cxx`:4779-4800) and `ScXMLAutoStylePoolP::exportStyleContent`
writes a `style:map` onto every cell style the same formats reach
(`sc/source/filter/xml/xmlstyle.cxx`:700-810). One model, two spellings.

*The first cut of `stylemap-cover.py` reported 41 of 53 documents holding uncovered cells, and it
was the instrument: `'Idea planner'.F9:'Idea planner'.F12` torn in three by a plain `str.split(' ')`
because the sheet name holds a space. The quoted-split it uses now is the tree's own
`SheetAddress.SplitList` rule, and the correction is in the script's docstring.*

**Closed as nil reach — 0 of 307 — with the caveat this corpus always carries**: the column is
26.2.4.2's own export, so this is a census of what LibreOffice writes rather than of what ODF
permits. A producer that wrote the specification's spelling alone would not be read, and the
exporter citation above is why no such file is in reach.

## 3. O5 — the blank page is a connector, and its rectangle is two points

`017_Timeline_Templates_for_Excel_b88faee6.ods` printed 2 pages against 26.2.4.2's 3, and the extra
page is the second page of the `VerticalTimeline` sheet, which carries **one drawn object and no
text**. Its `.xlsx` twin was already 3 of 3.

**Three ODF drawing elements state no rectangle at all.** `draw:line`, `draw:connector` and
`draw:measure` state `svg:x1`, `svg:y1`, `svg:x2`, `svg:y2` and neither `svg:width` nor `svg:x`:
`SdXMLLineShapeContext::startFastElement` (`xmloff/source/draw/ximpshap.cxx`:1045-1097) takes the
smaller of each coordinate as the shape's position, puts the pair in the polygon and leaves the
stated size at 1 × 1, so the object's bounding rectangle is the polygon's; `SdXMLConnectorShapeContext`
(`:1963-2035`) sets `StartPosition` and `EndPosition` from the same four attributes. `OdsDrawings`
looked only for `svg:width`, so every one of them became a zero-sized box — and a zero-sized box
widens no print area.

That document's `Straight Connector 2` is the timeline's spine: anchored in A1, running from
`svg:y1="0.6965in"` to `svg:y2="15.7957in"`, on a sheet whose 66 rows of cells add up to 18 622
twips — **12.93 in** — so the line reaches 2.86 in below the last cell. The one-attribute variant
settles it with no arithmetic at all (`variants-ods.py`):

| | 26.2.4.2 prints |
|---|---|
| as it stands | **3 pages** — p1 660 characters, p2 **0 characters and 1 drawn path**, p3 552 |
| every `draw:connector` deleted | **2 pages** |

**A second thing had to go with it, and it is not a fine judgement.** Calc writes a connector's
cached `table:end-cell-address` as the cell the object is *anchored in* — for this connector, `A1`
with an `end-y` of `0.0394in` — so the two-cell branch collapses a fifteen-inch line to a point.
Segments are excluded from it exactly as turned shapes already were.

**A segment's ink is deliberately still not painted.** The only outline `SheetShapeInk` can paint
for a shape naming no preset is the box's, and a rectangle is not a line; the corpus's 30 elbow
connectors state their real path as an `svg:d` (`M1238 6853v1508h808v1507` on
`016_Free_Organizational_Chart`'s `Elbow Connector 46`), which this reader has no parser for. The
two straight-segment kinds the corpus does hold are axis-aligned, so their boxes are degenerate and
`SheetShapeInk.Draw` returns on them anyway. What is fixed is the *extent*, which is what a print
area is made of.

Reach (`lineshape-census.py`): **3 of the 307 converted `.ods`** hold one — 41 `draw:line` in
`apron-area.ods`, 30 `draw:connector` in `016_Free_Organizational_Chart`, 1 in the timeline — 72
elements, 28 of them diagonal, all 72 stroked. Afterwards:

| document | ours before | ours after | 26.2.4.2 |
|---|---|---|---|
| `017_Timeline_Templates_for_Excel_b88faee6.ods` | 2 pages / 969 glyphs | **3 / 969** | 3 / 969 |
| `016_Free_Organizational_Chart…ods` | 4 / 1419 | 4 / 1419 | 4 / 1419 |
| `apron-area.ods` | 3 / 1814 | 3 / 1814 | 3 / 1814 |

## 4. O6 — not a page-column question, and the row height it *is* is not yet explained

`sistem-rekod-markah-srm-_-rekod-master.ods` printed 22 pages against 26.

**Round 92's reading of it is withdrawn.** It is not a page-column question. Pairing every text
baseline of our page 1 against the reference's shows both sides drawing the same 41 rows at the
same *x*, and the reference's "spill" page 2 holds three rows at x 418–520 — **inside** page 1's
own 51–530 range, so it is a vertical continuation and not a second page-column. What differs is
the pitch: the forty student rows are **13.8 pt** in ours and **14.9 pt** in the reference — 276
twips against 298, the pair round 92 established for the arithmetic and the measured branch — and
40 × 1.1 pt is the 44.2 pt that costs three rows a page. The drift is uniform: the difference at
row *n* is n × 1.105 pt, exactly, from the first student row to the last.

`--convert-to fods` gives the reference's own answer for sheet `6A`:

    rows 0-1     276.0     the arithmetic
    row  2       323.1
    rows 3-42    298.2     one measured line
    rows 43-51   276.0     the arithmetic again
    rows 52+     300.0     stored, past the 200-row limit

Rows 4–43 in one-based terms is exactly that sheet's conditional range, `C4:T43`. So this *is* the
seat round 92 opened, arriving on a document it did not name.

**What is measured there is an 11 pt line, and nothing in the range is 11 pt.** Every cell of
C..T in those rows is Arial 9 — 440 `ce18`, 200 `ce22`, 40 `ce9`, 40 `ce14`, each stating
`fo:font-size="9pt"` and its asian and complex siblings — and 298.2 twips is one measured line of
an **11 pt** face, which is what the document's `Default` cell style states. Six one-attribute
variants at 26.2.4.2, all reading rows 3–42 of `6A`:

| variant | rows 3–42 |
|---|---|
| as it stands | 298.2 |
| `calcext:conditional-formats` deleted | 298.2 — **unmoved**, because the `style:map` half still declares it |
| every cell of C..T emptied, styles kept | **276.0** — the arithmetic |
| every cell of column A emptied | 298.2 |
| every cell of U..AD emptied | 298.2 |
| every `"9pt"` in the file rewritten `"6pt"` | 298.2 |
| every `"11pt"` rewritten `"20pt"` | **522.1**, against 489.3 — the 20 pt arithmetic — above and below |

So the content of C..T is *necessary* (emptying it drops the row to the arithmetic), the C..T
cells' own size is *not what is measured* (six point leaves 298.2 exactly), and the measured size
follows the **document default** (moving 11 pt to 20 pt gives 522.1, one measured line at 20 pt,
where the unconditional rows take the 489.3 arithmetic). Clearing each of the eighteen columns
C..T one at a time leaves 298.2 in all eighteen cases, so no single cell is the source.

**A rule was fitted to that and it is refuted, so it is not in this tree.** The candidate was
*a conditionally formatted cell is measured in the conditional style's font, and a conditional
style stating none answers the document default's* — which has a real seat: `GetNeededSize` builds
its font from `pCondSet = rDocument.GetCondResult(...)` before anything else
(`sc/source/core/data/column2.cxx`:283-292) and `lcl_populateresult` (`patattr.cxx`:607-630) takes
each font item from `pCondSet->GetItemIfSet`, whose default `bSrchInParent` would walk the applied
style's parent chain to `Default`. Putting `fo:font-size="20pt"` on `sistem`'s applied
`ConditionalStyle_2` — a style that otherwise states nothing but a colour — does move those forty
rows to 522.1, which looked like confirmation.

**The clean probe says otherwise, and it is the sharper instrument.** `fontsource.fods` is two
cells: a `Default` style at **20 pt** and an 11 pt non-wrapping cell, one row conditional and one
not. If the applied style's resolved font were used, the conditional row would be 522. 26.2.4.2
answers **276 for the plain row and 298.2 for the conditional one** — one measured line of the
cell's *own* 11 pt — with the condition false and again with it true. Three companion probes agree:
a wrapping conditional cell wraps exactly like its unconditional twin (865.2 either way, so the
applied style does not supply the wrap flag), and a 9 pt conditional cell — plain string, formula
string, or `#N/A` error result — stays at the 256-twip floor rather than rising to 298.

So on a hand-built document a conditional cell is measured in its own font, and on
`sistem-rekod-markah-srm` an Arial 9 conditional range is measured at the default's 11 pt. **Both
are measurements and they contradict, which is why nothing was changed.** Implementing the fitted
rule was tried and scored: it takes `sistem-rekod-markah-srm` to 26 pages of 26 and takes
`flightstandards-doc-Cross-reference-table_version02.ods` — 338 conditional formats over
*wrapping* cells — from 461 pages of 461 to **464**, so it is **+1 −1 on the gate** as well as
being the wrong rule.

**What the next round should run first**, because it is the one experiment that separates the two
observations: the probe differs from the document in that the document's C..T cells are *formula*
cells inside a range whose condition (`cell-content()<40`) fires on the empty mark columns, and in
that the sheet's own `Default` differs from the cells' style in face as well as size (Calibri
against Arial). Vary those two on the probe — a firing condition over a *mixed* range, and a
default face different from the cell's — before looking anywhere else. The seat is
`ScColumn::GetNeededSize`'s font construction, not `ScPrintFunc::CalcPages`, and that much is
settled.

## 5. Reach, cost and confinement

**What changed in the tree is one reader and two remarks.** `OdsDrawings.Endpoints` is the whole
of the behaviour change; `SheetGrid.RowHeightsAreManual` and `OdsConditionalFormats` gained the
citations §1 and §2 establish and compute nothing new. `OdsLineExtentTests` and
`features/sheet-line-extent.fods` are the new coverage — four assertions on a fixture whose two
page counts, with and without the `draw:connector`, were read off 26.2.4.2 before anything was
asserted.

**Both sheets columns, scored by the gate's own rule against the banked 26.2.4.2 reference**
(`sweep-ours.py`; our half rendered at each binary, the reference bytes reused, which is sound
because the diff is confined to `dotnet/src` and cannot reach `soffice`):

| | base | head | moved |
|---|---:|---:|---|
| `.ods`, 296 comparable of 307 | 266 | **267** | 1 gained, 0 lost, **1 row's columns moved** |
| `.xlsx`/`.xls`/`.xlsm`, 307 | 294 | 294 | **0 rows moved** |

The eleven `.ods` rows excluded are the eleven the r80 bank has no reference PDF for; they are
excluded identically in both legs, so the totals are comparable without any further argument.
The one mover is `017_Timeline_Templates_for_Excel_b88faee6.ods`, 2/3 → 3/3 pages at 969 glyphs
either way.

*These absolute figures are not comparable with round 92's 274 → 278: that round rendered its own
reference, this one reuses r80's bank, and the eleven missing rows are the difference. Base against
head at one bank with one scorer is the comparison that means anything, and it is the table above.*

**Confinement, shown rather than asserted.** 128 words and slides documents — every ninth
`.docx`/`.doc`/`.pptx`/`.ppt` of the original corpus and every seventeenth `.odt`/`.odp` of the
converted one — rendered at both binaries under `SOURCE_DATE_EPOCH`, with the changed project's
`obj` and `bin` cleared before each build: **128 of 128 byte-identical, with nothing masked**.
`render-ours.sh`, `confine.list`.

**Every PDF this round produced was checked for `%%EOF` before it was deleted** — 256 of 256 on
the confinement pair and 128 of 128 on the `.xls` reach pair — because a truncated render scores as
a real difference rather than erroring, and `/` was between 6.6 and 8.2 GB free throughout.

**Tests.** Ten non-fidelity projects, run individually: 109 / 521 / 259 / 146 / 1044 / 164 / 1249 /
728 / 302 / 1864, **0 failed and 0 skipped in every one**. `XRay.Office.Fidelity.Tests` is at its
briefed baseline (§ below). Solution build 0 warnings, 0 errors.

## Files

| file | what it is |
|---|---|
| `condfmt-census.py` | BIFF `CONDFMT` over the corpus's `.xls`, parsed as `XclImpCondFormat::ReadCondfmt` parses it |
| `condfmt-census.tsv` | its output — 4 of 64 |
| `xls-rowflags.py` | clears `EXC_DEFROW_UNSYNCED` and every `ROW`'s `fUnsynced` and sets a uniform height, so the reference has to recompute if it ever would |
| `stylemap-census.py` | separates a conditional cell `style:map` from a number-format one |
| `stylemap-census.tsv` | its output — 0 documents state the cell form alone |
| `stylemap-cover.py` | whether the `calcext:` ranges cover every cell a `style:map` reaches |
| `stylemap-cover.tsv` | its output — 53 documents, 0 uncovered cells |
| `lineshape-census.py` | `draw:line`/`draw:connector`/`draw:measure` in cells |
| `lineshape-census.tsv` | its output — 3 documents, 72 elements |
| `condfont-census.py` | whether the style a conditional format applies ever states a font size |
| `condfont-census.tsv` | its output — 0 of 1971 |
| `variants-ods.py` | one-attribute variants of a packaged `.ods`, rendered by 26.2.4.2 |
| `sweep-ours.py` | our half of a column, scored against a banked reference by the gate's own rule |
| `render-ours.sh` | our half of a document list under `SOURCE_DATE_EPOCH`, for byte comparisons |
| `xls.list`, `confine.list` | the document lists those two were run over |
| `head-ods-rows.tsv`, `base-ods-rows.tsv` | the `.ods` column at both binaries |
| `head-orig-rows.tsv`, `base-orig-rows.tsv` | the same documents as `.xlsx`/`.xls`/`.xlsm` |
| `compare.py`, `compare-ods.txt`, `compare-orig.txt` | the joins, excluding any row that failed on either side in either run |
