# slides-r63 — prediction

Committed before anything is built or rendered post-change. Base `43142b73ccf`, branch
`wt-slides-r63`, reference **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`, `TMPDIR` on the host mount.

## The baseline, reproduced first

| | briefed | measured at `43142b73ccf` |
|---|---|---|
| passing over `MANIFEST.tsv` | 200 of 302 | **200 of 302, 0 disagreements, 302 of 302 visited** |
| major pages | 364 | **364** |
| `abs_ink` | 989.29 (round 62's final) | **989.29 after one correction, 1083.43 raw** |
| bullet census | 251 pages / 58 documents, median 0.056, max 5.003 | **identical, to the digit** |

**The 94.14 correction is an instrument defect, not a tree difference, and it is named here rather
than netted.** `slides/done-005/ppt/ITE106-Chapter 4` exists on this case-insensitive mount under
`.ppt` *and* `.PPT` — one inode, one manifest row. Two sweep workers therefore render *the same
document* to **the same output filename** (`ITE106-Chapter 4__ppt.pdf`, since the identity folds
case in the filesystem even though `look.py`'s identity does not), and one worker's comparison
reads the file while the other is writing it. Re-rendering both halves alone and re-running
`pdf-image-diff.py` gives **5.86 / 3.03**, which is round 62's final figure exactly. Every other
document of the 315 agrees with round 62's final `ink.tsv` to **0.00**.

## The two changes

### A. The axis title that sits below the plot is centred on the wrong rectangle and has no clearance

`changePositionOfAxisTitle`'s `ALIGN_BOTTOM` arm (`ChartView.cxx:1012-1015`) is

```
X = diagramPlusAxes.X + diagramPlusAxes.Width/2
Y = diagramPlusAxes.Y + diagramPlusAxes.Height + titleHeight/2 + pageHeight * 0.02
```

and `AddTitles` uses `area.X + area.Width/2` — the **inner plot rectangle** — and
`diagram.Bottom + height/2`, with no distance term at all. Two terms, both measured on
`Demick_JetBlue.pptx` page 4 against the reference's own `pdftotext -bbox`:

| | ours | reference | our rect |
|---|---:|---:|---|
| axis title ink x-centre | 374.55 | **352.78** | inner plot centre **374.55**, diagram-rect centre **352.80** |
| axis title ink y-top | 383.29 | **389.79** | difference 6.50; `frame.Height × 0.02` = **6.62** |

Both terms land on the reference within 0.15 pt with no free parameter, and the horizontal one
lands on it to **0.02 pt**.

### B. An OOXML marker's size is stated in the file and we ignore it

`TypeGroupConverter::convertMarker` sets the symbol to `convertPointToMm100(c:marker/c:size)`
with `mnMarkerSize(5)` as the default (`typegroupconverter.cxx:652-654`, `seriesmodel.cxx:118`).
We draw `labelSize × 0.7`, which is a transcription of chart2's **unset** 250 × 250 default and is
7.00 pt on the 10 pt labels nearly every corpus chart uses.

`003_advanced_powerpoint_line.pptx` states `<c:symbol val="circle"/><c:size val="6"/>`;
`6 pt → 211.67 → 212` hundredths of a millimetre is **6.01 pt**, which is exactly what round 62
measured the reference drawing and exactly what it refuted as a *legend key* claim. The symbol
shape is already read; only the size is not.

## What I expect to change

Reach censuses committed beside this file. **By declaration**, on the manifest's own path list:

| | slides | sheets | words |
|---|---:|---:|---:|
| A: a chart part states a title on an axis | **8** | 13 | 1 |
| B: an OOXML line/scatter/radar/stock part with a marker | **16** | 11 | 0 |
| union on slides | **20** | | |

### The band, per direction

| # | prediction |
|---|---|
| 1 | verdicts **0**, band −1 … +1; passing stays **200 of 302** |
| 2 | page counts changed **0 of 302** |
| 3 | documents moved on differing pixels: **14 … 20 improve, 0 … 4 worsen** |
| 4 | `abs_ink` (corrected) **−0.5 … −6** |
| 5 | plot-rect `dRight` over 1 pt stays at **10 of 57** — neither change touches the plot rectangle |
| 6 | `Demick_JetBlue` page 4's axis title x-centre moves 374.55 → **352.8 ± 0.3**, y-top 383.29 → **389.9 ± 0.4** |
| 7 | `003_advanced_powerpoint_line` page 1's plot markers go 7.00 → **6.01 ± 0.03** |
| 8 | controls unchanged: `tf-agreement` mean, exact `/Tf` pages, sheared glyphs, major pages |
| 9 | the fitted bullet: **251 pages, unchanged** — nothing here touches it |

**Why 0 … 4 rather than 0 worsen.** B makes a marker *smaller* on the eighteen series that state
5 pt and *larger* on the fourteen that state 14, the six that state 18 and the one that states 62.
A larger marker on a chart whose reference draws a small one would be a regression, and I have
measured only two of those sizes against the reference. A band that cannot be violated by a
regression is not a control.

## What these censuses cannot see

1. **`axistitle-census.py` reads `c:catAx|c:valAx|c:dateAx > c:title` and nothing else.** It cannot
   see a `.ppt` chart's axis title (binary records), an ODF `chart:title` under `chart:axis`, or a
   title that a `c:delete` suppresses. It counts a title carrying a `c:manualLayout` as reach even
   though the reference **skips** `changePositionOfAxisTitle` for one (`mbAutoPosTitleX`), so on
   `171128IPAP.pptx` — 10 of its 14 stated axis titles are manual — the reach is over-counted.
2. **The horizontal half of A changes nothing on a symmetric chart.** `area` and `diagram` share a
   centre unless the plot is off-centre inside the diagram rectangle — which needs a secondary
   axis, a one-sided legend, or asymmetric labels. So the count of documents A actually moves may
   be well below 8, and a low prediction that comes true reads as well-calibrated.
3. **`markersize-census.py` over-counts by construction.** It matches a marker-capable chart type
   and a `c:marker` that is not `none` on the *part*, so it cannot tell whether any series draws a
   marker at all: a type group's `c:marker val="0"`, a `seriesFrameFormat` chart type where
   `convertMarker` returns immediately, and a per-point `c:dPt` marker are all invisible to it.
   This is round 62's own miss — a part *declaring* something its shapes do not *resolve to* —
   arriving in the same shape.
4. **Neither census sees ODF or binary charts, which must not move.** B is gated on the OOXML
   reader; an ODF or `.ppt` chart keeps the 250 × 250 default, and if it does not, that is a
   regression this prediction does not bound.
5. **Neither census sees a cascade.** A marker's size does not feed any reservation, and the axis
   title's *position* does not feed `DiagramAreaOf` (its *height* does, and that is untouched) —
   so I expect no page-count or word-count movement at all. If any appears, this bullet is where
   it was not foreseen.
6. **The cross-track figure is measured in this worktree**, at this base. Round 62's sheets gain
   did not survive at HEAD because a sheets round merged in between. The parent's gate at HEAD is
   the authority.

## Shared layers

Both changes touch **`XRay.Office.Core/Charts`** (`ChartLayout.AddTitles`, `ChartLayout.AddLines`,
`ChartLayout.Plots.AddRadar`, `ChartSeries.MarkerSize`) and **`XRay.Office.Ooxml`**
(`DrawingChartPlot`'s marker reading). Nothing touches `XRay.Office.Vector`, `Text`, `Rendering`,
`Markup` or `Containers`. The sheets and words documents are named above and the parent gates the
corpus.

## What this round deliberately does NOT do, with the number

**The fitted bullet is not implemented, and the reason is a refutation rather than a deferral.**
See `results.md` §3: a 21-slide probe deck puts the placement rule against 26.2.4.2 over four text
sizes, four bullet faces, five bullet sizes, four line spacings and a numbered-bullet control, and
**our placement agrees with the reference on 20 of 20 arms to ≤0.10 pt**. The rule is right. The
251 pages are a mixture of at least two other things, 206 of them on `.ppt`.
