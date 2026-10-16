# slides-r62 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
base `337bc9fe17c`, branch `wt-slides-r62`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`,
`TMPDIR` on the host mount throughout. `prediction.md` beside this file was committed as
`4ed82e5bfc9`, before anything was built or rendered post-change.

## The baseline reproduces on the gate and on the instrument

| | briefed | measured at `337bc9fe17c` |
|---|---|---|
| passing over `MANIFEST.tsv` | 200 of 302 | **200 of 302, 0 disagreements, 302 of 302 visited** |
| `abs_ink` | 990.13 (round 60's final) | **990.51** |
| major pages | 364 | **364** |
| differing pixels | — | **19226.90 over 4530 pages** |
| plot-rect census, `dRight` over 1 pt | 27 of 57 chart pages | **27 of 57** |
| `dLeft` / `dBottom` / `dTop` over 1 pt | 9 / 11 / 9 | **9 / 11 / 9** |

The gate reproduces exactly and so does round 60's plot-rectangle census, page for page — which
matters more than the totals, because it is the instrument this round's target was defined on.

The 0.38 of `abs_ink` above round 60's final figure is **six documents and nothing else**:
`3495` +1.41, `031_Alarm_Clock_Pie-Chart` −1.25, `171128IPAP` +0.26, `Intersil…` −0.14,
`3492` +0.08, `Demick_JetBlue` +0.02. Those are the merged rounds 61 sheets (its pie first-pass
fix reaches `XRay.Office.Core/Charts`) and 61 words landing on this track. Recorded, not chased.

## The whole round

| | base | final |
|---|---:|---:|
| passing over `MANIFEST.tsv` | **200 of 302** | **200 of 302** (and **+1 on sheets**, see below) |
| page counts changed | | **0 of 302** |
| `abs_ink` | 990.51 | **989.29 (−1.22)** |
| signed ink | 693.89 | 693.40 |
| major pages | 364 | **364** |
| **differing pixels over 4530 pages** | 19226.90 | **19215.72 (−11.18)** |
| **plot-rect `dRight` over 1 pt** | **27 of 57** | **10 of 57** |
| face mismatches over the 112 slides chart pages | 3363 runs | **3215** |

**Twenty-eight documents moved on differing pixels — 28 improved, 0 worsened.** On unsigned ink
twenty moved, **14 improved and 6 worsened**; the six are named below rather than netted.

| Δ differing pixels | document | before → after |
|---:|---|---|
| −1.10 … −1.05 | `002/010/018/026_advanced_powerpoint_column.pptx` | ≈2.50 → ≈1.43 each |
| −0.86 … −0.84 | `001/009/017/025/033_advanced_powerpoint_bar.pptx` | ≈1.92 → ≈1.07 each |
| −0.26 … −0.25 | `006/014/022/030_advanced_powerpoint_area.pptx` | ≈0.87 → ≈0.61 each |
| −0.17 … −0.16 | `003/011/019/027_advanced_powerpoint_line.pptx` | ≈0.82 → ≈0.65 each |
| −0.15 | `stacked_area_chart.pptx`, `stacked_bar_chart.pptx` | 0.90 → 0.75, 1.64 → 1.49 |
| −0.11 … −0.10 | `004/012/020/028_advanced_powerpoint_pie.pptx` | ≈1.07 → ≈0.97 each |
| −0.05 | `005/013/021/029_advanced_powerpoint_doughnut.pptx`, `southern-classic…pptx` | |

The six documents that worsened on **unsigned ink** all improved on differing pixels in the same
sweep, which is the fourth consecutive round in which that column has behaved this way:

| Δ `abs_ink` | document | Δ differing pixels |
|---:|---|---:|
| +0.01 | `005/013/021/029_advanced_powerpoint_doughnut.pptx` | −0.05 each |
| +0.01 | `027_advanced_powerpoint_line.pptx` | −0.16 |
| +0.01 | `stacked_area_chart.pptx` | −0.15 |

## 1. The legend's 2.70 pt is a **typeface**, not legend arithmetic

The brief's item 1 said the legend box is 2.70 pt too wide on its right and named
`legend-census.py` as the instrument. The instrument was right about *where* and wrong about
*what*, and two independent readings of `001_advanced_powerpoint_bar__pptx` page 1 say so.

**The PDF's own font resources.** `facecensus.py` resolves every text run's `/Fn` through the
page's resource dictionary to the embedded `/BaseFont`:

| | ours, base | reference |
|---|---|---|
| ten-point runs in `LiberationSans` | **19** | **17** |
| ten-point runs in `Carlito-Regular` | **0** | **2** |

The seventeen are the axis and category labels — the axes state
`<a:latin typeface="Arial"/>` and Liberation Sans is Arial's metric substitute — and the two are
the legend's entries, which the reference sets in the theme's Calibri. Every other run on the
page (13 pt, 11 pt, 14 pt, 16 pt, 24 pt, 9 pt italic) matches face for face on both sides.

**The drawn extents.** `pdftotext -bbox` on the widest legend entry:

| | left | right | width |
|---|---:|---:|---:|
| ours, base | 603.723 | 631.533 | 27.810 |
| reference | 606.415 | 631.538 | 25.123 |

**The right edges agree to 0.005 pt.** That is structural rather than lucky: the widest entry's
right edge is `frame.Right − LegendMarginX − paddingX` in both stacks whatever the text
measures, because the legend box's width carries the entry and its position subtracts it again.
So the whole 2.692 pt is the entry's own width, and Carlito is that much narrower than Liberation
Sans at 10 pt.

### The seat

`DrawingChartPlot.FamilyOf` is `c:chartSpace/c:txPr`'s literal `a:latin`, then **the first
literal `a:latin` anywhere in the part**, then the theme's minor face. This deck's chart space
states no `c:txPr`; its `c:catAx/c:txPr` and `c:valAx/c:txPr` both state `Arial`; its `c:legend`
states nothing. The second term hands the axes' Arial to the legend.

The reference resolves each chart object separately. `ObjectFormatter`'s automatic text table
names `XML_minor` for every automatic entry it has (`objectformatter.cxx:415-434`) and an
object's own `c:txPr` overrides it for that object alone.

`ChartPlot.LegendFamily`, nullable, read as **`c:legend/c:txPr` → `c:chartSpace/c:txPr` → theme
minor**, and used by `ChartLayout.Legend`'s reservation, `AddLegend`'s placement walk and the
labels both emit. Null keeps `TextFamily`, so no ODF or BIFF chart can move.

### What it did

| `001_advanced_powerpoint_bar` p1 | base | after |
|---|---:|---:|
| legend text pen | 603.723 | **605.821** (reference 606.415) |
| plot rectangle `dRight` | −2.88 | **−0.79** |

**The residual 0.59 pt is the advance divergence `CLAUDE.md` already documents, and it is
visible in the reference's own operators.** Its `TJ` array for "Actual" carries positive
adjustments summing to 56 thousandths of an em — `<01>14<06>11<0E>8<15>12<04>11<11>` — which at
10 pt is **0.56 pt** of tracking we do not apply. That is the reference grid-fitting its glyph
advances, the open architectural item, not anything this round could close.

Over the census: **`dRight` over 1 pt went 27 → 10, and all seventeen of the pages that sat at
−2.71 / −2.73 / −2.88 now sit at −0.62 / −0.63 / −0.79.** `facediff.py` says the same thing from
the other side: those seventeen pages read exactly
`+LiberationSans@10.01×2 | −Carlito-Regular@10.01×2` before and **nothing at all** after.

"All four edges within 0.5 pt" stayed at 14 and that is worth saying plainly: the seventeen pages
land at 0.62–0.79 pt, which is inside 1 pt and outside 0.5, so a threshold at half a point cannot
see the change at all.

## 1b. The legend's entry **order**, found beside it

`001_advanced_powerpoint_bar` lists *Plan* above *Actual* in the reference and *Actual* above
*Plan* in ours. `002_advanced_powerpoint_column` and `006_advanced_powerpoint_area` — same deck
family, same two series, same legend position — list *Actual* above *Plan* on **both** sides.

`VSeriesPlotter::createLegendEntries` (`chart2/source/view/charttypes/VSeriesPlotter.cxx`
:2432-2447) inserts a series' entries at the *front* of the list rather than the back under two
conditions: with the coordinate system swapped — which is a horizontal bar chart and nothing else
— unless the series stack in Y; and with it unswapped, and only for a legend at the line start or
line end, when the series *do* stack in Y.

Five arms measured on the binary, two of them controls that must not move:

| chart | legend | grouping | reference | ours, base |
|---|---|---|---|---|
| `001_advanced_powerpoint_bar` (horizontal bar) | right | clustered | **Plan, Actual** | Actual, Plan |
| `002_advanced_powerpoint_column` | right | clustered | Actual, Plan | Actual, Plan ✓ |
| `006_advanced_powerpoint_area` | right | clustered | Actual, Plan | Actual, Plan ✓ |
| `stacked_bar_chart` (column, stacked) | right | stacked | **In-Store, Online** | Online, In-Store |
| `stacked_area_chart` (area, stacked) | right | stacked | **In-Store, Online** | Online, In-Store |

And a fifth, on a real corpus deck rather than a template:
`southern-classic-kennesaw-state-university-final.pptx` page 11, `chart13`, a five-series stacked
column with a right legend — the reference lists *Total Mainline Passenger Revenue / Latin
America / Pacific / Atlantic / Domestic* top to bottom and we listed them the other way up.

`ChartPlot.LegendReversed`; `Entries` reverses after building. The pie branch returns before it,
which is right — the rule reads a stacking direction and a swapped coordinate system and a pie
has neither.

`legendorder-census.py`: **17 corpus documents** — 8 slides, 9 sheets, no words.

## 2. Pavese's gradient bars: **measured, and deliberately not implemented**

The brief's item 2. The claim is confirmed and its *value* is the finding.

`fills.py` on page 8 of the current tree: the reference draws each bar as sixteen nested
rectangles sharing a bottom edge, `#FEFEFE` at the top through `#DDDDDD` at the base; we draw one
flat `#F9F9F9` per bar. Both bars, both stacks, same rectangle to a hundredth of a point.

What it is worth, measured rather than estimated. Page 8 at 150 dpi, total absolute channel
difference over the page against the reference:

```
  page total                          28,876,487
  inside the two bar rectangles       15,377,157   53.3% of the page's difference
  bar area                            371,475 px of 1,687,500   (22.0% of the page)
  mean |d| per pixel   in the bars 13.80 of 255      elsewhere 3.42
```

So the gradient is the **majority of what is left on that page** — and that page is
`diff% 3.47, |ink|% 0.23` in a document of 3.92 over 26 pages, which is **0.02% of the track's
990.51**, and pages 4 (0.65), 6 (0.60), 5 (0.37), 21 (0.34) and 14 (0.32) are each larger.

The cost is not proportionate. `ChartBox.Fill` and `ChartSeries.Fill` are `Colour?`; a themed
gradient needs a `Paint` through `XRay.Office.Core/Charts`, `XRay.Office.Ooxml` and the four
consumers that draw chart boxes (`SlideChart`, `SheetChart`, `FrameChart`, `XlsDrawing`) — a
change across all three tracks for **0.12 of one document's 3.92**, plus
`DynamicBubbleChart.xlsx` on sheets, those being the only two corpus chart parts above style 32.

**Not implemented, and the number is here so the next round can rank it rather than inherit it.**
The mechanism is known and unchanged: `spFilledSeries2dFills`'s `THEMED_STYLE_INTENSE` index
reaches `Theme::getFillStyle`, and `FillFormatter`'s constructor copies that whole
`FillProperties` — gradient included — before the pattern's colour is substituted for `phClr`
(`objectformatter.cxx:865-877`). `DrawingStyleMatrix` already reads `a:fillStyleLst`, so the
OOXML half is a short hop; the model and the four consumers are the work.

## 3. The rotated label's anchor: **refuted, and what it actually is is bigger**

The brief said to build the instrument before believing the hypothesis. The instrument is
`rotruns.py`: it takes each run's rotation from **`Tm × CTM`** rather than from either factor, so
it sees a rotated run whichever matrix carries the angle — which is the instrument problem round
60 named.

On `Demick_JetBlue.pptx` page 4 it says:

```
  ours       52 runs   {0: 29, 45: 21, 90: 2}
  reference  55 runs   {0: 29, 90: 26}
```

**The reference draws no 45° runs at all.** Round 60's hypothesis — that the reference anchors a
rotated category label by its end and we anchor it by its centre — cannot be right, because
there is no rotated *text* on the reference's side to anchor.

### And the instrument was still under-reaching, which a blind reader caught

A fresh reviewer given the composed page reported the same 24 rotated 45° category labels on
*both* halves. That contradicted the run census, so the census was checked rather than the
reader: `fills.py` finds **126 glyph-sized black filled paths** in the label band of the
reference's page 4 and **none** on ours. Twenty-one labels of six characters is 126 exactly.

**26.2.4.2 emits a 45°-rotated chart category label as filled outlines, not as text.** Its 90°
axis titles on the same page stay text (one glyph per run, 26 of them) and its 0° labels on pages
6 and 8 stay text, so the three arms are separated by the rendering itself.

### What that is worth: two of the track's word-gate failures are not ours

`Demick_JetBlue.pptx` fails the gate at **812 words against 608**. Per page:

```
  p1  7/7    p2 61/61   p3 92/92   p4 139/76  p5 130/52
  p6 57/57   p7 125/62  p8 83/83   p9 118/118 p10 0/0
```

Every page matches exactly except 4, 5 and 7, and those three are **63 + 78 + 63 = 204** — the
entire deficit, and exactly **three times** the 21, 26 and 21 rotated labels each page carries,
because `pdftotext` splits `2006-7` into three tokens.

`outlined-text-census.py` over the track's largest word surpluses finds the same signature on
`N2_E_Maestroni_Swarm_COP.pptx` page 7: **+171 words, 220 reference glyph-paths against our 47**,
against a document surplus of 170.

| document | words ours/ref | band | surplus on outlined pages |
|---|---|---:|---|
| `Demick_JetBlue.pptx` | 812 / 608 | 12.16 | **204 of 204** |
| `N2_E_Maestroni_Swarm_COP.pptx` | 5296 / 5126 | 102.5 | **171 of 170** |

Both would pass with those pages excluded. **Both stacks draw the same glyphs; only the
reference's PDF fails to carry them as characters, so the gate is measuring the reference's
export and our output is the better one.** This is exactly the class `COMMON.md`'s charstream
test exists to find, and the right response is not to degrade our output — it is a note for the
parent, who owns `MANIFEST.tsv`. **Proposed, not committed.**

Three other surpluses do **not** have this shape and are unexplained:
`OnTrac_StarCertificationProgram-3Day` p10 (+249, no filled paths on either side),
`16 - UTM - (NASA)` p7/p29 (+90, +103), `Thailand17` p8 (+88).

**And this is a third class the charstream test does not name.** `COMMON.md` says: same
characters with a failing word count is a tokenisation ceiling and our output may be the better
one; different characters is a real content or layout defect. These pages have **different
characters and no layout defect at all** — both stacks draw the same glyphs in the same places,
and only one of the two PDFs carries them as text. The discriminator is not the text layer on its
own; it is the text layer *against the ink*, which is what `outlined-text-census.py` pairs.

## 4. The fitted bullet: taken as a **decision**, with the number that has been missing

Round 60 deferred this as an explicit decision and asked the next round to take it or say why in
the same words. **This round does not take it, and here is why in those words: it is fully
characterised and it measures almost nothing on this corpus.**

The claim is confirmed still live and unchanged at this tree. `Lepore.ppt` page 2, the marker run
and its own text line:

| | marker pen y | text baseline | offset |
|---|---:|---:|---|
| ours | 423.31 | 422.39 | **0.92 pt above** |
| reference | 421.43 | 422.42 | **0.99 pt below** |

1.91 pt, the same figure round 54 recorded. And `Lepore.ppt` is **0.53 of the track's 989.29**,
its page 2 is `diff% 0.82, |ink|% 0.00` — the *lowest* diff% of its first seven pages — and the
document is a `match`.

**What was missing was the reach, and it is here now.** `bullet-census.py` pairs every one- or
two-glyph run with the first longer run to its right on the same line, on both sides, over the
whole track:

```
  1458 pages pair markers on both sides in 302 documents
   251 of them are over 0.25 pt out, in 58 documents
   median |d| 0.056 pt   mean 0.230   max 5.003
```

Twenty-five pages on `wells08_basic.ppt`, 19 on `berlin.ppt`, 18 on `RESPA_-_Section_8_Webinar`,
13 on `010605Vul.ppt`, 8 on `Lepore.ppt`. **So the item is 251 pages and not one**, the median
page is exact to 0.056 pt, and a next round can now rank it against everything else instead of
inheriting a single-page anecdote. That is the thing seven rounds of carrying it never produced.

## 5. The 24.2.7.2 audit — two re-checks, both VERIFIED

```
open sites 37 in 30 files
   WordProcessing 11   Spreadsheets 9   Presentations 8   Text 5   Core 2   Rendering 1   Ooxml 1
markers 31 → 33   (VERIFIED 26 → 28, FIXED 4, WRONG 1)
```

The open count matches the dispatch exactly and does not fall, for the reason the file's own
header gives: a site still cites 24.2.7.2 in its prose after it is marked.

**`PptxTextStyles`'s `p:otherStyle` claim.** That a shape with text can never reach the master's
`p:otherStyle` — `isOther` is `!getTextBody() && …` — and takes the presentation's
`p:defaultTextStyle` instead. Re-run through the site's **own fixture**,
`tests/corpus/features/slide-other-style.pptx`, which states 12 pt magenta on `p:otherStyle` and
24 pt green on `p:defaultTextStyle`: 26.2.4.2 draws `0 0.5019607843 0 rg … 24.009 Tf`. The
presentation's style, in both size and colour, byte-for-byte the recorded answer. **VERIFIED —
and both C++ citations were wrong and are corrected**: `pptshape.cxx:494-499`, not 424-429, and
`slidepersist.cxx:315-345`'s `for (int i = 0; i < 4; i++)` whose `case 4` is
`maOtherTextStylePtr`, not the single line 315.

**`GlyphRun`'s washout mapping.** That a stated `a:lum` of exactly 70/−70 is thrown away and
replaced by `ColorMode_WATERMARK`'s fixed +50 luminance and −70 contrast. Re-measured on the
document that produced it: `N2_E_Maestroni_Swarm_COP.pptx`'s title slide renders at
`diff% 1.71, |ink|% 0.01`, and at 100 dpi its mean channel is **224.02 against the reference's
223.68** (MAE 2.23 over the page, 4.75 over the middle band). The rival reading — the stated
70/−70 through the same modifier — was measured at MAE **30.98** when the site was written, so
the page separates the two by more than an order of magnitude. **VERIFIED.** Its cases two and
three are explicitly *not* re-checked and the marker says so: no corpus slide states a lone
brightness or a non-washout pair, so there is nothing here to point a probe at. The sibling
`a:lum` claim in `DrawingFill.cs` was verified independently by the words track in round 61.

## The vision reading

Three pages, each chosen for a stated reason, each handed to a fresh subagent with the composed
image and nothing else — forbidden from reading any project file, source, or notes, and from
running any command; asked to describe each half alone before comparing, to give the direction of
every difference, and to say what looks identical. The halves were rasterised from the sweeps'
own PDFs.

### `001_advanced_powerpoint_bar__pptx` page 1 at 200 dpi, **after** the change

Chosen because it is the round's own target and because a blind reader is the only check on
whether a 2.70 pt correction actually reads as corrected.

The reviewer listed among the identical: **"Legend: same position, same order, same swatch size,
same labels"**, and **"All horizontal geometry … legend swatch x≈1080 … matching in both halves
to within my measurement precision (~1–2 px). Nothing is shifted left or right anywhere on the
page."** It also called out, unprompted, that there was **no font substitution** — "both titles
have the same distinctive humanist 'a' and the same total advance width".

Both of this round's changes, confirmed by a reader who had never seen either claim: the legend's
face and its order.

**And its loudest finding is refuted.** It ranked first "the bottom half carries roughly 120–130
px more blank page below the footnote", calling it a canvas-height difference. The two rasters
are **both exactly 2667 × 1500** and both have their **last inked row at 1379**. It named the
measurement that would settle it — "record the exact pixel width and height of each panel before
compositing" — and that is exactly the measurement that kills it. Sixth instance of *a whole-page
composite is not a ruler*.

Two of its smaller findings are open and unchecked: the callout box's border rectangle ~8 px
lower in the reference while its text does not move, and the plot body ~4 px lower with the tick
labels ~3 px higher.

### `Demick_JetBlue__pptx` page 4 at 200 dpi

Chosen because round 60 deliberately left its central claim unconfirmed and told this round to
build the instrument first.

The reviewer described **"24 category labels rotated ~45°"** along the bottom edge of **both**
halves, and listed among the identical **"Category labels: same 24 strings, same ~45° rotation
direction, same font and size, same x anchoring (first label ink starts x ≈ 272 in both)"**.

**That contradicted `rotruns.py`, and the reader was right and the instrument was wrong.** The
fill census settles it: 126 glyph-sized filled paths in the reference's label band, none in ours,
and 126 = 21 labels × 6 characters. §3 above is that finding, and it is worth two of the track's
word-gate failures.

It also reported, ranked first, that **"the top half's plot is ~35 px taller (≈11%)"** and that
the same 35 px reappears as extra space between the tick labels and the axis title in the
reference — "the two are the same phenomenon seen from opposite ends", which is the right reading
and is now the clearest open item on that document. And it reported for the **fourth independent
time** that the reference's legend keys carry marker symbols and ours do not.

### `Reporting_responsibilities_matrix__pptx` page 138 at 150 dpi

Chosen because it is the track's **second-largest document at 34.88 `abs_ink`** and no handover
has ever named it, so it is the one page in this round's reading that no prior claim could
contaminate.

Two leads, neither previously recorded:

- **A justified line breaks one word later in the reference.** Item (5) of row 2 fits
  "…See paragraph II.B. below." on one line in ours; the reference stretches the line to
  "…II.B." with word gaps 2–3× normal and drops "below." to its own line — 16 lines against our
  15. The reviewer named the discriminating measurement itself: measure a *non*-stretched shared
  line's ink extent on both sides, because a 0.3–0.5% advance difference over ~110 characters is
  enough and would rule the column width out. **Not yet checked** — and it is the advance
  divergence's signature.
- **The refresh icon's backing is opaque white in ours and blends into the header gradient in the
  reference, with a soft drop shadow.** A colour sample says the reviewer is measuring something
  real: white pixels are **25.5% of the icon's box on our side and 9.0% on the reference's**,
  with the reference's box mean darker on all three channels. A picture alpha or a picture-shadow
  difference; **confirmed as a difference, not yet attributed.**

Its third finding — that the reference's footer is clipped and unbanded — is a composite
artefact of the same kind as the first reviewer's: the two source rasters are 2000×1125 and
2001×1125.

## Refutations

1. **The brief's item 1, on what the surplus is.** Not legend arithmetic: the key's size, the
   key-to-text gap, the padding and the margin were all already right, and the reference's own
   `pdftotext` extents prove it — the widest entry's right edge agrees to **0.005 pt** while its
   width differs by 2.692. It is a **typeface**.
2. **Round 60's second legend defect — "a line series' legend key is 7.00 pt wide against the
   reference's 6.01" — does not exist.** `legend-census.py` was run with a floor low enough to
   admit the plot's own data markers, and on `003_advanced_powerpoint_line` those are what it
   measured: our markers are 7.00 pt square at x 410–537 and the reference's are 6.01 pt at the
   same places. The reference's actual legend key on that page is a **5.98 pt round marker at
   x 589.24**, centred in a 22.68 pt symbol slot exactly as `getPreferredLegendKeyAspectRatio`'s
   800 hundredths of a millimetre predicts — `580.90 + (22.68 − 5.98)/2 = 589.25`. Two separate
   real defects (a marker size and a missing legend marker) were fused into one wrong one.
3. **Round 60's rotated-label anchor hypothesis.** There is no anchor difference, because the
   reference draws no rotated text on that page at all — it draws outlines. The instrument
   problem round 60 named ("our rotation is in the CTM and theirs in the text matrix") was also
   not the instrument problem: `rotruns.py` solves that one and still reported zero.
4. **`rotruns.py`, by a blind reader.** The census this round built to settle item 3 was itself
   under-reaching, and the thing that caught it was a reviewer describing labels the census said
   were not there. **A vision reading refuted an instrument** — the reverse of the direction this
   file's calibration section usually records.
5. **This round's own reviewer, on a 120-px canvas difference.** Both rasters are 2667 × 1500
   with their last inked row at 1379.
6. **The brief's item 2's priority.** The gradient bars are real and are 53.3% of what is left on
   Pavese's page 8 — and that page is `|ink|% 0.23` of a 3.92 document, so the whole item is
   **0.12 of the track's 989.29** and costs a `Paint` through Core and four consumers on three
   tracks.
7. **`abs_ink` as the instrument, for the fourth round running.** Six documents worsened on
   unsigned ink while improving on differing pixels in the same sweep, and the round's whole
   `abs_ink` movement (−1.22) is a twentieth of its plot-rectangle movement's significance.

## Controls

| | base | final | predicted |
|---|---|---|---|
| `tf-agreement` mean | 0.77065 | **0.77065** | unchanged ✓ |
| exact `/Tf` pages | 1709 of 4515 | **1709 of 4515** | unchanged ✓ |
| sheared glyphs (reference 16008) | 15792 | **15792** | unchanged ✓ |
| pages whose sheared-glyph counts disagree | 82 | **82** | unchanged ✓ |
| page counts changed | | **0 of 302** | 0 ✓ |
| major pages | 364 | **364** | — |

`tf-agreement` reads **1709** exact-`/Tf` pages where round 60 read 1708 at its own base and
final. The extra page is not this round's: it is present at **this round's base too**, so it
arrived with the merged rounds 61. Round 56's 0.85188 remains the outlier and remains unexplained.

A **determinism check** was run after the sweep, per round 60's own lesson: three of its
documents re-rendered at the finished tree come back **byte-identical** to the copies the sweep
kept. `verify-test.sh` was run only while no sweep was in flight.

## Measured against the prediction

| # | predicted | measured |
|---|---|---|
| 1 | verdicts **0**, band −1 … +1 | **0**, 200 → 200 ✓ |
| 2 | page counts 0 of 302 | **0 of 302** ✓ |
| 3 | documents moved on differing pixels **36**, band 33 … 39 | **28** ✗ |
| 4 | `abs_ink` −1 … −10 | **−1.22** ✓ |
| 5 | differing pixels −20 … −140 | **−11.18** ✗ |
| 6 | `dRight` over 1 pt **27 → 8 … 14** | **10** ✓ |
| 7 | `dLeft` over 1 pt 9 → 9 ± 1 | **9** ✓ |
| 8 | controls unchanged | all four unchanged ✓ |
| 9 | the 17 pages' face mismatch → **0** | **0**, and the total 3363 → 3215 ✓ |

**The documents-moved band has now missed seven rounds running — and this time it missed
*downward*, which is new.** The eight it over-counted are named exactly: the four
`advanced_powerpoint_scatter` and four `advanced_powerpoint_bubble` decks. Their chart parts do
carry a `c:legend`, which is what `legendfamily-census.py` reads — and **not one of their series
carries a `c:tx`**, so neither stack has a name to put in a legend and neither stack draws one.
Both sides render eleven text runs on those pages and agree face-for-face on all eleven.

That is `COMMON.md`'s rule 6 — *estimate reach from what a shape resolves to, not what a part
declares* — landing on this round from the other side. The prediction listed four blind spots and
this was not among them.

**And the band was stated as a magnitude, not per direction**, which `COMMON.md` now forbids.
Stated properly the outcome was **28 improved, 0 worsened** on differing pixels, and **14 improved,
6 worsened** on unsigned ink.

Item 5 missed low for a related reason and one more: twenty-eight small movements of 0.05–1.10
each do not reach the −20 floor a thirteen-document round-60-sized change reached.

## Shared layers — this diff reaches all three tracks

* **§1** touches `XRay.Office.Core/Charts` (`ChartPlot.LegendFamily`, `ChartLayout.Legend`,
  `ChartLayout.AddLegend`) and `XRay.Office.Ooxml` (`DrawingChartPlot.LegendFamilyOf`).
* **§1b** touches `XRay.Office.Core/Charts` (`ChartPlot.LegendReversed`, `ChartLayout.Entries`).
* Nothing touches `XRay.Office.Vector`, `Text`, `Rendering`, `Markup` or `Containers`.

Census reach outside slides, counted on what the parts state:

| change | sheets | words |
|---|---|---|
| §1 the legend's face differs from the one-face answer | **36 documents** | **none** |
| §1b the legend's entry order reverses | **9 documents** | **none** |

Measured rather than argued, by sweeping each track whole at this tree and scoring the verdict
column against `MANIFEST.tsv`:

| track | passing over `MANIFEST.tsv` at this tree | manifest disagreements |
|---|---|---|
| **words** | **321 of 337** (337 of 337 visited) | **0** |
| **sheets** | **282 of 307** (307 of 307 visited) | **1 — and it is a gain** |

**Words is unchanged and clean**, exactly as the census said it would be: no `.docx` chart part
in the corpus has a legend whose face or order either census flags.

**Sheets is 282 of 307 and the single disagreement is `+1`.**
`064_Small_business_cash_flow_Use_this_template_0a640756.xlsx` is `open` in the manifest and
reads **`match`** at this tree — 508 words against the reference's 520 before, **510** after, and
the gate's band is `d > b × 0.02 && d > 3`, so 12 fails on a band of 10.4 and 10 passes.

**And it is this round's, measured with a base-source control rather than argued.** The two
strings that changed are `Cash flow projection` and `Cash on hand minimum alert` — the chart's
two series names, which are its legend entries — and each gains its clipped leading `ash`. Its
`xl/theme/theme11.xml` names **Arial** as the minor face while the part's first literal
`a:latin` anywhere is **Sitka Heading**, so the legend was being measured in the wrong face and
its box was the wrong width. The three changed sources were checked out at `337bc9fe17c`, the CLI
rebuilt, the document rendered — **508 words, `flow projection` and `on hand minimum alert`** —
and then restored and rebuilt, with the shipped state re-rendered to prove the restore
(`001_advanced_powerpoint_bar`, `stacked_bar_chart` and `064` all byte-identical to their sweeps'
own copies).

**A census bug found on the way, and it did not change the count.** `legendfamily-census.py`
looked for the theme at `theme1.xml`; two workbooks call theirs `theme11.xml`, and for those the
census read no theme at all and reported the legend's new face as *nothing*. Both were already
counted as changing, so the 33/36/0 reach figure stands — but the census was reporting the wrong
*answer* for the very document that produced this round's cross-track verdict. Fixed in the
committed script, with the reason written into it.

## Tests

Two new files, **23 new tests**, and the total reconciles: **5102 = 5079 + 23**.

| test | mutation | outcome |
|---|---|---|
| `ChartLegendFaceAndOrderTests` (17) | `measurer.For(plot.LegendFamily)` → `measurer` | **DETECTED**, 2 of 17 |
| `ChartLegendFaceAndOrderTests` (17) | the reversal gated on `Legend == None` | **DETECTED**, 4 of 17 |
| `DrawingChartLegendFaceTests` (6) | `LegendFamilyOf(…)` → `FamilyOf(chartSpace, theme)` | **DETECTED**, 3 of 6 |

Three mutations, three detected by reintroduction; neither class is a drift guard. The first
mutation attempt — `if (false) entries.Reverse();` — was rejected by `verify-test.sh` itself with
*"the mutated tree does not build — that is not a detection"*, which is the guard working: an
unreachable-code error is not a behaviour change.

Each class's inert cases are controls by design: a chart stating one face throughout, a legend
stating `+mn-lt` (a theme reference is not a face name), a chart stating nothing with no theme at
all, the column and area charts that must keep file order, a top or bottom legend on an unswapped
chart, a line chart whose `Direction` defaulted to `Bar` and must not be treated as swapped, and
a pie whose categories must not reverse.

Ten non-Fidelity projects, one at a time, at the final tree, `--no-build` so that nothing could
be rebuilt under a sweep:

```
Core 407   Containers 109   Text 624   Vector 302   Rendering 153(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1225   Spreadsheets 1020   Presentations 878     = 5102
0 failed
```

`cd dotnet && dotnet build -v q -nologo` → **0 warnings, 0 errors**.

## Left open, in the order the next round should take them

1. **The reference outlines 45°-rotated chart labels, and two of this track's word-gate failures
   are that and nothing else.** `Demick_JetBlue.pptx` 204 of 204 words, and
   `N2_E_Maestroni_Swarm_COP.pptx` 171 of 170. Both stacks draw the same glyphs. This is a
   **proposal for `MANIFEST.tsv`/the gate, not a code change**, and it belongs to the parent.
   `outlined-text-census.py` is the instrument. Three other surpluses do *not* have this shape:
   `OnTrac_StarCertificationProgram-3Day` p10 (+249), `16 - UTM - (NASA)` p7/p29, `Thailand17` p8.
2. **`Demick_JetBlue`'s plot area is 11% shorter in the reference**, and the same 35 px reappears
   as extra clearance between the rotated tick labels and the axis title. A reader found it and
   named the discriminator: measure one rotated label's ink bounding box on both sides — if they
   match, it is a reservation rule and not a font metric, and then axis-line-to-label-top against
   label-bottom-to-title-top separates a bbox computation from a padding constant.
3. **The fitted bullet is 251 pages in 58 documents**, not one, and the census that says so is
   committed. Median 0.056 pt, mean 0.230, max 5.003. `wells08_basic.ppt` 25 pages,
   `berlin.ppt` 19, `RESPA_-_Section_8_Webinar` 18, `010605Vul.ppt` 13.
4. **A side legend wraps its entry text at 30% of the available width and we never wrap it.**
   `lcl_placeLegendEntries` sets `TextMaximumFrameWidth` to `rRemainingSpace.Width * 3 / 10` for
   `ChartLegendExpansion_HIGH` (`VLegend.cxx:295-301`). Measured on
   `southern-classic…pptx` page 11: the reference breaks *Total Mainline Passenger Revenue* over
   two lines and we fit it on one. Small — three corpus documents have a side legend and a series
   name of 25+ characters, one of them on slides.
5. **The automatic marker cycle**, now sighted by a **fourth** independent reader, and this round
   measured the reference's legend key directly: a 5.98 pt round marker centred in the 22.68 pt
   line-key slot on `003_advanced_powerpoint_line` page 1. **And a marker *size* defect beside
   it**: our plot markers are 7.00 pt square where the reference's are 6.01.
6. **Pavese's gradient bars** — mechanism known, worth 0.12 of the track, cost is a `Paint`
   through Core and four consumers. §2 above has the numbers to rank it with.
7. **`Reporting_responsibilities_matrix.pptx`, 34.88 and never worked.** Two fresh leads from
   this round's reading: a justified line breaking one word later in the reference (the advance
   divergence's signature, and the reviewer named the measurement), and a picture whose backing is
   opaque white on our side and transparent on the reference's — 25.5% white pixels against 9.0%.
8. **`N2_E_Maestroni_Swarm_COP.pptx`'s `c:manualLayout`**; **`2015-Civil-Rights-Website-training.ppt`,
   29.64**; the 11 EMF face-name documents; `WmfReader.CreateFont`'s missing record bound;
   `wordArtVert`; the **`pitchFamily` family nibble** (product decision with the user, still
   open); the `.ppt` `cdirFont` (Escher 137), still deliberately unread; a scatter chart's series
   paint order (no corpus page can measure it); Pavese's `(548/621)` wrap; and round 59's three
   unchased leads on `010605Vul.ppt` page 9.
