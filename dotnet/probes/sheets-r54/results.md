# Round 54 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r54`, base
`4b50291b09d`. Read `prediction.md` (`22aa807a4de`) beside this file first; it was committed before
a line of the change was written.

## 1. Baseline reproduced, to the document

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 288 MISMATCH 37`. Scored against
`MANIFEST.tsv`'s 307 sheets paths — the raw total counts 18 case-alias entries twice — that is
**274 match, 33 mismatch**, and the 33 mismatching paths are **exactly** the 33 rows the manifest
marks `open`.

## 2. Result

**sheets 274 → 274 of 307.** One document gained a verdict and one lost one, and both are named
below. **Zero page counts changed**, on either side, anywhere in the corpus.

| document | before | after | ref | verdict |
|---|---:|---:|---:|---|
| `005_Contextures_chart_sample…xlsx` | 293 | **296** | 300 | `words` → **`match`** |
| `013_Contextures_chart_sample…xlsx` | 168 | **165** | 169 | `match` → **`words`** — a regression |

Our column moved on **exactly** those two documents and on no other. Six more moved on the
**reference** side alone — `047_Date_tracker_Gantt_chart` (822 → 848), `033_Event_planning_tracker`
(505 → 503), `PBN Matrix NAAs (V01)` (5542 → 5547), `ans_mappings_of_eccairs_terms` (27896 →
27895), `SIL_TDB648` (7497 → 7500), `FAA-2019-0995-0002_attachment_2` (9994 → 9995) — which is the
date-volatility trap, and every one of the six is a document this round did not touch.

Cross-track, measured rather than reasoned about:

| batch | before | after |
|---|---|---|
| `words/chartset-001` | `TOTAL 10 MATCH 9` | `TOTAL 10 MATCH 10` — `pie-chart-result.docx` 36 → **39** of 40, `words` → **`match`**; `pie-chart-template.docx` 6 → **9** of 9, exact, held |
| `slides/chartset-007` | `TOTAL 10 MATCH 1` | `TOTAL 10 MATCH 1` — `035_Chemistry…` 289 → **293** of 325, still `words`, as predicted |

## 3. The change: LibreOffice's automatic chart title

`ChartSpaceConverter::convertFromModel` (`chartspaceconverter.cxx:177-208`) gives a chart a title
the part never spells out: the **single series' name**, or failing that the localized literal
**`Chart Title`**. Neither was ported; `DrawingChart`'s remarks had said for eleven rounds that
reporting either "would claim the file said something it does not", and that sentence was wrong
twice over — the substitute is on the reference's page, and the reader that draws the picture and
the reader that builds the content tree would have disagreed about the chart's name.

### 3.1 Both arms measured on the corpus, not on authored files

| shape | reference | ours, before |
|---|---:|---:|
| `005_Contextures_chart_sample` — five charts, empty `c:title`, one series named `Sales` | **13** `Sales` | 6 |
| `013_Contextures_chart_sample` — one chart, one series named `East` | **7** `East` | 6 |
| `pie-chart-result.docx` / `pie-chart-template.docx` | **1** `Production in 2017` each | 0 |
| `035_Chemistry_Column_PowerPoint_Chart.pptx` — two charts, two series each | **2** `Chart Title` | 0 |

### 3.2 And both *negative* arms, which are the larger population

157 of the corpus's 307 chart parts draw no title only because `autoTitleDeleted` holds — a
**default** for all but 20 documents, not a stated attribute — and for 82 of them the census can
name the string that would appear if the default went the other way. Three sheets documents where
it can, rendered and counted: `052_Manufacturing_output_chart` (`COMPONENTS COMPLETED`),
`058_Social_media_engagement_data` (`DAILY IMPRESSIONS`) and `001_Contextures_chart_sample` (`Amt`)
all read **ours = reference = one** occurrence, the worksheet cell. And `005`'s own **chart6** —
`autoTitleDeleted val="0"`, no `c:title`, a series with no `c:tx` — has ours = reference = one
`Sales` on its page. A negative control inside the document the change was aimed at.

**All twenty MSO-2007 documents' fourteen chart parts already carry their own title text**, so no
corpus document can separate the two `autoTitleDeleted` defaults in either direction. The flag is
plumbed through anyway, from `OoxmlMetadata.IsOffice2007`, which is exactly
`XmlFilterBase::checkDocumentProperties`' test.

### 3.3 Census and reach

`dotnet/probes/sheets-r54/census-autotitle.py`, over all 946 corpus documents. **307 `c:chartSpace`
parts in 167 zip documents**, and a separate pass that reads every `.xml` part in every corpus zip
and keys on the root element confirms the filename filter misses **0** of them.

| outcome | sheets | slides | words |
|---|---:|---:|---:|
| `own-text` — already drawn | 82 | 28 | 0 |
| `no-title-atd` | 37 | 113 | 7 |
| `no-title-nothing` | 13 | 16 | 1 |
| **`series`** | **6** | 0 | **2** |
| **`literal`** | 0 | **2** | 0 |

Ten parts in **five** documents across all three tracks, and **every one of the ten states
`<c:title>` and an explicit `<c:autoTitleDeleted val="0"/>`** — so no corpus hit depends on the
default at all. A first draft of the census had `doughnutChart` and the two surface types in the
`mbSingleSeriesVis` set, which is wrong (the type table's "1stvis" column is set for `TYPEID_PIE`
and `TYPEID_OFPIE` only); correcting it changed **no** hit, which is worth recording as a
robustness result rather than only as a bug.

### 3.4 Prediction against measurement

| | predicted | measured |
|---|---|---|
| `005_Contextures` | 300 of 300, `words` → `match` | **296** of 300, `words` → **`match`** |
| `013_Contextures` | 169 of 169, `match` holds | **165** of 169, **`match` → `words`** |
| `pie-chart-result.docx` | 39 of 40, `words` → `match` | **39 of 40, `words` → `match`** |
| `pie-chart-template.docx` | 9 of 9, `match` holds | **9 of 9, held** |
| `035_Chemistry…pptx` | 293 of 325, `words` holds | **293 of 325, held** |
| page counts | 0 change | **0 change** |
| documents touched | 5 | **5, and no others** |

Three of five exact. **Both misses are the same miss, and the prediction file named it**: blind
spot 3 said the title's height comes off the top of the diagram area, that the census counts titles
and cannot see whether a shrunk plot moves anything, and that this was "the regression risk and the
one thing the token arithmetic above assumes away". It is exactly what happened — but the direction
was the opposite of the one feared, and that is the interesting half.

## 4. The regression, and why it is the gate rather than the change

**`013` was passing by error cancellation and this round removed the errors on one side only.**

Its four value-axis tick labels on page 4 were **spurious**: the reference draws that chart with
ticks at 800/600/400/200/0 and we drew 800/700/…/100/0. Adding the title took the title's height
off the top of the diagram area, our interval law then chose the reference's step, and **page 4 of
our rendering is now the reference's page 4 — the same title, the same five ticks, the same two
`East`.** That removed four tokens we should never have had.

The four tokens it was cancelling are still missing and are **on page 2**, where the reference
prints the chart's `Jan`/`Feb`/`Mar` legend and a clipped `outh` and we print nothing at all
(§ 6.1). 168/169 was `d = 1`; 165/169 is `d = 4`, and the gate fails at `d > 3` with a 2% band of
3.38. The document is measurably closer to the reference and one token the wrong side of a
threshold.

`005` moved the other way for the same reason: our column chart's axis went from the reference's
`0…450` in fifties to `0…500` in hundreds, losing four tokens, while the five titles gained seven.
Its plot rectangle now agrees with the reference's to **1.2 pt** where the tick *step* does not, so
what the change exposed there is a pre-existing weakness in `ChartLayout.IntervalsThatFit`:
`available / needed` with `needed` the label's line height gives 8 where 26.2.4.2's
`estimateMaximumAutoMainIncrementCount` gives at least 9 on the same geometry. Page 2 of `005` is
now **identical** to the reference's.

**A note on the title's drawn position, so it is not read as this round's doing.** Our chart title
is drawn **9.8 pt above** the reference's, uniformly, on every one of the six charts measured.
It is pre-existing: `Keywords_Mapping_Graphs_and_Charts.xlsx`, which states its own titles and has
never been through this code path, shows the same 9.78 pt on a document that matches. 3.83 pt of it
is explained — `lcl_createTitle` adds `nYDistance = 0.02 x pageHeight + 135` (1/100 mm) *before*
the half-height, and `ChartLayout.AddTitles` starts its pen at the 2% alone — and the remaining
6 pt is not. `DiagramAreaOf` already reserves the 135, which is why the plot rectangles agree; only
the glyph is misplaced. Not touched, because a hunch about 6 pt is exactly what this project's
rules say not to ship.

## 5. Tests

Eighteen in `DrawingChartAutomaticTitleTests`, all driven through the public readers rather than
the internal helper. **Fifteen distinct mutations through `verify-test.sh`, every one detected, and
every one of the eighteen tests is a detector — there is not a drift guard in the class.**

| mutation | detected by |
|---|---|
| the plot reader stops falling back | eight of them at once |
| the content tree stops falling back | `TheContentTreeAndTheDrawingAgreeOnTheSubstitutedTitle` |
| the `autoTitleDeleted` default flipped to not-deleted | `NoTitleElementAndNoFlagDrawsNothing` |
| the disjunction read as `autoTitleDeleted` alone | `ADeletedAutomaticTitleIsStillDrawnWhenATitleElementIsStated` |
| the disjunction read as the title element alone | `ANotDeletedAutomaticTitleNeedsNoTitleElement` |
| doughnut treated as single-series-visible | `ADoughnutWithSeveralSeriesHasNoSingleSeriesName` |
| the pie single-series-visible arm dropped | `APieChartTakesItsFirstSeriesNameEvenWithSeveralSeries` |
| the second-axes-set clear dropped | `TwoAxesSetsClearTheAutomaticTitleAltogether` |
| the one-type-group test dropped | `TwoTypeGroupsOnOneAxisPairHaveNoSingleSeriesName` |
| empty type groups counted as type groups | `AnEmptyTypeGroupDoesNotCountAsATypeGroup` |
| the `c:spPr` clause dropped from `bShowEmptyTitle` | `AnEmptyTextBodyWithNoShapePropertiesStillTakesTheLiteral` |
| `isSingleSeriesTitle` forced false | `AFormattedButEmptyTitleOverASingleSeriesIsLeftEmpty` |
| the empty-rich-text escape dropped | `AnEmptyRichTextBodySuppressesTheLiteral` |
| the automatic title made to win over the model's own text | `ATitleThatStatesItsOwnTextIsUnaffected` |
| the bare `c:v` series name no longer read | `ASeriesNamedByALiteralValueIsNamedTheSameWay` |
| the no-title-element bail after an empty automatic title dropped | `AnUnnamedSeriesWithNoTitleElementDrawsNothing` |

`TheContentTreeAndTheDrawingAgreeOnTheSubstitutedTitle` is the round-53 lesson applied: it is a
shape control *and* a detector, and it is what stops one of the two readers being taught the rule
and the other not.

**One existing test asserted the superseded decision and had to be rewritten**, not deleted:
`DrawingChartTests.ATitleFallsBackToItsLinkedCellAndThenToNothing`'s second case is precisely
`getSingleSeriesTitle`'s shape and its comment said no title is invented. It now asserts the
substitution, and a **third** case was added for the arm that really does draw nothing — no title
element and no flag — so the test keeps the "and then to nothing" it is named for.

## 6. What was measured and left, with the mechanism attached

### 6.1 `013`'s remaining four tokens: a camera-tool picture drawn 100 pt too narrow

Its page 1 is not a chart part at all. Sheet `ChartDisplay` holds one `xdr:twoCellAnchor
editAs="oneCell"` wrapping an `mc:AlternateContent/mc:Choice Requires="a14"` **camera-tool picture**
(`a14:cameraTool cellRange="ShowChart"`) whose blip is an EMF of the chart on the other sheet. Its
`a:ext` is 326.25 x 221.25 pt and the legacy `vmlDrawing1.vml` shape states the same size to the
tenth.

**We draw it at that stated size. 26.2.4.2 draws it about 100 pt wider**, which is what puts the
reference's legend and its last category label across the page-1/page-2 split and ours inside
page 1. Four authored variants of the corpus file, one thing varied at a time:

| variant | reference | ours |
|---|---|---|
| the corpus file | `1000` at 133.8, `Jan` at 534.3 | 129.5, **414.5** |
| `editAs="oneCell"` removed | **unchanged** | **133.8, 534.1 — the reference, to 0.2 pt** |
| `a:ext/@cx` halved | **unchanged** | 124.2, 266.7 |
| the anchor's `to` column 6 → 9 | **unchanged** | unchanged |

So the reference's width answers to **neither** `editAs`, `a:ext`, nor the anchor's second corner —
and the one change on our side that reproduces it is honouring the two-cell span. The likely reason
is that Calc resolves the camera tool's `cellRange` (`ShowChart` → `INDIRECT(SelRange)` →
`ChartInfo!$B$7:$H$20`) and sizes the picture from **that range's own columns**, which measure
about 426 pt — but the `to`-column invariance is not explained by that either, and it is recorded
as unexplained rather than tidied away.

**Do not simply stop reading `editAs`.** The current behaviour was measured: `SIL_TDB648.xlsx`
anchors its cover photograph `editAs="oneCell"` over rows whose heights are recomputed shorter on
load, and the stated extent gives 300.75 pt against the reference's 300.73 where the anchor gives
286.6. A change here owes a census of `editAs` across the corpus and a full sweep.

And two hypotheses **refuted** on the way there, each by an authored probe:

- **It is not the default column width.** Four authored workbooks — Calibri 11 and Liberation Sans
  10, with and without a stated `defaultColWidth` — give **48.13 pt on both sides in every case**,
  and markers planted in columns A to J of `013`'s own `ChartInfo` sheet come out at **64.94 pt
  pitch on both sides, agreeing to 0.0005 pt**. The grid is right; the picture on it is not.
- **It is not a font-face divergence**, which the label widths first suggested: the same markers
  agreeing to five decimal places settle it.

### 6.2 The four `_advanced_excel_pie` documents, still 5 tokens each

Untouched, and re-costed rather than re-derived: `135/140`, `135/140`, `135/140`, `138/143`. The
gate needs **two** of the five back (`d = 3` passes), so this is the largest cluster left on the
track — four documents for one mechanism, which round 51 measured as the reference placing the `M1`
data label outside the pie so that it falls wholly onto page 2 while ours stays inside the pie and
is cut by the split. That is pie label placement (`csscd::AVOID_OVERLAP` in the type table), not
clipping, and it is a `ChartLayout` change.

## 7. The 24.2.7.2 audit — `SheetOptimalRowHeights.cs`, and it is still correct

`dotnet/probes/sheets-r54/audit_rowheight.py`, an authored probe against the installed 26.2.4.2.
The site claimed thirty exact reproductions of a wrapped row's optimal height fitted to a
**24.2.7.2** flat-ODF round trip. Re-run on a freshly authored thirty of the same shape — six sizes
(8, 10, 11, 12, 14, 18 pt) against one to five words that cannot share a line, no `ht` and no
`customHeight` anywhere so Calc recomputes — **30 of 30 agree to under half a twip**, largest
disagreement 0.05.

**The control ran first and passed**, which is round 53's lesson kept: the twelve-point single-line
row reads **300** twips, the figure the site already states for `National-Reports.xlsx`. And the
instrument is two independent readings of the same number — the y of an identical six-point marker
in the next row less this row's, off both PDFs, and the reference's own `--convert-to fods`
`style:row-height` — which agree throughout to 0.6 twips. Twenty-four of the thirty rows are
genuinely multi-line and so exercise `WrappedHeight` rather than the arithmetic path.

Marked at the site. `XRay.Office.Spreadsheets` now has **five of nine** re-checked and **all five
still correct**; the project-wide score is two wrong in ten and both wrong ones were shared-layer.

## 8. Shared layer

**Yes.** `XRay.Office.Ooxml/DrawingML/DrawingChartTitle.cs` (new), `DrawingChart.cs` and
`DrawingChartPlot.cs`, plus the three call sites that now pass the Office-2007 flag
(`XlsxCharts.cs`, `PptxShapeReader.cs`, `DocxContentReader.cs`). All three tracks read it.

The census names the reach exactly: **five documents**, two sheets, two words, one slides. I have
measured the two affected non-sheets batches myself and the figures are in § 2; **the parent should
still run the cross-track sweep**, and the prediction for it is `words/chartset-001` +1 and
everything else unchanged.

## 9. Build and tests

`dotnet build -v q -nologo` → **0 warnings, 0 errors.**

Ten non-Fidelity projects, run one at a time and totalled by hand: **4713 passed, 0 failed, 1
skipped**, against the base's 4695/0/1 — a delta of exactly the **18** new tests, all in
`XRay.Office.Presentations` (788 → 806). `Fidelity` is **521 passed / 31 failed / 552**, byte-for-byte
the base's figure.

Whole track swept before and after; `sheets/done-*` is inside both.

## 10. `MANIFEST.tsv`

Lives in the corpus repository and was not touched. Two rows change status and they cancel:

| path | from | to |
|---|---|---|
| `sheets/chartset-006/xlsx/005_Contextures_chart_sample_6e279b08.xlsx` | `open` | `done` |
| `sheets/chartset-012/xlsx/013_Contextures_chart_sample_21b98e22.xlsx` | `done` | `open` |

And on the words track, `words/chartset-001/docx/pie-chart-result.docx` `open` → `done`, for the
parent to confirm with its own sweep.

## 10a. The vision pass, and two new defects no gate can see

Two paired images, both on a page **chosen for a stated reason** rather than by `--worst`: page 1
of `005_Contextures` (where three of the five new titles land) and page 4 of `013_Contextures` (the
page whose tick set changed). Each went to a fresh subagent with no access to this project's
documents, source or shell, asked to describe each half on its own terms before comparing.

**Both reviewers, on unrelated documents and unrelated pages, named the same object: the reference
draws a thin light-grey rectangle around every chart and we draw none.** That is the discriminator
`HANDOVER.md` § 7 sets out — same object, pages chosen for a reason, and a *different instrument*
to confirm — and the third instrument agrees: decompressing both PDFs' content streams,
`005_Contextures`'s reference sets a light grey (0.8–0.95) stroke colour **8 times** and issues
**387** stroke operators; ours sets it **0** times and issues **155**. Six charts, one frame each.
Ink only; no token can move for it.

**And a second, from the `005` reviewer alone but measured independently:** its pie data labels are
white in the reference and black in ours. `chart1.xml` states
`c:dLbls/c:txPr/…/a:defRPr/a:solidFill/a:schemeClr val="bg1"`, and the content streams say the
reference emits **33** white fills on that document against our **0**. We are not honouring a data
label group's stated fill colour.

**One reading did not survive its own check, which is why it is written down.** The `005` reviewer
reported the reference's two right-hand charts sitting "6–8% of page width further right" than
ours. The bounding boxes say otherwise: the bar chart's title is at x = 509.32 in ours and 509.52 in
the reference, and the pie's at 176.29 against 176.38. The composition scales each half to 78% of
the composed image, and the apparent shift is the instrument. Two of the reviewers' three
observations are real and one is an artefact — which is the same ratio § 7 records, and the reason
its rule is "confirm with a different instrument", not "count readers".

The `013` reviewer independently found the **9.8 pt** title offset of § 4, describing our title as
"about 15 px higher" at 150 dpi — 7.2 pt, the same thing seen through a coarser ruler — on a page
where nothing else about the chart differs.

## 11. What the next round should do first

1. **`013`'s camera-tool picture (§ 6.1).** It is four tokens on a document that is otherwise
   exact, it has four measured variants bracketing it, and the one change that reproduces the
   reference is named. It owes a census of `editAs="oneCell"` and a full sweep, and it must not
   break `SIL_TDB648`.
2. **`ChartLayout.IntervalsThatFit` (§ 4).** `005`'s plot rectangle now agrees with the reference
   to 1.2 pt and its tick *step* does not; `available / needed` gives 8 where
   `estimateMaximumAutoMainIncrementCount` gives 9 on the same geometry. Four tokens on `005`, and
   it is a law that reaches every column chart on the track — so it wants a census before a line.
3. **The four `_advanced_excel_pie` documents (§ 6.2)** — the largest cluster left, and the gate
   needs only two of their five tokens.
4. **`SheetPageDecoration.cs`'s 24.2.7.2 site.** `SheetOptimalRowHeights` came back clean; page
   furniture is what the track's remaining page-count outliers hang off and it has no probe harness
   yet.
5. **The chart area's light-grey border (§ 10a)** — every chart in the corpus, two blind reviewers
   and a stroke count agreeing, and nothing on the gate can see it. Ink, not tokens.
6. **A data label group's stated fill colour (§ 10a)** — `005`'s pie labels are white in the
   reference and black in ours, and the part says `bg1`.
7. The chart title's **9.8 pt** vertical offset (§ 4), of which 3.83 pt is explained and 6 pt is
   not. It is worth no tokens and it is on every chart title in the corpus.

Still unworked from earlier rounds' blind readings: `068_Blue_inventory_list`'s two undrawn arrow
autoshapes and grey-for-teal title; `017_Timeline_Templates`' missing navy spine, five year badges
and every leader line; `065`'s literal `aaaa` where the reference draws `Thursday`;
`070_Equipment_inventory_list`'s advisory shape wrapping at different points; and the
eight-blank-line header on `FAA-2019-0995-0002`, which twelve probes in `probes/sheets-r51-bands/`
bracket and none explains.
