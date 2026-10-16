# slides-f-01 — prediction, committed before any census, sweep or pixel comparison

Written after reading only: `probes/slides-e-01/results.md`, the `TODO.batches.md` merge note,
and the four source sites the brief names (`DrawingChartPlot.cs:798/813/1423-1428`,
`ChartLayout.cs:2080-2089`, `ChartLayout.Plots.cs:118-134`,
`DrawingChartAutoFormat.cs:150-205`) plus `typegroupconverter.cxx:626-682`. **No census has been
run, nothing has been rendered, no test has been executed.** Base `4f0fd5fde66`, branch
`wt-slides-f`. `check-env.sh` green on all five sections (soffice 26.2.4.2 620(Build:2),
Carlito/Caladea/Liberation/DejaVu, pdftoppm 26.01.0, pdftotext 26.01.0).

## The briefed claims, as they look after reading the code and before measuring

**C1 — `LineOf` turns a stated `a:noFill` into "states nothing", and `:798`'s `?? autoLine` then
draws the line the file suppressed.** The first half is exactly true: `DrawingChartPlot.cs:1427`
returns `null` for `a:noFill`, indistinguishable from `:1425`'s `null` for "no `a:ln` at all", and
`:798` is `LineOf(properties, theme) ?? autoLine`.

**The second half I predict is wrong, and this is the prediction I most expect to be scored
against.** `:813-814` already sets `HasLine = scatterLine && …noFill is null`, and
`ChartLayout.cs:2082` draws the polyline only `if (series.HasLine …)`. So for a *line or scatter*
series the suppressed line is **not** drawn; what actually happens is that `ChartSeries.Line`
carries a colour the file denied, and that colour leaks into the consumers that do **not** consult
`HasLine`:

- `ChartLayout.cs:2089` — the marker's fill, `series.Fill ?? stroke` (the FAAAI case);
- `ChartLayout.Plots.cs:123-124, 130-131` — a radar's closed stroke and its markers, no `HasLine`
  anywhere in that method;
- the frame-series border at `Plots.cs:341, 637, 652, 683, 729` and `ChartLayout.cs:2375, 2461` —
  but only where `spFilledSeriesLines` is not `Invisible`, i.e. chart style 9–16 or 33–40;
- the legend key at `ChartLayout.cs:3064` and the trendline fallback at `:992`.

So I predict the defect is real and its **mechanism is misdescribed in the brief**: it is a leaked
colour, not a drawn line, on every line and scatter series in the corpus.

**C2 — a marker's `c:marker/c:spPr` is never read (`ChartLayout.cs:2089`).** I predict this holds
verbatim: `MarkerOf` (`DrawingChartPlot.cs:935-964`) reads `c:marker/c:symbol` and nothing else,
`ChartSeries` has no marker-paint member at all, and both marker painters derive their colours from
`series.Fill`/`series.Line`. LibreOffice's `TypeGroupConverter::convertMarker`
(`typegroupconverter.cxx:626-682`) takes `xShapeProps->getFillProperties().maFillColor` and, when
there is none, falls back to the marker's **line** colour (tdf#124817).

**C3 — fixing C1 without C2 makes it worse.** I predict this is true and now has a precise
mechanism: for `FAAAI…`, `ColourOf(LinearSeries, stroke: false)` returns `null` by construction
(`DrawingChartAutoFormat.cs:191` — the linear fill table is `[]`), so `series.Fill` is null and the
marker is painted from `stroke = series.Line ?? series.Fill ?? Colour.Black`. Null the `Line` and
the eight markers go **black**, not merely wrong by 4/255.

## The numbers

| # | prediction |
|---|---|
| P1 | Census, `c:ser/c:spPr/a:ln/a:noFill` **counted directly** rather than inferred: **4–12 series across 2–5 decks** of the 163. I expect it to be small and I expect at least one deck other than `FAAAI…`. |
| P2 | Census, a series stating `c:marker/c:spPr` carrying a fill or a line: **1–4 decks**. |
| P3 | Changed renderings across the 163: **1–4**. `FAAAI…` certainly; the rest are whichever census hits have a marker, a radar, or a filled series at style 9–16/33–40. |
| P4 | `FAAAI…` page 7's 8 scatter markers return to **`#850F89`**, the reference's own value, and the count of `#850F89` records on the page rises by 8. Direction: **1 page closer, 0 further** on that deck. |
| P5 | Verdicts moved: **0 of 163**. Slides stays 144 of 163 and 163 of 163 page-exact. A marker fill cannot move a page count and cannot move a word count; I say this plainly rather than hedging. |
| P6 | Cross-track `XRay.Office.Ooxml`: words **0 of 200**, sheets **0 of 171** — but *not* by the same construction slides-e used. My change is in `DrawingChartPlot`'s series reader and in `XRay.Office.Core`'s marker painter, neither of which is gated on the format matrix, so the e-01 call-site argument does not carry over and I will re-measure both single chart-bearing documents on both legs rather than inherit the zero. |
| P7 | `pdf-image-diff` on `FAAAI…` page 7 reads **the same value on both legs** again, to two decimals. Eight 6.3 pt discs at a 512 px raster is below its resolution. The measurement is the operator census; the pixel metric is reported as blind, not as agreement. |
| C1' | Control: a deck with charts but no `a:noFill` series and no marker `spPr` — `Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx`, e-01's control — must not change. |

## What the census cannot see, named in advance

1. **The census counts declarations; the renderer resolves.** A `c:ser` with `a:noFill` on a
   *filled* series at style 1–8 changes nothing, because `FilledSeriesLines` is `Invisible` there
   and `autoLine` is already null — so the census ceiling will be **above** the measured reach, and
   I expect the gap to be mostly filled series at the default style 2.
2. **`c:dPt/c:marker/c:spPr` and `c:dPt/c:spPr/a:ln/a:noFill`.** A per-point marker override is
   the same defect one level down, and `PointFills` has its own colour path. I will count them
   but I do **not** plan to implement the per-point marker in this round; if the census shows them
   the ceiling is understated by that amount and I will say so.
3. **ODF.** `ChartLayout.cs:2089` is in `XRay.Office.Core`, so it is shared with the ODF chart
   reader, which has no `c:marker/c:spPr` to give it. I intend the fallback expression to stay
   byte-identical when the new member is null so ODP/ODS/ODT reach is zero *by construction* — but
   that is an intention, and the sweep of all 534 is what will test it.
4. **A theme's own line colour standing behind a marker.** `convertMarker` reads the marker's
   `spPr` colour raw; it does **not** put it through the subtle line style the way e-01's stroke
   goes. If I am wrong about that, `FAAAI…`'s markers will land on a shaded `#850F89` rather than
   on `#850F89`, and the operator census will say so immediately.
5. **`c:marker` on `c:dLbls`-bearing line series in `Demick_JetBlue`.** That deck is the one with
   16 automatic line series; if any of them states a marker `spPr`, this round moves a deck e-01
   already moved, and the two changes will have to be separated by leg rather than by deck.

## If the coupled pair closes early

`c:minorGridlines` at `DrawingChartPlot.cs:374`, reach already measured at 3 decks / 12 instances.
I predict I will **not** reach it, because the marker change needs a `XRay.Office.Core` member and
therefore the full 534-document cross-track sweep, and that sweep is the expensive half of the
round.
