# Round 52 — sheets — prediction

Committed **before** the change is written and before anything is rendered post-change.

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r52`,
base `442b0298d80`.

## Baseline, reproduced before anything was touched

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 279 MISMATCH 46`. Scored against
`MANIFEST.tsv`'s 307 sheets paths (the raw total counts 18 case-alias directory entries twice):
**268 match, 39 mismatch**, and the 39 mismatching paths are **exactly** the 39 rows the manifest
marks `open`. Reproduced to the document.

## What the change is

The mechanism on `077_Inventory_list_with_highlighting`, read out of the markup and confirmed
against the reference PDF rather than from a blind reading:

```xml
<x14:cfRule type="iconSet" …>
  <x14:iconSet showValue="0" custom="1">
    <x14:cfvo type="percent"><xm:f>0</xm:f></x14:cfvo>
    <x14:cfvo type="num"><xm:f>-1</xm:f></x14:cfvo>
    <x14:cfvo type="num"><xm:f>1</xm:f></x14:cfvo>
    <x14:cfIcon iconSet="NoIcons" iconId="0"/>
    <x14:cfIcon iconSet="NoIcons" iconId="0"/>
    <x14:cfIcon iconSet="3Flags" iconId="0"/>
  </x14:iconSet>
</x14:cfRule><xm:sqref>B6:B30</xm:sqref>
```

`showValue="0"` suppresses the **cell text**, not the icon: `output2.cxx:1694-1697` clears
`bDoCell` when `pInfo->pIconSet && !mbShowValue`. But `ScIconSetFormat::GetIconSetInfo`
(`colorscale.cxx:1231-1239`) returns **nullptr** when the matched band's *custom* icon is
`NoIcons` (`iconId` forced to −1 by `IconSetRule::importIcon`), and a null `pIconSet` leaves
`bDoCell` alone. So a cell in a NoIcons band **keeps its text**.

Band selection is the **last** matching threshold, not the first (`colorscale.cxx:1200-1215`),
with `Compare` defaulting to `>=` and `gte="0"` making it `>`. Thresholds are resolved against the
range's own sorted numeric values for `percent`/`percentile`/`min`/`max`
(`ScColorFormat::getValues`, `colorscale.cxx:504-549`).

On `077` that gives: value `0` → band 1 (`>= -1`) → `NoIcons` → **drawn**; value `1` → band 2
(`>= 1`) → `3Flags` → **suppressed**. Twelve `1`s vanish, thirteen `0`s stay.

**This is measured, not inferred.** `pdftotext -layout` of the two sweep PDFs shows the reference
drawing `0` on rows 3,4,5,7,9,… and drawing *nothing* on rows 1,2,6,8,10,…, where we draw `1`.

Two things round 50 recorded about this document are **wrong and are refuted by the markup**:

- "thirteen holding `0`, which neither side draws" — the **reference draws all thirteen**, and so
  do we. The 12-token gap is entirely the twelve `1`s.
- "column B's style is `numFmtId=165` (`"$"#,##0`), under which we should draw `$1`" — column B's
  cells are `s="16"`, and `cellXfs[16]` is `numFmtId="0"`, General. `165` belongs to `cellXfs[19]`,
  which is column H (`Inventory value`). We draw a bare `1` because a bare `1` is what General
  says. There is no number-format defect on this document.

The same rule reaches `dataBar` (`<dataBar showValue="0">`, `mbOnlyBar`,
`condformatbuffer.cxx:386`). The x14 `dataBar` extension does **not** carry `showValue` and
`ExtCfDataBarRule::importDataBar` never touches `mbOnlyBar`, so the plain element is the only
source — which is why `036`'s value is suppressed even though its x14 twin says nothing.

Scope: **XLSX/XLSM only** (`XlsxSheetReader`). Nothing in the `.xls`, `.xlsb` or `.ods` readers is
touched.

## The census

Every corpus document (all three families, 946 rows) opened as a zip and every part byte-scanned
for `showValue`. It occurs in **exactly three documents**, all sheets, and every occurrence is a
false:

| document | status | construction | affected cells |
|---|---|---|---:|
| `077_Inventory_list_with_highlighting…xlsx` | **open** | x14 iconSet, custom, `showValue="0"` on `B6:B30` | 12 |
| `066_Agile_Gantt_chart_08f9de45.xlsx` | done | x14 iconSet ×2 per sheet, custom, on `I10:BL35` + `I36:BL36`, three sheets | 15 |
| `036_Simple_to-do_list…xlsx` | done | plain `<dataBar showValue="0">` on `I5:L5` | 1 |

Ten further documents carry `iconSet`/`dataBar` rules with `showValue` **absent** (default true) —
`sistem-rekod-markah-srm`, `088_To-do_list_with_progress_tracker`, `015_Free_Gantt_Chart_Template`,
`041_Business_budget`, `075_Idea_planner_tasks`, `085_Simple_Gantt_chart`,
`069_Blue_modern_balance_sheet`, `078_Modern_inventory_list`, `042_Business_monthly_budget`, and
the still-open `076_Inventory_list_accessibility_guide`. **These are the control group: the change
must move none of them.** Nine of the ten already pass, which is the control COMMON.md § 4 asks
for — the classifier has been run over the documents that already match.

## The prediction

**Three documents change, one verdict moves, sheets 268 → 269 of 307.** Zero page counts change.

| document | before | predicted after | verdict |
|---|---|---|---|
| `077_Inventory_list_with_highlighting…xlsx` | `words` 1/1 pages **335/323** | **323/323** | `words` → **`match`** |
| `066_Agile_Gantt_chart_08f9de45.xlsx` | `match` 5/5 948/933 | **933**/933 | `match`, unchanged |
| `036_Simple_to-do_list…xlsx` | `match` 4/4 78/77 | **77**/77 | `match`, unchanged |

The three landing values are derived, not observed: 12, 15 and 1 tokens, counted from the markup
(twelve `1`s in `B6:B30`; two `1`s and three `2`s per sheet in `I10:BL36` across three sheets; one
`33%` in `I5`) and cross-checked against the token multiset difference of the two baseline
extractions, which is `only-ours = {1×9, 2×6}` for `066` and `{33%}` for `036`.

**`066` and `036` both hold `TODAY()`.** Their *reference* counts move with the wall clock and ours
do not, so the right-hand column of their rows may not be 933 and 77 at the next sweep. What is
predicted about them is **our** count falling by exactly 15 and exactly 1, and their verdict
staying `match`. `077` holds no volatile function, so its row is predicted whole.

**Nothing else moves.** In particular the ten control documents above, and the 39 open documents
other than `077`, are predicted unchanged in every field.

## What this census cannot see

1. **BIFF conditional formatting.** The census opened documents as zips; the 64 `.xls` sheets
   documents were skipped entirely. `CF12`/`CFEX` records can carry an icon set with the value
   hidden and the census would not know. The `.xls` reader is not being changed, so nothing there
   can move — but "reach is 3 documents" is a claim about OPC parts, not about the corpus.
2. **ODS.** `calcext:icon-set` / `calcext:data-bar` were not scanned and are not implemented.
3. **XLSB.** The binary `BrtBeginIcon` record carries the flag in bits, not in text, so a byte
   scan for `showValue` cannot find it. The corpus happens to contain **zero** `.xlsb`, which is an
   extension-count argument rather than a part-level one.
4. **Row-height feedback.** Removing a cell's text can shrink an auto-height row and move a page
   break. All three affected documents state `customHeight="1"` on every affected row (`077` rows
   6–30 at 30 pt, `066` rows 10–36 at 40.05 pt, `036` row 5 at 39.9 pt), so it cannot fire here.
   On a document the census missed, it could — and it would show up as a **page-count** change,
   which is why 0 page-count movement is part of the prediction.
5. **Formula thresholds.** A `cfvo` whose value is a real formula cannot be evaluated here; those
   rules will be skipped rather than guessed at, so such a document would keep its text where
   LibreOffice hides it. None of the three needs it.
6. **`percentile` thresholds** are implemented from the range's sorted values but are exercised by
   **no** corpus document, so they are pinned by authored tests only.
7. **The value pool.** LibreOffice gathers it from the loaded document and shrinks a range that
   runs to the sheet's last row; we gather it from the materialised table. A rule stated over
   `B6:B1048576` could resolve `percent`/`percentile` thresholds differently on the two sides.
8. **Non-PDF extraction.** `xray-office text` will stop emitting these cells' text. No gate column
   sees it and it is not measured here.

## Shared layer

**No.** The diff is expected to touch `XRay.Office.Spreadsheets` only — one new file plus a hook in
`Ooxml/XlsxSheetReader.cs`. Nothing in `Core`, `Containers`, `Text`, `Vector`, `Rendering`,
`Markup` or `XRay.Office.Ooxml`. If that turns out not to hold, the results file will say so and name
a census.

## Second item, if the first lands

The `057_Simple_balance_sheet` legend-entry hypothesis — *a legend whose series carry no `c:tx`
loses its entries, and a legend with no entries draws nothing* — tested on an **authored** chart
varying one thing, plus the reverse direction on `037`/`029`. **No verdict movement is predicted
from it**: `057` fails on page count (4/3), so its legend cannot move its verdict, and any authored
result is a measurement, not a fix.
