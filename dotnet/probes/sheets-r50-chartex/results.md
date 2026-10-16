# Round 50 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, base `ac147b7e5bb`.
Read `prediction.md` beside this file first — it was committed (`ba56ee826c0`) before any
post-change rendering.

## 1. Baseline reproduced

`batch-check.sh sample-files 'sheets/chartset-*' … 6` → `TOTAL 136 MATCH 100 MISMATCH 36`.
The manifest's `chartset-*` rows are 100 `done` and 36 `open`, and the 36 mismatching paths are
**the same 36 paths**. Nothing had decayed; no stored figure had to be discarded.

## 2. The measurement against the prediction

Predicted: exactly 2 documents change, both to exact word matches, **+2 verdicts**, zero movement
anywhere else.

`TOTAL 136 MATCH 102 MISMATCH 34`. Row-by-row diff of the two sweeps, all 136 documents, all
fields (verdict, pages, words):

| document | before | after |
|---|---|---|
| `054_Problem_analysis_with_Pareto_chart_11058329.xlsx` | `words` 1/1 pages **87/61** words | `match` 1/1 **61/61** |
| `051_Manufacturer_defect_analysis_53db27ea.xlsx` | `words` 1/1 pages **95/69** words | `match` 1/1 **69/69** |
| every other document (134) | — | **no field changed** |

**Prediction met exactly, including the exact landing values.** sheets **265 → 267 of 307**.

Regression sweeps:

- `sheets/done-*` — `TOTAL 157 MATCH 157 MISMATCH 0`.
- `sheets/{ceiling,extra,metrics,missing,pagination,table,unstable}-001` — the only mismatches are
  the 5 known `ceiling` documents and the 1 known `unstable` one. No new failure.

Tests, run per project and totalled by hand (a whole-solution run is the one most likely to
truncate silently):

| project | result |
|---|---|
| Containers | 109 passed |
| Core | 337 passed |
| Markup | 259 passed |
| OpenDocument | 125 passed |
| Presentations | 736 passed |
| Rendering | 150 passed, 1 skipped |
| Spreadsheets | **886** passed (was 882; +4 new) |
| Text | 596 passed |
| Vector | 295 passed |
| WordProcessing | 1052 passed |
| **ten non-Fidelity projects** | **4545 passed, 0 failed** |
| Fidelity | 521 passed, **31 failed**, 552 discovered |

**The 31 Fidelity failures are pre-existing and this change moves none of them.** Measured, not
argued: the two changed source files were checked out at `ac147b7e5bb`, `XRay.Office.Ooxml`'s
`obj`/`bin` deleted, the tree rebuilt, and the *base* binary confirmed to still draw the advisory
sentence before the run. It reported **`Failed: 31, Passed: 521, Total: 552`** — identical. The
files were then restored with `git checkout HEAD -- … && touch …`, rebuilt from a deleted
`obj`/`bin`, and the restored binary confirmed to draw the sentence zero times. The failures span
words, slides *and* sheets fidelity classes, which is the second, independent tell.

`dotnet build -v q -nologo` → **0 warnings, 0 errors.**

## 3. Tests, and which are detectors

`OoxmlAlternateContentTests`, 4 tests, in `XRay.Office.Spreadsheets.Tests`. Verified with
`verify-test.sh` under two separate mutations:

| mutation | detected by |
|---|---|
| the original defect — chartex URI constant altered so the guard never matches | `AnExtendedChartChoiceBeatsItsAdvisoryFallback`, `TheExtendedChartExceptionDoesNotDependOnTheMarkupCompatibilityPrefix` |
| over-generalisation — the guard fires for *any* graphic-data URI | `ASlicerChoiceStillLosesToItsFallback` |

**Three of four verified by reintroduction, in both directions** — the rule is pinned against being
removed *and* against being widened. The fourth,
`AnUnreadableChoiceWithNoFallbackBesideItIsStillDropped`, is a **drift guard**: it documents
pre-existing behaviour the change cannot reach. It earned its place by failing on first write —
I had assumed a choice with no fallback beside it was taken, and it is dropped, which is what MCE
actually prescribes.

## 4. Refutations — the round's larger result

### 4a. `[CELLRANGE]` / `c15:datalabelsRange` is not the pool. Refuted twice, independently.

| | n |
|---|---:|
| sheets documents | 307 |
| carrying `c15:datalabelsRange` (markup, byte search, prefix-independent) | **5** |
| our rendered PDFs containing the literal `[CELLRANGE]` | **5** — the same five |
| **reference** PDFs containing it | **0** |
| …of the 30 open `text` documents | **3** |

The defect is real and it is ours. Its reach is **4 of the 42 open sheets documents**, not 30. The
open witnesses are `063_Sales_pipeline`, `055_Project_timeline_with_milestones`,
`059_Milestone_and_task_project_timeline`, `047_Date_tracker_Gantt_chart`; `061_Regional_sales_chart`
carries it and **passes the gate anyway**, which is the useful control — the gate is blind to it on
at least one document, so closing it should not be expected to move four verdicts either.

### 4b. "Every one of the 30 has a chart in it" — refuted.

7 of the 30 open `text` documents contain **no chart part of any kind**, only drawings; 2 more
carry `chartEx` rather than `c:chart`; 1 is OLE2. The pool is not one chart defect, and a round
briefed to look for one will not find it.

### 4c. What the brief got right, confirmed independently.

The **charstream test** (all whitespace stripped from both `pdftotext` extractions, remaining
character multisets compared) returns **`CONTENT` on 36 of 36**. Not one mismatching chartset
document has the same characters on both sides. "None of the 30 is a ceiling" survives.

## 5. What the 30 actually share: eight blind readings say ≥6 classes

Eight fresh subagents, none of which read this brief, the source, or any project document; each saw
one paired image and nothing else; each asked to describe both halves separately, give direction,
and say what looked identical. Classes, with the reviewers' own directions, for the next round:

1. **Chart legend, disagreeing in both directions.** Two reviewers, on unrelated documents
   (`003_advanced_excel_pie`, `057_Simple_balance_sheet`), *independently* named "the reference
   draws a legend, ours draws none" as the single most prominent defect on their page. Two others
   (`037`, `029`) saw the reverse — ours draws a legend the reference does not. Legends are
   implemented; the *selection* rule disagrees. **This is the strongest unexplored lead on the
   track and it is a two-reviewer agreement, which is the evidence standard this project trusts.**
2. **PivotTable regeneration — probably a reference-side divergence.** LibreOffice re-generates the
   pivot from its cache with its own captions ("Total Result", "Row Labels", "Account Checking") and
   draws pivot borders; we replay Excel's cached strings ("Grand Total") and draw no borders.
   Reviewers 2 and 7 both flagged, unprompted, that ours may be the faithful side. Corroborated by
   the token census: `Grand` only-in-ours on 3 documents. **Measure whose string is in the cell
   before implementing anything here.**
3. **Page footer not drawn** — `020_Free_Blood_Pressure_Chart`, onlyRef 344 characters, onlyOurs 0;
   a URL and a copyright line at the foot of all 6 pages. Note headers/footers *are* implemented
   (our date fields print), so this is specific, not a missing feature.
4. **Autoshape not drawn, and its text not clipped to it** — `076_Inventory_list_accessibility_guide`
   (ours 1114/1091, band 21.82, so `d=23` only just fails).
5. **Volatile-formula recalculation** — `071_Four-week_project_timeline` draws Feb 2023 against the
   reference's Aug 2026 and paginates 1 page against 2. LibreOffice recalculates `TODAY()` at load;
   we use the cached value. Reaches ~7 chartset documents in the token census.
6. **Slicer fallback not drawn** — the mirror of this round's fix. LibreOffice draws the slicer
   `mc:Fallback` advisory box (`Requires="a14"`/`sle15`, 7 and 5 sheets documents); our resolver
   selects the same branch and we draw nothing. `DynamicBubbleChart` ref 1 / ours 0.

## 6. A well-localised lead measured but not taken

`077_Inventory_list_with_highlighting` — ours 335 words, reference 323, and the whole difference is
**twelve literal `1` tokens we draw and the reference does not**, all at x = 48.7 pt, on twelve
rows. The sheet has **exactly twelve** cells in column B holding the value `1` (and thirteen holding
`0`, which neither side draws) — a `IFERROR((stock<=reorder)*(discontinued="")*valHighlight,0)`
conditional-formatting helper column. The band is `max(2%,3)` = 6.46, so removing them is one
verdict. **Not resolved**: column B's style is `numFmtId="165"` (`"$"#,##0`), under which we should
draw `$1`, and we draw a bare `1` — so the format we resolve for those cells is not the one the
markup states, and why the reference draws neither the ones nor the zeros is still open. Worth a
crop of that column on both sides as the first step.

## 7. Shared layer

The diff touches **`XRay.Office.Ooxml/OoxmlXml.cs` and `OoxmlNamespaces.cs` — a shared layer**, used
by words, slides and sheets.

The reach is **measured, not reasoned about**. Every part of every one of the 946 corpus documents
whose bytes contain the MCE namespace URI was scanned, for every prefix bound to it (`mc` in 560
documents, `ve` in 8), and every `a:graphicData/@uri` inside a `Choice` collected:

| `a:graphicData/@uri` inside an `mc:Choice` | documents | families |
|---|---:|---|
| `…/word/2010/wordprocessingShape` | 109 | words |
| `…/word/2010/wordprocessingGroup` | 51 | words |
| `…/drawing/2010/slicer` | 7 | sheets |
| `…/word/2010/wordprocessingCanvas` | 4 | words |
| **`…/drawing/2014/chartex`** | **2** | **sheets** |
| `…/drawingml/2006/picture` | 1 | words |

The change is keyed on the last-but-one row and nothing else, so **the cross-track blast radius is
2 documents, both sheets, 0 words and 0 slides**. The parent should still run the cross-track sweep;
if anything moves, the documents to look at are the 109 `wps`, 51 `wpg` and 4 `wpc` words documents,
which share the code path but not the branch. Note also that all ten non-Fidelity test projects —
including WordProcessing (1052) and Presentations (736) — are green, and Fidelity is bit-identical
to the base commit at 521/31/552.

## 8. Costed and deliberately not attempted: a real chartex reader

Both `chartEx1.xml` parts reference their data by **defined name** (`<cx:f>_xlchart.v1.1</cx:f>`)
with **no `numCache`**, and their `paretoLine` series carry `ownerIdx` and no `dataId`, so the line
is *derived* from the owner series. It needs defined-name resolution into the workbook,
`cx:aggregation`, and Pareto derivation — a feature, not a parse.

**The warning for whoever takes it**, because it inverts the obvious expectation: LibreOffice's own
chartex rendering is degraded — a blind reviewer described the reference as "a blue descending
polyline plus a flat green line… no visible axes lines, tick marks, axis number labels, category
labels, chart title, or legend". It contributes **zero words**. So a *correct* chartex
implementation that drew tick labels would overshoot the word gate that this round's much smaller
change has just closed, and would read as a regression on the scoreboard while being better output.
Record the ink measurement alongside the word count when that round happens.
