# r101 — chart-type coverage audit: LibreOffice against this tree

A survey, not a fix round. **No code was changed**, so the full suite was not run; see
§9.

Four passes, kept separate and run in this order so that the C# reading could not
contaminate the LibreOffice enumeration:

1. LibreOffice's chart types, from `chart2/source/model/template/ChartTypeManager.cxx`
   and `chart2/source/inc/servicenames_charttypes.hxx`.
2. Each stream's spelling — OOXML `c:`/`cx:`, BIFF `CHTYPEGROUP` children, ODF
   `chart:class`, and the OLE route that `.doc`/`.ppt` charts take.
3. What this tree reads and what it draws.
4. A census of all 947 corpus documents, twice over and by two independent methods.

Everything about "the reference" is either a measurement of `/opt/libreoffice26.2`
(26.2.4.2, build `0229ac93fcf0d7cbc6376066c6f35021cef002dc`) or is labelled as read out
of **this C++ tree**, which is 27.2.0.0.alpha0 and is not that binary's source (C8).

---

## 1. The census, and why it can be trusted

Two censuses of the same 947 documents, by methods that share no code.

**Leg A — markup.** OOXML chart parts unzipped and grepped namespace-agnostically
(`census-ooxml.py`); BIFF chart-type records scanned out of `.xls` `Workbook` streams
(`census-biff.py`); OLE objects inflated out of `.ppt` `ExOleObjStg` records and their
embedded compound files scanned the same way (`census-ppt-ole.py`).

**Leg B — the reference's own resolution.** `chart:class` on `<chart:chart>` in
`/home/user/corpus-odf`, which is 26.2.4.2's own export of the same corpus
(`meta:generator` checked on a sample: `LibreOffice/26.2.4.2$Linux_X86_64
LibreOffice_project/0229ac93…`). Joined back to the source document by
family/batch/basename against `MANIFEST.tsv`; **0 of 947 failed to join**.

```
leg A, documents stating chart markup        leg B, documents the reference
  OOXML chart part          169                resolves to a chart
  .xls  BIFF CHCHART          7                <chart:chart chart:class>  182
  .ppt  OLE  CHCHART          7
  union                     183
A minus B (1):  slides/done-007/ppt/71393_pp7.ppt
B minus A (0):
```

The two legs agree on **182 of 183 documents**, and the single disagreement is
predicted by §5: `71393_pp7.ppt`'s two charts are `MSGraph.Chart.8` OLE objects, whose
CLSID is the one CLSID absent from the reference's conversion table. That is the base
rate this audit's numbers sit against — the censuses are not two views of one grep.

Corpus denominators (`MANIFEST.tsv`): 947 documents — 272 docx, 251 pptx, 241 xlsx, 66
doc, 64 xls, 51 ppt, 2 xlsm. 413 of them are in the fifteen `chart*` batches. The ODF
corpus is 338 odt + 307 ods + 302 odp, one per source document.

`sheets/done-010/xls/Special-Procedures_2025-07-10.xls` is not an OLE file and was
skipped by leg A; leg B finds no chart in it either.

---

## 2. Pass 1 — LibreOffice's chart types, definitively

`ChartTypeManager.cxx` is the registry. In **this tree (27.2)** it maps **76 template
service names** (75 distinct — `template.DonutAllExploded` is listed twice, at
`:184` and `:186`) onto **74 `TemplateId` values**, and the templates instantiate **19
chart-type services** named in `servicenames_charttypes.hxx:23-59`.

Two entries are commented out in the registry and have no service at all:
`template.Surface` (`ChartTypeManager.cxx:223`) and `template.Addin` (`:224`).
**There is no `SurfaceChartType` in LibreOffice, in either version.**

### 2.1 Which of the 19 exist in 26.2.4.2

The C++ tree is 27.2 and the calibration target is 26.2.4.2, so each service name was
grepped for in the shipped 26.2 binaries (`libmergedlo.so`, `libscfiltlo.so`,
`services/services.rdb`):

| chart2 service | in 27.2 tree | in 26.2.4.2 binary |
|---|---|---|
| Area, Bar, Column, Line, Scatter, Pie, Net, FilledNet, CandleStick, Bubble | yes | yes |
| BoxWhisker, ClusteredColumn, Funnel, ParetoLine, RegionMap, Sunburst, Treemap, Waterfall | yes | **yes** |
| **Histogram** | yes | **no** |
| Surface | — (commented out) | — |

The same test over the 75 template names finds exactly one absent from 26.2:
`com.sun.star.chart2.template.Histogram`. `com.sun.star.chart.HistogramDiagram` and
the `binning` token are also absent.

**So the brief's guess was half right.** `Funnel` is *not* new — it is in 26.2.4.2 and
so are Pareto, RegionMap, Sunburst, Treemap, Waterfall, BoxWhisker and
ClusteredColumn. `Histogram` **is** new in 27.2 and must not be recorded as something
this tree lacks: the reference cannot draw one either. Its corpus reach is nil twice
over (§4).

### 2.2 Variants, and which of them are a shared path

The 74 template ids are 19 types × their variants. The variants the registry
distinguishes, and what actually changes:

| variant axis | template spelling | separate chart type? |
|---|---|---|
| stacked / percent-stacked | `Stacked*`, `PercentStacked*` | no — same type, `StackingDirection` + `Percent` |
| horizontal vs vertical | `Bar` vs `Column` | **yes** — `BarChartType` vs `ColumnChartType`, two services |
| 3-D flat vs deep | `ThreeD*Flat` vs `ThreeD*Deep` | no — same type, `Dim3D` + `DeepStacking` |
| line / symbol / both | `Line`, `Symbol`, `LineSymbol` | no — `LineChartType` throughout, `Symbol` property |
| spline vs straight | not a template — `CurveStyle` property | no — same type |
| donut vs pie | `Donut*` vs `Pie*` | no — `PieChartType` with `UseRings` |
| exploded | `*AllExploded` | no — `Offset` on each point |
| of-pie | `BarOfPie`, `PieOfPie` | no — `PieChartType` with `SubPieType` |
| filled vs plain net | `FilledNet*` vs `Net*` | **yes** — `FilledNetChartType` vs `NetChartType` |
| stock, 4 forms | `Stock{,Volume}{,Open}LowHighClose` | no — `CandleStickChartType`, `Japanese`/`ShowFirst` |

---

## 3. Pass 2 — each stream's spelling

### 3.1 OOXML (`c:` — `oox/source/drawingml/chart/`)

Sixteen `c:` plot-group elements exist; `plotareacontext.cxx:131-155` dispatches them
and `typegroupconverter.cxx:178-201` assigns the `TypeId`. `typegroupconverter.cxx:69-88`
is the type-id → chart2-service table.

| `c:` element | `TypeId` | chart2 service the reference builds |
|---|---|---|
| `barChart`, `bar3DChart` | `TYPEID_BAR`, or `TYPEID_HORBAR` when `c:barDir val="bar"` (`:209-212`) | `ColumnChartType` |
| `lineChart`, `line3DChart` | `TYPEID_LINE` | `LineChartType` |
| `areaChart`, `area3DChart` | `TYPEID_AREA` | `AreaChartType` |
| `stockChart` | `TYPEID_STOCK` | `CandleStickChartType` |
| `radarChart` | `TYPEID_RADARLINE`, or `TYPEID_RADARAREA` when `c:radarStyle val="filled"` (`:213-216`) | `NetChartType` / `FilledNetChartType` |
| `pieChart`, `pie3DChart` | `TYPEID_PIE` | `PieChartType` |
| `doughnutChart` | `TYPEID_DOUGHNUT` | `PieChartType` + `UseRings` (`:467`) |
| `ofPieChart` | `TYPEID_OFPIE` | `PieChartType` + `SubPieType` (`:474-481`) |
| `scatterChart` | `TYPEID_SCATTER` | `ScatterChartType` |
| `bubbleChart` | `TYPEID_BUBBLE` | `BubbleChartType` |
| `surfaceChart`, `surface3DChart` | `TYPEID_SURFACE` | **`ColumnChartType`** |

The surface row is the one to keep in view. `typegroupconverter.cxx:79` reads

```cpp
const char SERVICE_CHART2_SURFACE[]   = "com.sun.star.chart2.ColumnChartType";    // Todo
```

and `:217-219` forces `mnGrouping = XML_standard` under the comment "create a deep 3D
bar chart from surface charts". **LibreOffice's own answer to a surface chart is a bar
chart.** Not reading one and reading one as a bar are therefore *both* wrong about the
projection; only one of them agrees with the reference.

### 3.2 OOXML chartex (`cx:` — the 2014 vocabulary)

Eight `cx:` layout ids, dispatched in the same switch
(`typegroupconverter.cxx:182,184,186,190,193,197,200,201`): `boxWhisker`,
`clusteredColumn`, `funnel`, `paretoLine`, `regionMap`, `sunburst`, `treemap`,
`waterfall`. All eight resolve to a chart2 service of the same name, and all eight
tokens are present in the 26.2 binary.

`clusteredColumn` becomes `TYPEID_HISTO` when `mbForceHistogram` (`:228-231`) — the
histogram-from-binning path, which is 27.2-only.

### 3.3 BIFF (`sc/source/filter/excel/xichart.cxx`, `xlchart.cxx`)

`CHTYPEGROUP` (0x1014) has exactly one type child, read by
`XclImpChType::ReadChType` (`xichart.cxx:2227-2278`) and resolved by
`XclImpChType::Finalize` (`:2280-2314`). Thirteen type ids, table at
`xlchart.cxx:476-489`.

| record | id | `EXC_CHTYPEID_` | discriminator | chart2 service |
|---|---|---|---|---|
| `CHBAR` | 0x1017 | `BAR` / `HORBAR` | flag `EXC_CHBAR_HORIZONTAL` | `ColumnChartType` |
| `CHLINE` | 0x1018 | `LINE` / `STOCK` | `bStockChart` (drop-bars / chart-lines) | `LineChartType` / `CandleStickChartType` |
| `CHAREA` | 0x101A | `AREA` | — | `AreaChartType` |
| `CHPIE` | 0x1019 | `PIE` / `DONUT` | `mnPieHole > 0` | `PieChartType` |
| `CHPIEEXT` | 0x1061 | `PIEEXT` | — | `PieChartType` |
| `CHSCATTER` | 0x101B | `SCATTER` / `BUBBLES` | flag `EXC_CHSCATTER_BUBBLES` | `ScatterChartType` / `BubbleChartType` |
| `CHRADARLINE` | 0x103E | `RADARLINE` | — | `NetChartType` |
| `CHRADARAREA` | 0x1040 | `RADARAREA` | — | `FilledNetChartType` |
| `CHSURFACE` | 0x103F | `SURFACE` | — | **`ColumnChartType`** (`xlchart.cxx:469`, same `// Todo`) |

There is no BIFF spelling for funnel, treemap, sunburst, waterfall, box-whisker,
region-map, pareto or histogram: the vocabulary predates all of them.

### 3.4 ODF (`xmloff/source/chart/`)

`aXMLChartClassMap` (`SchXMLTools.cxx:136-151`) is the whole vocabulary — **thirteen
`chart:class` values**, of which twelve are drawable:

`chart:line`, `chart:area`, `chart:circle`, `chart:ring`, `chart:scatter`,
`chart:radar`, `chart:filled-radar`, `chart:bar`, `chart:histogram`, `chart:stock`,
`chart:bubble`, `chart:surface`, `chart:add-in`.

Two substitutions are written into the map itself:

- `{ XML_SURFACE, XML_CHART_CLASS_BAR }` with `//@todo change this if a surface chart
  is available` (`:148`), and `GetChartTypeByClassName` appends `"Column"` for
  `XML_SURFACE` (`:263-270`) — the same substitution as the other two streams.
- `chart:histogram` is 27.2-only; 26.2 has neither the token nor `HistogramDiagram`.

Everything chart2 can hold that ODF has no class for is written as an `ooo:` service
name on `<chart:chart>` — measured: `chart:class="ooo:com.sun.star.chart2.
ClusteredColumnChartType"` — with `chart:class="chart:add-in"` on the series that
carry no ODF type.

Of-pie has no class; it is three `loext:` attributes on `<chart:chart>` —
`loext:sub-bar`, `loext:sub-pie`, `loext:split-position`
(`SchXMLChartContext.cxx:446-458` reading, `SchXMLExport.cxx:1331-1372` writing). All
three strings are present in the 26.2 binary.

### 3.5 MS binary Writer and Impress — establishing the route

A chart in `.doc`/`.ppt` is an embedded OLE object, and whether it reaches the BIFF
reader is decided by one CLSID table:
`SvxMSDffManager::CheckForConvertToSOObj` (`filter/source/msfilter/msdffimp.cxx:7130-7144`).
It converts, among others,

```
MSO_EXCEL5_CLASSID       00020810   ->  scalc
MSO_EXCEL8_CLASSID       00020820   ->  scalc
MSO_EXCEL8_CHART_CLASSID 00020821   ->  scalc      // "114465: additional Excel OLE chart classId"
```

and `MSGraph.Chart.8`, CLSID **`00020803`**, is not in the table — `git grep 00020803`
over `include/ filter/ sc/ sd/ sw/ oox/` returns nothing but a match inside a binary
test PDF. An `Excel.Chart.8`
object is therefore loaded by Calc and reaches `xichart.cxx`; an `MSGraph` object is
not converted and falls back to its stored replacement metafile.

**Both legs confirm this.** Leg A found `00020803` charts in exactly one document
(`slides/done-007/ppt/71393_pp7.ppt`, two objects, each with `CHCHART` +
`CHTYPEGROUP` + `CHSCATTER`), and that document is the *only* one of 183 that leg B —
the reference's own export — resolves to no chart.

---

## 4. The table

`c` = OOXML `c:` element, `cx` = chartex layout id, `B` = BIFF record, `O` = ODF
`chart:class`. **Reads** and **draws** are this tree at `83d75ef9`. **Reach** is
distinct corpus documents stating one, from the markup leg, over all streams;
`chart:class` counts beside it are the ODF corpus (which is the ODF reader's whole
reach, the corpus having no native ODF originals).

| LibreOffice type | OOXML | chartex | BIFF | ODF | in 26.2? | reads | draws | reach (docs / 947) |
|---|---|---|---|---|---|---|---|---|
| `ColumnChartType` (vertical bar) | `c:barChart`, `c:bar3DChart` | — | `CHBAR` | `chart:bar` + `chart:vertical="false"` | yes | **yes** | **yes** | 77 c + 6 B = **80** (93 `chart:bar`) |
| `BarChartType` (horizontal) | `c:barChart` + `barDir="bar"` | — | `CHBAR` + horiz flag | `chart:bar` + `chart:vertical="true"` | yes | **yes** | **yes** | 25 c + 2 B = **27** (27 `@vertical`) |
| `LineChartType` | `c:lineChart`, `c:line3DChart` | — | `CHLINE` | `chart:line` | yes | **yes** | **yes** | 32 c + 3 B = **35** (22 dia / 34 ser) |
| `AreaChartType` | `c:areaChart`, `c:area3DChart` | — | `CHAREA` | `chart:area` | yes | **yes** | **yes** | 12 c + 2 B = **14** (14) |
| `PieChartType`, solid | `c:pieChart`, `c:pie3DChart` | — | `CHPIE`, hole = 0 | `chart:circle` | yes | **yes** | **yes** | 18 c + 1 B = **19** (24) |
| `PieChartType` + `UseRings` (donut) | `c:doughnutChart` | — | `CHPIE`, hole > 0 | `chart:ring` | yes | **yes** (OOXML, ODF) / **no** (BIFF) | **yes** | 15 c + **0** B (15) |
| `PieChartType` + `SubPieType` (of-pie) | `c:ofPieChart` | — | `CHPIEEXT` | `loext:sub-bar` / `loext:sub-pie` | yes | **yes** (OOXML) / **no** (ODF, BIFF) | **yes** | 2 c + **0** B (1 `sub-bar`) |
| `ScatterChartType` | `c:scatterChart` | — | `CHSCATTER`, plain | `chart:scatter` | yes | **yes** | **yes** | 23 c + 5 B = **28** (18 dia / 22 ser) |
| `BubbleChartType` | `c:bubbleChart` | — | `CHSCATTER` + bubble flag | `chart:bubble` | yes | **yes** (OOXML, ODF) / **no** (BIFF) | **yes** | 10 c + **0** B (6) |
| `NetChartType` | `c:radarChart` | — | `CHRADARLINE` | `chart:radar` | yes | **yes** | **yes** | 6 c + **0** B (5) |
| `FilledNetChartType` | `c:radarChart` + `radarStyle="filled"` | — | `CHRADARAREA` | `chart:filled-radar` | yes | **yes** (OOXML, ODF) / **no** (BIFF) | **yes** | 1 c + **0** B (1) |
| `CandleStickChartType` (stock) | `c:stockChart` | — | `CHLINE` + drop-bars | `chart:stock` | yes | **yes** (OOXML, ODF) / **no** (BIFF) | **yes** | **0** everywhere |
| *(no type)* surface | `c:surfaceChart`, `c:surface3DChart` | — | `CHSURFACE` | `chart:surface` | **never existed** | **yes**, as bar | **yes**, as bar | **0** everywhere |
| `ClusteredColumnChartType` | — | `cx:clusteredColumn` | — | `ooo:…ClusteredColumnChartType` | yes | **no** | **no** — empty frame | **2** |
| `ParetoLineChartType` | — | `cx:paretoLine` | — | none (series → `chart:add-in`) | yes | **no** | **no** — empty frame | **2** (same 2) |
| `FunnelChartType` | — | `cx:funnel` | — | none | **yes** | no | no | **0** |
| `BoxWhiskerChartType` | — | `cx:boxWhisker` | — | none | **yes** | no | no | **0** |
| `TreemapChartType` | — | `cx:treemap` | — | none | **yes** | no | no | **0** |
| `SunburstChartType` | — | `cx:sunburst` | — | none | **yes** | no | no | **0** |
| `WaterfallChartType` | — | `cx:waterfall` | — | none | **yes** | no | no | **0** |
| `RegionMapChartType` | — | `cx:regionMap` | — | none | **yes** | no | no | **0** |
| `HistogramChartType` | — | `cx:clusteredColumn` + `cx:binning` | — | `chart:histogram` (`loext:` on export) | **NO — 27.2 only** | no | no | **0** |

### 4.1 Variants

| variant | OOXML | BIFF | ODF | reads | draws | reach |
|---|---|---|---|---|---|---|
| stacked | `c:grouping="stacked"` | `CHBAR`/`CHLINE`/`CHAREA` flag | `chart:stacked="true"` | yes | yes | 20 c + 1 B (20) |
| percent-stacked | `c:grouping="percentStacked"` | flag | `chart:percentage="true"` | yes | yes | 7 (7) |
| 3-D | `*3DChart`, `c:view3D` | `CHCHART3D` | `chart:three-dimensional="true"` | read as flat | **flat** | 3 c + 3 B = **6** (6) |
| **spline** | `c:smooth val="1"` | — | `chart:interpolation="cubic-spline"` | **no** | polyline | **11** (12) |
| marker shape | `c:marker/c:symbol` | — | `chart:symbol-type` | yes | yes | (57) |
| exploded | `c:explosion` | `CHPIEFORMAT` | `chart:pie-offset` | — | — | not censused |

---

## 5. Pass 3 — what this tree does, and where it parts from the reference

Nine `ChartPlotKind` values (`XRay.Office.Core/Charts/ChartPlot.cs:25-61`): Bar, Line,
Pie, Area, Scatter, Radar, Bubble, Stock, OfPie. Every one has a drawer in
`ChartLayout.cs:1300-1330`, so for this tree **read and drawn coincide** — there is no
type that is parsed and then rendered as an empty frame *by the layout*. The
empty-frame states all come from the readers returning nothing.

Per reader:

- **OOXML** — `DrawingChartPlot.KindOf` (`:576-587`) covers all sixteen `c:`
  elements including both surface spellings. Complete against §3.1.
- **chartex** — no `cx:` reader exists. `OoxmlXml.ResolveAlternateContent`
  deliberately prefers the unreadable `cx1` choice over Excel's advisory-text fallback
  (`OoxmlXml.cs:167-196`), so the frame draws **empty** rather than printing "This
  chart isn't available in your version of Excel". The reference draws the chart.
- **ODF** — `OdfChartPlot.KindOf` (`:374-393`) covers 9 of the 13 classes; an
  unrecognised class returns null and *"the frame goes back to drawing nothing"*
  (`:86-89`). Missing: `chart:histogram` (nil reach, and 27.2-only anyway),
  `chart:add-in`, `ooo:…` service names, and the `loext:sub-bar`/`sub-pie` of-pie
  attributes.
- **BIFF** — `XlsChartReader` handles 7 of the 9 type records: `Bar` 0x1017,
  `Line` 0x1018, `Pie` 0x1019, `Area` 0x101A, `Scatter` 0x101B, `RadarLine` 0x103E,
  `RadarArea` 0x1040. **`CHSURFACE` 0x103F and `CHPIEEXT` 0x1061 are absent from
  `BiffChartRecords` entirely.** Four discriminators the reference applies are also
  not applied: `CHPIE`'s `mnPieHole` (so a `.xls` doughnut would draw as a solid pie),
  `CHSCATTER`'s bubble flag (a bubble chart would draw as bare markers), `CHLINE`'s
  stock condition (a stock chart would draw as plain lines), and `CHRADARAREA` vs
  `CHRADARLINE` (a filled radar would draw stroked). **Every one of those five has
  zero corpus witnesses** — §6.

### 5.1 A stale comment, not a stale behaviour

`ChartLayout.Plots.cs:14-52` argues at length that *"a surface chart's frame stays
empty"* and that drawing it as a bar chart is "the tempting shortcut". That is an
earlier round's reasoning and it no longer describes the code:
`DrawingChartPlot.KindOf:578` and `OdfChartPlot.KindOf:384` both map surface onto
`ChartPlotKind.Bar`, and `DrawingChartPlot.cs:556-574` says so explicitly and records
the measurement that overturned the older view. The two doc-comments contradict each
other. Nothing renders differently; a reader of `Plots.cs` is misled. Left in place —
this is a survey (§9).

---

## 6. Pass 4 — reach, and which gaps are scope decisions

N11: a count of markup is not a reach figure, and a gap with no witness is a scope
decision, not a bug. Sorted by what it would take:

### Nil reach — record as scope, not as missing features

| gap | witnesses | how established |
|---|---|---|
| `c:surfaceChart`, `c:surface3DChart` | **0 of 766** OOXML documents | `ooxml-raw.tsv`; a second prefix-agnostic pass over every `*chart*.xml` member of every OOXML document found only `pie3DChart` among the 3-D/surface spellings |
| `chart:class="chart:surface"` | **0 of 947** ODF renderings | `odf-joined.tsv` |
| `CHSURFACE` 0x103F | **0 of 64** `.xls`, 0 of 51 `.ppt` | `biff-raw.tsv`, `ppt-ole.tsv` |
| `CHPIEEXT` 0x1061 (BIFF of-pie) | **0** | same |
| BIFF doughnut (`CHPIE` hole > 0) | **0** — the corpus' only BIFF `CHPIE`, in a `.ppt` OLE object, is `CHPIE.solid` | same |
| BIFF bubble (`CHSCATTER` bubble flag) | **0** — all 5 `CHSCATTER` are `.plain` | same |
| BIFF stock, BIFF radar (either) | **0** — no `CHRADARLINE`, `CHRADARAREA`, `CHDROPBAR` or `CHCHARTLINE` in any BIFF chart in the corpus | same |
| `c:stockChart` | **0 of 766** | `ooxml-raw.tsv` |
| `c:bar3DChart`, `c:area3DChart`, `c:line3DChart` | **0 of 766** | `ooxml-raw.tsv` |
| `cx:funnel`, `treemap`, `sunburst`, `waterfall`, `boxWhisker`, `regionMap` | **0 of 766** | `ooxml-raw.tsv` |
| histogram, any spelling | **0**, and 26.2.4.2 has no histogram chart type at all | binary grep, §2.1 |
| **charts in `.doc`** | **0 of 66** | no `00020821` and no `00020803` among any `.doc` `ObjectPool` storage CLSID; 0 `.doc` in leg B |

The whole MS-binary-Writer route is therefore nil reach: this corpus cannot tell
whether a chart in a `.doc` would work, because it holds none.

### Real reach — measured, and ranked

| gap | reach | shows as |
|---|---|---|
| **`c:smooth val="1"` ignored** (spline drawn as polyline) | **11 documents**, 14 chart parts; corroborated at **12 documents** by `chart:interpolation="cubic-spline"` in the reference's own export | wrong path shape, same endpoints. LibreOffice maps it to `CurveStyle_CUBIC_SPLINES` (`typegroupconverter.cxx:689-690`) |
| **chartex charts draw an empty frame** | **2 documents**, both xlsx, both Pareto (`cx:clusteredColumn` + `cx:paretoLine`) | measured on `054_Problem_analysis_with_Pareto_chart` with `--convert-to fods` against 26.2.4.2: the reference resolves a full diagram — `chart:class="ooo:…ClusteredColumnChartType"`, 4 series, 4 data-points, categories, 2 axes, wall and floor. So the whole plot is missing ink, not a detail |
| **3-D drawn flat** | **6 documents** — 3 docx `c:pie3DChart`, 1 `.ppt` OLE 3-D pie, 2 `.xls` 3-D bar | for the four 3-D pies this is nearly free: 26.2.4.2 draws a 3-D pie as a *raster* and emits no pie paths (`probes/chart-resid-r99`), so no flat drawing can match it and no path metric will read the difference honestly |
| **ODF of-pie (`loext:sub-bar`) unread** | **1 document** in the ODF corpus | the source `.docx` is read correctly through the OOXML path (`c:ofPieChart` → `OfPie`); only the ODF round-trip loses it, drawing a plain pie where the reference draws pie-plus-stacked-bar |
| **`MSGraph.Chart.8` OLE chart** | **1 document**, 2 objects | *the reference does not draw it either* — established twice, §3.5. Not a gap in this tree |

One measurement worth recording because it is the reference behaving oddly: of the two
`c:ofPieChart` witnesses, 26.2.4.2's ODF export preserves the sub-type for the
`ofPieType="bar"` one (`loext:sub-bar` + `loext:split-position`) and **loses** it for
the `ofPieType="pie"` one — verified directly with `--convert-to fodt`, which yields a
bare `chart:class="chart:circle"`, one series, four data-points and no `loext:sub-pie`,
even though both `PieOfPieDiagram` and `sub-pie` are strings present in the 26.2
binary. Whether the *rendering* also loses it was not measured; the export and the
view are different code paths, and this survey did not render anything.

---

## 7. Answers to the three questions the brief asked directly

1. **Do `.doc` and `.ppt` charts reach the same reader?** Yes for
   `Excel.Chart.8` / `Excel.Sheet.8` (CLSIDs `00020821` / `00020820`), which
   `CheckForConvertToSOObj` loads through Calc and therefore through
   `xichart.cxx`. No for `MSGraph.Chart.8` (`00020803`), which no code path converts.
   Corpus: 6 of 51 `.ppt` hold `Excel.Chart.8` charts, 1 holds `MSGraph` charts, 0 of
   66 `.doc` hold either.

2. **Is `Funnel` new in 27.2?** No. `FunnelChartType`, `template.Funnel` and the
   `cx:funnel` token are all in the 26.2.4.2 binary.

3. **Is `Histogram` new in 27.2?** Yes. Nothing named `Histogram` —
   service, template, `HistogramDiagram`, `binning` — appears anywhere in the 26.2.4.2
   install. It is the only one of the 19 chart-type services that does not.

---

## 8. What I could not settle

- Whether 26.2.4.2's **rendering** of a pie-of-pie keeps the second plot. Its ODF
  export drops it; export and view are separate paths and nothing was rendered.
- Whether the 2 `.xls` 3-D bar charts and the 1 `.ppt` 3-D pie are drawn as rasters
  the way the OOXML 3-D pie is (`probes/chart-resid-r99`). Not measured here.
- The BIFF record scanner reads `CHBAR`/`CHPIE` flag words directly and did not detect
  the 3-D flag on the three BIFF 3-D charts the reference's export does report. The
  reference's export is taken as the authority for the 3-D count (6); the raw-record
  leg under-counts it, probably because BIFF states 3-D in `CHCHART3D` 0x103A rather
  than in the type record's flags. Not chased — it changes no row.
- Exploded pies (`c:explosion` / `chart:pie-offset`) were not censused.
- Every citation into `/home/user/libreoffice-core` is **this tree, 27.2.0.0.alpha0**,
  not the 26.2.4.2 binary's source (C8). Where a claim mattered it was confirmed a
  second time against the binary — by string presence in `libmergedlo.so` /
  `services.rdb`, or by `--convert-to`, or by the reference's own corpus export.

---

## 9. Deliverable state

**No code was changed.** Per the brief's adjustment for a survey, the full suite was
not run and no baseline is quoted. The one code-adjacent defect found is a stale
doc-comment (§5.1), which changes no rendering and was left alone so that the audit
would not be half a fix round.

Files here: `results.md`, five census scripts (`census-ooxml.py`, `census-odf.py`,
`census-biff.py` with its extracted helper `census_biff_lib.py`, `census-ppt-ole.py`),
the cross-check (`crosscheck.py`), the aggregator (`summarise.py`), and their outputs
(`ooxml-raw.tsv`, `odf-joined.tsv`, `biff-raw.tsv`, `ppt-ole.tsv`, `summary.txt`,
`crosscheck.txt`). Every number in this file came from one of those four `.tsv`s or
from a `grep` over `/opt/libreoffice26.2/program`; all four exist and are non-empty
(487, 1069, 31 and 211 lines).

---

## 10. Register rows added

`dotnet/probes/OPEN-ISSUES.md` gains seven rows and nothing else in it changed:

- **Nil reach** — `N24` surface charts in all four streams, `N25` the six chartex types with
  no witness, `N26` the five BIFF chart-type discriminators `XlsChartReader` does not apply,
  `N27` histogram (which 26.2.4.2 does not have either), `N28` charts in `.doc`.
- **Seated, not dispatched** — `O35` `c:smooth`/`chart:interpolation` unread, reach 11
  documents; `O36` the chartex frame drawing empty, reach 2, with ODF's of-pie beside it at 1.
