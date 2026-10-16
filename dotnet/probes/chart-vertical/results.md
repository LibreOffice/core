# A chart's line height and ascent come off the same 96 dpi device, on all three tracks

**Round `chartvert` — 2026-09-06, base `465f27031`.** Measured against LibreOffice **24.2.7.2**
(`/usr/bin/soffice`) and **26.2.4.2** (`/opt/libreoffice26.2/program/soffice`, with its Latin
metric duplicates and its Latin `NotoSans-*`/`NotoSerif-*` moved aside so it resolves the system
faces), system fonts from `/usr/share/fonts`, corpus at `/home/user/sample-files`, repository at
`/home/user/wt-chartvert`, `XRay.Office.Cli` built from this worktree.

## The finding in one line

The horizontal half of `chart2`'s 96 dpi device landed on all three tracks one round ago. **The
vertical half was on the sheets track only**, and the same device decides it: a chart's line
height is

    hpx      = round(size_mm100 x 96 / 2540)                    -- the em in whole device pixels
    ascent   = round(asc  / upem x hpx)  device pixels
    descent  = round(desc / upem x hpx)  device pixels
    height   = max( length(ascent) + length(descent), length(ascent + descent) )

with `length(px) = round(px x 2540 / 96)` hundredths of a millimetre — 0.75 pt to the pixel — and
**no external leading**, because `IsAddExtLeading()` is false in EditEngine and a chart's label is
an EditEngine text. Round 60 established this on a *workbook*. This round establishes it on a
**deck and on a Writer document, on both binaries**, and implements it for the two tracks that
did not have it.

## Instrument A — baseline-to-baseline inside one label

`pitch.py` rewrites the chart title of `tests/corpus/features/chart-face-theme-minor.pptx` and of
`chart-bar-text.docx` as three lines joined by `<a:br/>`, at three faces × twelve sizes, renders
each through both binaries, and reads the three baselines out of the PDF's own `Tm`/`Td`. **Not
`pdftotext -bbox`**: that channel's quantum is several times the effect, and it is the trap that
cost this project four rounds.

**144 of 144 cases lie within 0.019 pt of the pixel law**, where scaling the face's own metrics
exactly is out by as much as **1.208 pt**:

| track | binary | n | mean residual | range |
|---|---|---:|---:|---|
| slides | 24.2.7.2 | 36 | −0.0012 | −0.0025 … −0.0005 |
| slides | 26.2.4.2 | 36 | −0.0013 | −0.0022 … −0.0003 |
| words | 24.2.7.2 | 36 | −0.0097 | −0.0185 … −0.0043 |
| words | 26.2.4.2 | 36 | −0.0097 | −0.0185 … −0.0048 |

A worked row, Liberation Sans, showing that the correction is a **sawtooth in the size** and not a
narrowing:

| sz | hpx | drawn pitch | pixel law | exact scaling |
|---:|---:|---:|---:|---:|
| 9 | 12 | 10.488 | **10.488** | 10.349 |
| 10 | 13 | 11.252 | **11.254** | 11.499 |
| 11 | 15 | 12.755 | **12.756** | 12.649 |
| 12 | 16 | 12.755 | **12.756** | 13.799 |
| 16 | 21 | 17.262 | **17.263** | 18.398 |

Three things fall out of the table and are worth keeping:

- **The two binaries agree on the vertical to 0.002 pt at every size.** 24.2.7.2's whole-pixel
  *position* snapping — the thing that separates the two binaries horizontally, worth a whole
  pixel every fifth glyph — has no vertical counterpart at all.
- **The words track carries a systematic deficit that grows with the size**, −0.004 pt at 7 pt to
  −0.019 pt at 24 pt, which is a flat **0.063%** and is the Writer frame drawing the chart
  slightly reduced. It is not a metric: the slides track, whose frame is 1:1, sits at −0.001 flat.
- **Carlito separates the leading term from the faces rather than by argument**: its `hhea` line
  gap is zero and both Liberation faces' is not, and the "with the gap" reading is out by 0.9 pt
  on Liberation Sans at 12 pt while being exactly right on Carlito.

## Instrument B — the offset from a tick to its label

The pitch fixes the *height*. The **ascent** is the other half, and it is what decides where a
label sits against the mark it belongs to. `tickoffset.py` reads a value-axis label's baseline and
**its own tick's `y`, straight off the PDF's `m`/`l`/`S` path operators** — the content stream
carries no `cm`, so both are stated in one space and the constant term vanishes instead of being
fitted. A value label is centred on its tick, so

    tick_y − baseline_y  =  ascent − height/2

| binary | n | mean residual | range | with exact scaling |
|---|---:|---:|---|---|
| 24.2.7.2 | 72 | **−0.0004 pt** | −0.020 … +0.013 | spread 0.997 pt |
| 26.2.4.2 | 72 | **+0.0279 pt** | +0.009 … +0.041 | spread 0.996 pt |

**24.2.7.2 needs no free parameter at all** — 72 of 72 inside 0.02 pt of a prediction with nothing
fitted, mean residual four ten-thousandths of a point. **26.2.4.2 needs one constant**, +0.0279 pt
= **0.98 hundredths of a millimetre**, one unit of the device's own map unit, and then 72 of 72
are inside 0.02 pt of it. That constant is the only vertical difference between the two binaries:
26.2.4.2 places a chart label's block one map unit lower than 24.2.7.2 does.

Exact scaling is not close on either: allowing it a fitted constant per binary still leaves it
right on **3 of 72**, against 72 of 72.

## Instrument C — our own renderer, before and after

`ours.py` runs the identical variant documents through `XRay.Office.Cli` and reads them with the
identical instrument.

| | pitch worst | pitch mean | tick offset worst | offset mean |
|---|---:|---:|---:|---:|
| slides, before | 0.879 | 0.279 | 0.385 | 0.143 |
| words, before | **1.196** | 0.432 | **0.647** | 0.230 |
| slides, after | **0.000** | 0.000 | **0.005** | 0.002 |
| words, after | **0.000** | 0.000 | **0.000** | 0.000 |

72 of 72 after, against 7 of 72 before. The residual against 26.2.4.2 is now that binary's own
+0.028 pt constant and nothing else, which is where 24.2.7.2 sits too.

**The two tracks were wrong in different ways and the slides one was not "no device".** The words
path scaled the face's metrics exactly *and kept the external leading* — the worst case in the
table. The slides path was already on a device: Impress's, `MetricGrid.Presentation`, 600 dpi in
1/100 mm. That is the wrong device rather than no device, which is why its error was smaller, and
it is also why the two are far apart only at some sizes — at 12 pt Liberation Sans, Impress's grid
stacks 13.436 pt where `chart2`'s stacks 12.756, and at 10 pt the two are 0.085 pt apart.

## What was implemented

- **`XRay.Office.Presentations`** — `SlideTextBody` gains a `Device`, defaulting to
  `MetricGrid.Presentation`, and `SlideTextLayout` reads its two `LineSpacing.Resolve` sites off
  it. `SlideChart.Measurer.Body` sets `MetricGrid.Chart`. Both `SlideTextLayout.Height`, which
  reserves the label's room, and `SlideTextLayout.Place`, which puts its baselines down, take the
  body — so the height a label is measured at and the baseline it is drawn on cannot come from two
  devices, which is the same discipline the horizontal half needed.
- **`XRay.Office.WordProcessing`** — `ChartFace` resolves its metrics on `MetricGrid.Chart`, which
  is one argument; `AscentAt` and `LineHeightAt` are both paths there already, and
  `ChartFace.Measure` is what `ChartLayout` reserves room from.

Nothing on the sheets track was touched: `SheetBandText.OnChartDevice` has done this since round
60, and every sheets test is unchanged.

**The height and the ascent had to move together and that is the whole reason this is one
change.** A chart label is drawn at `blockCentre − height/2 + ascent`, so an error shared by the
two disappears from a single-line label — which is why round 60's sheets defect went unseen while
every wrapped label and every fit test was wrong. It also sets a trap for a *test*: on
`chart-face-theme-minor.pptx`'s Liberation Mono at 10 pt, `chart2`'s device and Impress's differ
by 0.014 pt in that quantity, so a deck-level assertion on it passes either way. The unit test
therefore uses `chart-face-stated.pptx`, whose Liberation Serif puts them **0.043 pt** apart, and
asserts explicitly that the band it accepts *excludes* the 600 dpi answer.

## Reach, before and after

`corpus.py` renders the **78 slides and words documents** `probes/chart-text-metafile/reach.tsv`
found carrying a chart — the sheets 90 were already right — and records, for each, the page or
slide count, the number of text-showing operators, the number of *turned* text runs, and a
date-masked md5.

| | |
|---|---:|
| documents rendered | 78 of 78 |
| bytes changed | **61** (slides 51 of 68, words 10 of 10) |
| page or slide count changed | **0** |
| turned-run count changed | **0** |
| show-operator count changed | **0** |
| page count equal to 26.2.4.2's, before | 77 of 78 |
| page count equal to 26.2.4.2's, after | 77 of 78 |

**Nothing that decides how much text fits moved.** That was the risk worth checking and it is the
one a page-count gate cannot see: a taller line can push a label onto another line, make an axis
turn or thin its labels, or grow a legend. The show-operator count catches a wrap or a thinning —
either changes how many runs are drawn — and the turned-run count catches
`ChartAxisLabels.Resolve` reaching for its 45 degree rotation. Both are identical on all 78.
`ChartAxisWrapLimitTests` is unchanged too, and its decisions are width decisions in any case:
`Wraps` and `Wrap` measure `Measure(...).Width` against a fraction of the tick spacing, and the
height enters only `Collides` and `Depth`.

The one document whose page count differs from 26.2.4.2's — `ABCD-FE-01-00 Flight Envelope - v1
08.03.16.docx`, ours 14 against 16 — differs by the same 2 before and after, so it is not this
round's and is left where it was.

**The 17 documents that did not move draw no chart text at all**, and the reference draws none
either: checked on `008_advanced_powerpoint_bubble.pptx`, whose reference PDF holds eleven runs,
every one of them ordinary slide text, although the deck carries a chart part declaring 10 pt.
They are decorative chart decks — bubble, scatter, doughnut, battery and radar templates.

## Two probe-design notes, both of which cost a re-run

- **Our reader carries one axis-label size and one face for a whole chart**, taken from the first
  axis that states one (`DrawingChartPlot.AxisLabelSizeOf`, `FamilyOf`). Both are documented
  simplifications of the model, not defects — but a variant that names a size or a face on
  `c:valAx` alone therefore changes nothing on our side while changing the reference. The first
  cut of instrument C read Liberation Sans' ascent back from a Carlito variant, to the last
  thousandth of a point, because our renderer had drawn the *title's* face. `ours.py` names the
  face and the size on every axis of the part; the reference columns are unaffected, because the
  quantity is read against the tick rather than against the plot area.
- **Our PDF writer wraps every glyph run in `q 1 0 0 1 tx ty cm` and the reference writer emits no
  `cm` at all.** A tick read off the path operators and a baseline read off a `Td` are therefore
  in different spaces in our output and in the same space in theirs — worth a constant 42.52 pt on
  the fixture deck, which is more than a tick spacing, so the label/tick pairing silently went to
  the wrong tick before it was folded in.

## What this contradicts in `dotnet/CLAUDE.md`

1. **The vertical half was described as belonging to the sheets track and to 26.2.4.2.**
   `MetricGrid.Chart`'s remark cites round 60, which measured 39 pitches on one workbook against
   26.2.4.2 alone. **24.2.7.2 follows the identical vertical rule** — 72 of 72 here, and the two
   binaries' pitches agree to 0.002 pt at every one of twelve sizes on both a deck and a Writer
   document. The rule is not a property of the newer binary.
2. **Rule 3 says 24.2.7.2 "additionally snaps each glyph position to a whole 96 dpi pixel", and
   that is a horizontal statement only.** There is no vertical snapping in either binary; what
   separates them vertically is a single constant of **one hundredth of a millimetre** in where a
   label's block is placed, which was not previously recorded.
3. **Two file-level remarks stated the vertical rule wrongly and are corrected in place.**
   `FrameChart`'s class remark said a chart's line height is "the face's own ascent plus descent
   plus leading — 1.1499 em for Liberation Sans", and `SlideChart.Measurer.Body`'s said the same.
   Both halves are wrong: the leading is not in it, and the metrics go through a device, so it is
   not a fixed fraction of the em at all — 1.1254 em at 10 pt and 1.1596 at 11.
4. **`dotnet/CLAUDE.md` still gives the repository root as `/c/sandbox/workdir/libreoffice-core`
   and the corpus as `/c/sandbox/workdir/sample-files`.** They are `/home/user/...` here, as the
   previous round already recorded.

## Files

| | |
|---|---|
| `facemetrics.py` | a face's line metrics by `LineSpacing.Resolve`'s own precedence, and the 96 dpi device's answer at a size — the prediction, with no free parameter |
| `pitch.py` | three lines of one chart title, three faces × twelve sizes × two binaries × two tracks → `pitch.tsv` |
| `tickoffset.py` | a value-axis label's baseline against its own tick, read off the path operators → `tickoffset.tsv` |
| `ours.py` | the same two measurements through `XRay.Office.Cli` → `ours-before.tsv`, `ours-after.tsv` |
| `corpus.py` | the 78 chart-bearing slides and words documents: pages, show operators, turned runs, md5 → `corpus-before.tsv`, `corpus-after.tsv`, `corpus-ref262.tsv` |
