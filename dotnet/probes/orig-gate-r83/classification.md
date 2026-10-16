# The 33 original-corpus failures at `gate-orig-r83`, classified

Rows: `rows.tsv` in this directory — 947 documents, reference **26.2.4.2**
(`/opt/libreoffice26.2/program/soffice`), the calibration target. **914 match, 33 do not.**

> **Rows closed since this run.** This file is a snapshot of `rows.tsv` and its table is not
> re-measured; check here before treating a row below as open.
> - `040_Blood_pressure_tracker` (`words`, +18) — closed by the print-zoom interval cap,
>   `probes/chart-axis-r87`, merged at `47259cf60`.

This supersedes the classification written against `orig-gate-r81`, which used the 24.2.7.2
reference and reported 76 failures. Two of that document's three "diagnosed outliers" do not
exist against 26.2.4.2 and must not be dispatched:

- `sectors-defense-and-aerospace.xlsx` — **449/449, `match`.** The 227-page reference was
  24.2's behaviour. There is no font-resolution defect here; the DejaVu Sans mechanism written
  up for it was invented to explain a number from the wrong gate.
- `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` — **88/88, `match`**, 49864 glyphs against 49734.

## By verdict

| verdict | rows |
|---|---:|
| `words` (glyph count outside the band) | 21 |
| `pages` | 7 |
| `pages,words` | 4 |
| `unembedded` | 1 |

## By batch family

`chartset` 14 · `ceiling` 8 · `metrics` 3 · `done` 3 · `unstable` 1 · `table` 1 ·
`pagination` 1 · `missing` 1 · `extra` 1

## Not work: 7 of the 8 ceiling rows

Seven `ceiling-*` rows are **positive** — we draw more text than the reference, because we
replay a metafile as searchable text where 26.2.4.2 rasterises it:

    +500 Thailand17.ppt · +500 W3_Case_Study_of_a_Tsunami… · +1476 OnTrac_StarCertification…
    +1145 16 - UTM - (NASA).pptx · +510 8_P-Pavese_AIRBUS… · +340 Demick_JetBlue.pptx
    +466 150_5300_13_chg8.doc (18 pages against 17)

The eighth, `ABCD-FE-01-00 Flight Envelope`, is **negative** (−72 glyphs, 14 pages against 16)
and is a real pagination row despite its batch.

## The largest live cluster: 14 `chartset` rows

Eleven are `words` with small deltas against a 15–53 band, three are `pages,words`:

| Δ glyphs | band | pages | document |
|---:|---:|---|---|
| −300 | 67 | 5/8 | `047_Date_tracker_Gantt_chart` |
| +308 | 38 | 3/3 | `057_Simple_balance_sheet` |
| −188 | 15 | 1/2 | `071_Four-week_project_timeline` |
| +136 | 29 | 3/3 | `038_Competitive_Advantage_Card` (pptx) |
| +103 | 32 | 8/8 | `030_Basic_balance_sheet` |
| −85 | 21 | 2/2 | `055_Project_timeline_with_milestones` |
| +70 | 53 | 3/3 | `033_Event_planning_tracker` |
| +31 | 15 | 4/2 | `053_Personal_asset_inventory` |
| +29 | 18 | 2/2 | `029_Annual_budget` |
| −28 | 15 | 1/1 | `045_Check_register_with_chart` |
| −24 | 21 | 1/1 | `070_Equipment_inventory_list` |
| +20 | 15 | 2/2 | `065_Weight_loss_tracker` |
| +18 | 15 | 1/1 | `040_Blood_pressure_tracker` |
| +18 | 15 | 1/1 | `075_Idea_planner_tasks` |

Every one of these is a chart-bearing template. The deltas are the size of a handful of axis
tick labels or data labels, which is what makes the cluster worth one round rather than
fourteen: the backlog entries for **chart data labels / value-axis scale** and **`c:smooth`
cubic splines** both live here. `053` additionally carries the hidden-column pagination
question (4 pages against 2).

## The rest, one document each

| verdict | Δ glyphs | band | document |
|---|---:|---:|---|
| `pages` | −2 | 137 | `absrc-pac-01-info-note-en.doc` — 6 pages against 7, text complete |
| `pages` | +68 | 5400 | `OM template for non-complex NCC operators` — 166/165 |
| `pages` | +6775 | 9584 | `SPA-02_mcar_part-2_and_IS_v2.9.docx` — 268/266 |
| `pages` | +6762 | 8290 | `02_mcar_part-2_and_IS_v2.10.docx` — 314/312 |
| `pages` | −259 | 796 | `CRIF - Spécification technique` — 28/29 |
| `pages,words` | +2398 | 2322 | `150_5300_13_chg10.doc` — 77/78 |
| `words` | +1384 | 815 | `UG.CAO.00006 Foreign Part 145 approvals` |
| `words` | −1005 | 324 | `sistem-rekod-markah-srm-_-rekod-master.xlsx` |
| `words` | +68 | 44 | `fse_identification_form.xlsx` |
| `words` | +31 | 25 | `Statement of Work presentation.pptx` |
| `unembedded` | 0 | 168 | `vvsummit2022-Research-Roadmap…pptx` — glyph-exact; a face is named and not embedded |

The two `mcar` rows are the same document at two revisions and should be one round. Both are
+2 pages with a positive glyph delta inside a very wide band, so the pagination is the whole
question — the text is all there.
