# Round 190 — the of-pie composition, and two findings beside it

Measured 2026-09-28 against **LibreOffice 26.2.4.2** (`/opt/libreoffice26.2/program/soffice`,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`), corpus `/home/user/sample-files`, everything
rendered under `SOURCE_DATE_EPOCH=0`.

The seat is task #20, `028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx` — the corpus's one
**drawn** `c:ofPieChart`, at 16.47 `diff%` on its only page. (`029` is the other one and is
exploded, which 26.2.4.2 draws as a plain pie; that was closed in round 102 and it does not move
here.)

## 0. The instrument: read the wedges out of the page, not off it

`pdf-ops.py dump <pdf> --only fill` gives every filled path's colour and bounding box, and for a
pie that is the whole geometry: each wedge's path runs centre → rim → centre, so the boxes fix the
centre, the radius and each wedge's angular span, and the colour is exact rather than sampled. A
blind reading of the composed pair (`page-vision`) named five differences and **two of them were
wrong** — it read the reference's legend as 12 entries where the operators show all 16, and its
bar split as 75/25 where the operators give 47.7/52.3. It was right about direction on the rest,
which is exactly what that skill says a reading is for.

## 1. Three things `PieChart` decides and the file does not

| | 26.2.4.2 | this tree, before |
|---|---|---|
| composite wedge's colour | `#7E0021` | `#4472C4` (the series' own accent1) |
| bar's top segment | Leaf 16, the **last** split point | Leaf 15, the first |
| main ring opens at | −13.73°, so the composite straddles three o'clock | +13.73°, composite wholly above it |

**The composite wedge's colour is not in the document.** That series carries sixteen values and
**seventeen** `c:dPt`; the seventeenth states `accent4 lumMod 50%`, which is `#806000`. The
reference draws `#7E0021`. `propIndex` gives the composite wedge the property index
`pSeries->getTotalPointCount()` (`PieChart.cxx`:1185-1191) — one past the last value, *always* —
and `DataSeries::getDataPointByIndex` answers an empty reference outside
`0 <= nIndex < getData().getLength()` (`DataSeries.cxx`:312-337), so both the import's
`DataPointConverter` and the view's `getPropertiesOfPoint` come back with nothing,
`hasPointOwnColor` is false, and `createOneRing` falls through to
`m_xColorScheme->getColorByIndex(nPropIdx)` (`:1325-1331`). That scheme is
`Office.Chart/DefaultColor/Series` — twelve values in
`officecfg/registry/schema/org/openoffice/Office/Chart.xcs`:35-36, indexed `nIndex % 12`
(`ConfigColorScheme.cxx`:126-133) — and `16 % 12 = 4` is `0x7E0021`. It is a divergence from what
the writer meant and it is reproduced anyway, because this tree is calibrated to the reference.

**The bar stacks upward in data order.** `createOneBar` opens at `fBarTop = -0.5` and adds each
share, under the comment `// make the bar go from -0.5 to 0.5` (`:1416-1430`); the value axis
points up, so the first split point is the foot. Reference `028`: `#A9D18E` (Leaf 15, value 23)
from y 348.84 to 419.64 and `#843C0B` (Leaf 16, value 21) from 419.64 to 484.24 — green below,
brown above. This tree drew brown below at the same two heights.

**The main ring opens at minus half the composite sweep.** `createOneRing`'s `sAngle` lambda is
`clockwiseWedges() ? 360 - degAng : degAng` with `degAng = compositeVal*360/(2*ringSum)`
(`:1229-1244`). Reference: composite from y 395.14 to 437.94 about a centre at 416.54 — ±21.4 pt,
symmetric, which is `r·sin(13.73°)` at r = 90.3. This tree opened at *plus* half, which put the
whole composite above the axis and left its two connecting lines meeting nothing.

## 2. What the of-pie's constants already got right

Read out of the same two pages, our unit radius against the reference's:

| | reference | ours |
|---|---:|---:|
| main ring radius | 90.30 | 124.0 |
| unit radius (ring / (2/3)) | 135.45 | 185.7 |
| unit centre x | 304.59 | 305.2 |
| bar width / unit radius | 0.750 | 0.747 |
| bar height / unit radius | 1.499 | 1.495 |

So `m_fLeftShift`, `m_fLeftScale`, `m_fBarLeft`, `m_fBarRight` and `m_fFullBarHeight` are all
right and the *centre* is right to 0.6 pt. **Only the unit radius is wrong, by 1.37×**, which is
the ratio round 104 recorded and left.

## 3. The labels went through the wrong placer, and it is still not the radius

`AddOfPie` placed each label flat on its bisector at half the radius (`AddWedgeLabel`) where a
plain pie goes through `PieLabels` — the wrapping, the inner best-fit and the outside fallback
that `createTextLabelShape` gives every pie, of-pie included (`:1346-1350` skips only the
composite). Worse, `PieConsumedRect` — which is what the pie's second pass shrinks against —
measured those labels **on the unit circle**, which nothing is drawn on, instead of on the main
ring at two thirds of it. Both are now the same placer at the same place.

It changes nothing on `028`, and the reason is worth writing down: that chart states
`<c:dLblPos val="inEnd"/>`, which is neither best-fit nor outside, so both placers take the fixed
branch and put the block at `0.5 r` on the bisector — byte-identical output. Traced at pass 1
(unit radius 84.25, `reduceToMinimumSize`'s 1/2.2 square): all sixteen labels land inside the
square, the consumed rectangle comes back exactly as tall as the square, and `AdjustInnerSize`
therefore grows the diagram to the whole available rectangle.

**What the reference's second pass must have consumed** follows from the same arithmetic: for its
final square to be 270.9 rather than 370.68, `consumed.Height` has to be **268.27** against our
168.49 — about 50 pt of label overflow above and below at pass 1. So the missing piece is that
26.2.4.2's `inEnd` labels leave the ring where ours do not: `INSIDE` is not "half the radius", it
is the rim less a flat `150` in the radius direction
(`PieChart.cxx`:439-452 and `:490-497`), and `getLabelScreenPositionAndAlignmentForUnitCircleValues`
places it against `mfUnitCircleOuterRadius * fRadiusScale`. That is the next thing to measure and
it is left open (task #26).

## 4. A page-relative vertical anchor, and why the headline number cannot see any of this

`028`'s chart frame is **71.6 pt higher on our page than on the reference's** — same size
(595.30 × 451.89 against 594.50 × 451.10, the difference being the border inset), same body text
to a twentieth of a point on every line, only the frame displaced. `variants.py` settles the rule
at the reference, one attribute at a time; the chart's own background is the white fill spanning
the page:

| variant | frame top, doc y |
|---|---:|
| as authored (`page`, offset 175.52 pt) | 247.86 |
| `relativeFrom="margin"` | 247.86 — identical |
| offset 0 | 72.36 |
| offset +72 pt | 319.86 |
| offset −36 pt | 36.36 |
| offset −200 pt | 0.36 — clamped to the page |
| `w:top="2880"` (144 pt) | 319.86 |
| `w:top="0"` | 175.86 — **which is ours** |
| `behindDoc="0"` | 247.86 — unchanged |
| `wrapNone` | 247.86 — unchanged |
| **`layoutInCell="0"`** | **214.94, i.e. ours** |

So on this document the base is the **top margin**, it moves one for one with `w:pgMar/@w:top`,
it is clamped to the page, and **`layoutInCell` is what switches it** — although the object is
not in a table, which is the only place `GraphicImport` is documented to act on that attribute
(`GraphicImport.cxx`:777-788, `:1315-1317`, both guarded by `IsInTable()`).

**It is not the general rule, and the control says so.** `page-anchor-fixture.py` builds a
minimal DOCX — one `wps:wsp` at `relativeFrom="page"`, a `word/settings.xml` present so the
OOXML compatibility defaults apply, `layoutInCell="1"` — and 26.2.4.2 draws it at the page's own
top edge at both `w:top="1440"` and `w:top="2880"`. That is what this tree does. So the
mechanism on `028` is a second condition the fixture does not reproduce, and it is recorded
rather than implemented. Reach if anyone takes it: **16 corpus DOCX, 41 page-relative vertical
offsets** (`census-pagev.py`).

**And that displacement is why `diff%` rises.** With the chart 71.6 pt out of place, no
chart-internal change can lower a page-fraction metric — it can only move which pixels differ:
`028` goes **16.48 → 16.72**. Scored against a reference rendered from the `incell0` variant,
which puts 26.2.4.2's chart where ours is and changes nothing else, the same two renderings give
**15.21 → 14.47**. That is the isolating measurement and it is the one to read.

## 5. Reach

`confine.py` over the 168 chart-bearing corpus documents, our half rendered at the round's base
and again after, one output directory per document: **1 moves, 167 are byte-identical.** The one
is `028`. `029` does not move because it is exploded and is drawn as a plain pie by both
renderers.

Full suite green — Core 610, Text 756, Vector 309, Containers 109, Markup 259, Rendering 164,
OpenDocument 210, WordProcessing 2189, Spreadsheets 1492, Presentations 1220 — and Fidelity
`Failed: 10, Passed: 542`, which is the standing baseline.

## 6. Left, with what is known

- **The unit radius, 1.37×.** §3 says what the reference's pass-1 consumed rectangle must have
  been and names the `INSIDE` offset as the likely reason its labels leave the ring.
- **`relativeFrom="page"` on `028`.** §4. Measured, reach censused, mechanism not found.
- **The chart's own background.** That chartSpace states
  `<a:pattFill prst="dkDnDiag">` with `lt1 lumMod 95%` on `lt1`; 26.2.4.2 draws it as a white
  fill plus 45° `#F2F2F2` strokes across the whole frame. `ChartPlot.Background` is a
  `Colour?`, so a pattern fill reads as nothing and we draw neither.

## 7. An `inEnd` pie label is at the rim, not at half the radius — and that is what shrinks the diagram

This is the arm §3 said to measure, and it is bigger than the of-pie: the corpus's pie charts
state `inEnd` on **76 points in 5 documents** and `outEnd` on 37 in 4, against 188 points in 19
documents that state nothing (`census-dlblpos.py`, which counts the value that survives the
point-over-series-over-group merge inside a `c:pieChart`, `c:pie3DChart`, `c:doughnutChart` or
`c:ofPieChart` and nowhere else).

`PolarLabelPositionHelper::getLabelScreenPositionAndAlignmentForUnitCircleValues` takes the
ring's **outer** radius for `INSIDE` and `OUTSIDE` alike and the middle of the ring for
everything else — `bCenter` is exactly `nLabelPlacement != OUTSIDE && != INSIDE`
(`PolarLabelPositionHelper.cxx`:68-76) — and `createTextLabelShape` then pulls an `INSIDE`
anchor **back** along the radius by the same flat 150 hundredths of a millimetre it pushes an
`OUTSIDE` one out by (`PieChart.cxx`:439-452, applied at `:490-497`). The eight-row alignment
table is `bOutside ? A : B` with B the opposite of A on every row (`:112-137`), so the block
hangs back over the slice instead of away from it. The wrapping width stays the default
`0.8 * fPieRadius`: the room-to-the-edge arm is guarded by `nLabelPlacement == OUTSIDE` and the
comment beside it is a TODO asking for a better guess for `INSIDE` (`:544-576`).

Measured on `029_Unit_Circle_Chart_Pie_Theme_8a922142.docx`, whose four labels 26.2.4.2 places
`inEnd` (`labelreach.py`). Each label's distance from the pie's centre as a fraction of its
radius — the ratio, because the two pies differ by 2 pt:

```
reference  0.804  0.839  0.917  0.953  0.890  0.923  0.841
before     0.414  0.451  0.563  0.588  0.527  0.549  0.467
after      0.806  0.841  0.918  0.953  0.891  0.924  0.842
```

**Seven of seven within 0.002**, with no free parameter — and the distance is measured to the
*block*, not to the anchor, so it says the mirrored alignment is right as well as the anchor.

**Reach 5 of the 168 chart documents, 163 byte-identical.** `005_Contextures_chart_sample`
1.66 → 1.57, `100_Lime_and_Lemon` 10.49 → 10.37, `029` 5.53 → 5.36, `bitesize-writing-a-report`
level, and `028` 16.72 → 16.78 on the naive metric and **14.47 → 14.42** against the reference
placed at our position.

### And the of-pie radius is the BAR's labels, not the ring's

`028`'s radius does not move with this, and measuring where the reference actually puts that
chart's labels says why: all sixteen of its ring labels sit within 93 pt of a centre whose radius
is 90.3, so **the ring's labels do not leave the ring and never could have consumed anything**.
What does is the **bar's** labels, and they are a unit bug in the reference:

```cpp
const double fTextMaximumFrameWidth = 0.8 * (m_fBarRight - m_fBarLeft);   // PieChart.cxx:780
const sal_Int32 nTextMaximumFrameWidth = ceil(fTextMaximumFrameWidth);
```

`m_fBarRight - m_fBarLeft` is `1.25 - 0.75` in **unit-circle logic** units, so the wrapping width
handed to the label is `ceil(0.4)` = **1 hundredth of a millimetre** — one character per line.
That is the "vertical column of single letters" a blind reading saw in the reference's bar, and
it is why the diagram shrinks: two columns 17 and 25 lines tall at a 10.47 pt pitch reach far
past a bar that is 135 pt tall, and at pass 1 they are the same height beside a bar of 84.

Measured on the reference's page: the bar's glyphs form two runs at x ≈ 437.8, pitch 10.47 —
**17 rows** centred on y 454.35 against the upper segment's centre of 451.94, and **25 rows**
centred on 381.06 against the lower's 384.24. Against the labels' own text, `Branch 3 Leaf 16`
+ `48%` and `Branch 3 Stem 6 Leaf 15` + `52%`, "one line per non-space character" predicts 17 and
21 and "one line per character" predicts 19 and 26 — so the first is exact on one label and four
short on the other and neither is settled. **That exact count is what task #20 needs**, and it
has to come from a measurement of EditEngine's own breaking at a one-unit paper rather than from
either guess. `AddOfPieBar` currently emits the label unwrapped and `PieConsumedRect` does not
measure it at all.
