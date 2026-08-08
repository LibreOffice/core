# Sheets round twenty-four — probe data

Base commit `73934b1b5`. Reference renderer LibreOffice 24.2.7.2. Every sweep here was run
against a checksummed copy of the CLI so that a rebuild could not disturb it; the `measuring …`
line on each run named the copy.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep at `73934b1b5`, before any change |
| `after-charts-whole-track.tsv` | the same after this round's two commits |

Both: 171 rows, no duplicate path.

## The baseline, and it reproduces exactly

| | matches | abs page error | exact page counts | abs word error |
|---|---|---|---|---|
| `base-whole-track.tsv` | 144/171 | 94 | 153 | 43903 |
| `after-charts-whole-track.tsv` | 144/171 | 94 | 153 | 43694 |

Per batch, identical in both: 001–009 all pass, then 010 6/10, 011 6/10, 012 8/10, 013 8/10,
014 9/10, 015 5/9, 016 4/9, 017 6/10, 018 3/4. That is round twenty-three's figure to the digit,
per batch as well as in total.

One row of the baseline needed re-running. `grants-2005.xls` came back `ref-failed` under load
and converts fine on its own — 219 pages against 220, 36005 words against 36003 — so the spliced
row is a page mismatch, which is what round twenty-three recorded for it. The reproduction is
therefore of the verdict and not only of the total.

## What changed: a `.xls` chart's series now reach the plot

`CHSOURCELINK` was scoped by two previous rounds and started by neither. Three pieces:

- **`XlsChartFormula`** decodes the single reference token a chart link's formula consists of —
  `tArea3d`, `tRef3d` and their sheetless forms, in both BIFF generations. An expression names no
  rectangle and is refused rather than guessed at.
- **`XlsExternSheets`** reads `SUPBOOK` and `EXTERNSHEET`, which is the only way a token's
  `ixti` becomes a sheet. A reference into another workbook resolves to nothing rather than to
  this workbook's sheet of the same number.
- **A pre-scan and a gather pass** in `XlsWorkbookReader`. The pre-scan walks the whole record
  stream and reads the payload of chart records only, collecting every rectangle any chart names;
  the gather pass then reads those sheets — with the existing sheet reader, into a builder whose
  output is thrown away — before the content pass begins.

### Why the data is fetched before the chart rather than after

A chart is built the moment its substream ends, and its substream is embedded in the sheet its
*picture* sits on. In both corpus documents the numbers are on a different sheet, and a later
one: the logbook's charts are on sheets 3 and 4 and plot sheet 7. Resolving at the point of use
resolves against a workbook not yet read, which is exactly what the first version of this did —
`EHEST` worked and the logbook silently produced no series at all, because `EHEST`'s chart happens
to sit on the sheet it plots and the cell records happen to precede the drawing records.

The two alternatives are both worse. A decoder of its own for the cell records is a second copy
of the eleven record layouts `ReadSheetRecords` already handles. Deferring the chart's
construction means rebuilding a finished sheet's `SheetDrawings` afterwards. Reading the
referenced sheets twice costs one extra pass over them, only for a workbook that holds a chart
naming cells at all.

### A BIFF category axis labels every category, overlapping

`XclImpChLabelRange::Convert` (`sc/source/filter/excel/xichart.cxx:3039-3047`) turns
`CHLABELRANGE`'s label frequency into three properties, with the reason written beside it —
*do not overlap text unless all labels are visible*. `XclChLabelRange`'s constructor defaults that
frequency to 1, so an axis that says nothing draws every label whatever it collides with, and none
of the thinning, rotation or staggering in `ChartAxisLabels` happens at all. Nothing read the
record, so `EHEST` drew every second category label where the reference draws all fifty-one.

`ChartPlot.CategoryAxisText` already existed and the OOXML and ODF readers already set it, so this
is confined to `Paperless.Spreadsheets`. **Nothing in `Paperless.Core` was touched this round.**

### Measured on the two documents

| | ours before | ours after | reference |
|---|---|---|---|
| `Template Pilot Logbook JAR-FCL V3.0.xls` | 1305 words, axis 0…12 | 1342 words, axes 0…1400 and 0…1200 | 1610 words, axes 0…1400 and 0…1200 |
| `EHEST-Pre-departure-checklist…xls` | 7825 words, axis 0…12 | 8018 words, axis 0…90 | 8382 words, axis 0…90 |

The value axes now agree exactly, and the series carry the names the reference's legend shows.
`EHEST`'s word count moved 7825 → 8148 with the source links alone and back to 8018 with the
label-overlap change on top — see below, because the direction is informative rather than a
regression.

## Reach, measured by rendering

All 171 rendered with both CLIs and the PDFs compared with `/CreationDate`, `/ModDate` and `/ID`
masked: **4 of 171 change what is drawn**, all four `.xls` carrying charts —
`EHEST-Pre-departure-checklist…`, `Template Pilot Logbook…`, `TOGAF9-Tool-ConfReqts-CSQ.xls` and
`orbus_togaf_tool_csq.xls`. The other 167 are byte-identical.

**A fifth document is a false positive and is worth passing on.** `PBN Matrix NAAs (V01).xlsx`
differed between the two sweeps — an `xlsx`, which no BIFF change can reach. Rendered again with
both CLIs under identical conditions it is byte-identical. So **our PDF writer is not reproducible
across sweep runs**, and any byte-level reach measure has a false-positive floor. It was one
document in 171 here; it is not zero, and the check that separates it is cheap.

## Why `EHEST` still fails, and it is not the chart data

Its residue is now **−26 words on each of eight chart pages**. On page 11 the reference draws all
51 category labels with a 8.28 pt pitch and `pdftotext` reads them as 51 tokens; we draw the same
51 with a 7.30 pt pitch and it reads `101113141618…` as one. **Our plot area is 365 pt wide where
the reference's is 414 pt**, and the difference is the legend beside it: the reference's legend
holds two entries in Carlito and ours holds three in Liberation Sans.

So the label-overlap change is right — it is what the reference does, and it is why the labels are
all drawn at all — and it scores worse on `wc -w` because a *separate* defect makes them collide.
Both halves are measured; the numbers are 8148 with the source links alone and 8018 with both, and
the second figure is the more faithful rendering.

## The chart-label font gap, and the census the last round asked for

`ChartLabel` (`Paperless.Core/Charts/ChartLayout.cs`) carries no font family, so `SheetChart`,
`SlideChart` and `FrameChart` all draw chart text in one hardcoded Liberation Sans.
Round twenty-three measured its reach on this track as **one document** and recorded it rather
than fixing it, asking that the other two tracks be censused before anyone sized the work.

Censused, by opening every one of the 534 corpus documents and looking for a `chart*.xml` member:

| track | documents | zip containers | non-zip | carrying a chart part |
|---|---|---|---|---|
| words | 200 | 160 | 40 | **1** |
| slides | 163 | 114 | 49 | **15** |
| sheets | 171 | 118 | 53 | **1** |

**Fifteen of the slides track's 163 documents carry chart parts, nine of them more than one, one
of them twenty-one.** So this is not a special case: it is a shared defect that happens to be rare
on sheets. The figure is a ceiling in the sense the skill means — a document carrying a chart part
need not draw text the face changes — and it is a *floor* in another, because the 102 binary
`.ppt`/`.xls`/`.doc` documents are not counted at all and at least one of them, `EHEST` above,
demonstrably draws chart labels the reference sets in Carlito.

It is also load-bearing on *this* track, which the one-document figure hid: `EHEST` is an `.xls`
with no chart part, and the face is half of why its plot area is 12% too narrow.

**Not done this round, and deliberately.** Closing it means widening a `Paperless.Core` type and
the `IChartTextMeasurer` interface, which is the file two other agents are in concurrently, and it
needs its own whole-track sweep on all three tracks. Handing over a widened Core type without that
sweep is the half-built feature the brief forbids.

## `INDEX_Digital_Transformation_Toolkits.xls` — the numbers reproduce, the explanation does not

20 pages against 24, 1982 words against 1982. Measured page by page, and this is the whole of it:

| page | 1–12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| words, ours | equal | 0 | 0 | 172 | 183 | 150 | 165 | 227 | 120 | — | — | — | — |
| words, reference | equal | 0 | 0 | 0 | 0 | 0 | 0 | 172 | 183 | 150 | 165 | 227 | 120 |
| images, ours | 1 each | 15 | 10 | 16 | 10 | 0 | 0 | 0 | 0 | — | — | — | — |
| images, reference | 1 each | 15 | 21 | 19 | 16 | 18 | 8 | 16 | 21 | 19 | 16 | 18 | 8 |

- **Pages 1 to 13 agree exactly**, images included and in the same positions.
- The totals are **63 drawn images against 207**, which is the figure carried in the brief — so
  that number reproduces, and it is a count of images *drawn*, not of records in the file.
- The reference prints the same block of pictures **twice**, once alone over its pages 13–18 and
  once again under the text on 19–24. So does ours, over two pages each instead of six.
- The divergence starts on **page 14**, not at the end: ours holds 10 images there and the
  reference's 21. A shape walk that stopped after 25 shapes would not put the first 15 in exactly
  the right places and then thin the sixteenth onwards.

So the "truncated shape walk" reading does not survive its own measurement. What the figures fit
is a printed range that does not extend far enough down the sheet to cover the drawings anchored
below it, which is `SheetDrawingArea`'s question rather than the Escher reader's. That is a
hypothesis, and it is the one to test first; the table above is the measurement.

## `Template Pilot Logbook JAR-FCL V3.0.xls` cannot pass the word gate

Two independent reasons, both measured, and neither is the chart data:

1. **Its category axis is a date axis in the reference.** 615 category cells, of which 17 hold
   anything, and the reference labels 30 evenly spaced ticks running from 30/12/1899 to about
   2111 — a linear scale over the date serials, not one label per category. We have no date axis.
2. **The reference's word count on those pages is a PDF-writer artefact.** Of the 264 text
   records on its page 16, **251 hold exactly one glyph**: LibreOffice writes rotated text one
   `Tj` per glyph and `pdftotext` reads each as a word boundary, so 30 labels of eight characters
   score about 240 words. Drawing the same 30 labels as strings scores 30. Page 16 is 142 words
   against our 29, and drawing every label correctly reaches about 59.

This is the class `ChartAxisLabels`' own remarks already record for `bnc889755.pptx` and
`tdf106217.pptx`, now on a sheet. The document is 1342 words against 1610 and the reachable
figure is around 1409; the 2% band needs 1578.

## Tests

`XlsChartSourceTests`, 13 cases, and **every one of the thirteen fails under at least one of eight
reintroduced bugs** — there are no drift guards in this file:

| mutation | cases that fail |
|---|---|
| source links never read | 3 |
| `CHLABELRANGE` never read, default as before | 4 |
| `EXTERNSHEET` entries appended rather than inserted at the front | 1 |
| a BIFF5 area read with the BIFF8 column layout | 1 |
| an all-null series drawn anyway | 1 |
| an external supbook treated as this workbook | 1 |
| a deleted sheet accepted | 1 |
| a truncated token array read anyway | 1 |

## Test counts

Every project run individually, whole output captured, **0 skipped** everywhere:

| Core | Containers | Text | Vector | Rendering | Markup | OpenDocument | WordProcessing | Spreadsheets | Presentations | Fidelity |
|---|---|---|---|---|---|---|---|---|---|---|
| 243 | 109 | 240 | 291 | 119 | 259 | 125 | 619 | **483** | 523 | 546 |

Every count is the briefed known-good except Spreadsheets, which is 470 + the 13 new
`XlsChartSourceTests` cases.
