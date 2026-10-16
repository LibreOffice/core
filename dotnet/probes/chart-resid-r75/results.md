# Three chart residuals: a polyline that is not clipped, a hidden column that is, and a wrap restart that does not exist

Round `agent/chartresid`, 2026-09-07, branch `agent/chartresid` from `0e54dba0a`.

## Environment

- Reference: **`/opt/libreoffice26.2/program/soffice`, LibreOffice 26.2.4.2**
  (`0229ac93fcf0d7cbc6376066c6f35021cef002dc`). Not `/usr/bin/soffice`, which is 24.2.7.2.
- The tarball's bundled duplicates are aside: `share/fonts/truetype/.duplicates-aside` holds 38
  faces (`LiberationSansNarrow` among them) and `.noto-aside` 8. `fc-match "DejaVu Sans"` answers
  `DejaVuSans.ttf` and `fc-match Calibri` answers `Carlito-Regular.ttf`.
- Corpora: `/home/user/sample-files` (947, the OOXML/BIFF original) and `/home/user/corpus-odf`
  (1285, 26.2.4.2's own `--convert-to`).
- Every rendering on both sides was taken with `SOURCE_DATE_EPOCH=1700000000`, at 2 workers,
  one output directory per *document* keyed on an MD5 of its corpus-relative path.
- LibreOffice profiles are keyed on a hex digest of the document path, never on its name.

## What was measured, and on which columns

| | column | documents |
|---|---|---|
| the polyline clip | `.pptx`/`.xlsx`/`.xls`/`.docx` — the 176-document chart census (`probes/chart-fit/census.tsv`) | 176 |
| the same, ODF | `.ods`/`.odp`/`.odt` of `/home/user/corpus-odf`, censused here | 182 |
| the hidden-cell rule | the same 176 | 176 |

`census-odf.py` is the ODF census: **182 of the converted corpus embed a chart** — sheets 99,
slides 73, words 10 — of which 53 hold a `chart:class="chart:line"` or `"chart:scatter"` part.
That is the same order as the OOXML figure of 176 and is the first time this project has counted
charts on the ODF side.

---

## 1. A polyline that leaves the plot — the reference clips its geometry, and we drew it whole

**The brief's first branch is the right one, and it is now closed.**

### The rule

Every chart2 plotter runs its series polygon through `Clipping::clipPolygonAtRectangle` against
`PlottingPositionHelper::getScaledLogicClipDoubleRect` *before* it makes a shape of it, so what
reaches the page is already cut:

- `chart2/source/view/charttypes/AreaChart.cxx`:318 (stepped), 336 and 343 (splines), **359**
  (the straight-line default), 445 (the filled area, with `bSplitPiecesToDifferentPolygons`
  **false**);
- `NetChart.cxx`:138, 144, 213; `BarChart.cxx`:533; `VSeriesPlotter.cxx`:1423 (a regression curve).

The rectangle is the axis' own minimum and maximum put through the scaling function —
`getScaledLogicClipDoubleRect`, `chart2/source/view/main/PlottingPositionHelper.cxx`:295-311 —
and the scene transform maps it exactly onto the plot rectangle, so clipping in page space
against the plot rectangle is the same cut.

The algorithm is Liang–Barsky per segment (`Clipping.cxx`:47-71 `lcl_CLIPt`, :86-128
`lcl_clip2d`) with chart2's own piece joining on top (:340-421): consecutive surviving segments
are collected into one run and a run that had to skip a segment starts a new one, because
`bSplitPiecesToDifferentPolygons` defaults to **true** (`chart2/source/view/inc/Clipping.hxx`:51)
and only the two filled-polygon callers pass `false`. **A line that leaves the plot and comes
back is two strokes with no chord across the gap.** The bounding-box short circuit at :350-365 —
wholly inside is returned untouched, wholly outside is rejected — is what makes the whole thing
free for a chart that already fitted.

Beside it, `AreaChart::createShapes` adds every point to the series polygon and *then* asks
`isLogicVisible` (`AreaChart.cxx`:715), and `if( !bIsVisible ) continue;` (**:760-761**) sits
above the symbol, the error bars **and** the data label. So an out-of-range point holds its place
in the line and gets no mark of its own. `isLogicVisible` is
`PlottingPositionHelper.hxx`:333-339, inclusive at both ends except for a shifted category axis,
which is a bar chart's position helper and not a line's.

### The witness, and it is a line chart

`slides/done-011/pptx/171128IPAP.pptx` slide 38 → `ppt/charts/chart7.xml`: **`c:lineChart`**,
three series of 132 cached points each, a `c:dateAx` stated `c:min 40179` / `c:max 43831` over
categories beginning in 2006, and a value axis `−20 … 30`. Confirmed from the part, not inferred.
Both sides' axis and category labels come back from `get_text` as real text, so nothing here is
the reference outlining a turned run.

Read out of the PDFs' path operators, page 38, the three series polylines:

| | x range | y range | line items |
|---|---|---|---:|
| 26.2.4.2 | **119.54 … 615.49** | 270.03 … 428.14 | 1602 |
| before | **−866.50 … 758.03** | 89.69 … 570.97 | 262 |
| after | **119.55 … 615.56** | 273.87 … 427.72 | 82 |

119.54 … 615.49 is the reference's own plot background rectangle to the hundredth of a point. The
whole deck: **three of our forty pages carried vector ink outside the page and none of the
reference's did**; after, none does.

*The 1602-against-262 item count is a different, open difference and not this one*: all three
series state `c:smooth val="1"`, so 26.2.4.2 draws cubic splines through the 132 points
(`AreaChart::impl_createLine`'s `CurveStyle_CUBIC_SPLINES` branch) and we draw straight segments.
Left.

### What moved

`ChartClipping` (new, `XRay.Office.Core/Charts`) is the port; `ChartLayout.AddLines` collects each
series into runs broken at its gaps, clips each run, and emits one subpath per surviving piece.
A point `ChartClipping.Contains` rejects is kept out of the list markers and data labels are made
from.

Base binary against this one, byte for byte:

| census | unchanged | moved |
|---|---:|---:|
| 176 OOXML chart documents | **174** | 2 |
| 182 ODF chart documents | **180** | 2 |

and the movers are the **same two documents in both columns**. Against 26.2.4.2:

| | pages ref/before/after | alnum ref/before/after | mean ink before → after |
|---|---|---|---|
| `171128IPAP.pptx` | 40 / 40 / 40 | 25432 / 25454 / 25454 | 5.1858 → **5.1384** |
| `RPA P4 - Advanced Material.pptx` | 20 / 20 / 20 | 8743 / 8748 / 8748 | 3.3062 → **3.2994** |
| `171128IPAP.odp` | 40 / 40 / 40 | 25432 / 25438 / 25438 | 8.3371 → **8.3231** |
| `RPA P4 - Advanced Material.odp` | 20 / 20 / 20 | 8743 / 8678 / 8678 | 5.8003 → **5.7902** |

**No page count and no alphanumeric count moves anywhere**, so no gate row can move in either
direction; every page that moved improved (`171128IPAP.pptx` pages 36, 37, 38 and 40 by −0.28,
−0.25, **−1.26** and −0.11).

`outside.py` is the coarse witness and is worth keeping: of the 176, **78 drew vector ink more
than a point outside their own page before and 76 after**, and the two that stopped are exactly
the movers. The other 76 are not this defect — they are whole charts anchored beyond the printed
page (`064_Small_business_cash_flow` page 6 draws a chart from x = −866.86, `DynamicBubbleChart`
one from x = 321 … 917 on a 612 pt page), which is an anchoring question and not a clipping one.

### What is left here

The **area**, **net/radar** and **regression** clips are not done. The reference clips all three
(`AreaChart.cxx`:445 and `NetChart.cxx`:213 with splitting **off**, because a filled polygon is
closed rather than broken), and the corpus produced no witness for them once the polyline was
cut. The splined line is left as well.

---

## 2. A chart plotting a pivot category its cache does not hold — the rule is hidden columns, and the brief's mechanism is refuted

### What the brief and the record said, and what is actually true

`probes/chart-label-anchor/results.md` §3 concluded that `053_Personal_asset_inventory` draws six
categories rather than seven because *"`053` contains an Excel table whose `displayName` is
`Assets`, which is also the name of the sheet its chart references … and `Assets!$H$24:$H$30`
then resolves to nothing"*, and that *"there is no rule that matches this reference on both
files, because the reference is not following one."*

**Both halves are wrong.** `pivot-variants.py` builds twelve one-attribute variants of that
workbook; each was rendered through 26.2.4.2 and read out of the PDF.

| variant | what it changes | 26.2.4.2 draws |
|---|---|---|
| `v0-control` | repackaged, nothing else | 6 categories, axis to $300,000 |
| `v1-table-renamed` | the table's `name`/`displayName` `Assets` → `Tbl9` | **6, unchanged** |
| `v2-no-table` | the `table` part, its relationship and `tableParts` removed | **6, unchanged** |
| `v5-no-pivot` | the `pivotTable` part removed | **6, unchanged** |
| `v3-cache-has-seven` | a seventh `c:pt` added to the chart's own caches | 7, `Grand Total`, axis to $400,000 |
| `v6-chart-on-table` | `c:f` retargeted to the table's own visible cells, cache untouched | the **table's** order (401k, Bonds, Car, House, Savings, Stocks) |
| **`v7-columns-shown`** | `hidden="1"` removed from `<col>` 8 and 9 | **7, `Grand Total`, axis to $400,000** |
| `v8-value-column-shown` | only column I unhidden | 6, axis to $300,000 |
| `v9-category-column-shown` | only column H unhidden | 7 categories, **no values at all** |
| `v10-no-cache` | the chart's `c:pt` stripped, formulas kept | **nothing** — one `Series1`, axis $0–$12 |
| `v11` = v8 + v10 | value column shown, cache stripped | **nothing** |

So: the table name decides nothing (v1, v2); the pivot part decides nothing (v5); **the chart
reads the sheet** (v6 changes what is drawn without touching the cache); and **the discriminator
is that columns H and I are `hidden="1"`** (v7). v0-against-v10 is what says the six drawn points
are the *cache*: the same file with its `c:pt` removed draws nothing.

### The rule, at the seat

`ScChart2DataSequence::BuildDataCache` asks `ColHidden` and `RowHidden` per cell and `continue`s
past a hidden one — the cell is **dropped from the sequence, not blanked** —
`sc/source/ui/unoobj/chart2uno.cxx`:**2636-2646**:

```cpp
bool bColHidden = m_pDocument->ColHidden(nCol, nTab, nullptr, &nLastCol);
bool bRowHidden = m_pDocument->RowHidden(nRow, nTab, nullptr, &nLastRow);
if (bColHidden || bRowHidden) { aHiddenValues.push_back(nDataCount-1);
                                if( !m_bIncludeHiddenCells ) continue; }
```

`m_bIncludeHiddenCells` reaches the sequence from the data provider (`:2101`, `:2248`), which
`ChartModel::attachDataProvider` sets from the diagram (`chart2/source/model/main/ChartModel.cxx`
:855-866, and `setIncludeHiddenCells` at :1473-1497 keeps the two in step). The OOXML importer
sets the diagram's property to `!c:plotVisOnly`
(`oox/source/drawingml/chart/chartspaceconverter.cxx`:**264**), and `plotVisOnly` itself defaults
to `!bMSO2007Document` — `rAttribs.getBool(XML_val, !bMSO2007Document)`,
`chartspacefragment.cxx`:130-131, with the same fallback in `chartspacemodel.cxx`:30.

**So the default is that a hidden cell is not chart data**, and it is the Office generation and
not a constant that decides it.

*`053`'s ranges are `Assets!$H$24:$H$30` and `Assets!$I$24:$I$30`, and `<col min="8" max="8" …
hidden="1"/>` and `<col min="9" max="9" … hidden="1"/>` hide every cell of both.* The pivot's
output really is in the sheet — `H30` is the shared string `Grand Total` and `I30` is `363500` —
and the reference simply does not read it.

### `plotVisOnly` was recorded as refuted, and that recording is too strong

`dotnet/TODO.batches.md`:15115 says *"`plotVisOnly` is refuted; a chart range stops before an
Excel table's totals row"*. That is right **about `029_Annual_budget`**, whose sheet carries no
`hidden` attribute anywhere — and it was generalised into a statement about the attribute. The
attribute is exactly what decides `053`, `026` and `027`. Both rules are real, they live three
lines apart in the same loop, and a round reading only the headline would have skipped this.

### The two rules answer "nothing survived" differently

A range wholly inside a totals row resolves to an **empty** sequence and the chart draws nothing
(`029`, measured by round 53). A range wholly hidden leaves the **cache** standing (`053`,
measured here as v0 against v10). The C++ produces an empty `m_xDataArray` in both cases, so this
asymmetry is an observation about what the two documents render as and not something this round
can derive; `XlsxChartRanges` implements it as measured and says so at the seat.

### Reach, and the control

`census-hidden.py` over all 947: **28 sequences in 4 documents** name a hidden cell, and in every
one of the four **every** cell of the sequence is hidden.

| document | sequences | `c:plotVisOnly` |
|---|---:|---|
| `sheets/chartset-009/xlsx/053_Personal_asset_inventory…` | 3 | on (absent) |
| `sheets/chartset-012/xlsx/026_Monthly_cash_flow_statement…` | 12 | on (absent) |
| `sheets/chartset-014/xlsx/027_Simple_personal_cash_flow_statement…` | 12 | on (absent) |
| `sheets/chartset-008/xlsx/055_Project_timeline_with_milestones…` | 1 | **`val="0"`** |

`055` is the control: seventeen hidden cells that *are* chart data, and its rendering must not
move.

### What moved

`XlsxChartHiddenCells` reads a sheet's hidden rows and columns; `XlsxChartRanges.Resolve` gains
`plotVisibleOnly` and skips them, answering null rather than an empty sequence when that leaves
nothing; `XlsxChartRanges.Resolver(bool)` binds the flag per chart;
`DrawingChart.PlotsVisibleCellsOnly` reads `c:plotVisOnly` with the 2007 default; `XlsxDrawings`
and `XlsxCharts` take the workbook's resolver rather than one delegate and bind it once per chart
part. The BIFF reader has had this rule since round 62 (`XlsChartSource.IsHidden`) and the OOXML
one did not.

Over the 176 chart documents, the item-1 binary against this one: **173 unchanged, 3 moved**, and
the three are the three predicted. `055` is byte-identical.

| | pages ref/before/after | alnum ref/before/after | mean ink | worst page |
|---|---|---|---|---|
| `053_Personal_asset_inventory` | 2 / 4 / 4 | 188 / 260 / **251** | 3.0970 → **2.6957** | 5.4226 → 4.6837 |
| `026_Monthly_cash_flow_statement` | 11 / 11 / 11 | 7852 / 7935 / **7864** | 1.0218 → **0.7569** | 6.5183 → **3.6043** |
| `027_Simple_personal_cash_flow_statement` | 10 / 10 / 10 | 7949 / 7994 / **7932** | 1.7501 → **1.6412** | 5.2679 → 4.9489 |
| `055_Project_timeline_with_milestones` (control) | 2 / 2 / 2 | 1043 / 958 / 958 | 1.9003 → 1.9003 | unchanged |

No verdict moves: `026` and `027` were and remain inside the gate's `max(2%, 15)` band (their
alphanumeric distance goes 83 → 12 and 45 → 17), and `053` is a page-count mismatch before and
after. `053`'s 4-against-2 pagination is a **separate** defect and it is now pinned: our
rendering of the file as it stands is 26.2.4.2's rendering of `v7-columns-shown` — same four
pages, same page-2 layout, `Grand Total` on the chart — while the reference's own `v0` has two.
So the residual is that we paginate the hidden columns' block onto pages 3 and 4; the sheet's own
printing already honours the flag (our `v0` and `v7` differ on which pages carry `Row Labels`).
Left.

---

## 3. The chart label wrap restart — the premise does not hold in either implementation

**Not established, and the brief's description does not match the code on either side.** No fix
and no witness; recorded so the next round does not re-derive it.

The claim was that *an axis whose labels wrap does not restart the wrap when the layout is
retried, so the wrapped width from an earlier attempt survives into the final one*. At the seat:

- The wrap width is `nLimitedSpaceForText`, taken from `nScreenDistanceBetweenTicks`
  (`VCartesianAxis.cxx`:749). That value is computed **once**, in `createLabels` at
  :1733, *before* the retry loop, and passed unchanged into every attempt of
  `while (!createTextShapes(…)) {}` at :1753-1755. **The reference deliberately does not widen
  the wrap when the rhythm rises**, so there is nothing to restart there.
- The one restart that does happen is the line-break one: the first label after the first that
  breaks *inside a word* sets `m_bLineBreakAllowed = false`, calls `removeTextShapesFromTicks()`
  and returns false (:899-905). Every shape is dropped, and on re-entry `isBreakOfLabelsAllowed`
  is false so `nLimitedSpaceForText` stays −1 and no label is laid out wrapped at all.
- What is carried forward across a restart is carried forward on purpose: the loop passes the
  *same* `AxisLabelProperties` object, so `m_nRhythm`, `m_bLineBreakAllowed`,
  `m_fRotationAngleDegree` and `m_eStaggering` all accumulate.
- `ChartAxisLabels.Resolve` does the same: `spacing` is computed once outside the loop, `limit`
  is a function of `spacing` alone, and `Wrap(texts, …)` is recomputed from the **original**
  `texts` on every attempt. No wrapped string and no wrapped width crosses an iteration.

The one thing in this area that *is* worth a witness, and which this round did not get one for:
`ChartLayout.Place` settles `arranged` in two passes and then lets `IntervalsThatFit` recompute
`scale` and `area` a third time **without re-arranging**, so the final rectangle can differ from
the one the arrangement was measured against. Whether 26.2.4.2 re-runs `createLabels` after that
correction is decided by `impl_createDiagramAndContent`'s own pass structure and was not
measured here.

---

## What this round's brief got wrong

1. **Item 3's stated mechanism is refuted.** The `Assets` table-name collision does not decide
   anything: renaming the table (`v1`) and deleting it outright (`v2`) both leave 26.2.4.2's
   rendering unchanged. Nor is it true that *"there is no rule that matches this reference on
   both files"* — there is, it is `c:plotVisOnly` over hidden rows and columns, and it also
   explains `Keywords_Mapping_Graphs_and_Charts.xlsx` drawing its third point, whose sheet hides
   nothing.
2. **Item 3's framing — "present in the pivot definition but absent from its cached records" —
   is the wrong axis.** The pivot is a coincidence: three of the four documents the rule reaches
   have nothing to do with pivots, and removing `053`'s pivot part changes nothing.
3. **Item 1's premise is not reproducible at either seat.** Nothing carries a wrapped width from
   one attempt into the next, in chart2 or in `ChartAxisLabels`.
4. **Item 2's second branch is refuted and its first confirmed.** We did not clip by the bounding
   box; we did not clip at all. The reference clips the geometry, against the axis' own range in
   scaled logic space, splitting the result into separate strokes.
5. **A standing note in `TODO.batches.md` is too strong.** *"`plotVisOnly` is refuted"* is a
   claim about `029_Annual_budget` and not about the attribute.

## Files

| | |
|---|---|
| `sweep.py` | render a census list with one CLI, one directory per document |
| `score.py` | pages, alphanumeric characters and per-page ink of before/after against a reference |
| `outside.py` | how far a bank's renderings draw outside their own page |
| `census-odf.py` | which converted-ODF files embed a chart, and which hold a line or scatter part |
| `census-hidden.py` | which corpus charts name a range whose rows or columns the sheet hides |
| `pivot-variants.py` | the twelve one-attribute variants of `053_Personal_asset_inventory` |
| `odf-charts.tsv`, `hidden.tsv` | the two censuses |
| `sweep-*.log`, `score-hidden.txt` | the runs |

## Reproducing

```sh
export PATH=/opt/libreoffice26.2/program:$PATH        # 26.2.4.2, never /usr/bin/soffice
export SOURCE_DATE_EPOCH=1700000000
CLI=<worktree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli

python3 sweep.py "$CLI" ../chart-fit/census.tsv after 2                   # OOXML charts
CORPUS=/home/user/corpus-odf python3 sweep.py "$CLI" odf-charts.tsv odf-after 2
python3 census-hidden.py > hidden.tsv                                     # 4 documents
python3 pivot-variants.py pivot && soffice --headless --convert-to pdf --outdir pivot-ref pivot/*.xlsx
python3 score.py <ref.pdf> <before.pdf> <after.pdf>
python3 outside.py <bank>
```

`sweep.py` keys its output directory on an MD5 of the document's corpus-relative path, one per
*document* rather than per worker slot, which is the trap `dotnet/CLAUDE.md` records.

## Provenance

Measured 2026-09-07 in the `/home/user` container, against `/opt/libreoffice26.2` 26.2.4.2 with
the tarball's Latin duplicates and its `LiberationSansNarrow` moved aside, `fonts-dejavu-core`
present. `PROVENANCE.tsv` rows to add by hand — do not regenerate the index in this container.
