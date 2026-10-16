# Round 50 — sheets — prediction (committed before any post-change rendering)

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files` at `MANIFEST.tsv` (946 rows, 307 sheets);
worktree `wt-sheets-r50`, base `ac147b7e5bb`. `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## 0. Baseline reproduced before anything was believed

```
batch-check.sh sample-files 'sheets/chartset-*' … 6
TOTAL 136  MATCH 100  MISMATCH 36  REF-CANNOT-RENDER 0
```

The manifest says 100 `done` and 36 `open` in `chartset-*`. The sweep reproduces it **document for
document** — the 36 mismatching paths are exactly the 36 rows the manifest marks open (30 `text`,
6 `pagination`). Baseline matches. No stored figure on this track had to be discarded.

## 1. The briefed lead is REFUTED, by two independent measurements

The brief's named next action was `[CELLRANGE]` chart data labels, reached via a
`c15:datalabelsRange` census.

**Measurement A — markup.** All 307 sheets documents, opened as OPC, every
`charts?/chart*.xml` part scanned for `datalabelsRange` (byte search, so namespace-prefix
independent):

| | n |
|---|---:|
| sheets documents | 307 |
| carrying `c15:datalabelsRange` | **5** |
| …of the 30 open `text` documents | **3** |
| …of the 6 open `pagination` documents | 1 |
| …already **passing** | 1 (`061_Regional_sales_chart`) |

**Measurement B — our own output.** `pdftotext` over all 136 rendered `chartset` PDFs, both
sides, searching for the literal string `[CELLRANGE]`:

| | n |
|---|---:|
| our PDFs containing `[CELLRANGE]` | **5** |
| reference PDFs containing `[CELLRANGE]` | **0** |

The five are exactly the five the markup census names. So the defect is real and it is *ours*
(the reference never draws the placeholder) — but **it reaches 4 of the 42 open documents, not 30.**

Per the brief's own instruction ("if the census says it reaches only a handful, say so and pivot"):
**pivoting.** Implementing it would have addressed 3 of the 30 `text` documents. Recorded here so
the next round does not re-derive it: the four open witnesses are `063_Sales_pipeline`,
`055_Project_timeline_with_milestones`, `059_Milestone_and_task_project_timeline`,
`047_Date_tracker_Gantt_chart`.

## 2. "Every one of the 30 has a chart in it" is also REFUTED

| of the 30 open `text` documents | n |
|---|---:|
| with ≥1 `xl/charts/chart*.xml` part | 21 |
| with a `chartEx*.xml` part instead (extended chart) | 2 |
| with **no chart part of any kind** — drawings only | **7** |
| OLE2 `.xls` (chart, if any, is BIFF) | 1 |

`068_Blue_inventory_list`, `017_Timeline_Templates_for_Excel`, `070_Equipment_inventory_list`,
`077_Inventory_list_with_highlighting`, `020_Free_Blood_Pressure_Chart`,
`076_Inventory_list_accessibility_guide` and `054`/`051`'s siblings carry no `c:chart` at all.
The pool is not one chart defect.

## 3. The charstream test: the brief's central claim about the pool SURVIVES

All whitespace stripped from both `pdftotext` extractions of all 36 mismatching documents and the
remaining character multisets compared:

**36 of 36 `CONTENT` — zero tokenisation ceilings.** Not one of the 36 has the same characters on
both sides. The brief's "none of the 30 is a ceiling; every one is a real content or layout
difference" is confirmed by an independent measurement.

## 4. What the 30 actually share: nothing. Eight blind page reviews name ≥6 classes.

Eight fresh subagents, none of which had read this brief, the source or any project document, and
each of which saw one paired image and nothing else. Classes named, with the reviewer's own
direction:

1. **chartEx (`cx:`) charts** — we draw Excel's `mc:Fallback` advisory sentence, the reference
   draws the chart. 2 documents (`054`, `051`).
2. **Chart legend** — two independent reviewers (`003_advanced_excel_pie`, `057_Simple_balance_sheet`)
   each named "the reference draws a legend, ours draws none" as *the single biggest thing on the
   page*. Two other reviewers (`037`, `029`) saw the opposite — ours draws a legend the reference
   does not. Legends are implemented and disagree in **both** directions.
3. **PivotTable regeneration** — the reference re-generates the pivot from its cache with
   LibreOffice's own captions ("Total Result", "Row Labels", "Account Checking") and draws the pivot
   *borders*; we replay Excel's cached strings ("Grand Total") and draw no borders. Reviewers 2 and
   7 both flagged this as possibly a **reference-side** divergence. Confirmed in the token census:
   `Grand` only-in-ours on 3 documents, `Total Result` only-in-ref.
4. **Page footer not drawn** — `020_Free_Blood_Pressure_Chart`: the reference draws a URL and a
   copyright line at the foot of all 6 pages; we draw nothing. onlyRef 344 chars, onlyOurs 0.
5. **Autoshape not drawn / text not clipped to it** — `076`: the reference draws a rounded-rect
   "pill" and clips its text; we draw neither the shape nor the clip.
6. **Volatile-formula recalculation** — `071_Four-week_project_timeline` renders Feb 2023 against
   the reference's Aug 2026 (today), and paginates 1 page against 2. LibreOffice recalculates
   `TODAY()` at load; we use the cached value.

## 5. THE CHANGE THIS ROUND MAKES, and the prediction

**Change.** `OoxmlXml.ResolveAlternateContent` prefers `mc:Fallback` whenever a `Choice`'s
`Requires` names a namespace not in `UnderstoodExtensions`. For `Requires="cx1"` (chartex) the
Fallback is a generated `xdr:sp` carrying Excel's advisory prose *"This chart isn't available in
your version of Excel. Editing this shape or saving this workbook into a different file format will
permanently break the chart."* — 26 words that no correct reader shows and the reference never
draws. The change makes the chartex branch selection agree with LibreOffice's: the `Choice` wins,
and because we cannot yet read a `cx:chart` part the frame draws empty.

**Documents expected to change at all — exactly 2**, and this is measured prefix-agnostically over
all 946 corpus documents (every part whose bytes contain the MCE namespace URI, for every prefix
bound to it — `mc` in 560 documents and `ve` in 8):

| document | family | now | predicted |
|---|---|---|---|
| `054_Problem_analysis_with_Pareto_chart_11058329.xlsx` | sheets | 1/1 pages, **87**/61 words | 1/1, **61**/61 |
| `051_Manufacturer_defect_analysis_53db27ea.xlsx` | sheets | 1/1 pages, **95**/69 words | 1/1, **69**/69 |

**Verdict movement predicted: +2.** sheets 265 → **267** of 307. Both documents fail on words
alone (`d=26`, band `max(2%,3)` = 3), pages already agree, and the reference draws *no* text for
these charts at all (blind reviewer 3: "no visible axes lines, tick marks, axis number labels,
category labels, chart title, or legend"), so removing our 26 advisory words should land exactly on
the reference's count rather than near it.

**Predicted movement elsewhere: zero.** 0 words documents, 0 slides documents, 0 other sheets
documents, 0 `done-*` documents.

## 6. What this census CANNOT see

Written down before the sweep, per COMMON.md §6.

1. **It is a census of what a part declares, not of what a page draws.** For these two documents
   the gap is closed — the advisory sentence was confirmed present in our *rendered PDF text layer*,
   not merely in the markup — but that verification does not generalise to any other document.
2. **144 of 946 corpus documents are not OPC** (OLE2 `.xls`/`.doc`/`.ppt`: 55 sheets, 49 slides,
   40 words). A binary chart cannot state `mc:AlternateContent` at all, so the census under-reaches
   by construction on that population. The mechanism being changed is an OPC-only construct, so
   this is a structural rather than a hidden under-reach — but it is exactly the shape of
   under-reach that conceals itself, and it is why the prediction is stated as an exact 2 rather
   than as a floor.
3. **It cannot see inheritance or defaults.** `mc:AlternateContent` has none — a branch is chosen
   at the element — so this particular blind spot is closed here. It would not be for a style or
   theme property, and no conclusion in this file should be carried to one.
4. **It cannot see LibreOffice's branch-selection rule**, only its output on two documents. The
   claim "LibreOffice prefers the chartex Choice and the slicer Fallback" is inferred from two
   rendered pages, not from the binary's code. The slicer half is *not* being changed.
5. **The reference half is re-rendered by `soffice` on every sweep.** If the reference's word count
   for these two documents is not stable at 61 and 69, the prediction is falsified for a reason
   that has nothing to do with the change.
6. **`SOURCE_DATE_EPOCH` is pinned**, which makes our date fields print 2023-11-14 where the
   reference prints today. That affects the *token diff* on ~7 chartset documents and affects
   **no** word verdict (one token either way). Neither of the two documents in the prediction has
   a date field.

## 7. Shared layer

`OoxmlXml` is in **`XRay.Office.Ooxml`**, which serves words, slides and sheets. The change is keyed
on the chartex `a:graphicData/@uri`, which the census above finds in **2 documents of 946, both
sheets, and 0 words and 0 slides documents** — so the cross-track blast radius is measured at zero
rather than argued. The parent should still sweep the other two tracks; the documents to watch, if
any regression appears, are the 109 words documents using `wps`, the 51 using `wpg` and the 4
using `wpc` Choices, since those share the same code path.

## 8. Not attempted, and costed

**A real chartex reader.** Both `chartEx1.xml` parts reference their data by *defined name*
(`<cx:f>_xlchart.v1.1</cx:f>`) with **no `numCache`**, and their `paretoLine` series carry
`ownerIdx` and no `dataId`, so the line is *derived* from the owner series. Implementing it needs
defined-name resolution into the workbook, `cx:aggregation`, and Pareto derivation — a feature, not
a parse. Note for whoever takes it: the reference draws **zero words** for these charts, so an
implementation that draws tick labels would overshoot the word gate that this round's smaller
change is predicted to close.
