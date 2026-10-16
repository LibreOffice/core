# Round 53 — sheets — prediction (committed before any post-change rendering)

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r53`,
base `fead153ec7c`.

Baseline reproduced before anything was touched: `batch-check.sh sample-files 'sheets/*' … 8` →
`TOTAL 325 MATCH 284 MISMATCH 41`; scored against `MANIFEST.tsv`'s 307 sheets paths that is
**271 match / 36 mismatch**, and the 36 mismatching paths are **exactly** the 36 rows the manifest
marks `open`. Reproduced to the document.

## What round 52 handed over, and what it actually is

Round 52's first item was *"`029_Annual_budget` and `plotVisOnly` — measure whether its source rows
are hidden"*. **They are not, and `plotVisOnly` is not the mechanism.** Two measurements:

- `sheet11.xml` carries **no `hidden` attribute on any row or column**, and the reference's own
  printout draws rows 18 and 39 (`Total $4,000 …` and `Total $2,476 …`) in the sheet body. Nothing
  on that sheet is hidden in either sense.
- An authored variant with `<c:plotVisOnly val="0"/>` on both charts renders through
  `soffice` **byte-for-byte identically in the observable**: left chart still empty, right chart
  still 17 bars.

## The mechanism, established by an authored one-attribute variant

**LibreOffice drops the last row of a chart data range when an imported Excel table with a totals
row ends on that row.** `ScChart2DataSequence::BuildDataCache`,
`sc/source/ui/unoobj/chart2uno.cxx:2616-2632`, whose own comment reads *"Excel behavior: if the
last row is the totals row, the data is not added to the chart."* The table becomes a **named
database range** with `TotalsRow=true` (`sc/source/filter/oox/tablebuffer.cxx:133-137`); the test
is `GetDBAtCursor(nCol, nRow, nTab, AREA)` **per column**, `HasTotals()`, and the range's own last
row equal to the row being read.

Authored variants of `029`, one thing varied at a time, each rendered through `soffice`:

| variant | left chart | right chart |
|---|---|---|
| the corpus file | **empty, axis 0–12** | **17 bars, axis 0–$900** |
| `totalsRowCount="1"` → `"0"` on both tables | **plotted** | **18 bars, axis 0–$3,000** |
| left chart's values `$C$18:$M$18` → `$C$14:$M$14` (a data row) | **plotted** | 17 bars |
| `plotVisOnly` `1` → `0` | empty | 17 bars |
| right chart's ranges shortened to `…$38` (matching its cache) | empty | 17 bars |

One varied attribute turns **both** observables on and off together. That is the single mechanism
round 52 predicted would explain an empty plot, a missing category and an axis rescale at once —
it just is not the one it named.

It also explains the shapes: `029`'s left chart takes its two value series from `$C$18:$M$18` and
`$C$39:$M$39`, which are **entirely** the totals rows of the two tables, so every column is
dropped and the series are empty; the right chart's `$B$22:$B$39` / `$O$22:$O$39` end on one, so
one point goes. And Excel's own caches agree independently — `ptCount` is 17 for an 18-cell range
on `029`, 8 for a 9-cell range on `040_Blood_pressure_tracker`, 3 for a 4-cell range on
`041_Business_budget`.

## Census, and it is small

Every corpus document opened; tables resolved through each worksheet's relationships, honouring the
two conditions `Table::finalizeImport` bails on (a positive `id` and a non-empty `displayName`);
every chart `c:f` parsed and intersected against them.

| | documents |
|---|---:|
| corpus scanned | 946 |
| with a totals-row table at all | 23 |
| **with a chart range ending on such a totals row** | **3** |

All three are sheets; **0 words, 0 slides**. They are:

| document | status | shape |
|---|---|---|
| `029_Annual_budget…xlsx` | open | 2 whole-range hits (series empty) + 2 last-row hits |
| `040_Blood_pressure_tracker…xlsx` | **done** | 3 last-row hits, `$E/$F/$G$12:$20` against table `B11:H20` |
| `041_Business_budget…xlsx` | **done** | 4 last-row hits, `$C/$D/$E$7:$10` against table `C6:F10` |

## Predicted verdict movement: **+1**

| document | before | expected after | verdict |
|---|---|---|---|
| `029_Annual_budget…xlsx` | 341/312, band 6.24 | **310–318** | `words` → **`match`** |
| `040_Blood_pressure_tracker…xlsx` | 167/164, band 3.28 | 164–166 | `match`, held |
| `041_Business_budget…xlsx` | 234/230, band 4.60 | 230–234 | `match`, held |
| every other sheets document | — | unchanged | unchanged |

**0 of 307 page counts change.** A chart is an anchored object; nothing in this diff touches the
print area, the row heights or the column widths.

`040` and `041` are both currently inside the band **by one token** (`d=3` against a `d>3` test,
`d=4` against a band of 4.60). The change moves them **toward** the reference, but they are the two
documents where a second-order effect could put them out, and they are named here because of it.

## What this census cannot see — stated before the sweep

1. **`.xls` (BIFF) list objects.** The census reads `xl/tables/*.xml` out of an OPC package. An
   Excel table in a `.xls` is a BIFF `LIST` record, invisible to it, and the `.xls` chart path
   (`MsBinary/XlsChartSource`) is **not** being changed. If LibreOffice's BIFF import creates a DB
   range with totals, some `.xls` document could be affected and this predicts nothing about it.
2. **Whether our legend drops a series that has lost all its points.** LibreOffice draws no legend
   at all for `029`'s left chart once both series are empty; if `ChartLayout.Entries` keeps a named
   series with no data, `029` lands ~2 tokens above the prediction.
3. **The axis rescale is second-order and its token count is a guess.** The reference's empty left
   chart labels `$0 $2 … $12` (7 tokens) where ours labels `$0 … $4,500` (10), and its right chart
   labels `$0 … $900` (10) where ours labels `$0 … $3,000` (7). Whether our tick chooser reproduces
   either count on the new data is not predicted; it is why the landing band above is ±4 rather
   than exact.
4. **Category-axis label thinning.** The reference prints `Jan … Oct` (10) on the left chart where
   we print `Jan … Nov` (11). Whether the eleventh label survives an empty plot is not predicted.
5. **`pdftotext` fragments a rotated category label.** `029`'s right chart writes its category names
   rotated, and the extractor breaks them into two- and three-letter pieces, so removing the `Total`
   category removes an unpredictable *number of tokens* rather than one.
6. **Multi-column ranges are untested by the corpus.** LibreOffice skips **per column** and fills its
   cache **column-major**; our resolver fills **row-major**. For a 2-D range ending on a totals row
   the two orders differ. All four corpus hits are single-column or single-row, so the corpus does
   not exercise the disagreement and an authored test is the only thing that will.
7. **The `c:strLit` category defect on `029`'s third chart is not being fixed.** The reference draws
   `1` / `2` where we draw `Income` / `Expenses` — LibreOffice's Calc converter hands a literal
   sequence to `createDataSequenceByRangeRepresentation`, which cannot take an array, so the
   categories are lost. Two tokens against two tokens, assumed net **0**. If that assumption is
   wrong, `029` lands ±2 off.
8. Documents whose chart `c:f` the resolver already refuses (multi-area, defined name, external,
   whole-column) keep their cache and are unaffected — but the census parses `c:f` with its own
   regex, so a reference shape neither side parses is counted by neither.

## Shared layer

**No.** The change is confined to `XRay.Office.Spreadsheets` (`Ooxml/XlsxChartRanges.cs`,
`Ooxml/XlsxFile.cs`, one new file). The rule lives in Calc's chart data provider, so a chart
embedded in a `.docx` or `.pptx` — which is fed from its `c:*Cache` and never from a Calc sheet —
is out of reach by construction as well as by census.
