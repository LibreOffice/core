# Results — a BIFF date axis, and three things the tree says that the binary does not

Scores `prediction.md`, committed first at `38de349a259`. The reference half is the banked
26.2.4.2 set at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/`; nothing was re-rendered on the
reference side except the purpose-built probe workbooks described below.

## 0. The deficit was real, and the first measurement was the one that said so

`sheets/batch-010/xls/Template Pilot Logbook JAR-FCL V3.0.xls`, page-exact at 38/38, 204 words
short. `TODO.raster-ceiling.md` names four shapes where a word count lies; the whitespace-stripped
character streams rule all four out here:

| page | ours, before | reference |
|---|---:|---:|
| whole document | 6121 | 6529 |
| 16 | 128 | 352 |
| 17 | 91 | 383 |
| 18 | 19 | 55 |

Five other pages differ by a few characters each way at identical word counts — non-breaking
spaces and column padding. **Pages 16, 17 and 18 are the whole deficit and they are the three
chart pages.** The reference draws 30, 38 and 4 date labels there; we drew 2, 2 and 0.

## 1. LibreOffice's tick rule, measured

The rule was established by building probe workbooks and rendering them through the installed
26.2.4.2, then confirming each step against the banked reference for the corpus document.
`mkchart.py` builds a one-chart `.xlsx` with a stated category set; `labels.py` reads the tick
labels back out of a PDF whether they are horizontal, at 45° or at 90°.

The reference's page 17 draws `30/12/99`, then `02/01/03`, `02/01/06` … `02/01/11` — 38 labels.
Page 16 draws `30/12/99`, then `02/11/03`, `02/09/07` … `02/03/11` — 30, at an apparent
**46-month** step. Neither sequence is a whole number of years from its own first label and both
keep the day `02` for ever. Four rules account for all of it:

1. **The axis minimum is serial 0** — 30 December 1899 — on data that runs 37935 to 41292.
2. **The tick grid is 2 months**, not 3 years. `lcl_getMaximumAutoIncrementCount` gives a DATE
   axis `MAXIMUM_MANUAL_INCREMENT_COUNT` = 500, and
   `VCoordinateSystem::prepareAutomaticAxisScaling` **returns early for a date X axis, before the
   call that would narrow it** from the axis' own measured labels. So the interval is always
   `days / 499`: 41292/499 = 82 days, which is more than a month and less than a year, so months,
   `floor(82/31)` = 2 of them. 679 ticks.
3. **Ticks are calendar additions with roll-over, not clamping.** 30 December + 2 months is
   30 February, and `comphelper::date::normalize` subtracts the month's length and carries —
   2 March in a common year. Every later tick keeps day 02. .NET's `DateOnly.AddMonths` clamps to
   the 28th and gets every tick after the first wrong.
4. **The thinning is the ordinary collision ladder.** 679 ticks at rhythm 18 give 38 labels
   (page 17) and at rhythm 23 give 30 (page 16). Both counts reproduce exactly.

A probe carrying the same 799 categories — one at 37935, 24 at 41258–41292, 774 blank — draws
`30/12/1899, 02/11/1902, 02/09/1905 …`: the same minimum, the same two-month grid, the same
day-02 roll-over.

### The axis minimum is a plotter's doing, and the workbook says the opposite

`AreaChart::addSeries` (`chart2/source/view/charttypes/AreaChart.cxx:136-143`) silently promotes a
series' `LEAVE_GAP` to `USE_ZERO` for any **area** chart, so 774 blank category cells count as
serial 0. Measured by single-variable probes rather than inferred:

| the same 799 categories, as | axis minimum |
|---|---|
| an **area** chart | `30/12/1899` |
| a line chart | `10/11/2003` |
| a bar chart | `10/11/2003` |
| an area chart with `dispBlanksAs="span"` | `10/11/2003` |

The workbook's own `CHPROPERTIES` states empty mode **0 — skip — which is the gap**, and
LibreOffice's own round-trip of it writes `<c:dispBlanksAs val="gap"/>`. So the model says gap and
the plotter overrides it. Flipping one element from `areaChart` to `lineChart` in the
round-tripped workbook moves the axis from `30/12/99` to `10/11/03`, which is the single-variable
proof.

**The same promotion applies to the Y values, and that is what makes the picture right rather than
merely the axis.** With gaps, our area chart draws a hairline at the right edge of the plot;
with zeros it draws the reference's shape — a run along the baseline and a spike at the end.

### And the points have to be sorted by date

`AreaChart::createShapes` calls `pSeries->doSortByXValues()` for every series on a date category
axis. It is not tidiness: the cells run 17/11/2003, then 774 blanks that now count as 30/12/1899,
then a cluster in 2012, and a polyline through them in cell order goes to 92% of the plot, back to
0%, and out to 100% again. That is exactly what we drew before sorting, and it filled a third of
the plot area with solid colour.

## 2. What changed

| file | change |
|---|---|
| `Core/Charts/ChartDateScale.cs` (new) | `ChartTimeUnit`, `ChartTimeInterval`, `ChartDateAxis`, and the resolver: automatic time resolution, limit snapping, the `/499` interval rule, roll-over `AddMonths`/`AddYears`, tick generation |
| `Core/Charts/ChartPlot.cs` | `DateAxis`, resolved by the reader because none of it depends on geometry |
| `Core/Charts/ChartLayout.cs` | ticks, labels, label measuring and end-label overhang take the date axis where there is one; `CategoryFraction` places a point by its date and answers null for a category with none |
| `Core/Charts/ChartAxisLabels.cs` | an axis label's shape is its text, with **no insets** |
| `Spreadsheets/MsBinary/XlsChartReader.cs` | reads all of `CHDATERANGE` and `CHPROPERTIES`' empty mode; resolves the date axis; zero-fills blanks for an area chart; sorts the points by date |
| `Spreadsheets/MsBinary/XlsWorkbookReader.cs` | passes the workbook's date epoch |

### The label inset is the second half, and it was not in the brief

`ChartAxisLabels` measured every label's collision box as the text plus 0.18 em either side and
0.30 em above and below, citing `ChartLayout`'s constants of the same name. **Those are a data
table's.** `PropertyMapper::getTextLabelMultiPropertyLists` — which is what `VCartesianAxis`
builds every tick label from — sets auto-grow and the two adjusts and **no** `TextLeftDistance` or
`TextUpperDistance` at all, and the SDR default for both is zero
(`svx/source/svdraw/svdattr.cxx:247`). The two places that do set them do so explicitly and for
their own reasons: `DataTableView` at 0.18/0.30 of the font height, and
`getPreparedTextShapePropertyLists` at a flat 250/125 1/100 mm for a shape that may show a border.

It is worth a third of the labels, because two 45°-rotated labels clear each other as soon as
their separation reaches `height × √2`: 0.6 em of invented inset takes an 11.6 pt box to 17.6 pt
and the threshold from 16.4 pt to 24.9 pt. With the insets we labelled every 37th tick and drew
18; without them we label every 19th and draw 36, against the reference's 18 and 38.

## 3. Score

| # | prediction | measured | |
|---|---|---|---|
| 1 | the deficit is real content, not tokenisation | pages 16–18, 552 characters | ✔ |
| 2 | exactly **1 of 171** sheets renderings change; 0 elsewhere | **3 of 171 sheets, 4 of 163 slides, 0 of 200 words** | ✘ |
| 3 | the page count stays 38/38 | 38/38 | ✔ |
| 4 | our page 17 draws **between 30 and 45** labels against 38 | **36** | ✔ |
| 5 | our first two labels read `30/12/99` and `02/mm/yy` | `31/12/99` and `02/03/03` — a day late, see §5 | ✘ |
| 6 | the word gate does not close, best estimate 1520–1590 against 1531 | **1587 against 1531**, band ±34 | ✔ |
| 7 | the series mark moves to 91.8–100% of plot width | it leaves the baseline at **91.5%** | ✔ |
| 8 | `XRay.Office.Fidelity.Tests` stays at 30 failed of 550, the same 30 by name | 30, and the two name lists `diff` clean | ✔ |
| 9 | batches 001–006 stay at 57/60; 007–009 unchanged | **57/60**, the same three; 007–009 **29/29**; 010 9/10 | ✔ |
| 10 | the legend stays wrong and moves with the geometry | 406 ink pixels against the reference's 0, was 932 | ✔ |

Prediction 2 is the interesting miss, and it is the two changes the round grew after the
prediction was written. The census was of BIFF date axes and was right — one document — but the
label inset reaches every chart whose category axis is crowded enough to be thinned, and the
area-chart zero-fill reaches every BIFF area chart with a blank cell. **A reach prediction is only
as good as the change it was made about**, and neither of those two changes existed when it was
made.

Prediction 5's miss is §5 and belongs to a different defect.

## 4. Reach, measured

Every document of all three tracks rendered twice with `SOURCE_DATE_EPOCH` fixed — once from the
prediction commit and once after — and compared byte for byte.

| track | documents | changed |
|---|---:|---:|
| sheets | 171 | **3** |
| slides | 163 | **4** |
| words | 200 | **0** |

That 527 of 534 are byte-identical across two full sweeps is also the control the brief asks for
on unstable documents: our renderer is deterministic under a fixed epoch, so a per-document
difference here is the change and not the document. None of the four sheets known to differ from
themselves — `ans_mappings_of_eccairs_terms`, `PBN Matrix NAAs (V01)`, `fse_identification_form`,
`SIL_TDB648` — is among the seven.

**The seven, and which half of the round moved each:**

| document | cause | words before → after, against the reference |
|---|---|---|
| `sheets/…/Template Pilot Logbook JAR-FCL V3.0.xls` | the date axis | 1327 → **1587** against 1531 |
| `sheets/…/EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls` | the area zero-fill | 8339 → 8339 against 8345, **match either way**; ink only |
| `sheets/…/Keywords_Mapping_Graphs_and_Charts.xlsx` | the label inset | 4511 → 4511 against 4519, **match either way** |
| `slides/…/171128IPAP.pptx` | the label inset | 4688 → 4734 against 4670, **match either way** |
| `slides/…/southern-classic-kennesaw-state-university-final.pptx` | the label inset | 2196 → 2217 against 2270, closer |
| `slides/…/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | the label inset | 2120 → 2118 against 2010 |
| `slides/…/Demick_JetBlue.pptx` | the label inset | 713 → 812 against 608 |

No verdict changes from match to mismatch. Demick moves furthest away in the word column, and its
three moved pages — 4, 5 and 7 — are exactly its three rows in `TODO.raster-ceiling.md`, where the
reference draws a picture and ours draws text; that column cannot be won there in either
direction.

**Three of the four slides are the decks that carry a `c:dateAx`,** and they carry it on a
category axis we still read as a category axis, so the inset change makes a wrong axis denser.
That is the honest cost of not wiring OOXML in this round — see §7.

### The regression sweep

`batch-check.sh` over `sheets/batch-0[01][0-9]` — the whole track, not only the ten batches asked
for, because the reference half is the expensive part and it costs the same:

```
TOTAL 171  MATCH 155  MISMATCH 16  REF-CANNOT-RENDER 0
```

| range | match | mismatch |
|---|---:|---:|
| batches 001–006 | **57** | 3 — `fse_identification_form` and the two Lease Transition twins, all three documented ceilings |
| batches 007–009 | 29 | 0 |
| batch 010 | 9 | 1 — the target |
| batches 011–018 | 60 | 12 — untouched by this round |

Run twice, once mid-round and once on the final tree, and the two verdict columns `diff` clean.

## 5. `SpreadsheetDate` is a day late below serial 61, and it is not a chart defect

Our first tick reads `31/12/99` where the reference reads `30/12/99`. It is not the axis.

Rendered a workbook holding serials 0, 1, 2, 58…62 in date-formatted cells, as `.xlsx` **and** as
`.xls`, through 26.2.4.2 and through our own CLI:

| serial | 0 | 1 | 2 | 59 | 60 | 61 |
|---|---|---|---|---|---|---|
| LibreOffice 26.2.4.2 | 30/12/99 | 31/12/99 | 01/01/00 | 27/02/00 | 28/02/00 | 01/03/00 |
| ours | 31/12/99 | 01/01/00 | 02/01/00 | 28/02/00 | 01/03/00 | 01/03/00 |

**LibreOffice does no phantom-leap-day adjustment at all** — it is a plain `1899-12-30 + serial`,
and its serial 60 is 28 February where Excel's is a 29 February that never existed.
`SpreadsheetDate.FromSerial` adds a day below 61 and carries a comment saying that is what
LibreOffice does; the comment is about `XclRoot::GetDateTimeFromDouble` and is not what the
running binary displays. Note also that our serials 60 and 61 both print 01/03/00, so two distinct
cells collide.

**Not fixed here, deliberately.** It reaches every date cell in the corpus below serial 61 — a
cell holding a literal 0 under a date format is the common case, not a 1900 date — and measuring
that is a sweep of its own with its own regression. `ChartDateScale.Label` therefore passes the
raw serial through, exactly as `FixedNumberFormatter::getFormattedString` is handed it, and
`ChartDateScaleTests.ATickIsWrittenThroughTheAxisFormatAsTheSerialItIs` pins **our** answer with
the reference's beside it, so that fixing the converter surfaces as that test failing rather than
as a silent change to an axis.

A first attempt did correct it locally, by nudging the serial past the rule. It is recorded
because the failure was instructive: the corpus's format is `dd/mm/yy;@`, two sections, so a
nudged serial 0 becomes −1, `SelectFor` takes the **negative** section, and the first label came
out empty. One label of 38 vanished and nothing said so.

## 6. The blind review

A fresh subagent, given only the composed page-17 pair and forbidden to read or run anything,
reported under *what looks identical*:

> Y-axis scale exactly: min 0.0, max 1200.0, major step 200.0 … Same six gridline/tick positions
> … X label rotation angle (~45°), font, and font size look the same; the label count is
> essentially the same (36 vs 36–37) … The dense baseline tick picket: same visual density and
> same run across the full plot width in both … The qualitative shape of the data: flat at zero
> for ~90% of the axis, then a single straight-line ramp at the far right.

and, under differences, found three things independently:

- **the one-day offset** — *"Ours `31/12/99`; reference `30/12/99`"*, with "a one-day epoch offset
  in spreadsheet date serials" as its first candidate cause. That is §5, named blind;
- **the rhythm** — *"ours' label interval is therefore longer in real elapsed time (~3.03 years
  vs. a flat 3)"*, which is rhythm 19 against 18;
- **the legend** — present in ours, absent in the reference.

It also read the series as reaching ~1000 in ours against ~310 in the reference and proposed the
right test for it: *"the height difference is most likely just a consequence of the ramp being cut
at a different point by the plot's right edge"*. Measured: the chart spills onto page 18 in both,
and the wedge's ink height there is **490 px ours against 493 reference**. The peak agrees; only
where the page breaks differs, which is a pre-existing print-area difference and not this round's.

**An earlier reviewer, sent the same page before the zero-fill and the sort, reported "no plotted
data" in our half.** That was true and it was this round's own regression — the date axis had
moved the 25 points into 0.08% of the plot width and drawn them as a hairline. It is the single
most useful thing any measurement did in this round, and no number in the gate could see it.

## 7. Not fixed, with numbers

- **The OOXML `c:dateAx`.** Three decks carry one — `171128IPAP.pptx`,
  `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx`,
  `southern-classic-kennesaw-state-university-final.pptx` — and all three still get a category
  axis. `Core/Charts/ChartDateScale` is reader-agnostic and `ChartPlot.DateAxis` is the only thing
  `XRay.Office.Ooxml` would have to fill; the work is in `axisconverter`-equivalent code, not in
  Core. Left out so that this round's reach stayed measurable on one track.
- **The legend on page 17.** Still 406 non-white pixels in the reference's own reported legend text
  box against the reference's 0. `sheets-chart-01` left `VLegend`'s `ChartLegendExpansion_CUSTOM`
  path as the lead and it is still the lead.
- **The ~22 pt rightward offset of the whole chart.** Pre-existing, seen by two independent blind
  reviewers now, and untouched.
- **Rhythm 19 against 18, and 29/36/3 labels against 30/38/4.** One step of the ladder. The
  reference's own shown-label separation is 16.81 pt and our box clears at 16.4 pt, so we are
  within one tick of it; closing it means measuring a label's height the way `EditEngine` does,
  which is a font-metrics question and not a chart one.
- **The word gate on this document is now a ceiling from the other side.** 1587 against 1531,
  band ±34. Its cause is `pdftotext`'s tokenisation of rotated text: the reference emits **one
  `Tj` per glyph** inside a `Tm`, and we emit one `Tj` for the whole label inside a `cm`, and
  poppler fragments the two differently — about 3 gate-words per label for the reference and 4
  for us. Drawing the two labels we are still missing would take us to about 1603. The document
  belongs in `TODO.raster-ceiling.md` beside `architecture6.ppt` as another instance of the third
  shape, with the sign reversed.

## 8. Tests

Every project run individually. Counts compared against `--list-tests` where the number moved.

| project | passed | failed | skipped | note |
|---|---:|---:|---:|---|
| `XRay.Office.Core.Tests` | **332** | 0 | 0 | 332 discovered; +19 from this round |
| `XRay.Office.Containers.Tests` | 109 | 0 | 0 | |
| `XRay.Office.Markup.Tests` | 259 | 0 | 0 | |
| `XRay.Office.OpenDocument.Tests` | 125 | 0 | 0 | |
| `XRay.Office.Presentations.Tests` | 679 | 0 | 0 | |
| `XRay.Office.Rendering.Tests` | 150 | 0 | 1 | the skip is pre-existing |
| `XRay.Office.Spreadsheets.Tests` | **762** | 0 | 0 | 762 discovered; +4 from this round |
| `XRay.Office.Text.Tests` | 339 | 0 | 0 | |
| `XRay.Office.Vector.Tests` | 295 | 0 | 0 | |
| `XRay.Office.WordProcessing.Tests` | 818 | 0 | 0 | |
| `XRay.Office.Fidelity.Tests` | 520 | **30** | 0 | 550 discovered, 550 run |

The fidelity project was run before any change and after, and the two failure lists `diff` with no
output — 30 names each. The baseline was taken first, as the brief asks, and was 30 of 550.

**One existing test changed and it is the inset.**
`ChartAxisLabelTests.AWordExactlyAsWideAsTheSpacingDoesNotWrap` built ten labels *exactly* the tick
spacing wide and expected them to collide, which they did only because the shape was 0.36 em wider
than the text. Two labels that touch edge to edge do not overlap — `doesOverlap` clips their
polygons — so the old expectation was an artefact of the inset. It now uses a two-word label
wider than the spacing whose individual words are not, which is the state the test was written to
describe.

New cases, 23 in all: `ChartDateScaleTests` (19) and four in `XlsChartDateAxisTests`.

## 9. Scripts

* `mkchart.py` — builds a minimal `.xlsx` holding one chart on a `c:dateAx` or `c:catAx`, with a
  stated category set, chart kind and `c:dispBlanksAs`. Every rule in §1 was established by
  varying one of those and reading the result.
* `labels.py` — reads a chart's category-axis tick labels out of a PDF. Rotated labels arrive from
  `pdftotext` as several fragments per label and in an order that is neither by x nor by y; this
  clusters on whichever of three perpendicular coordinates yields the most strings that parse as a
  date. Reconstructing them by hand was the slowest part of the round.
* `census-date.py` — decodes every OLE2 workbook's chart substreams and reports each
  `CHDATERANGE` whose flags set `DATEAXIS`. One document in 534.
