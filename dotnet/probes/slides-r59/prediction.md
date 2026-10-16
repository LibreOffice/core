# slides-r59 — prediction

Written and committed **before** anything was built or rendered post-change.

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
base `dc9ca5900c2`, branch `wt-slides-r59`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## Baseline, re-derived rather than quoted

Full sweep of `slides/*` at the base commit, reference PDFs reused from round 56's
`ink-base/ref` (302 of them, unchanged since nothing has touched `soffice`):

| | briefed | measured at `dc9ca5900c2` |
|---|---|---|
| passing over `MANIFEST.tsv` | 199 of 302 | **199 of 302, 0 disagreements** |
| `abs_ink` | 1106.97 (r56 final) | **1107.04** |
| signed ink | 802.45 | **802.52** |
| major pages | 385 | **385** |

The sweep's own `TOTAL` is **315** files for 302 manifest paths — four more than round 56's 311,
with no commit to the corpus. That is the alias materialisation `CLAUDE.md` records. Scored
against `MANIFEST.tsv` by a scorer that now **refuses to print** unless every manifest path found
a row (`scratch-r59-slides/score.py`, one added `SystemExit`).

## What this round changes

### §1 The plot rectangle is a tick length the axis never draws

The brief's item 1. Measured, not argued: a census of every rendered chart page's plot rectangle,
read off the *gridline* families in both PDFs (`plot/plotrect-census.py`), reproduces both of the
brief's known figures exactly — `Demick_JetBlue` page 4 `dLeft +5.69 dBottom +5.54 dRight −0.09
dTop +1.08`, `N2_E_Maestroni` page 7 `dLeft +15.60 dBottom −21.65 dRight +8.56` — and then shows a
**cluster at `dLeft ≈ +4.2, dBottom ≈ +4.9`** on a dozen unrelated documents. `TickLength` is
`150` hundredths of a millimetre = **4.252 pt**.

The discriminator is `c:majorTickMark`, which this reader does not read at all. A probe that
patches that one property, one axis at a time, in a corpus chart already stating `none` on both
axes gives **6 of 6**:

```
              dLeft   dBottom
none            0.00     0.00      in         0.00   0.00
out            +4.25     0.00      cross     +4.25  +4.25   (both axes)
```

So the tick length is reserved **only for `out` and `cross`**, on that axis' own edge, and we
reserve it for every visible axis. `axismodel.cxx:42-48` defaults it to `out` for a 2007 chart and
`cross` otherwise — both of which reserve — so an **absent** `c:majorTickMark` keeps today's
behaviour and only a stated `none` or `in` changes.

### §2 The automatic gridline and axis-line format is the theme's subtle line style

The brief's item 2, and it is **three things, not one**. A five-arm probe patching one thing at a
time in the same deck:

| arm | major grid | minor grid | axis line | width |
|---|---|---|---|---|
| base (`tx1` = black) | `#666666` | `#8B8B8B` | `#666666` | 0.73 pt |
| theme `dk1` → `2050C0` | `#676E9C` | `#8B8FA7` | `#676E9C` | 0.73 pt |
| theme `dk1` → `FFFFFF` | `#BCBCBC` | `#BCBCBC` | `#BCBCBC` | 0.73 pt |
| `lnStyleLst[0] w` 9525 → 38100 | `#666666` | `#8B8B8B` | `#666666` | **3.00 pt** |
| `lnStyleLst[0] w` 9525 → 4763 | `#666666` | `#8B8B8B` | `#666666` | **0.37 pt** |

We draw `#B3B3B3` for both grids, **`#000000` for the axis line** — which the brief does not
name — and a hairline for all three. The white arm is the discriminator that kills "a constant
grey": a tint of white is white, and both tints collapse onto one value only because the theme's
own `shade 50000` is applied on top of the substituted `phClr`, exactly as
`LineFormatter::convertFormatting` does for a series stroke. `DrawingChartAutoFormat`
already has `ThroughSubtleLineStyle` for that; the gridline and axis-line tables are what is
missing.

The **tick label** colour does not move: both stacks draw page 4's labels `#000000`, so only the
axis *line* changes.

## What I expect to move

| | prediction |
|---|---|
| verdicts, slides | **0**, band −1 … +1 |
| page counts | **0 of 302** |
| documents moved on ink by ≥0.005 | **12 – 22** |
| `abs_ink`, slides | **−4 … −14** |
| `Demick_JetBlue.pptx` | 12.85 → **below 11** |
| `N2_E_Maestroni_Swarm_COP.pptx` | 3.93 → **below 3.5**, major stays 1 |
| plot-rect census: chart pages with all four edges inside 0.5 pt | 10 of 57 → **20 or more** |
| controls: `tf-agreement`, exact `/Tf` pages, sheared glyphs | **all three unchanged** — no text metric is touched |

### The documents I expect to move, named

`c:majorTickMark` `none`/`in` on a live axis of a chart that does **not** state a plot-area
`c:manualLayout` — 38 chart parts in the slides track:

`southern-classic-kennesaw-state-university-final`, `Demick_JetBlue`, `171128IPAP`,
`8_P-Pavese_AIRBUS-ATB-journee-CRATB`, `1_Country-Updates_DRC_English`,
`FAAAIandtheArtandScienceofV&Vfinal`, `N2_E_Maestroni_Swarm_COP`, `flying-by-numbers-presentation`,
`RPA P4 - Advanced Material`, `Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final`,
`038_Competitive_Advantage_Card…`, `bar_chart`, `line_chart`, `scatter_chart`,
`stacked_bar_chart`, `stacked_area_chart`, `combo_bar_line_chart`,
`007/015/023/031_advanced_powerpoint_scatter`.

Plus, for §2 only: the 4 slides documents with an automatic **major** grid, the 2 with an
automatic **minor** grid and the 5 with an automatic **axis line** — `Demick_JetBlue` is in all
three sets and is the only one also in the §1 list.

## What the census cannot see

1. **It reads what a chart part states, not what a shape resolves to.** A chart on a hidden slide,
   on a page neither stack renders, or inside a group that is never drawn is counted and will not
   move. This over-counts and conceals nothing.
2. **It cannot see the other formats.** `.ppt` and `.xls` charts do not come through
   `DrawingChartPlot` at all, and neither does an ODF chart; a chart that reaches the page as an
   **EMF or WMF picture** is unaffected by both changes and is invisible to this census in the
   other direction — it is counted as no chart at all.
3. **It cannot see whether 4.25 pt changes a pixel.** A chart whose plot area carries no grid, no
   wall fill and no axis line moves its contents and may still register almost no ink.
4. **It cannot see the theme.** §2 does nothing for a chart whose theme states no `a:lnStyleLst`;
   `DrawingStyleMatrix.LineStyle` returns null and the colour stays what it is today. No corpus
   theme is known to omit it, and that is an assumption this census does not test.
5. **It cannot see `c:manualLayout` on anything but the plot area.** A chart stating a manual
   layout for its *legend* or *title* still takes the computed plot path and is counted correctly;
   one whose axis is deleted in a form my regex misses is not.
6. **The 4.25 pt is a reservation, and reservations interact.** On a column chart the bottom edge
   takes `max(categoryHeight + categorySpace, valueHeight / 2)`. Dropping the tick from the first
   term may leave the second the larger, in which case the bottom edge moves by **less** than
   4.25 and the residual is a different defect. `bar_chart.pptx`'s `dBottom` is `+4.89`, which is
   `4.25` plus a `0.64` I have not explained and am not fixing.

## Cross-track reach — this is a shared-layer diff and the parent must gate the corpus

Both changes touch **`XRay.Office.Core/Charts`** and **`XRay.Office.Ooxml`**. Named from the census,
counted as chart *parts* on the computed path with a live `none`/`in` axis:

* **sheets — 56 chart parts** in ~62 workbooks, `Keywords_Mapping_Graphs_and_Charts.xlsx` (22 live
  axes) the largest, plus `046_Cost_analysis_with_Pareto_chart`, `029_Annual_budget`,
  `041_Business_budget`, `053_Personal_asset_inventory`, `009/010/023_advanced_excel_*`,
  `004/006_Contextures_chart_sample`, `052_Manufacturing_output_chart`, `061_Regional_sales_chart`,
  `040_Blood_pressure_tracker`, `037_Personal_money_tracker` and the rest of `chartset-00*`.
* **words — 2 chart parts** in 2 documents.
* §2 additionally reaches **8 sheets documents** with an automatic major grid, **9** with an
  automatic axis line, and **1 words document** with an automatic major and minor grid.

Predicted cross-track verdict movement: **0**. That is a prediction, not a measurement; the
parent's gate is the authority.
