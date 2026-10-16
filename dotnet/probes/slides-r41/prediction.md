# Slides round 41 — reach and verdict prediction, committed before the sweep

Base `e5f54617c`, reproduced: 151/163 match, 0 `unembedded`, `|ink|%` **1448.71**,
signed `ink%` 1132.20, 392 major pages.

Three changes are in the tree at the time of writing. This is what I expect the whole-track
sweep to say about them. Written before rendering anything but the three review documents.

## The census ceilings, and what each counted over

Every figure below is a count of documents that **state** a property, over the formats the
census can read. The slides track is 163 documents, of which 114 are zip containers and 51
are `.ppt`; the chart censuses read only the zip half, because a `.ppt` has no
`ppt/charts/chart1.xml`. The EMF census reads both — it inflates every deflate stream of a
binary container, because a `.ppt` keeps its blips zlib-wrapped inside Escher records.

| Property | Documents | Over |
|---|---:|---|
| an EMF with `EMR_POLYBEZIERTO` inside `BeginPath`/`EndPath` | **15** | all 163 |
| a chart series stating no `c:spPr` at all | **4** | the 15 decks with chart parts |
| a chart space stating a visible `a:ln` | **6** | the same 15 |
| a chart series with an `a:gradFill` | **4** | the same 15 |
| union of the three chart censuses | **11** | the same 15 |

## The prediction

| | predicted |
|---|---|
| renderings byte-changed | **18–26 of 163** |
| verdicts moved | **0** |
| signed `ink%` | 1132.20 → **1116–1124** |
| `\|ink\|%` | 1448.71 → **1430–1441** |
| major pages | 392 → **388–392** |

Reasoning, so that a wrong number says which half was wrong:

- **Byte reach** should be near the union of the ceilings (15 + 11 = 26 minus one overlap,
  `16 - UTM` and `8_P-Pavese` and `Intersil` appearing in both lists) rather than far below
  it, because unlike round 39's theme-gradient census these count what a *drawn* object
  states rather than what a part declares. A chart part exists to be drawn; a theme fill
  style mostly is not. The one place I expect over-count is
  `redac-sas-201403-ppt-portfolio-rev-sim.pptx`, whose whole EMF corpus holds **two**
  `BezierTo` records — one figure of one record draws identically either way.
- **Zero verdicts.** None of the three touches page count, extractable words or font
  embedding. A ligature drawn as a glyph instead of as a filled blot draws the *same*
  characters into the PDF either way, because the blot was a path and the text layer was
  already correct.
- **Ink.** Measured individually before the sweep, on signed `ink%`:
  `Demick_JetBlue` 35.97 → 29.13, `16 - UTM - (NASA)` 20.40 → 19.95,
  `N2_E_Maestroni_Swarm_COP` 2.36 → 1.72. That is −7.93 from three documents; the remaining
  twenty are smaller instances of the same three mechanisms.

## What I expect *not* to move, and why it is worth saying

`Demick_JetBlue` will not reach nought. Two differences on its page 4 are untouched:
LibreOffice draws **minor gridlines** on both axes (`c:minorGridlines`, which nothing in the
tree reads) and labels **all 21 categories** where we label eleven. Neither is in scope here
and both are recorded as open.
