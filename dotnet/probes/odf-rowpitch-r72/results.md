# The `.ods` remainder: what the row pitch turned out to be, and what it was not

## Environment

    ours   = XRay.Office.Cli at agent/odsrow, base 6ab0681b9
    ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 -- the build that wrote
             /home/user/corpus-odf, so both halves of every comparison are one build's artefacts
    bank   = /home/user/odsgap-work/bank/ref, reused; nothing in this diff can reach soffice
    fonts  = system fontconfig, all four tarball confounds moved aside
    rule   = batch-check.sh of 2026-09-05: pages exact, alnum characters within max(2%, 15)
    date   = 2026-09-07

## The result

| | base `6ab0681b9` | + allocated columns | + font level | of |
|---|---:|---:|---:|---:|
| `.ods` | 202 | 201 | **207** | 307 |
| `.odt` | 259 | 259 | **261** | 338 |

Both halves re-rendered in full at each of the three trees, scored against the banked 26.2.4.2
reference with `batch-check.sh`'s own rule. 53 rows move in all — 42 `.ods` and 11 `.odt` — of
which ten gain a match and three lose one. The original words and sheets tracks are byte-identical
across all 645 documents under a fixed `SOURCE_DATE_EPOCH`.

## 1. The handed-over diagnosis was right about one document and wrong about the group

The brief this round inherited said the 40 `ods pages-short` rows are **a row-pitch difference**,
generalised from `Aviation_Abbreviations.ods`, where we pitch rows at 12.78 pt against 26.2's
13.78 -- "exactly 20 twips, which is `2 x ATTR_MARGIN`".

**Measured over all 40, 27 have a page-1 row pitch identical to the reference's** (`pitch.py`,
median of the gaps between consecutive text baselines on page 1 of the two banked PDFs). Eight are
short, two are long, three could not be measured. So the group is not one cause and the pitch is
not what most of it is about.

Worse for the stored figure: of the 27, **ten have a page 1 that is identical in every quantity the
instrument can take** -- same number of rows, same first baseline, same last baseline, to a
hundredth of a point -- while their totals differ by up to 21 pages
(`EASA-IFP-147Scope_WEB` 83/94, `Laser Report 2024 FOIA` 486/506, `aircraft_analysis-2018-06`
38/42, `TK-Syllabus-Comparison-Document-v2` 1231/1235). Their divergence is *later in the
document*, and on `aircraft_analysis-2018-06` the first differing page is **page 4**, where we fit
157 lines and the reference 154. That is a few individual rows being short, not a pitch.

## 2. And the 20 twips were a coincidence

`Aviation_Abbreviations.ods` really does pitch at 256 twips where 26.2 pitches 276, and
`trunc(220 x 1.18) + 40 - 23 = 276` really is the reference's number. But 256 is not that formula
with half the margin. **256 is `ScGlobal::nStdRowHeight`, the sheet's optimal minimum**, which is
what an 11 pt arithmetic height is *floored to* when nothing asks for more --
`GetOptimalMinRowHeight` (`sc/inc/table.hxx`:882-887) returns it whenever the sheet states no
default of its own, which is every `.ods` (only the OOXML filter calls `SetOptimalMinRowHeight`,
`sc/source/filter/oox/worksheethelper.cxx`:965). The two numbers differ by 20 because 276 - 256 is
20, and for no other reason.

What actually decides it is **which columns the row is measured across**.

- `ScTable::aCol` holds only the columns that have been *allocated*, and anything applying a
  pattern to a column allocates it -- a cell that states a format and holds no value exactly as
  much as one holding a value.
- `GetOptimalHeightsInColumn` (`sc/source/core/data/table1.cxx`:88-127) walks **every allocated
  column**, not the print area's columns, and `ScColumn::GetOptimalHeight`
  (`column2.cxx`:899-1110) writes each pattern's arithmetic height over the whole row range that
  pattern covers. A column holding nothing carries the sheet's *default* pattern over every row.
- So one formatted blank cell anywhere makes the default cell style's font a floor for every row
  of the sheet.

`Aviation_Abbreviations.ods` has exactly that: `<table:table-cell table:style-name="ce57"/>` in
column C of row 681, a cell with a format and no value. Its cells are Arial 9 pt (asking for
`trunc(180 x 1.18) + 40 - 23 = 229`, floored to 256) and its document default is Calibri 11 pt
(asking for 276).

**Bisected on the file itself** (`aviation-truncate.py`, which truncates Sheet1 to N rows and reads
26.2's own recomputed heights out of its flat-ODF export -- the only instrument here that reads the
grid rather than a rendering of it):

| Sheet1 truncated to | 26.2.4.2's first row |
|---|---:|
| 600 rows | 256 tw |
| 680 rows | 256 tw |
| **685 rows** | **276 tw** |
| whole sheet | 276 tw |

Row 681 is the boundary, and row 681 is the row with the empty `ce57` cell.

The exception that keeps the rule from making this 16383 on every file is in the same function: a
pattern applied out to the sheet's last column allocates nothing -- `maxCol = max(nStartCol,
aCol.size()) - 1` and the remainder goes to `aDefaultColData`
(`ScTable::ApplyPatternArea`, `table2.cxx`:2980-2999). That is exactly the
`table:table-cell table:style-name="Default" table:number-columns-repeated="16376"` that pads every
row of every `.ods` Calc writes.

`tests/corpus/features/sheet-row-height-allocated-column.fods` is the two-sheet discriminator:
identical but for one formatted blank cell in a third column, and 26.2's own export gives 489 twips
a row for the sheet that has it and 256 for the sheet that does not.

## 3. `style:font-name` and `fo:font-family` are two spellings of one item

Found while measuring the above, and much the larger of the two.

LibreOffice imports `style:font-name` through `XMLTextImportPropertyMapper::handleSpecialItem`'s
`CTF_FONTNAME` branch, which hands the `office:font-face-decls` entry to
`XMLFontStylesContext::FillProperties` and fills the `CTF_FONTFAMILYNAME` slot beside it
(`xmloff/source/text/txtimppr.cxx`:58-101). `fo:font-family` writes that same slot. **One slot, one
item** -- so a child style stating either spelling shadows whatever its parent stated.

Both ODF readers resolved the two spellings independently, each through the whole parent chain, so
an ancestor's `fo:font-family` beat a child's `style:font-name`. That is not a corner: it is the
shape LibreOffice writes for every workbook and document converted to ODF -- the named style
carries both spellings and the automatic styles carry `style:font-name` alone.

Censused with `fontcensus.py` over `/home/user/corpus-odf`, counting styles whose two spellings sit
at different levels of one parent chain:

| | documents | styles |
|---|---:|---:|
| `.ods` | **290 of 307** | 16 687 |
| `.odt` | **109 of 338** | 2 419 |

`odf-font-name-precedence.fods` and `.fodt` are the pair: a cell and a paragraph asking for Arial
and inheriting Calibri. 26.2.4.2 embeds `LiberationSans` in both; we embedded `Carlito`.

**This is also the confound the brief warned about on `Aviation_Abbreviations`** -- "the file says
Calibri, we resolve Carlito, and 26.2 resolves Liberation Sans". It is not a fontconfig difference
between the two binaries. The file says *Arial* for its cells, in `style:font-name`, and we were
reading the *document default's* Calibri out of a parent's `fo:font-family`.

## 3b. What the font level moved, and what it cost

51 rows, 40 of them `.ods`. Ten gain a match and two lose one; both losses are `.odt` that go one
page over with their glyph counts unchanged, which is what changing a face does to a page that was
already nearly full. Four of the gains are exact page counts on documents that were far out:

| | before | after | reference |
|---|---:|---:|---:|
| `atspp_pay_tables.ods` | 145 | **101** | 101 |
| `2023-qhp-form-and-rate-combined-checklist-final.ods` | 103 | **95** | 95 |
| `Global_Market_Forecast_2016-2035_Airbus_Data_Set.ods` | 546 | **567** | 567 |
| `237287_0486190101_0486190101_geolink2neu.odt` | 5 | **8** | 8 |

`Global_Market_Forecast` was in the `ods pages-short` group whose page 1 was identical in every
measurable quantity — the sub-group §5 calls "divergence is later in the document". So part of that
sub-group is the face, resolved lower down where the sheet's fonts stop agreeing with the document
default's.

## 4. The frame that is placed but not grown

The previous round left this saying growth "wants the content laid out before the frame is placed,
which is the reverse of `FrameLayout`'s current order".

**The first half is true and the second is not.** `FrameLayout.Place` is a pure function of the
frame's stated size and the page and anchor geometry; the content's own layout needs the frame's
*width*, which the file states, and nothing else. So the height can be measured in a pass that runs
before `Place` without inverting anything -- the order that would have to invert is the one where
placement feeds back into measurement, and it does not.

**What the largest witness needs is a different thing entirely.** `Case-Study-Heathrow-Airport.odt`
is one `text:p` holding one `draw:frame` that is the whole three-page document, and that frame
carries `loext:may-break-between-pages="true"`. Growing it makes it three pages tall on page one;
what the reference does is *split* it, which is `SwFlyFrame::IsFlySplitAllowed`
(`sw/source/core/layout/fly.cxx`:689-730) and the fly-splitting layout behind it. Censused with
`framecensus.py`: **51 of the 58 height-less `draw:frame` in the 338 converted `.odt` declare
`may-break-between-pages`**, in 43 of the 49 documents that hold one.

So: growth is a small change and splitting is an architectural one, and the two are separable --
growth alone will push body text down on the documents whose frame fits its page, and will do
nothing for the ones whose frame does not. Neither was implemented this round.

## 5. What the rest of the `ods pages-short` group is, as far as it was measured

Sub-grouped from `rowsper.py` (page-1 row count, first and last baseline, both banked halves) and
`pitch.py`:

| | rows | witness |
|---|---:|---|
| page 1 identical in every measurable quantity; divergence is later | **10** | `EASA-IFP-147Scope_WEB` 83/94, `aircraft_analysis-2018-06` 38/42 |
| our band starts lower than the reference's header — the header band | **6** | `activespecs` 254/266, `PA_Delaware` 118/122 |
| pitch shorter than the reference's | 8 | `Aviation_Abbreviations` 80/94 (closed), `ans_mappings_of_eccairs_terms` 170/179 |
| pitch longer, or page 1 unreadable | 5 | `SIL_TDB648` 61/88, `EHEST-Pre-departure-checklist` 22/24 |
| remainder | 11 | |

**The header band is the sharpest of the four and it is diagnosed but not fixed.** Calc's band is
`max(nManHeight, maxTextHeight + nDistance)` (`UpdateHFHeight`,
`sc/source/ui/view/printfun.cxx`:838-850) and ODF states both terms directly — `fo:min-height` is
the first and the header's `fo:margin-bottom` the second. `OdsPrintSetup.BandHeight` reads the
declared height alone, which is right only while text + gap stays under it. An `.ods` converted
from a workbook carries the workbook's header margin, so the pair is routinely
`fo:min-height="0.2953in"` (21.26 pt) with `fo:margin-bottom="0.361in"` (25.99 pt) — a gap larger
than the whole declared band. `hfcensus.py`: **58 of 307 `.ods`** declare a band smaller than one
line plus its gap.

On `activespecs.ods` the reference's header sits at the 36 pt top margin and its first cell row at
72.69 — a band of 36.69 pt against the declared 21.26 — and we start the body at 57.27 and fit
three more rows on every page. 254 pages against 266. The full note, with the seat and the number
to fit against, is in `src/XRay.Office.Spreadsheets/TODO.md`.

**The ten whose page 1 is identical are not a header or a pitch question at all.** They need the
first page on which each diverges found and read, one at a time; `firstdiff.py` does the finding.

## 6. The banked baseline this round was handed is not a render of its own base commit

The brief's baseline — `.ods` 202 of 307, `.odt` **257** of 338 — is
`/home/user/odsgap-work/bank/ours-head2` scored with `score.py`, and it reproduces exactly. But
that bank is `ours-head` (a full 645-document render at `bin/head`) with **49** documents
re-rendered at `bin/head2` under `SWEEP_ONLY`, and `bin/head2` is not `6ab0681b9`: rendering
`TE.CAO.00125 Foreign Part 145 approvals - OJT Logbook.odt` with `bin/head2` gives 14 893
alphanumeric characters and the base commit gives 15 283. That document holds **no `draw:frame`
at all**, so the last round's own confinement argument cannot reach it.

Rendering the whole ODF half at `6ab0681b9` gives **`.ods` 202 of 307 and `.odt` 259 of 338**. The
`.ods` column was right and the `.odt` column was two low.

The same staleness is in the original-track bank. `orig-head2` and a clean base render differ on
**22 of 645** documents; re-rendering those 22 at the base commit shows only **7** differ between
the base and this round's tree, and all seven are `.xlsx` — the fifteen `.docx`/`.doc` were the
bank, not the diff. Nothing in this round's diff can reach a `.docx`.

**The rule that follows is the one `dotnet/CLAUDE.md` already states about stored evidence, in a
sharper form: a bank built with `SWEEP_ONLY` is a claim about the documents on the list and about
nothing else.** Proving confinement by diffing that bank against the one it was seeded from is
circular — the off-list rows are the same bytes by construction. The check that is not circular is
a full render, and it costs about forty minutes.

## 7. What is left, reclassified at the end of the round

`classify.py` over `parity-fixC.tsv`, the same screens the previous round's `classify3.py` used:

| group | `.ods` | `.odt` |
|---|---:|---:|
| ink-missing | 38 | 25 |
| pages-short | **36** (was 40) | 21 (was 24) |
| pages-long | 11 | 25 |
| ink-extra | 14 | 6 |
| ref-failed | 1 | — |
| **total failing** | **100** (was 105) | **77** (was 79) |

*The `volatile` and `raster-ceiling` screens did not reproduce this time and their rows fall into
the ink groups instead, so those two columns are not comparable with the previous round's table.
The totals are.*

Six of the `.ods pages-short` rows are the header band of §5, six more are its neighbours by sign,
and ten have a page 1 that is identical in every quantity the instrument can take. The `.odt`
`pages-long` column grew from 20 to 25, which is the two font-level losses plus three rows that
crossed from short to long; a document whose face is now the reference's and whose page count is
one over is a different question from one whose face was wrong.

## What was not done, and why

- **The header band** (§5). Diagnosed, censused at 58 of 307 `.ods`, seat identified as the layout
  rather than `OdsPrintSetup`, and left because it wants the band's own text height and the number
  to fit that against is one measurement (`activespecs`' 10.70 pt) rather than a family of them.
- **Frame growth and frame splitting** (§4). The first is small and the second is architectural;
  the write-up is in `src/XRay.Office.WordProcessing/TODO.md`.
- **`SIL_TDB648.ods`**, the brief's top `pages-short` witness, is neither: its first differing page
  is page 1 and what differs is that our long cell strings are broken and clipped at their column
  where the reference runs them across the empty cells beside them. A cell-overflow question, and
  the only row in the group with that shape.
