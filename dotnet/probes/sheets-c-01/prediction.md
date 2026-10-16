# sheets-c-01 — the prediction, committed before any post-change measurement

Written after reading `probes/sheets-b-01/results.md`, commit `661496ecd76`, the four seats it
names, and after running four **pre-change** censuses over the corpus. Nothing here was written
after a changed line was rendered.

## 0. What the censuses already establish, before a line is changed

These are pre-change measurements over the corpus as it sits on disk, so they are facts rather
than predictions. They are recorded here because the predictions below are derived from them and
would otherwise read as luck.

| census | method | result |
|---|---|---|
| OOXML chart parts in the sheets track | open all 171 as zip, list `xl/charts/chart*.xml` | **1 document** — `Keywords_Mapping_Graphs_and_Charts.xlsx`, 11 charts |
| …of those, sequences whose `c:f` range is longer than its `c:ptCount` | count cells in the ref, compare | **22 of 22** (11 `c:cat` + 11 `c:val`) |
| BIFF chart substreams in the sheets track | walk `Workbook` stream, `BOF dt=0x0020` | 6 documents, 15 charts |
| …of those, a category axis with `CHDATERANGE`'s `DATEAXIS` flag | record `0x1062`, flags & `0x0010` | **1 document** — `Template Pilot Logbook JAR-FCL V3.0.xls`, both its charts |
| area charts anywhere in the 534-document corpus | `c:areaChart` in every OOXML chart part; `CHAREA` (`0x101A`) in every BIFF stream; `chart:class="chart:area"` in every embedded ODF chart | **1 document, 2 charts** — the Pilot Logbook. Zero in words, zero in slides |
| hyperlink records in the sheets track | `<hyperlink>` in every `xl/worksheets/sheetN.xml`; `HLINK` (`0x01B8`) in every BIFF stream | **33 documents**, 4025 records |
| embedded ODF charts anywhere in the corpus | `chart:chart` in any nested `content.xml` | **0** in all three tracks |

## 1. The brief's task 1 is two defects, not one, and I predict its shared-root claim is wrong

The brief says the Pilot Logbook's missing 45° rotation "has the same root" as
`Keywords_Mapping`'s axis range — that the 615 category labels never reach `ChartAxisLabels`
because the cache is short.

**I predict that is refuted, and that the seat named for the range defect is also the wrong
file.** Two reasons, both read out of source before any change:

1. `DrawingChart.cs:363-371` — the seat the brief names — is the **extraction** reader. It builds
   a `ContentSection` and is reached from `XlsxCharts.Read`. Nothing it produces is drawn. The
   **rendering** reader is `DrawingChartPlot.ReadSequence`
   (`src/XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs:1735-1771`), a second, near-identical
   cache reader. A fix in `DrawingChart` alone moves no ink at all.
2. The Pilot Logbook is a `.xls`, and the BIFF chart reader **already resolves against the live
   sheet**: `XlsChartReader.BuildSeries` calls `data.Numbers(valueSheet, values)` and
   `data.Texts(labelSheet, labels)` (`XlsChartReader.cs:407-450`). Its 615 categories therefore
   already reach `ChartAxisLabels`. What stops the rotation is
   `XlsChartReader.ReadLabelRange`, which sets `OverlapAllowed: true` whenever
   `CHLABELRANGE`'s label frequency is 1 — and `ChartAxisLabels.Resolve` returns on the first
   test when overlap is allowed, before the auto-rotate ladder is reached.

   That is a faithful port of `XclImpChLabelRange::Convert` — but only of its **`else`** branch.
   `xichart.cxx:3018-3047` sets `TEXTOVERLAP`/`TEXTBREAK` from the label frequency *only when the
   axis is not a date axis*; on a date axis it takes the other branch and never sets them, so
   chart2's own defaults stand — `TextBreak=false`, `TextOverlap=false`,
   `ArrangeOrder=AUTO` (`chart2/source/model/main/Axis.cxx:239-242`). The Pilot Logbook's
   category axis states `CHDATERANGE` with flags `0x00ff`, `DATEAXIS` (`0x0010`) included, and we
   do not read the record at all.

So I predict **three** chart fixes rather than two, in three different files, and that fixing the
`c:f` seat does **not** rotate the Pilot Logbook's axis.

## 2. Predicted reach, per fix, across the 171-document sheets track

| fix | documents whose rendering changes | why that number |
|---|---|---|
| 1a — resolve `c:f` against the live sheet, XLSX only | **1** | it is the only workbook in the track with any OOXML chart part |
| 1b — a BIFF date axis takes chart2's overlap/break defaults | **1** | it is the only `.xls` in the track whose chart states `DATEAXIS` |
| 2 — an area series skips a missing point instead of plotting 0 | **1** (the same document) | it is the only area chart in the whole 534-document corpus |
| 3 — a hyperlink cell's text is painted `#000080` | **20–33** | 33 documents hold hyperlink records; some of those cells are numeric (not fields), some fall off the printed page, and some may already state navy |

**Union predicted: 22–35 of 171 renderings change.** I am least sure of the hyperlink number and
name a band rather than a figure; 33 is the ceiling the census proves, and 20 is my floor.

## 3. Predicted verdict movement: **zero**

An axis range, an axis label's rotation, a polygon's vertices and a text colour are all invisible
to all three gate checks — page count, extractable words inside a 2%+3 band, unembedded fonts.

**One caveat I will check rather than assume.** Rotating 848 glyphs could move the Pilot Logbook's
extractable *word* count, because `pdftotext` splits rotated text at every `Tj` and LibreOffice
emits one `Tj` per glyph for it — the effect already recorded against `bnc889755.pptx` in
`ChartAxisLabels.cs`'s remarks. If it moves, it moves **towards** the reference, which rotates the
same labels. I predict the verdict still does not change, because the document's word count is
dominated by 38 pages of logbook table.

## 4. Predicted cross-track movement: **zero renderings, both tracks**

Two of the four fixes touch shared libraries and therefore owe words and slides a measurement:

- `XRay.Office.Ooxml` — `DrawingChartPlot` gains an optional resolver whose default is null, so the
  PPTX and DOCX callers keep the cache-only path unchanged by construction.
- `XRay.Office.Core` — `ChartLayout.AddAreas`. The census finds **zero** area charts in the words
  and slides tracks, in OOXML and ODF alike.

I will sweep both anyway and report the count, normalising `/CreationDate` out.

## 5. What these censuses cannot see, written down before the sweep

- Whether a chart falls on a **printed page** at all. A chart anchored past the print area is
  counted by the census and draws nothing. This can only make the reach *smaller* than predicted.
- Whether a hyperlink cell's value is **text**. `HoldsField` requires it, and a hyperlink on a
  numeric cell stays an attribute. This too can only shrink the reach.
- Whether the resolved cells differ from the cache when the **lengths already agree**. My census
  compared lengths only; a stale cache of the right length would change ink without being
  counted. On the one affected workbook all 22 sequences differ in length anyway.
- Charts inside **embedded OLE objects** in any of the three tracks. I did not open them.
- The `.xlsb` path (`XlsbReader`) reaches the same `XlsxDrawings.Read`. There is no `.xlsb` in the
  sheets track, so I cannot measure whether wiring it would move anything, and I will say so
  rather than claim it is done.

## 6. What would make this round wrong

If the sheets sweep moves more than 35 renderings, something changed that I did not predict and
the extra is the finding. If it moves fewer than 20, my hyperlink floor was guesswork dressed as
a bound.
