# slides-r60 — prediction

Committed before anything is built or rendered post-change. Base `2870991a4dd`, branch
`wt-slides-r60`, LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## Baseline reproduced first

| | briefed | measured at `2870991a4dd` |
|---|---|---|
| passing over `MANIFEST.tsv` | 200 of 302 | **200 of 302, 0 disagreements** |
| `abs_ink` | 1039.95 | **1036.75** |
| major pages | 375 | **365** |
| differing pixels | — | **19402.35 over 4530 pages** |

The gate reproduces exactly. `abs_ink` is 0.31% below the briefed figure and major pages ten
below; the reference renderings were reused from round 59's sweep and were checked to be
ink-identical to a freshly rendered pair (`Intersil…`, `3495` — same `|ink|%` to the hundredth
on all 39 and 26 pages), and `git status` shows nothing modified under `dotnet/`. The residue is
between the parent's whole-corpus gate and this track sweep and is recorded, not chased.

## The change

The brief's item 1 — the plot rectangle's **right edge**, unmoved on 31 of 57 chart pages. The
mechanism found before writing this file is **not** a right-edge reservation at all: it is
`c:crossBetween`. A nine-arm probe (three chart types x between / midCat / element deleted),
one property patched per arm, rendered through 26.2.4.2 and read back from the label pen
positions, says the running binary decides *shifted categories* like this:

| | `between` | `midCat` | absent |
|---|---|---|---|
| `c:areaChart` | shifted | unshifted | unshifted |
| `c:lineChart` | shifted | unshifted | **shifted** |
| `c:barChart` (column) | shifted | **shifted** | shifted |

`ChartPlot.ShiftedCategories` reads the chart *type* only — bar and stock — so every OOXML line
and area chart stating `c:crossBetween="between"` is drawn one half-slot wrong, its category
labels centred on the plot's own corners instead of in the middle of each slot, and its plot
rectangle then shrunk on the right by half the last label to make room for an overhang the
reference does not have.

## Documents I expect to change, from `crossbetween-census.py` over the whole corpus

Thirteen slides documents, 28 chart parts:

| document | parts | now |
|---|---:|---|
| `southern-classic-kennesaw-state-university-final.pptx` | 8 | `words`, ink 11.65 |
| `Demick_JetBlue.pptx` | 5 (3 with two `c:valAx`) | `words`, ink 5.65 |
| `171128IPAP.pptx` | 4 of 5 (`chart7` states `midCat` and must not move) | `match`, ink 12.02 |
| `1_Country-Updates_DRC_English.pptx` | 2 | `match`, ink 1.16 |
| `003/011/019/027_advanced_powerpoint_line.pptx` | 1 each | `match`, ink ≈0.35 |
| `006/014/022/030_advanced_powerpoint_area.pptx` | 1 each | `match`, ink ≈0.15 |
| `line_chart.pptx` | 1 | `match`, ink 0.33 |

Two controls that must **not** move: `stacked_area_chart.pptx` (`areaChart` + `midCat`, already
unshifted) and `combo_bar_line_chart.pptx` (`barChart+lineChart`, the bar override wins).

## The numbers

1. **Verdict movement: 0**, band −1 … +1. Neither failing document fails on a chart page's word
   count as far as I know, and no page count can move.
2. **Page counts changed: 0 of 302.**
3. **Documents moved on differing pixels: 13**, band 11 … 16. It is 13 and not more because the
   change is gated on an element this census reads directly.
4. **`abs_ink` −2 … −10.** Ranked, not decided on: round 59 refuted this column for
   chart-geometry work. The decision column is differing pixels.
5. **Differing pixels −40 … −250.**
6. **Plot-rect census, `dRight` over 1 pt on the 57 chart pages: 31 → 20 … 24.** The eight
   `advanced_powerpoint_line/area` pages at −9.66/−9.68 and `line_chart` at −7.77 should go to
   near zero; `1_Country-Updates_DRC_English` p14 (−15.22) and `Demick_JetBlue` p6 (−11.75) are
   line charts and should improve but I do not claim they reach the threshold.
7. **`dLeft` over 1 pt: 9 → 9 ± 2.** The same `EndLabelOverhang` term feeds the left edge and
   will stop firing on the same pages, so the left edge moves too — in the same direction.
8. Controls unchanged: sheared glyphs 15792, `tf-agreement` 0.77063 (**this base's reading**, see
   round 59 — round 56's 0.85188 remains unexplained), exact `/Tf` pages 1708 of 4515.

## What this census cannot see

- **ODF charts.** `chart:` has no `c:crossBetween` and the change is written so that an ODF plot
  keeps today's answer exactly. Whether 26.2.4.2 agrees for a *native* line chart is **not
  measured here** — `ChartTypeTemplate::adaptScales` shifts only Column, Bar and Close, which is
  today's behaviour, so the untested assumption is that the ODF path is already right.
- **Binary `.ppt` decks.** A chart in a `.ppt` is an OLE object and does not reach
  `DrawingChartPlot`, so no `.ppt` appears in this census even if it draws a line chart.
- **Combination charts whose *first* type group is the line.** The reference reads
  `rTypeGroups.front()`; this reader scans every series for a bar. The corpus has one slides
  combo (`combo_bar_line_chart`, bar first) and the sheets combos are unchecked for order.
- **The knock-on onto data labels and markers.** Shifting the categories moves every plotted
  point by half a slot, so a page's ink can move far more than the plot rectangle alone suggests
  — and on a stacked area chart it changes the shape of the filled region, not only its position.
- **The other two tracks.** `XRay.Office.Core/Charts` and `XRay.Office.Ooxml` are shared. The census
  says **sheets: 12 `c:lineChart` parts stating `between`**, words: **none**. The parent gates
  the corpus.

## What the prediction is weakest on

Documents-moved has missed four rounds running, always upward, always because a second item
arrived after the prediction was written. This one names 13 documents from an element the census
reads directly, so if it misses upward the cause will again be a *second* item — most likely the
24.2.7.2 audit, which has outweighed the briefed target for two consecutive rounds. **If the
audit produces a fix, this band is void and I will say so rather than re-fit it.**
