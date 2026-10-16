# slides-r63 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`fc-match Calibri` → `Carlito-Regular.ttf`, base `43142b73ccf`, branch `wt-slides-r63`,
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`, `TMPDIR` on the host mount throughout.
`prediction.md` beside this file was committed as `0ece9314c69`, before anything was built or
rendered post-change.

## The baseline reproduces — on four instruments, and it exposes a fifth

| | briefed | measured at `43142b73ccf` |
|---|---|---|
| passing over `MANIFEST.tsv` | 200 of 302 | **200 of 302, 0 disagreements, 302 of 302 visited** |
| major pages | 364 | **364** |
| `abs_ink` | 989.29 | **989.29** (1083.43 raw — see below) |
| plot-rect `dRight` over 1 pt | 10 of 57 chart pages | **10 of 57**, and `dLeft`/`dBottom`/`dTop` **9 / 11 / 9** |
| the fitted-bullet census | 251 pages, 58 documents, median 0.056, max 5.003 | **identical, to the digit** |
| `tf-agreement` mean / exact `/Tf` pages | 0.77065 / 1709 of 4515 | **0.77065 / 1709 of 4515** |
| sheared glyphs | 15792 against the reference's 16008, 82 pages disagreeing | **15792 / 16008 / 82** |

**And 314 of the sweep's 315 documents agree with round 62's final `ink.tsv` to 0.00.**

### The one document that does not is a sweep race, and it is worth 94.14 `abs_ink`

`slides/done-005/ppt/ITE106-Chapter 4` read **100.00** where round 62's final read 5.86. It is
not the tree. That document exists on this case-insensitive mount under **`.ppt` and `.PPT`** —
one inode, one `MANIFEST.tsv` row — so two sweep workers render *the same file* and write to
**the same output name**, because the identity `look.py` computes (`…__ppt` / `…__PPT`) folds to
one path in the filesystem even though the harness treats them as two documents. One worker's
`pdf-image-diff.py` then reads a PDF the other is still writing.

Settled by isolation rather than by argument: re-rendering both halves alone and re-running the
comparison gives **5.86 / 3.03**, which is round 62's figure exactly, and the reference PDF
re-rendered alone is byte-identical to the sweep's copy **but for the 98 bytes of XMP `dc:date`**
that `CLAUDE.md` already documents. The **final** sweep did not hit the race — both spellings read
5.86 there — so the two sweeps are compared with the base's figure corrected, and the correction
is named rather than netted:

```
  base abs_ink        1083.43 raw   →   989.29 corrected
  base signed ink      739.68 raw   →   693.39 corrected
  base diff-pixels   19637.80 raw   → 19215.65 corrected
```

**This is a new failure mode for the instrument** and it is not the alias *counting* problem
`CLAUDE.md` describes — the counts were right and one document's *measurement* was garbage. Any
round comparing two `abs_ink` totals on this track should check `ITE106-Chapter 4` first.

## The whole round

| | base | final |
|---|---:|---:|
| passing over `MANIFEST.tsv` | **200 of 302** | **200 of 302** |
| page counts changed | | **0 of 302** |
| word counts changed | | **0 of 302** |
| verdicts changed | | **0** |
| `abs_ink` | 989.29 | **988.49 (−0.80)** |
| signed ink | 693.39 | 692.41 |
| major pages | 364 | **362 (−2)** |
| differing pixels over 4530 pages | 19215.65 | **19214.63 (−1.02)** |
| plot-rect `dRight` over 1 pt | 10 of 57 | **10 of 57** — and **0 of 57 plot rectangles moved at all** |

**Thirteen documents moved on differing pixels: 6 improved and 7 worsened.** Stated per direction,
because the magnitude alone conceals exactly the thing that matters:

| Δ differing pixels | document | before → after |
|---:|---|---|
| −0.50 | `scatter_chart.pptx` | 3.04 → 2.54 |
| −0.49 | `Demick_JetBlue.pptx` | 113.05 → 112.56 |
| −0.08 | `171128IPAP.pptx` | 268.51 → 268.43 |
| −0.03 | `FAAAIandtheArtandScienceofV&Vfinal.pptx` | 59.12 → 59.09 |
| −0.03 | `line_chart.pptx` | 1.01 → 0.98 |
| −0.02 | `RPA P4 - Advanced Material.pptx` | 54.31 → 54.29 |
| **+0.01** | `1_Country-Updates_DRC_English.pptx` | 28.90 → 28.91 |
| **+0.01 … +0.02** | `003/011/019/027_advanced_powerpoint_line.pptx` | ≈0.65 → ≈0.66 each |
| **+0.02** | `Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx` | 267.74 → 267.76 |
| **+0.05** | `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 105.93 → 105.98 |

On unsigned ink ten moved, **4 improved and 6 worsened**; `Demick_JetBlue` carries the whole
improvement at **6.88 → 5.93**.

**The four `advanced_powerpoint_line` decks are the round's own target and they worsened by a
hundredth each while becoming exact.** That is not a paradox and it is worth writing down: our
marker was 7.00 pt square where the reference's is 6.01, and the oversized one **contained** the
reference's entirely, so only the annulus differed. The correctly-sized one is still 0.41 pt to
the left of the reference's — the plot-position residue, untouched by this round — so it now
differs on a sliver at each side plus a differently-rasterised anti-aliased edge. **A correctly
sized mark that is slightly displaced can differ on more pixels than an oversized one that covers
its target**, and any round that decides a marker change on differing pixels alone will read the
fix as a regression.

## 1. The axis title under the plot is centred on the wrong rectangle and has no clearance

The brief's item 1. A round-62 reviewer said `Demick_JetBlue` page 4's plot area was "~35 px
taller (≈11%)" in our half and that "the same 35 px reappears as extra space between the tick
labels and the axis title in the reference — the two are the same phenomenon seen from opposite
ends". **Half of that is refuted and half of it is the finding, and they are not the same
phenomenon.**

### The plot heights are equal — refuted by two instruments

| `Demick_JetBlue` p4 | ours | reference |
|---|---:|---:|
| plot rectangle, from the gridline families | 145.69 … 603.41 × h **181.55** | 144.25 … 603.50 × h **181.76** |

**0.21 pt on a 181 pt plot, with 37 horizontal and 44/45 vertical gridlines matched on both
sides.** A blind reviewer given the composed page this round, asked the question directly, came
back independently with *"effectively the same height … any difference is ≤ ~3–5 px … nothing like
a 10 %/30 px squeeze"*, and checked it a second way off the 47 px tick pitch. Two instruments, one
of them a reader who had never seen the claim.

### What is real is the axis title, and it is 21.77 pt

`pdftotext -bbox` on the title `Year + (3rd Quarter)`:

| | left | right | centre | top |
|---|---:|---:|---:|---:|
| ours, base | 314.84 | 434.26 | **374.55** | 383.29 |
| reference | 294.39 | 411.17 | **352.78** | **389.79** |

The inner plot rectangle's centre is **374.55** — exactly where we drew it — and the chart frame
on the slide runs 36.00 … 669.60 pt, whose centre is **352.80**.

### The seat: `lcl_createTitle` is a *provisional* placement and a second pass overrides it

`createShapes2D` reserves the axis title's band with `lcl_createTitle`, and then, once the diagram
exists, calls **`changePositionOfAxisTitle`** on every auto-positioned axis title
(`ChartView.cxx:1996-1998`). Its `ALIGN_BOTTOM` arm is

```
X = diagramPlusAxes.X + diagramPlusAxes.Width / 2
Y = diagramPlusAxes.Y + diagramPlusAxes.Height + titleHeight / 2 + pageHeight * 0.02
```

`AddTitles` transcribed only the first pass: `area.X + area.Width/2` — the **inner plot**
rectangle — and `diagram.Bottom + height/2`, with no distance at all.

**The two passes do not even use the same constant**, which is what makes this easy to get wrong
from the source: `lcl_createTitle` uses a flat `nYDistance = 420` hundredths of a millimetre for
`TITLE_AT_STANDARD_X_AXIS_POSITION` (`ChartView.cxx:1067-1070`) and
`changePositionOfAxisTitle` uses `constPageLayoutDistancePercentage` — two per cent of the page's
height. Our `CategoryTitleGap` is the 420 and was right *for the reservation*; nothing carried the
two per cent to the pen.

Both halves check out with no free parameter. Demick's chart frame is 633.60 × 331.20 pt:

* the diagram rectangle is `frame ± 2%` less the two rotated axis-title bands, which is
  **78.83 … 626.77, centre 352.80** against the reference's measured **352.78**;
* `frame.Height × 0.02` is **6.62 pt** against a measured gap of **6.50**.

### What it did

| `Demick_JetBlue` p4 axis title | base | after | reference |
|---|---:|---:|---:|
| ink centre x | 374.55 | **352.81** | 352.78 |
| ink top y | 383.29 | **389.92** | 389.79 |

**0.03 pt across and 0.13 pt down.**

### And the reviewer's "same phenomenon from opposite ends" was wrong in a useful way

The extra clearance above the axis title is real; the shorter plot is not. They cannot be the same
phenomenon, because the plot rectangle **did not move at all** — 0 of 57 chart pages changed by
more than 0.02 pt on any edge. The title moved into space the reservation had already taken. That
is the failure mode that leaves the picture the right size, which is why no ink measure had ever
found it.

## 2. An OOXML marker's size is stated in the file and we ignored it

`TypeGroupConverter::convertMarker` makes a symbol `convertPointToMm100(c:marker/c:size)` square
and defaults to `mnMarkerSize( 5 )` (`typegroupconverter.cxx:652-654`, `seriesmodel.cxx:118`). We
drew `labelSize × 0.7`, which is a transcription of `VDataSeries::getSymbolProperties`' **unset**
250 × 250 default and comes to 7.00 pt on the 10 pt labels nearly every corpus chart uses.

`003_advanced_powerpoint_line.pptx` states `<c:symbol val="circle"/><c:size val="6"/>`. The
conversion is `o3tl::convert(6, pt, mm100)` = `round(6 × 2540 / 72)` = **212**, which is
**6.0094 pt** and not 6. Measured off the two PDFs' own filled paths:

| `003_advanced_powerpoint_line` p1, eight markers per series | side |
|---|---:|
| ours, base | **7.00 × 7.00** |
| ours, after | **6.01 × 6.01** |
| reference | **6.01 × 6.01** |

**This is the other half of round 62's refutation, landed.** That round showed the "7.00-versus-6.01
legend key" does not exist — its census' floor had admitted the plot's own data markers — and said
plainly that two real defects had been fused into one wrong one. The marker *size* was the real one
and it is now closed; the missing legend *key marker* is the other and is untouched (§5).

`ChartSeries.MarkerSize` is nullable and **null is not five points**: it means no `c:marker`
element reached the model at all, which is every ODF and binary chart, and those keep the 250. A
non-null default would have moved every `.ppt` and `.odp` chart in the corpus unmeasured.

## 3. The fitted bullet: **not implemented, and the reason is a refutation**

The brief said the reach now exists and demanded a ranking or an implementation, not another
deferral. Both are here. The reach reproduces exactly — **251 pages over 0.25 pt, in 58 documents,
median 0.056 pt, mean 0.230, max 5.003** — and it is **not a defect of the placement rule**.

### The rule is correct, measured on 26.2.4.2 over a 21-slide grid

`make-bullet-probe.py` builds one box per slide, `tIns="0"`, `anchor="t"`, `a:noAutofit`, one
bulleted paragraph of three lines joined by `a:br`, and varies exactly one thing per arm. The
bullet's baseline minus the text's, in points, positive = above:

| arm | ours | reference | |ours − ref| |
|---|---:|---:|---:|
| 12 / 20 / 24 / 40 pt text, Arial bullet | +0.595 / +1.049 / +1.276 / +2.211 | +0.652 / +1.134 / +1.333 / +2.183 | 0.057 / 0.085 / 0.057 / **0.028** |
| bullet face Times / Wingdings / Courier / Verdana | +1.219 / +3.203 / +2.679 / +1.049 | +1.276 / +3.232 / +2.722 / +1.134 | 0.057 / 0.029 / 0.043 / 0.085 |
| `buSzPct` 50 / 75 / 100 / 125 / 200 | +4.578 / +2.792 / +1.049 / −0.695 / −5.839 | +4.592 / +2.892 / +1.134 / −0.623 / −5.867 | 0.014 / **0.100** / 0.085 / 0.072 / 0.028 |
| `lnSpc` 80 / 100 / 150 / 200 % | +1.190 / +1.049 / +1.049 / +1.049 | +1.276 / +1.134 / +1.134 / +1.134 | 0.086 / 0.085 / 0.085 / 0.085 |
| text Times + bullet Arial, text Arial + bullet Times | +1.049 / +1.219 | +1.134 / +1.276 | 0.085 / 0.057 |
| **control: `a:buAutoNum`** | **+0.000** | **+0.000** | **0.000** |
| **control: `a:buNone`** | no bullet | no bullet | — |

**Twenty arms, maximum disagreement 0.100 pt, and the numbered-bullet control is exact on both
sides.** The arms discriminate: the bullet's own size moves the offset from +4.58 to −5.84 and
both stacks track it together; the numbered control separates the symbol rule from the baseline
rule and comes out at zero on both. The line-spacing arms are inert on both sides *equally*, which
is itself a result — EditEngine's `nFirstLineTextHeight` and `nFirstLineMaxAscent` move together
under `Prop`.

### So what are the 251 pages?

`bulletdetail.py` reports, per bad page, both sides' marker em, text em and offset:

```
  206 of the 251 pages are .ppt and 45 are .pptx;  33 of the 58 documents are .ppt
   82 pages have marker em == text em    median |d| 0.340   max 4.351
  169 pages have them differing          median |d| 1.149   max 5.004
   50 pages our marker's em differs from the reference's own    -- a SIZE defect, not placement
   33 pages our text run's em differs from the reference's      -- an autofit/rounding defect
```

and the constant the reference's own placements imply is **not a constant**: on `010605Vul.ppt`
alone it reads 0.3478 where the two ems are equal and 0.3939 where they differ, against
LiberationSans' hhea half-difference of 0.3467. **At least two other mechanisms are folded into the
251,** and 50 of the pages are a marker *size* difference where the placement rule has nothing to
answer for.

`Lepore.ppt` page 2, the demonstration page seven rounds have carried, is one of the 169: its
bullet em is 20.409 (`fround(847 × 0.85)`, the autofit scale on the unrounded height) against its
text em of 20.013 (`setRoundFontSizeToPt`'s whole point). Our placement is **−0.93 pt**, which is
what the transcribed rule predicts to 0.01, and the reference draws **+0.99**. The rule is
verified; something upstream of it on that `.ppt` is not.

**Ranked against items 1 and 3, honestly.** Item 1 is 21.77 pt on a page with a mechanism that
falls out of the source with no free parameter and reaches every chart drawing a bottom axis title.
The bullet is a **median 0.056 pt over 1458 paired pages**, its worst documents are all `match`,
and the item as carried is now known to be at least three different defects. **It should not be
taken as one item again.** The next round that wants it should take the *marker-size* half — 50
pages where our bullet's own em differs from the reference's, which is a `.ppt` autofit-scale
question and not a placement one.

## 4. Pavese's gradient bars, and the side legend's 30% wrap: **ranked, not re-derived**

Both were priced by round 62 and neither is re-measured here. The gradient is **0.12 of the
track's 989** against a `Paint` through `XRay.Office.Core/Charts`, `XRay.Office.Ooxml` and four
consumers on three tracks; the side-legend wrap (`VLegend.cxx:295-301`) is **three corpus
documents, one of them on slides**. Both rank below §1 and §2 on value-per-line-changed and below
the marker-size half of §3 on reach. Recorded so the next round inherits a ranking rather than a
rediscovery.

## 5. The vision reading

Three pages, each chosen for a stated reason, each handed to a fresh subagent with the composed
image and nothing else — forbidden from reading any project file, source or note, and from running
any command but the single `Read` of the image; asked to describe each half alone before comparing,
to give direction and magnitude for every difference, to state confidence, and to say what looks
identical. Halves rasterised from the finished sweep's own PDFs at 200 dpi.

### `Demick_JetBlue__pptx` page 4, **after** the change

Chosen because it is the brief's item 1 and because a blind reader is the only check on whether a
21.77 pt correction reads as corrected.

* **Both of the round's claims about this page were put to it directly and both came back.** On the
  plot: *"effectively the same height … ≤ ~3–5 px … nothing like a 10 %/30 px squeeze"*, with the
  47 px tick pitch checked as a second route. On the axis title: *"Horizontally: same place.
  Centres agree to within ~2 px … I see no horizontal shift."* Before the change that shift was
  21.77 pt, which is 39 px in the image the reviewer was given.
* **Confirmed by a second instrument**, both of them: the gridline census gives 181.55 against
  181.76 for the plot height and `pdftotext -bbox` gives 352.81 against 352.78 for the title.
* **The reference's legend keys carry marker symbols and ours do not — the fifth independent
  sighting**, and the reviewer named the glyphs: orange square, dark-red diamond, navy triangle.
  It also reports (medium confidence) that inside the plot the reference draws markers on all
  three series where we draw them on one. **Not yet checked**, and it is the strongest-shaped
  unimplemented reading this track has.
* Two new, unattributed: a **heavy vertical line at x ≈ 540** spanning the full plot height in our
  half alone (medium-high confidence), and orange gridlines visible across the reference's upper
  band and not ours (medium, and the reviewer said itself this could be paint order).
* It discounted its own header-artwork finding on the grounds that the label banner clips our
  half's top edge — a reviewer refuting its own reading, unprompted.

### `003_advanced_powerpoint_line__pptx` page 1, **after** the change

Chosen because it is the deck that produced the 6.01 measurement, and because the marker is the
thing `page-vision` warns is below the composite's reliable floor.

* On the markers: *"approximately 9–10 px in the top half and 9–10 px in the bottom half. I cannot
  separate them … I am explicitly declining to say one half's markers are bigger — at this
  resolution that would be a guess."* **That is the right answer and it is not a confirmation.**
  The change is 0.99 pt, which is 1.8 px in the image after compositing; the reviewer's stated
  floor is 1–2 px. The fill census is what confirms it, at 7.00 → 6.01 against 6.01.
* **The legend key markers again**, unprompted, on an unrelated deck: *"the bottom half draws a
  filled round marker centred on each legend line stroke; the top half draws a bare horizontal
  stroke"*. **Sixth sighting, second this round, second document.**
* **Its second-ranked finding is refuted.** It reported the plot area ~5–9 px shorter in the
  reference, at low-to-medium confidence, from the tick labels and the footnote. The gridline
  census on that page reads `dTop +0.00, dBottom −0.12` — the plot rectangle agrees to **0.12 pt**.
  What may be real is that its *tick labels* sit differently from ours while the gridlines do not,
  which is a different claim and is open.

### `Reporting_responsibilities_matrix__pptx` page 254

Chosen because it is the track's second-largest document at 34.88, has never been worked, and
because round 62 read **page 138** — so a recurrence here is two readers on two pages rather than
one page read twice.

**The lead recurred, and this time the reviewer's own proposed measurement settled it — against
the explanation both rounds would have assumed.**

The reviewer reported item (5) of row 2 wrapping **one word earlier** in the reference, with
"below." orphaned onto its own line and the remaining line stretched into justification: sixteen
lines against our fifteen, "(6)" pushed down one 29.7 px pitch. It then named two rivals — *text
metrics* (our glyphs a hair wider) against *frame geometry* (our column a hair wider) — and named
the measurement: take a line that is **ragged on both sides**, so justification cannot contaminate
it, and compare its natural width; then compare a **justified** line's right edge, which is the
column margin itself.

Run exactly as specified, on `(6) Loss of any part of the aircraft structure in flight.`:

| | ragged line width | justified right margin, 9 lines |
|---|---:|---:|
| ours | 334.12 … 585.07 = **250.95** | **952.79 … 952.83** |
| reference | 334.09 … 584.68 = **250.59** | **951.30 … 951.53** |

**Both rivals are real and the geometry one is four times the larger.** Our advance is +0.36 pt
over 250.95 — **+0.144%**, which is the advance divergence `CLAUDE.md` documents and nothing new —
while **our text column's right margin is 1.30 pt further right than the reference's**, with the
left edges agreeing to 0.03. Our (5) line ends at 952.63, inside our margin and **1.13 pt outside
the reference's**; at the reference's margin it would wrap, and it would wrap in the same place.

So the 34.88-ink document that no round has ever worked has a named, measured, one-sided defect:
**a table cell's right inset or column width, 1.30 pt, on the right edge only.** Round 62's own
page-138 lead is the same defect seen once already.

Its other findings: the refresh icon smaller and offset in the reference (round 62 measured a
different property of the same object on page 138 — 25.5% white pixels against 9.0% — so this is
**twice reported and still unattributed**), and a clipped footer that the reviewer itself flagged
as possibly a crop of the composite. It is: the composite runs off the end of the bottom half.

## Refutations

1. **The brief's item 1, on the plot area.** `Demick_JetBlue` page 4's plot rectangle is
   **181.55 pt against 181.76** — the reference's is 0.12% *taller*, not 11% shorter. Refuted by a
   gridline census over 37 + 44 matched gridlines and, independently, by a blind reader asked the
   question directly. **The clearance above the axis title is real and is not the same phenomenon**:
   the plot rectangle did not move on **any** of the 57 chart pages.
2. **The fitted bullet as a placement defect.** The rule is verified against 26.2.4.2 on
   **20 of 20 controlled arms to ≤0.10 pt**, with a numbered-bullet control exact at 0.000 on both
   sides. Its 251 pages are at least three mechanisms — 50 of them a marker *size* difference, 33 a
   text *size* difference — and the metric constant the reference's placements imply is not
   constant. Seven rounds of carrying "the fitted bullet's vertical placement, 1.9 pt" named the
   wrong object.
3. **This round's own reviewer on `003`'s plot height.** `dTop +0.00, dBottom −0.12`.
4. **The assumption both rounds would have made about `Reporting_responsibilities_matrix`.** The
   wrap is **not** the advance divergence. That is present at +0.144% and is a quarter of the
   cause; the column's right margin is 1.30 pt out and is the rest.
5. **`abs_ink` as the instrument, for the fifth round running.** `scatter_chart.pptx` is the
   round's largest improvement on differing pixels (−0.50) and its largest **regression** on
   unsigned ink (+0.13), in the same sweep.
6. **And differing pixels is not clean either, on this class.** The four
   `advanced_powerpoint_line` decks worsened by 0.01–0.02 while their markers went from 7.00 to
   exactly the reference's 6.01, because an oversized mark that *contains* its target differs on
   less area than a correctly sized one displaced by 0.41 pt.

## Controls

| | base | final | predicted |
|---|---|---|---|
| `tf-agreement` mean | 0.77065 | **0.77065** | unchanged ✓ |
| exact `/Tf` pages | 1709 of 4515 | **1709 of 4515** | unchanged ✓ |
| sheared glyphs (reference 16008) | 15792 | **15792** | unchanged ✓ |
| pages whose sheared-glyph counts disagree | 82 | **82** | unchanged ✓ |
| plot rectangles moved | | **0 of 57** | unchanged ✓ |
| page counts changed | | **0 of 302** | 0 ✓ |
| word counts changed | | **0 of 302** | — ✓ |
| major pages | 364 | **362** | — |

**Determinism check.** Four documents re-rendered at the finished tree — `003_advanced_powerpoint_line`,
`Demick_JetBlue`, `171128IPAP` and `ITE106-Chapter 4` — come back **byte-identical** to the copies
the final sweep kept, so no build swapped the binary under it. `verify-test.sh` was run **five
times and only while no sweep was in flight**; the cross-track sweeps were started after the last
of them finished.

## Measured against the prediction

| # | predicted | measured |
|---|---|---|
| 1 | verdicts **0**, band −1 … +1, passing stays 200 of 302 | **0**, 200 → 200 ✓ |
| 2 | page counts 0 of 302 | **0 of 302** ✓ |
| 3 | differing pixels: **14 … 20 improve, 0 … 4 worsen** | **6 improve, 7 worsen** ✗ **both halves** |
| 4 | `abs_ink` −0.5 … −6 | **−0.80** ✓ |
| 5 | `dRight` over 1 pt stays 10 of 57 | **10 of 57**, and 0 rectangles moved ✓ |
| 6 | Demick p4 title x → 352.8 ± 0.3, y-top → 389.9 ± 0.4 | **352.81 / 389.92** ✓ |
| 7 | `003` markers 7.00 → 6.01 ± 0.03 | **6.01** ✓ |
| 8 | controls unchanged | all five unchanged ✓ |
| 9 | the fitted bullet unchanged at 251 pages | **251** ✓ |

**The documents-moved band has now missed eight rounds running, and this is the first time it has
missed in both directions at once.** Stated properly it was **6 improved, 7 worsened**, against a
predicted 14–20 and 0–4.

### Why the improve half missed low, and it is the prediction's own blind spot 3

Nineteen documents were in the census union and **six of them cannot move**, every one for a reason
the prediction wrote down before the sweep:

| document | why it cannot move |
|---|---|
| `007/015/023/031_advanced_powerpoint_scatter.pptx` | `c:scatterStyle val="lineMarker"` with no data — **neither stack draws a single mark on the page**; a fill census finds 2 filled paths on ours and 3 on the reference's, none of them a marker. These are the same four decks round 62's legend census over-counted, failing a second time on a different property |
| `047_Female_Radar_Brain…pptx` | `c:radarStyle val="filled"` → `MarkerOf` returns `None`; no markers exist to resize |
| `3492.pptx` | both its stated axis titles are on the **`c:valAx` of a column chart**, so they are drawn *beside* the plot and this change does not reach them |

That is *"estimate reach from what a shape resolves to, not what a part declares"* landing for the
third consecutive round on this track, and this time the prediction had named it — blind spot 3
says the census "cannot tell whether any series draws a marker at all" and blind spot 1 says it
"cannot see which of the two axes ends up at the bottom". **Naming a blind spot is not the same as
bounding it**, and the band was not widened to account for the two it named.

### Why the worsen half missed high

Predicted 0–4 on the grounds that a marker can grow as well as shrink; measured 7, all of them
between +0.01 and +0.05. Six of the seven are §2's own decks, and the mechanism is refutation 6:
the metric rewards containment over correctness on a small filled mark. The seventh,
`8_P-Pavese_AIRBUS` at +0.05, is a chart whose markers are stated with a gradient fill and whose
axis title carries a `c:manualLayout` — the arm the prediction said would be over-counted.

## Shared layers — this diff reaches all three tracks

* **§1** touches `XRay.Office.Core/Charts` (`ChartLayout.AddTitles`).
* **§2** touches `XRay.Office.Core/Charts` (`ChartSeries.MarkerSize`, `ChartLayout.AddLines`,
  `ChartLayout.Plots.AddRadar`) and `XRay.Office.Ooxml` (`DrawingChartPlot.MarkerSizeOf`).
* Nothing touches `XRay.Office.Vector`, `Text`, `Rendering`, `Markup` or `Containers`.

Census reach outside slides, counted on what the parts state, with the over-counting above:

| change | sheets | words |
|---|---|---|
| §1 a chart part states an axis title | **13 documents** | **1** |
| §2 an OOXML line/scatter/radar/stock part with a marker | **11 documents** | **none** |

Measured by sweeping each track whole at this tree and scoring the verdict column against
`MANIFEST.tsv`:

| track | passing over `MANIFEST.tsv` at `43142b73ccf` + this diff | manifest disagreements |
|---|---|---|
| **words** | **323 of 337** (337 of 337 visited) | **0** |
| **sheets** | **280 of 307** (307 of 307 visited) | **0** |

Both match the parent's own figures at HEAD exactly, so the corpus stands at **803 of 946**.

**And the cross-track *ink* comparison is confounded and is reported as such rather than
attributed.** The only earlier cross-track sweeps available are round 62's, taken at `337bc9fe17c`
— three merges ago — so the 58 sheets documents that differ between them include everything rounds
61 and 62 landed. The check that can be made was: of the twenty-four sheets documents the two
censuses name, the ones that moved **all improved** (`019_Free_Blood_Sugar` −0.81,
`025`/`009_advanced_excel_bar` −0.23 each, `064_Small_business` −0.11, `021_Control_Chart` −0.10,
`055_Project_timeline` −0.07). And the largest sheets *regressions* in that list are provably not
this round's: `022_advanced_excel_scatter` (+0.12) states `<c:symbol val="none"/>`, so it draws no
marker at all and §2 cannot reach it. **A round's cross-track sweep is evidence about its own
worktree; the parent's gate at HEAD is the authority.**

## Tests

Two new files, **23 new tests**, and the total reconciles: **5147 = 5124 + 23** against the
parent's verified round-62 figure.

| test | mutation | outcome |
|---|---|---|
| `ChartAxisTitleAnchorAndMarkerSizeTests` (8) | `diagram.X + diagram.Width/2` → `area.X + area.Width/2` | **DETECTED**, 2 of 8 |
| `ChartAxisTitleAnchorAndMarkerSizeTests` (8) | the `frame.Height × PageMargin` term removed | **DETECTED**, 2 of 8 |
| `ChartAxisTitleAnchorAndMarkerSizeTests` (8) | `series.MarkerSize ?? …` → the fallback alone | **DETECTED**, 3 of 8 |
| `DrawingChartMarkerSizeTests` (15) | `defaultPoints = 5` → `7` | **DETECTED**, 5 of 15 |
| `DrawingChartMarkerSizeTests` (15) | `Math.Round(…, AwayFromZero)` → truncation | **DETECTED**, 5 of 15 |

Five mutations, five detected by reintroduction; neither class is a drift guard. A sixth attempt
was **rejected by `verify-test.sh` itself** — "the mutated tree does not build — that is not a
detection" — because the `sed` produced a type error rather than a behaviour change, which is the
guard working exactly as round 62 recorded it.

**One test was written as a control and turned out to be false, which is the more useful outcome.**
`ASymmetricChartPutsTheTwoCentresTogether` asserted that on an ordinary chart the plot rectangle's
centre and the diagram rectangle's coincide, so the two readings would be indistinguishable there.
They do not: a one-series column chart has its value-axis labels down the left and nothing down the
right, so the two centres are **11 pt apart on a 400 pt frame**. There is therefore **no chart on
which the two readings agree**, and every chart drawing a bottom axis title moves. The test is kept
as `ThePlainestChartTakesTheSameRuleAndItsTwoCentresDiffer`, asserting the inequality it found.

Ten non-Fidelity projects, one at a time, at the final tree, `--no-build`:

```
Core 415   Containers 109   Text 625   Vector 302   Rendering 153(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1231   Spreadsheets 1035   Presentations 893    = 5147
0 failed
```

`cd dotnet && dotnet build -v q -nologo` → **0 warnings, 0 errors**.

## The 24.2.7.2 audit

```
open sites 37 in 30 files          (round 62: 37 in 30 — unchanged)
markers 33 → 34   (VERIFIED 29, FIXED 4, WRONG 1)
```

**No new re-check was run this round, and the reason is that this track has none left to run.**
Every `24.2.7` site in `XRay.Office.Presentations` and `XRay.Office.Ooxml` already carries a marker:
`SlideAutofit.cs` ×4, `SlideDrawing.cs` ×2, `PptxSlideLayout.cs` ×2, `PptxTextStyles.cs`,
`DrawingFill.cs`. The nine files still carrying the bare string with **no marker at all** are
`XRay.Office.WordProcessing` ×4, `XRay.Office.Spreadsheets` ×3, `XRay.Office.Text` and
`XRay.Office.Rendering` — none of them this track's.

**And three of those nine are miscounted, in the direction that hides work.**
`SheetFonts.cs`, `SheetDeviceUnits.cs` and `SheetGeneralWidth.cs` appear in
`TODO.24-2-7-audit.md`'s outcomes table as *verified 26.2.4.2, 2026-08-21, round 53*, and the
re-check **is** written into each file — "Re-checked against LibreOffice 26.2.4.2 on 2026-08-21 and
correct", with the 45-of-45 and 27-of-27 figures. What is missing is the
`[24.2.7-audit: VERIFIED …]` marker the file's own rule counts progress by. So the marker count
under-reports the audit by three, on the very files whose write-ups are complete. **Reported, not
fixed** — they belong to the sheets track and editing them here would collide with a live round.

## Left open, in the order the next round should take them

1. **The legend key marker, now sighted by a sixth reader on a second document.** Round 62 measured
   the reference's key directly: a **5.98 pt round marker at x 589.24** on
   `003_advanced_powerpoint_line` page 1, centred in the 22.68 pt symbol slot exactly as
   `getPreferredLegendKeyAspectRatio` predicts (`580.90 + (22.68 − 5.98)/2 = 589.25`). With §2
   landed, the size is now known from the same place the plot's markers take it. **This is the
   strongest-shaped unimplemented reading the track has and it is one draw call.**
   Beside it: a reviewer reports the reference drawing markers on **all three** of
   `Demick_JetBlue` page 4's series where we draw them on one — unchecked.
2. **`Reporting_responsibilities_matrix.pptx`, 34.88 and never worked, now has a measured
   one-sided defect.** Its table cell's right margin is **1.30 pt further right on our side**
   (justified right edge 952.79–952.83 against 951.30–951.53, over nine lines, with the left edges
   agreeing to 0.03), and that alone accounts for the line that wraps one word later on **two
   different pages read by two different reviewers**. The advance divergence is also present at
   +0.144% and is a quarter of the size. Look for the cell's right inset or the column width.
3. **The `.ppt` half of the fitted bullet, which is the only part still unexplained.** Not the
   placement rule — §3 verifies that on 20 of 20 arms. **50 pages where our bullet's own em
   differs from the reference's**, and `Lepore.ppt` is the demonstration: bullet em 20.409 from
   `fround(847 × 0.85)` against text em 20.013 from `setRoundFontSizeToPt`, and the reference puts
   the bullet 1.92 pt below where that arithmetic says. `bulletdetail.py` is the instrument.
4. **`Demick_JetBlue` page 4's heavy vertical line at x ≈ 540**, drawn by us and not by the
   reference, spanning the full plot height — new, from this round's reading, unattributed.
5. **The marker's x position**, exposed by §2: our markers are now exactly the reference's size and
   still **0.41 pt to its left** on `003_advanced_powerpoint_line`. Small, but it is now the only
   thing left on those eight marks.
6. **Pavese's gradient bars** (0.12 of the track, a `Paint` through Core and four consumers) and
   **the side legend's 30% wrap** (`VLegend.cxx:295-301`, three documents). Both priced by round
   62, both ranked below the above, neither re-derived here.
7. **`2015-Civil-Rights-Website-training.ppt`, 29.64**; the 11 EMF face-name documents;
   `WmfReader.CreateFont`'s missing record bound; `wordArtVert`; the **`pitchFamily` family
   nibble** (product decision with the user, still open); the `.ppt` `cdirFont` (Escher 137);
   `N2_E_Maestroni`'s `c:manualLayout`; Pavese's `(548/621)` wrap; and round 59's three unchased
   leads on `010605Vul.ppt` page 9.

**And one for whoever runs a sweep on this track next**: check `ITE106-Chapter 4`'s row before
believing an `abs_ink` total. Two names, one inode, one output file, two workers.
