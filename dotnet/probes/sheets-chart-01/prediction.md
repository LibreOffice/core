# Prediction — a BIFF chart axis takes its number format from the cells it plots

Committed before any post-change rendering. What has been measured so far is the **file**
(`biffdump.py`, `census.py`) and the **banked reference** (`pdftotext` on
`refpdfs-26.2.4.2-fonts/sheets/Template Pilot Logbook JAR-FCL V3.0__xls.pdf`), not our own
output after the change.

## The seat

`XRay.Office.Spreadsheets/MsBinary/XlsChartReader.cs` never sets `ChartPlot.ValueFormat`. Both
readers that do — `DrawingChartPlot` (OOXML) and `OdfChartPlot` (ODF) — set it from an axis
element the markup carries; BIFF has no such element on this chart, so the field stayed null and
`ChartDataLabel.Write` fell through to `NumberFormatter.General`, which is the shortest
round-trip form. The number-format engine is reachable from `Core/Charts` — the move recorded in
`dotnet/CLAUDE.md` did happen — it was simply never called with a format code on this path.

Two records decide what the format is, and `xichart.cxx:3363-3377` orders them:

1. `CHFORMAT` (0x104E) inside `CHAXIS` — a bare `ifmt`. When it resolves, it wins and
   `LinkNumberFormatToSource` is turned **off**.
2. Otherwise the axis links to its source, and `AxisHelper::getExplicitNumberFormatKeyForAxis`
   (`chart2/source/tools/AxisHelper.cxx:135-310`) asks the value sequence for
   `getNumberFormatKeyByIndex(-1)`, which `ScChart2DataSequence` (`sc/source/ui/unoobj/chart2uno.cxx:3257`)
   answers with "the format of the **first non-empty numeric cell**" of the range.

`CHSOURCELINK`'s own `ifmt` is **not** the axis' format — it feeds data labels
(`xichart.cxx:1684`). On the target chart it is 370, an index no `FORMAT` record defines.

## What the file says

`Template Pilot Logbook JAR-FCL V3.0.xls` holds two chart substreams. Page 17 is the second
(its only series is `Real Total Flight Time`). Neither axis carries a `CHFORMAT`. The series
plots `GraphData!D2:D800` against categories `GraphData!A2:A800`; the first non-empty numeric
cell of each is `D2` with `ifmt 175 = "0.0"` and `A2` with `ifmt 176 = "dd/mm/yy;@"`.

The banked reference draws `1200.0 1000.0 800.0 … 0.0`. `0.0` applied to those ticks is exactly
that. This is a prediction about the mechanism that the reference's own output already confirms.

## The census, and what it counted over

`census.py` walks every OLE2 file in all three tracks, decodes each chart substream and resolves
the source range against the workbook's `XF`/`FORMAT` tables — so it counts what a chart
**resolves to**, not what a file declares.

| | |
|---|---:|
| OLE2 documents holding a chart substream, whole corpus | **6** |
| chart substreams | **15** |
| substreams whose **value** axis resolves to a non-General format | **4** |
| substreams whose **category** axis resolves to a non-General format | 11 |
| documents affected on the value axis | **3** |

All six are on the sheets track (`batch-002` ×2, `batch-010` ×3, `batch-017` ×1). No `.doc`,
`.ppt` or any other OLE2 file in `words/` or `slides/` holds a chart substream, and no library
outside `XRay.Office.Spreadsheets` references `XlsChartBuilder`, so **the predicted cross-track
effect is zero renderings**.

## What is predicted to change, document by document

| document | charts | mechanism | predicted visible change |
|---|---:|---|---|
| `Template Pilot Logbook JAR-FCL V3.0.xls` | 2 | link-to-source, `0.0` | ticks gain one decimal place |
| `2012-GA-Survey-Chapter-6-…-V2.xls` | 1 | `CHFORMAT` ifmt 3 → `#,##0` | thousands separators appear |
| `2012-GA-Survey-Chapter-5-…-V2.xls` | 1 | `CHFORMAT` ifmt 1 → `0` | **none** — `0` and General agree on whole ticks |
| `EHEST-…`, `TOGAF9-…`, `orbus_…` | 11 | value axis is General | none from this change |

So on the number-format change alone: **2 documents move, 1 more changes its model without
changing a pixel, and 168 of 171 on the track are untouched.** That is a deliberately small
number and it is the honest one; the previous round that predicted 35–55 and measured 2 did so
by counting declarations rather than resolutions, and this census does not repeat that.

## The second change in the same commit, and its separate reach

The same reader states no chart font size or weight either, so `ChartPlot` kept its chart2
defaults (title 13 pt regular, axis title 9 pt regular). `CHFONT` under a `CHTEXT`/`CHAXIS` names
a workbook `FONT`, and `XclImpChText::UpdateText` (`xichart.cxx:1042`) falls back to the default
text's font. A census of the same 15 substreams finds **every one of them states a chart-title
`CHFONT` that differs from the 13 pt regular default** — 14 pt bold, 18 pt bold, 12 pt bold and
10.8 pt regular across the six documents.

Predicted reach of the font half: **6 documents, all on the sheets track, 0 elsewhere.**

## What is predicted *not* to fix

The X axis draws 2 labels against the reference's 38. That is **not** the same defect: the two
values we draw (`10/11/03`, `23/12/12`) are the first and last *categories*, correctly formatted
`dd/mm/yy` already, thinned by collision; the reference's are **ticks on a date scale** running
from serial 0 to 2012 at a 3-year step, which no category of this chart holds. Fixing it needs a
date axis in `ChartLayout`, not a number format. Predicted change to the X label count: **none**.

## Scoring

Filled in after measuring. See `results.md`.
