# Round 53 — sheets — prediction addendum: a pivot table's repeated row labels

Committed before the post-change sweep. **Process note, stated plainly rather than hidden:** unlike
the first prediction, this one was written *after* a four-document spot render (`DynamicBubbleChart`,
`037`, `049`, `053`), so the four figures in the table below are **measurements, not predictions**.
The prediction proper is everything else: the remaining 303 documents of the track, the page counts,
and the ten other pivot documents.

## What `DynamicBubbleChart`'s remaining 8 words are

Its whole gap is three things, and `pdftotext`'s token multiset names them exactly:

| | ours | reference |
|---|---|---|
| `Finance`, `Information Technology`, `Production`, `Purchase` on page 1 | **3 times each** | **once each** |
| the slicer advisory's line break | `2010.If` (1 token) | `2010.` + `If` (2 tokens) |
| a pivot filter caption on page 2 | absent | `(empty)` |

## The mechanism, and it is *not* `fillDownLabels`

Rows 29–41 of the `Chart` sheet are a **pivot table** — `pivotTable1.xml` states
`<location ref="A29:E41" firstHeaderRow="1" firstDataRow="1" firstDataCol="5"/>` with five row
fields and no data fields. Excel wrote its laid-out result into those cells, filling the repeated
labels down because all five fields carry `<x14:pivotField fillDownLabels="1"/>`. **Calc regenerates
the output from the definition through `ScDPOutput` and writes a row field's label only where its
group starts.**

Three authored variants rendered through LibreOffice 26.2.4.2, one thing varied at a time:

| variant | labels drawn per department on page 1 |
|---|---|
| the corpus file | **1** |
| `fillDownLabels="1"` → `"0"` | **1** — the attribute is ignored |
| the pivot table part removed | **3** — which is what we draw with it present |

The third is the decisive one: with no pivot part the reference prints exactly what we print, so the
blanking is the pivot's doing and nothing else's.

## The rule, and why it is a prefix test

A cell in a pivot's row-label columns is drawn blank when the labels **from the outermost row field
through this one** all repeat the row above. Testing the cell alone would blank
`DynamicBubbleChart`'s `Cost` column, which holds `150` on two consecutive rows under two different
risk values and which the reference prints both times.

## Census

| | documents |
|---|---:|
| zip documents scanned | 802 |
| carrying a `pivotTable` part | **11** |
| stating `fillDownLabels="1"` | **1** (`DynamicBubbleChart`, open) |

The other ten are all `done`. On each of them **Excel itself wrote the repeats blank**, because
without "Repeat All Item Labels" that is what Excel's own layout does — so there is nothing for this
rule to blank and it is a no-op. Eight of the ten have a single row field, where a repeat cannot
occur at all: a pivot field's items are distinct. The two with more are
`037_Personal_money_tracker` (2 fields, `done`) and `alle einzeln.xlsx` (6 fields, 1010 rows,
`done`), and they are the two to watch.

## Predicted verdict movement: **+1**

| document | before | after | verdict |
|---|---|---|---|
| `DynamicBubbleChart.xlsx` | 349/341, band 6.82 | **339** *(measured)* | `words` → **`match`** |
| `037_Personal_money_tracker…xlsx` | 501/505 | **501** *(measured)* | `match`, held |
| `049_Expenses_calculator…xlsx` | 330/332 | **330** *(measured)* | `match`, held |
| `053_Personal_asset_inventory…xlsx` | 59/45 | unchanged | `pages,words`, held |
| `alle einzeln.xlsx`, `Keywords_Mapping…`, `026`, `027`, `007`, `033`, `035` | — | **unchanged** | `match`, held |
| every other sheets document | — | unchanged | unchanged |

**0 of 307 page counts change.** Blanking a cell's text cannot move a print area: the row still
exists, the column widths are unchanged, and nothing here touches `SheetPrintSetup`.

Cumulative for the round: **271 → 273 of 307**, +2.

## What this census cannot see

1. **`.xls` and `.xlsb` pivot tables.** The census reads `xl/pivotTables/*` out of a zip. A BIFF
   `SXVIEW` pivot is invisible to it and the `.xls` path is not changed. 144 corpus documents are
   `.xls`.
2. **A pivot whose output range the `location` understates.** The rule blanks only inside
   `location/@ref`; if Calc regenerates a pivot to a *different* size than Excel wrote — which is
   the rest of the PivotTable-regeneration class and is not being attempted — the cells outside
   that range are untouched and this predicts nothing about them.
3. **Whether the repeat test on the stated cell content agrees with the test on the displayed
   string.** Two cells holding different shared-string indices for the same text would not be
   treated as a repeat. No corpus case exercises it; the choice is deliberate (a pivot item is
   distinct by construction) and is asserted by a test rather than by a document.
4. **The remaining two tokens on `DynamicBubbleChart` are not addressed**: the advisory's
   `2010.If` line break (a text-box width question, the same shape as `070_Equipment_inventory_list`'s
   12-word gap) and the reference's `(empty)` pivot filter caption. The document is predicted to
   land inside its band *with* both still wrong.

## Shared layer

**No.** `XRay.Office.Spreadsheets` only — one new file, plus `LoadPivotTables` on `XlsxFile`, one
optional parameter on `XlsxSheetReader.ReadSheet` and one call site in `XlsxChartRanges`.
