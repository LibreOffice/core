# slides-r62 — prediction

Committed before anything is built or rendered post-change. Base `337bc9fe17c`, branch
`wt-slides-r62`, LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`, `TMPDIR` on the host mount.

## Baseline reproduced first

| | briefed | measured at `337bc9fe17c` |
|---|---|---|
| passing over `MANIFEST.tsv` | 200 of 302 | **200 of 302, 0 disagreements, 302 of 302 visited** |
| `abs_ink` | 990.13 (round 60's final) | **990.51** |
| major pages | 364 | **364** |

The gate reproduces exactly. `abs_ink` is 0.04% above round 60's final figure and **six documents
account for all of it** — `3495` +1.41, `031_Alarm_Clock_Pie-Chart` −1.25, `171128IPAP` +0.26,
`Intersil…` −0.14, `3492` +0.08, `Demick_JetBlue` +0.02 — which are the merged rounds 61 sheets
(the pie first pass) and 61 words landing on this track's charts, not a measurement fault. It is
recorded rather than chased, per round 60's own precedent.

## The change

### §1 The legend's 2.70 pt is a **typeface**, not legend arithmetic

The brief's item 1 said the legend box is 2.70 pt too wide on its right and named
`legend-census.py` as the instrument. The instrument was right about *where* and wrong about
*what*. Two independent readings of `001_advanced_powerpoint_bar__pptx` page 1:

* **The PDF font resources.** The reference sets its two legend entries in
  `CAAAAA+Carlito-Regular` at 10.005 pt; we set ours in `CAAAAA+LiberationSans` at 10.006 pt.
  The page's other 17 ten-point runs — the axis and category labels — are LiberationSans on
  *both* sides. So the two stacks disagree about the legend's face and about nothing else on the
  page.
* **The drawn extents.** `pdftotext -bbox`: the widest entry, "Actual", is
  `603.723 … 631.533` on our side and `606.415 … 631.538` on the reference's. **The right edges
  agree to 0.005 pt** — which is structural, because the widest entry's right edge is
  `frame.Right − LegendMarginX − paddingX` in both stacks whatever the text measures — and the
  whole difference, 2.692 pt, is the entry's own width. Carlito is that much narrower than
  Liberation Sans at 10 pt.

The seat is `DrawingChartPlot.FamilyOf`: `c:chartSpace/c:txPr`'s literal `a:latin`, then **the
first literal `a:latin` anywhere in the part**, then the theme's minor face. This deck's chart
space states no `c:txPr`; its `c:catAx/c:txPr` and `c:valAx/c:txPr` both state
`<a:latin typeface="Arial"/>`; its `c:legend` states nothing at all. The second term hands the
axes' Arial to the legend. The reference resolves each object separately —
`ObjectFormatter`'s auto-text table names `XML_minor` for every automatic entry
(`objectformatter.cxx:415-434`) and a stated `c:txPr` overrides for that object only — so the
legend takes the theme's Calibri → Carlito.

`ChartPlot.LegendFamily`, nullable, read as **`c:legend/c:txPr` → `c:chartSpace/c:txPr` →
theme minor**, and used by `ChartLayout.Legend`, `AddLegend` and the labels they emit. Null keeps
today's answer, so ODF and BIFF charts cannot move.

### §1b The legend's entry **order**, found beside it

`001_advanced_powerpoint_bar` lists *Plan* above *Actual* in the reference and *Actual* above
*Plan* in ours; `002_advanced_powerpoint_column` and `006_advanced_powerpoint_area`, same deck
family, same two series, list *Actual* above *Plan* on **both** sides.
`VSeriesPlotter::createLegendEntries` (`VSeriesPlotter.cxx:2432-2447`) is explicit: with the
coordinate system swapped — a horizontal bar chart — the entries reverse unless the series stack
in Y; otherwise, and only for a legend at the line start or line end, they reverse when the
series *do* stack in Y. Confirmed on the second arm too: `stacked_bar_chart.pptx` and
`stacked_area_chart.pptx` (stacked, legend right) list *In-Store Sales* above *Online Sales* in
the reference and the other way round in ours.

Four arms measured on the binary, two of them controls that must not move.

## Documents I expect to change

`legendfamily-census.py` over every OOXML chart part in the corpus that draws a legend:
**69 chart parts in 69 documents — 33 `.pptx`, 36 `.xlsx`, 0 `.docx`.** Every slides one is an
`advanced_powerpoint_*` deck and every one of them goes **Arial → Calibri**.

`legendorder-census.py`: **17 documents** — 8 slides
(`001/009/017/025/033_advanced_powerpoint_bar`, `stacked_bar_chart`, `stacked_area_chart`,
`southern-classic-kennesaw-state-university-final` (two parts)) and 9 sheets.

Union on slides: **36 documents** (the five bars are in both lists).

## The numbers

1. **Verdict movement: 0**, band −1 … +1. No page count can move and no failing slides document
   fails on a chart page's word count.
2. **Page counts changed: 0 of 302.**
3. **Documents moved on differing pixels: 36**, band 33 … 39.
4. **`abs_ink`: −1 … −10.** Ranked, not decided on — refuted three rounds running for
   chart-geometry work. The decision column is differing pixels and the plot-rect deltas.
5. **Differing pixels: −20 … −140.**
6. **Plot-rect census, `dRight` over 1 pt: 27 → 8 … 14.** Seventeen of the twenty-seven sit at
   −2.71/−2.73/−2.88 and every one of them is an `advanced_powerpoint` bar/column/line/area page
   whose legend this change re-measures.
7. **`dLeft` over 1 pt: 9 → 9 ± 1.** The legend is on the right; nothing here touches the left
   reservation.
8. Controls unchanged: sheared glyphs 15792, exact `/Tf` pages 1708 of 4515,
   `tf-agreement` 0.77065.
9. **The face census** (`facediff.py`, multiset difference of (face, size) over every text run of
   every slides chart page): the 17 pages currently reading exactly
   `+LiberationSans@10.01×2 | −Carlito-Regular@10.01×2` go to **0**, and the pie/doughnut/scatter/
   bubble pages lose their legend term while keeping whatever else they have.

## What this census cannot see

- **Whether the reference resolves the *other* four roles the same way.** The census says an axis
  label's face differs from the one-face answer on 45 documents and a data label's on 46, but on
  slides those are all pie and doughnut decks that draw **no axis labels at all**, and the
  data-label column is known under-reaching — it reads `c:plotArea/c:dLbls` and a type group's,
  not `c:ser/c:dLbls`, which is where these decks put theirs. **Only the legend is implemented**,
  because only the legend has a rendering measurement behind it.
- **The other two tracks.** `XRay.Office.Core/Charts` and `XRay.Office.Ooxml` are shared. 36 `.xlsx`
  documents are named above for the legend face and 9 for the legend order; **words: none**, and
  no `.docx` in the corpus has a chart part with a legend that either census flags. The parent
  gates the corpus.
- **Whether a face that is *stated* on the legend resolves the way we resolve it.** No corpus
  chart states `c:legend/c:txPr/a:latin` with a literal face, so that arm of the precedence is
  exercised only by a unit test.
- **The knock-on.** Moving the plot's right edge by 2.70 pt moves every mark inside the plot, so
  a page's ink can move much more than the legend's own two labels suggest — and on a bar chart
  the bars are drawn from the left edge, so their *lengths* change.
- **Whether reversing the legend changes the legend's own width.** Each column takes its own
  widest entry, so on a one-column legend it cannot; on a two-column one it can, and the corpus
  has none among the 17.

## What the prediction is weakest on

**Documents-moved has missed six rounds running, always upward, and round 60 named the mechanism
and still got the item wrong.** This band is built from two censuses that read the element the
change is gated on, so a miss upward again means a *third* item arrived after this file was
written. The three candidates are named here in advance so that the miss can be attributed rather
than re-fitted: **Pavese's gradient bars** (brief item 2), **the rotated label's anchor** (brief
item 3, which explicitly needs an instrument built before the hypothesis is believed), and **the
24.2.7.2 audit**, which outweighed the plan in two of the last four rounds. If any of those
ships, this band is void and I will say so rather than re-fit it.
