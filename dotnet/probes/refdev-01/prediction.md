# refdev-01 — prediction

Written and committed before any reach or verdict was measured. Round `refdev-01`, 2026-08-15,
worktree `wt-refdev`, reference LibreOffice **26.2.4.2**, references reused from
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/`, `SOURCE_DATE_EPOCH=1700000000` on every render
that is diffed.

## What is already established, before the predictions

These are measurements, not guesses, and they are stated here so the predictions below can be
scored against something. Both were taken with `probe-impress.py` and `probe-calc.py`, which
author one page per (face, size), read the baselines out of the reference PDF's own text
matrices at full stream precision, and score six candidate devices side by side.

| application | device | logical unit | fit | tree today |
|---|---|---|---|---|
| Impress | **600 dpi** | 1/100 mm | ascent **507/507**, line height **507/507** | 82/507, 52/507 |
| Calc | **720 dpi** | 1/100 mm | ascent **468/468**, line height **273/273** | 94/468, 96/273 |
| Writer (`lineheight-01`) | 8640 dpi | twip | 195/195 | — |

The line-height rule for both is EditEngine's rather than Writer's: no external leading, and
the height is `max(round(a) + round(d), round(a + d))` — the taller of the text portion's own
height and the formatter metric, `editeng/source/editeng/impedit3.cxx`:1516-1518. Neither
grouping alone fits (600 dpi split is 274/312 and sum 270/312; their max is 312/312).

**Calc is 720 dpi and not the 8640 the tree says.** `ScDocument::GetVirtualDevice_100th_mm`
really is `RefDevMode::MSO1`, and it is not what formats a printed cell: `ScOutputData` formats
against the *output* device, which on a PDF export is the writer's own reference device,
`RefDevMode::PDF1` = 720 dpi. 8640 dpi in 1/100 mm scores 92/273 against 720 dpi's 273/273.
This is the thirteenth prediction in this project to have burned on reading the tree instead of
measuring the binary, and it is the reason the brief said to measure.

## Predictions

| | claim | conf |
|---|---|---:|
| P1 | The slides track holds **no ODP at all** — 112 pptx and 51 ppt — and PPTX/PPT set `FontIndependentLineSpacing`, which never reads a face metric. So the Impress grid reaches only chart labels, and **fewer than 30 of 163 slides renderings change**. | 80% |
| P2 | Sheets is the opposite: every cell's drawn text is `ascent + descent` off a grid-less `LineMetrics`, so **more than 120 of 171 sheets renderings change**. | 75% |
| P3 | Words is byte-identical, **0 of 200**. The two new grids are scoped to their own applications and Writer's path is untouched. | 85% |
| P4 | Net verdicts across slides and sheets: **won ≥ lost**. | 65% |
| P5 | **No `done-*` document in any of the three tracks loses its verdict.** This is the one that matters; a metric touching every line on two tracks is exactly the shape that trades a win for two losses. | 55% |
| P6 | Total verdicts won across both tracks is **3 or fewer** — 14 of slides' 16 failures and 5 of sheets' 8 are documented ceilings, so most of the fidelity gain has nowhere to land. | 70% |
| P7 | Fidelity is **no worse than 30 of 550**. | 50% |
| P8 | `MetricGrid` needs a logical-unit member *and* a second line-height rule: EditEngine's `max` grouping cannot be expressed by the existing `ScaledLineHeight`/`ScaledAscent`. | 80% |
| P9 | Existing slides or sheets unit tests carry line heights computed the old way and **at least three will need their expectations moved**. | 75% |
| P10 | The 39 unmeasured Calc pairs in the `extra` set are one whole face (OpenSymbol or IPAGothic) failing to produce two baselines, not a scattering — an instrument artefact rather than a rule failure. | 70% |
| P11 | Calc's row heights are untouched: `SheetOptimalRowHeights` reads raw design units and applies its own 96 dpi grid, so a grid inside `LineMetrics` cannot reach it. | 85% |
| P12 | Slides' `|ink|` will move on decks holding charts and on nothing else, so any slides verdict that moves will be a chart deck. | 60% |
