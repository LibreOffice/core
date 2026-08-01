# Paperless.Vector — TODO

Importers for embedded vector graphics: WMF, EMF, EMF+ and SVG.

**Decided: full support for all four.** Not a subset, and no rasterise-via-LibreOffice
shortcut. SVG reuses an existing library (see below); WMF, EMF and EMF+ are ours to write,
because nothing exists in C# for them. **All four are done.** The shared metafile groundwork
in `Metafiles/` is what the second and third formats were built on rather than a blank file,
and *What the three metafile formats taught* below is the section written for whoever
maintains them.

That makes this the largest single body of work in the project, so it is worth being clear
why it earns that. Office documents embed these constantly — pasted clip art, chart and
Visio snapshots, equation images, and the fallback rendering that accompanies SmartArt and
OLE objects. Anything less than real vector import shows up as blank rectangles on visually
important pages, and rasterised substitutes look wrong at print resolution.

Reference: `research/06-rendering.md` section D. For the metafile formats, LibreOffice's
`emfio/` is the working reference — port its record handling rather than reading the
specifications cold.

---

## The seam, and what a metafile implementer has to supply

**Read this first.** SVG settled the shape of this seam and WMF was the first format to plug
a stateful command stream into it; EMF and EMF+ plugged into the same one without changing
it, which is the strongest evidence the seam is in the right place. *What the three metafile
formats taught* is the section written for whoever maintains them.

### What a decoded vector image is

Not pixels, and not a live replay. `IVectorImageDecoder.Decode` answers a `VectorImage`:

| Piece | What it is | SVG | WMF | EMF and EMF+ |
|---|---|---|---|---|
| `Content` | A `DisplayList` — recorded `IDrawingSink` calls, replayable | the translated command list | the replayed records, already mapped to EMUs | the replayed records |
| `ViewBox` | The rect of `Content`'s coordinates that fills the destination, **in EMUs** | the viewport, converted at 9525 EMU per user unit | the viewport extent, which the mapping has already resolved | the frame's own size, since the mapping subtracts the frame origin |
| `IntrinsicSize` | The physical size the picture asks for when nothing imposes one | `width`/`height` against the CSS inch | the window extent against the placeable header's units-per-inch | `rclFrame`, falling back to `rclBounds` through `szlDevice`/`szlMillimeters` |

**EMF+ has no row of its own because it has no file of its own.** It rides inside an EMF's
comment records, so the header question, the seam and the registration are all the EMF
decoder's; see *EMF+* below.

`DisplayList` is itself an `IDrawingSink`, so a decoder writes to the same interface a
backend implements and needs no second output path. Recording rather than drawing straight
through is what makes one decode serve a logo that appears on forty slides, and what makes
the commands countable for `VectorLimits`.

**Keeping `ViewBox` and `IntrinsicSize` apart is the point.** For SVG they usually coincide,
and for WMF they always do — because every coordinate is mapped to 1/100 mm as it is read, so
the space the commands are in *is* the physical one. For EMF the two are computed from
different header fields and neither is derived from the other: `rclBounds` is the logical
space in device units, `rclFrame` the physical extent in 1/100 mm, and `szlDevice` against
`szlMillimeters` the scale relating them. They then agree *numerically*, because the mapping
subtracts the frame origin — but a file whose bounds disagree with its frame about the scale
gets both right here where a decoder that derived one from the other gets one of them wrong.
Collapsing them is the classic wrongly-scaled metafile.

### What the seam does not give you

- **No device context.** `Save`/`Restore`/`Transform` on the sink are a clip-and-transform
  stack, nothing more. There is no selected pen, brush, font or palette, no mapping mode and
  no current position, because `IDrawingSink` is deliberately stateless per draw call. The DC
  is the metafile implementer's to build, once, shared by all three metafile formats — **it is
  built, in `Metafiles/`**; see *Shared groundwork* below.
- **No path arithmetic.** `ClipPath` intersects and nothing unions, subtracts or offsets.
  The SVG translator gets away with expressing a union as one path of several subpaths under
  the non-zero rule; a *region* turns out to get away with it too, because a GDI region is a
  scan list of disjoint rectangles and disjoint subpaths are their own union under either
  rule. Subtraction turned out to be reachable too, once the rectangular part of a clip was
  kept apart from the arbitrary part: a rectangle minus a rectangle is at most four
  rectangles, and subtraction distributes over intersection. What is left is *union* and
  *symmetric difference* with the clip itself as an operand, which need the clip's own area;
  those raise `PL6034` and leave the clip as it was, drawing too much rather than too
  little.
- **No stroked text.** `DrawGlyphRun` takes one paint. SVG's `fill="none" stroke="red"` text
  is filled in the stroke's colour with a `PL6015` diagnostic. Neither metafile family needs
  more: GDI has no stroked text at all, and EMF+ `DrawString` and `DrawDriverString` both
  name a *brush* rather than a pen — so SVG is still the only caller this costs anything.

### What a decoder owes

1. **Convert to EMUs once, at the boundary.** `Svg/ShimGeometry.cs` and
   `Metafiles/MetafileMapping.Emu` are the two worked examples: one constant, one function,
   and nothing downstream that knows what unit the source used. A scale factor applied in two
   places is a scale factor applied twice, and that is the commonest way vector import comes
   out at the wrong size.
2. **Honour `VectorLimits`.** Charge every recorded command, check the clock on every one,
   cap path segments, records and nesting depth. `Svg/SvgPictureTranslator.cs` and
   `Metafiles/MetafileBudget.cs` show the pattern: a charge that returns false and sets
   `IsTruncated` rather than throwing.
3. **Resolve nothing outside the document.** No URL, no file path, whatever the format
   allows. WMF's `ESCAPE` records are the metafile analogue of the SVG `href` that turned out
   to read `/etc/passwd`: an open extension point in an attacker-supplied file. Two payloads
   are read there and both name only bytes inside their own record; everything else in an
   escape is ignored, and `WmfSafetyTests` asserts it. An EMF comment is the same hole one
   layer down — a whole EMF+ record stream that no validator, scanner or thumbnailer has
   looked at — and `EmfPlusSafetyTests` asserts the same invariants over it.
4. **Never throw for malformed input.** A picture that cannot be read is a document to draw
   *without* that picture: return `VectorImage.Empty` with a diagnostic. Diagnostic codes
   `PL60xx` belong to this library.
5. **Register in `VectorImages`** and sniff by content in `CanDecode`. Nothing that calls
   `VectorImages.Decode` changes when a format is added, which is the point of the seam.

### What the three metafile formats taught

**Read this before changing anything in `Metafiles/`, `Wmf/`, `Emf/` or `EmfPlus/`.** The
groundwork below exists and works; what follows is what it turned out to need that was not
obvious from the seam. The first eight points are WMF's, found while it was the only format;
the last group is what EMF and EMF+ added, and closes the section. Every point cost something
to find.

**The clip wants to be a list of intersections, not a region.** This is the discovery with
the most leverage. `IDrawingSink.ClipPath` intersects and offers nothing else, which looks
like a hole until you notice that a clip held as an ordered *list* of paths to intersect
needs no path arithmetic at all: replaying the list **is** the intersection, and the sink
does the work. Everything GDI expresses by intersecting — `IntersectClipRect`, selecting a
region, an `ETO_CLIPPED` text rectangle — then lands exactly, with no approximation
anywhere. `Metafiles/MetafileClip.cs`. Only subtraction is left over, and it is reported
(`PL6034`) rather than approximated. **`ExtSelectClipRgn` with `RGN_AND` and `RGN_COPY` will
need nothing new; only `RGN_DIFF` and `RGN_XOR` will.**

**But the clip has to be emitted lazily, with a restore and a save.** A metafile's clip is
*device state* — it changes when a record says so and stays changed — while a sink's clip is
a *scope* that only ever narrows. The only way to widen a sink's clip is to restore. So the
painter keeps exactly one save level open for the clip and, when the clip has changed since
the last drawing call, restores and saves again (`Metafiles/MetafilePainter.cs`,
`EnsureClip`). Doing it lazily rather than on every clip record matters because files set
the clip far more often than they draw through it. This is `MtfTools::UpdateClipRegion`
(`emfio/source/reader/mtftools.cxx:1254-1289`) and it is not obvious from either side.

**Map at read time, so the display list holds no logical coordinates.** Every point goes
through `MetafileMapping` as the record is read and the recorded commands are already in
EMUs. That is why `ViewBox` and `IntrinsicSize` coincide for WMF — the coordinate space the
commands are in *is* the physical one — and it is also why the two must stay apart for EMF,
whose `rclBounds` is in device units and `rclFrame` in 1/100 mm, stated independently. The
intermediate unit throughout is 1/100 mm, because that is what every conversion factor in
`ImplMap` is expressed in, which makes the port checkable line by line; 914400/2540 = 360 is
exact, so nothing is lost turning it into EMUs at the one boundary
(`MetafileMapping.Emu`).

**A pen's width and a font's size are mapped when the object is *created*, not when it is
used.** GDI states them in logical units and freezes them at creation
(`mtftools.cxx:1027-1035`), so a file that changes the window extent between creating a pen
and drawing with it will disagree with any decoder that maps late. `MetafilePen.Width` and
`MetafileFont.Size` are therefore already in EMUs.

**The device context needed eight fields beyond the obvious four.** Pen, brush, font and
palette are the easy part. The ones that are load-bearing and easy to miss: a flag for
*whether a brush has ever been selected* (until one has, GDI fills with the **background**
colour, not with the default white brush — i57205); the current position in **logical**
units, because a mapping record may intervene between the `MoveTo` and the `LineTo`; the
polygon fill mode, which is the sink's `FillRule`; the background mode, which decides whether
hatch gaps and text cells are painted; the text-alignment word; the arc direction; and a
no-op flag for `R2_NOP`. All in `Metafiles/MetafileDeviceContext.cs`.

**`SaveDC`/`RestoreDC` is not the sink's `Save`/`Restore` and cannot be built on it.**
`RestoreDC` takes a *signed* argument: negative counts back, positive names a specific saved
state. WMF writes both spellings, sometimes in one file, and treating a positive value as a
count discards states a later record still expects. It also saves the mapping and the
selected objects, which the sink's stack knows nothing about — while *not* saving the object
table, because GDI objects belong to the device and a `RestoreDC` does not un-create a brush.

**Nothing in `Paperless.Core` had to change.** Not the sink, not `Paint`, not `GraphicsPath`.
The two gaps the seam already records — no path arithmetic, no stroked text — are the only
two WMF ran into, and both were expressible as an honest diagnostic. Two things that looked
like they would need Core and did not: a hatch brush became stroked lines clipped to the
shape (`Paint`'s own remarks already prescribe this), and a DIB became `RasterImage.Encoded`
by prepending fourteen bytes of BMP file header — a DIB *is* a BMP without one, so nothing
here decodes a pixel (`Metafiles/DeviceIndependentBitmap.cs`). **A source rectangle survives
the same way**: rather than cropping, place the whole image scaled so the wanted part lands
on the destination and clip to the destination. Same picture, no codec.

**Text needs three things the seam does not hint at.** GDI's point may be any of nine
positions, so resolving it needs the run's *measured* width and the face's ascent — a
decoder that ignores the alignment word draws every centred label half a string too far
right. The DX array is cumulative in the file and per-glyph in `GlyphRun`, and must be lined
up through the cluster map rather than by position. And a rotated baseline has nowhere to
live on `GlyphRun`, so it becomes a `Transform` about the run's own origin — which keeps the
run one run, so a PDF backend still emits real searchable text.

### And what the other two added, which closes this section

**Three formats, one device context, and the sharing is the reason all three fit.** WMF, EMF
and EMF+ are a 16-bit, a 32-bit and a wholly different command stream, and they share
`MetafileDeviceContext`, `MetafileClip`, `MetafilePainter`, `MetafileBudget`,
`MetafileTextEngine` and `DeviceIndependentBitmap` without any of them branching on format.
The two things they do **not** share are worth stating, because a maintainer's instinct will
be to unify them and both attempts would be wrong:

- **The mapping.** WMF and EMF share `MetafileMapping` because both are GDI: eight mapping
  modes, a window, a viewport and a world transform. EMF+ has none of that — a world
  transform, a page transform with a unit, and the reference device — so it carries its own
  four-stage composition (`EmfPlusReader.Remap`). Forcing it through `ImplMap` would mean
  adding a ninth pseudo-mode that no GDI file ever uses.
- **The object table.** `GraphicsObjectTable` implements GDI's lowest-free-slot rule and its
  delete semantics because WMF states no handle at creation. EMF+ names a slot on every
  record, has no create and no delete, and simply overwrites — 256 slots, indexed by the
  flags word's low byte. Sharing the table would mean giving GDI's rule an exemption that
  swallows it.

**A format that carries another format is the rule rather than the exception, and the
decision must be made once, in front of the record loop.** A WMF carries a whole EMF in its
escape records; an EMF carries EMF+ in its comment records; an EMF+ image object can carry a
whole further metafile. In every case replaying both descriptions draws the picture twice,
and switching part-way produces a picture that is coherent nowhere. `EmfReader.Replays` is
where the EMF/EMF+ decision lives and it is three lines; getting it *stated* took reading
`emfreader.cxx:955-963` rather than reasoning about it, because the rule is not the obvious
one — see the EMF+ section.

**`Paperless.Core` changed exactly once, and only where two formats asked for the same thing.**
Six things looked as though they would need it and did not: a hatch became stroked lines clipped
to the shape; a DIB became `RasterImage.Encoded` by prepending fourteen bytes of BMP header; a
source rectangle became a scale plus a clip; a constant source alpha became the sink's own
opacity; a transparent-bitmap idiom became straight RGBA arithmetic over an uncompressed DIB,
which is not a decode; and an EMF+ parallelogram destination became a `Transform` plus a
fixed-size placement square. **Three did need it, and the test that decided each was the same:
does a second format ask for it too.** All three were gradients, all three were recorded twice
— once here and once by SVG — and all three now live in `Paperless.Core/Graphics/Paint.cs`:

| What | Why it could not be worked round | What it retired |
|---|---|---|
| `MeshPaint` — vertices with a colour each, plus index triples | A `GradientPaint` has one colour at each *end* however many stops sit between; a GDI+ path gradient states one at every vertex of an arbitrary boundary. No ramp says that. | `PL6040` |
| `SpreadMethod` on `GradientPaint` | Both backends tile natively and the IR could not ask them to | `PL6041`, SVG's `PL6021` |
| `Focus` on `GradientPaint` | PDF's `/ShadingType 3` and Skia's two-point conical both take two circles; the IR could only state one | SVG's `PL6018` |

**Why a mesh and not something Skia- or PDF-shaped.** Both backends have a native form and the
two agree on exactly this much: a PDF type 4 shading is a stream of triangles carrying a
coordinate and a colour per vertex, and Skia's `SkVertices` is a position array, a colour array
and an index array. Vertices plus index triples is their intersection, so neither backend
reconstructs anything and Core gains no dependency. A patch mesh — PDF type 6 or 7 — would have
carried curved edges that Skia cannot draw, and a bitmap would have needed a rasteriser in a
library arranged not to have one.

**A seventh thing that looked like a Core change and was not: the caps.** Seven of GDI+'s ten
line caps are *decorations* rather than caps — a triangle, four anchors and a custom path planted
at a line's end — and `LineCap` expresses none of them. They needed nothing new, because a
decoration is a filled path: `EmfPlusReader.DrawCaps`. `PL6038` retired with them.

**Measure against LibreOffice, but do not assume LibreOffice is right.** The method works —
WMF reached `mae 0.0000` and EMF `0.0001` on LibreOffice's own exports — and it found real
bugs in this reader. It also found three places where LibreOffice's PDF export drops or
mis-draws what its own importer produced, and on each of those a raw `ink_ratio` reads as a
defect here when it is not:

| File | What LibreOffice does | Measured |
|---|---|---|
| `TestEmfPlusFillClosedCurveWinding.emf` | fills a winding-rule star **even-odd**, leaving the pentagon hole its own test name says should be solid | `ink_ratio 1.431` |
| `TestEmfPlusFillRectsWithTextureBrush.emf` | draws the outline and not the texture, though its unit test asserts the texture is in the primitive tree | `ink_ratio 24.3` |
| `TestEmfPlusSetPageTransform.emf` | draws the label and neither of the two filled rectangles its unit test asserts | `ink_ratio 144` |
| `TestEmfPlusDrawLineWithDash.emf` | applies every `RotateWorldTransform` **last**, whatever the record's pre-multiply flag says | `mae 0.0662` |
| `TestEmfPlusBrushPathGradientMultiSurroundColor.emf` | draws only the ramp from the centre to `surround[0]`, so a star with a red, a green and a blue point comes out with no red or green pixel anywhere | `mae 0.0113` |

So read the diff image before believing a number. A high `ink_ratio` with a low
`mean_abs_error` — the first three above — means the two renderers disagree about a *small*
area, and it is as likely to be theirs as ours.

**The two additions to that table are the two most expensive measurements in this file**, and
both are cases where matching LibreOffice would have meant being wrong:

- **The rotation.** `emfphelperdata.cxx:2606-2621` reads the angle, logs the post-multiply
  flag, and then says *"Skipping flags & 0x2000 — for rotation transformation there is no
  difference between post and pre multiply"*. That is false whenever the world transform is not
  a uniform scale, which is exactly what this file states, and LibreOffice honours the same flag
  correctly on `Translate`, `Scale` and `Multiply` five lines away. The symptom is a *fan* of
  rays whose first few match exactly and whose angles then drift apart — which reads as a dash
  problem, because the file is nothing but dashes, and is not one. Adopting LibreOffice's
  shortcut takes this file from **`mae 0.0651` to `0.0034`** and was not done.
- **The path gradient.** LibreOffice 24.2 renders one, and the tree beside it renders another:
  the mesh implementation in `emfphelperdata.cxx`'s `BrushTypePathGradient` branch is what
  tdf#143031 added, and its unit test says in as many words *"Without the fix, an empty or
  flat-coloured fill primitive was emitted"*. The reference `soffice` here is 24.2.7.2 and emits
  the flat one. So the number went **up** when the mesh landed, and the picture got right.

**Check which LibreOffice wrote the reference before attributing a regression to yourself.**
`soffice --version` against `git log` on the file you ported from costs a minute and is the
difference between a real defect and a fixed bug measured backwards.

---

## Shared groundwork

Done, and shared with EMF and EMF+ when they arrive. `src/Paperless.Vector/Metafiles/`.

- [x] **A device-context model**: mapping, clip, selected pen/brush/font/palette, the drawing
      modes and the save stack. `MetafileDeviceContext.cs`, ported from `MtfTools`
      (`emfio/source/reader/mtftools.cxx:3043-3160` for the save stack). See *What the first
      metafile format taught* above for what it needed beyond the obvious.
- [x] **A graphics-object table** with handle reuse and delete semantics.
      `GraphicsObjectTable.cs`. WMF never states a handle when it creates an object — the
      handle is the index of the *lowest free slot* — so a decoder that appends assigns
      different handles from the producer as soon as the file deletes anything, and every
      later `SelectObject` picks the wrong pen. A `_lowestFree` cursor keeps creation from
      being a linear scan, which is a denial of service on untrusted input rather than merely
      a slow decode. Deleting a handle twice, deleting one that was never created, and
      selecting one that was never created are all tolerated: real files do all three.
- [x] **Mapping-mode and window/viewport arithmetic.** `MetafileMapping.cs`, a straight port
      of `MtfTools::ImplMap` (`mtftools.cxx:541-626`). Five of the eight modes flip the y
      axis, so a decoder that ignores the mode does not merely draw at the wrong size — it
      draws upside down. `MapSizeMm100` is a separate path from `MapPointMm100` because the
      window origin, the viewport origin and the frame offset are translations and must not
      be applied to a distance.
- [x] **Bounded replay: a record count and time cap.** `VectorLimits` caps bytes, commands,
      path segments, nesting depth, expanded source nodes, wall-clock time and — added for
      the metafiles — `MaxRecords`. **A record cap and a command cap bound different things**
      and both are needed: most metafile records emit no drawing command at all, so a file of
      ten million `SaveDC`s stays under any command cap while still costing ten million
      parses; conversely one hatch fill emits a stroke per line. `MetafileBudget.cs` charges
      both plus the clock, and `MetafileBudget.Plausible` refuses a stated count before it
      allocates — the cheapest guard there is, and the one that catches the commonest attack,
      a record claiming 400 000 points inside a 200-byte file.

## WMF — done

`Wmf/WmfImageDecoder.cs` + `Wmf/WmfReader.cs`, ported record by record from
`emfio/source/reader/wmfreader.cxx`. Registered in `VectorImages`; sniffed by content, so a
WMF stored under an `.emf` part name is decoded anyway.

### What draws

- [x] **Placeable header** (`D7 CD C6 9A`) and the bare metafile header, including the
      `0x00010000`/`0x0009` spelling a memory metafile writes.
- [x] Object records: pen, brush, font, palette and **region** — region scan lists are read
      into a path, which `emfio` does not do at all (`wmfreader.cxx:1246-1251` creates a
      placeholder and warns), so a WMF that clips through a region draws unclipped there and
      here does not.
- [x] Drawing records: `TextOut`, `ExtTextOut`, `Polygon`, `PolyPolygon`, `Polyline`,
      `Rectangle`, `RoundRect`, `Ellipse`, `Arc`, `Chord`, `Pie`, `LineTo`/`MoveTo`,
      `SetPixel`, `PatBlt`, `FillRegion`/`PaintRegion`.
- [x] Bitmap records: `StretchDIB`, `DIBBitBlt`, `DIBStretchBlt`, **`SetDIBitsToDevice`**
      (which `emfio` does not implement either), and `BitBlt`/`StretchBlt` for the monochrome
      device-dependent form. A DIB goes through as `RasterImage.Encoded` — a DIB is a BMP
      without its fourteen-byte file header, so re-wrapping it needs no codec here.
- [x] Clipping: `IntersectClipRect`, `OffsetClipRgn`, `SelectClipRegion`, and the
      `ETO_CLIPPED` rectangle on a text record, which is scoped to that record alone.
- [x] `ESCAPE`: the `WMFC` comment that carries a chunk of a complete EMF is **reassembled
      and replayed** rather than reported (`PL6030` remains for the file that carries only
      part of one), and the private Unicode text escape is
      drawn, with the count of following legacy records honoured so nothing double-strikes.
- [x] Text through `Paperless.Text` — the same substitution table and the same HarfBuzz as
      body text — with the code page taken from the selected font's character-set byte, all
      nine alignment positions, per-character DX arrays and baseline escapement.
- [x] Hatch brushes as stroked lines clipped to the shape, and pattern brushes as a
      `BitmapPaint` tile.

### The rules, and what they were measured against

**The placeable header's bounding rectangle is a decoy.** It is read and then discarded in
favour of a scan of the records, which is what `emfio` does too (`wmfreader.cxx:1580`
overwrites it unconditionally). The window records win, the viewport records are the
fallback, and the extent of everything drawn is the last resort — and that last resort is not
a repair for broken files, since plenty of real metafiles state no window and are meant to be
measured. What the header supplies that the records cannot is the **units-per-inch** field,
and that is the half that is kept.

**The logical coordinate space and the physical extent are stated independently**, which is
the `ViewBox`/`IntrinsicSize` split in its WMF form. The window records give the coordinate
space; units-per-inch turns it into millimetres. Deriving either from the other gives a
picture that is the right shape and the wrong size. Measured on LibreOffice's own WMF export:
a placeable header of 2540 units to the inch with an 8000 × 6000 window is 80.00 × 60.00 mm,
and it renders at exactly that.

**An anisotropic placeable metafile ignores its own resolution when it exceeds its window.**
Undocumented, and load-bearing: honouring 1440 units to the inch for a 100-unit window would
make the picture 1.8 mm across. Other office suites ignore it and so does LibreOffice
(`wmfreader.cxx:2142-2156`); the fallback is `max(width, height)` units to the inch.

**Fidelity, by rasterising our PDF and LibreOffice's with the same rasteriser so antialiasing
cancels** (`pdftoppm -r 150`, then `render-comparison`'s `compare-images.py`). Both PDFs place
the same metafile at the same size on the same A4 page:

| Picture | `mean_abs_error` | Note |
|---|---|---|
| LibreOffice's own WMF export of a drawing: 38 filled polygons, 2 poly-polygons, a clip | **0.0000** | `differing=0.0001`, `shifted=0` — antialiasing level |
| rectangle, ellipse, pie, polyline, one text run, five pen widths | **0.0002** | `ink_ratio=1.003`, `shifted=0` |
| the same with one hatched brush added | 0.0047 | 18 % less ink; see the hatch note below |

**A WMF is 78 % EMF, in the file LibreOffice itself writes.** Measured: a 16 700-byte WMF
exported by LibreOffice 24.2 carries **12 964 bytes of a complete EMF** inside two `ESCAPE`
comment records, and an 18 276-byte one carries 14 032 bytes byte for byte identical to what
the same drawing exports to as an EMF on its own. Replaying both would draw the whole picture
twice. **The switch has since been flipped**: a WMF that carries a complete EMF replays the
EMF, decided once in front of the record loop, which is the same rule and the same reason as
the EMF/EMF+ decision below.

### Traps, each of which cost time

**`TA_BASELINE` is 0x0018 in the specification and 0x0010 in reality.** [MS-WMF] 2.1.2.3 has
it wrong, `emfio` says so in a comment (`inc/mtftools.hxx:184-186`), and producers that
believed the specification write 0x0018 — which is `TA_BASELINE | TA_BOTTOM`. A masked field
comparison then matches *neither* and falls through to top alignment. Measured: a
baseline-aligned run asked for at y = 6.00 mm was drawn at **y = 9.62 mm**, the face's entire
ascent lower. The fix is to test the bits in priority order rather than to compare the
masked field, which is what `MtfTools::DrawText` does (`mtftools.cxx:2016-2021`). Nothing
about the symptom looks like an alignment bug; it looks like a mapping bug, because the whole
run is displaced.

**An arc's ends are points on a ray, and the ray's angle is not the ellipse's parameter.**
GDI names an arc's ends by points that need not lie on the ellipse: the point names the ray
from the centre that the end sits on. Converting has to go through the ellipse's own
parameter, `atan2(rx·sin θ, ry·cos θ)` — `ImplGetParameter`,
`tools/source/generic/poly.cxx:60-67`. Measured on a 4:1 ellipse: a ray at 45° crosses at
parameter 1.326 rad = **76°**, so using the ray's angle puts the end of the arc 31° away,
about a twelfth of the way round. It is invisible on a circle, which is what makes it a trap:
every test with a circular arc passes. **EMF+ states its arcs as angles instead**, and those
angles *are* the ellipse's parameter — so none of this conversion applies there, and the two
arc implementations look wrongly duplicated until you notice why.

**The arc parameter is measured with y upwards.** GDI's default arc direction is
anticlockwise in the *logical* space, and the parameterisation is
`(cx + rx·cos t, cy − ry·sin t)`. Both signs have to survive the port together; getting one
of them wrong mirrors every arc, and a symmetric test picture cannot see it. A degenerate
start point — a file naming the ellipse's own centre as an arc end — makes `atan2(0, 0)`
answer 0, and LibreOffice and this decoder then disagree about which 45° of the ellipse is
meant. Not worth chasing; it is a malformed record either way.

**A blit record with no bitmap is told apart from one with a bitmap by its size alone.**
There is no flag. The record's high byte is its original 16-bit parameter count, so a record
exactly three words longer than that has nothing after its parameters
(`wmfreader.cxx:919`). Reading the bitmap that is not there is how a decoder walks off the
end of a *legitimate* file — not a malformed one.

### Not done, and why

- [ ] **Ternary raster operations other than `SRCCOPY`.** `PATINVERT`, `SRCAND`, `SRCINVERT`
      and the rest combine the source with what is already on the page, which a display list
      cannot read back. The source is drawn alone and `PL6033` is raised, which is what
      LibreOffice falls back to as well. `BLACKNESS` and `WHITENESS` are honoured exactly.
- [x] **The transparent-bitmap idiom, closed when EMF arrived.** A monochrome mask blitted
      with `SRCAND` followed by a colour image with `SRCPAINT` is how a WMF says
      "transparent", and merging the two into one alpha bitmap is what
      `MtfTools::ResolveBitmapActions` (`mtftools.cxx:2557`) exists for. It looked as though
      it needed a codec and it does not: an **uncompressed** DIB is not an encoded format but
      a stride, a channel order and a palette, so merging it is arithmetic. One blit is held
      back so the pair can be seen; the compressed forms answer null and fall back to drawing
      the source alone.
- [x] **`ExcludeClipRect`, closed when EMF arrived.** A rectangle minus a rectangle is at most
      four rectangles, and subtraction distributes over intersection — so keeping the
      rectangular part of a clip apart from the arbitrary part makes an exclusion exact even
      under a non-rectangular clip path, with no general path arithmetic anywhere.
      `PL6034` now means only union and symmetric difference with the clip as an operand.
- [ ] **A hatched brush is a solid fill in LibreOffice.** Measured: LibreOffice paints a WMF
      `HS_DIAGCROSS` brush as a solid fill of the hatch colour, on **both** its PDF and its
      PNG export paths; Windows paints lines. Lines are what is drawn here, because that is
      what GDI does and what Word and PowerPoint show — at the cost of `mae 0.0047` and 18 %
      less ink against the reference on a picture with one hatched shape in it. Switching to
      LibreOffice's answer is one branch in `MetafilePainter.Fill`, so this is a decision
      rather than a limitation. Assess against real clip art before changing it.
- [ ] **Device-dependent bitmaps deeper than one bit.** `BitBlt`/`StretchBlt` with an inline
      24- or 32-bit DDB. `emfio` reads them (`wmfreader.cxx:963`); they are rare because
      almost everything writes `STRETCHDIB` instead, and reading them means building a BMP
      header from the device's own format. `PL6031`.
- [ ] **`FloodFill`, `ExtFloodFill`, `InvertRegion`, `FrameRegion`, `DrawText`.** The three
      fills need to read the page back; the other two are rare enough not to have been seen.
      `PL6031`.
- [ ] **A pattern brush's tile size is assumed to be 96 pixels to the inch.** A GDI pattern
      brush tiles in *device* pixels and a display list has none, so something has to choose.
      96 is what the rest of Paperless assumes a nominal pixel is; a pattern-filled shape will
      therefore be right in colour and possibly wrong in scale.

## EMF — done

`Emf/EmfImageDecoder.cs` + `Emf/EmfReader.cs`, ported record by record from
`emfio/source/reader/emfreader.cxx` onto the groundwork WMF built. Registered in
`VectorImages`; sniffed by content, which is load-bearing rather than tidy — LibreOffice
writes a genuine EMF into `word/media/image1.wmf` and `[Content_Types].xml` declares no type
for `.wmf` at all, so neither the name nor the declared type identifies it.

### What draws

- [x] The header, and the four independent quantities in it. `rclFrame` is the physical
      extent in 1/100 mm, `rclBounds` the logical space in device units, and `szlDevice`
      against `szlMillimeters` the scale that relates them; none is derivable from another,
      and the frame's origin is subtracted from every mapped point.
- [x] Path construction — `BeginPath`/`EndPath`/`StrokePath`/`FillPath`/`StrokeAndFillPath`
      and `CloseFigure` — through `MetafilePath`, the one piece of state WMF did not need.
- [x] World-transform records, `SetWorldTransform` and `ModifyWorldTransform` in all four of
      its modes.
- [x] Drawing records including `PolyDraw`, `AngleArc`, `ArcTo`, the 16-bit forms of every
      polygon record, and `EMR_GRADIENTFILL` in both rectangle modes.
- [x] Text: `ExtTextOutW`/`A`, `PolyTextOut`, `SmallTextOut`, glyph-index runs, per-glyph DX
      arrays and the record's own page-to-device scale factors.
- [x] Bitmaps: `StretchDIBits`, `SetDIBitsToDevice`, `BitBlt`, `StretchBlt`, `AlphaBlend`
      with both kinds of transparency, and `TransparentBlt`.
- [x] Clipping: `ExtSelectClipRgn`, `IntersectClipRect`, `ExcludeClipRect`, `OffsetClipRgn`
      and `SelectClipPath` — and the clip gained exact rectangle algebra here, which
      retroactively closed WMF's `ExcludeClipRect` as well.
- [x] Pens: geometric against cosmetic, the five predefined dash styles and a user dash
      array, caps and joins.
- [x] **The EMF a WMF is hiding.** A WMF now replays the complete EMF its escape records
      carry, decided once in front of the record loop.

### What it was measured against

Measured when EMF landed, both PDFs rasterised at `pdftoppm -r 150`:

| Picture | `mean_abs_error` | Note |
|---|---|---|
| LibreOffice's own EMF export of a four-shape drawing | **0.0001** | `differing=0.0007`, `shifted=0` |
| a plain blit, the transparent-mask idiom and an `AC_SRC_ALPHA` `AlphaBlend` | **0.0000** | `differing=0.0000` — identical |

And the dual-format measurement that settled the WMF question: an 18 276-byte WMF
LibreOffice writes carries **14 032 bytes of EMF**, byte for byte the EMF the same drawing
exports to on its own.

## EMF+ — done

`EmfPlus/EmfPlusReader.cs` and its three companions, ported from
`drawinglayer/source/tools/emfphelperdata.cxx` and the seven files beside it, with the
record dispatch in `emfio/source/reader/emfreader.cxx` deciding which description is
replayed.

**Nothing registers in `VectorImages` and nothing new is sniffed**, because EMF+ is not a
file format: it has no header on disk, no signature and no extension of its own. It is a
second description of the same drawing, carried in `EMR_COMMENT_EMFPLUS` records inside an
ordinary EMF, so the EMF decoder is what finds it and the seam does not change at all.

### The rule that had to be settled first

**Which of the two descriptions to replay is `emfreader.cxx:955-963`, and it is not the
obvious rule.** The obvious rule is "if the header says dual, prefer EMF+", and it is wrong
in both directions. What LibreOffice does, and what `EmfReader.Replays` now does:

1. **Before any EMF+ record has been seen, GDI records are drawn.** A file carrying no EMF+
   at all is untouched by any of this, and one whose EMF+ starts late still draws what came
   before it.
2. **Once one has been seen, GDI records are not drawn — whether or not the header called the
   file dual.** This is the half that surprises. An EMF+ *Only* file still carries GDI
   records, and they are a "this needs GDI+" notice rather than a drawing; a *Dual* file's
   are the same picture again. The dual flag turns out to gate nothing in the decision.
3. **Except immediately after `EmfPlusGetDC`**, which exists precisely to hand the device
   context back for the records GDI+ has no form for, and which lasts until the next comment.
   And except `EMR_EOF`, which is structural.

So the choice is made once, in front of the record loop, never mid-stream — the requirement
the WMF note above states — with the hand-back as the one bracket the format itself defines.
Because the two readers share one `MetafileDeviceContext` and one `MetafilePainter`, a
bracketed GDI record draws under the EMF+ clip, which is what makes the hand-back mean
anything.

### What draws

- [x] Record framing, the header, and the multi-part **continuation** form: an object larger
      than one record repeats with the top flag bit set, and each part after its four-byte
      total-size field is appended until a record arrives that is not part of the same
      object. A reader that parses each fragment alone reads the first as a whole object and
      draws nothing.
- [x] The object table — 256 slots named by the flags word's low byte, overwritten in place,
      with no create, no delete and no handle arithmetic. Brushes, pens, paths, regions,
      images, fonts and string formats are read; image attributes and custom line caps
      consume their slot and nothing else.
- [x] `FillRects`, `DrawRects`, `FillPolygon`, `DrawLines`, `FillEllipse`, `DrawEllipse`,
      `FillPie`, `DrawPie`, `DrawArc`, `FillPath`, `DrawPath`, `FillRegion`, `DrawBeziers`,
      `DrawCurve`, `DrawClosedCurve`, `FillClosedCurve` and `Clear`.
- [x] `EmfPlusPath` with its point-type array, in all three point encodings — float,
      compressed 16-bit and **relative**.
- [x] Brushes: solid, hatch, texture, linear gradient and path gradient, with blend-factor and
      preset-colour ramps on both gradients, and every wrap mode on both.
- [x] **A path gradient as the fan of Gouraud triangles it is** — `(centre, V(i), V(i+1))` with
      the centre colour and the two surround colours at its corners — through Core's
      `MeshPaint`. One ring of triangles where the ramp is linear, because barycentric
      interpolation across a triangle is; twelve when a blend curve or a preset colour list bends
      it, which a triangle cannot.
- [x] Pens, including all thirteen optional fields of the pen-data blob, the five predefined
      dash styles, a custom dash array, and **the seven line caps that are decorations rather
      than caps**: triangle, the four anchors and a custom cap's own path.
- [x] **A metafile carried as an image**, replayed into the same placement square a bitmap goes
      in and bounded by `VectorLimits.MaxNestingDepth` rather than by the shared budget.
- [x] `DrawString` with `EmfPlusFont` and `EmfPlusStringFormat` — alignment, line alignment,
      margins and tracking — and `DrawDriverString` in both its character and glyph-index
      forms, split into one run per shared baseline.
- [x] `SetWorldTransform`, `ResetWorldTransform`, `Multiply`/`Translate`/`Scale`/`Rotate`
      with the post-multiply flag honoured on each, and `SetPageTransform` with its unit.
- [x] `Save`/`Restore` and `BeginContainer`/`BeginContainerNoParams`/`EndContainer`.
- [x] `ResetClip`, `SetClipRect`, `SetClipPath`, `SetClipRegion` and `OffsetClip`.
- [x] `DrawImage` and `DrawImagePoints`, including the parallelogram destination and a source
      rectangle, for both an encoded image and a GDI+ native bitmap.

### The rules, and what they were measured against

**A world unit is a device pixel until something says otherwise, and the chain is four
stages.** World transform, then the page transform's scale and unit, then the reference
device's `szlMillimeters` against `szlDevice`, then the frame origin — composed into one
matrix whenever any of them changes (`EmfPlusHelperData::mappingChanged`). The reference
device is the only thing in the whole file that says how big a pixel is, exactly as it is for
an EMF in `MM_TEXT`.

**A pen's width is in the space the world transform maps *from*, and it scales.** Unlike a
GDI cosmetic pen, which is one device pixel whatever the picture's scale. A width of zero
means the thinnest line the device draws, which has no device-independent value; 0.18 world
units and 0.05 pixels are LibreOffice's two substitutes and they are what is used here.

**An EMF+ hatch is opaque and a GDI hatch is not.** GDI+ states both colours on the brush, so
the background is painted and then the lines; GDI takes its background from the device
context and paints it only when the background mode is opaque. Twenty of the fifty-three GDI+
hatch styles are one of GDI's six at a different weight, and those are drawn as lines; the
twelve percentage styles are dot screens with no line form at all and are blended, which is
what LibreOffice draws for every style it does not recognise.

**Fidelity, against LibreOffice 24.2's rendering of all twenty-seven EMF+ files in
`emfio/qa/cppunit/emf/data`**, both PDFs rasterised with `pdftoppm -r 150`:

| Band | Files | Worst in band |
|---|---|---|
| `mae` ≤ 0.0002 | 10 | `TestEmfPlusSave`, `TestDrawLine`, `tdf143031_BrushPathGrad`, `TestEmfPlusDrawPathWithCustomCap` and `TestEmfPlusFillClosedCurve` all at **0.0000** |
| 0.0002 < `mae` ≤ 0.002 | 8 | `TestEmfPlusGetDC` 0.0018 |
| `mae` > 0.002 | 9 | `TestEmfPlusDrawLineWithDash` 0.0662; each of the nine is named in *Not done* below or in the LibreOffice table above |

**What the paint work and the caps moved, on the same twenty-seven files.** The bands barely
changed and that is the point: what moved was the *pictures*, and three of the six are files
whose `mae` was already small because the shape was right and the colours were not.

| File | Before | After | Why |
|---|---|---|---|
| `TestLinearGradient` | `mae 0.0523`, `differing 0.1702` | **`mae 0.0017`** | `SpreadMethod`. Every pixel was painted before and the pattern was wrong — one ramp then flat colour where the file asked for stripes |
| `TestEmfPlusDrawImagePointsWithMetafile` | `mae 0.0289`, `ink_ratio 0.000` | **`mae 0.0105`**, `ink_ratio 1.006` | the nested metafile is replayed; a 7 px vertical offset is what is left |
| `TestEmfPlusDrawPathWithCustomCap` | `mae 0.0001`, `ink_ratio 0.538` | **`mae 0.0000`**, `ink_ratio 1.035` | the arrow head is drawn |
| `TestEmfPlusBrushPathGradientWithBlendColors` | `mae 0.0193` | **`mae 0.0129`** | the mesh honours the preset colour curve per vertex |
| `TestEmfPlusDrawLineWithCaps` | `mae 0.0008`, `ink_ratio 0.932` | **`mae 0.0004`**, `ink_ratio 0.977` | the anchors are drawn |
| `TestEmfPlusBrushPathGradientMultiSurroundColor` | `mae 0.0017` | `mae 0.0113` | **deliberately worse** — see the LibreOffice table above |

`tdf143031_BrushPathGrad` stayed at **0.0000** through all of it, which is the measurement that
mattered most: it is the file the old bounding-ellipse ramp was exact on, so it is the one a
mesh could most easily have broken.

And on the committed fixture, which is the only file here that LibreOffice has never
written: `mae 0.0010`, `ink_ratio 1.011`. That measurement is worth more than its number —
it is what proves `emfplus-shapes.emf` is a *valid* EMF+ rather than only one this reader
can read.

### Traps, each of which cost time

**The relative point encoding is a delta chain, and LibreOffice does not accumulate it.**
`EmfPlusPointR` packs each coordinate into one or two bytes as an `EmfPlusInteger7` or
`EmfPlusInteger15`, and each is a delta from the previous point — the first from (0, 0).
`emfppath.cxx`'s `0x800` branch pushes the delta itself, so a path written that way draws in
LibreOffice as a small cluster near the origin. Nothing about such a file looks wrong until
the geometry does, and the encoding is chosen by producers precisely because it is compact,
so it appears in the files that are hardest to eyeball. The sign is the second half of the
trap: it lives in **bit 6** of the first byte in both widths, so a seven-bit value is
sign-extended from there and not from bit 7.

**A pen's thirteen optional fields are positional, and the brush sits after all of them.**
Each flag bit names a field present only when the bit is set, read in bit order, and a pen
ends with a whole brush. Skipping a set bit or reading an unset one moves the brush, and the
line comes out whatever the dash array's first float looks like as a colour — which is a
plausible colour, not an obviously wrong one. `APensOptionalFieldsArePositional` exists to
catch exactly that: it sets three of the thirteen and asserts the colour.

**The measurement harness cost more time than any record did.** LibreOffice converts a bare
metafile by opening it in Draw, on an A4 page with 10 mm margins, and **fitting it inside the
printable area preserving its aspect ratio**. Until the harness reproduced that placement, a
1300 × 800 mm EMF+ measured `mae 0.7540` with `ink_ratio 18.4` — which reads exactly like a
catastrophic mapping bug in the reader, and was not one:
`TestEmfPlusBrushPathGradientMultiSurroundColor` went to `mae 0.0017` with no change to any
reader code at all. Two other files improved by more than 0.1 on the same fix and five more
by more than 0.01. **A fidelity number is a statement about two renderings, and the harness
is half of it.**

**A path gradient's boundary need not be anywhere near the shape it fills, and the shape is
still painted.** `tdf143031_BrushPathGrad` states a boundary at 77–148 mm and fills a shape at
1–11 mm. GDI+ does not leave the uncovered part blank: outside the sweep the centre-to-edge
parameter clamps at 1 and the pure edge colour is used, which is LibreOffice's second
rasterisation pass. Without that undercoat the mesh landed off the shape and drew **nothing** —
`ink_ratio 0.000` on a file the old code measured `0.0000` on. Nothing about a blank shape says
"the boundary is elsewhere"; it reads as a mesh that failed to build.

**A custom cap's width goes through the mapping's *x* scale where the pen's width goes through
its *y* scale.** Mixing the axes looks like an oversight in `EMFPPlusDrawPolygon` and is not one
to depart from: on `TestEmfPlusDrawPathWithCustomCap`, whose world transform is anisotropic,
using the y scale for both draws an arrow head 17 px across where LibreOffice draws 28 —
`ink_ratio 0.673` against 1.035, with `mae` unchanged at 0.0001 either way, because the head is
a small part of a large page. **A cap is the one place a `mae` cannot see the defect**; the ink
ratio is what caught it.

**A record's flags word is three fields at once.** The low byte is an object slot, bit 15
says "this is a colour rather than a brush index", bit 14 says the points are compressed,
bit 13 means winding *or* post-multiply *or* close-the-figure depending on the record, and
bits 8-11 are a clip combine mode. There is no single mask that is right twice.

### Not done, and why

- [x] **A path gradient's colour per boundary vertex — closed by `MeshPaint`.** The boundary is
      partitioned into `(centre, V(i), V(i+1))` and each triangle carries the centre colour and
      two surround colours at its corners, which is what GDI+ draws and what
      `emfphelperdata.cxx`'s `BrushTypePathGradient` branch draws into a 256-pixel texture. Both
      backends state the triangles directly instead. `PL6040` now means only a boundary of fewer
      than three points, which has no interior to shade and no place for a colour per vertex —
      a malformed brush, and the one case that still falls back to the bounding-ellipse ramp.
- [x] **A gradient's spread mode — closed by `SpreadMethod`.** Four of GDI+'s five wrap modes
      collapse into two and that is exact rather than lossy: the flips name an axis and a ramp
      varies along one of them, so a flip in y is a copy indistinguishable from the original.
      `PL6041` and SVG's `PL6021` retired together.
- [x] **A metafile carried as an image.** It is replayed: `EmfPlusImage` keeps the nested bytes
      undecoded, `VectorImages.Decode` picks the decoder by sniffing them, and
      `MetafilePainter.DrawNestedPicture` puts the display list in the same placement square a
      bitmap goes in — so the parallelogram destination, the transform and the clip are shared
      entirely with the bitmap path. **The bound is `VectorLimits.MaxNestingDepth`, not the
      budget**, and that distinction is the whole of the problem: a budget is spent as work is
      done, and a picture nested a thousand deep that draws almost nothing at each level never
      spends any of it. `MetafileBudget.Nested` hands down one less level and answers null when
      there is none left; `PL6039` now means only that. Measured:
      `TestEmfPlusDrawImagePointsWithMetafile` went from `ink_ratio 0.000` to **1.006**, `mae`
      0.0289 to 0.0105 — a 7 px vertical offset in the nested picture's own placement is what
      remains.
- [x] **Custom, triangular and anchor line caps.** Seven of the ten are decorations rather than
      caps and none of them needed anything new: a decoration is a filled path, stated in a unit
      space, scaled to the pen's width and turned to face along the line. The outlines and their
      multipliers are `CreateLineEnd`'s and the docking rule is
      `createAreaGeometryForLineStartEnd`'s, so a diamond is LibreOffice's diamond. `PL6038` now
      means only the *adjustable arrow* form of a custom cap, which states a width, a height and
      a middle inset and no path at all — the same one LibreOffice reads and does not use.
- [ ] **Image attributes.** A colour matrix, a gamma, a chroma key and a colour remap table,
      all of which need the pixels of a JPEG or a PNG rather than of an uncompressed DIB — so
      this is the one remaining case that a codec here would buy, and the reason the
      *pixels* open question is answered "no" rather than "never". The object's slot is
      cleared so that a stale object of another kind cannot be drawn through it, and the
      image is drawn unadjusted.
- [ ] **`EmfPlusStrokeFillPath`.** It names a path and nothing else: the pen and brush are
      whatever a preceding record left current, which EMF+ has no state for and this reader
      does not track. `PL6037`, and nothing is drawn — better than drawing it with an
      arbitrary slot.
- [ ] **Region symmetric difference and complement, and what closing them would take.**
      Measured rather than guessed, because the answer decides whether it is worth starting.
      Intersection and rectangular exclusion are exact in the clip's rectangles-and-shapes form,
      and union of two rectangle sets is too — overlapping rectangles are still their own union
      under the non-zero rule. What is left is **XOR and complement with an arbitrary path as an
      operand**, and both need the same one thing: a general polygon boolean over Bézier
      subpaths. There is no way to fake it, because the result's *edges* are new curves that
      neither operand states.
      **The size of it, from `emfpregion.cxx`'s own dependencies:** a Bézier-aware
      Greiner–Hormann or Vatti implementation is roughly what `basegfx`'s
      `basegfx/source/polygon/b2dpolypolygoncutter.cxx` is — about 1200 lines — plus
      adaptive flattening and a crossover-point solver it leans on. That is a *third* of this
      whole library and it buys two operations that appear in none of the twenty-seven reference
      files. **So: do not start it for the metafiles.** The case that would justify it is SVG's,
      where the same arithmetic closes `PL6012` (difference clips) and `PL6023` (a clip on one
      member of a union) as well — three diagnostics for one body of work, and the decision
      should be made there, on real documents, rather than here.
- [ ] **`TestEmfPlusDrawLineWithDash` is at `mae 0.0662`, and it is not the dash phase.** That is
      what it was recorded as and the attribution was wrong; see the LibreOffice table above.
      The dash lengths are right, the offset is applied where LibreOffice ignores it — worth
      `0.0011` — and the remaining `0.062` is LibreOffice applying every `RotateWorldTransform`
      last whatever the record's flag says. Ours follows the flag, as it does for `Translate`,
      `Scale` and `Multiply` and as LibreOffice itself does for those three. Nothing to do here
      unless a real document turns up that needs LibreOffice's answer.
- [ ] **Arabic text draws as its Latin characters only.** `TestAlignRtlReading` measures
      `ink_ratio 0.166`. This is font resolution rather than EMF+ — the same string through a
      GDI `ExtTextOutW` would do the same — but it is where it was found, so it is recorded
      here.

---

## SVG — done

`Svg.SceneGraph` + `Svg.Model` + `ShimSkiaSharp`, without `Svg.Skia` itself, so no SkiaSharp
is pulled in and the output is a device-independent canvas-command list. Hand-rolling would
have meant reimplementing a CSS cascade, the path grammar, gradients, masks and filters for
no fidelity gain, since the command list is what we would have produced anyway.

### What draws

- [x] Open through `SvgService`, build the scene with `SvgSceneRuntime.CreateModel`, record
      it to a `ShimSkiaSharp` picture.
- [x] Translate the command list to `IDrawingSink` (`Svg/SvgPictureTranslator.cs`).
- [x] Geometry and paints: `SKPath` → `GraphicsPath`, including the shim's whole-primitive
      commands (rect, round-rect, oval, circle, poly) and SVG elliptical arcs;
      `LinearGradientShader`/`RadialGradientShader` → `GradientPaint`; `SKPaint` stroke state
      → `Stroke` with caps, joins, miter, dashes and phase.
- [x] Clipping, including nested `ClipPath` and per-clip transforms.
- [x] Transparency groups from `SaveLayer`, kept as groups rather than folded into per-shape
      alpha — a group at 40 % is not the same picture as its members at 40 % where they
      overlap.
- [x] Text, shaped by `Paperless.Text` (`Svg/SvgTextEngine.cs`): `text-anchor`, `<tspan>`
      positions, letter-spacing and `<textPath>`.
- [x] Embedded raster images, handed on **undecoded** as `RasterImage.Encoded`, with only the
      pixel dimensions read from the header (`Svg/EncodedImageSize.cs`) — so nothing here
      needs a codec, which is the same reason `RasterImage.Encoded` exists.
- [x] SVGZ, by sniffing the gzip magic rather than trusting the media type.

### The rules, and what they were measured against

**A user unit is 1/96 inch, exactly 9525 EMU.** LibreOffice fixes the same value:
`o3tl::Length::px` is "15 twip (96 ppi)" (`include/o3tl/unit_conversion.hxx:44`), `svgio`
converts every other unit through it (`svgio/source/svgreader/SvgNumber.cxx:41-54`), and the
finished picture is scaled px → 1/100 mm at `svgio/source/svgreader/svgsvgnode.cxx:677`.
914400/96 divides exactly, so no rounding enters.

**The viewport rule is LibreOffice's, not the library's, and they disagree.** For
`<svg width="100" viewBox="0 0 200 50">` LibreOffice derives the missing height from the view
box's ratio — `fH = fW / fViewBoxRatio`, `svgsvgnode.cxx:504-516` — giving a 100 × 25
viewport. `Svg.SceneGraph` 5.1.1 keeps the view box's own height and gives 100 × 50, which
makes `xMidYMid meet` letterbox the drawing into half the frame. Measured on LibreOffice
24.2: a `width="200" viewBox="0 0 400 100"` SVG converted to PDF and rasterised at 300 dpi
had a content area of **624 × 156 px, aspect 4.000**, not 2.000. `Svg/SvgViewport.cs`
computes LibreOffice's viewport and writes it back onto the document, so the library's rule
never runs.

**The document's frame extent wins, and the mapping is a plain stretch.** Measured: an ODT
frame of `svg:width="12cm" svg:height="4cm"` holding a 120 × 80 mm SVG rendered the picture's
own background filling **119.2 × 39.3 mm** — squashed to the frame, not letterboxed inside
it. `VectorImage.Draw` does exactly that, and clips to the frame because SVG clips to its
outermost viewport (`svgsvgnode.cxx:634`).

**Fidelity, by rasterising our PDF and LibreOffice's with the same rasteriser so
antialiasing cancels** (`pdftoppm -r 150`, then `render-comparison`'s `compare-images.py`):

| Picture | `mean_abs_error` | Note |
|---|---|---|
| shapes, strokes, dashes, arcs, a clip | **0.0005** | antialiasing level, `shifted=0` |
| two gradients | 0.0069 | LibreOffice bands its gradients and we do not |
| three text runs | 0.0128 | see the anchoring note |
| all of the above at once | 0.0100 | |

**Anchored text lands within about 0.6 pt of LibreOffice's.** Start-anchored text matches to
0.003 pt (`pdftotext -bbox`: 28.344 against 28.347). Centre- and end-anchored text differs by
0.5–0.6 pt because the two stacks measure the string differently: for `text-anchor="end"` at
a nominal 396.85 pt, we place the right edge at 396.86 and LibreOffice at 396.22. Ours sits
on the nominal anchor; LibreOffice's measurement is about 0.45 % wider. Whether that is a
trailing side bearing or a different advance is not yet run down.

### Enforced at the boundary

- [x] **No external references.** `SvgProcessingMode.SecureStatic` with
      `SvgExternalResourcePolicy.SameDocumentAndDataOnly`, *and* `Svg/SvgAssetLoader.cs`
      refusing any URI that is not `data:` on its own account, so a change of the library's
      default cannot quietly reopen the hole.
- [x] **No scripting.** `<script>`, event attributes and `javascript:` URLs are inert, with a
      test asserting it rather than trusting the default.
- [x] **Bound the work.** `VectorLimits`: bytes, commands, path segments, nesting depth,
      expanded nodes and wall-clock time. See the fan-out trap below.
- [x] **Render the initial state of any declarative animation**, which is what `SecureStatic`
      produces.

### Traps, each of which cost time

**The `file://` default was a live local-file read.** With the library's stock settings, an
`<image href="file:///etc/passwd">` caused `LoadImage` to be called with **1 238 bytes of
that file's contents**. A picture in a spreadsheet e-mailed to a victim would have read a
local file and had it drawn onto the page. Two independent gates now stand in front of it and
`SvgSafetyTests` asserts both, because this is exactly the kind of thing a dependency upgrade
restores silently.

**A 1 057-byte file hung the library for over two minutes.** Thirteen nested `<g>` elements,
each holding three `<use>` references to the one below: 3¹³ ≈ 1.6 million instantiated nodes.
No byte cap sees it — it is a kilobyte. No command cap fires — the explosion happens inside
the library's scene compiler before a single command exists. No time cap helps — the loop
belongs to a third party and .NET cannot interrupt it. The only place to stand is in front,
counting the expansion arithmetically: `Svg/SvgExpansionGuard.cs` parses the XML once and
walks it with memoisation and saturating arithmetic, refusing before the document is handed
over. **Any new format with a macro-style reference needs the same treatment.**

**Rounding the viewport changed the aspect ratio and silently shrank the picture.** The
library rounds its computed viewport to whole pixels, and rounds each dimension
independently: 120 × 80 mm is 453.543 × 302.362 px, which rounds to **454 × 302** — aspect
1.5033 where the view box's is 1.5000. `xMidYMid meet` then pillarboxed the drawing by half a
pixel each side and it came out **119.74 mm wide in a 120 mm frame**. Nothing about that
looks like rounding; it looks like a scaling bug, and the source SVG is exact. The fix is in
`SvgViewport.Impose`: when there is a view box, the imposed viewport is expressed in the view
box's own units, so it rounds to itself whenever the two ratios agree.

**Both rendering backends dropped every undecoded image.** `DrawImage` guarded on
`image.Width <= 0`, which was right until `RasterImage.Encoded` arrived — an undecoded image
has no size until a codec has looked at it, and the guard ran *before* the decode. Fixed in
`Paperless.Rendering`, and found only because an SVG holding a `data:` PNG was the first
thing in the tree to emit an encoded image into a sink.

### Not done, and why

- [ ] **Filter effects.** `feGaussianBlur`, `feColorMatrix`, `feDropShadow` and the rest are
      parsed by the library and arrive as an `SKImageFilter` on the paint, which
      `IDrawingSink` cannot express at all. Today the unfiltered shape is drawn and `PL6016`
      is raised. Doing better needs either a filter primitive in Core — which every backend
      would then have to implement, and PDF has no equivalent for most of them — or an
      offscreen raster pass, which reintroduces the codec dependency this library was chosen
      to avoid. **Assess against real documents before choosing**: office clip art uses
      filters far less than web SVG does, and a missing drop shadow looks better than a
      missing picture.
- [ ] **Pattern fills** (`<pattern>`, `PictureShader` in the shim). `BitmapPaint` exists but
      wants a `RasterImage`, and producing one means rasterising the tile. The honest
      alternative is a paint that carries a `DisplayList` tile; that is a Core change and was
      to wait until EMF+ texture brushes wanted the same thing. **They did not**: an EMF+
      texture brush carries a raster image, so it goes through as an ordinary `BitmapPaint`
      tile and nothing new was needed. So this remains SVG's alone. `PL6019`, and the shape is
      left unfilled.
- [ ] **Text as a clip path or mask.** Needs glyph outlines, which live in
      `Paperless.Rendering`. `PL6005`, and the clip is empty — deliberately the safer wrong
      answer, since an unclipped shape paints over its neighbours.
- [x] **Focal-point radial gradients** (`fx`/`fy`), closed when EMF+ needed a spread. A focus is
      the inner circle of the two-circle form PDF's `/ShadingType 3` and Skia's two-point conical
      shader both already took, so it cost a nullable `DocPoint` on `GradientPaint` and no second
      code path in either backend. **The mesh was the recorded answer and turned out to be the
      wrong one**: a mesh approximates a focal radial with piecewise-linear colour and the
      two-circle form states it exactly, so the mesh took the path gradient and the focus took
      this. `PL6018` retired.
- [x] **`spreadMethod` other than `pad`.** `SpreadMethod` on `GradientPaint`, which Skia takes as
      a tile mode and PDF as a lengthened shading axis. `PL6021` retired.
- [ ] **Non-scaling strokes** (`vector-effect`). Scaled with the shape, `PL6022`.
- [ ] **Blend modes other than normal.** Composited normally, `PL6017`.
- [ ] **Difference clips.** `IDrawingSink.ClipPath` only intersects. `PL6012`.
- [ ] **A clip on one member of a clip-path union.** The union is expressed as one path,
      so a clip belonging to a single member cannot be scoped to it — intersecting it
      would clip the other members away. Honoured when there is only one member, `PL6023`
      otherwise. Real path operations would fix this and `PL6012` together.
- [ ] **The library's viewport rounding**, in the one case `SvgViewport.Impose` cannot fix:
      both dimensions absolute with a ratio that disagrees with the view box's, where
      letterboxing is *intended*. The letterbox then sits up to 0.04 % out.
- [ ] **SVG fonts** (`<font>`, `<glyph>`). Switched off, because they would bypass the
      resolver and shaper everything else goes through. Rare outside decorative web art.

## Open questions

- [x] **Lazy decoding?** Answered by the seam rather than by a mechanism: a reader emits the
      encoded bytes and nothing decodes until something draws. A slide master's unused
      placeholder art costs a relationship lookup.
- [x] **Cache decoded output as a display list?** That is what `VectorImage` is. It is an
      immutable record and `Content.Replay` is non-destructive, so one decode serves every
      slide the picture appears on. Nothing caches them *for* a caller yet — the natural home
      is beside the package, keyed by part name, and that belongs to whoever wires the
      readers up.
- [x] **Do `ShimSkiaSharp`'s text commands give us glyph ids?** No. Measured: even with
      `ISvgTextGlyphRunResolver` implemented, the library emits `DrawTextCanvasCommand`
      carrying a *string*, a family name and a size; that resolver is consulted only for
      `<textPath>` measurement. So SVG text is re-shaped on our side, which is what
      `SvgTextEngine` does. The consequence worth knowing: `ISvgAssetLoader.MeasureText` is
      load-bearing, because the library resolves `text-anchor` from it *before* emitting the
      draw — a `text-anchor="middle"` run at `x="150"` came out at `x=120` for a 60-unit
      measurement.
- [x] **EMF+ path gradients have no direct `GradientPaint` equivalent.** Answered twice. First by
      building the nearest ramp and measuring it: from the centre colour to the first surround
      colour over the boundary's bounding ellipse is *exact* whenever the surround colours agree,
      which is the case that reads as a radial gradient in the first place. Then by building the
      real thing — Core's `MeshPaint`, a triangle per boundary segment with a colour at each
      corner, which is what GDI+ draws and what a PDF type 4 shading and Skia's `SkVertices` both
      state natively. `tdf143031_BrushPathGrad` measures `0.0000` under both, which is what makes
      the second safe to prefer.
      **What the mesh did *not* close, against the expectation recorded here:** SVG's focal
      radial. A mesh would approximate it and the two-circle form both backends already had
      states it exactly, so `PL6018` retired through a `Focus` on `GradientPaint` instead. The
      lesson is worth the line: *"both backends could express it"* is necessary and not
      sufficient — ask which of the forms they have is the **exact** one.
- [ ] A picture drawn at several sizes re-uses one `DisplayList`, which is right for geometry
      and indifferent to hinting. Nothing depends on it yet; it will matter when a thumbnail
      and a print rendering share a cache.
- [x] **Should `Paperless.Vector` be allowed to see decoded pixels?** Settled by EMF, and
      EMF+ needed nothing further. The distinction that resolves it: an **uncompressed** DIB
      or a GDI+ native bitmap is not an encoded format at all — it is a stride, a channel
      order and a palette, so turning one into straight RGBA is arithmetic rather than a
      decode, and no codec dependency follows. That is enough for the WMF transparent-bitmap
      idiom, for `AlphaBlend`'s `AC_SRC_ALPHA`, for `TransparentBlt` and for EMF+'s native
      bitmaps. The compressed forms answer null and fall back to drawing the source
      undecoded. What is *still* out of reach is anything needing the pixels of a **JPEG or
      PNG** — EMF+ image attributes are the only such case left — and that would need a real
      codec here or a sink extension, so the answer stands.
- [x] **Which representation of a dual-format file to replay.** Settled twice with the same
      answer, and the answer is `emfio`'s rather than one reasoned from first principles: a
      WMF replays the complete EMF its escape records carry, and an EMF stops replaying its
      GDI records the moment an EMF+ record appears — whether or not the header calls it
      dual, with `EmfPlusGetDC` as the one bracket that hands control back. Both decisions are
      made once, in front of the record loop. See *EMF+ → The rule that had to be settled
      first*.

## Wiring it into the readers — done

**Every family draws an embedded vector picture.** The wiring turned out to be smaller than the
list below predicted and to need one thing the list did not mention. What follows is what it
actually was, kept because the next format added here will follow the same three lines.

### What each family gained

Three properties and one call, and no new abstraction anywhere:

| Family | Where the picture lands | Where the bytes are sniffed |
|---|---|---|
| `Paperless.WordProcessing` | `PageFrame.Vector`, a `Lazy<VectorImage>?` beside `PageFrame.Image` | `EmbeddedPicture.Read`, which now answers a `FramePicture` — the pair — so all four front ends thread one value instead of two nullable fields |
| `Paperless.Presentations` | `PlacedPicture.Vector`, beside a now-nullable `Image` | `PptxSlideLayout.Loaded` and `OdpFills.Drawable` |
| `Paperless.Spreadsheets` | `SheetDrawing.Vector` | `XlsxDrawings.Load` and `OdsDrawings.Load` |

`PageDrawing.DrawFrame`, `SlideDrawing.DrawPicture` and `SheetPageGraphics.Draw` each gained one
branch: the vector if it decodes to something, else the raster. That is the whole of the drawing
side.

**The two-line sniff is written out five times and that is deliberate.** `VectorImages.For(bytes)`
then `new Lazy<VectorImage>(() => VectorImages.Decode(bytes))` appears in each of the five readers,
because there is nowhere above `Paperless.Vector` that all three families can see: `Paperless.Core`
cannot name `VectorImage`, and `Paperless.Ooxml`, `Paperless.OpenDocument` and `Paperless.MsBinary`
are one per file family rather than one per document family. A helper in any of them would have an
ODF reader depending on the OOXML library. Two lines duplicated beats that; a *third* thing to
duplicate would not, and the place to put it then is a new Core-only library beside
`Paperless.Markup`, which exists for exactly this shape of problem.

### The Core abstraction was still not needed, and this is the confirmation

The section below predicted it and the wiring bore it out. `VectorImage` *is* the abstraction a
frame wants — `Draw(IDrawingSink, DocRect)` plus `IntrinsicSize`, immutable, decoded once — and the
layering already permits every family to name it, because all three already carried a
`ProjectReference` on `Paperless.Vector` for `Paperless.Ooxml`'s sake. **Not one line of
`Paperless.Core` changed**, and the interface that was not written would have had two members and
one implementation.

### What the prediction missed: it has to be *lazy*, and the number is why

The seam said "decoded once". It did not say *when*, and when turns out to be the load-bearing
half. Measured on this tree with a throwaway console over `Paperless.Vector`:

| Picture | First decode in the process | Warm |
|---|---|---|
| `wmf-shapes.wmf` (one text run) | **1043.9 ms** | 0.21 ms |
| `emfplus-shapes.emf` | 381.3 ms | 0.13 ms |
| `emf-shapes.emf` (no text) | 67.0 ms | 0.08 ms |

Nearly all of the first is `Paperless.Text` resolving and loading faces. **DOCX, ODT, PPTX, XLSX
and ODS read their pictures only when a layout source asks for one — but RTF and DOC read theirs
while parsing the document**, which is the extraction path, so decoding there would have put a
second of font work on a caller that wanted the words and nothing else. A `Lazy<VectorImage>` is
the whole fix: it defers, it caches its answer, and the per-part caches the readers already keep
mean a logo on forty slides still decodes once. Three tests assert `IsValueCreated` is false after
a full layout, one per family, because nothing else can see the difference.

### What each format taught the wiring

- **The part name and the declared type are both worthless, in the sharpest possible form.**
  LibreOffice's own OOXML export writes a genuine EMF into `word/media/image1.wmf`,
  `ppt/media/image2.wmf` and `xl/media/image2.wmf`, and `[Content_Types].xml` declares nothing
  usable for `.wmf` at all. `VectorImages.For` is the only thing consulted.
- **`BlipReference.Choose` is now called by all three OOXML readers** rather than `r:embed` being
  read directly, and the `asvg:svgBlip` extension turns up in every one of the three — including
  the spreadsheet, which was not obvious. The raster is kept *beside* the vector rather than
  discarded, so an empty decode falls back to what the file put there for that; it is the only
  case where a frame holds both.
- **ODF needed no selection step, as predicted.** A `draw:frame` lists alternatives as sibling
  `draw:image` children and the first drawable one wins, which is already what the readers did.
- **A DOC's metafile blip is deflate-compressed and a raster blip is not.** This was the one real
  piece of work. `Ww8Blips` used to drop a vector blip's bytes; it now reads the 34-byte
  `OfficeArtMetafileHeader` and inflates what follows, where a raster blip has a single tag byte in
  the same place. `SvxMSDffManager::GetBLIPDirect` sets its ZCodec for the EMF, WMF and PICT cases
  and no other (`msdffimp.cxx:6518-6549`). Measured: 892 bytes of EMF arrive as 262 bytes of
  deflate, so a reader that skipped the header without inflating finds no placeable magic and no
  `METAHEADER` and declines the picture as an unrecognised blob — which reads as a corrupt document
  and is not one.
- **LibreOffice's RTF export rasterises every picture** — `\pngblip` for both metafiles where its
  DOC export keeps them byte for byte — so `vector-picture-text.rtf` is hand-written with
  `\wmetafile8` and `\emfblip`. Both control words are then ignored anyway; the sniff decides.
- **DOC has no blip type for SVG**, so LibreOffice's own export rasterises it while keeping the WMF
  and EMF. A vector picture survives a round trip through three of the four word-processing formats
  and not through that one.

### What it measures

Six sweep rows moved and five are new. Pages/words against `soffice --convert-to pdf`, with
`mean_abs_error` from `pdftoppm -r 150` through `render-comparison`'s `compare-images.py`:

| File | Words before | Words after | `mae` | `ink_ratio` |
|---|---|---|---|---|
| `svg-picture.odt` | 7/8 | **8/8** | 0.0007 | 1.028 |
| `svg-picture.docx` | 7/8 | **8/8** | 0.0009 | 1.025 |
| `wmf-picture.odt` | 6/8 | **8/8** | 0.0005 | 1.003 |
| `wmf-picture.docx` | 6/8 | **8/8** | 0.0007 | 1.001 |
| `emf-picture.odt` | 0/2 | **2/2** | 0.0037 | 1.208 |
| `emf-picture.docx` | 0/2 | **2/2** | 0.0037 | 1.208 |
| `vector-picture-deck.odp` | new | 7/7 | 0.0085 | 1.043 |
| `vector-picture-deck.pptx` | new | 7/7 | 0.0086 | 1.043 |
| `vector-picture-sheet.ods` | new | 10/10 | 0.0059 | 1.051 |
| `vector-picture-sheet.xlsx` | new | 10/10 | 0.0104 | 1.055 |
| `vector-picture-text.rtf` | new | 9/9 | 0.0044 | 1.080 |

**The `ink_ratio` above 1 is the same three EMF differences everywhere, and none of them is a
defect here.** `emf-shapes.emf` states an `EMR_GRADIENTFILL` bar, a dashed polyline and an outlined
ellipse; LibreOffice draws no bar, a thin solid line and no outline. Checked the way the section
above says to check such a thing — by converting the bare `.emf` with `soffice` and looking, rather
than by believing the number — and LibreOffice's own EMF import drops all three. The WMF rows at
`1.001` and `1.003` are the ones a regression would show up in first.

### What is left

- **The DOC as-character metafile picture comes back floating.** `vector-picture-text.doc` decodes
  and draws all three of its pictures and lays out none of them: they overlap the paragraphs around
  them, because none reaches `FrameAnchor.AsCharacter` and a floating frame with `Wrap.Through`
  moves no text. It is not about the pictures — the same document with rasters in the same three
  places is exact, and so is a single as-character metafile. The `Data` stream of the metafile
  version holds one valid `PICF` where the raster version holds one per picture. The sweep row
  still reads OK on pages and words, which is why it is written down. Belongs to
  `Paperless.WordProcessing`'s DOC reader.
- **XLS and PPT still read no pictures at all**, vector or raster, because nothing extracts bytes
  out of an Escher `F007` entry outside `Paperless.WordProcessing/Ww8/Ww8Blips.cs`. That file is now
  the only complete blip-store reader in the tree, metafile inflation included, and moving it into
  `Paperless.MsBinary/Escher/` is what buys both.
- **A rotated vector picture** turns with its frame no better than a raster does: all three
  families place a picture by a rectangle. The vector path could in principle do better —
  `VectorImage.Draw` already emits a `Transform` — but making it differ from the raster path would
  be worse than the shared limitation.

### The frame seam, and why it needed nothing new

**A `PageFrame.Image` is a `RasterImage`, and a decoded vector picture is a display list**, so a
word-processing frame holding an EMF, a WMF or an SVG had nowhere to put what it decoded and drew
nothing (`PL2370`, raised in `Paperless.WordProcessing/EmbeddedPicture.cs`). That was the last
missing piece of the wiring above and it was worth asking whether it needed an abstraction in
`Paperless.Core` — a picture interface a frame could hold without knowing which library produced it.

**It did not, and the reason is worth stating rather than the interface being written.**
`VectorImage` already *is* that abstraction: `Draw(IDrawingSink, DocRect)` plus an
`IntrinsicSize`, immutable, replayable, decoded once. A Core interface would have had exactly those
two members and one implementation, and the layering already permitted the direct reference —
arrows point at dependencies and `Paperless.Vector` sits beside `Paperless.Text` under Core, above
nothing that reads a document. **An interface with one implementation on the far side of a
dependency that is already legal buys nothing and costs a name.** Built, and `Paperless.Core` did
not change.

**The seam was proven rather than asserted**, because the library had a caller of its own that used
exactly it: an EMF+ image object carrying a whole further metafile is decoded with
`VectorImages.Decode` and drawn into a destination rectangle with
`MetafilePainter.DrawNestedPicture` — one decode, one display list, one `Draw` into a rectangle a
transform maps. `EmfPlusNestedMetafileTests` pins it, including the part a reader gets wrong first:
**it is the nested picture's whole *frame* that is stretched onto the destination, not its ink.**
A 10 mm square inside an 80 mm frame drawn into a 20 mm destination lands at 2.5 mm. Taking the ink
instead makes the picture four times too large and clipped, which looks like a mapping bug in the
decoder and is not one.

**That trap is now pinned from the reader side too**, in three tests that measure it on a real
document rather than on a fixture built to show it:
`VectorFrameTests.TheFramesWholePictureIsStretchedOntoItRatherThanItsInk` asserts the ink starts
*inside* the frame and is narrower than it for all seven word-processing fixtures;
`SlideVectorPictureTests` measures the SVG's rounded rectangle, which sits at x = 2 in a 200-unit
view box and must land 0.8 mm into an 80 mm destination rather than at its edge; and
`SheetVectorPictureTests` draws the WMF at its view-box size and again at half of it and asserts the
ink halves exactly. All three need a sink that *composes* transforms rather than counting them,
which is why `Paperless.TestKit.PlacedDrawingSink` now exists beside `RecordingDrawingSink` — the
layout one counts, because layout emits no transform that matters, and a picture is nothing but
transforms.
