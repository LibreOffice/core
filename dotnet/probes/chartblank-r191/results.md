# A chart label's trailing blank is part of its width

Measured 2026-09-28 against `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** —
with the tarball's five bundled font confounds moved aside as `dotnet/CLAUDE.md` describes.
Corpus at `/home/user/sample-files`; tree at `/home/user/libreoffice-core`.

## 0. Where the question came from

`Demick_JetBlue.pptx` page 5 was the open chart seat at 20.31 `diff%`, and a blind reading of the
pair (`page-vision`, one subagent, no repo access) reported the reference's plot rectangle as
*"inset relative to ours — 6 px further right on the left edge and 6 px higher at the bottom"*,
with the y tick labels *"roughly 5 % wider"* at the same cap height. It also reported the rotated
category labels as text in both halves, which they are not in the reference — that is the shear
ceiling — so the reading is taken for **direction and kind** only, as the skill says.

The instrument that turned it into a number is `pdf-ops.py dump --only stroke`, grouped by colour:
a chart's plot rectangle is the extent of its major gridlines, and they come out exactly.

| | ours | 26.2.4.2 |
|---|---|---|
| plot x | 161.99 … 662.93 | **165.71** … 661.95 |
| plot y | 134.38 … 324.96 | 138.44 … 325.42 |
| widest label's ink | 87.79 … 159.33 | 88.69 … 159.80 |
| ink right → plot left | **2.66** | **5.91** |

The *ink* agrees to half a point in position and to 0.6 % in width — which is the chart device's
own scale, ours 10.01 pt against the reference's 9.89 — so nothing is measured wrongly. What
differs is how much room is left between the last glyph and the plot: 3.25 pt of it.

## 1. The variant that settles it

`variants.py` rewrites one token of `ppt/charts/chart2.xml`'s `formatCode` and nothing else. The
authored code is the US accounting format Excel writes:

```
_("$"* #,##0.00_);_("$"* \(#,##0.00\);_("$"* "-"??_);_(@_)
```

`plotrect.py` reads each rendering's plot rectangle and its widest label's ink box.

| rendering | plot left | ink right | gap |
|---|---:|---:|---:|
| ours, base | 161.99 | 159.33 | **2.66** |
| ours, after | 165.09 | 159.33 | **5.76** |
| 26.2.4.2, as authored | 165.71 | 159.80 | **5.91** |
| 26.2.4.2, `_)` deleted | **162.65** | 159.80 | **2.85** |
| 26.2.4.2, `_(` deleted | 162.65 | 156.73 | 5.92 |
| 26.2.4.2, both deleted | 159.53 | 156.73 | 2.80 |
| 26.2.4.2, the zero row's `??` deleted | 165.71 | 159.80 | 5.91 |

Read it as three statements.

- **The base tree renders the reference's `_)`-deleted answer.** 2.66 against 2.85, where the
  authored reference is 5.91. So the trailing blank is the whole of the gap.
- **Both ends count and one blank is 3.06 pt here.** Deleting `_(` moves the plot edge by the
  same amount as deleting `_)`, and deleting both moves it by twice. The leading one was already
  in our width; only the trailing one was not.
- **The `??` control does not move.** The zero row is `$-` and is not the widest label, so its two
  blank digit placeholders decide nothing on this axis — which is what makes the `_)` row a
  measurement of the blank rather than of the format.

**A trap in the first cut of the variant:** replacing `_)` with `)` leaves a literal closing
parenthesis behind, so `$1,200,000.00)` is drawn and the plot edge moves the wrong way. Delete
both characters.

## 2. The seat, and the two measurers that do not have it

Only the slides path is affected, and the reason is that it is the only one of the three that
measures a chart label through a *line layout* rather than by shaping the string:

| measurer | how it gets the width | trailing blank |
|---|---|---|
| `SlideChart.Measurer` | sums the advances `SlideTextLayout.Place` emitted | **lost** |
| `SheetBandText.ChartShape` | `TextShaper.Default.Shape(face, text)`, every advance | kept |
| `FrameChart.ChartFace.Shape` | the same | kept |

`SlideTextLayout.Emit` runs to `TextLine.VisibleEnd`, and `VisibleEnd` exists precisely to keep a
line's trailing blanks out of its width — they hang past the right edge rather than pushing a word
onto the next line. That is right for a wrapped body and wrong for a chart label, which is an
EditEngine text shape autogrown around the paragraph whole.

**This is not the rule round 190 refuted.** *"A slide line break charges its trailing blank"* was
about where a line **breaks**, was implemented, and regressed 109 documents against 16; it stays
refuted. This is the width of a paragraph that does not break at all, and it is measured at the
reference rather than inferred from a `cx` sweep.

The fix adds the trailing blanks back in `SlideChart.Measurer.Measure`, measuring them where they
are not trailing — a digit after them, the digit taken back off.

## 3. Confinement and cost

`sweep.py` renders one document per directory under `SOURCE_DATE_EPOCH=0`, three workers, at the
round's base and with the change. The 67 corpus decks holding a `ppt/charts/chart*.xml` part:

**4 of 67 renderings move and 63 are byte-identical.**

`score.py` scores each mover against a freshly rendered 26.2.4.2 with `pdf-image-diff.py`:

| document | worst `diff%` | summed `diff%` | MAJOR |
|---|---|---|---|
| `Demick_JetBlue` | 20.31 → **19.03** | 86.78 → **82.14** | 2 → 2 |
| `southern-classic-kennesaw-state-university-final` | 7.64 → 7.64 | 75.64 → **74.94** | 4 → 4 |
| `Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack` | 15.93 → 15.93 | 139.85 → **139.74** | 2 → 2 |
| `171128IPAP` | 12.91 → 12.91 | 250.84 → 250.84 | 1 → 1 |

Three improve, one is level in pixels although its bytes moved, **none worsens**. On Demick four
pages improve — 5 (20.31 → 19.03), 6 (8.84 → 8.03), 7 (11.51 → 9.91), 8 (9.89 → 8.94) — because
all four carry a chart on the same accounting axis.

**No gate column can see it, and that is checked rather than asserted.** Alphanumeric characters
and page counts over the four movers: 3519/10, 10979/23, 23961/24, 25455/40 — identical before and
after. A blank is not an alphanumeric character.

## 4. The fixture, and what is still open on that page

`dotnet/tests/corpus/features/slide-chart-accounting-axis.pptx` is `chart-bar-deck.pptx` with the
accounting code on its value axis and nothing else changed, so the only difference from a deck
`SlideChartDrawingTests` already asserts exactly is the format. Its grey wall:

| | wall left |
|---|---:|
| ours, base | 128.70 |
| ours, after | **131.41** |
| 26.2.4.2 | **131.41** |

Exact, with no free parameter. `SlideChartTrailingBlankTests` pins it, and fails on the base source.

**What is left on `Demick_JetBlue` page 5, measured and not taken:**

- The chart's text is drawn at 10.01 pt where the reference draws 9.89 with a horizontal scale of
  9.92 — the anisotropic fit squeezing a chart whose labels overflow. It is 0.6 % of every advance
  and 0.62 pt of the residual plot-left difference.
- The plot is 190.58 pt tall against the reference's 186.98, with the top edges 0.46 pt apart, so
  the reference gives up 4.06 pt more below the plot. The category axis is interior here — the
  `$-` gridline — and `PlotAreaOf` reserves **nothing** for an interior axis' labels, which is
  right as far as it goes: `VDiagram::adjustInnerSize` then shrinks the inner rectangle by how far
  the drawn labels *overflow* the available one, and we do not. Our rotated labels also hang about
  3.3 pt further below the axis line than the reference's, so the two have to be separated before
  either is worth implementing.
- The reference's legend sits 4.20 pt lower and its rotated axis title 4.09 pt further right.
- 26.2.4.2 outlines that page's 26 turned category labels — 156 glyph-sized black fills, no text
  records at all — so part of the page is the shear ceiling and cannot be closed.

## 5. And the tile question this round opened and closed

The same page's theme fills a 5 × 5 px bitmap with `<a:tile sx="65000" sy="65000"/>` about 37 000
times; our pitch is 3.25 pt and the reference's ≈ 3.2325, so the phase drifts across the page. It
is **not** what the 20.31 was: the plot rectangle is, and the tiled ground is the part the blind
reader called *"the closest match on the page"*. Censused for whoever takes it —
**91 documents, 400 `a:tile` elements**, by part `theme` 171, `slides` 160, `document.xml` 41,
`slideLayouts` 16, `slideMasters` 8, `charts` 4 — with the heaviest at
`048_Visual_Product_Roadmap_Template_Quality_Layout` (26) and
`082_Infographic_Funnel_with_4_Stages_for_PowerPoint` (24). `Demick_JetBlue` is not in the top
fifteen.
