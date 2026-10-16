# Round 88 — the `.ods` column, re-measured, and the two attributes ODF spends on a chart label

    ours   = XRay.Office.Cli @ 494ad1a8a (+ this round's diff)
    ref    = /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2 0229ac93fcf0…
    corpus = /home/user/corpus-odf/sheets — 307 `.ods`, 26.2.4.2's own conversion of the corpus
    fonts  = the five tarball confounds as they stood on 2026-09-08; none moved during this round
    rule   = batch-check.sh of 2026-09-05 (column 9, `glyphs`, within max(2 %, 15))
    date   = 2026-09-08

## 0. The headline: the track is not 225 of 307, it is 269

Every figure in the brief came from `probes/odf-gate-r80/rows.tsv`, which is eight merges old.
Re-measured at `494ad1a8a` with a **900 s** render bound instead of 240 s:

| | r80 (banked) | r88 at `494ad1a8a` |
|---|---:|---:|
| `match` | 225 | **269** |
| `words` | 36 | 29 |
| `pages` | 23 | 6 |
| `pages,words` | 10 | 3 |
| `ref-failed` | 11 | **0** |
| `ours-failed` | 2 | **0** |
| total | 307 | 307 |

`head-rows.tsv` is that sweep. The `.ods` column is **87.6 %**, not 73.3 %, and it is no longer
the worst track in the converted corpus by the margin the brief describes.

## 1. The eleven `ref-failed` rows are not work, and the evidence is a stopwatch

`refail.sh` renders each of `odf-gate-r80`'s eleven `.ods` `ref-failed` documents through the
same `/opt/libreoffice26.2/program/soffice`, **one at a time**, with a 900 s bound:

| document | wall | pages | glyphs |
|---|---:|---:|---:|
| `Capability_List_9-14-2022_Dallas_…unsorted.ods` | 2 s | 147 | 146 846 |
| `Keywords_Mapping_Graphs_and_Charts.ods` | 2 s | 46 | 27 201 |
| `RMP 2011-2014 and Inventory.ods` | 3 s | 38 | 98 381 |
| `SIL_TDB605.ods` | 1 s | 44 | 18 693 |
| `cy06_primary_np_comm.ods` | 1 s | 11 | 30 224 |
| `links-2026.ods` | 1 s | 45 | 16 080 |
| `Application_for_authorisation…crowdfunding…ods` | 1 s | 48 | 33 601 |
| `List of EQS securities_0.ods` | 2 s | 104 | 154 929 |
| `FY2023-AIP-grants.ods` | 2 s | 33 | 353 363 |
| `NPIAS_2007_App_A.ods` | 1 s | 128 | 170 540 |
| `ECA Sinters.ods` | 2 s | 163 | 189 068 |

**Eleven of eleven render, in one to three seconds each.** The whole-column sweep agrees:
`REF-CANNOT-RENDER 0` over 307 documents. So the r80 figures were the box, exactly as
`dotnet/CLAUDE.md`'s *a gate run under CPU contention undercounts on the REFERENCE side*
paragraph predicts — a wedged `oosplash` is indistinguishable from a slow render, and three
rounds were sweeping in their own worktrees while r80 ran. The same is true of the two
`ours-failed` rows: both render inside the 900 s bound and both `match`.

**The denominator was never 294.** It is 307, and the brief's *"11 rows the reference itself
could not render"* should be struck.

## 2. Splitting the residual: 29 rows are the ODF reader's, 9 are shared

`join.py` joins each failing `.ods` row to the same document's `.xlsx`/`.xls` row in
`probes/orig-gate-r83/rows.tsv`, which is the original corpus at 292 of 307. `head-split.txt`
is the output.

| | rows |
|---|---:|
| the `.ods` fails and the original **passes** — the ODF reader's | **29** |
| the `.ods` fails and the original fails too — shared layout | **9** |

The nine shared rows are documents the sheets track already carries: `057_Simple_balance_sheet`
and `065_Weight_loss_tracker` are the measured shear/outlining ceiling, `055_Project_timeline`
and `053_Personal_asset_inventory` are the volatile-date and hidden-cells rows the rulebook
already seats, and the rest sit at 3–16 % on both spellings. **Nothing on this track will close
them that would not equally close the `.xlsx`**, so a round working `.ods` should work the 29.

## 3. The cause closed: ODF spends **four** attributes on `DataCaption`, and two of them were unread

### The claim that was in the tree

`OdfChartPlot.LabelOf`'s own remark said:

> ODF folds the four OOXML flags into two attributes … There is no separate "series name" flag,
> which is why nothing here sets `ChartDataLabel.ShowSeries`.

That is false, and it is the disguise `dotnet/CLAUDE.md` already names once for `text:line-break`:
*"ODF states no such thing" is a claim that needs the same grep as any other.*

### What LibreOffice does

`xmloff/source/chart/PropertyMaps.cxx`:247-250 holds **four** `MID_FLAG_MERGE_PROPERTY` rows
against the one `PROP_DataCaption` bit field, not two:

| attribute | row | bit |
|---|---|---|
| `chart:data-label-number` | `MAP_SPECIAL` | `VALUE` / `PERCENT` |
| `chart:data-label-text` | `MAP_SPECIAL` | `TEXT` |
| `chart:data-label-symbol` | `MAP_SPECIAL` | `SYMBOL` |
| `chart:data-label-series` | **`MAP_SPECIAL_ODF13`** | `DATA_SERIES` |

`handleSpecialItem`'s import arms set and unset those bits per attribute
(`PropertyMaps.cxx`:966-991), and `lcl_CaptionToLabel`
(`chart2/source/controller/chartapiwrapper/WrappedDataCaptionProperties.cxx`:74-90) turns them
into `DataPointLabel::ShowSeriesName` and `::ShowLegendSymbol`.
`VSeriesPlotter::createDataLabel` then fills a **four-slot** list — category, **series**, value,
percentage (`chart2/source/view/charttypes/VSeriesPlotter.cxx`:566-596) — and joins the non-empty
slots with the separator. `ChartDataLabel.Compose` was already written to that order; it was
simply never told to show the second slot.

The `ODF13` marking is why this was missed: it is newer than its three neighbours, so a reader
written from the older three looks complete.

### The witness

`003_advanced_excel_pie.ods`'s series style states all four at once —

```xml
<style:chart-properties chart:data-label-number="value-and-percentage"
                        chart:data-label-text="true" chart:data-label-symbol="true"
                        chart:data-label-series="true">
  <chart:label-separator><text:p>; </text:p></chart:label-separator>
</style:chart-properties>
```

— and 26.2.4.2 draws `M1; Actual; 93; 17%` in each wedge where this tree drew `M1; 93; 17%`.
`Actual` is the series' own name, out of the local table's header row via
`chart:label-cell-address="Data.B1:Data.B1"`, which the reader already resolved for the legend.

The **same deck decides it in the other spelling too**: `171128IPAP.pptx` states
`<c:showSerName val="1"/>` eighteen times, that row matches the reference, and its converted
`171128IPAP.odp` twin did not draw one series name.

### The change

`OdfChartPlot.LabelOf` reads `chart:data-label-series` into `ChartDataLabel.ShowSeries` and
`chart:data-label-symbol` into `ChartDataLabel.ShowLegendKey`, each inherited **per flag** from
the plot area's style when the series' own does not state it. Both attributes join the guard that
decides whether a style overrides its parent at all, so a style that states only one of them now
produces a label instead of passing the parent's through unchanged.

`ShowLegendKey` is not decoration: `ChartDataLabel`'s own remark records the key as a
5.98 pt square and an 8.818 pt shift at 10 pt, **measured on `003_advanced_excel_pie`'s reference
rendering** — the very document whose reader could not see the attribute that turns it on.

### What it bought, per document

`before/` and `after/` are ours-only renders of every document in the converted ODF corpus that
mentions either attribute, at the binary either side of the change, with `SOURCE_DATE_EPOCH` set.

| document | before | after | 26.2.4.2 | verdict |
|---|---:|---:|---:|---|
| `003_advanced_excel_pie.ods` | 430 | **487** | 479 | `words` → **match** |
| `011_advanced_excel_pie.ods` | 411 | **463** | 457 | `words` → **match** |
| `019_advanced_excel_pie.ods` | 425 | **481** | 476 | `words` → **match** |
| `027_advanced_excel_pie.ods` | 404 | **461** | 451 | `words` → **match** |
| `018_Weight_Loss_Chart….ods` | 4721 | 4767 | 4808 | match → match, gap 87 → 41 |
| `021_Control_Chart_Template….ods` | 3665 | 3675 | 3695 | match → match, gap 30 → 20 |
| `microsoft_learn_multi_chart_examples.ods` | 794 | 803 | 798 | match → match, gap 4 → 5 |
| `171128IPAP.odp` | 25438 | 25521 | 25432 | match → match, gap 6 → 89 |

Alphanumeric characters, `batch-check.sh`'s own column. Page counts are unchanged on all eight.

**Four verdicts gained, none lost.** The four pies now overshoot by 5 to 10 characters, which is
inside the floor of 15; the residual there is a page-boundary clip — 26.2.4.2 splits
`M3; Actual; 107; 20%` across pages 1 and 2 where this tree draws it whole — and not a label
question.

`171128IPAP.odp` is the one row that moves the wrong way, by **83 characters in 25 432**, or
0.33 % of a deck whose band is 508. It is worth naming rather than hiding: the reference draws
fewer series names on that deck than its own `DataCaption` asks for, which is the same shape as
the five `.ppt` decks `dotnet/CLAUDE.md` records 26.2.4.2 declining to make a field on. Not
chased.

### Confinement, shown rather than asserted

Censused over all 1285 files of the converted ODF corpus by unzipping every `content.xml`:

| | states either attribute (any value) | states one `="true"` |
|---|---:|---:|
| `.ods` | 26 of 307 | **8** |
| `.odp` | 23 of 302 | **1** |
| `.odt` | 8 of 338 | **0** |

Rendering all **57** of those documents at both binaries: **49 are byte-identical and 8 moved**,
and the 8 movers are exactly the 8 that state an attribute `="true"` and whose chart is drawn.
Every document that states the attributes `="false"` — which is most of them, including all 23
`.odp` and all 8 `.odt` in the census — renders byte for byte the same file.

The original corpus cannot be reached at all: `OdfChartPlot` lives in `XRay.Office.OpenDocument`
and its callers are the ODF readers alone, so no `.xlsx`, `.xls`, `.pptx`, `.ppt`, `.docx` or
`.doc` path touches it.

### The gate

| | before | after |
|---|---:|---:|
| `.ods` of 307 | 269 | **273** |

Comparing like for like — no row failed on either side in either run, `REF-CANNOT-RENDER 0`
both times — so the totals are directly comparable for once.

## 4. A second cause, diagnosed and left with its seat

**`style:print="… annotations …"` is not read, and the reference prints the sheet's notes on
pages of their own after the sheet.** `Hazard Analysis Template.ods` (`pages,words` 2/3,
−35.68 %) and `RMP 2011-2014 and Inventory.ods` (`pages` 36/38, −0.44 %) are the only two
documents of the 307 whose page layout states it, and **both are in the failing set**. The
reference's extra pages hold exactly the note text — `RMP`'s page 38 is
`Inventory / B54: / Elina Zheleva: / ex OPS.026 / …`, four notes and nothing else.

The seat is `ATTR_PAGE_NOTES` → `ScPrintFunc::aTableParam.bNotes`
(`sc/source/ui/view/printfun.cxx`:944), which gates `CountNotePages` (`:2557-2600`, filling
`aNotePosList` by walking `HasColNotes`/`HasNote` over the print area) and `PrintNotes`/`DoNotes`
(`:2004-2067`), each note page carrying the sheet's own header and footer. The ODF import is
`PROP_PrintAnnotations` from `style:print`'s `annotations` token
(`xmloff/source/style/PageMasterStyleMap.cxx`:80, `PageMasterPropHdlFactory.cxx`:85).

`XRay.Office.Spreadsheets/TODO.md` already carries *"ODS is not wired at all"* for this; what it
did not have is the reach (**2 of 307**, both failing) or that it is worth two page counts.

## 5. The other groups, sized but not worked

- **Four `pages` rows whose glyph counts are identical** — `Special-Procedures_2025-07-10`
  (21/22, 153 843 both), `sistem-rekod-markah-srm` (22/26, 16 180 both),
  `017_Timeline_Templates` (2/3, 969 both) and `hdss-bulletin-index-2019-2022` (21/24, −0.13 %).
  On `Special-Procedures` the per-page character counts are 6235, 6960, 6647 … against the
  reference's 5808, 6372, 6643 …, so **the shortfall is made on the first two pages and then
  accumulates**; every page from 3 on is within a hundred characters. Our early pages hold more
  rows than the reference's. Three of the four pass as `.xlsx`, so it is the ODF reader's row
  height or its body height and not the shared paginator.
- **A −27.3 % pair**, `052_Manufacturing_output_chart` (357/491) and
  `058_Social_media_engagement_data` (356/490), near-identical templates. Both carry 24
  volatile `TODAY()`, and both draw a date-category axis where 26.2.4.2 emits its labels
  character by character (`9/`, `2`, `2`, …) — the turned-label representation, not a missing
  arrangement. Screen these with `probes/odp-chart-r72/classify.py` before treating the gap as
  text we fail to draw.
- **A `+2.4 % to +4.2 %` cluster of eight chartset workbooks** where we draw *more* than the
  reference. Positive deltas are the raster/outlining ceiling's own sign; run
  `probes/odt-split-r82/overdraw.py` before working any of them.

## Files

| file | what it is |
|---|---|
| `batch-check-tmo.sh` | `batch-check.sh` with `$RENDER_TIMEOUT` (default 240) in place of the two hard-coded 240 s bounds |
| `refail.sh`, `refail-times.tsv` | the eleven `ref-failed` documents rendered alone at 900 s |
| `head-rows.tsv` | the `.ods` column at `494ad1a8a`, 269 of 307 |
| `after-rows.tsv` | the same column with this round's diff, 273 of 307 |
| `join.py`, `head-split.txt` | the `.ods`-fails / original-passes split |
| `score.py` | pages and alphanumeric characters of a PDF, in the gate's own metric |
| `movers.tsv` | the eight documents whose rendering changed, before / after / reference |
