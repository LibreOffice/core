# Two BIFF chart types that have no record of their own — r186

Measured 2026-09-27 against **LibreOffice 26.2.4.2** (`/opt/libreoffice26.2/program/soffice`,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`). The C++ read is this checkout, which is
27.2.0.0.alpha0+ and **not** the reference binary's source; both arms are confirmed a second
time against 26.2.4.2's own rendering.

## 0. What the review found

A round reviewing every chart path against LibreOffice's source found that
`XlsChartReader`'s type switch is a switch on the *record id*:

```csharp
case BiffChartRecords.Pie:      SetKind(ChartPlotKind.Pie);     break;
case BiffChartRecords.Scatter:  SetKind(ChartPlotKind.Scatter); break;
```

and that two of BIFF's chart types have no id. `XclImpChType::Finalize`
(`sc/source/filter/excel/xichart.cxx`:2288-2303) decides the type **after** reading the
record body:

```cpp
case EXC_ID_CHPIE:
    maTypeInfo = GetChartTypeInfo( (maData.mnPieHole > 0) ?
        EXC_CHTYPEID_DONUT : EXC_CHTYPEID_PIE );
break;
case EXC_ID_CHSCATTER:
    maTypeInfo = GetChartTypeInfo( ::get_flagvalue(
        maData.mnFlags, EXC_CHSCATTER_BUBBLES,
        EXC_CHTYPEID_BUBBLES, EXC_CHTYPEID_SCATTER ) );
break;
```

Both bodies were being skipped, so **every BIFF doughnut was drawn as a pie and every BIFF
bubble chart as a scatter**.

## 1. The bodies, and what is and is not in them

`ReadChType` (`:2240-2273`) reads, under `GetBiff() == EXC_BIFF8` for the last field of each:

```
CHPIE     0x1019   anStart u16   pcDonut u16   grbit u16
CHSCATTER 0x101B   pcBubble u16  wBubble u16   grbit u16
```

Three of those six fields are read by the reference and then used by nothing:

* **`anStart`**, the first wedge's angle, becomes `StartingAngle` on the diagram
  (`ConvertPieRotation`, `:384-388`) — and neither `ChartPlot` nor the DrawingML reader's
  `c:firstSliceAng` models one, so reproducing it is a separate piece of work rather than
  part of this. Skipped, with the reason written at the call site.
* **`pcBubble` and `wBubble`** are stored in `XclChTypeData` and never read again
  (`git grep mnBubbleSize mnBubbleType -- sc/` finds only the writer and the reader itself).
  They are the BIFF spellings of `c:bubbleScale` and `c:sizeRepresents`, which
  `ChartPlot` already records that `oox` also parses and ignores. So the reference draws
  every BIFF bubble chart at the defaults, and leaving them alone is what matches it.

`pcDonut` is likewise dropped once the type is chosen: `PieChart`'s constructor sets
`m_fRadiusOffset = 1.0` for a ring chart and nothing else, so the hole is half the radius
whatever the file states — which is already what `ChartPlot.Rings` records.

## 2. The third link

Switching the kind alone is not enough, because a bubble's sizes arrive by a source link
this reader had no case for. `XclImpChSeries::ReadChSourceLink` files a link by its
destination byte (`:2130-2136`) and `EXC_CHSRCLINK_BUBBLES` is **3**, beside title 0, values
1 and categories 2; the sequence is then offered to chart2 as
`EXC_CHPROP_ROLE_SIZEVALUES` (`:2062`), which only `BubbleChart` asks for. Without it every
bubble is drawn at one radius.

## 3. The fixtures

`make.sh` regenerates them: 26.2.4.2's own `--convert-to 'xls:MS Excel 97'` of the corpus's
`advanced_excel_*` chart workbooks. Converting rather than authoring means both renderers
read identical bytes and every divergence is ours.

> **Name the filter.** A first census reported *no chart records at all* in the output of a
> bare `--convert-to xls`. The records were there; the scanner was reading the wrong
> stream. Scan the compound file's streams — `records.py` — rather than grepping the bytes.

`records.py` prints the two fields for each:

```
003_advanced_excel_pie        CHPIE     anStart=0 pcDonut=0  grbit=0x0000  -> PIE
028_advanced_excel_doughnut   CHPIE     anStart=0 pcDonut=50 grbit=0x0000  -> DONUT
006_advanced_excel_scatter    CHSCATTER pcBubble=100 wBubble=1 grbit=0x0000 -> SCATTER
007_advanced_excel_bubble     CHSCATTER pcBubble=100 wBubble=1 grbit=0x0001 -> BUBBLES
```

Each mover has its own control differing in exactly the field under test, and `pcBubble`
and `wBubble` are **identical** across the scatter/bubble pair — so the flag is the only
thing that can explain a difference between them.

## 4. The measurement

`measure.sh`, both legs in one session with the same instrument, `SOURCE_DATE_EPOCH=0`,
three pages each:

| fixture | worst before | worst after | sum before | sum after |
|---|---:|---:|---:|---:|
| `001_advanced_excel_bar` | 0.03 | 0.03 | 0.04 | 0.04 |
| `002_advanced_excel_line` | 0.09 | 0.09 | 0.17 | 0.17 |
| `003_advanced_excel_pie` | 0.24 | 0.24 | 0.24 | 0.24 |
| `005_advanced_excel_area` | 0.06 | 0.06 | 0.07 | 0.07 |
| `006_advanced_excel_scatter` | 0.23 | 0.23 | 0.43 | 0.43 |
| **`007_advanced_excel_bubble`** | **0.88** | **0.37** | **0.93** | **0.39** |
| `016_advanced_excel_radar` | 0.11 | 0.11 | 0.11 | 0.11 |
| **`028_advanced_excel_doughnut`** | **0.36** | **0.16** | **0.36** | **0.16** |

Two movers, six controls unmoved to the hundredth — including both of the movers' own
type-mates, which is what rules out the change reaching the pie and scatter paths.

The "before" leg was taken by restoring `XlsChartReader.cs` from `HEAD`, **rebuilding**, and
re-rendering; the source was put back with `cp` and `touch` afterwards, because `mv` keeps
the old mtime and MSBuild then skips the project — the trap `dotnet/CLAUDE.md` records.

## 5. Reach on the corpus: nil

`census.py` walks every OLE2 file in the corpus — not every `.xls`, because a chart
substream can also sit in a `.doc` or `.ppt` ObjectPool, which is the trap that made an
earlier chart census short by eight. Over **180 OLE2 files**:

```
scatter      014_Contextures_chart_sample_991ecfc5.xls
OLE2 files scanned: 180
```

One document, one plain scatter, already right. **No corpus rendering moves.** This is
correctness for a construct the corpus holds only in its OOXML spelling — where both types
have had a record of their own since the first chart round — measured on the reference's
own conversion of that same content.

## 6. Two BIFF gaps recorded and not implemented

Neither has a fixture that can be obtained here, so both are left named rather than guessed
at:

* **`CHSURFACE` 0x103F.** Read as the default `Bar`, which is coincidentally what the
  reference substitutes for it — see `DrawingChartPlot.KindOf`'s own measurement of
  `c:surface3DChart`. No corpus file states one and `--convert-to xls` will not write one.
* **`CHPIEEXT` 0x1061**, pie-of-pie. `Finalize` gives it `EXC_CHTYPEID_PIEEXT` and
  `CreateChartType` then declines to apply the pie rotation to it (`:2420`). `ChartPlot`
  has `ChartPlotKind.OfPie`, so the reader change would be small; what is missing is
  something to measure it against.

## 7. Confinement

Our half of every document the diff can possibly reach, rendered twice — at the round's base
and with the change — under `SOURCE_DATE_EPOCH=0`, one output directory per document, three
workers:

| set | documents | renderings that moved |
|---|---:|---:|
| every OLE2 file under `sheets/` plus `corpus-odf/ods` | 370 | **0** |
| the 10 `corpus-odf/odt` holding a `chart:chart` | 10 | **0** |

**380 of 380 byte-identical**, which is what both censuses predict and is the whole reach
statement for this round: the corpus has no BIFF doughnut, no BIFF bubble chart and no
unrecognised ODF chart class.

`XRay.Office.Fidelity.Tests` is **10 failed of 552 before and after**, the same four classes
each time — `TabStopComparisonTests`, `PageDrawingComparisonTests`,
`JustificationShrinkComparisonTests` and `SheetDrawingComparisonTests` — all of which
`dotnet/CLAUDE.md` records as left failing on purpose, and none of which is a chart test.
The base leg was measured by restoring both source files from `HEAD`, rebuilding the whole
solution, and re-running.

## Files

| file | what it is |
|---|---|
| `make.sh` | regenerates the eight fixtures through 26.2.4.2 |
| `records.py` | the `CHPIE`/`CHSCATTER` bodies of a workbook, out of its OLE2 streams |
| `measure.sh` | renders both ways and prints worst and summed `\|ink\|%` |
| `census.py` | which corpus OLE2 files state a doughnut or a bubble chart |
