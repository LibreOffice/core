# Round 54 — sheets — prediction, committed before the change

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r54`, base
`4b50291b09d`.

## Baseline, reproduced before anything was touched

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 288 MISMATCH 37`. Scored against
`MANIFEST.tsv`'s 307 sheets paths — the raw total counts 18 case-alias directory entries twice —
that is **274 match, 33 mismatch**, and the 33 mismatching paths are **exactly** the 33 rows the
manifest marks `open`. Reproduced to the document.

## The change: LibreOffice's automatic chart title

`ChartSpaceConverter::convertFromModel` (`chartspaceconverter.cxx:177-208`) gives a chart a title
the file does not spell out. Transcribed from the 27.2 tree in this checkout and then **measured on
26.2.4.2**, the rule is:

- `mbAutoTitleDel` is `c:autoTitleDeleted/@val`, defaulting to `!isMSO2007Document()`
  (`chartspacemodel.cxx:29`, `chartspacefragment.cxx:113`).
- The block is entered when `!mbAutoTitleDel || <c:title> exists`.
- `aAutoTitle` is the **single series title**: only when the first axes set holds exactly **one**
  type group (`plotareaconverter.cxx:170-176`, and a second axes set *clears* it at `:491`), the
  group is a pie/ofPie or states exactly one series, and that series' `c:tx` carries a cached
  string (`typegroupconverter.cxx:272-281`).
- If `<c:title>` exists or `aAutoTitle` is non-empty, a title is created. Its text is the title
  model's own — rich text, then `c:txPr` paragraphs, then the `c:tx` cache
  (`titleconverter.cxx`, `TextConverter::createStringSequence`) — and `aAutoTitle` only when all
  three are absent. When `aAutoTitle` is empty as well, the localized literal **`Chart Title`**
  (`STR_DIAGRAM_TITLE`) is substituted, unless `bShowEmptyTitle` or `bEmptyRichText` holds.

## Census — 946 documents, every `c:chartSpace` part

`dotnet/probes/sheets-r54/census-autotitle.py`. **307 chart parts in 167 zip documents**, and a
separate completeness pass that reads every `.xml` part in every corpus zip and keys on the root
element confirms the census's filename filter misses **0** of them.

| outcome | sheets | slides | words |
|---|---:|---:|---:|
| `own-text` — the title model already carries text; we already draw it | 82 | 28 | 0 |
| `no-title-atd` — `autoTitleDeleted` (stated or defaulted) suppresses it | 37 | 113 | 7 |
| `no-title-nothing` — no title element *and* no single-series title | 13 | 16 | 1 |
| **`series` — the reference draws the series name and we draw nothing** | **6** | 0 | **2** |
| **`literal` — the reference draws `Chart Title` and we draw nothing** | 0 | **2** | 0 |

**Ten chart parts in five documents across all three tracks.** Every one of the ten states
`<c:title>` **and** an explicit `<c:autoTitleDeleted val="0"/>`, so no corpus hit depends on the
default at all.

### Four measured controls on the real corpus, not on authored files

1. **The `series` branch fires.** `005_Contextures` reference draws **13** `Sales` against our
   **6**; `013_Contextures` draws **7** `East` against our **6**; both words documents draw
   `Production in 2017` once and we draw it zero times.
2. **The `literal` branch fires.** `035_Chemistry_Column_PowerPoint_Chart` reference draws
   **`Chart Title` twice** — one per chart part — and we draw it zero times.
3. **`no-title-atd` really draws nothing**, which is the census's largest exposure: 157 parts rest
   on `mbAutoTitleDel` defaulting to *true* for a non-MSO2007 document, and 82 of them would gain
   a title if that default were the other way round. Three sheets documents where the census says
   "suppressed" but names the string that would appear — `052_Manufacturing_output_chart`
   (`COMPONENTS COMPLETED`), `058_Social_media_engagement_data` (`DAILY IMPRESSIONS`) and
   `001_Contextures_chart_sample` (`Amt`) — have **ours = ref = 1** for that string, the one
   occurrence being the worksheet cell. **The default is confirmed on 26.2.4.2.**
4. **`no-title-nothing` really draws nothing.** `005`'s own **chart6** has
   `<c:autoTitleDeleted val="0"/>`, no `<c:title>` and a series with no `c:tx`; its page (page 6)
   has **ours = ref = 1** `Sales`. A negative control inside the very document the change is aimed
   at.

And the MSO-2007 half: 20 corpus documents satisfy `oox`'s rule (`docProps/app.xml` `Application`
starting `Microsoft` and `AppVersion` starting `12.`). **All 14 of their chart parts are
`own-text`**, so no corpus document can separate the two defaults in either direction.

## What I predict

| document | track | before | after | ref | verdict |
|---|---|---:|---:|---:|---|
| `005_Contextures_chart_sample_6e279b08.xlsx` | sheets | 293 | **300** | 300 | `words` → **`match`** |
| `013_Contextures_chart_sample_21b98e22.xlsx` | sheets | 168 | **169** | 169 | `match`, holds |
| `pie-chart-result.docx` | words | 36 | **39** | 40 | `words` → **`match`** |
| `pie-chart-template.docx` | words | 6 | **9** | 9 | `match`, holds |
| `035_Chemistry_Column_PowerPoint_Chart…pptx` | slides | 289 | **293** | 325 | `words`, holds |

**Verdict movement: sheets +1, words +1, slides 0.** Zero page counts change. No other document in
the corpus is touched.

`005` reaches exactly 300 because five titles produce **seven** tokens: charts 2 and 3 straddle the
vertical page split between pages 1 and 2, and the reference emits their title text into both
pages' streams (`ref` page deltas are +3, +2, +1, 0, +1, 0 over pages 1–6). We already emit those
charts' axis labels on both pages, so I expect the same of a title.

## What this census cannot see

1. **Binary charts.** 64 of the 307 sheets documents are `.xls`, and there are `.doc` and `.ppt`
   documents besides; their charts live in a BIFF `Chart` substream, not a `c:chartSpace` part, and
   LibreOffice imports them through `sc/source/filter/excel/xichart.cxx`, which has its own
   auto-title rule. The census cannot see them and the change cannot reach them — so if a binary
   chart *should* gain a title, this round neither finds it nor fixes it.
2. **`cx:chartSpace`.** Two chartex parts in the corpus, a different title model, untouched.
3. **Reflow.** `ChartLayout.DiagramAreaOf` takes the title's height *and* `TitleGap` off the top of
   the diagram area, so on all ten charts the plot area shrinks. The census counts titles; it
   cannot see whether a shrunk plot moves a data label across a page split — which is precisely the
   mechanism the `_advanced_excel_pie` item names. **This is the regression risk and it is the one
   thing the token arithmetic above assumes away.** It is bounded to the five documents named.
4. **Whether *our* renderer emits a straddling title once or twice.** If once, `005` lands at 298
   against 300, a difference of 2, which is inside the gate's `d > 3` floor — the *verdict*
   prediction survives, the *token* prediction does not.
5. **The MSO-2007 default**, which this reader cannot evaluate because `XRay.Office.Ooxml`'s chart
   entry point takes an element and not a package. It is implemented as "not MSO 2007", which is
   right for 926 of the 946 corpus documents and unfalsifiable on the other 20.
6. **Localization.** `STR_DIAGRAM_TITLE` is `Chart Title` in the en-US UI this container runs; a
   reference under another locale would draw another string.

## Shared layer

**Yes.** The change is in `XRay.Office.Ooxml/DrawingML/DrawingChart.cs` and
`DrawingChartPlot.cs`, which words, slides and sheets all read through. The census above names the
five affected documents; the parent should run the cross-track sweep. My own measurement of the two
affected batches will be `words/chartset-001` and `slides/chartset-007`, whose baselines are
recorded here: words 10 → MATCH 9, slides 10 → MATCH 1.
