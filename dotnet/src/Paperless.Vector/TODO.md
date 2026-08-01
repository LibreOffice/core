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

**`Paperless.Core` never had to change, across all three.** Not the sink, not `Paint`, not
`GraphicsPath`, not `GlyphRun`. Six things looked as though they would and did not: a hatch
became stroked lines clipped to the shape; a DIB became `RasterImage.Encoded` by prepending
fourteen bytes of BMP header; a source rectangle became a scale plus a clip; a constant
source alpha became the sink's own opacity; a transparent-bitmap idiom became straight RGBA
arithmetic over an uncompressed DIB, which is not a decode; and an EMF+ parallelogram
destination became a `Transform` plus a fixed-size placement square. The two that genuinely
exceed the drawing model are named in *Not done* below, and both are **gradients**: a GDI+
path gradient wants a colour per boundary vertex where `GradientPaint` has one ramp, and
every GDI+ gradient wants a spread mode where `GradientPaint` has none. Neither is a metafile
problem — the SVG side records the same two gaps as `PL6018` and `PL6021` — which is the
argument for fixing them in Core rather than per format.

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

So read the diff image before believing a number. A high `ink_ratio` with a low
`mean_abs_error` — all three above — means the two renderers disagree about a *small* area,
and it is as likely to be theirs as ours.

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
every test with a circular arc passes.

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
- [ ] **The transparent-bitmap idiom.** A monochrome mask blitted with `SRCAND` followed by a
      colour image with `SRCPAINT` is how a WMF says "transparent", and merging the two into
      one alpha bitmap is what `MtfTools::ResolveBitmapActions` (`mtftools.cxx:2557`) exists
      for. It needs pixels, and pixels need a codec. Today both are drawn in order, so the
      transparent area comes out black. **This is the strongest single argument for letting
      `Paperless.Vector` see decoded pixels**, and it should be weighed against the
      dependency the whole library was arranged to avoid.
- [ ] **`ExcludeClipRect`.** Subtraction, which `ClipPath` cannot express. The excluded area
      is left unclipped — drawing too much rather than too little, which loses no content —
      and `PL6034` says so. Real path operations would fix this and the SVG side's `PL6012`
      together.
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
      preset-colour ramps on both gradients.
- [x] Pens, including all thirteen optional fields of the pen-data blob, the five predefined
      dash styles and a custom dash array.
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
| `mae` ≤ 0.0002 | 10 | `TestEmfPlusSave`, `TestDrawLine`, `tdf143031_BrushPathGrad` and `TestEmfPlusFillClosedCurve` all at **0.0000** |
| 0.0002 < `mae` ≤ 0.002 | 8 | `TestEmfPlusGetDC` 0.0018 |
| `mae` > 0.002 | 9 | `TestEmfPlusDrawLineWithDash` 0.0662; each of the nine is named in *Not done* below or in the LibreOffice table above |

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

**A record's flags word is three fields at once.** The low byte is an object slot, bit 15
says "this is a colour rather than a brush index", bit 14 says the points are compressed,
bit 13 means winding *or* post-multiply *or* close-the-figure depending on the record, and
bits 8-11 are a clip combine mode. There is no single mask that is right twice.

### Not done, and why

- [ ] **A path gradient's colour per boundary vertex.** GDI+ runs a path gradient from one
      centre colour out to a colour *per vertex of the boundary*, Gouraud-shaded between
      them — a star with three surround colours has three coloured points and no radial
      gradient anywhere in it. `GradientPaint` has one ramp and one centre, so what is drawn
      is the ramp from the centre colour to the first surround colour over the boundary's own
      bounding ellipse, with `PL6040` when the surround colours differ. That is exact for the
      common case, and `tdf143031_BrushPathGrad` measures **0.0000** because of it.
      LibreOffice's answer is to triangulate the boundary and Gouraud-shade each triangle into
      a 256-pixel bitmap used as a texture; doing that here means rasterising in a library
      arranged not to. **What it would take in Core:** a `Paint` kind carrying a triangle mesh
      with per-vertex colours, which PDF has natively (a type 4 or 5 shading) and Skia has as
      `SkVertices` — so both backends could state it, and it would also close SVG's `PL6018`
      focal radial if the mesh form allowed a focus.
- [ ] **A gradient's spread mode.** Every GDI+ gradient has a wrap mode — tile, tile-flip-X,
      tile-flip-Y, tile-flip-XY or clamp — and `GradientPaint` has none. A brush whose
      rectangle is small against the shape it fills therefore comes out as one ramp and then
      flat colour where the file asked for stripes; measured on `TestLinearGradient`,
      `differing 0.1702` at `ink_ratio 1.000` — every pixel is painted and the pattern is
      wrong. `PL6041`, and **the same gap the SVG side records as `PL6021`**, which is the
      argument for a `SpreadMethod` on `GradientPaint` rather than a per-format workaround.
- [ ] **A metafile carried as an image.** An `EmfPlusImage` may hold a whole WMF or EMF, and
      `DrawImagePoints` then places it under a transform. It is recognised and reported
      (`PL6039`) rather than replayed, because replaying it means re-entering the decoder from
      inside itself and the recursion has to be bounded by something other than the budget,
      which is already shared. Measured: `TestEmfPlusDrawImagePointsWithMetafile` draws
      nothing where LibreOffice draws the nested picture, `ink_ratio 0.000`.
- [ ] **Custom and anchor line caps.** Six of the eleven GDI+ cap types are arrow heads,
      diamonds and anchors — line *decorations*, which the drawing model has no place for at
      all — and a custom cap is a whole path with its own scale. Reported as `PL6038` and the
      line is drawn without them. Measured: `TestEmfPlusDrawPathWithCustomCap` at `mae 0.0001`
      but `ink_ratio 0.538`, which is what a missing arrow head costs on a picture that is
      mostly arrow head.
- [ ] **Image attributes.** A colour matrix, a gamma, a chroma key and a colour remap table,
      all of which need the pixels. The slot is consumed so that later objects land in the
      right ones; the image is drawn unadjusted.
- [ ] **`EmfPlusStrokeFillPath`.** It names a path and nothing else: the pen and brush are
      whatever a preceding record left current, which EMF+ has no state for and this reader
      does not track. `PL6037`, and nothing is drawn — better than drawing it with an
      arbitrary slot.
- [ ] **Region union, symmetric difference and complement.** A region is a binary tree of set
      operations; intersection and rectangular exclusion stay exact in the clip's
      rectangles-and-shapes form, and union of two rectangle sets does too, because
      overlapping rectangles are still their own union under the non-zero rule. The other
      three need real polygon arithmetic, which `emfpregion.cxx` has and this does not. Marked
      approximate rather than approximated silently.
- [ ] **The dash *phase* differs from LibreOffice's.** Measured on
      `TestEmfPlusDrawLineWithDash`: `mae 0.0662` on a picture that is nothing but dashes,
      with `ink_ratio 1.011` — the same amount of ink in slightly different places. The dash
      lengths are right (they are the pen width times the file's own array); the offset is
      applied and LibreOffice ignores it. Not run down.
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
- [ ] **Focal-point radial gradients** (`fx`/`fy`). `GradientPaint` has no focus; the
      gradient is centred, `PL6018`.
- [ ] **`spreadMethod` other than `pad`.** Clamped, `PL6021`.
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
- [x] **EMF+ path gradients have no direct `GradientPaint` equivalent.** Answered by
      building it and measuring: the ramp from the centre colour to the first surround colour
      over the boundary's bounding ellipse is *exact* whenever the surround colours agree,
      which is the case that reads as a radial gradient in the first place —
      `tdf143031_BrushPathGrad` measures `mae 0.0000`. It is wrong at the edges when they do
      not, and says so (`PL6040`). What the general case needs is not a decomposition into
      bands but a **triangle mesh with per-vertex colours**, which is what GDI+ actually
      draws and what both backends could state natively: a PDF type 4 or 5 shading, and
      Skia's `SkVertices`. That is a `Paint` kind in Core, and it would close SVG's `PL6018`
      at the same time. See *EMF+ → Not done*.
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

## Wiring it into the readers

`Paperless.Ooxml.DrawingML.BlipReference.Choose` picks the SVG over the raster fallback for a
DrawingML `a:blip`, and `VectorImages` decodes whatever bytes a reader fetches. **Neither is
called from a reader yet**, because the three picture-drawing paths were being written in
parallel with this and a shared edit would have collided. What is left is one hook per
family, each a few lines:

1. Where a picture's relationship id is resolved, call `BlipReference.Choose(blip)` rather
   than reading `r:embed` directly. `Paperless.Presentations.Ooxml.PptxShapeReader.ReadPicture`
   and `Paperless.WordProcessing.Ooxml.DocxContentReader` both read `r:embed` today.
2. Where the bytes become something drawable, try `VectorImages.For(bytes)` first and fall
   back to `RasterImage.Encoded` — and if the vector decode comes back empty, fall back to
   `Choice.FallbackRelationshipId`, which is exactly what it is for.
3. ODF needs no selection step: a `draw:image` whose target is `image/svg+xml` *is* the
   vector, so only step 2 applies. `VectorImages.IsVectorMediaType` answers the media-type
   half for a reader that has the declared type before it has the bytes.

`tests/corpus/features/svg-picture.odt` and `.docx` are the fixtures for both, and
`CorpusSvgTests` already reads them the way a reader would.

**WMF changes nothing about the shape of that hook, which is the point of the seam** — but it
does raise the stakes, because a WMF has no `BlipReference` alternative to choose between and
no SVG-style declared media type to sniff for. It is simply a picture, and the only thing
that says so is its first four bytes. So step 2 is the whole of it: `VectorImages.For(bytes)`
before `RasterImage.Encoded`, for every picture, in every family.
`tests/corpus/features/wmf-picture.odt` and `.docx` are the fixtures — both packages keep the
426-byte metafile rather than rasterising it, the ODT alongside a 9 685-byte PNG preview that
is exactly what decoding avoids — and `CorpusWmfTests` reads them the way a reader would.
**The legacy binary families matter more here than they do for SVG**: DOC, XLS and PPT store
a WMF as the presentation of every embedded OLE object, so the Escher path in
`Paperless.MsBinary` is where most real WMFs in a corpus will actually be found.

**EMF and EMF+ change nothing again, and the DOCX fixture settles the sniffing argument in
its sharpest form.** LibreOffice writes a genuine EMF into `word/media/image1.wmf`, and
`[Content_Types].xml` declares no type for `.wmf` at all — so neither the part name nor the
declared type identifies the format, and only the bytes do. An EMF+ file is even less
distinguishable: it *is* an EMF, with the same `EMR_HEADER` and the same signature, and the
EMF+ inside it has no signature of its own anywhere. Nothing in the hook has to know that,
which is the whole point of putting the decision behind `VectorImages.For`.
