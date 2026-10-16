# A chart's text is measured on chart2's own 96 dpi device, and two of our three tracks did not

**Round `charttext` — 2026-09-06, base `8d9dae86d`.** Measured against LibreOffice **24.2.7.2**
(`/usr/bin/soffice`) and **26.2.4.2** (`/opt/libreoffice26.2/program/soffice`, with its bundled
Latin metric duplicates, its Latin `NotoSans-*`/`NotoSerif-*` and `opens___.ttf` moved aside so it
resolves the system faces), system fonts from `/usr/share/fonts`, `fc-match "DejaVu Sans"`
answering `DejaVuSans.ttf` and `fc-match "Liberation Mono"` answering
`LiberationMono-Regular.ttf`, and `XRay.Office.Cli` built from this worktree. Corpus at
`/home/user/sample-files`, repository at `/home/user/wt-charttext`.

## The finding in one line

A chart's labels are not laid out by Writer, Calc or Impress. `chart2`'s view builds them as
plain text shapes on the `VirtualDevice` that `DrawModelWrapper` creates from
`Application::GetDefaultDevice()` with `MapUnit::Map100thMM`
(`chart2/source/view/main/DrawModelWrapper.cxx`:88-99), and **that device is 96 dpi**
(`SvpSalGraphics::GetResolution`, `vcl/headless/svpgdi.cxx`:44). An `OutputDevice` instantiates a
font at a whole number of device pixels, so a 10 pt label is laid out at **13** pixels rather than
13.34 and every advance in it comes back **2.5% narrow**; at 11 pt the device sets 15 for 14.67 and
they come back 2.3% **wide**.

    scale = round(size_pt x 96/72) / (size_pt x 96/72)

That is the whole of the `TJ` adjustment of 16 at every inter-glyph position:
`600 x (1 - 13/13.34) = 15.4` thousandths of an em.

## The three briefed facts, re-established

`facts.py` renders `tests/corpus/features/chart-face-theme-minor.pptx` through both binaries and
through us, and `tabbed.docx` through both binaries as a Writer control.

**Fact 1 — the digit advance.** `pen("80") - pen("100")` on the value axis, the quantity
`SlideChartFaceComparisonTests` asserts:

| | size | pen `100` | pen `80` | digit | design |
|---|---:|---:|---:|---:|---:|
| 24.2.7.2 | 10.005 | 83.703 | 89.713 | **6.010** | 6.004 |
| 26.2.4.2 | 10.005 | 83.703 | 89.542 | **5.839** | 6.004 |
| ours, before | 10.006 | 40.847 | 46.828 | 5.981 | 6.005 |
| ours, after | 10.006 | 40.847 | 46.687 | **5.839** | 6.005 |

**Fact 2 — 16 at every position.** Every inter-glyph gap of every value label, read from the `TJ`
integers and stated as 96 dpi pixels, which is what makes the three stacks legible side by side:

| stack | adjustments on `180` | gaps, pt | gaps, px at 96 dpi |
|---|---|---|---|
| 24.2.7.2 | `[76, 0]` | 5.2426, 6.003 | **6.99, 8.004** |
| 26.2.4.2 | `[16, 16]` | 5.8429, 5.8429 | **7.791, 7.791** |
| ours, before | `[0, 0]` | 6.0038, 6.0038 | 8.005, 8.005 |
| ours, after | `[16, 16]` | 5.8429, 5.8429 | 7.791, 7.791 |

The design advance is 8.005 px; the device's is `0.60009766 x 13 = 7.801`. **24.2.7.2 additionally
snaps each glyph position to a whole pixel** — 7 or 8, never 7.79 — which is the difference between
the two binaries and nothing else. It is the subpixel-positioning change; 26.2.4.2 keeps the
sub-pixel position and so spreads one constant adjustment across every gap where 24.2.7.2 dumped a
whole pixel every fifth glyph.

**Fact 3 — the origins move, and the Writer control does not.** The chart's value labels sit at
89.713 under 24.2.7.2 and 89.542 under 26.2.4.2 (−0.171), and the `0` label at 95.722 against
95.410 (−0.312). On `tabbed.docx`, **35 of 39** runs are identical between the binaries to a
thousandth of a point; the 4 that move do so by exactly **0.05 pt — one twip** — and they are the
right- and centre-aligned ones. So the discriminator holds, but the honest statement of it is *the
chart moves by a fifth to a third of a point where Writer moves by at most a twip on a quarter of
its runs*, not *Writer does not move at all*.

## What the scale actually is: twelve sizes, two binaries, one control

`chart-gap.py` builds variants of the fixture deck changing exactly one thing — the chart title's
size, the graphic frame, or the `spc` — and reads the mean inter-glyph gap of a sixteen-glyph
monospaced title back out of each PDF. `chart-vs-textbox.py` puts the identical string in an
ordinary `p:sp` text box on the same slide of the same deck, so the two are drawn by one binary
from one document into one PDF and the only difference is which path laid the text out.

| sz | px96 | round | predicted | chart 24.2 | chart 26.2 | **text box 26.2** |
|---:|---:|---:|---:|---:|---:|---:|
| 7 | 9.335 | 9 | **0.96407** | 0.96329 | 0.96395 | 0.99373 |
| 8 | 10.658 | 11 | **1.03206** | 1.02917 | 1.02883 | 1.00284 |
| 9 | 12.019 | 12 | 0.99843 | 1.00017 | 0.99884 | — |
| 10 | 13.342 | 13 | **0.97439** | 0.97384 | 0.97517 | 0.99550 |
| 11 | 14.665 | 15 | **1.02287** | 1.02050 | 1.02050 | — |
| 12 | 15.987 | 16 | 1.00081 | 0.99884 | 0.99984 | — |
| 13 | 17.348 | 17 | **0.97994** | 0.98040 | 0.98106 | 0.99650 |
| 14 | 18.671 | 19 | **1.01763** | 1.01683 | 1.01539 | — |
| 16 | 21.317 | 21 | **0.98515** | 0.98284 | 0.98451 | — |
| 18 | 24.000 | 24 | 1.00000 | 0.99917 | 0.99984 | 0.99984 |
| 20 | 26.683 | 27 | **1.01186** | 1.01106 | 1.01095 | 1.00017 |

Ratio is the drawn gap over `hmtx x size`. The prediction has **no free parameter**: it is
`round(px)/px` at 96 dpi and nothing else. It is right on the sign at 12 of 12 sizes for both
binaries and within 0.003 of the magnitude everywhere. A sweep of the device resolution fits 96 at
max error 0.020 pt against 0.071 for the next candidate (168) and 0.097 for 240.

Three controls come with it:

- **The frame does not matter.** At 3.96, 5.94, 7.92 and 9.90 million EMU of chart width the drawn
  gap is identical to the last thousandth of a point in both binaries. The scale is a property of
  the stated size, not of the chart's placement or of any metafile scale.
- **The text box does not follow it.** At 7, 8, 10, 13, 18 and 20 pt the same string in a slide
  text box stays within 0.7% of the design metric while the chart beside it swings −3.6% to +2.9%.
  It is the chart path.
- **`spc` is not it.** `spc="-1"` and `spc="0"` give byte-identical output; `spc="100"` moves the
  gap by 0.98 pt, as a point of letter spacing should.

## On a real corpus workbook, with its own internal control

`measure.py` reads the drawn ratio as `1 - sum(TJ adjustment)/sum(declared /Widths)`, which needs
no font file, and reports only runs whose text is entirely digits and separators — an axis label
carries no kern pair in any of these faces, so every adjustment in one is the round trip.

On `sheets/chartset-001/xlsx/001_advanced_excel_bar.xlsx`:

| stack | size | runs | drawn | predicted |
|---|---:|---:|---:|---:|
| 24.2.7.2 | 10.008 (the chart) | 18 | 0.92743 | 0.97439 |
| 26.2.4.2 | 10.008 (the chart) | 18 | **0.97233** | **0.97439** |
| 24.2.7.2 | 10.998 (the cells) | 63 | **1.00000** | — |
| 26.2.4.2 | 10.998 (the cells) | 63 | **1.00000** | — |

The 63 cell runs in the same PDF are the control and sit exactly on the design metric. 24.2.7.2's
0.927 is its whole-pixel position snapping biasing a mean taken over one- and two-gap labels.

## What moved

`chart-vs-textbox.py`, re-run after the change, with the slide text box beside the chart as the
control that the change reached only the chart path:

| sz | predicted | 26.2.4.2 chart | **ours chart, after** | ours chart, before | ours text box |
|---:|---:|---:|---:|---:|---:|
| 7 | 0.96407 | 0.96395 | **0.96484** | 0.99984 | 0.99817 |
| 8 | 1.03206 | 1.02883 | **1.03150** | 0.99984 | 0.99817 |
| 10 | 0.97439 | 0.97517 | **0.97484** | 0.99984 | 0.99817 |
| 13 | 0.97994 | 0.98106 | **0.97984** | 0.99984 | 0.99817 |
| 18 | 1.00000 | 0.99984 | **0.99984** | 0.99984 | 0.99817 |
| 20 | 1.01186 | 1.01095 | **1.01239** | 0.99984 | 0.99984 |

Ours now agrees with 26.2.4.2 to **0.003 at every size**, against 0.006 to 0.036 before. The text
box column is unchanged, which is the assertion that nothing but a chart's text moved.

## The decision: we were wrong, and the tree already knew

**This is not a 26.2 regression and it is not a LibreOffice bug to decline.** Both binaries
quantise the em; 26.2.4.2 merely stopped *also* snapping positions, which moved it toward the
unquantised metric. Nothing in the 26.2 release notes, in `chart2`, or in any `tdf#` describes the
em quantisation as intended or as broken — it is the ordinary consequence of formatting against a
reference device, which LibreOffice does everywhere, with a coarse device.

And **`XRay.Office.Spreadsheets` has reproduced it since round 62.** `SheetBandText.ChartShape`
applies `MetricGrid.Chart.PixelEmScale`, `MetricGrid.Chart` is `new(96, false, MetricUnit.Mm100)`,
and `SheetChartDeviceMetricTests` already carries the vertical half of the same device from round
60. The horizontal rule's own remark records what not applying it cost: `003_advanced_excel_pie`'s
`M3` label came out 1.7 pt too wide and missed the best-fit inner placement by 0.33 of a degree.

So the seat is not in the metafile and not in `vcl`. It is that **the slides and words tracks were
left out of a rule the sheets track already had**, and `SlideChartFaceComparisonTests` is a slide
chart. Implemented:

- `XRay.Office.Presentations/Layout/SlideChart.cs` — a new `OnChartDevice` scales the runs
  `SlideTextLayout` produced, and **both** the measuring path and the drawing path go through that
  one call. They have to: the width the value axis' labels measure is what reserves the plot area
  and right-aligns them against it, and 24.2.7.2 is the worked example of getting that wrong —
  it reserves 18.012 pt for `100` and draws 17.249.
- `XRay.Office.WordProcessing/Layout/FrameChart.cs` — `ChartFace.Shape` scales its advances, which
  is the same two-line change round 62 made to `SheetBandText.ChartShape`; that one method is both
  paths there already.

Deliberately not taken: the reference's *further* rounding of each glyph's advance to a whole
hundredth of a millimetre. It is visible in the data — 26.2.4.2's gaps alternate 5.8429/5.8729 at
10 pt, one unit of 1/100 mm apart — and it is worth at most 0.014 pt a glyph.
`MetricGrid.PixelEmScale`'s remark already records the reasoning for leaving it out and this round
found no evidence to overturn it.

## Reach

`reach.py` walks all 947 `MANIFEST.tsv` rows, finds every document carrying a chart, reads the text
sizes the chart declares and reports the worst per-glyph error the rule predicts on each.

**168 of 947 documents carry a chart: sheets 90, slides 68, words 10.** 167 are OOXML chart parts
and one is an OLE2 chart storage. By worst predicted per-glyph error:

| |0| | documents |
|---|---:|
| < 0.5% | 35 |
| 0.5 – 1% | 2 |
| 1 – 2% | 20 |
| 2 – 3% | 106 |
| ≥ 3% | 5 |

**131 of the 168 are at 1% or worse and 111 at 2% or worse.** 10 pt is much the commonest chart
text size and sits at −2.56%; the worst are four documents at 8 pt (−3.21%). The 90 sheets
documents were already right before this round; the **78 slides and words documents are what
moved**.

## Gate

Measured at this worktree's HEAD, reference `/usr/bin/soffice` = **24.2.7.2**:

| batches | total | match | mismatch |
|---|---:|---:|---:|
| `slides/chart*` | 140 | **140** | 0 |
| `words/chart*` | 137 | 134 | 3 |
| `slides/done-*` | 144 | 142 | 2 |

**None of the five mismatches carries a chart** — confirmed by the census and, for the three
`.pptx`/`.docx` that matter, by `unzip -l | grep -c charts/chart` returning 0 — so none can be this
change. Two of them (`087_Printable_Graph_Paper_Template_Green_Theme_0c14ea04.docx`,
`097_Business_Case_Template_Elegant_Layout_3ba9cbf2.docx`) are the version gap: both render **2
pages under 26.2.4.2**, which is what we render, against the 24.2.7.2 gate's 1.

## What this contradicts in `dotnet/CLAUDE.md`

Rule 3's closing paragraph on `SlideChartFaceComparisonTests` is wrong in three specific ways, and
each was load-bearing for the brief this round was given:

1. **"The seat is in the metafile a chart is drawn into and replayed from."** There is no metafile.
   `ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithParameters` takes a chart's content as
   *primitives* straight from the chart's own draw page
   (`ChartHelper::tryToGetChartContentAsPrimitive2DSequence`), and the quantisation happens when
   `chart2` measures the text, long before any playback. The frame sweep is the measurement that
   rules a metafile scale out: four chart widths, identical output.
2. **"`tdf#168002` and `GetSubpixelPositioning` are the leads."** They explain the *difference
   between the two binaries* — 24.2.7.2's whole-pixel position snapping — and none of the 2.5%.
   Removing that snapping made 26.2.4.2 better, not worse.
3. **"The chart's `Tm` origins move between the two binaries where a Writer document's do not."**
   The chart's move; a Writer document's move too, on 4 of 39 runs of `tabbed.docx`, by one twip
   each. The claim needs the magnitudes to mean anything.

The brief's own framing — *"we and 24.2 both sit on the design metric and only 26.2 departs, which
is suggestive of a regression in 26.2"* — is refuted by the twelve-size table: 24.2.7.2's mean
advance follows the same 96 dpi rule at every size. Its 6.010 reading in the fidelity test was not
agreement with the design metric but an internal inconsistency, right-aligning on one width and
drawing at another.

`dotnet/CLAUDE.md` also still gives the repository root as `/c/sandbox/workdir/libreoffice-core`
and the corpus as `/c/sandbox/workdir/sample-files`; in this container they are `/home/user/...`
and there is no `/c/sandbox` at all. The `grep -r` doubling it warns about does **not** occur on
`/home/user/sample-files` — `find` and `git ls-files` both count 963.

## Files

| | |
|---|---|
| `pdftext.py` | reads a LibreOffice PDF's text operators back as glyph pens: `Tm`/`Td` origins, `Tf` sizes, `TJ` adjustments and the gaps they imply |
| `facts.py` | the three briefed facts, on the fixture deck, with `tabbed.docx` as the Writer control → the tables above |
| `chart-gap.py` | twelve sizes, four frame widths and three `spc` values, three stacks each |
| `chart-vs-textbox.py` | the control: the same string in a chart and in a slide text box, one deck, one PDF |
| `measure.py` | the drawn ratio on real corpus documents from `TJ` and `/Widths` alone, restricted to unkerned numeric runs |
| `reach.py` | the corpus census: which documents carry a chart, and the error predicted at their declared sizes |
