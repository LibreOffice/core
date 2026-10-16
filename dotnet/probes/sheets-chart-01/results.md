# Results — a BIFF chart axis takes its number format, and its type, from what the file states

Scores `prediction.md`, which was committed first (`768ed18106c`). Reference output is the banked
26.2.4.2 set at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/`; nothing was re-rendered on the
reference side.

## The seat, and it was not where the brief pointed

The brief's lead was that `Core/Numbers` "came down" so a chart axis could reach the number-format
engine, and that the axis still wrote shortest round-trip form — so the engine was either
unreachable or being called without a format code. **It was the second, and one layer further
back than the axis.** `XRay.Office.Spreadsheets/MsBinary/XlsChartReader.cs` never set
`ChartPlot.ValueFormat` at all. `ChartLayout` calls `ChartDataLabel.Write(tick, plot.ValueFormat)`
and has done throughout; `DrawingChartPlot` and `OdfChartPlot` fill that field from an axis element
their markup carries, and BIFF has no such element on this chart, so the field stayed null and
`Write` took its `NumberFormatter.General` branch. Nothing in `Core/Charts` needed changing.

## What BIFF actually states, and in which of two places

`XclImpChAxis::Convert` (`sc/source/filter/excel/xichart.cxx:3363-3377`) is two lines and an
ordering:

1. `CHFORMAT` (0x104E) inside `CHAXIS` — a bare `ifmt` into the workbook's format table. When it
   resolves it is set on the axis and `LinkNumberFormatToSource` is turned **off**.
2. Otherwise the axis links to its source, and
   `AxisHelper::getExplicitNumberFormatKeyForAxis` (`chart2/source/tools/AxisHelper.cxx:135-310`)
   asks the value sequence for `getNumberFormatKeyByIndex(-1)`, which `ScChart2DataSequence`
   answers with *the format of the first non-empty numeric cell*
   (`sc/source/ui/unoobj/chart2uno.cxx:3257-3277`, comment `// TODO: use nicer heuristic`).

**The field that looks like the answer and is not** is `CHSOURCELINK`'s own `ifmt`. It sits on the
same record as the series' range and feeds a *data label* (`xichart.cxx:1684`). The target
document settles it without argument: page 17's chart states `ifmt` **370** there, an index no
`FORMAT` record in that workbook defines, while the cells the link names (`GraphData!D2:D800`)
carry `ifmt` 175 = `0.0` — which is what the reference draws. Its *other* chart happens to state
175 and 176 on the same field, so a reader that took it would look right on one chart of the two.

## What changed

| file | change |
|---|---|
| `MsBinary/XlsChartSource.cs` | `XlsChartData` keeps each offered cell's `NumberFormatCode`; `FormatOf(sheet, range)` answers with the first non-empty *numeric* cell's |
| `MsBinary/XlsWorkbookReader.cs` | offers that format; `NumberFormatAt(ifmt)` resolves a bare format index for `CHFORMAT` |
| `MsBinary/XlsChartReader.cs` | reads `CHFORMAT` on the value axis; sets `ValueFormat` by the precedence above; reads `CHFONT`'s **size and weight** for the title, the axis titles and the axis labels |
| `MsBinary/XlsCellFormats.cs` | `FontAt(index)` hands the whole `FONT` over, not only its family |
| `Layout/SheetFonts.cs`, `Layout/SheetBandText.cs`, `Layout/SheetChart.cs` | a chart's bold text is measured **and** drawn in the family's bold face |

The last row is the half the brief did not ask for and item 3 could not be fixed without.
`SheetChart`'s measurer carried a comment saying so outright — `ChartPlot.IsTitleBold` "now hands a
weight to this measurer and to `SheetChart`'s drawing, and both drop it … that belongs to a round
that sweeps this track". The model had been right and the consumer had been throwing it away, so
reading `CHFONT`'s weight alone changed nothing on the page.

**`CategoryFormat` is deliberately still not set on this path**, and that is not an omission. A
BIFF chart's categories already arrive as the sheet's *displayed text* — `XlsChartData.Texts` — which
is the cell's own format applied once, and is exactly what Calc's `getTextualData()` hands chart2.
`ChartDataLabel.WriteCategory` would apply it a second time. Idempotent at best, destructive on a
locale-formatted string.

## Score

| prediction | measured | |
|---|---|---|
| the seat is the reader, not `Core/Charts` | yes | ✔ |
| 6 OLE2 documents in the whole corpus hold a chart substream, 15 substreams | 6 and 15 | ✔ |
| **0 renderings change outside the sheets track** | 0 | ✔ |
| 6 documents change on the sheets track (font half) | **6 of 171** | ✔ |
| 2 documents change their *tick text* (number-format half) | **1 of 171** | ✘ over by one |
| the X label count does not change | it did not | ✔ |

The miss is worth stating rather than rounding away. `2012-GA-Survey-Chapter-6-…V2.xls` states
`CHFORMAT` `ifmt` 3 = `#,##0`, and the prediction said thousands separators would appear. Its value
axis runs **0 to 12** — the chart's series link resolves to nothing, so the axis is on the
0…12 default — and `#,##0` and General write those identically. The model changed; no pixel did.
The same trap the prediction called out for chapter 5 caught chapter 6 too: *a format only shows
where the numbers reach it*, and counting formats is not counting renderings.

## Reach, measured

Rendered all 171 documents of the sheets track twice with `SOURCE_DATE_EPOCH` fixed — once from
the tree at `768ed18106c` (the prediction commit, no code change) and once after — and compared
byte for byte.

```
6 of 171 renderings changed
```

They are exactly the six workbooks holding a chart substream: `2012-GA-Survey-Chapter-5`,
`2012-GA-Survey-Chapter-6`, `EHEST-Pre-departure-checklist`, `TOGAF9-Tool-ConfReqts-CSQ`,
`Template Pilot Logbook JAR-FCL V3.0`, `orbus_togaf_tool_csq`. **Page counts are unchanged on all
six** — a chart is drawn inside its anchor's rectangle and cannot move a page break — and word
counts move by −1 to +4 as labels reflow.

Only one document changed its label *text*: the Pilot Logbook's two charts, `0 → 0.0`,
`1000 → 1000.0`, `1200 → 1200.0`, `1400 → 1400.0`. The other five moved in type weight, type size
and the reflow that follows.

**Cross-track effect: none, and by construction rather than by luck.** `XlsChartBuilder` is
reachable only from `XlsWorkbookReader`; `SheetBandText` and `SheetChart` are referenced from no
library but `XRay.Office.Spreadsheets` (`XRay.Office.WordProcessing/Layout/FrameChart.cs` names
`SheetChart` in a comment only); and the census walked every OLE2 file in all three tracks and
found chart substreams only under `sheets/`. Decks reach `ChartPlot` through
`XRay.Office.Presentations`, which already honoured the weight.

Worth recording for the next round: **no `.xlsx` chart moved.** The bold wiring reaches OOXML
charts too — `DrawingChartPlot` makes a title bold when the part states nothing — and not one of
the track's SpreadsheetML charts changed, because the ones that carry a title state `b="0"`
explicitly, as `tests/corpus/features/chart-bar-sheet.xlsx` does. The reach of a default is the
set of files that decline to state the property, and here that set is empty.

## The blind review

A fresh subagent, given only the composed page-17 pair and forbidden to read anything else,
reported under *what looks identical*:

> Vertical axis tick values and formatting: identical set `0.0 / 200.0 / 400.0 / 600.0 / 800.0 /
> 1000.0 / 1200.0`, identical one-decimal format … Neither half carries a decimal the other lacks.
>
> Chart title text, weight, size and position … No difference in weight (both bold) or size.
> Vertical axis title "Total hours" … bold in both. Horizontal axis title "Date" … bold in both.

Items 1 and 3 are closed on an uncontaminated reading.

## Items 2, 4 and 5 — and one thing in the brief that is wrong

### Item 2 (the legend) is real, and my first reading of it was wrong

The brief says the reference draws no legend. Reading the reference PDF's *text* contradicts that —
`pdftotext -bbox` puts "Real Total Flight" on page 17 at x 771–842 pt — and I recorded that as the
brief being mistaken. **It is not.** Rasterising and counting ink settles it:

| | non-white pixels in the reference's own reported legend text box |
|---|---:|
| ours | 932, of which **144 are the magenta swatch** |
| reference | **0** |

The reference emits the legend text into the content stream and paints **no ink at all** for it.
`pdftotext` reads the stream; the page is blank there. So a text extraction is not evidence about
what a page shows, and this is the second time in this project a PDF-text reading has produced a
confident wrong answer about ink.

The mechanism is not established and I am not guessing at a fix. Two further measurements, for
whoever takes it: the *same document's* other chart (page 16, three series) **does** get a painted
legend from the reference, so this is not "LibreOffice drops legends"; and page 17's `CHLEGEND`
states a custom box 575/4000 of the chart wide against a single entry whose text alone is ~96 pt,
which points at `VLegend`'s `ChartLegendExpansion_CUSTOM` path dropping an entry that does not fit
its stated box (`chart2/source/view/main/VLegend.cxx:325-400`). We read no legend geometry at all —
`_hasLegend` becomes `ChartLegendPosition.Right` and nothing else. **Not fixed.**

### Items 4 and 5 are one defect, not two

The brief lists the X labels (2 against ~37) and the series mark's position and shape as separate
observations. They have one root: **the reference draws a date axis and we draw a category axis.**

* `CHDATERANGE` on this chart is `min=37935 max=41292 majorStep=1 majorUnit=YEARS baseUnit=DAYS
  flags=0x00FF`. Every auto bit is set, including `EXC_CHDATERANGE_AUTODATE`, which
  `XclImpChLabelRange::Convert` (`xichart.cxx:3013-3047`) turns into `AutoDateAxis` — an axis whose
  *scale* is dates and whose ticks are date intervals.
* The reference's 38 labels are `30/12/99`, then `02/01/03 … 02/01/11` at a **three-year step**:
  serial 0 to ~41 292 on a continuous scale. None of them is a category of this chart.
* Our two labels are the **first and last categories** — `GraphData!A2:A800` has values in only 25
  of its 799 cells, at rows 2 and 601–624 — already correctly formatted `dd/mm/yy` from the cell,
  and thinned to indices 0 and 622 by `ChartAxisLabels`' collision ladder.

So this is not a date/serial interpretation error and not label thinning gone wrong; it is a missing
axis *type*. And it explains item 5 exactly, which is the strongest evidence for the diagnosis:
on a category axis the populated points sit at ordinals 600–623 of 799 = **75–78 % of the plot
width**, and the blind reviewer measured our mark at "~74 % across"; on a date axis they sit at
37 935–41 292 of 0–41 292 = **91.8–100 %**, and the reviewer measured the reference's at "the
right border … partially outside/clipped". The reviewer had no access to either number.

Fixing it means a date axis in `ChartLayout` — a numeric X scale with date-unit tick generation, an
auto min/max over the category values, and `CHDATERANGE`'s six fields plumbed through the reader.
That is a feature, not a repair. **Not fixed**, and it should be one round of its own.

### Two smaller things the brief states that the measurements do not support

* *"the chart's position on the page … identical"*. Our chart's contents sit **~13 pt right** of the
  reference's, uniformly: the rotated axis title starts at x 98.5 against 85.6, the value labels
  end at 159.4 against 146.1, the legend starts at 787.7 against 771.4. The offset is the same
  before and after this change — it is pre-existing and untouched — and the blind reviewer saw it
  independently ("the bottom's plot rectangle starts slightly further left"). Small, but it is not
  zero and the next round should not assume it is.
* *"its 'Total hours' axis title is visibly larger"* is right, and it is 10 pt against our former
  9 pt — the model's chart2 default, not a scale factor.

## One limit taken deliberately

`ChartPlot` holds **one** `LabelSize`/`IsLabelBold` for both axes and BIFF gives each axis its own
`CHFONT`. The category axis' is the one kept, because its labels are what `ChartAxisLabels` tests
for collision — the size they are measured at decides whether the axis rotates or thins, and so how
many labels a page shows — while the value axis' size only widens a band. Fourteen of the corpus's
fifteen substreams state the same size on both and the choice is moot;
`2012-GA-Survey-Chapter-6-Tables-16Dec2013-V2.xls` states 8 pt on its category axis and 10 pt on its
value axis and is the one file the choice is visible on. Recorded rather than resolved: resolving it
means a second property on the model and a reason from more than one file.

## Tests

Every project run individually, counts compared against `--list-tests` where it matters.

| project | passed | failed | skipped | note |
|---|---:|---:|---:|---|
| `XRay.Office.Core.Tests` | 313 | 0 | 0 | |
| `XRay.Office.Containers.Tests` | 109 | 0 | 0 | |
| `XRay.Office.Markup.Tests` | 259 | 0 | 0 | |
| `XRay.Office.OpenDocument.Tests` | 125 | 0 | 0 | |
| `XRay.Office.Presentations.Tests` | 631 | 0 | 0 | |
| `XRay.Office.Rendering.Tests` | 148 | 0 | 1 | the skip is pre-existing |
| `XRay.Office.Spreadsheets.Tests` | **704** | 0 | 0 | 704 discovered, +20 from this round |
| `XRay.Office.Text.Tests` | 289 | 0 | 0 | |
| `XRay.Office.Vector.Tests` | 295 | 0 | 0 | |
| `XRay.Office.WordProcessing.Tests` | 792 | 0 | 0 | |
| `XRay.Office.Fidelity.Tests` | 519 | **31** | 0 | 550 discovered, 550 run |

The fidelity project was run **twice** — once on the tree with the change reverted and once with it
— and the two failure sets are identical name for name, 31 each. So none of the 31 is new. Both runs
report 550 of 550 discovered with 0 skipped, which is what makes the count trustworthy rather than
just green.

New cases, 20 in all: `XlsChartNumberFormatTests` (7), `XlsChartTextSizeTests` (11),
and two in `SheetChartFaceTests` for the drawing half.

## Scripts

* `census.py` — walks every OLE2 file under a corpus root, decodes each chart substream, and
  resolves the value and category ranges against the workbook's `XF`/`FORMAT` tables. It counts what
  a chart *resolves to*, not what a file declares, which is the correction the previous round's
  35–55-against-2 miss called for.
* `biffdump.py` — prints a workbook's chart substreams as a tree, with the fields of the records
  this round turned on. It is what found `ifmt` 370 and `CHDATERANGE`'s `0x00FF`.
