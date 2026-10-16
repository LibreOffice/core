# A preset subpath's own fill, stroke and shade, on the slide side

Round `agent/draw-shapes`, base `6bf527227`. Environment: this container, `/usr/bin/soffice` =
24.2.7.2, `/opt/libreoffice26.2/program/soffice` = 26.2.4.2 with the Latin Noto and the
metric-compatible duplicates moved aside. Corpus `/home/user/sample-files`.

## The leftover this closes

`probes/renderer-parity-sweep-01/L8-drawing/findings.md` RC-4 landed the data and the API — a
`PresetPath` carrying `fill` and `stroke`, a `Geometry.Subpaths`, and `FillOutline`/`StrokeOutline`
that fall back to the whole outline — and said so explicitly:

> **It is inert until a caller uses the new properties**, and that is deliberate: the two call
> sites that fill and stroke a shape outline are `XRay.Office.Presentations`', which this lane does
> not own.

`XRay.Office.WordProcessing` grew its consumer in `81e72b7d8`. The slide side had not, so
`SlidePresetGeometry.Outline` still answered with one path that was filled *and* stroked.

## What the reference does, and the two rules read out of it

A one-slide probe deck of nine presets, each a solid `4472C4` with a `C00000` pen
(`make-deck.py`), rendered by 26.2.4.2 and read with `pdf-ops.py dump`:

```
stroke  bentConnector3      #C00000          <- stroked, never filled
stroke  curvedConnector3    #C00000
fill    foldedCorner body   #4472C4
fill    foldedCorner fold   #365B9C          <- the shape's fill, darkened
stroke  foldedCorner        #C00000
fill    cube base           #4472C4
fill    cube                #365B9C
fill    cube                #698ECF          <- and lightened
stroke  cube (three records, one per polyline of the stroke-only subpath)
fill    can                 #4472C4
fill    can                 #8EAADB
stroke  can                 #C00000
fill/stroke rect, ellipse, diamond, homePlate — one of each
```

**Rule one: a subpath states whether it is filled and whether it is stroked.** 69 of the 187
presets carry a subpath that is not the default; 96 of the table's 320 subpaths say `fill="none"`
(every connector) and 84 say `stroke="false"` (every pseudo-3D shading face). LibreOffice routes a
`NONE` subpath into the stroke-only list in `EnhancedCustomShape2d::CreateSubPath`
(`svx/source/customshapes/EnhancedCustomShape2d.cxx`).

**Rule two: a shaded subpath is drawn in the shape's own fill taken towards white or black.** The
four magnitudes are in the same function (`EnhancedCustomShape2d.cxx`:2112-2121) — `darken` −0.4,
`darkenLess` −0.2, `lighten` +0.4, `lightenLess` +0.2 — and the blend is
`GetColorData` (:1084-1105): `c(1−b) + 255b` for a positive brightness and `c(1+b)` for a negative
one, both truncated. Against the probe's `4472C4` = (68,114,196) that predicts exactly what the
reference draws: `darkenLess` (54,91,156) = `365B9C`, `lightenLess` (105,142,207) = `698ECF`,
`lighten` (142,170,219) = `8EAADB`. 27 presets carry a shaded subpath, and in every one of the 27
the plain subpaths precede the shaded ones, so painting the plain fill and then the shaded parts in
table order is the reference's own paint order.

RC-4 recorded the magnitudes as **not established**. They are established now.

## Our probe deck after the change

`pdf-ops.py dump` on our own render, against the block above: **every fill record matches the
reference's colour and bounding box, and both connectors are stroked with no fill.** The only
residual difference is that the reference emits three `stroke` records for the cube where we emit
one whose box is their union — the same ink, split differently because LibreOffice makes an
`SdrPathObj` per polyline and we keep one path with three subpaths.

That is RC-4's own refutation test, which it wrote as: *"`bentConnector3` must become a bent line
of the stated width with no fill; the other seven must be byte-identical to today."*

## Reach, measured by rendering twice

117 slides-track documents can be reached — every `.ppt` (an Escher shape type becomes a preset
name only at layout time, so it cannot be censused from the bytes), plus every OOXML deck naming
one of the 69 presets or stating an `a:path` of its own with `fill="none"` or `stroke="false"`
(`affected.py`). 25 documents that name none were rendered as controls.

Both sets were rendered with the old binary and the new one, `SOURCE_DATE_EPOCH` set so the two
runs are byte-comparable:

| | documents | renderings changed |
|---|---:|---:|
| can be reached | 117 | **45** |
| controls | 25 | **0** |

44 of the 45 shrank — a connector's spurious fill going away — and one grew by 3 160 bytes,
`vvsummit2022-…`, which is the deck carrying six shaded presets.

## Direction, against the 26.2.4.2 reference

Eight of the reachable documents, scored with `pdf-image-diff.py` against 26.2.4.2, summing the
per-page `|ink|%`:

| document | pages | before | after | delta | MAJOR before/after |
|---|---:|---:|---:|---:|---:|
| `004_2-Stage_Vertical_Funnel_Diagram…` | 1 | 0.76 | 0.76 | +0.00 | 1/1 |
| `086_Infographic_Org_Chart…6_Blocks` | 3 | 0.49 | 0.49 | +0.00 | 1/1 |
| `7-Zulkefli_Part147n66_IKMAS` | 18 | 1.73 | 1.68 | **−0.05** | 2/2 |
| `bitesize-writing-a-report` | 15 | 3.81 | 3.45 | **−0.36** | 2/2 |
| `ghgp-supply-chain-initiative_20100323_wri` | 52 | 5.00 | 5.00 | +0.00 | 5/5 |
| `passiv` | 21 | 3.52 | 3.28 | **−0.24** | 6/6 |
| `vvsummit2022-Research-Roadmap…` | 33 | 6.10 | 5.60 | **−0.50** | 1/**0** |
| `wopanets-innovationdays-06-16` | 15 | 0.98 | 0.98 | +0.00 | 0/0 |
| **total** | **158** | **22.39** | **21.24** | **−1.15** | **18/17** |

Better on four, unchanged on four, **worse on none**, and one page stops being *major*. The four
that do not move are a mixture of documents outside the changed 45 and documents whose change is
below what 512 px on the long edge can see.

## One regression found and fixed on the way, which is worth keeping

The first build of the change broke
`SlideShapeGeometryComparisonTests.EveryDashedLineCarriesLibreOfficesOwnDashArray`: an arrowhead's
shaft started at 72 pt where the reference starts at 80.419. `PptxSlideLayout.Add` shortens a line
to make room for its markers with `shape with { Outline = shaft }`, and the two new painted paths
still pointed at the *full-length* outline, so the stroke was drawn unshortened. The fix is to let
whichever of them *was* the whole outline follow it onto the shaft, and to leave one that is a
proper part of the outline alone.

**It also contaminated the first reach measurement** — 64 documents changed on the broken build
against 45 on the fixed one, because the arrowhead regression reached 36 documents of its own. The
figures above are all from the fixed build; the byte-for-byte re-render is what separated them.

## And a second finding from the same probe deck: the join default

`0 J 1 j` against our `0 J 0 j` was carried as recorded-and-unverified. It is verified, and it is
not in `XRay.Office.Rendering` where the note placed it — the PDF writer emits whatever
`Stroke.Join` says. It is the DrawingML reader's default.

Counting the operators in the two content streams of the probe deck, whose nine shapes each state
an `a:ln` with a width, a colour and **no join child at all**:

| | join setups | value | cap setups | value |
|---|---:|---|---:|---|
| 26.2.4.2 | 11 | **`1 j`** (round) | 11 | `0 J` (butt) |
| ours, before | 9 | `0 j` (mitre) | 9 | `0 J` |
| ours, after | 9 | **`1 j`** | 9 | `0 J` |

The mechanism: `LineProperties::pushToPropMap` sets `ShapeProperty::LineJoint` **only** when the
markup states one of the three children (`oox/source/drawingml/lineproperties.cxx`:491-492), so an
`a:ln` with none leaves the draw layer's pool default — and `XLineJointItem`'s is
`LineJoint_ROUND` (`include/svx/xlinjoit.hxx`:35, `svx/source/svdraw/svdattr.cxx`:182). The oox
helper itself falls through to `ROUND` for a token it does not recognise (`lineproperties.cxx`:220).

**The Escher side is the opposite and is left alone.** `SvxMSDffManager::ApplyLineAttributes`
defaults `DFF_Prop_lineJoinStyle` to `mso_lineJoinMiter` for every shape type but `mso_sptMin`
(`filter/source/msfilter/msdffimp.cxx`:1052-1061), which is what `PptSlideLayout.Join` already
does. ODF is not touched either: `OdpSlideLayout` builds its `Stroke` without a join and would
need its own probe.

Reach, by the same render-twice method: **41 of the 142 sample renderings changed, every one of
them a `.pptx`.** Direction, on the eight scored documents, is what a corner-geometry change
should be — invisible to a 512-px diff: `7-Zulkefli` −0.02 and one of its major pages cleared,
`vvsummit2022` +0.04, the other six unmoved, **+0.02 over 158 pages**. The evidence that it is
right is the operator, not the raster.

## What is left

- **`XRay.Office.WordProcessing` shades nothing.** `PageDrawing.Outlines` uses `FillOutline` and
  `StrokeOutline`, which is rules one and two's first half, but a `PageFrame` has one `Fill` and no
  place for a shaded part — so a `cube` or a `foldedCorner` in a DOCX still draws flat. That file
  is another agent's half this round; the change is the same shape as this one.
- **`OdfEnhancedGeometry` skips the `F` and `S` commands** (`OdfEnhancedGeometry.cs`:619-621), so a
  stated `draw:enhanced-path` still reports no subpaths and is painted whole. Only ODP's preset
  fallback benefits from this round. The ODF vocabulary also has `H`, `I`, `J` and `K` for the four
  shades, which map onto the same four magnitudes.
- **We fill with `f` where LibreOffice fills with `f*`** — visible in every record of the reference
  dump above, which confirms the second of the two recorded-and-unverified items. Not changed
  here: it agrees on every preset with a hole because the preset table winds the inner subpath the
  other way, so the change would move ink only on a `custGeom` whose author did not, and no corpus
  document is known to be one. Unlike the join, this one really is in `XRay.Office.Rendering` —
  `PdfContentSink.cs`:241 already writes `f*` when asked, and no caller ever asks.
