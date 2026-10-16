# L8-drawing — findings

14 documents. They collapse into **nine root causes plus two non-defects**. Two of the nine are
in the files this lane owns and carry patches; the rest are under
[Cross-lane dependencies](#cross-lane-dependencies) with the markup, the measurement and the
`file:line` that has to change.

The most useful thing measured this round is a **negative**: the shared DrawingML library this
lane owns — the preset-geometry evaluator, the 187-entry preset table, the fill readers, the
style matrix, the gradient paints and both backends' gradient arithmetic — is **already correct
and already complete**. Every "shape drawn as a rectangle", "gradient not painted" and "picture
fill missing" in the DOCX half of this lane is a *wiring* fault in `XRay.Office.WordProcessing`,
which reimplements a subset of that library instead of calling it. Two probes below establish
that, so nobody has to re-derive it.

## Two corrections to the dispatch brief

**1. No document in this lane is tagged `lo-broken`.** The brief said two were. `pl-cases.json`
holds exactly six (`Ramp Up Campaign - French.pptx`,
`018_Project_Timeline_Template_Editable_Format`, `016_Project_Timeline_Template_Complete_Guide`,
`AATF-Fact-Sheet-2025.pptx`, `097_Business_Case_Template_Elegant_Layout`,
`NAS-Infrastructure-Roadmaps-v16.0.pptx`) and none is in this lane's 14. The closest thing is
#152, whose case note records that the *reference* clips its own title and `Product Name:` label
while we print both — a partial reference defect on a document that is otherwise ours. Nothing
was chased on this basis.

**2. "A logo drawn at the wrong scale and clipped at the page edge" (#176) is neither.** The
picture is drawn at exactly the reference's size — `612.5 × 792.65 pt` on both sides, to the
digit — on the wrong *page*. See RC-7. This matters because it is a *size* claim, and the
coordinator's standing instruction is to check size and position claims against the version
question; there is nothing here for a version to explain.

## The reference-version check, applied to every patch

The sweep's reference is **LibreOffice 24.2.7.2**; the tree is developed against **26.2.4.2**.
Before proposing anything:

- `git grep '24\.2\.7\|26\.2\.4'` over `XRay.Office.Rendering/Fills/`,
  `DrawingML/CustomShapeGeometry.cs`, `DrawingML/PresetShapeGeometry.cs` → **no calibration
  markers.** Neither patched file states a claim about either binary.
- Both patches restore something we omit outright — a fill that stops three quarters of the way
  across a slide, and two columns of a data table that are parsed and discarded. No reference
  version makes either right.
- Both are checked against the **C++ in this checkout**, which is the *newer* tree:
  `GeoTexSvxTiled::iterateTiles` still has no tile cap, and `EnhancedCustomShape2d` still routes
  a `fill="none"` sub-path into the stroke-only list. So they are right for both references.
- **One finding turned out to be a version divergence and is filed as one rather than patched** —
  RC-10, #100's background gradient, where the tree is right for 26.2.4.2 and the sweep's
  reference is the older binary. That check is the single most consequential thing in this
  write-up, because a naive reading of the page would have produced a patch that reverted a
  deliberate, measured fix.

---

## RC-1 — The DOCX drawing path draws every DrawingML shape as its bounding rectangle, and reads only `a:solidFill`

**Documents: #062, #148, #152, #167, #169** (5 of 14 — every `shape-fallback` tag in the lane).
**Not this lane's files.** Recorded here because the diagnosis is this lane's, and because the
picture cannot distinguish its three limbs, which have three different fixes.

### What the pages show, and why it is three faults and not one

`pairs-view/167.jpg` separates them by itself, which is why it is the document to work from:

| what the reference draws | what we draw | limb |
|---|---|---|
| 3 yellow milestone **diamonds** | 3 yellow **squares**, right size, right fill, right stroke | 1a geometry |
| 13 blue Gantt **chevrons** carrying white date text | **nothing** — only the date text, floating on white | 1b fill |
| white "Project Timeline" on a blue gradient panel | white on white, i.e. invisible | 1b fill |

A shape that keeps its fill and loses its outline is a *geometry* failure. A shape that loses
everything except its text is a *fill* failure. They are not the same bug and #167 carries both.
#062 is the same pair (ellipses → squares; gradient page panel → nothing, taking the white title
and the five white stage captions with it). #148 adds a third case — four ellipses whose fill is
a picture — and #169 a fourth — six `custGeom` shapes filled `a:grpFill`.

The `content-missing` tag on all five is a **consequence**, not a separate defect: the missing
fill is the dark ground the white text was legible against. That was the specific question the
brief asked to settle, and it settles arithmetically rather than by argument — #062's title run
is `<w:color w:val="FFFFFF"/>` and *is present in our PDF's text operators*. It is drawn, in
white, onto white paper. Same for #176's copyright line (`1 1 1 rg` at y = 45.35 and 34.9 on our
page 1) and for #167's title. **The text is never dropped; the ground under it is.**

### What the documents actually contain

Every `wps:wsp` in the five documents, classified by `(preset, fill elements stated, fillRef idx)`:

```
020_Project_Timeline_Template_Modern_Theme (#167)
  13  homePlate    fills=()            fillRef=1     <- the Gantt bars
   4  rect         fills=(noFill)      fillRef=-
   3  diamond      fills=(solidFill)   fillRef=1     <- the milestones
   1  rect         fills=(gradFill)    fillRef=1     <- the page background

050_Visual_Product_Roadmap (#148)
  10  rect         fills=(noFill)
   4  ellipse      fills=(blipFill)    fillRef=1     <- the circular icons
   4  ellipse      fills=()            fillRef=1     <- their blue discs
   2  custGeom     fills=(solidFill)   fillRef=1     <- the yellow arrow, the blue triangle
   1  line         fills=()            fillRef=0

043_Visual_Product_Roadmap (#152)
   7  rect            fills=(noFill)
   4  bentConnector3  fills=()          fillRef=0    <- the missing connectors
   4  ellipse         fills=(solidFill) fillRef=1    <- concentric circles drawn as squares

077_Storyboard_Template (#169)
   9  custGeom     fills=(solidFill)   fillRef=1     <- rings and ruled panels
   8  rect         fills=(noFill)
   6  custGeom     fills=(grpFill)     fillRef=1     <- the yellow arrows
   3  custGeom     fills=(noFill)

019_Project_Timeline_Template (#062)
  20  ellipse (solidFill) + 1 rect 10058400x7751135 EMU with a:gradFill + 5 line
```

The Gantt bar, verbatim from `word/document.xml`, is the clearest specimen — it states **no fill
element at all**, only a style reference, and an `a:ln` with a colour and **no `@w`**:

```xml
<wps:spPr>
  <a:xfrm><a:off x="0" y="0"/><a:ext cx="2321960" cy="256654"/></a:xfrm>
  <a:prstGeom prst="homePlate"><a:avLst/></a:prstGeom>
  <a:ln><a:solidFill><a:srgbClr val="002060"/></a:solidFill></a:ln>
</wps:spPr>
<wps:style>
  <a:lnRef idx="2"><a:schemeClr val="accent1"><a:shade val="50000"/></a:schemeClr></a:lnRef>
  <a:fillRef idx="1"><a:schemeClr val="accent1"/></a:fillRef>
  <a:effectRef idx="0"><a:schemeClr val="accent1"/></a:effectRef>
  <a:fontRef idx="minor"><a:schemeClr val="lt1"/></a:fontRef>
</wps:style>
```

And #062's page background is a **shape**, not a `w:background` — the part contains no
`<w:background>` element at all, which refutes the obvious first story before it costs a day:

```xml
<wps:spPr><a:xfrm><a:ext cx="10058400" cy="7751135"/></a:xfrm>
  <a:prstGeom prst="rect"/>
  <a:gradFill flip="none" rotWithShape="1"><a:gsLst>
      <a:gs pos="0"><a:schemeClr val="accent5"><a:lumMod val="40000"/><a:lumOff val="60000"/></a:schemeClr></a:gs>
      <a:gs pos="46000"><a:schemeClr val="accent5"><a:lumMod val="95000"/><a:lumOff val="5000"/></a:schemeClr></a:gs>
      <a:gs pos="100000"><a:schemeClr val="accent5"><a:lumMod val="60000"/></a:schemeClr></a:gs>
    </a:gsLst>
    <a:path path="circle"><a:fillToRect l="50000" t="130000" r="50000" b="-30000"/></a:path>
  </a:gradFill></wps:spPr>
```

(That `fillToRect` derives a centre of X=50%, Y=100% — *not* a corner, so RC-10's version
divergence does not touch it. The reference's own PDF confirms: page 1 of #062's reference draws
nested circles centred at (396, 1.7) on a 792×612 pt page with an outer radius of 493 against a
half-diagonal of 500. A radial, in both binaries.)

### Where it lives in the source

- **1a, geometry.** `DocxFrames.cs:860` is the *only* place a DOCX reads `a:prstGeom`, and it
  reads it to answer one question — is this preset `line` or `straightConnector1`
  (`DocxFrames.cs:855-870`)? `a:custGeom` is never looked at at all. `PageFrame` has no outline
  member, and `PageDrawing.DrawFrame` (`PageDrawing.cs:230-310`) fills `frame.Area` and strokes a
  four-point rectangle built from `frame.Area`. There is no shape geometry in the Word drawing
  path: a `homePlate`, a `roundRect` and an `ellipse` are all their bounding box.
- **1b, fill.** `DocxFrames.Appearance` (`DocxFrames.cs:798-818`) reads
  `Child(properties, "solidFill")` and nothing else, and `PageFrame.Fill` (`PageFrame.cs:333`) is
  a `Colour?` — the type cannot carry a gradient, a picture or a pattern even if one were read.
  The file's own remark (`DocxFrames.cs:783-795`) states both limitations as deliberate: *"Only
  `a:solidFill` is read… A gradient, a pattern or a picture fill is a real fill this cannot yet
  draw"*, and *"A shape stating no fill element at all is left unfilled rather than given the
  theme's default."* Five documents in this lane are the bill for that second sentence.
- **1c, outline width.** `Appearance` returns `Emu(line.Attribute("w")?.Value)`, so an `<a:ln>`
  with a colour and no `@w` yields `Length.Zero`, and `DrawFrame` returns early on
  `frame.Frame.BorderWidth <= Length.Zero` (`PageDrawing.cs:262`). All 13 of #167's Gantt bars are
  exactly that shape, so even the navy outline the file *does* state is not drawn. The width
  belongs to `a:lnRef idx="2"` in the theme's `a:lnStyleLst`, which
  `DrawingStyleMatrix.Line`/`.Overlay` already resolve for the slides path.

### The probe that settles which layer is at fault

An image cannot tell "the preset did not resolve" from "the preset resolved and the fill was
wrong". So the shared evaluator was measured directly, by putting the same presets through the
pipeline that *does* call it. `work/mkdeck.py` builds a minimal valid `.pptx`; the probe deck
carries `ellipse`, `diamond`, `homePlate`, `bentConnector3`, `rightArrow`, `triangle`, `chevron`
and `roundRect`, each 1.8 M EMU square with `a:solidFill` and a 3 pt outline, rendered with the
prebuilt `XRay.Office.Cli`.

**Seven of the eight come out as the correct outline** — a true circle, a true diamond, a true
home plate, a true chevron, correctly rounded corners, a true isoceles triangle, a true block
arrow. (The eighth, `bentConnector3`, comes out as a filled triangle rather than a bent line;
that is RC-4, below, and it *is* this lane's.)

So `CustomShapeGeometry`, `PresetShapeGeometry` and the 187-entry table are not the fault, and
nothing needs to be added to `XRay.Office.Ooxml/DrawingML` for RC-1. Everything the Word path needs
already exists and is exercised daily by the slides path:

| what DOCX needs | what already exists, in this lane, unused by DOCX |
|---|---|
| shape outline from `a:prstGeom`/`a:custGeom` | `CustomShapeGeometry.Preset` / `.Custom` |
| gradient fill | `DrawingFill.ReadGradient` → `GradientPaint` → `PdfContentSink.FillGradient` |
| picture fill | `DrawingFill.ReadBlip` → `BitmapPaint` → `Tiles` / `PdfImages` |
| theme fill from `a:fillRef` | `DrawingStyleMatrix.Fill` |
| theme line, width included, from `a:lnRef` | `DrawingStyleMatrix.Line` + `.Overlay` |

`git grep` confirms the asymmetry: `DrawingStyleMatrix` is referenced from
`XRay.Office.Presentations` and the chart readers and **never once** from
`XRay.Office.WordProcessing`; `DrawingFill.ReadGradient` likewise.

**Confidence: high** for the diagnosis and the three-way split — the classification is mechanical
over the parts and the probe is decisive. **Not established:** how much of each document's
residual error survives the fix. #148's title also rewraps (`Template` onto a second line), which
is L1's advance divergence and independent of this.

**The probe that would refute me:** a one-shape DOCX with `<a:prstGeom prst="ellipse"/>` and an
explicit `a:solidFill`. If it draws a square, 1a is confirmed and 1b is irrelevant to it. A second
with `prst="rect"` and only `<wps:style><a:fillRef idx="1">` isolates 1b: if it draws nothing, the
theme fill is the fault; if it draws a coloured rectangle, it is not and the missing chevrons are
something else.

---

## RC-2 — A tiled bitmap fill stops after 8192 tiles, leaving three quarters of the region unpainted  *(this lane; patch)*

**Document: #076.** Patch: `patches/tiled-fill-truncated.diff`.

### What the page shows

`pairs-view/076.jpg`: the reference lays a very faint mottled wash over the whole slide; ours is
pure white below the header ribbon. The case note reads it as "the slide's background wash is not
painted", and that is *nearly* right — it is painted, over the top 22% of the slide, and the eye
reads a fifth of a faint texture as none of it. This is the one place in the lane where the
picture's own reading needed correcting rather than confirming.

### What the document actually contains

`ppt/slides/slide2.xml` states no background. `slideMaster1.xml` does:

```xml
<p:bg><p:bgRef idx="1003"><a:schemeClr val="bg1"/></p:bgRef></p:bg>
```

`idx=1003` is the **third** entry of the theme's `a:bgFillStyleLst`, which in
`ppt/theme/theme1.xml` is not a gradient at all but a tiled, duotoned bitmap:

```xml
<a:blipFill><a:blip r:embed="rId1"><a:duotone>…</a:duotone></a:blip>
  <a:tile tx="0" ty="0" sx="65000" sy="65000" flip="none" algn="tl"/></a:blipFill>
```

All of which we resolve correctly: `DrawingStyleMatrix.Background` handles the 1000-offset and
the clamp, the duotone is applied, `a:tile`'s `sx`/`sy`/`algn`/`tx`/`ty` are all consumed
(`PptxSlideLayout.cs:1657-1668`), and **the tile image we emit is byte-identical to the
reference's** — both PDFs carry a 5×5 `/DeviceRGB` XObject whose 38-byte Flate stream is the same
bytes, mean sample 251.72. So the fill is right and only its extent is wrong.

### The measurement

Counting `q … cm /Im Do` groups on page 2 of both PDFs (`work/tile-measurement.txt`):

```
LO: page 2 tile draws=37550 tilesize=3.231x3.231pt x[-3.231,717.392] (226 cols) y[-2.863,536.797] (170 rows)
PL: page 2 tile draws= 8192 tilesize=3.250x3.250pt x[  0.000,718.250] (222 cols) y[419.750,536.750] ( 37 rows)
```

**8192 exactly** — 222 columns × 37 rows — stopping at y = 419.75 pt on a 540 pt slide. Sampling
the two rasters at 60 dpi agrees: 12% down the page ours carries the texture (248) and below 30%
it is 255 where the reference is 243.

### Where it lives in the source

`dotnet/src/XRay.Office.Rendering/Fills/Tiles.cs:26` — `public const int Maximum = 8192;` — and
`Tiles.Cover:81`, `if (drawn++ >= Maximum) yield break;`.

The remark above the constant argues the truncation is *"visible and therefore reportable"*. On a
faint texture it is neither: it reads as a background that was never painted. Only the PDF backend
is affected — `SkiaDrawingSink.Shader:505-520` takes `Cover(...).FirstOrDefault()` and lets a
repeat shader do the rest — so the two backends have silently disagreed on every large tiled fill
since the constant was written.

LibreOffice imposes no limit: `GeoTexSvxTiled::iterateTiles`
(`drawinglayer/source/texture/texture.cxx:1009-1019`) guards only against a zero-sized tile and
emits one transform per cell. Hence its 37 550, and hence the version-neutrality — that source is
the *newer* tree, so the behaviour is the same at 24.2.7.2 and 26.2.4.2.

### The change

Replace the constant with a ceiling **derived from the region**: one tile per square point of the
region being filled, because a tile smaller than a point cannot be told from its neighbour at the
resolution a PDF's own coordinates are written in. For this slide that is 388 800 against a need
of 37 550. This is deliberately *not* a bigger magic number: `AGENTS.md` forbids those, and 8192
was one.

No test asserts the old constant — `git grep` over `dotnet/tests` finds no reference to `Tiles`,
`Tiles.Maximum` or `8192` — and the raster backend is unaffected because it reads only the first
cell. If a reviewer prefers to match LibreOffice exactly, delete the ceiling: nothing else in
`Cover` needs it, since `Grid` already rejects a non-positive step.

**The probe that would refute me:** a deck whose background tile is 3.2 pt on a 720×540 pt slide,
rendered before and after. Before, the fill stops 22% down; after, the `Do` count is ~37 550 and
the wash reaches the bottom edge. A second probe at 0.001 pt confirms the ceiling still bounds the
work. The alternative explanation the image alone could not exclude — that we resolve the wrong
`bgFillStyleLst` entry, or lose the duotone — is excluded by the byte-identical tile XObject.

**Confidence: high.** Cause, seat and remedy are all measured. **Not established:** the corpus
reach. The mechanism needs only a small tile over a large region, and the third
`a:bgFillStyleLst` entry of every stock Office theme is exactly that, so it is probably wider
than one document; I did not sweep for it.

---

## RC-3 — `wp:anchor/@relativeHeight` is not read, so DOCX frames paint in document order  *(new; matches L2's #024)*

**Documents: #148 today; #062 latently.** Not this lane's file. **This carries a sequencing
constraint on RC-1 and is the reason it is written up separately rather than folded into it.**

`git grep -rn "relativeHeight" -- dotnet/src` returns **nothing**. `behindDoc` is read
(`DocxFrames.cs:183`) and decides only the text/graphic split, not the order among graphics.

Dumping every `wp:anchor` in document order with its `@relativeHeight`:

```
#148, 23 anchors                          #062, 44 anchors
 idx  relH        geom      fills          idx  relH        geom     fills
 ...                                        ...
  9-12 2516736..  ellipse   blipFill         2-7 2516766..  ellipse  solidFill
 16-19 2516623..  ellipse   (theme)            8  251748352 rect     (theme)
   20  251661312 custGeom  solidFill  <-       9  251658239 rect     gradFill   <- LOWEST of 44
   21  251660288 custGeom  solidFill  <-      ...
```

**#148's two big `custGeom` shapes — the yellow arrow and the blue triangle — carry the two
*lowest* `relativeHeight` values of the document's 23 anchors and sit at document positions 20
and 21 of 23.** Drawn in document order they land on top of the eight text boxes (relH
251680768–251693056) and the eight icon discs (251662336–251679744) that belong above them. That
is exactly what `pairs-view/148.jpg` shows: the blue shape covering the *Sketching & Designing*,
*Finance* and *Processing* captions and the icons. Some of that is RC-1a — a `triangle` drawn as
a rectangle covers more — but **the ordering is wrong independently**, and with the correct
triangle the icons inside it would still be buried.

**#062 is the sequencing hazard.** Its full-page gradient background is anchor index **9 of 44**
in document order and carries `relativeHeight="251658239"`, the **lowest value in the document**
(the next is 251659264). Today it draws as nothing, so the ordering costs nothing. The moment
RC-1b lands and the gradient is painted, document order will paint a 10058400 × 7751135 EMU
rectangle over anchors 0–8 — which include six of the timeline's ellipses — and the document will
go from "background missing" to "background over the artwork". **RC-3 must land with or before
RC-1b.**

Seat: `DocxFrames.cs` must read `@relativeHeight` onto `PageFrame`, and whatever places frames on
a page must sort by it within each of the behind-text and in-front-of-text bands. **L2/L3's.**
This is the same defect L2 reports on #024, where all eight shapes are in the right places in the
exact reverse of LibreOffice's order.

**Confidence: high** that the attribute is unread and that #148's overlap is partly ordering.
**Not established:** whether Word's tie-break within one `relativeHeight` band is document order
(it almost certainly is), and whether `behindDoc` partitions the sort or merely biases it.

---

## RC-4 — A preset sub-path's `fill` and `stroke` flags are parsed off the table line and thrown away  *(this lane; patch)*

**Documents: #152's four missing connectors, and every connector or pseudo-3D preset in the
corpus.** Patch: `patches/preset-subpath-fill-stroke.diff`. **Enabling** — see the note at the end.

This is the "read but never consumed" pattern the brief said to grep for on purpose, in its
purest form: the data file records the two columns, the file's own header documents them, and the
parser reads them off the line and drops them on the floor.

`PresetShapeGeometry.txt`'s header says:

```
#   p <w> <h> <fill> <stroke>      a subpath; w/h are its own coordinate space, 0 for the shape's
```

`PresetShapeGeometry.Load`, `case 'p'` (`PresetShapeGeometry.cs:110-118`), takes `fields[0]` and
`fields[1]` and stops. `PresetPath` (`CustomShapeGeometry.cs:764`) has no member for either.
`CustomShapeGeometry.Evaluate` (`:216-233`) emits every sub-path into one `GraphicsPath Outline`
that callers both fill and stroke. `CustomShapeGeometry.Custom` (`:117-120`) ignores
`a:path/@fill` and `@stroke` in an authored `a:custGeom` for the same reason.

The table's 320 `p` lines, counted by the two dropped columns:

```
138  -            -        (nothing stated)
 95  none         -        <- not filled
 52  -            false    <- not stroked
 16  darkenLess   false
 12  darken       false
  3  lighten      false
  2  lightenLess  false
  1  none         false
  1  darkenLess   -
```

**96 sub-paths say `fill="none"` and 84 say `stroke="false"`.** `bentConnector3` — the preset
#152 uses four times — is one line of the table:

```
s bentConnector3
a adj1 val 50000
g x1 */ w adj1 100000
r l t r b
p 0 0 none -
m l t
l x1 t
l x1 b
l r b
```

The RC-1 probe shows the consequence directly, and it is the reason that probe is worth keeping:
seven of eight presets draw correctly and `bentConnector3` comes out as a **filled triangle** —
an open three-segment polyline filled as though it were a region — where LibreOffice draws a bent
line. The `darken`/`lighten` rows are the shading faces of `cube`, `can`, `bevel` and their
neighbours, which today take the shape's flat fill *and* a stroke the file does not ask for, so a
cube renders as a flat outlined hexagon instead of three tones.

LibreOffice honours both flags: `Path2D::getFillMode` feeds
`EnhancedCustomShape2d::CreateSubPath`, which routes a `NONE`-mode sub-path into the stroke-only
list (`svx/source/customshapes/EnhancedCustomShape2d.cxx`) — in this checkout, i.e. the newer
tree, so the behaviour is version-neutral.

### The change

`PresetPath` gains `Fill` and `Stroke`, both defaulted, so all three existing construction sites
keep compiling — including `OdfEnhancedGeometry.cs:469` in another lane. `Load` reads fields 2 and
3. `Custom` reads `a:path/@fill` and `@stroke`. `Geometry` gains `Subpaths` (defaulted null) plus
`FillOutline` and `StrokeOutline`, which fall back to `Outline` when no sub-paths are recorded, so
**every existing caller behaves exactly as before**, `OdpSlideLayout.cs:471`'s two-argument
construction included.

**It is inert until a caller uses the new properties**, and that is deliberate: the two call sites
that fill and stroke a shape outline are `XRay.Office.Presentations`', which this lane does not own.
They are named under Cross-lane dependencies. I would rather land the data and the API here —
where the table, the parser and the evaluator all live — than propose an edit outside the lane or
leave the two columns rotting for another round.

**The probe that would refute me:** the eight-preset deck in `work/`, re-rendered after both this
patch and the two-line consumer change. `bentConnector3` must become a bent line of the stated
width with no fill; the other seven must be byte-identical to today. If `bentConnector3` still
fills, the flag is not what decides it and the fault is in how the shape's fill is chosen rather
than in the sub-path.

**Confidence: high** that the flags are dropped and that this is why a connector fills.
**Not established:** what `darken`/`lighten` should multiply the fill by. LibreOffice's factors
are in `EnhancedCustomShape2d`; the patch deliberately carries the modes without implementing
them, so a caller that ignores them draws what it draws today.

---

## RC-5 — A picture's `a:scene3d` camera rotation and its `a:ln` border are not read

**Document: #100** (the poster). Not this lane's file.

The case note says "the poster image loses its rotation". The obvious mechanism is
`a:xfrm/@rot`, and **the file states none** — the `p:pic`'s `a:xfrm` in `slide3.xml` is
`<a:off x="5257800" y="1600200"/><a:ext cx="2638032" cy="3962400"/>` and nothing else. What it
does state is:

```xml
<p:spPr>
  <a:xfrm><a:off x="5257800" y="1600200"/><a:ext cx="2638032" cy="3962400"/></a:xfrm>
  <a:prstGeom prst="rect"/>
  <a:solidFill><a:srgbClr val="FFFFFF"><a:shade val="85000"/></a:srgbClr></a:solidFill>
  <a:ln w="190500" cap="sq"><a:solidFill><a:srgbClr val="FFFFFF"/></a:solidFill><a:miter lim="800000"/></a:ln>
  <a:effectLst><a:outerShdw blurRad="65000" dist="50800" dir="12900000" …/></a:effectLst>
  <a:scene3d><a:camera prst="orthographicFront"><a:rot lat="0" lon="0" rev="360000"/></a:camera>
    <a:lightRig rig="twoPt" dir="t"><a:rot lat="0" lon="0" rev="7200000"/></a:lightRig></a:scene3d>
  <a:sp3d contourW="12700">…</a:sp3d>
</p:spPr>
```

`rev="360000"` is **6°** about the view axis. LibreOffice turns exactly that into a shape
rotation, and says so: *"Look for 3D. Its z-rotation and extrusion color become shape properties.
We consider a z-rotation of an image even we currently do not extrude an image to 3D-scene"*
(`oox/source/drawingml/shape.cxx:1054-1064`), through
`Scene3DHelper::setExtrusionProperties` (`scene3dhelper.cxx:225-275`) and back out as
`nShapeRotateInclCamera` at `shape.cxx:1064`, applied at `:1214-1216` or `:2216-2223`.

`a:scene3d` is read **nowhere** in `dotnet/src` — `git grep -n "scene3d\|sp3d\|camera" --
'dotnet/src/**/*.cs'` returns only unrelated spreadsheet camera-tool comments. The 15 pt white
`a:ln` photo border is the second half of the same shape's appearance and is also absent, which is
what makes ours look *smaller* as well as upright.

This is exactly the trap the brief warned about, and worth stating plainly: the picture says
"rotation lost", the mechanism the picture implies (`@rot`) **is not in the file**, and grepping
the part for the attribute about to be blamed cost one minute.

**Confidence: high** that `scene3d` is the source of the tilt and that we read none of it.
**Not established:** the sign and exact magnitude LibreOffice applies — the helper negates twice
and adds the shape rotation for non-legacy cameras. A one-picture probe at `rev="360000"` against
the reference settles it in one render, and would also say whether the 6° should be clockwise.

---

## RC-6 — VML WordArt (`v:textpath`) is not read, so a Word watermark is absent

**Documents: #142, #190.** Not this lane's file.

Both are EASA templates and both put the watermark in a header as a VML WordArt shape, not as a
DrawingML shape:

```xml
<w:pict><v:shapetype id="_x0000_t136" o:spt="136" adj="10800"
     path="m@7,l@8,m@5,21600l@6,21600e"> … <v:textpath on="t" fitshape="t"/> … </v:shapetype>
  <v:shape id="PowerPlusWaterMarkObject34338737" type="#_x0000_t136"
     style="position:absolute;margin-left:0;margin-top:0;width:317.5pt;height:317.5pt;
            rotation:315;z-index:-251657216;mso-position-horizontal:center;
            mso-position-horizontal-relative:margin;mso-position-vertical:center;
            mso-position-vertical-relative:margin"
     o:allowincell="f" fillcolor="silver" stroked="f">
    <v:fill opacity=".5"/><w10:wrap anchorx="margin" anchory="margin"/></v:shape></w:pict>
```

(#142's `word/header1.xml` and `header2.xml` carry `string="EASA example document"` on the shape's
own `v:textpath`; `header3.xml`, quoted above, is the variant that inherits the shapetype's.
#190 is the same construct with `EASA Example Documents`.)

`git grep -rn "textpath" -- dotnet/src` returns **nothing**. `DocxVmlFrames.cs` reads VML shapes
but not the `_x0000_t136` WordArt type, so the shape contributes an empty unfilled box and the
words never reach the page. `rotation:315` and `opacity=.5` are both in the style string and both
unread.

**Confidence: high.** **Not established:** whether a faithful rendering needs the text warped onto
the shapetype's path or merely set once, centred, scaled to the box and rotated. For `_x0000_t136`
with `adj="10800"` the path is a straight baseline, so plain rotated text is almost certainly
indistinguishable — but that is an assertion, not a measurement.

---

## RC-7 — An inline picture taller than the text area is moved to the next page instead of overflowing

**Document: #176.** Not this lane's file. The case note diagnosed this one wrong and the
correction matters.

The note reads *"Every graphic on the cover is missing: the green header bar, the green footer
band with its copyright notice, the pale blue substation line-art, and both the IEEE and PES
logos."* All of those are **one JPEG** — `word/media/image1.jpeg`, 1.18 MB, 2550×3300 — placed as
a single `wp:inline` picture, `wp:extent cx="7778812" cy="10066699"` (8.51 × 11.01 in) on a Letter
page. There is exactly one media part in the package.

It is not missing. Reading the two PDFs' page contents:

```
ours       page 2 : /Im1 Do  with  612.5 0 0 792.65 0 -11.00 cm
reference  page 1 : /Im7 Do  with  612.5 0 0 792.65 0 -13.25 cm
```

**Same image, same size to the digit, same x, one page apart.** We decode it, we write it as an
XObject (`/Type/XObject/Subtype/Image/Width 2550/Height 3300/ColorSpace/DeviceRGB`), we place it
correctly — on the wrong page, because it is 792.65 pt tall in a 648 pt text area and our flow
moves it on rather than letting it overflow. Everything the note lists as "missing graphics"
follows, including the white copyright text, which our page 1 draws as `1 1 1 rg` at y = 45.35 and
34.9 onto white paper.

Seat: `XRay.Office.WordProcessing/Layout` — the flow's handling of a single inline object taller
than the remaining, and then the whole, text height. **L2's.**

**Confidence: high** — this is read out of both content streams, not inferred. **The version
check the coordinator asked for:** this is a *position* fault, and 24.2.7.2 is the only binary I
can run, so "the reference keeps it on page 1" is established for 24.2.7.2 only. It should be
re-checked against 26.2.4.2 before the fix lands. Two things reduce the risk: the page *count*
agrees (14 = 14 on both sides), and the geometry is identical, so the divergence is a single
break decision rather than a metric.

---

## RC-8 — Anchored frames land at the wrong end of the page

**Document: #051.** Not this lane's file, and the weakest evidence in this write-up.

`pairs-view/051.jpg`: the Health Cluster masthead logo and the photograph sit at the *bottom* of
our page 1, over the footer, where the reference has them at the top; the bulletin number and
date are stranded beside the caption band. Every one of the document's 16 anchors is
`positionV relativeFrom="paragraph"` with a small offset (−80 645 to +1 552 068 EMU) and
`positionH relativeFrom="column"` — every position is relative to the anchor *paragraph*, so a
frame attached to the wrong paragraph, or resolved after the flow rather than against it, lands
anywhere. That is consistent with what the page shows and I did not narrow it further. Same
family as RC-7. **L2's.**

**Confidence: low-medium** on cause; high that the file states nothing unusual — no
`relativeFrom="page"`, no `behindDoc` oddity, no `w:framePr`. **Position fault: re-check against
26.2.4.2 before fixing.**

---

## RC-9 — `.ppt` connector arrowheads are not read

**Document: #157.** Not this lane's file.

The case note: *"The connector between the two boxes loses its arrowhead — X-Ray.Office draws the
shaft as a plain rule."* The machinery to draw one exists and is used by PPTX —
`XRay.Office.Presentations/Layout/SlideLineEnds.cs`, driven from `PptxSlideLayout.cs:914-915`
and `:1920-1935` off `a:headEnd`/`a:tailEnd`, applied at `:637`. The legacy binary path never
fills it in: `git grep -n "lineStartArrow\|lineEndArrow"` over `dotnet/src` returns **nothing**,
and `PptShapeGeometry.cs` names Escher property 471 (`LineEndCap`) but not 464/465
(`lineStartArrowhead`/`lineEndArrowhead`) or their width/length companions 466-469.

So this is a reader gap in `XRay.Office.Presentations/MsBinary` feeding a renderer that is already
there. **L5's.** The document's other defect — second-level items set narrower, so each takes an
extra line — is L1's.

**Confidence: high** on the gap. **Not established** that reading 464/465 alone is sufficient: the
shape may also need `PptSlideLayout` to route a connector through `SlideLineEnds.Apply` the way
`PptxSlideLayout.cs:637` does.

---

## RC-10 — NOT A DEFECT: the corner-focus circle gradient is a reference-version divergence, and this lane closed the open question on it

**Document: #100** (the background). **Do not patch.**

### What the page shows

`pairs-view/100.jpg`: both sides put white at the bottom-right of a navy slide, but the
reference's pale wedge sweeps the whole lower-right half and lightens the left edge from
(23,41,119) at the top to (104,115,166) at the bottom, while ours is a tight glow in the corner
and dead navy elsewhere. The case note calls it "rendered much darker" — the symptom of a
*smaller ramp*, not of a wrong colour.

### What the document contains, and what the reference's own operators say

```xml
<p:bg><p:bgPr><a:gradFill><a:gsLst>
  <a:gs pos="0"><a:schemeClr val="bg1"/></a:gs>
  <a:gs pos="100000"><a:srgbClr val="172977"/></a:gs></a:gsLst>
  <a:path path="circle"><a:fillToRect l="100000" t="100000"/></a:path>
</a:gradFill></p:bgPr></p:bg>
```

`fillproperties.cxx` derives the centre per axis as `(MAX_PERCENT + X1 − X2) / 2`, so this is
X=100%, Y=100% — the bottom-right corner. LibreOffice writes gradients as banded polygons, so the
geometry can be read straight out of the reference PDF: page 3 carries **115 bands that are
strips, not rings** — four-point parallelograms sharing two fixed vertices at (990,270) and
(360,−360), the moving edge at 45°, marching from a line through the top-left corner to a line
through the bottom-right, total span 890.9 pt = `w·|dx| + h·|dy|` for a 45° ramp on 720×540.
That is `OutputDevice::DrawLinearGradient`'s signature output.

### The probe, and why it matters more than the finding

`work/probe-grad.pptx` is seven slides, identical two-stop `path="circle"` gradient, only
`a:fillToRect` varying, rendered through the **installed soffice 24.2.7.2 — the binary that made
this sweep's references — and through `XRay.Office.Cli`** (`work/gradient-probe.txt`; samples are
TL, TR, BL, BR then centre):

```
centre X=100 Y=100      ref   23, 41,119 156,164,197 122,132,177 254,254,254 | 140,149,188
                        ours  23, 41,119  23, 41,119  23, 41,119 251,251,252 |  23, 41,119
centre X=50  Y=100      ref   23, 41,119  23, 41,119  71, 86,147  71, 86,147 | 116,127,173
                        ours  23, 41,119  23, 41,119  71, 86,147  72, 87,148 | 116,127,173
centre X=50  Y=50       ref   25, 43,120  25, 43,120  25, 43,120  25, 43,120 | 255,255,255
                        ours  26, 44,121  27, 44,121  27, 44,121  27, 45,122 | 255,255,255
centre X=0   Y=0        ref  254,254,254 122,132,177 156,164,197  23, 41,119 | 138,147,186
                        ours 252,252,253  23, 41,119  23, 41,119  23, 41,119 |  23, 41,119
centre X=100 Y=0        ref  122,132,177 254,254,254  23, 41,119 156,164,197 | 138,147,186
                        ours  23, 41,119 251,252,253  23, 41,119  23, 41,119 |  23, 41,119
```

**24.2.7.2 draws a radial when the derived centre is not a corner, and a 45°/135° linear ramp to
the named corner when it is.** Whenever it draws a radial, we match it to within one or two levels
of 255 — centre, radius, ramp and all — which is a second useful negative: `SlideGradients.Centred`'s
half-diagonal radius, `Fills/Gradients.cs` and `Pdf/PdfShadings.cs` are all correct, and nothing
in this lane's own rendering is at fault.

### Why this is not a patch

The tree **used to have** that branch, at `PptxSlideLayout.cs:1411-1418`:

```
if (gradient.Path == "circle" && (cx is 0 or 100) && (cy is 0 or 100))
    // The focus is a corner: the reference draws the diagonal linear ramp instead, stop 0 at that corner.
    return SlideGradients.Linear(box, cx == 0 ? 1 : -1, cy == 0 ? 1 : -1, stops) …
```

— my six-arm probe reproduces that condition exactly, on the binary it was written for. Round 59
re-checked the site against **26.2.4.2** (`dotnet/TODO.24-2-7-audit.md`, `PptxSlideLayout.cs:1591`;
`probes/slides-r59/results.md` §5) by re-running round 39's own four-arm fixture, found all four
arms export `draw:style="radial"` there, and **removed the branch**. It was worth **−54.26
`abs_ink` and −244.20 differing pixels on this very document**, which was then the slides track's
third largest by unsigned ink.

Round 59 recorded one thing it could not do: *"I did not verify it against 24.2.7.2 — that binary
is not installable here — so 'the reference changed' is inference; 'the reference at 26.2.4.2 draws
a radial ramp' is measurement."* `probes/slides-b-01/results.md` §2c says the same. **That binary
is installed in this container and I ran it.** The corner rule is 24.2.7.2 behaviour, measured
directly on six arms rather than two. So the inference is now a measurement, the reference genuinely
changed between the two versions, and round 59's `FIXED` marker is validated rather than merely
plausible.

**Consequence for this sweep:** #100's SSIM 0.639 / MAE 0.229 is *expected* against a 24.2.7.2
reference and is not a defect to fix. #062's and #167's gradients are unaffected — their derived
centres are X=50%, which is radial in both binaries, as the reference's own nested circles at
(396, 1.7) on #062 page 1 confirm.

**Confidence: high.** **Not established:** *why* 24.2 chose a 45° angle for the linear case; the
version of `fillproperties.cxx` in this checkout is the newer one and sets `GradientStyle_RADIAL`
unconditionally, so the older branch cannot be read out of it. Nothing depends on knowing.

---

## RC-11 — NOT A DEFECT: a CMYK JPEG carrying an ICC profile

**Document: #176**, recorded so that the next reader does not "fix" it.

Once RC-7 puts the cover picture back on page 1, our colours will still not match. Sampling the
same picture in both renderings:

| sample | reference | ours |
|---|---|---|
| green header band | 73, 226, 0 | 84, 166, 69 |
| blue substation line-art | 102, 188, 220 | 109, 165, 201 |

`word/media/image1.jpeg` is a **4-component (YCCK/CMYK) baseline JPEG** with an Adobe APP14
transform marker and an embedded **ICC profile spread over nine APP2 segments (≈490 kB)**, written
by Photoshop CS5.1. `PdfImages.Write` correctly refuses to pass a 4-component JPEG through to
`DCTDecode` (`PdfImages.cs:53-70` — the comment there already says why) and falls through to
`RasterImageDecoder`, which decodes through Skia; Skia colour-manages, applying the embedded CMYK
profile on the way to sRGB. LibreOffice ignores the profile and converts naively, which is why its
green sits on the gamut edge.

**Ours is the more faithful conversion and it scores as a mismatch.** Same shape as
`TODO.raster-ceiling.md`. No patch proposed; whether to reproduce LibreOffice's naive conversion is
a policy call above this lane. Note also that `XRay.Office.Rendering/Images/RasterImageDecoder.cs:239`
is the one still-open `XRay.Office.Rendering` entry on `TODO.24-2-7-audit.md` (the `Bitmap::Adjust`
brightness/contrast branch); it is a different site and no document in this lane reaches it.

---

## Documents whose defect is not in this lane's area

- **#049** (`BMFE-06-03 (Gerflor)`): the hatch "spreading further" is **not** a hatch bug — the 14
  `<a:pattFill prst="wdUpDiag">` cells are the same 14 in both renderings, and the block looks
  taller because our **rows are taller**, which is also why the last row runs off the slide.
  Table row height in a PPTX table: L5/L1. The bottom artwork is `image3.png` and `image4.emf` on
  `slideLayout4`; both PDFs draw exactly one image on the page, so the PNG is drawn and the
  **EMF** logo is not — `XRay.Office.Vector`, shared.
- **#178** (`flying-by-numbers`): text set fractionally wider, definition list sits lower. The
  advance divergence. L1.
- **#190** (`ABCD-FE-01-00`): besides RC-6's watermark, every contents entry is drawn as a blue
  underlined hyperlink where the reference prints plain black — the recurring TOC field-styling
  problem — and the document runs one page longer. L3/L2.

---

## Cross-lane dependencies

Ordered by how many of this lane's documents each unblocks. **RC-3 must not be sequenced after
RC-1's fill limb.**

1. **`dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxFrames.cs` (L3) with
   `.../Layout/PageFrame.cs` and `.../Layout/PageDrawing.cs` (L2)** — RC-1, five documents.
   `Appearance:798` must read the whole fill chain rather than `a:solidFill` alone:
   `DrawingStyleMatrix.Fill(style, theme)` as the base, then the shape's own
   `a:solidFill`/`a:gradFill`/`a:blipFill`/`a:pattFill`/`a:grpFill` over it, producing a `Paint`
   instead of a `Colour?`. `PageFrame.Fill:333` widens from `Colour?` to `Paint?` and gains an
   outline. `DocxFrames` calls `CustomShapeGeometry.Preset` / `.Custom` and stores the result;
   `PageDrawing.DrawFrame:230` fills and strokes that outline instead of `frame.Area`. Line width
   falls back to `DrawingStyleMatrix.Line(style, theme)`'s `@w` when the shape's `a:ln` states
   none (`Appearance:810`) — what all 13 of #167's bars need. **Nothing has to be added to
   `XRay.Office.Ooxml/DrawingML`;** the probe above shows it is complete.
2. **`DocxFrames.cs` (L3) + frame placement (L2)** — RC-3, `@relativeHeight`. Read it onto
   `PageFrame` and sort by it within the behind-text and in-front-of-text bands. **Land this with
   or before item 1**, or #062 goes from a missing background to a background over its artwork.
   Same defect as L2's #024.
3. **`dotnet/src/XRay.Office.Presentations/Ooxml/PptxSlideLayout.cs` (L5)** — RC-4's consumer.
   Where a shape's geometry is filled use `geometry.FillOutline`; where it is stroked use
   `geometry.StrokeOutline`. Both fall back to `Outline`, so the change is safe on a geometry
   built without sub-paths (`OdpSlideLayout.cs:471`'s, for instance).
4. **`PptxSlideLayout.cs` (L5)** — RC-5. Read `a:scene3d/a:camera/a:rot/@rev` (60 000ths of a
   degree) and add it to the shape's rotation per `oox/source/drawingml/shape.cxx:1054-1064`; and
   draw a `p:pic`'s own `a:ln`. If a shared reader is wanted this lane will add one to
   `XRay.Office.Ooxml/DrawingML` on request — it did not add inert code speculatively.
5. **`dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxVmlFrames.cs` (L3)** — RC-6, two documents.
   Recognise `v:shape type="#_x0000_t136"` / a `v:textpath` with `on="t"`, take its `string`, and
   draw it filled with the shape's `fillcolor` and `v:fill/@opacity`, rotated by the style
   string's `rotation:`.
6. **`dotnet/src/XRay.Office.WordProcessing/Layout/` (L2)** — RC-7 and RC-8, both **position faults
   needing a 26.2.4.2 re-check first**. An inline object taller than the text area must overflow
   the page rather than move to the next (`612.5 × 792.65 pt` in a 648 pt text height: reference
   page 1, ours page 2); and a `positionV relativeFrom="paragraph"` anchor must resolve against
   its own paragraph's placed position.
7. **`XRay.Office.Presentations/MsBinary/PptShapeGeometry.cs` and `PptSlideLayout.cs` (L5)** —
   RC-9. Escher properties 464/465 (and 466-469 for width and length) into the `SlideLineEnd` pair
   `SlideLineEnds.Apply` already consumes.
8. **`XRay.Office.Vector` (shared)** — #049's `image4.emf` logo.

## Files this lane changed

```
patches/tiled-fill-truncated.diff        dotnet/src/XRay.Office.Rendering/Fills/Tiles.cs
patches/preset-subpath-fill-stroke.diff  dotnet/src/XRay.Office.Ooxml/DrawingML/CustomShapeGeometry.cs
                                         dotnet/src/XRay.Office.Ooxml/DrawingML/PresetShapeGeometry.cs
```

Both verified with `git apply --check -p1` against the checkout at `582c8c671`. Neither was built
or tested: per the brief the checkout is read-only to this lane. Nothing in `dotnet/tests`
references `Tiles`, `Tiles.Maximum` or `8192`, or constructs a `PresetPath` or a
`CustomShapeGeometry.Geometry`, so both are expected to be compile-neutral for the suite; the
three production `new PresetPath(...)` sites and the one `new CustomShapeGeometry.Geometry(a, b)`
site keep compiling because every added parameter is defaulted.

## Reproduction material

`work/` holds everything measured here:

- `mkdeck.py`, `parts.py` — build a minimal valid `.pptx` from slide bodies.
- `mkgrad.py`, `probe-grad.pptx`, `gradout/` (soffice 24.2.7.2), `gradout-pl/` (XRay.Office.Cli),
  `gradient-probe.txt` — RC-10's six-arm probe and its numbers.
- `bands.py` — pulls a page's content stream out of a PDF and classifies gradient bands.
- `tile-measurement.txt` — RC-2's tile counts, regenerable in one command.
