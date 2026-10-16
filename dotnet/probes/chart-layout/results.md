# The chart plot area, and the label decisions that depend on it

Measured 2026-09-06 in `/home/user/wt-chartlayout`, branch `agent/chartlayout`, base `faadf7dda`.
Reference `/opt/libreoffice26.2/program/soffice` **26.2.4.2**, its Latin metric duplicates and its
Latin `NotoSans`/`NotoSerif` moved aside; `/usr/bin/soffice` is **24.2.7.2** and is what the gate's
own reference half was rendered with. Corpus `/home/user/sample-files`, 947 documents.
Ink is the mean absolute grey difference at 30 dpi, page for page, over the shared pages — the same
measure as `probes/words-apo-table/inkcheck.py`, with the reference render cached so that the same
forty-five documents can be scored three times over (`ink.py`).

The brief was `probes/chart-cat-reverse/results.md`'s five open faults. **Two are closed, one is
closed as a reading and cannot be seen until a third is, and two are left** — one of them with its
mechanism found and named, which is the round's most reusable result.

`N2_E_Maestroni_Swarm_COP.pptx` — the deck the brief is about — **moves from `words` to `match` on
the gate**, its page-7 ink from 38.53 to 34.98, and nothing anywhere in the corpus moves
backwards.

---

## 0. What the round found that the brief did not have

### `VDiagram::adjustInnerSize` is not the seat of §1, and it is never called on such a chart

The brief named `VDiagram::adjustInnerSize` as owning most of `N2_E_Maestroni_Swarm_COP.pptx`
page 7's residual, and it owns none of it. `c:layoutTarget val="inner"` reaches
`XDiagramPositioning::setDiagramPositionExcludingAxes`
(`oox/source/drawingml/chart/plotareaconverter.cxx`:510-538), which sets `PosSizeExcludeAxes`
(`chart2/source/controller/chartapiwrapper/DiagramWrapper.cxx`:816-823), which becomes
`CreateShapeParam2D::mbUseFixedInnerSize` (`ChartView.cxx`:946-980) — and **every one of the four
calls to `adjustInnerSize` in `impl_createDiagramAndContent` is guarded by
`!rParam.mbUseFixedInnerSize`** (`ChartView.cxx`:559, 594, 619, 690). A stated inner rectangle is
not refitted around the labels that overflow it. Nor is it reduced to a minimum first: the
`reduceToMinimumSize()` at `:559` is behind the same guard.

What it does get is one correction, and it is in the importer rather than in the view.

### The chart's primitives are fitted to their own drawn extent, not to the chart's page

This is the mechanism behind the brief's item 3 — *"26.2 scales the chart's primitives 0.950
vertically and 0.835 horizontally"* — and it is four lines of C++:

```cpp
// svx/source/sdr/contact/viewcontactofsdrole2obj.cxx:88-116
basegfx::B2DRange aChartContentRange;
aChartSequence = ChartHelper::tryToGetChartContentAsPrimitive2DSequence(xModel, aChartContentRange);
basegfx::B2DHomMatrix aEmbed(createTranslateB2DHomMatrix(-range.getMinX(), -range.getMinY()));
aEmbed.scale(1.0 / fWidth, 1.0 / fHeight);
aEmbed = aObjectMatrix * aEmbed;
```

`aChartContentRange` is `aRetval.getB2DRange(...)` — **the bounding box of every primitive the
chart's own draw page produced** (`svx/source/svdraw/charthelper.cxx`:96-100). It is normalised to
the unit square and stretched onto the OLE object's rectangle. So whenever a chart's labels
overflow its page, the *whole chart* — plot, bars, gridlines and type — is squeezed until the
overflow fits, by two different factors, one per axis. That is where the anisotropy comes from, and
it is a general rule rather than anything special about this deck.

**Checked arithmetically on page 7 and it accounts for the page exactly.** The chart's white
background rectangle is drawn at `119.083 … 719.660` × `92.58 … 516.983` inside a graphic frame of
`0 … 720` × `92.57 … 540.0`, which is `0.834135` across and `0.948534` down — the brief's 0.835 and
0.950, read off the frame instead of off the type. Its leftmost category label starts at
`x = 0.02`, which is `(0.02 − 119.083)/0.834135 = −142.7` in the chart's own coordinates: the
content range really does begin 142.7 pt left of the chart page, and 0.951 of an 8 pt label is the
7.609 pt the PDF states.

**XRay.Office already has the machinery.** `ChartLayout.Place` composes at `plot.Space` and calls
`Stretch(drawing, own, frame)`, which carries the residual `sx/sy` on `ChartLabel.Stretch` for each
consumer to fold into its own transform. The one thing missing is that LibreOffice's `from`
rectangle is the *drawn extent* and ours is the chart's page. Implementing it is a change to
`Place` and not to the renderers. It is left for a round of its own because it moves every chart
whose labels overflow, which is a corpus-wide reach this round could not have swept twice.

---

## 1. `c:manualLayout` — the position moves, the size does not shrink. **Closed.**

### The mechanism

`PlotAreaConverter::convertPositionFromModel` resolves the four fractions against the chart's page
(`LayoutConverter::calcAbsRectangle`, `oox/source/drawingml/chart/converterbase.cxx`:353-370) and
hands the rectangle to `DiagramHelper::setDiagramPositioning`
(`chart2/source/tools/DiagramHelper.cxx`:434-476), which converts it back to fractions, clamps each
of the four to `[0, 1]` with `lcl_ensureRange0to1`, and then:

```cpp
if( (aNewPos.Primary + aNewSize.Primary) > 1.0 )
    aNewPos.Primary = 1.0 - aNewSize.Primary;
```

**The position gives way; the width the file asked for is kept.** The rival reading — shrink the
size to `1 − x` — is what `lclCalcRelSize` does (`converterbase.cxx`:322-338), and that function is
reached by a *title's* layout and not by the plot area's.

### Measured

`N2_E_Maestroni_Swarm_COP.pptx` states `x = 0.20148`, `w = 0.82271`: 1.0242 between them, so the
plot ran **17.4 pt off the right edge** of a 720 pt frame. 26.2.4.2 draws the plot from
**225.581 to 719.660** on the page; dividing out the content-range fit of §0 puts its left edge at
`(225.581 − 119.083)/(719.660 − 119.083) × 720 = ` **127.65** pt in the chart's coordinates, and
`(1 − 0.82271) × 720 = ` **127.65**. No free parameter. Shrinking the width instead gives 145.06.

**A second witness, built rather than found, separates the two readings by 89 pt.**
`mkprobe3.py`'s `OVER.pptx` is a bar chart in a 590.4 pt frame at `x = 0.30`, `w = 0.85` — an
overrun of 0.15, sixty times Maestroni's. 26.2.4.2 draws its first value label `0` at
**135.5 … 141.0**, centred on the plot's left edge at 138.25. Moving the position puts that edge at
`50.4 + 0.15 × 590.4 = ` **138.9**; shrinking the width puts it at `50.4 + 0.30 × 590.4 = ` 227.5.

### Reach and cost

**4 documents, 5 chart parts** carry an inner `c:manualLayout` whose rectangle does not fit
(`census.py` over all 947): `N2_E_Maestroni_Swarm_COP.pptx`, `037_Personal_money_tracker`,
`034_Personal_net_worth_calculator` and `032_Business_expenses_budget` (two parts). 51 documents
state an inner manual layout at all, so 47 of them are untouched.

Three of the four are unmoved or improved. The fourth, `032_Business_expenses_budget`, is
**0.02 mean / 0.07 worst ink worse**, and it is the only regression in the round. Its overrun is
0.14% — `x + w = 1.00142` — so the correction moves that chart's plot **0.4 pt** left, and the
before/after page diff is exactly twelve category labels moving from a right edge of 515.7 to
515.3 against the reference's 515.9. That is smaller than our own tick-and-label-spacing model's
error on the same axis, and it is not separable from it at this resolution.

---

## 2. Label arrangement on a vertical category axis. **Closed.**

### It is not the horizontal rule on the other axis, and the source says so in one function

`canAutoAdjustLabelPlacement` (`chart2/source/view/axes/VCartesianAxis.cxx`:539-556) is the joint
prerequisite for auto-rotation *and* auto-staggering, and its last three lines are the whole rule:

```cpp
// automatic adjusting labels only works for
// horizontal axis with horizontal text
// or vertical axis with vertical text
if( bIsHorizontalAxis ) return !rAxisLabelProperties.m_bStackCharacters;
if( bIsVerticalAxis )   return  rAxisLabelProperties.m_bStackCharacters;
return false;
```

So a bar chart's category axis, whose text is horizontal, **never turns 45° and never staggers**.
The one move left to it is to thin the labels out.

Line breaking survives — `isBreakOfLabelsAllowed` (`:513-535`) returns `bIsVerticalAxis` for a
swapped chart — but **its limit is not the tick spacing**: `:768-773` replaces it with
`pTickFactory->getXaxisStartPos().getX()`, the whole band between the chart's own left edge and the
axis, and takes **no** five per cent off it.

### Measured, on decks built for it

`mkprobe2.py` builds a bar chart from a corpus deck with the category count, the label text and the
label size all swept, and renders each through 26.2.4.2 — **75 decks in all**.

| what was varied | renderings | result |
|---|---:|---|
| 8, 16, 24, 32, 40, 48, 56 categories, short names | 7 | every label upright, thinned to every 1st, 1st, 1st, 2nd, 2nd, 2nd and 3rd |
| 16/24/32/48 categories, 184 pt multi-word names | 4 | identical rhythms, **one line per label**, plot's left edge moved to fit |
| the same as one unbreakable 33-character word | 4 | identical again |
| 25–31 categories, to bracket the rhythm boundary | 7 | all drawn at a 11.848 pt slot, every second at 11.437 |
| 8/9/10/11/12/14 pt label size, n swept per size | 44 | see below |

Three facts come out of it and all three are asserted in `ChartVerticalAxisLabelTests`:

1. **No rotation and no staggering, in 15 of 15 renderings**, whatever the labels are.
2. **The rhythm follows the label's height along the axis, and that height is quantised.** The
   boundary brackets are (9.156, 9.410] at 8 pt, (10.672, 11.054] at 9, (11.437, 11.848] at 10,
   (12.812, 13.351] at 11 **and at 12**, and (16.029, 16.865] at 14. That 11 and 12 pt come out
   *identical* is the discriminator: a fixed fraction of the em would separate them by 1.15 pt, and
   the brackets for 9 and 10 pt do not intersect at all. It is `chart2`'s 96 dpi device, exactly as
   `probes/chart-vertical` found for the baseline pitch — `round(size × 96/72)` gives 15 pixels for
   11 pt and 16 for 12, and both round the ascent and descent to the same 17.
3. **The wrap limit is the band and it is its own fixed point.** On an automatically laid-out chart
   the plot gives up exactly the width the widest label needs, so the limit is never binding and
   nothing ever breaks — which is why all fifteen of those decks draw one line per label however
   long the names are. Give the same chart a `c:manualLayout` fixing the plot at 0.10 of the frame
   and the labels break onto four lines whose longest is **58.7 pt against the stated band's
   59.04**, and the axis thins to every 2nd, 3rd and 5th label at 8, 16 and 32 categories.

**A line that starts with punctuation is not a word break.** Three variants of the fixed-layout
deck — plain, `"Alpha Bravo - Charlie Delta 07"` and `"Alpha Bravo Charlie Delta [0007]"` — all
wrap identically, so `lcl_hasWordBreak`'s `nWordStart != nLineStart` does not fire on a hyphen or a
bracket beginning a line. That hypothesis was raised to explain page 7 and is refuted.

### What this does not close

**Page 7's own rhythm is still not the reference's, and the residual is not in this rule.**
26.2.4.2 draws 19 of its 55 category labels, on one line each, running off the left of the chart;
this now draws 11, wrapped onto two and three lines. Two other faults decide that number and
neither is in scope here:

- **the chart is squeezed** (§0), so the reference's category slot is 6.97 pt on the page where
  ours is 7.35, and its labels are drawn at 7.609 pt where ours are at 9.01;
- **one label size serves both axes** — `ChartPlot.LabelSize` — so this chart's category labels
  take the value axis' `sz="900"` where they should take the chart space's `sz="800"`, which is the
  fault the previous round listed fifth and left open. At 8 pt those labels wrap onto two lines and
  two lines of an 8 pt label is 19.5 chart pt against three slots' 22.04 — which is rhythm 3 and 19
  labels exactly. At 9 pt many of them reach three lines, and three lines is rhythm 5.

So the arrangement is right and it is being fed a label one point too large in a plot one twentieth
too tall. **Do not tune the arrangement to hit 19; fix the size and the fit.**

### Reach and cost

**25 documents** hold a chart with `c:barDir val="bar"` — 17 sheets, 8 slides. Only a crowded axis
can move, and only one of the twenty-five is crowded: **23 are unchanged in ink**, the
twenty-fourth (`032_Business_expenses_budget`) moved by §1 and not by this, and
`N2_E_Maestroni_Swarm_COP.pptx` goes from **49 drawn label lines to 21** against the reference's
19, worst-page ink **38.53 → 35.09**. That the other twenty-three do not move is the result worth
having: an arrangement that had never run on this axis could easily have thinned charts the
reference draws in full, and it thins none of them.

### The drawing half, which had never worked on a slide

A wrapped label arrives at the renderer as one string holding a newline.
`SheetChart` and `FrameChart` have split on it and anchored line by line since they were written;
**`SlideChart` measured the joined run**, so a right-aligned two-line label was placed a whole
line's width to the left of the axis it hangs from — page 7's first attempt drew them at
`x = −87.45`. `SlideChart.Measurer.Measure` now returns the widest line and the summed height, and
a broken label is built as one paragraph per line with the paragraph alignment carrying the anchor.
A single-line label takes exactly the path it took before.

---

## 3. `c:valAx/c:txPr/a:bodyPr rot`. **Read, and unverifiable until §0 is done.**

### The reach is one document, not nineteen, and the axis title is most of why

A first census counted 37 value axes carrying an in-range non-zero `rot` and 19 documents. That
count is wrong twice over.

**First, `<c:valAx>` contains `<c:title>`**, whose own `a:bodyPr` states the quarter turn that
stands an axis title on its side, and **35 of the 37 are that**. Read from `c:txPr` alone — the
axis' own tick-label text properties, with the title element removed first — the corpus holds
**two**: `N2_E_Maestroni_Swarm_COP.pptx` at `rot="-2700000"` (−45°) and
`047_Date_tracker_Gantt_chart` at `rot="-1800000"` (−30°).

**Second, one of those two is not a value axis.** `047_Date_tracker_Gantt_chart` is a
`c:scatterChart` — a scatter's *domain* is spelt `c:valAx` too, and this one carries `axPos="b"`,
so the rotation is on the bottom axis that `ChartPlot.DomainScale` carries and `AddDomainAxis`
draws. That path is untouched here, which is exactly why the document's ink does not move.

**So the reach is one document**: `N2_E_Maestroni_Swarm_COP.pptx`, a bar chart whose value axis
runs along the bottom.

The other 102 value axes state `rot="-60000000"`, a thousand degrees, which
`ObjectFormatter::convertTextRotation` throws away — the same out-of-range marker
`DrawingChartPlot.AxisTextOf` already handles.

### What was implemented

`ChartPlot.ValueAxisText` and `SecondaryValueAxisText`, read by the same `AxisTextOf` that reads
the category axis'; `AddValueAxis` turns a horizontal value axis' labels through it, anchored by
the same `Lean` correction the category axis uses for `lcl_correctRotation_Bottom`, negated for
`_Top`; and `PlotAreaOf` reserves the rotated shape's height as the band and half its rotated width
as the overhang. A stated rotation of zero takes every previous path unchanged.

### Why its one witness cannot be scored

On page 7 the labels are now turned, and they are drawn **below the bottom of the page**. That is
not the anchoring: the reference's date band is 37.5 pt deep on the page, which is 39.5 pt in the
chart's coordinates at 8 pt, and ours is 44.2 at 9 pt — the ratio is `9/8` to within a tenth of a
point, so the band is the right depth for the label size we are using. What puts it off the page is
that the reference's plot bottom is at 491.41 with 48.6 pt of frame below it and ours is at 513.09
with 26.9, and that difference is §0's vertical fit of 0.9485.

Ink moves **35.09 → 34.98** on page 7, and nothing at all elsewhere. So the reading is closed and
the drawing is not scoreable until the chart is fitted; that is stated rather than tuned, and it
is the one of this round's three changes whose value is a mechanism read correctly rather than a
page made better.

---

## 4. The category axis' crossing position. **Left, deliberately.**

Unchanged from `probes/chart-cat-reverse/results.md` §3, which established both the mechanism
(`ST_TickLblPos`' `nextTo` → `ChartAxisLabelPosition_NEAR_AXIS`, `axisconverter.cxx`:97-99, then
`VCartesianAxis::getLabelLineIntersectionValue`, `:1103-1113`) and that ours is our bug rather than
LibreOffice's. This round re-censused its reach with the same rule the previous round used —
`nextTo` on the category axis, `autoZero` on the value axis, and a negative value in a series —
and finds **5 documents**: `Demick_JetBlue.pptx`,
`southern-classic-kennesaw-state-university-final.pptx` (two parts),
`055_Project_timeline_with_milestones`, `031_Business_expense_budget` and
`061_Regional_sales_chart`. It is a feature — the category axis' line and its label line both move
— and it was not taken because §0 and the label size are larger and sit under it.

---

## 5. Where the five interact

Every one of the five is downstream of the same two numbers, and the round's ordering was wrong
until that was measured:

- **§0 decides the scale of everything else.** It changes the plot's slot, the label's drawn size
  and the room below the plot, so §2's rhythm and §3's band are both scored against a chart that is
  5% too tall and 20% too wide.
- **§1 changes what §2 has room for.** The manual-layout correction moves page 7's plot 17.4 pt
  left, which takes 17.4 pt off the band the category labels break in — so §1 makes §2 wrap harder,
  not less.
- **§2 and the label size compound.** One point of label size is the difference between two-line
  and three-line labels on page 7, and that is the difference between rhythm 3 and rhythm 5.

The order a later round should take them in is: the content-range fit (§0), then the per-axis label
size, then §4. Both of the first two are corpus-wide and neither can be scored while the other is
open.

---

## Verification

| | baseline at `faadf7dda` | after |
|---|---|---|
| `dotnet build XRay.Office.slnx -v q -nologo` | 0 warnings, 0 errors | **0 warnings, 0 errors** |
| ten non-fidelity projects, run individually and totalled | 5817 passed, 0 failed, 0 skipped | **5830 passed, 0 failed, 0 skipped** — the 13 are this round's own new tests |
| `XRay.Office.Fidelity.Tests` | 542 / 10 / 0 of 552 | **542 / 10 / 0 of 552**, and the ten names are the briefed ones: `PageDrawing` ×4, `TabStop` ×4, `SheetDrawing`, `JustificationShrink` |

Per project: Containers 109, Core 454, Markup 259, OpenDocument 128, Presentations 949,
Rendering 162, Spreadsheets 1113, Text 719, Vector 302, WordProcessing 1635. No project needed a
second run; nothing failed once.

### The corpus sweep, and what its `was` column is

`sweep.py` scores all three tracks against the gate's banked reference PDFs at `2f4709c08` — the
verdict rule reproduces **947 of 947** of that gate's own verdicts before it scores anything, so it
is the gate's rule and not a transcription of it. **But the `was` column is the gate's verdict at
`2f4709c08`, and this round's base is `faadf7dda`**, many commits later; a verdict that moves
between them is not necessarily this round's. Every mover is therefore attributed by re-rendering
it against a binary built with these five files copied back to their base contents
(`attribute.sh`).

**947 of 947 scored, 864 match against the gate's 860, and every one of the five movers moves
forward.**

| track | this round | gate at `2f4709c08` |
|---|---|---|
| words | 314 match, 21 pages, 1 pages+words, 2 words | |
| slides | **292 match**, 9 words, 1 unembedded | 291 / 10 / 1 at `faadf7dda` |
| sheets | 258 match, 2 pages, 6 pages+words, 41 words | |
| **total** | **864 match, 83 mismatch** | 860 / 87 |

| document | gate said | now | base binary | attribution |
|---|---|---|---|---|
| `N2_E_Maestroni_Swarm_COP.pptx` | `words` | **`match`** | 30 pages, **29088** glyphs | **this round.** 29088 against the reference's 28136 is 952 out and fails the band; 28100 is 36 out and passes |
| `AAC-AD-No-2021-01-Boeing-737-8…doc` | `pages` | `match` | 20 pages, 38761 glyphs | the page count is the era; the **3 glyphs** are this round, and they take it to 38764 against the reference's 38764 |
| `062_Run_chart` | `pages,words` | `match` | 2 pages, 643 glyphs — identical | not this round |
| `057_Simple_balance_sheet` | `pages,words` | `words` | 3 pages, 2185 glyphs — identical | not this round |
| `024_Unit_Circle_Chart_Colorful_Circles.docx` | `words` | `match` | 1 page, 557 glyphs — identical | not this round |

**Nothing moved backwards in 947 documents**, and the slides track — the one whose baseline the
brief states at `faadf7dda` — goes from **291 match to 292**.

## Files

| file | what it is |
|---|---|
| `census.py`, `census.tsv` | manual layouts that overrun, bar-direction charts, per-axis `bodyPr rot`, and category axes crossing inside the plot, over all 947 documents |
| `mkprobe.py`, `mkprobe2.py` | the bar-chart probe decks — category count, label text and label size swept, with and without a fixed inner size |
| `ink.py`, `compare.py`, `affected.txt`, `ink-before.tsv`, `ink-after.tsv` | the 45 corpus documents this round can reach, and their ink against 26.2.4.2 before and after. The list was built from the *first* census, before `valrot` was corrected from 19 documents to 2, so it is a superset of what the round can actually touch — which is the right direction for a regression check and is why 41 of the 45 come back unchanged to the hundredth |
| `mkprobe3.py` | the fixed-inner-size half of the vertical-axis probe, and the overrunning rectangle that witnesses §1 independently of any corpus document |
| `attribute.sh` | rebuilds with this round's five files returned to their base contents, so a moved sweep verdict can be attributed to this round or to the commits between `2f4709c08` and `faadf7dda` |
| `sweep-after.tsv` | all 947 documents, scored |
| `sweep.py` | all three tracks re-scored against the gate's banked reference PDFs; the verdict rule is validated against the gate's own two halves and reproduces 947 of 947 before it scores anything |
