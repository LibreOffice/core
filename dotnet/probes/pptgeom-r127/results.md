# Escher against DrawingML, preset by preset — one table is a mirror and the adjustment conversion is not a constant

Round 127, `/home/user/wt-pptgeom`, branch `agent/pptgeom`, base `bdfa0c947`, 2026-09-14.
Two seats out of round 124's ink hunt — **O73** (a stored `adjustValue` is discarded for every
preset but two) and **O74** (Escher's `mso_sptTrapezoid` and DrawingML's `trapezoid` are vertical
mirrors) — plus the census round 124 left open beside them: *is any other Escher preset a mirror
of the DrawingML preset of the same name?*

| | |
|---|---|
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| our renderings | `dotnet/tools/XRay.Office.Cli` built in this worktree, before and after, from the same working tree |
| C++ tree | `/home/user/libreoffice-core`, read only (C8: it is `27.2.0.0.alpha0+`, **not** 26.2's source, so every arm below is measured a second time against 26.2.4.2's own output) |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything in this round** |
| population for the census | the **148** `MSO_SPT` values `PptShapeGeometry.PresetOf` maps to a DrawingML preset name (142 distinct names) |
| population for reach | round 124's `escher-adjust.tsv`: **158** shapes in **10** of the 49 passing `.ppt` state `DFF_Prop_adjustValue` |

The order was fixed by the brief and it mattered: **the census first**. Had several presets been
mirrored, O74's fix would have had to be a table; the census says exactly one is, so a table with
one row in it is the honest shape — and the same instrument, extended by one axis, then answered
O73 far better than reading the two definitions side by side could have.

---

## 0. The instrument: make 26.2.4.2 expand the Escher table for us

The comparison this round needs is *"what does the reference draw for `MSO_SPT` N"* against
*"what does this tree draw for the preset it maps N to"*. Reading the first out of
`EnhancedCustomShapeGeometry.cxx` is a C8 reading of a 27.2 alpha. Getting it out of the binary is
better, and there is a channel that does it without authoring a single `.ppt`.

`SdrObjCustomShape::MergeDefaultAttributes` (`svx/source/svdraw/svdoashp.cxx`:814-900) fills a
custom shape's `Coordinates`, `Equations`, `ViewBox`, `TextFrames` and `AdjustmentValues` from
`GetCustomShapeContent(eSpType)` whenever the shape states a **type** and no path of its own, and
`SvxShape::setPropertyValue("CustomShapeGeometry")` (`svx/source/unodraw/unoshap2.cxx`:1749)
calls it on every ODF import. `EnhancedCustomShapeTypeNames`' table gives the ODF `draw:type`
string for each of `MSO_SPT` 0-202. So:

```xml
<draw:custom-shape svg:width="4cm" svg:height="4cm" svg:x="…" svg:y="1cm">
  <draw:enhanced-geometry draw:type="trapezoid"/>
</draw:custom-shape>
```

`census.fodg` is 148 of those in one row, 30 cm apart so no callout's tail can reach its
neighbour's cell, drawn `draw:fill="none" draw:stroke="solid"`. `--convert-to svg` then hands back
the **evaluated** outline in page coordinates, curves already flattened:

```
$ /opt/libreoffice26.2/program/soffice --convert-to svg census.fodg
<path fill="none" stroke="…" d="M 1000,1000 L 5000,1000 4000,5000 2000,5000 Z "/>   # trapezoid
```

Ours comes from `probes/pptgeom-r127/PresetDump`, which calls `SlidePresetGeometry.Of(name, size)`
and prints the path. Both sides are normalised into the placement rectangle (**not** the drawn
bounding box, which a callout's tail moves) and compared by symmetric Chamfer distance in units of
the box side, under the identity and all three reflections. A match is a mean Chamfer distance
within **0.010** — one per cent of the box.

Three properties of the instrument worth stating, because two of them were wrong first:

- **Stroked subpaths only, on both sides.** `Geometry.Outline` merges every subpath; the reference
  leg is a stroke-only render, and Escher marks a subpath `0xab00 NO STROKE` for exactly the shapes
  DrawingML marks `stroke="false"`. Comparing `Outline` against a stroke-only render scored
  `callout1`'s rectangle as a difference that does not exist. `Geometry.StrokeOutline` is the
  like-for-like column, and moving to it took the "identical" count from **81 to 92**.
- **A square box.** Most DrawingML guides are a fraction of `ss = min(w, h)` and every Escher
  vertex is a fraction of a 21600 view box scaled anisotropically onto the shape. In a square box
  `ss = w = h` and that whole family of differences cannot enter; §2 then puts the aspect ratio
  back deliberately, which is how the trapezoid's second defect was separated from its first.
- **A reflection and a half-turn are the same map on a symmetric shape**, exactly as round 124's
  blind reader warned. A vertical mirror and a 180° turn score identically on the trapezoid
  (0.0000 both), and `heart`, `noSmoking`, `wave` and nine others score their mirrors identically
  to the identity because they *are* symmetric. The classifier therefore tests the identity first
  and only then a reflection, so a symmetric shape can never be reported as mirrored.

---

## 1. THE CENSUS — 1 of 147 Escher presets is a mirror of the DrawingML preset of its name

`preset-census.tsv`, 148 rows. Base rate, per C9: the denominator is **every** type mapped by
name, not the ones that looked suspicious.

| verdict | count | of 148 |
|---|--:|--:|
| **identical** — the two draw the same shape at their own defaults | **92** | 62.2 % |
| **differs** — no reflection accounts for it | **50** | 33.8 % |
| **fits a reflection** | **5** | 3.4 % |
| **no Escher geometry at all** | **1** | 0.7 % |

The five that fit a reflection:

| MSO | preset | fit | identity | mirror-V | mirror-H | rot-180 |
|--:|---|---|--:|--:|--:|--:|
| **8** | `trapezoid` | **mirror-V** | 0.0766 | **0.0000** | 0.0766 | 0.0000 |
| 43 | `callout3` | mirror-H | 1.2668 | 1.2707 | **0.0013** | 0.0583 |
| 46 | `accentCallout3` | mirror-H | 1.2044 | 1.2046 | **0.0008** | 0.0140 |
| 49 | `borderCallout3` | mirror-H | 0.0334 | 0.0334 | **0.0003** | 0.0088 |
| 52 | `accentBorderCallout3` | mirror-H | 0.0417 | 0.0417 | **0.0003** | 0.0047 |

**And four of the five are not mirrored templates.** This is the refutation the census exists to
make, and it had to be made in the two definitions rather than in the fitted numbers. The whole of
`callout3`'s tail is adjustment values:

```
# PresetShapeGeometry.txt, callout3 / borderCallout3 / accentCallout3 / accentBorderCallout3
a adj1 val 18750   a adj2 val -8333   a adj3 val 18750   a adj4 val -16667
a adj5 val 100000  a adj6 val -16667  a adj7 val 112963  a adj8 val -8333
g y1 */ h adj1 100000   g x1 */ w adj2 100000   …
p 0 0 none -
m x1 y1   l x2 y2   l x3 y3   l x4 y4
```

and Escher's is the same four free points — `mso_sptCallout3Verta` is
`{0,0} {21600,0} {21600,21600} {0,21600} {6 MSO_I,7 MSO_I} {4,5} {2,3} {0,1}` over
`mso_sptCalloutCalc`, which is eight bare `adjustNValue` reads. The path templates are
**identical**; what differs is `mso_sptCalloutDefault4` = `{23400, 24500, 25200, 21600, 25200,
4000, 23400, 4000}`, which puts the tail at x = +1.083…+1.167 of the box, against DrawingML's
`adj2/4/6/8` = −8333/−16667/−16667/−8333, which put it at −0.083…−0.167. The two defaults happen
to be reflections of one another, so the *shape* fits a mirror while the *table* does not.

**A second, sharper hazard falls out of the same pair, and it is general to the callout family:
the two vocabularies transpose the coordinates.** Escher's adjustment pairs are stated (x, y) —
`{0 MSO_I, 1 MSO_I}` is (adj1, adj2) — and DrawingML's are (y, x): `adj1` is `y1`, `adj2` is `x1`.
Any future conversion that feeds Escher's adjustments to a DrawingML callout in order will put the
tail's abscissa into its ordinate.

So the census's answer is: **the trapezoid is the only mirrored preset table among the 147 that
have one — 1 of 147, against a base rate of 147 that could have been.** `mso_sptTrapezoidVert` is
`{0,0} {21600,0} {0 MSO_I,21600} {1 MSO_I,21600}` (full width at the shape's **top**) and
DrawingML's `trapezoid` is `m l b / l x2 t / l x3 t / l r b` (full width at the **bottom**), and
26.2.4.2's own expansion of the first, in a 4 cm square at (1 cm, 1 cm), is

```
M 1000,1000 L 5000,1000 4000,5000 2000,5000 Z          # wide edge at y = 1000, the top
```

against this tree's `M 0,4000 L 1000,0 L 3000,0 L 4000,4000 Z` — wide edge at the bottom. Both
inset a quarter of the box, so the *defaults* agree exactly and only the orientation does not.

### 1.1 The one type with no Escher geometry

`mso_sptFlowChartOfflineStorage` (129) has **no `case` in `GetCustomShapeContent`** — the switch
jumps from `mso_sptFlowChartMerge` to `mso_sptFlowChartOnlineStorage` — and 26.2.4.2 draws nothing
at all for it, which the census leg confirms: 147 of the 148 shapes produced a path and that one
produced none. This tree maps 129 to `flowChartOfflineStorage` and draws the DrawingML preset.
**Nil reach**: no `.ppt`, `.doc` or `.xls` in the passing set states type 129
(`escher-adjust.tsv`'s type column, and a full type histogram of the two witness decks). Recorded
here so it is not rediscovered; not seated.

### 1.2 What the 50 "differs" are, and what this round does *not* claim about them

Ranked on the identity residual they run from 0.0101 (`actionButtonBlank`) to 0.1470
(`callout1` at MSO 178). Spot-checking the two ends, the residual is dominated by the two
vocabularies' **adjustment defaults** rather than by their templates — `leftRightArrow` at 0.0945
is Escher's `{5400, 5400}` against DrawingML's `{50000, 50000}` on guides that are not the same
fraction, and the `actionButton*` family differs only in the glyph drawn inside the button. §2
measures that directly for the 105 types that have an adjustment at all and settles it for 23 of
them; for the rest **this round does not claim to know which half of the definition is
responsible**, and the 50 is reported as a count of "does not draw the same shape at its own
defaults", which is what was measured.

### 1.3 C8 — the Escher defaults, read twice

`escher-defaults.tsv`. The source table (`pDefData` per `mso_CustomShape`, parsed out of
`EnhancedCustomShapeGeometry.cxx`) against the binary's own `draw:modifiers`, read back from
`--convert-to fodg` of the same 148 shapes: **144 of 148 agree outright.** The four that do not are
the source *parser*'s limits and every one agrees when read by hand — `octagon`'s default is a
computed constant (`(21600 − (√2−1)·21600)/2` = 6326), `star16`'s reaches `msoSeal16` through a
fall-through `case`, and the two braces carry a comment on the same line as the struct name. The
binary's column is the one the census uses.

---

## 2. THE CONVERSION CENSUS — the rescale is right for 18 types, needs `w/ss` for 3, and is wrong for 82

Same instrument, one axis added: the reference is asked for each type at **three in-range values**
of `adjustValue` — 2000, 5000 and 9000, inside the 0-10800 a one-handle Escher preset's own
`SvxMSDffHandle` declares — and at **two aspect ratios**, square and 2:1.
`vary2.py` writes the documents, `compare4.py` scores them, `adjust-conversion.tsv` is the result.

*The in-range part is not a detail.* A first pass at 3000/8000/**15000**, square box only, reported
`roundRect` — a preset this tree has converted correctly for twenty rounds — as `rescale-wrong`,
because 15000 is outside Escher's own 0-10800 handle range and DrawingML pins `a` at 50000 while
the reference does not. That pass certified **11 of 105**; this one certifies **22** in a square
box (18 of them in a 2:1 box as well), and every one of the eleven that moved was correct at 3000
and 8000 and wrong only at 15000. `compare3.py` is the discarded pass and
`adjust-conversion-outofrange.tsv` its eleven moved rows. **A conversion tested outside the range either side declares is testing the clamp.**

Of the 148 name-mapped types, **105 have an Escher adjustment at all** and 43 have none.

| verdict on `value × 100000/21600` | count | of 105 |
|---|--:|--:|
| **correct at both aspect ratios** | **18** | 17.1 % |
| correct in a square box only | 4 | 3.8 % |
| a *mirror* fits in a square box only | 1 | 1.0 % |
| **wrong** | **82** | 78.1 % |

The 18: `roundRect` (2), `triangle` (5), `octagon` (10), `plus` (11), `cube` (16), `plaque` (21),
`can` (22), the six bent and curved connectors `bentConnector3/4/5` and `curvedConnector3/4/5`
(34-36, 38-40), `bevel` (84), `leftBracket`/`rightBracket` (85, 86), `moon` (184), `bracketPair`
(185). Worst residual across all six points: 0.0079 of the box.

**Twelve of the 18 are in the shipped table and six are not.** The six connectors pass this test
as *presets* and are still declined, because the reference does not draw a `.ppt` connector from
its preset — §3.1. `PptShapeGeometry.AdjustmentInViewBox` therefore names 2, 5, 10, 11, 16, 21,
22, 84, 85, 86, 184 and 185, and `AdjustmentAcrossWidth` names 7, 8 and 9.

### 2.1 The five that are square-only are measuring across a different edge, and three of them close

Escher measures across the shape's **width**; the DrawingML preset of the same name measures across
`ss = min(w, h)`. So the conversion needs the extra factor `w/ss`, and in a square box — where
`w = ss` — the plain rescale is indistinguishable from it. Measured with that factor, at three
values and three aspect ratios (1:1, 2:1 **and 1:2**, so the `min` is exercised in both directions):

| MSO | preset | identity, 2:1 | identity, 1:2 | closes? |
|--:|---|---|---|---|
| 7 | `parallelogram` | 0.0011, 0.0008, 0.0007 | 0.0011, 0.0009, 0.0007 | **yes** |
| 8 | `trapezoid` (under mirror-V) | 0.0012, 0.0010, 0.0006 | 0.0012, 0.0010, 0.0006 | **yes** |
| 9 | `hexagon` | 0.0012, 0.0006, 0.0002 | 0.0012, 0.0006, 0.0002 | **yes** |
| 23 | `donut` | 0.0220, 0.0409, 0.0277 | — | no |
| 57 | `noSmoking` | 0.0230, 0.0414, 0.0122 | — | no |

`donut` and `noSmoking` are **not** closed by `w/ss` and are left discarding the value; whatever
their second rule is, this round did not find it and says so rather than guessing.

### 2.2 A blanket rescale would be a regression, and here is the number

The obvious "fix" for O73 is to drop the `roundRect or triangle` test and rescale everything.
`compare2.py` measures exactly that: our preset expanded at the Escher default rescaled, against
the reference at that default, over all 147. It takes the "identical" count from **92 to 68** —
closing **1** (`noSmoking`) and breaking **25**. The conversion is per preset, and the note at
`PptSlideLayout.cs`:2112 that round 124 quoted was right to be cautious.

---

## 3. SEAT O73 — the reach is 158, and 80 of it was never real

`escher-adjust.tsv` counted shapes that *state* property 327 and called them all cost. Two of the
three biggest contributors state it on a type that **has no adjustment in the reference either**:

```cpp
const mso_CustomShape msoStraightConnector1 = {            // and msoBentConnector2, msoCurvedConnector2
    std::span<const SvxMSDffVertPair>(mso_sptStraightConnector1Vert), …
    std::span<const SvxMSDffCalculationData>(),            // empty
    nullptr,                                               // no pDefData
```

`mso_sptStraightConnector1Vert` is `{0,0} {21600,21600}` and there is nothing for an adjustment to
move. So 26.2.4.2 draws those shapes identically whatever 327 says, and so do we.

| MSO | preset | shapes | what the reference does with 327 | this round |
|--:|---|--:|---|---|
| 37 | `curvedConnector2` | 50 | no adjustment in the table at all | **nil reach** |
| 32 | `straightConnector1` | 28 | no adjustment in the table at all | **nil reach** |
| 33 | `bentConnector2` | 2 | no adjustment in the table at all | **nil reach** |
| 38 | `curvedConnector3` | 45 | has one — but see §3.1 | declined |
| 34 | `bentConnector3` | 9 | has one — but see §3.1 | declined |
| 39 | `curvedConnector4` | 2 | has one — but see §3.1 | declined |
| 35 | `bentConnector4` | 1 | has one — but see §3.1 | declined |
| 63 | `wedgeEllipseCallout` | 10 | measured `rescale-wrong`, worst 0.076 | not converted |
| 62 | `wedgeRoundRectCallout` | 2 | measured `rescale-wrong`, worst 0.066 | not converted |
| 66 | `leftArrow` | 1 | measured `rescale-wrong`, worst 0.115 | not converted |
| 88 | `rightBrace` | 1 | measured `rescale-wrong`, worst 0.031 | not converted |
| 136, 144 | WordArt | 2 | `PresetOf` maps neither | out of vocabulary |
| **8** | **`trapezoid`** | **5** | `adj/21600` across the **width** | **fixed** |

**80 of the 158 are nil reach, 57 are declined with a measurement, 14 are measured
not-convertible, 2 are outside the preset vocabulary, and 5 are fixed.**

### 3.1 The connectors pass the unit test and are still declined, because a `.ppt` connector is not drawn from its preset

`bentConnector3/4/5` and `curvedConnector3/4/5` are in §2's list of 18: fed the rescaled value
they reproduce 26.2.4.2's own `draw:modifiers` render at every one of six points. They are still
not honoured, because the type that reaches the layouter from a `.ppt` never goes through the
preset at all:

```cpp
// filter/source/msfilter/msdffimp.cxx:4391
bool bIsConnector = ( ( aObjData.eShapeType >= mso_sptStraightConnector1 )
                   && ( aObjData.eShapeType <= mso_sptCurvedConnector5 ) );
…                                                        // :4792-4888
basegfx::B2DPolyPolygon aPoly( static_cast<SdrObjCustomShape*>(xRet.get())->GetLineGeometry( true ) );
xRet = new SdrEdgeObj(*pSdrModel);                       // the custom shape is thrown away
…
xRet->NbcSetPoint(aPoint1, 0);                           // both ends reset to the bound rect
xRet->NbcSetPoint(aPoint2, 1);
switch( eConnectorStyle ) { case mso_cxstyleBent: aSet.Put( SdrEdgeKindItem( SdrEdgeKind::OrthoLines ) ); …
static_cast<SdrEdgeObj*>(xRet.get())->SetEdgeTrackPath( aPoly );
```

**Confirmed twice**, per C8. Source above; and 26.2.4.2's own output — the three corpus decks
holding all 57 of those shapes were rendered with the arm on and with it off and scored against
the reference by pairing stroked polylines on their **end points** (which are the bound rectangle's
corners and do not move with the adjustment) and measuring the **interior** vertices, which are
exactly what the adjustment moves. `connscore.py`, on
`ws_prod-…-Approval-of-Flight-Conditions-RM-NAA-16032007.ppt`, the only one of the three whose
connectors pair at all:

| | paired polylines | interior vertex within 1 pt | median residual | mean residual |
|---|--:|--:|--:|--:|
| arm off | 11 | 6 | 0.091 pt | **6.63 pt** |
| arm on | 11 | 6 | 0.091 pt | **8.73 pt** |

Whole-page ink at 72 dpi moved the same way on all three decks — 3.3808 → 3.4003, 3.3985 → 3.4310,
3.4268 → 3.4537 — so the arm is a small regression by two independent measures and it is declined.
Named examples from page 9 of that deck: the connector at (459.25, 314.375) bends at y = 278.500
with the arm off and 26.2.4.2 bends it at **278.504**; with the arm on it bends at 254.377. The
one at (632.25, 243) goes the other way — 222.938 off, 221.875 on, reference **221.811** — which is
why the count within a point does not move and the mean gets worse.

**What this leaves open.** It is not established that a `.ppt` connector *never* keeps its stored
bend; `SetEdgeTrackPath` supplies the preset's own polygon and the two mismatches above point in
opposite directions. Modelling `SdrEdgeObj`'s routing is the seat, not converting the adjustment,
and it is filed as **O76**.

---

## 4. SEAT O74 — fixed; the flag was always read correctly and the base was upside down

`PptShapeGeometry.MirrorsVertically(8)` and `PptSlideLayout.Mirrored` turn the expanded geometry
over **in the shape's own coordinates, before the placement matrix** — which is where `fFlipV`
lives. That ordering is the whole fix: the flag is applied to a base that is now the same way up as
Escher's.

Witness `ws_prod-g-doc-Events-industrymeeting18112004-European-Safety-Strategy-Initiative.ppt`,
page 8, drawn coordinates, all three legs at the same commit of the corpus:

| | wide edge | narrow edge | inset each side |
|---|---|--:|--:|
| **before** | y = 403.125 (page **top**), 284.625 → 702.500 | **236.688** at y = 40.750 | 90.594 |
| **after** | y = 40.750 (page **bottom**), 284.625 → 702.500 | **76.378** at y = 403.125 | 170.748 |
| **26.2.4.2** | y = 40.791 (page **bottom**), 284.627 → 702.539 | **76.365** at y = 403.172 | 170.759 |

```
before  284.625 403.125 m 375.2187 40.7501 l 611.9062 40.7501 l 702.4999 403.125 l h S
after   284.625 40.7501 m 455.3733 403.125 l 531.7516 403.125 l 702.4999 40.7501 l h S
26.2    284.627 40.791  m 702.539  40.791  l 531.751 403.172  l 455.386 403.172  l h S
```

All **four** drawn trapezoids of that deck, on pages 7, 8 and two on page 9, now agree with
26.2.4.2 **corner for corner to 0.047 pt or better**, and the pair on pages 7 and 8 are drawn the
*opposite* way up from each other on both sides — which is the proof that `fFlipV` was never the
problem:

| page | before, wide edge | after, wide edge | 26.2.4.2, wide edge | worst corner, after |
|--:|---|---|---|--:|
| 7 | y 40.750 (bottom) | y 403.125 (top) | y 403.143 (top) | 0.035 pt |
| 8 | y 403.125 (top) | y 40.750 (bottom) | y 40.791 (bottom) | 0.047 pt |
| 9 a | y 40.750 (bottom) | y 403.125 (top) | y 403.143 (top) | 0.035 pt |
| 9 b | y 402.000 (bottom) | y 39.625 (top) | y 39.628 (top) | 0.038 pt |

**The flags settle it outright.** Read out of the two documents' OLE2 streams, the five type-8
shapes' `msofbtSp` flag words are `0x0a00`, `0x0a80`, `0x0a00`, `0x0a80` and `0x0a82` — so **two of
the European deck's four state `fFlipV` and two do not**, and after the fix the two that do are
drawn wide-edge-down (pages 8 and 9 b) and the two that do not wide-edge-up (pages 7 and 9 a),
each agreeing with 26.2.4.2. A defect in how the flag is read could not produce that; a mirrored
base applied under both settings does. `trapezoid-flags.tsv`. (Round 124's "3 of the 5" is right —
the third is the Nigerian deck's, §4.1.)

**Scope.** Both fixes are `.ppt`-only, and not by choice of population: `PptShapeGeometry` is
reached from `PptSlideLayout` and nowhere else (`git grep PptShapeGeometry -- src`), because the
`.ppt` reader is the only one that maps a numbered Escher type onto a DrawingML preset name. The
`.doc` and `.xls` Escher readers resolve a preset only where the file names one in DrawingML terms
already, so neither the mirror nor the conversion can reach them.

### 4.1 A refutation of O74's own reach figure

O74 is recorded as **5 Escher trapezoids in 2 documents**. Counting the `msofbtSp` records is
right — `outlook_of_nigerian_pension_sector.ppt` holds exactly one type-8 shape of its 374 — but
**neither 26.2.4.2 nor this tree draws it**: no four-point polygon with two distinct ordinates and
four distinct abscissae appears anywhere in either side's 12 pages, as a stroke, a fill or a clip.
The **drawn** reach of O74 is therefore **4 shapes on 3 pages of 1 document**, and the 5-in-2 is a
declaration count.

---

## 5. Regression: the twelve decks that state an adjustment, before and after

Every `.ppt` in `escher-adjust.tsv` rendered with the same binary before and after, mean absolute
grey difference at 72 dpi, page for page:

| deck | pages | mean | worst |
|---|--:|--:|--:|
| `ws_prod-…-European-Safety-Strategy-Initiative` | 10 | **3.9564** | **13.3013** |
| every other one of the twelve | 371 | 0.0000 | 0.0001 |

The one non-zero row is the four trapezoids, and it is an improvement against the reference on
exactly the three pages that hold them and on no other page of the document:

| page | before vs 26.2.4.2 | after vs 26.2.4.2 |
|--:|--:|--:|
| 7 | 15.4483 | **2.2241** |
| 8 | 22.3240 | **14.3607** |
| 9 | 22.3385 | **14.6241** |
| 1-6, 10 | 3.5341 … 7.5930 | identical to four decimals |
| **document mean** | **8.9356** | **6.0454** |

The residual on pages 8 and 9 is a **fill** difference and not a geometry one: 26.2.4.2 paints each
of these trapezoids as ~62 full-width bands of a gradient through the clip path — 63 fill operators
on page 7, 63 on page 8 and 124 on page 9, which holds two — and this tree fills them flat. The
clip path itself now agrees on all four. Why page 7's residual falls all the way to the document's
own baseline (2.22 against 1.85-4.97 on the pages with no trapezoid) while pages 8 and 9's stop at
14.4 is **not measured here**; all three carry the same number of bands per shape.

The 0.0001 on `FAA_Form_337` is `Adjustment` returning a `double` where it returned a truncated
`int` — `8826 × 100000/21600` is 40861.11, not 40861 — on the `roundRect` and `triangle` shapes
that were already converted. No page of any deck changed its page count.

---

## 6. C11 control

The witness's reference was rendered **twice**, in separate profiles, on the same UTC day. Page 8's
content stream is **byte-identical** between the two renders, and the trapezoid's clip path

```
284.627 40.791 m 702.539 40.791 l 531.751 403.172 l 455.386 403.172 l 284.627 40.791 l h W* n
```

is the same in both and the same as the one round 124 recorded. The quantity every number in §4
rests on is reproducible.

The census leg is reproducible in the stronger sense that it does not depend on a render at all
being stable: `escher-defaults.tsv`'s binary column comes from `--convert-to fodg`, which is a
resolved view rather than a drawing, and it agrees with the C++ table on 144 of 148 (§1.3).

---

## 7. Refutations recorded against this round's own work

1. **"`callout3`'s two definitions are mirrors."** The fitted numbers say so — mirror-H at 0.0003
   against identity 0.0334 — and the two *templates* are identical. §1. Had the census stopped at
   the fit it would have seated four mirrors that do not exist.
2. **"The rescale is wrong for `roundRect`."** An artefact of testing at 15000, outside the
   0-10800 range Escher's own handle declares. §2.
3. **"The connectors' adjustment can be converted."** It can, and doing it makes the corpus worse,
   because the reference replaces every connector with an `SdrEdgeObj`. §3.1.
4. **"O73 costs 158 shapes."** 80 of them state the property on a type that has no adjustment in
   `EnhancedCustomShapeGeometry` at all. §3.
5. **"O74 costs 5 shapes in 2 documents."** One of the five is drawn by neither side. §4.1.
6. **The first pass of the geometry census compared `Outline` against a stroke-only render**, and
   scored 11 shapes as differing on a subpath the reference declines to stroke and so does the
   definition. §0.

---

## 8. What this round could not settle

- **The 50 "differs".** A count, not a diagnosis. §1.2.
- **`donut` (23) and `noSmoking` (57).** Their adjustment converts correctly in a square box and
  neither the plain rescale nor the `w/ss` factor reproduces the reference in a 2:1 one. No corpus
  document states an adjustment on either.
- **`SdrEdgeObj` routing** — the real seat behind the connectors, filed as **O76**.
- **The gradient a `.ppt` shape carries.** Pages 8 and 9 of the witness still differ from the
  reference by ~14 ink after the geometry agrees, because 26.2.4.2 paints ~62 gradient bands
  through the clip and this tree fills flat. Page 7 carries the same 62 bands and falls to the
  document's baseline anyway, which is not explained. Not seated here; it is a fill question, not a
  geometry one.
- **`adjust2Value` (property 328) and beyond.** This tree reads only 327, and round 124's census
  counted only 327. Every two- and four-handle preset in §2 was measured with its *other*
  adjustments at the reference's defaults, so nothing here bounds what a file stating 328 costs.

---

## 9. Test totals

The ten non-fidelity projects, run one at a time on this commit, all `Passed!` with **0 failed and
0 skipped**: Presentations 1205, WordProcessing 1965, Spreadsheets 1387, Text 744, Core 591,
Vector 309, Markup 259, OpenDocument 169, Rendering 164, Containers 109 — **6902**. The run was
made under a load average above 20 with two other rounds' fidelity suites in flight, which is the
condition `CLAUDE.md` warns can drop tests or invent failures; the counts are stated so the next
round can compare them rather than only the colour.

`XRay.Office.Fidelity.Tests`: **552 total, 542 passed, 10 failed, 0 skipped.** All ten are the
word-processing and sheets families this tree leaves failing on purpose and every one of them is
named as such in `CLAUDE.md` or in its own remarks — four `PageDrawingComparisonTests`
(`paginated.docx/.doc/.rtf/.fodt`) and four `TabStopComparisonTests`
(`list-label-overrun.docx/.doc/.odt/.fodt`), which are the PDF-writer truncation family; one
`JustificationShrinkComparisonTests`; and one `SheetDrawingComparisonTests`, whose own remark
classifies it as 26.2.4.2 clamping a full-cell anchor offset. **No slides test is among them and
none of the ten touches a `.ppt` at all.**

And it **cannot see either fix** in any case: its only two `.ppt` fixtures are
`shape-geometry-ppt.ppt` and `slides-ppt.ppt`, and a type histogram of the first over its own
Escher streams is `{0: 12, 1: 31, 202: 8, 4095: 4}` — freeforms and groups, plain rectangles, text
boxes and `mso_sptNil`. No preset with an adjustment, and no trapezoid. Two new unit tests in
`PptPresetShapeTypeTests` carry the fix instead: one pinning the trapezoid's `w/ss` factor at two
aspect ratios, one pinning that it is the only mirrored type and that a connector's adjustment is
still discarded.

---

## 10. Files

| file | what |
|---|---|
| `census.fodg` | the 148-shape document 26.2.4.2 expands the Escher table into |
| `PresetDump/` | this tree's side: `SlidePresetGeometry.Of` printed as SVG path data |
| `gen.py`, `parse_ref.py` | build the census document; pull the reference's paths out of the SVG |
| `compare.py` | the classifier — Chamfer distance under four rigid transforms |
| `compare2.py` | the blanket-rescale pass (§2.2) |
| `vary2.py`, `compare4.py` | the conversion census at three values × two aspect ratios |
| `acrosswidth.py`, `acrosstall.py` | the `w/ss` factor at 2:1 and 1:2 (§2.1) |
| `escher_defaults.py` | the C++ `pDefData` table, for the C8 cross-check |
| `connscore.py` | connector polylines paired on end points, scored on interior vertices |
| `ink.py`, `refpage.py`, `pdfops.py` | mean grey difference; one page's content stream through the real page tree (C10) |
| `preset-census.tsv` | **the census** — 148 rows, verdict and all four residuals |
| `preset-census-rescaled.tsv` | the same at the rescaled Escher defaults |
| `adjust-conversion.tsv` | **the conversion census** — 105 rows, both aspect ratios |
| `escher-defaults.tsv` | source table against the binary's `draw:modifiers`, 148 rows |
| `adjust-conversion-outofrange.tsv` | the discarded out-of-range pass's eleven moved rows |
| `trapezoid-corners.tsv` | the four drawn trapezoids, before / after / 26.2.4.2 |
| `trapezoid-flags.tsv` | the five type-8 shapes' `fFlipV`, against which way up each is drawn |
| `witness-ink.tsv` | the witness deck page by page, before and after, against the reference |
