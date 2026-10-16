# slides-r60 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
base `2870991a4dd`, branch `wt-slides-r60`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.
`prediction.md` beside this file was committed as `3a5638b5953`, before anything was built or
rendered post-change.

## The baseline reproduces on the gate and is 0.31% under it on ink

| | briefed | measured at `2870991a4dd` |
|---|---|---|
| passing over `MANIFEST.tsv` | 200 of 302 | **200 of 302, 0 disagreements** |
| `abs_ink` | 1039.95 | **1036.75** |
| major pages | 375 | **365** |
| differing pixels | — | **19402.35 over 4530 pages** |

The gate reproduces exactly, which is the non-negotiable. The ink residue was chased far enough
to say what it is *not*: `git status` shows nothing modified under `dotnet/`, and the reference
renderings reused from round 59 were checked against freshly rendered ones — `Intersil…` and
`3495` come out with the *same* `|ink|%` on every one of their 39 and 26 pages, so reference
reuse is not the cause. It is a difference between the parent's whole-corpus gate and this track
sweep, and it is recorded rather than explained away.

## The whole round

| | base | §1 | §1+§2 | **final (+§3)** |
|---|---:|---:|---:|---:|
| passing over `MANIFEST.tsv` | **200 of 302** | 200 | 200 | **200 of 302** |
| page counts changed | | | | **0 of 302** |
| `abs_ink` | 1036.75 | 1041.20 | 995.08 | **990.13 (−46.62)** |
| signed ink | 739.27 | | 698.58 | **693.61** |
| major pages | 365 | 369 | 366 | **364** |
| **differing pixels over 4530 pages** | 19402.35 | 19366.62 | 19312.41 | **19240.53 (−161.82)** |

**Twenty-six documents moved on differing pixels — 24 improved, 2 worsened — and 26 on unsigned
ink, 22 improved and 4 worsened. The regressions are named, not netted.**

| Δ differing pixels | document | before → after |
|---:|---|---|
| **−48.50** | `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 154.49 → **105.99**, ink 47.26 → **3.92**, major 1 → **0** |
| **−20.79 … −20.27** | `006/014/022/030_advanced_powerpoint_area.pptx` | ≈21.4 → **≈0.86** each |
| −8.78 | `171128IPAP.pptx` | 277.00 → 268.22 |
| −5.19 | `1_Country-Updates_DRC_English.pptx` | 34.09 → 28.90 |
| −4.64 | `Demick_JetBlue.pptx` | 121.60 → 116.96 |
| −3.10 | `line_chart.pptx` | 5.12 → 2.02 |
| −2.42 | `southern-classic-kennesaw-state-university-final.pptx` | 93.43 → 91.01 |
| −1.12 | `combo_bar_line_chart.pptx` | 4.04 → 2.92 |
| −0.81 … −0.74 | four `*_advanced_powerpoint_line.pptx` | ≈1.6 → ≈0.8 each |
| −0.73 … −0.03 | `stacked_area_chart`, `stacked_bar_chart`, `bar_chart`, `scatter_chart`, `3495`, `FAAAI…`, `Intersil…`, `bitesize-writing-a-report`, `flying-by-numbers` | |
| **+0.20** | `035_Chemistry_Column_PowerPoint_Chart_45bf8a76.pptx` | 8.92 → **9.12** |
| **+0.11** | `038_Competitive_Advantage_Card…pptx` | 11.19 → **11.30** |

**The two documents that worsened on differing pixels improved on ink and on major pages** —
1.74 → 1.37 with major 1 → 0, and 1.38 → 1.28 with major 2 → 1. Both state
`a:schemeClr val="bg1"` on their data labels; five of `035`'s runs moved from black to
`#D9D9D9`, which is the colour the reference draws 37 of its 60 runs in, and its page 1 improved
on both columns while page 2 gained 0.29 of differing pixels and lost 0.26 of ink.

The four documents that worsened on **ink**:

| Δ `abs_ink` | document | |
|---:|---|---|
| **+1.21** | `Demick_JetBlue.pptx` | 5.65 → 6.86, major 1 → 2 — and **every one of its ten pages improved or held on differing pixels** (4: 18.98 → 17.62, 5: 20.73 → 19.65, 6: 14.96 → 13.86, 7: 20.49 → 20.11, 8: 18.53 → 17.81). Page 7 crosses the major threshold on ink while its differing pixels fall. |
| +0.19 | `3495.pptx` | 8.49 → 8.68 |
| +0.12 | `FAAAIandtheArtandScienceofV&Vfinal.pptx` | 4.32 → 4.44 |
| +0.01 | `flying-by-numbers-presentation.pptx` | 3.96 → 3.97 |

Major pages went 365 → 364 net, and the composition is worth a line: Pavese's page 8 and
`035`'s and `038`'s go, `Demick_JetBlue` page 7 and `171128IPAP` page 32 arrive. The latter is
`diff% 2.94, |ink|% 0.15` — a page the tool calls major on a region count rather than on either
magnitude.

## 1. The brief's item 1 is not a right-edge reservation. It is `c:crossBetween`

The brief asked for the plot rectangle's right edge, unmoved on 31 of 57 chart pages. Reading the
census' own tightest cluster first — four `advanced_powerpoint_area` and four
`advanced_powerpoint_line` pages at `dRight −9.68 / −9.66`, against four
`advanced_powerpoint_column` pages at `−2.73` with an *identical* reference rectangle — put the
question one level down: **why do an area chart and a column chart with the same frame, the same
legend and the same eight categories give up different amounts of right edge?**

Because `ChartPlot.ShiftedCategories` reads the chart *type* and nothing else. It is `Bar` or
`Stock`, so an area or line chart is drawn unshifted — its eight categories as eight points, the
first on the plot's left edge and the last on its right — and its last label then overhangs, and
`EndLabelOverhang` takes half of it off the right edge to make room. The reference does no such
thing, because it does not draw those categories unshifted at all.

### The probe, nine arms, one property each

`make-crossbetween-probe.py` patches `c:crossBetween` — and only that — in three corpus decks,
three arms each (`between` / `midCat` / element deleted), rendered through 26.2.4.2 and read back
by `cbread.py` from the category labels' own pen positions. The ratio of the label span to the
plot width is `(n−1)/n` when the categories are slots and 1 when they are points, and the label
width cancels because every label in those decks is the same width.

| | `between` | `midCat` | absent |
|---|---|---|---|
| `c:areaChart` | shifted 0.8750 | unshifted 1.0000 | unshifted 1.0000 |
| `c:lineChart` | shifted 0.8751 | unshifted 1.0000 | **shifted 0.8751** |
| `c:barChart` (column) | shifted 0.8750 | **shifted 0.8750** | shifted 0.8750 |

Nine of nine, and two of them are the ones worth having.

* **`lineChart` with the element absent is shifted where an area chart is not.** That is
  `oox/source/drawingml/chart/axisconverter.cxx:300-301`'s fall-back, which names `TYPEID_LINE`
  alongside the bar category and leaves everything else on the axis' created default.
* **`barChart` with `midCat` is shifted anyway**, and its rendering is byte-identical to its own
  `between` arm. So a bar or column chart ignores the element outright.

### Where the source and the running binary disagree, and the binary wins

The 27.2 tree in this checkout reads `c:crossBetween` *ahead* of the chart type for everything but
a 3-D bar or stock chart (`axisconverter.cxx:292-301`), which predicts `column + midCat` →
unshifted. 26.2.4.2 does not do that. The implementation puts the bar test first, in
`ChartPlot.ShiftedCategories`, and reads the element into a new `ChartPlot.CategoriesBetween`
behind it. `CategoriesBetween` is `null` when the format made no statement, which is every ODF
chart and every binary workbook chart, and null keeps exactly today's answer — so nothing outside
the OOXML reader moves.

The element is read from **the axis the category axis crosses** (`c:catAx/c:crossAx`), not from
the first `c:valAx` in the part, which is what `plotareaconverter.cxx:229-231` hands the
converter as its `pCrossingAxis`.

### Reach and result

`crossbetween-census.py` over every OOXML chart part in the corpus: **28 slides chart parts in 13
documents** state `between` on a line or an area chart; two more state `midCat` and must not move
(`171128IPAP.pptx` `chart7`, `stacked_area_chart.pptx`); one slides combination chart
(`combo_bar_line_chart.pptx`) states `midCat` on a bar and must not move either.

| | base | after §1 |
|---|---:|---:|
| passing over `MANIFEST.tsv` | 200 of 302 | **200 of 302** |
| `abs_ink` | 1036.75 | 1041.20 |
| major pages | 365 | 369 |
| differing pixels | 19402.35 | **19366.62 (−35.73)** |

**Thirteen documents moved, thirteen improved, none worsened** — the predicted set exactly, and
both controls held.

## 2. `8_P-Pavese_AIRBUS…pptx`: four claims from a page reading, four instruments, four answers

The brief's item 2, and its instruction was to check the claims before fixing anything. Three
instruments were written for it and all four claims come back confirmed — one of them with a
correction that changes what the defect *is*.

`fills.py` reports every filled path on a page with its colour and its device-space box.
Page 8, before anything:

| | reference | ours |
|---|---|---|
| chart-object background | `#000000` (0.00, 36.03)–(719.97, 427.49) | **nothing** |
| plot wall | `#454545` (68.74, 85.52)–(708.95, 377.18) | **nothing** |
| the two bars | **16 nested rectangles each**, `#FEFEFE` at the top through `#DDDDDD` at the base | one flat `#F9F9F9` each |

`textcols.py` reports the fill colour in force at every show-text operator. Page 8: the reference
draws **14 white runs**; we drew **22 black** ones.

**And the correction.** The reading said our bars are "a nearly flat, extremely pale near-white
that is almost invisible against the white page". They are — and so are the reference's, which
run `#FEFEFE` to `#DDDDDD`. What makes the reference's bars visible is the **black panel behind
them**, not their own colour. The bar colour was never the defect.

### The two backdrops

`ObjectFormatter`'s `spChartSpaceFill` and `spPlotArea2dFills` (`objectformatter.cxx:174-197`),
ported for styles 33 and up only. Below 33 a pptx chart paints neither, and that is a quirk rather
than a table row: `ObjectTypeFormatter`'s constructor overrides the fill *style* for exactly these
two object types when the style is 32 or less (`objectformatter.cxx:956-959`) and
`PptGraphicHelper` answers `XML_noFill` where the base helper answers `XML_solidFill`
(`oox/source/ppt/pptimport.cxx:309-312`). 160 of the corpus' 163 slides chart parts are style 2,
so that case is nearly the whole corpus and it must stay empty.

Pavese's chart states `c:style val="42"`, so `dk1` for the chart space and `dk1 tint 95000` for
the wall. Rendered:

| | reference | ours, after |
|---|---|---|
| background | `#000000` (0.00, 36.03)–(719.97, 427.49) | `#000000` (0.00, **36.00**)–(**720.00**, **427.50**) |
| wall | `#454545` (68.74, 85.52)–(708.95, 377.18) | `#454545` (**68.75**, **85.44**)–(**709.00**, **377.16**) |

`#454545` is `dk1` under `tint 95000` through the reader's existing colour path, to the byte, and
it is the strongest evidence yet that `ChartTint` is right.

### The one hardcoded black

`ChartLayout` drew every piece of chart text in one `private static readonly Colour AxisColour =
Colour.Black`. It is now five properties on `ChartPlot` — `LabelColour`, `TitleColour`,
`AxisTitleColour`, `DataLabelColour`, `LegendColour` — read from the parts that state them, each
defaulting to black so that a format stating nothing keeps the answer it had. `AxisColour` stays,
documented down to what it still is: the fallback for a radar spoke, a candlestick whisker and a
marker with no fill, none of which any format states a colour for.

Pavese page 8, after: **14 white runs against the reference's 14.** (We still draw 8 black runs to
its 6 — two extra runs that predate this and are not this.)

`8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` page 8, whole:

```
   base   diff% 51.45   ink% 43.44   |ink|% 43.67   16 regions   MAJOR
   after  diff%  3.53   ink%  0.01   |ink|%  0.23   23 regions   shifted
```

The document goes **47.26 → 3.92** on unsigned ink and loses its only major page. It was the
track's largest.

Reach: `style-census.py` finds **three chart parts above style 32 in the whole corpus** — Pavese
and `DynamicBubbleChart.xlsx` — all of them 42. `txpr-census.py` finds a non-black chart text
colour stated in **26 slides, 68 sheets and 10 words documents**, which is the outer bound for the
second half; most of those name `tx1`, which resolves to black on a light theme and changes
nothing.

## 3. The unstacked area chart paints its first series last

Not in the plan. It came out of this round's own vision reading (see below) and it is the largest
single page movement after Pavese.

`AreaChart::createShapes` reverses its own slot list before it draws anything —
`lcl_reorderSeries(m_aZSlots)` under `m_nDimension == 2 && (m_bArea || !m_bCategoryXAxis)`
(`chart2/source/view/charttypes/AreaChart.cxx:565-568`), which is tdf#127813's switch — so series
1 ends up on top of the pile rather than under it.

`006_advanced_powerpoint_area.pptx` page 1:

```
   base      diff% 21.11   |ink|% 0.19   24 regions   shifted
   after §1  diff% 18.67   |ink|% 1.54   26 regions   MAJOR
   after §3  diff%  0.84   |ink|% 0.03   39 regions   ok
```

**A stacked area is exempt, and that is a measurement rather than the source.** `m_bArea` does not
exempt it, but `stacked_area_chart.pptx` is `diff% 1.82, |ink|% 0.16` in file order and
`1.87 / 0.22` reversed: its bands abut rather than nest, so the shared edge belongs to whichever
polygon is drawn last, and the reference draws them in file order. Only the *emission* is
reversed; the accumulation stays in file order or a stacked chart's bands come out in the wrong
sequence, and a test asserts exactly that.

The same source condition covers a scatter chart (`!m_bCategoryXAxis`). **Not implemented**: no
corpus scatter page has overlapping series, so there is nothing to measure it against, and a
change with no measurement behind it is what this list keeps warning about.

## 4. The 24.2.7.2 audit — two re-checks, both VERIFIED

```
open sites 37 in 26 files
   WordProcessing 11   Spreadsheets 9   Presentations 8   Text 5   Core 2   Rendering 1   Ooxml 1
markers 25
```

Round 59 counted 39 open in 30 files with 22 markers; the three that closed are round 59's own and
the other tracks'.

**`PptxSlideLayout.Shadow`** claims that the *first source stating an outer shadow* wins rather
than the first source that exists, and that `EffectProperties::assignUsed` — which replaces the
whole effect list when the source states any effect at all — does not describe the binary. Re-run
on 26.2.4.2 through this site's own fixture (`tests/corpus/features/slide-drop-shadow.pptx`,
flat-ODF export), its five shapes come out

```
  Angled       0.149 / 0.149  #000000  100%
  Themed       0     / 0.056  #000000   38%
  EmptyList    0     / 0.056  #000000   38%     <- the arm the rule exists for
  Glow         0     / 0.056  #000000   38%     <- and the other one
  Translucent  0     / 0      #000000   60%
```

**3 of 3.** The empty `a:effectLst` and the lone `a:glow` still keep the theme's shadow.

**`SlideAutofit.Quantised`** claims the twip → 1/100 mm conversion is `(n * 127 + 36) / 72`.
It still is — but the *citation* has moved. `convertTwipToMm100` no longer spells it out; it
delegates to `o3tl::convert(n, twip, mm100)` (`include/tools/UnitConversion.hxx:23-26`), whose
positive branch is `(n * num + den / 2) / den` with `num = 127`, `den = 72` — `den / 2` being the
36. `SvxFontHeightItem::PutValue` still rounds through twips with `fPoint * 20.0 + 0.5`
(`textitem.cxx:943, 976`). The worked case still divides the two readings: 13.33 pt is 267 twips,
`(267 * 127 + 36) / 72 = 471` against the direct ratio's 470. **VERIFIED**, with the citation
corrected so the next reader is not sent to a line that no longer holds the formula.

Both markers are written into the sites themselves.

## 5. What the plot rectangle's right edge actually is, now that this round has been through it

Plot rectangles over the 57 chart pages the census can measure:

| | r59 base | this round's base | **final** |
|---|---:|---:|---:|
| all four edges within 0.5 pt | 10 | 13 | **14** |
| `dLeft` over 1 pt | 22 | 9 | **9** |
| `dBottom` over 1 pt | 23 | 11 | **11** |
| `dTop` over 1 pt | 10 | 9 | **9** |
| `dRight` over 1 pt | 31 | 31 | **27** |

The prediction said 20 … 24 and it missed, upward, for a reason the round can name exactly. The
eight `line`/`area` template pages did not go to zero: they went from `−9.66 / −9.68` to
**`−2.71 / −2.73`** — and that residual is the same number the four `column` pages and the five
`bar` pages were already sitting at.

**Seventeen of the twenty-seven remaining pages sit at exactly `−2.71`, `−2.73` or `−2.88`, and
`legend-census.py` says what it is.** On all four families the legend's key, the key's own size
and the key-to-text gap agree between the two stacks to within 0.02 pt — and the whole legend sits
**2.70 pt to the left** of the reference's:

| | key left, ours | key left, ref | Δ | key width ours/ref | Δ text pen |
|---|---:|---:|---:|---|---:|
| `001_advanced_powerpoint_bar` | 594.90 | 597.60 | **−2.70** | 6.00 / 5.98 | −2.69 |
| `002_advanced_powerpoint_column` | 594.90 | 597.60 | **−2.70** | 6.00 / 5.98 | −2.69 |
| `003_advanced_powerpoint_line` | 410.51 | 412.72 | −2.21 | **7.00 / 6.01** | −2.33 |
| `006_advanced_powerpoint_area` | 594.90 | 597.60 | **−2.70** | 6.00 / 5.98 | −2.69 |

The plot's right edge is `frame.Right − margin − legend.Width − LegendMarginX`, and the legend is
drawn at `frame.Right − LegendMarginX − legend.Width`, so both move together: the legend box is
**2.70 pt too wide on its right**, and the plot gives that up. The line family is 2.21 rather than
2.70 because a line series' legend key is drawn 7.00 pt wide against the reference's 6.01 — a
second, smaller defect visible in the same table.

That is the next round's first item and it is worth seventeen of fifty-seven chart pages.

## The vision reading — committed here, not only in the report

Three pages, each chosen for a stated reason, each handed to a fresh subagent with the composed
image and nothing else — forbidden from reading any project file or running any command, and
asked to describe each half alone before comparing and to give the direction of every difference.
The halves were rasterised from the sweep's own PDFs rather than re-rendered, so the reviewer saw
exactly what the numbers above were computed from.

### `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` page 8 at 130 dpi, after the fix

Chosen because it is the round's largest single change and because the round-59 reading of the
same page is what produced it; the point of reading it again is to see what the change left.

The reviewer described a **black chart panel with a dark charcoal plot rectangle and a white bold
title** on *both* halves, and listed as identical: "the black outer panel colour and the dark
charcoal plot-area colour, and the plot area's left/right extent", "bar geometry: both bars are
the same width, at the same horizontal positions, and reach the same heights", the y-axis, the
category labels, the journal clip and the header. Round 59's first three claims are closed and a
reader who had never seen them says so.

What it ranked first among the remaining differences: **"the top half's bars are a flat, uniform
off-white with no shading; the bottom half's carry a top-to-bottom white→light-grey gradient with
visible banding."** **Confirmed** by `fills.py`, which counts **16 nested rectangles per bar**
running `#FEFEFE` at the top to `#DDDDDD` at the base — the fourth of round 59's claims, still
open, and now with its shape and its endpoints measured. Second: our `(548/621)` breaks onto a
third line carrying a lone `)` where the reference fits it on one, because our in-bar text is
drawn slightly larger. Also open.

And one claim this round **refutes with an instrument**: the reviewer reported that "the bottom
half's whole chart block sits lower" and "the top half's black panel extends further down". The
fill census says the two panels are `(0.00, 36.00)–(720.00, 427.50)` and
`(0.00, 36.03)–(719.97, 427.49)` — **the same rectangle to three hundredths of a point.** A
whole-page composite at 130 dpi is not a ruler, which is the calibration this section exists for.

### `006_advanced_powerpoint_area.pptx` page 1 at 150 dpi

Chosen because the *ink* column called this page a regression — `|ink|% 0.19 → 1.54`, and a new
major page — while differing pixels called it an improvement, and a reader is the cheapest way to
find out which is right.

The reviewer answered the geometry question and then found something nobody had asked about.

On the geometry, asked specifically where the shaded region begins and ends: **"the bottom half's
shaded region does not start further left or further right than the top half's; both left edges
land on the M1 tick and both right edges land on the M8 tick, with the same half-category white
margin at each end. I could not detect a shift in either direction beyond a pixel or two."** That
is §1 landing, seen rather than measured — and it is **confirmed** by the polygon coordinates:
the reference's area runs `x 116.62 … 551.45`, ours ran `85.99 … 572.84` and now runs
`116.85 … 548.93`.

Unprompted, and ranked as the loudest difference on the page: **"the dominant colour of the area
chart flips from blue (top) to red (bottom) … the two series are painted in opposite order"**,
together with the observation that **"both halves cross over at the same place, around M4, and
both have the same outer silhouette"** — which is precisely the discriminator between a paint
order and a value. That is §3, found by a reader and by no metric, and it took the page from
`diff% 18.67 / |ink|% 1.54 / MAJOR` to `0.84 / 0.03 / ok`.

### `Demick_JetBlue.pptx` page 4 at 150 dpi

Chosen because round 59's reader reported on this page that "the top half's last category label
runs off the right edge and is clipped" and called it a new lead pointing at the right edge, and
because this round's change moves that document's categories.

The reviewer reported: **"the top half's labels sit further right; the bottom half's labels sit
further left. The total offset between the two is roughly 45 px, a bit over one category width…
the top's labels land ~25–30 px right of their own ticks; the bottom's land ~15–20 px left of
theirs."** And, contradicting round 59: **"neither half clips or truncates any label — both are
contained within the chart frame."** It also reported the x-axis title centred in ours and ~50 px
left of centre in the reference's, and marker symbols present in the reference's legend keys and
absent from ours.

**Not confirmed by a second instrument, and deliberately left that way.** A rotated-label pen
census was attempted and abandoned: our rotated runs carry the rotation in the CTM and the
reference's in the text matrix, so the naive detector counted 23 runs on our side against 26 on
theirs and could not band them. The offset the reviewer describes is the right size for an
*anchor* rather than a category: at 150 dpi 45 px is 21.6 pt, which is about the horizontal extent
of one of those labels turned 45°, so the natural hypothesis is that the reference anchors a
rotated category label by its end and we anchor it by its centre. **That is a hypothesis, not a
measurement**, and the next round should build the instrument before believing it. The legend
markers are round 56's open item 8, now sighted by a third independent reader.

## Refutations

1. **The brief's item 1.** The plot rectangle's right edge is not one defect and the part of it
   this round could reach was not a right-edge reservation at all: it was `c:crossBetween`, which
   moves the *categories* — labels and plotted data alike — and only reaches the right edge
   through an end-label overhang that then stops applying. Nine probe arms, one property each.
2. **The 27.2 source tree, on a bar chart.** `axisconverter.cxx:292-301` reads `c:crossBetween`
   ahead of the chart type for everything but a 3-D bar; 26.2.4.2 renders a column chart stating
   `midCat` **byte-identically** to the same chart stating `between`. The binary wins and the
   implementation puts the bar test first.
3. **`AreaChart`'s own condition, on a stacked chart.** `m_bArea` does not exempt a stacked area
   from the series reversal; the measurement does — `stacked_area_chart.pptx` is `diff% 1.82,
   |ink|% 0.16` in file order and `1.87 / 0.22` reversed.
4. **Round 59's fourth Pavese claim, corrected rather than refuted.** Our bars *are* a flat,
   nearly invisible near-white — and so are the reference's, `#FEFEFE` to `#DDDDDD`. What makes
   the reference's visible is the black panel behind them. The bar colour was never the defect;
   the missing backdrop was.
5. **This round's own reviewer, on the black panel's extent.** "The top half's black panel extends
   further down" — the two panels are the same rectangle to 0.03 pt. A whole-page composite is
   not a ruler.
6. **`abs_ink` as the instrument, for the third round running.** Sixteen documents worsened on
   unsigned ink at the intermediate stage while improving on differing pixels, `006_advanced_
   powerpoint_area` most sharply: `|ink|% 0.19 → 1.54` for a change that moved its polygon from
   `85.99 … 572.84` onto the reference's `116.62 … 551.45`. The ink rose *because* the polygons
   came into register and exposed a paint-order difference that had been smeared across the page
   before. Fixing that took the same page to `|ink|% 0.03`.

## Tests

Three new files, **29 new tests**, and the total reconciles: **4970 = 4941 + 29** (see the
final count at the end of this file).

| test | mutation | outcome |
|---|---|---|
| `DrawingChartCategoryShiftTests` (13) | `return CategoriesBetween ?? false` → `return false` | **DETECTED**, 4 of 13 |
| `DrawingChartBackdropTests` (13) | `if (style <= 32) return null` → `if (style <= 0)` | **DETECTED**, 4 of 13 |
| `DrawingChartBackdropTests` (13) | `LabelColour = AxisLabelColourOf(…)` → `Colour.Black` | **DETECTED**, 3 of 13 |
| `ChartAreaPaintOrderTests` (3) | the reversal put back to file order | **DETECTED**, 1 of 3 |

Four mutations, four detected by reintroduction; none of the four classes is a drift guard. Each class's inert cases are controls by
design: the `barChart` arms that must ignore `c:crossBetween`, the radar chart that must ignore it
too, the scatter chart with no category axis, the four styles below 33 that must paint nothing,
the stated `c:spPr` that must beat the table, the chart stating no colour anywhere that must draw
all five in black, and the stacked area that must keep file order.

**One test was wrong when first written and the failure is worth recording.** The area paint-order
control first asserted that a stacked chart's topmost vertex sits above an unstacked one's. It
does not: each chart is drawn against its own automatic scale, so both reach the top of their own
plot and the assertion is about the scale rather than the stack. It now asserts the thing that
actually distinguishes a reversal of the *paint order* from a reversal of the *data* — the polygon
drawn last still rises 10 → 40 across the categories, which is series 1's own slope. A second
version of the same test then failed for a different reason worth knowing: a `Close` command
carries a default `Point` of the origin, so a naive "leftmost vertex" scan over
`GraphicsPath.Commands` finds `(0, 0)` on every closed path.

## Controls

| | base | final | predicted |
|---|---|---|---|
| `tf-agreement` mean | 0.77065 over 4515 pages | **0.77065** | unchanged ✓ |
| exact `/Tf` pages | 1708 of 4515 | **1708 of 4515** | unchanged ✓ |
| sheared glyphs (reference 16008) | 15792 | **15792** | unchanged ✓ |
| pages whose sheared-glyph counts disagree | 82 | **82** | unchanged ✓ |
| page counts changed | | **0 of 302** | 0 ✓ |

`tf-agreement.py` was run on this round's own base *and* on its final directory and prints
**0.77065** for both. Round 59 read 0.77063 at the same base and the briefed original was
0.77061; the three agree to four decimals and **round 56's 0.85188 is still the outlier and still
unexplained.** Nothing here touches a glyph's transform or a drawn em size.

## Measured against the prediction

| # | predicted | measured |
|---|---|---|
| 1 | verdicts **0**, band −1 … +1 | **0**, 200 → 200 ✓ |
| 2 | page counts 0 of 302 | **0 of 302** ✓ |
| 3 | documents moved on differing pixels **13**, band 11 … 16 | **13 exactly** for §1, and **26** for the round ✗ |
| 4 | `abs_ink` **−2 … −10** | **−46.62** ✗ |
| 5 | differing pixels **−40 … −250** | **−161.82** ✓ |
| 6 | `dRight` over 1 pt **31 → 20 … 24** | **27** ✗ |
| 7 | `dLeft` over 1 pt **9 ± 2** | **9** ✓ |
| 8 | controls unchanged | all four unchanged ✓ |

**The documents-moved band has now missed five rounds running, and this round predicted the miss
and got the reason wrong.** The prediction said in as many words that if it missed upward the
cause would be a second item arriving after it was written, "most likely the 24.2.7.2 audit". It
missed upward by exactly that mechanism and the audit was not it: the two extra items were the
Pavese backdrops (which the brief *did* name, and which the prediction chose not to band because
its four claims were unconfirmed when the prediction was written) and the area paint order (which
came out of this round's own vision reading). The audit produced two clean VERIFIEDs and no ink at
all — the first round in three where it did not outweigh the plan.

**Item 6 is the useful miss.** The band assumed the eight `line`/`area` template pages would go to
near zero; they went to −2.71/−2.73, which is where the `column` and `bar` pages already were.
That is the legend, it is one number, and §5 above measures it.

## Shared layers — this diff reaches all three tracks and the parent must gate the corpus

Three of the four changes are outside `XRay.Office.Presentations`:

* **§1** touches `XRay.Office.Core/Charts` (`ChartPlot`) and `XRay.Office.Ooxml`
  (`DrawingChartPlot`).
* **§2** touches `XRay.Office.Ooxml` (`DrawingChartAutoFormat`, `DrawingChartPlot`) and
  `XRay.Office.Core/Charts` (`ChartPlot`, `ChartLayout`, `ChartLayout.Plots`).
* **§3** touches `XRay.Office.Core/Charts` (`ChartLayout`).
* Nothing touches `XRay.Office.Vector`, `Text`, `Rendering`, `Markup` or `Containers`.

Census reach outside slides, counted on what the parts state:

| change | sheets | words |
|---|---|---|
| §1 `c:crossBetween` on a line or area chart's crossing axis | **12 `c:lineChart` parts** in 8 documents | **none** |
| §2 automatic backdrop, `c:style` above 32 | **1 document** (`DynamicBubbleChart.xlsx`) | none |
| §2 a stated non-black chart text colour | **68 documents** | **10 documents** |
| §3 unstacked area paint order | 4 `c:areaChart` parts, 9 more mixed with scatter | none |

The 68 and the 10 are an outer bound and not a prediction: most of those parts name `tx1`, which
resolves to black on a light theme and changes nothing.

**Measured rather than argued**, by sweeping each track whole at this tree and scoring the verdict
column against `MANIFEST.tsv`'s `status`:

| track | passing over `MANIFEST.tsv` at this tree | manifest disagreements |
|---|---|---|
| **words** | **319 of 337** (337 of 337 visited) | **0** |
| **sheets** | **279 of 307** (307 of 307 visited) | **2 — and neither is this round** |

**Words is unchanged and clean**, and its verdict column is robust: three independent sweeps of
this track were run during the round (two of them spoiled, see below) and **all 356 comparable
rows agree on the verdict in all three.**

**Sheets is 279 of 307 with two disagreements, and both are stale manifest rows rather than
movement.** They are in the `advanced_excel_pie` family round 59 already flagged, and each is one
word from its own band:

| document | words ours / reference | band | verdict | manifest |
|---|---|---|---|---|
| `003_advanced_excel_pie.xlsx` | **143 / 143** | 2.86 | `match` | `open` |
| `011_advanced_excel_pie.xlsx` | **136 / 140** | 2.80 | `words` | `done` |

Both were checked **at the base commit's own chart sources** — `git checkout 2870991a4dd -- ` the
two chart directories, build the CLI alone, render, restore, rebuild, and the `XRay.Office.Core.dll`
md5 comes back identical — and they read **143 and 136 there too.** Neither number is this round's.
The gate's rule is `d > b * 0.02 && d > 3`, so `011` fails on the fourth word and would pass on the
third; `003` matches the reference exactly and the manifest has not been told.

## An instrument failure of my own, caught by a byte comparison

`CLAUDE.md`'s rule — **a sweep and a rebuild must never overlap** — was broken in this round, and
the way it announced itself is worth recording because it did not look like a rebuild.

The first pair of cross-track sweeps was scored and then differed from the second pair by
**31 words documents**, one of them by 19.82 of unsigned ink on a questionnaire with no chart in
it at all. Rendering nondeterminism was the obvious suspect and it is not: rendering
`te.iors.00048-002 SUP Questionnaire.docx` twice at the same tree gives **byte-identical** PDFs,
and the fresh one is byte-identical to the *first* sweep's copy and not to the second's — 157 696
bytes against 157 807. The reference copies of that document in the two sweeps are byte-identical
to each other, so the reference is not it either.

What happened is that `verify-test.sh` — which mutates a source file, **rebuilds**, tests,
restores and **rebuilds again** — was run while those two sweeps were still going, and its build
replaced `XRay.Office.Core.dll` in the CLI's output directory underneath them. Every document the
sweeps had not yet reached was rendered by a binary carrying a deliberately introduced defect.

Both cross-track sweeps were re-run from scratch with nothing else touching the tree, and the
numbers below are from the clean pair. The slides sweep is unaffected — it had printed its
`TOTAL` before `verify-test.sh` was started, and eight of its documents re-rendered at the current
tree come back byte-identical to their copies in it.

**The check that caught it is cheap and should be routine**: re-render two or three of a finished
sweep's documents and `cmp` them against the copies it kept. A sweep contaminated this way looks
exactly like a real result.

## Left open, in the order the next round should take them

1. **The legend is 2.70 pt too wide, and it is 17 of the 57 chart pages.** After this round
   `dRight` over 1 pt stands at 27, and seventeen of those pages sit at exactly `−2.71`, `−2.73`
   or `−2.88` — one number across bar, column, line and area alike. `legend-census.py` locates it:
   the key's own size and the key-to-text gap agree with the reference to within 0.02 pt, and the
   whole legend sits 2.70 pt to the *left*, which is the plot's right edge paying for a legend box
   that is too wide on its right. The plot's right edge is
   `frame.Right − margin − legend.Width − LegendMarginX` and the legend is drawn at
   `frame.Right − LegendMarginX − legend.Width`, so the two move together and the surplus is in
   `legend.Width`. **A second, smaller defect is visible in the same table**: a *line* series'
   legend key is drawn 7.00 pt wide against the reference's 6.01, which is why the line family is
   2.21 rather than 2.70.
2. **Pavese's gradient bars — the last of round 59's four claims.** The reference draws each bar
   as **16 nested rectangles** running `#FEFEFE` at the top to `#DDDDDD` at the base; we draw one
   flat `#F9F9F9`. The mechanism is the half of the automatic fill table this reader deliberately
   does not read: `DrawingChartAutoFormat`'s own remarks say "a fill entry's `mnThemedIdx` reaches
   only `Theme::getFillStyle`'s gradient — the colour comes from the pattern alone". For a chart
   above style 32 that gradient is what the reference paints. Worth what is left of that document
   (3.92) and reachable by the same route §2 took.
3. **The rotated category label's anchor.** A reviewer on `Demick_JetBlue` page 4 reports our
   45°-rotated labels landing 25–30 px *right* of their ticks and the reference's 15–20 px *left*
   of theirs, ≈45 px apart at 150 dpi — about the horizontal extent of one such label, which is
   what an end-anchor versus a centre-anchor would give. **No instrument has checked this**: our
   rotated runs carry the rotation in the CTM and the reference's in the text matrix, so a naive
   pen census cannot band them. Build the instrument first.
4. **`N2_E_Maestroni_Swarm_COP.pptx`'s `c:manualLayout`.** Untouched again. Its plot rectangle is
   still `dLeft +15.60, dBottom −21.65, dRight +8.56` and round 59's analysis stands:
   `layoutTarget="inner"` with `x + w = 1.024`, and `lclCalcRelSize`'s clamp to `1 − x` is not what
   the reference draws either. 51 corpus chart parts state a plot-area manual layout, 20 in slides.
5. **The fitted bullet's vertical placement** — 1.9 pt too high, `ALIGN_BOTTOM` /
   `aBulletArea.Bottom()`, `outliner.cxx:909-919`. **Untouched for six rounds now.** This round did
   not take it either, and that was a decision rather than a default: three chart items with
   direct measurements behind them outranked it. The next round should either take it or say in
   the same words why not.
6. **The automatic marker cycle**, now sighted by a *third* independent reader — this round's
   `Demick_JetBlue` reviewer reports marker symbols in the reference's legend keys and none in
   ours. `typegroupconverter.cxx` names *square, diamond, arrow down, arrow up* and `ChartMarker`
   has one triangle.
7. **A scatter chart's series paint order.** `AreaChart`'s condition covers `!m_bCategoryXAxis` as
   well as `m_bArea`, so a scatter chart should reverse too. Not implemented: no corpus scatter
   page has overlapping series, so there is nothing to measure it against.
8. **Pavese's `(548/621)`** wraps onto a third line carrying a lone `)` where the reference fits it
   on one, because our in-bar data-label text is drawn slightly larger.
9. **`171128IPAP.pptx` page 32 is called MAJOR at `diff% 2.94, |ink|% 0.15`.** The major flag is a
   region count as well as a magnitude, and a page can cross it while improving on both numbers.
   Worth knowing before a round reads a major-page total as a verdict-shaped quantity.
10. **`2015-Civil-Rights-Website-training.ppt`, 29.64**, still untouched; the 11 EMF face-name
    documents from round 56's census; `WmfReader.CreateFont`'s missing record bound; `wordArtVert`;
    the `pitchFamily` family nibble (product decision with the user, still open); and the `.ppt`
    `cdirFont` (Escher 137), still deliberately unread.
11. **Round 59's three unchased leads on `010605Vul.ppt` page 9** are still unchased: the
    timeline's leftmost run black where the reference's is red, node markers blue where the
    reference's are maroon, and a pixelated coat-of-arms with an extra black bar along its edge.

## Tests, the final count

Ten non-Fidelity projects, one at a time, at the final tree:

```
Core 358   Containers 109   Text 624   Vector 302   Rendering 153(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1188   Spreadsheets 980   Presentations 872     = 4970
0 failed
```

**4970 = 4941 + 29**, and `cd dotnet && dotnet build -v q -nologo` → **0 warnings, 0 errors**.

**`XRay.Office.Vector.Tests` reported 20 failures and none of them was real** — run alone, 302 of
302. Eighth sighting of that pattern. The machine was carrying a concurrent round-61 words agent
at the time, which is also what produced the two truncated diff reports in the sheets sweep.
