# A chart's pattern-fill backdrop: measured, implemented, withdrawn

Measured 2026-09-28 against `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** —
with the tarball's five bundled font confounds moved aside. Corpus at `/home/user/sample-files`.

**The feature works and it does not pay yet.** The whole change is banked as `withdrawn.patch`
(492 lines, applies to `850adadcf`) and nothing in `dotnet/src` carries it. What blocks it is a
*hatch phase* rule that is shared with every slide shape's pattern fill and has a measurement of
its own behind it — so replacing it belongs to a round with a slides sweep, not to this one.

## 1. The reach, and why a census of the element is the wrong census

`ChartPlot.Background` and `ChartPlot.PlotBackground` are `Colour?`, so a chart whose chart area
or plot area states `a:pattFill` draws nothing there. Censused over the 947 corpus documents by
**parsing the chart parts and looking at the element the fill sits on**, not by grepping:

| where | noFill | solidFill | gradFill | pattFill |
|---|---:|---:|---:|---:|
| `c:chartSpace/c:spPr` | 161 | 113 | **1** | **1** |
| `c:plotArea/c:spPr` | 222 | 47 | 0 | **1** |

So the entire non-solid chart backdrop surface of this corpus is **three fills in three
documents**: `028_Unit_Circle_Chart_Optimized_Graph.docx` (chart area, `dkDnDiag`),
`065_Weight_loss_tracker.xlsx` (wall, `wdDnDiag`) and
`Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx` (chart area, a gradient).

**A grep finds 13 pattern fills in 6 documents and 11 of them are in `style1.xml`** — the chart
*style* part, which LibreOffice does not read at all. Counting the string would have quadrupled
the apparent reach and pointed four rounds at parts nothing consumes.

## 2. It is worth drawing: 295 of 298 strokes on one page

On `028` 26.2.4.2's page 1 carries **298 strokes, of which 295 are `#F2F2F2`** — the
`dkDnDiag` hatch of the chart area, `lt1 lumMod 95%` on white. We drew none of them.
`a:pattFill` is not the 8×8 bitmap its preset names: `oox/inc/drawingml/hatchmap.hxx` maps each of
the fifty-four onto a `drawing::Hatch`, and `DrawingHatchPresets` is already that table.

## 3. A Writer document draws a hatch at its distance in TWIPS

This is the round's transferable finding and it is not about charts.

`dkDnDiag` states `Distance = 50` hundredths of a millimetre — 1.4173 pt. 26.2.4.2 draws 028's
hatch **2.51 pt** apart. Over four one-attribute variants of that document's `a:pattFill/@prst`
(`variants.py`'s shape, rendered and read with `spacing.py`):

| preset | stated (1/100 mm) | lines drawn | drawn spacing (1/100 mm) | ratio |
|---|---:|---:|---:|---:|
| `dkDnDiag` (135°) | 50 | 295 | 88.4 | **1.768** |
| `wdDnDiag` (135°) | 100 | 148 | 176.3 | **1.770** |
| `ltHorz` (0°) | 50 | 180 | 88.4 | **1.764** |
| `dkHorz` (0°) | 25 | 347 | 45.9 | 1.836 |

`2540 / 1440 = 1.76389`. **A Writer document's drawing layer works in twips, so the number
arrives unconverted and one hundredth of a millimetre is drawn as one twip.** The angle does not
enter it; the fourth row is the same rule read off a 1.30 pt step.

**The other two tracks do not do it**, which is what makes this the unit rather than the chart.
On `slide-pattern-fill.pptx` this tree already draws 112, 70, 70, 70 and 28 hairlines against
26.2.4.2's 115, 71, 71, 70 and 28 with no correction at all; on `065`'s hatched wall the
reference draws 238 lines where the stated 100 predicts about 219. Both are 1.0 within the
measurement, not 1.76.

Confirmed the other way too: the same rule read out of VCL. `OutputDevice::ImplDrawHatch`
(`vcl/source/outdev/hatch.cxx`:178) takes the distance in *logic* units, and
`CalcHatchValues`'s `nDist = fround(nDist / cos(45°))` then gives `round(50/cos45) = 71` — and
**71 twips is 3.55 pt, which is exactly the corner step measured off the reference's own
operators.** In hundredths of a millimetre the same arithmetic gives 125, which is 3.5433 pt and
is not what the page holds.

With the correction the implementation drew **296 hatch lines against the reference's 295**.

## 4. What blocks it: the phase, and it is two rules rather than one

The lines are the right count at the right spacing in the right direction and land **in the
reference's gaps**. Read out of the raw content streams rather than from a bounding box — a
`pdf-ops.py` stroke record is a *box*, and for a 45° line its corners are its endpoints only for
one of the two diagonal senses, which is how a first cut of this read 295 strokes as 42 lines:

- ours, on 028: `299.4177 -81.3724 m  -224.1773 442.2227 l`, stepping `(+1.7678, +1.7678)`;
  the family's invariant `x + y` runs `218.045 + 3.5356k`.
- 26.2.4.2 (the `incell0` variant, so the chart is not displaced):
  `593.2 666.039 m 594.85 664.389 l`, stepping −3.55 in x along the top edge; `x + y` runs
  `1259.239 − 3.55k`.

A line through the chart rectangle's **top-left corner** has `x + y = 666.39`, and that is
`k = 167` exactly in the reference's family and `k = 126.81` in ours. So **the reference phases
this family on the rectangle's top-left corner and we phase ours on its centre**, 0.81 of a step
apart — and a hatch half a step out is worth less than no hatch to any pixel measure.

`CalcHatchValues` says so outright: `aRef = (!IsRefPoint() ? rRect.TopLeft() : GetRefPoint())`,
and for a positive shallow angle `nOffset` comes out zero so the first line passes through the
top-left corner, while for a negative one it is `nDist − (nYOff % nDist)`.

**But `Hatching.Family` deliberately reproduces `GeoTexSvxHatch`'s centred phase, and that has a
measurement behind it** — its own remark records `BMFE-06-03 (Gerflor)` page 3 scoring 3.28 of
unaccounted ink centred against 0.00 in phase. Checked again here on `slide-pattern-fill.pptx`'s
green `wdUpDiag` shape: the reference's family is `y − x = 1.842 − 3.9685k` and ours is
`1.546 − 4.009k`, which agree to **0.296 pt out of a 3.97 pt step**, and a line through that
shape's top-left corner is at `k = −24.5` — half-integer, so the reference is *not*
corner-phased there.

So there are two phases in evidence, the slides one is already right, and picking a single rule
needs the 65 pattern fills in 7 decks re-measured. That is the next round's, not this one's.

## 5. What it scored, which is why it is withdrawn

The two witnesses are the only ones that can be scored — the third is a gradient, which the
patch leaves collapsing to one colour because `DrawingGradient.Paint` needs the rectangle it
fills and a chart's frame is not known until layout.

| | `diff%` | `|ink|%` |
|---|---|---|
| `028` against the `incell0` reference, base | 14.42 | 3.58 |
| `028`, with the hatch (twips-corrected) | **14.43** | **3.95** |
| `065` page 1, base | 26.71 | 2.88 |
| `065` page 1, with the hatch | **26.70** | **3.24** |

`diff%` flat both ways and `|ink|%` about 0.36 worse on each, which is exactly what a correct
hatch half a step out of phase costs: the ink we add does not cover the ink we were missing.

***And 028 cannot be scored as it stands at all***, for the reason `dotnet/CLAUDE.md` already
records: its chart frame is 71.6 pt out of place, so page-fraction metrics measure the
displacement. Every figure above is against `out-incell0`, the one-attribute variant that puts
26.2.4.2's chart where this tree draws it. Against the authored reference the same two renderings
read 16.72 and 16.87.

## 6. What the patch contains, for whoever picks it up

`withdrawn.patch`, against `850adadcf`:

- `ChartPlot.Background` and `ChartPlot.PlotBackground` become `Paint?`; `ChartBox.Fill` with
  them. Three construction sites and seven consumption sites, plus ~20 test assertions that gain
  a `Paint.Solid`.
- `DrawingChartPlot.BackdropOf` reads `a:pattFill` through the existing `DrawingHatch` and falls
  through to `FillOf` for everything else; the ODF, BIFF and chartex readers wrap their colour.
- `Hatching.Fill(outline, paint, sink)` moves the hatch expansion into Core, where all three
  tracks reach it — `SlideDrawing.Fill` had the only copy and now delegates to it, which is what
  `Hatching`'s own note asks for.
- `FrameChart.InTwips` applies §3.

The suite is green with it applied (Presentations 1222, Spreadsheets 1492, WordProcessing 2189,
Core 612, Fidelity 10 failed / 542 passed — the standing baseline) and the solution builds with
0 warnings. What it needs before it ships is §4.
