# A secondary axes set, the cells a chart is allowed to plot, and three smaller readings

Measured 2026-09-06 in `/home/user/wt-secaxis`, branch `agent/secaxis`, base `0fc357beb`.
Reference `/opt/libreoffice26.2/program/soffice` **26.2.4.2**, its Latin metric duplicates and its
Latin `NotoSans`/`NotoSerif` moved aside; `/usr/bin/soffice` is **24.2.7.2** and is what the gate's
own reference half was rendered with. Corpus `/home/user/sample-files`, 947 documents.
Ink is the mean absolute grey difference at 30 dpi, page for page, over the shared pages — the same
measure as `probes/chart-layout/ink.py`, with the reference render cached.

The brief was three documents read blind by fresh reviewers, eleven symptoms between them. **Seven
are closed, one is closed as a ruling that leaves our output alone, and three are left with their
mechanisms named and their reach measured.** Nothing in the corpus moves backwards.

**There is no fresh reader in this container** — `CLAUDE.md`'s standing note is still correct — so
every reading below is either the brief's (uncontaminated, written by reviewers who had never seen
the documents) or arithmetic off the PDFs: filled-rectangle extents from PyMuPDF's `get_drawings`,
glyph strings and spans from its text dictionary. My own looks at these pages are contaminated and
nothing here rests on one.

---

## 0. What the three documents turned out to share, and what they did not

The brief asked whether the Pareto chart's `$1`/`$0` ticks were a wrong axis maximum or truncated
labels, on the grounds that one cause — the secondary axis not being honoured — might explain the
ticks, the overshoot and the missing percentage axis together. **It is a wrong maximum, the
brief's reading is right, and it explains a fourth symptom as well.** But the shape of the defect is
not "we ignore the secondary axis and plot everything on the primary": it is that the BIFF chart
reader had **one** value-scale field, and the *second* `CHAXESSET`'s `CHVALUERANGE` overwrote the
first's. The primary axis was drawn with the secondary's numbers and the primary's currency format.

That is arithmetic, not a reading. The file states primary 0…800 step 80 and secondary 0…1.01 step
0.1; eleven ticks from 0 to 1.0 through `$#,##0` are `$1 $1 $1 $1 $1 $1 $0 $0 $0 $0 $0`, six ones
and five zeros, which is exactly the string the reviewer read off the page.

**Documents A and B share a class and not a cause.** Both are charts with *two axis pairs*, and in
both the defect is which pair the rest of the chart is taken from — but A is a BIFF substream whose
reader had no notion of an axes set at all, and B is an OOXML part where the axes were already
resolved correctly and only the *categories* came from the wrong group. Different readers,
different seats, one shared idea.

**C shares nothing with either.** Its three symptoms are three unrelated readers: a per-point fill
that one drawing function did not ask for, a chart-space text weight that no reader read, and a
category difference that is LibreOffice's own limitation.

---

## 1. `014_Contextures_chart_sample_991ecfc5.xls` — the Pareto chart

`sheets/chartset-006/xls`. Seven symptoms; **five closed, two left.**

| symptom | cause | closed |
|---|---|:-:|
| value axis reads `$1 … $0` | the secondary axes set's `CHVALUERANGE` overwrites the primary's | yes |
| no secondary percentage axis | the same — there was one scale field and no `AxisIndex` | yes |
| bars overshoot the plot by ~300× | the same — a 305 bar against a 0…1.01 scale | yes |
| `Cumulative %` draws no line, legend key a hollow square | the second `CHTYPEGROUP`'s type record was dropped, so its series was drawn as a bar — and that series states `EXC_PATT_NONE`, which fills nothing | yes |
| seven categories plus thirteen `#N/A` | `CHPROPERTIES`' *show visible cells only* was not read, and rows 20–32 of the source sheet are hidden | yes |
| bars a third of their slot with gaps | `CHBAR`'s overlap and gap were skipped past | yes (not in the brief) |
| legend outside right and vertical | `CHLEGEND`'s *not docked* mode and its `CHFRAMEPOS` rectangle | **no** |
| no thin black outer frame | an *automatic* `CHLINEFORMAT` on the chart's own `CHFRAME` | **no** |

### 1.1 The three records a BIFF chart states its structure in, and none of them were read

A chart substream is a tree written flat, and the structure is three records deep:

- **`CHAXESSET`** carries the axes-set id as its first field — `XclImpChAxesSet::ReadHeaderRecord`,
  `sc/source/filter/excel/xichart.cxx`:3542-3546, with `EXC_CHAXESSET_PRIMARY` 0 and
  `_SECONDARY` 1 (`sc/source/filter/inc/xlchart.hxx`:583-584).
- **`CHTYPEGROUP`** carries its own group index after sixteen ignored bytes and the flags —
  `XclImpChTypeGroup::ReadHeaderRecord`, `:2683-2688` — and its *type* is a separate record inside
  it (`CHBAR`, `CHLINE`, `CHSCATTER`, …), all of which fall through `ReadSubRecord` to
  `maType.ReadChType` (`:2714-2715`).
- **`CHSERGROUP`** names the group a series belongs to — `XclImpChSeries::ReadSubRecord`,
  `:1825-1826` — and the index is unique across the whole chart, which is why
  `XclImpChChart::GetTypeGroup` looks it up in the primary set and then in the secondary
  (`:3948-3954`).

`XlsChartBuilder` read none of the three. `_valueScale` was one field written by every
`CHVALUERANGE`; `_valueFormatIndex` one field written by every `CHFORMAT` on a Y axis; `SetKind`
kept the first type record and discarded the rest; and `ChartSeries.AxisIndex` was never set, so
`ChartPlot.HasSecondaryAxis` was false for every BIFF chart in existence.

**The order the file writes them in is what made the defect this shape.** The primary set is
written first, so the last `CHVALUERANGE` read is the secondary's — and the last `CHFORMAT` read is
the *primary's*, because the secondary Y axis states none. Hence a 0…1.01 scale written through a
currency format.

### 1.2 The hidden cells, which are a different rule in a different library

`CHPROPERTIES`' `EXC_CHPROPS_SHOWVISIBLEONLY` (`xlchart.hxx`:599) becomes the diagram's
`IncludeHiddenCells`, negated, at `XclImpChChart::Convert`, `xichart.cxx`:4027-4028. Calc then
honours it **where the cells are fetched**, not where the chart is built:
`ScChart2DataSequence::BuildDataCache` asks `ColHidden` and `RowHidden` per cell and `continue`s
past a hidden one (`sc/source/ui/unoobj/chart2uno.cxx`:2636-2646) — so the points after it move up
and the sequence is *shorter*, rather than carrying a gap.

Read out of the file: the chart plots `ChartSourceData!C13:C32` against `B13:B32`, twenty rows;
rows 20–32 hold `#N/A` (formula cached error code `0x2A`) and every one of them carries a `ROW`
record with the hidden flag; `CHPROPERTIES`' flags are `0x001B`, which includes `SHOWVISIBLEONLY`.
Twenty minus thirteen is seven, and seven is what the reference draws.

**And the arithmetic settles it without looking at the page.** The reference's plot spans
x = 93.68 … 509.22 and its first bar 93.68 … 153.04, so one slot is 59.36 and the plot is 415.54 —
exactly **7.0** slots. Ours was 412.23 wide with a 20.61 pitch, exactly **20**.

### 1.3 `CHBAR`'s two skipped fields

`XclImpChType::ReadChType` reads `mnOverlap` then `mnGap` (`xichart.cxx`:2234-2238) and
`CreateChartType` hands chart2 **`-mnOverlap`** and **`mnGap`** (`:2404-2410`) — where `oox` hands
the same two properties `c:overlap` and `c:gapWidth` unchanged
(`oox/source/drawingml/chart/typegroupconverter.cxx`:459-461. **BIFF counts an overlap the opposite
way round from OOXML.** The reader was doing `stream.Skip(4)` over precisely those two fields, so
every BIFF bar chart was drawn at `ChartPlot`'s own 100 % gap. This file states nought for both,
which is why the reference's seven bars touch.

### 1.4 Measured, before and after

| | before | after | 26.2.4.2 |
|---|---|---|---|
| primary value axis' eleven ticks | `$1 $1 $1 $1 $1 $1 $0 $0 $0 $0 $0` | `$800 $720 $640 … $0` | identical |
| secondary axis | absent | `100.0% 90.0% … 0.0%` | identical |
| category labels | 7 names + 13 `#N/A` | the 7 names | identical |
| category slots | 20 | 7 | 7 |
| first bar | x 65.81–72.68, top **y = −41629** | x 69.88–121.72, top y = 215.16 | x 93.68–153.04, top y = 215.60 |
| bar height as a fraction of the plot | — | **0.3812** | **0.3815** |
| `Cumulative %` | no stroke drawn | one 6-segment blue path | one 6-segment blue path, plus 7 square markers |
| page-1 ink | **7.84** | **5.49** | — |
| document mean ink | 2.68 | 1.89 | — |

The residual is the plot rectangle — ours 69.88–432.73 × 125.85–270.19 against the reference's
93.68–509.22 × 120.87–274.08 — and most of that width is §1.5: our legend takes a band on the right
where the reference's sits inside the plot.

### 1.5 What is left on this document, with its reach

**The legend's stated position.** `CHLEGEND`'s dock mode is 7, `EXC_CHLEGEND_NOTDOCKED`, and
`XclImpChLegend::CreateLegend` (`xichart.cxx`:2561-2620) then ignores the dock mode entirely and
sets `RelativePosition` from the `CHFRAMEPOS` inside the legend — *"Only the settings from the
CHFRAMEPOS record are used by Excel, the position in the CHLEGEND record will be ignored"*. This
file's is x = 330, y = 941, w = 1295, h = 191 in 1/4000 of the chart, which is a wide, short block
8 % from the left and 24 % down: inside the plot, upper left, horizontal — the reviewer's reading
exactly. `ChartPlot.Legend` is a four-way enum with no manual placement and no overlay, and
`ChartLayout` reserves a band for whichever side it names, so honouring this is a **feature**: a
legend rectangle that does not take room from the plot. **Reach: 14 of the corpus's 16 BIFF chart
substreams, in 5 documents**, state `NOTDOCKED`. Left because it changes the plot rectangle of every
one of them and this round could not sweep twice.

**The chart's outer frame.** The chart's own `CHFRAME` holds a `CHLINEFORMAT` with
`EXC_CHLINEFORMAT_AUTO` set, and `ReadLineFormat` returns on that flag and on any frame at all —
*"the axis lines and the frames' borders already have their own rules"*. An automatic chart-frame
border is a black hairline in both Excel and LibreOffice. Left because the automatic *colour* comes
from a chart palette this reader does not have (the same limit `ReadAreaFormat` already records)
and because reading only the line would need that palette to be right about the axis lines too.

**Whether a secondary axis is *shown* is not read, and BIFF states it.** `ChartPlot`'s
`SecondaryAxisVisible` and `SecondaryLabelsVisible` default to true and the BIFF reader sets
neither, so a chart with a secondary axes set always gets a drawn secondary axis. `CHTICK`'s
tick-label position and `CHAXISLINE`'s line format are what say otherwise, and neither is read on
either axis — the primary's visibility is not read either, so this is a pre-existing limit rather
than something the round introduced. It costs nothing on this corpus: the one document with a
secondary axes set draws it, and the reference draws it too.

**The line's markers.** `CHMARKERFORMAT` (0x1009) is not read at all. This file states type 1
(square), fore and back `#0000FF`, size 100 (5 pt) — the reference draws seven 5 × 5 pt squares and
we draw none. It is a self-contained record with its own colour precedence and belongs with the
legend work rather than with this.

---

## 2. `055_Project_timeline_with_milestones….xlsx` — the timeline

`sheets/chartset-008/xlsx`. Five symptoms; **three closed, one part-closed, one left.**

### 2.1 `[CELLRANGE]` is never drawn — neither resolved nor unresolved

The reader already substituted a `c15:dlblRangeCache` string for a `CELLRANGE` field and a previous
round recorded the mechanism correctly. What it did **not** have is what happens when the cache has
no entry for the point: it left the field carrying its own `a:t`, which is the localised placeholder.

LibreOffice reaches *empty* down both of its paths, and that is the whole rule:

- with a `c15:datalabelsRange` present, `oaCellRange` is set from the source's **formula** — which
  every such element has — so the `CELLRANGE` branch is taken whether or not the point has a cached
  string, and the string is `oaLabelText.value_or("")`
  (`oox/source/drawingml/chart/seriesconverter.cxx`:366-410);
- with none present, `setDataLabelsRange` is never called, and `VSeriesPlotter`'s `CELLRANGE` case
  writes an empty string for exactly that reason (`chart2/source/view/charttypes/VSeriesPlotter.cxx`:535-541).

Neither path can reach the `a:t`. The seat is `ChartDataLabel.Resolve`'s `default:` arm, which
appended a part's literal text for any field it did not know; a `CellRange` part now draws nothing,
and the reader marks a *resolved* one as `Literal` so the two cannot be confused.

**Reach: 6 chart parts in 6 documents state a `CELLRANGE` field**, and the same six state a
`c15:datalabelsRange`.

### 2.2 The categories come from the primary axes set, and the primary axes set is not the first group

The part holds a `c:barChart` (an invisible `a:noFill` position series, categories `D20:D36` = the
thirteen milestone names) and a `c:lineChart` (categories `C20:C36` = seventeen dates), on two axis
pairs; the `c:catAx` is `c:delete val="1"` and the `c:dateAx` is not.

chart2 has **one** coordinate system and therefore one set of categories —
*"For now, all type groups from all axes sets have to be inserted into one coordinate system"*
(`oox/source/drawingml/chart/plotareaconverter.cxx`:180-183) — and
`AxisConverter::convertFromModel` takes them from one axes set only:

```cpp
// oox/source/drawingml/chart/axisconverter.cxx:282-290
if( nAxesSetIdx == 0 )
    aScaleData.Categories = rTypeGroups.front()->createCategorySequence();
```

Which set is index 0 is `plotareaconverter.cxx`:466-468: normally the first type group's, but for a
**combined** chart — exactly two type groups of different kinds — it is the set whose value axis
comes first in the plot area's own element order, whichever group wrote it. Here the bar group is
written first and the line group's `c:valAx` is, so the dates win: on the axis, and as every data
label's `[CATEGORY NAME]`.

Our reader took the categories from the first group in document order that had any. It already
resolved the two *axes* correctly — `ChartAxes.Read` picks the first `c:valAx` as the primary — and
a census confirms that is not a coincidence worth fixing separately: **over all 15 corpus chart
parts that hold two axes sets, our primary value axis is LibreOffice's on 15 of 15**, while the
*category group* differs on **3 parts in 3 documents**, all three combined bar-over-line charts
(`axesset-census.py`). So the change is one condition and its reach is three documents.

| | before | after | 26.2.4.2 |
|---|---|---|---|
| category axis | 13 milestone names, rotated, spilling over the heading below | 15 date labels, `5 Apr … 23 Aug` | ~38 date labels, `5 Apr … 19 Apr` |
| a data label's second line | `Design phase` | `5/1/2023` | `1 May` |
| `[CELLRANGE]` drawn | twice | none | none |
| page-1 ink | **1.64** | **1.38** | — |

### 2.3 What is left here

**The date axis' automatic maximum, and it is the whole residual.** The categories are serials
45021 … 45169 — 5 April to 31 August 2023, 148 days — and the axis states
`c:majorUnit val="10"` with `baseTimeUnit days`. We draw 5 Apr … 23 Aug at that ten-day step, which
is the data's own range. The reference draws **5 Apr 2023 … 19 Apr 2026 at a 30-day step**, 38
labels, read off its own PDF at x = 48.4 … 593.9 — so its axis maximum is about serial 46132, a
thousand days past the last category, and its step is months because ten days over that range would
be a hundred ticks. Four of the seventeen rows the range covers are **empty**, and neither the cells
nor the caches hold anything near 2026, so where its maximum comes from is not yet established.
The label *format* and the axis *kind* are the halves the brief named and both are now the
reference's; the range is a `ScaleAutomatism` question for a date axis and is a round of its own.

**Leader lines.** The part states `c15:showLeaderLines` and `c15:leaderLines` with their own
`c:spPr`; nothing in `dotnet/src` reads either. Not censused; not attempted.

---

## 3. `029_Annual_budget….xlsx` — the two-bar column chart

`sheets/chartset-009/xlsx`. Three symptoms; **two closed, one ruled on and deliberately left
alone.**

### 3.1 A bar's colour is the point's, and one drawing function did not ask

`ChartSeries.PointFills` has carried `c:dPt/c:spPr` since it was written, and `AddWedges`,
`AddAreas`, `AddBubbles` and every label builder ask `ChartSeries.FillAt` for it. **`AddBars` alone
asked `ChartSeries.Fill`**, so a bar chart's per-point fills were parsed, stored and never drawn.

`029_Annual_budget`'s chart states `tx2` at `lumMod` 60000 / `lumOff` 40000 for point 0 and
20000 / 80000 for point 1; its theme's `dk2` is `#242852` and its `accent1` is `#4A66AC`. Read off
the two PDFs:

| | before | after | 26.2.4.2 |
|---|---|---|---|
| bar 1 | `#4A66AC` | **`#5C64B7`** | `#5C64B7` |
| bar 2 | `#4A66AC` | **`#C9CBE7`** | `#C9CBE7` |

**Reach: 33 non-pie plot groups in 13 corpus documents state a per-point fill** — thirty of them
bar groups — and **no non-pie group anywhere in the corpus states `c:varyColors val="1"`**
(`dpt-census.py`), so the other half of `PointFills`, the accent cycle, cannot fire and this change
can only put a stated colour where a stated colour belongs.

### 3.2 The chart space's own text weight

`c:chartSpace/c:txPr/a:p/a:pPr/a:defRPr` sits between the auto-text table and each object's own
statement, exactly as it already does for the *size*: `TextFormatter` seeds its automatic properties
from the table — a title bold, an axis title bold, everything else regular — and then
`mxAutoText->assignUsed(*pTextProps)` with the chart space's own properties
(`oox/source/drawingml/chart/objectformatter.cxx`:906-929, the properties fetched by
`lclGetTextProperties` at `:897-901` and the text body handed in at `:950`). `assignUsed` copies
every property the source *states*, so a global `b="1"` makes an axis label bold and a global
`b="0"` makes an otherwise-bold title regular.

`AutoText` already read the height out of that element; `AutoWeight` now reads the weight, and
`IsTitleBold`, `IsAxisTitleBold`, `IsLabelBold`, `IsLegendBold` and `IsDataLabelBold` all fall
through it.

Measured on this document's page 1, by font of every span: `DejaVuSans-Bold` **68 glyphs in the
reference, 68 before → 80 after** — and the twelve are exactly the difference between the
reference's `1`/`2` category labels and our `Income`/`Expenses`. Span for span, the set of bold
strings is now the reference's: `SUMMARY`, `BALANCE`, `PERCENTAGE OF INCOME SPENT`, `$1,524`,
`62%`, the eleven value-axis ticks, both data labels and both category labels — where before only
the five sheet-cell strings were bold and every string inside the chart was regular.

**Reach: 17 chart parts in 8 documents**, and both directions occur — twelve parts in six documents
state `b="0"` and five parts in two state `b="1"`.

Page-1 ink **0.51 → 0.32**.

### 3.3 The ruling: `Income`/`Expenses` stays, and the reference's `1`/`2` is a Calc limitation

**Ours is right and the reference is failing to read data it holds. Left alone, deliberately.**

The chart states its categories as a `c:strLit` — a literal list, not a cell reference. For a
workbook the chart converter is `ExcelChartConverter::createDataSequence`
(`sc/source/filter/oox/excelchartconverter.cxx`:66-121), which has two arms: a formula, which this
has not, and constant data, for which it generates an API array `{"Income";"Expenses"}` and calls
`createDataSequenceByRangeRepresentation`. `ScChart2DataProvider::createDataSequenceByRangeRepresentation`
compiles that string looking for **reference tokens**, finds none in a literal array, and returns an
empty reference (`sc/source/ui/unoobj/chart2uno.cxx`:2092-2103); its
`createDataSequenceByValueArray` — the entry point `oox`'s own converter would have used — is a
one-line `return uno::Reference<...>();` (`:2106-2112`). With no category sequence, chart2 generates
`1`, `2`, … Impress and Writer do not have this hole: `InternalDataProvider::createDataSequenceByValueArray`
is implemented and parses the array (`chart2/source/tools/InternalDataProvider.cxx`:836-842).

**Measured rather than inferred, and the control is decisive.** `mkstrlit.py` builds two decks from
one corpus `.pptx`, identical but for the one element: `strlit.pptx` states the same `c:strLit`
categories and `strref.pptx` a `c:strRef` with a cached copy. Rendered through 26.2.4.2, **both draw
`Income` and `Expenses`.** The same `c:strLit` in a workbook draws `1` and `2`. So the difference is
the application that owns the chart, not the chart markup — a Calc data-provider limitation, and
the standing instruction to close the gap against LibreOffice 26 does not apply to it.

The corpus holds **6 chart parts in 5 documents** with `c:strLit` or `c:numLit` data, of which two
are `c:strLit` categories and both are workbooks: this one and
`037_Personal_money_tracker`, whose single literal category sits on a deleted axis and so shows on
neither side.

---

## 4. Reach, gathered

Every figure from `census.py`, `dpt-census.py` and `axesset-census.py` over all 947 documents.

| mechanism | parts | documents |
|---|---:|---:|
| BIFF chart substreams at all | 16 | 7 |
| … stating **two axes sets** (`CHUSEDAXESSETS` > 1) | **1** | **1** |
| … stating more than one `CHTYPEGROUP` | 1 | 1 |
| … stating `SHOWVISIBLEONLY` | 14 | 6 |
| … stating it **and** holding a hidden row or column | 10 | **2** |
| … whose legend is `NOTDOCKED` | 14 | 5 |
| OOXML chart parts at all | 309 | 169 |
| … with two axis pairs | 15 | 11 |
| … where our primary value axis differs from LibreOffice's | **0** | **0** |
| … where the *category group* differs | **3** | **3** |
| … with a `CELLRANGE` field | 6 | 6 |
| … with a `c15:datalabelsRange` | 6 | 6 |
| … with a chart-space `c:txPr` weight | 17 | 8 |
| non-pie plot groups with a per-point `c:dPt` fill | **33** | **13** |
| non-pie plot groups with `c:varyColors val="1"` | **0** | **0** |
| chart parts with `c:strLit`/`c:numLit` data | 6 | 5 |

**The two largest are the two the brief did not lead with.** The per-point fill reaches 13
documents and the chart-space weight 8; the BIFF secondary axes set reaches **one**. That ordering
is the round's most reusable result: a symptom that produces the worst-looking page is not the one
with the widest reach, and both were worth doing for different reasons.

---

## 5. Verification

| | baseline at `0fc357beb` | after |
|---|---|---|
| `dotnet build XRay.Office.slnx -v q -nologo` | 0 warnings, 0 errors | **0 warnings, 0 errors** |
| ten non-fidelity projects, run individually and totalled | 5841 / 0 failed / 0 skipped | **5859 / 0 / 0** — the 18 are this round's own new tests |
| `XRay.Office.Fidelity.Tests` | 542 / 10 / 0 of 552 | **542 / 10 / 0 of 552** |

Per project after: Containers 109, Core 457, Markup 259, OpenDocument 128, Presentations 956,
Rendering 162, Spreadsheets 1121, Text 723, Vector 302, WordProcessing 1642.

**One run truncated and was caught by the count.** The final pass reported WordProcessing at
**1469 passed, 0 failed** where the two earlier passes both reported 1642 — `CLAUDE.md`'s exact
signature, *"a drop with zero failures is a truncated run, not a fixed test"*. Re-run alone it
reported **1642 twice**, so 1642 is the figure and 5859 is the total. Nothing else needed a second
run and nothing failed once.

The ten fidelity failures are the briefed ones, by name:
`TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop` ×4 (`docx`, `doc`, `fodt`, `odt`),
`PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt` ×4 (`doc`, `rtf`, `fodt`,
`docx`), `SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt` (`sheet-rich-text.xlsx`)
and `JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt`
(`justify-shrink-2013.docx`).

### The corpus, before and after

**947 of 947 scored against the gate's banked reference PDFs: 865 match, 82 mismatch**, against
the gate's own 860 / 87 at `2f4709c08`. Per track: words 314 match / 20 pages / 2 pages+words /
2 words; slides 292 match / 9 words / 1 unembedded; sheets 259 match / 40 words / 6 pages+words /
2 pages.

**Seven verdicts differ from the gate's, and the gate's column is `2f4709c08`'s while this round's
base is `0fc357beb`** — many commits later — so each one is attributed rather than claimed.

| document | gate said | now | attribution |
|---|---|---|---|
| `014_Contextures_chart_sample_991ecfc5.xls` | `words` | **`match`** | **this round.** 564 glyphs against the reference's 561; the thirteen `#N/A` category labels are 52 glyphs and the axis labels changed with them |
| `N2_E_Maestroni_Swarm_COP.pptx` | `words` | `match` | `probes/chart-layout` §Verification, which attributed it to itself and states the same 28100 |
| `062_Run_chart` | `pages,words` | `match` | recorded there as *not this round*, identical binary |
| `057_Simple_balance_sheet` | `pages,words` | `words` | recorded there as *not this round*, identical binary |
| `024_Unit_Circle_Chart_Colorful_Circles.docx` | `words` | `match` | recorded there as *not this round*, identical binary |
| `AAC-AD-No-2021-01-Boeing-737-8…doc` | `pages` | `match` | recorded there as the era plus that round's own three glyphs |
| `150_5300_13_chg10.doc` | `pages` | `pages,words` | **not this round, measured.** Rendered with a binary built from these six source files at their base contents it gives **77 pages and 118561 glyphs**, identical to this tree's; the gate's stored 117451 is from a commit before this round's base |

**Nothing moved backwards that this round caused**, and the one thing that moved backwards at all
was reproduced on the unfixed tree.

### Ink against 26.2.4.2, over the 39 documents this round can reach

`affected.txt` is the union of every census: the seven BIFF chart documents, the thirteen with a
non-pie per-point fill, the eleven with two axes sets, the six with a `CELLRANGE` field and the
eight with a chart-space weight.

**15 improve, 0 worsen, 24 unchanged to the hundredth.** The sum of the 39 means goes
**66.96 → 64.74**.

| document | mean before | mean after | worst before | worst after |
|---|---:|---:|---:|---:|
| `014_Contextures_chart_sample_991ecfc5.xls` | 2.68 | **1.89** | 7.84 | **5.49** |
| `096_Lab_Flask_Charts…pptx` | 1.96 | **1.50** | 3.70 | **2.32** |
| `033_Battery_Percentage_Chart` | 1.44 | **1.15** | 2.26 | **1.66** |
| `055_Project_timeline_with_milestones` | 0.86 | **0.73** | 1.64 | **1.38** |
| `034_Personal_net_worth_calculator` | 0.20 | **0.10** | 1.04 | **0.35** |
| `029_Annual_budget` | 0.65 | **0.55** | 0.78 | 0.78 |
| `036_Simple_to-do_list` | 1.02 | **0.92** | 2.09 | **1.59** |
| `045_Check_register_with_chart` | 0.38 | **0.32** | 0.38 | **0.32** |
| `052_Flame_Percentage_Chart` | 1.26 | **1.22** | 1.87 | **1.76** |
| `062_Run_chart` | 2.63 | 2.57 | 2.79 | 2.79 |
| `southern-classic-kennesaw-state-university-final.pptx` | 1.84 | 1.78 | 3.91 | 3.91 |
| `003_Contextures_chart_sample.xlsm` | 0.25 | 0.24 | 0.81 | 0.70 |
| `037_Personal_money_tracker` | 1.70 | 1.69 | 3.34 | 3.34 |
| `orbus_togaf_tool_csq.xls` | 1.85 | 1.84 | 5.02 | 4.96 |
| `TOGAF9-Tool-ConfReqts-CSQ.xls` | 1.40 | 1.40 | 5.51 | 5.49 |
| the other 24 | — | unchanged | — | unchanged |

**Two of the three documents the category rule can reach do not move, and that is the result worth
having.** `038_Baby_growth_chart` and `039_Baby_growth_tracker` are the other two combined
bar-over-line charts, and both groups of both charts state the *same* four categories
(`At birth`, `4 days`, `7 days`, `11 days`) — so taking them from the other group is a no-op, and
the change moves the drawing on the one document where the two groups disagree.

### The new tests, run against the unfixed tree

Eighteen tests were added and **ten of them fail** on a binary built from these six source files at
their base contents; the other eight are the controls and pass on both sides, which is what a
control is for.

| fails unfixed | passes on both (control) |
|---|---|
| `ASecondaryAxesSetDoesNotOverwriteThePrimaryScale` | `OneAxesSetLeavesNoSecondaryScale` |
| `ASeriesGroupPutsItsSeriesOnThatGroupsAxis` | `AChartWithNoBarGroupKeepsTheDefaultGap` |
| `ASeriesTakesItsOwnTypeGroupsKind` | `AHiddenRowIsKeptWithoutTheFlagAndAShownRowIsAlwaysKept` |
| `ABarGroupStatesItsGapAndItsOverlap` | `ASeriesWithNoPointFillsIsAllOneColour` |
| `AHiddenRowIsLeftOutWhenTheChartPlotsVisibleCellsOnly` | `AChartWithOneGroupIsUnchanged` |
| `EachBarTakesItsOwnPointsFill` | `SwappingWhichValueAxisComesFirstSwapsTheCategories` |
| `APointStatingNothingKeepsTheSeriesFill` | `AResolvedCellRangeFieldDrawsItsCachedString` |
| `TheCategoriesComeFromThePrimaryAxesSet` | `AnAxisStatingItsOwnWeightKeepsIt` |
| `TheChartSpacesOwnWeightIsWhatUnstatedTextFallsBackTo` | |
| `AnUnresolvedCellRangeFieldDrawsNothing` | |

The unfixed binary was built by copying the six files aside, writing `git show HEAD:<path>` over
each and clearing `obj`/`bin` for the four projects; it was restored with `cp` and `touch`, the
same four `obj`/`bin` cleared again, and the restored tree checked by re-rendering four documents
and comparing them byte for byte against the sweep's own copies — **identical on all four once
`/CreationDate` is masked.**

### The sweep, and a rebuild that overlapped it

`sweep.py` renders and scores one document at a time. On this four-core container, shared with
three other agents, that came to about four rows a minute — three hours for the corpus — so the
rendering was split out into `prerender.py` (six workers, each render moved into place with
`os.replace`, which is atomic, so the scorer can never read a half-written file) and the scoring
into `sweep-parallel.py`. Both carry `batch-check.sh`'s verdict rule unchanged and both validate
it against the gate's own two halves before scoring: **947 of 947 stored verdicts reproduced.**

**A rebuild landed while that sweep was in flight and it was mine.** Two XML doc comments and one
`//` comment were corrected after the sweep started and the solution was rebuilt without thinking
about it — which is `CLAUDE.md`'s "a sweep and a rebuild must never overlap", broken. The check it
prescribes was run rather than the rule being argued away: four documents rendered before the
rebuild were re-rendered after it and compared byte for byte against the sweep's own copies. All
four differ raw and are **identical once `/CreationDate` is masked**, which is the same file. A
comment cannot change the IL, and the comparison says so rather than assuming it.

## Files

| file | what it is |
|---|---|
| `biffdump.py` | a BIFF record dumper that indents a chart substream by `CHBEGIN`/`CHEND`; how the Pareto chart's structure was read |
| `census.py`, `census-biff.tsv`, `census-ooxml.tsv` | axes sets, type groups, `SHOWVISIBLEONLY`, legend docking, per-point fills, `CELLRANGE`, literal data and chart-space weights, over all 947 documents |
| `dpt-census.py`, `census-dpt.tsv` | per-point fills and `varyColors` on **non-pie** groups only, which is the population `AddBars` can move |
| `axesset-census.py`, `census-axesset.tsv` | LibreOffice's primary axes set against ours, part by part, for every chart with two |
| `mkstrlit.py` | the two probe decks that separate a Calc data-provider limitation from a chart one |
| `ink.py`, `affected.txt`, `ink-before.tsv`, `ink-after.tsv` | the corpus documents this round can reach, and their ink against 26.2.4.2 before and after |
| `sweep.py` | the serial sweep, taken unchanged from `probes/chart-layout` with the CLI path repointed |
| `sweep-parallel.py`, `sweep-after.tsv` | the same verdict rule scored with a thread pool, which is what actually produced the scoreboard; it validates against the gate's own two halves and reproduces 947 of 947 before it scores anything |
