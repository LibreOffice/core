# Paperless.Vector — TODO

Importers for embedded vector graphics: WMF, EMF, EMF+ and SVG.

**Decided: full support for all four.** Not a subset, and no rasterise-via-LibreOffice
shortcut. SVG reuses an existing library (see below); WMF, EMF and EMF+ are ours to write,
because nothing exists in C# for them.

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

**Read this first.** SVG is done, and it exists as much to settle the shape of this seam as
to draw SVG. Everything below is what WMF, EMF and EMF+ plug into.

### What a decoded vector image is

Not pixels, and not a live replay. `IVectorImageDecoder.Decode` answers a `VectorImage`:

| Piece | What it is | SVG | EMF |
|---|---|---|---|
| `Content` | A `DisplayList` — recorded `IDrawingSink` calls, replayable | the translated command list | the replayed records |
| `ViewBox` | The rect of `Content`'s coordinates that fills the destination, **in EMUs** | the viewport, converted at 9525 EMU per user unit | `rclBounds`, or the window rect once the mapping mode is applied |
| `IntrinsicSize` | The physical size the picture asks for when nothing imposes one | `width`/`height` against the CSS inch | `szlMillimeters` |

`DisplayList` is itself an `IDrawingSink`, so a decoder writes to the same interface a
backend implements and needs no second output path. Recording rather than drawing straight
through is what makes one decode serve a logo that appears on forty slides, and what makes
the commands countable for `VectorLimits`.

**Keeping `ViewBox` and `IntrinsicSize` apart is the point.** For SVG they usually coincide;
for EMF they will not, because an EMF states its logical coordinate space and its physical
extent independently. Collapsing them is the classic wrongly-scaled metafile.

### What the seam does not give you

- **No device context.** `Save`/`Restore`/`Transform` on the sink are a clip-and-transform
  stack, nothing more. There is no selected pen, brush, font or palette, no mapping mode and
  no current position, because `IDrawingSink` is deliberately stateless per draw call. The
  DC is the metafile implementer's to build, once, shared between WMF and EMF — see
  *Shared groundwork* below.
- **No path arithmetic.** `ClipPath` intersects and nothing unions, subtracts or offsets.
  The SVG translator gets away with expressing a union as one path of several subpaths under
  the non-zero rule; a region record cannot, and `ExtSelectClipRgn` with `RGN_DIFF` will
  need real path operations or an honest diagnostic.
- **No stroked text.** `DrawGlyphRun` takes one paint. SVG's `fill="none" stroke="red"` text
  is filled in the stroke's colour with a `PL6015` diagnostic; EMF+ `DrawDriverString` with
  a pen will want the same compromise, or an extension to Core.

### What a decoder owes

1. **Convert to EMUs once, at the boundary.** `Svg/ShimGeometry.cs` is the worked example: a
   single `EmuPerPixel` constant, one `Emu(double)`, and nothing downstream that knows what
   unit the source used. A scale factor applied in two places is a scale factor applied
   twice, and that is the commonest way vector import comes out at the wrong size.
2. **Honour `VectorLimits`.** Charge every recorded command, check the clock on every one,
   cap path segments and nesting depth. `Svg/SvgPictureTranslator.cs` shows the pattern: a
   `Charge()` that returns false and sets `IsTruncated` rather than throwing.
3. **Resolve nothing outside the document.** No URL, no file path, whatever the format
   allows. WMF's `ESCAPE` records and EMF's comment payloads are where this will come up.
4. **Never throw for malformed input.** A picture that cannot be read is a document to draw
   *without* that picture: return `VectorImage.Empty` with a diagnostic. Diagnostic codes
   `PL60xx` belong to this library.
5. **Register in `VectorImages`** and sniff by content in `CanDecode`. Nothing that calls
   `VectorImages.Decode` changes when a format is added, which is the point of the seam.

---

## Shared groundwork

Still to do, and still before any individual metafile format.

- [ ] A device-context model: current transform, clip stack, and the selected pen/brush/
      font/palette state. All three metafile formats are stateful command streams over
      essentially this same model, so build it once. **The seam gives you none of this** —
      see above.
- [ ] A graphics-object table with correct handle-reuse and delete semantics — real files
      reuse handles aggressively and leak others.
- [ ] Mapping-mode and window/viewport arithmetic. **The most common source of
      wrongly-scaled or mirrored output**, and it is shared between WMF and EMF. The
      `ViewBox`/`IntrinsicSize` split above is where its answer lands.
- [x] **Bounded replay: a record count and time cap.** Done, on the seam rather than per
      format: `VectorLimits` caps bytes, commands, path segments, nesting depth, expanded
      source nodes and wall-clock time. A metafile decoder inherits the type and has only to
      charge against it.

## WMF

Oldest and smallest. Good first target — it exercises the shared groundwork on the simplest
of the three.

- [ ] Placeable header (`D7 CD C6 9A`) and the bare metafile header
- [ ] Object records: pen, brush, font, palette, region creation and selection
- [ ] Drawing records: `TextOut`, `ExtTextOut`, `Polygon`, `PolyPolygon`, `Polyline`,
      `Rectangle`, `RoundRect`, `Ellipse`, `Arc`, `Chord`, `Pie`, `LineTo`/`MoveTo`
- [ ] Bitmap records: `StretchDIBits`, `SetDIBitsToDevice`, `BitBlt`, `StretchBlt` — a DIB
      can go through as decoded pixels or as `RasterImage.Encoded`; both draw, and neither
      needs a codec in this library
- [ ] Clipping and region records
- [ ] `ESCAPE` records, which is where some producers hide EMF payloads

## EMF

- [ ] Header, bounds, frame, and the reference-device fields that set the coordinate scale.
      `rclFrame` is in 1/100 mm and gives `IntrinsicSize`; `rclBounds` is in device units and
      gives `ViewBox`
- [ ] Path construction (`BeginPath`/`EndPath`/`StrokePath`/`FillPath`/`StrokeAndFillPath`)
- [ ] World-transform records (`SetWorldTransform`, `ModifyWorldTransform`)
- [ ] Text: `EMR_EXTTEXTOUTW`/`A`, including per-glyph DX arrays — honour the DX array
      rather than re-measuring, or text spacing drifts from what the producer intended.
      `PositionedGlyph.Advance` is per-glyph precisely so a DX array survives
- [ ] `EMR_SMALLTEXTOUT`, and the glyph-index variants that bypass character mapping
- [ ] Bitmap records: `StretchDIBits`, `AlphaBlend`, `TransparentBlt`, `BitBlt`
- [ ] Clipping: `ExtSelectClipRgn`, `IntersectClipRect`, `OffsetClipRgn`
- [ ] Gradient fill (`EMR_GRADIENTFILL`)
- [ ] Pen styles: geometric vs cosmetic, dash patterns, caps and joins

## EMF+

Roughly fifty record types, carried inside `EMR_COMMENT` records.

- [ ] `EmfPlusRecord` dispatch and `EmfPlusHeader` handling
- [ ] **Dual-mode files.** Many EMF+ files also carry an equivalent EMF representation for
      older consumers. Decide *once* which interpretation to replay and follow it
      consistently — replaying both double-draws everything, and switching mid-stream
      produces incoherent output. LibreOffice's handling of this is the reference.
- [ ] Object table: pens, brushes, paths, images, fonts, string formats, image attributes,
      custom line caps
- [ ] Brush types: solid, hatch, texture, path gradient, linear gradient. Path gradients are
      the awkward one and are common in real clip art.
- [ ] Path and point record forms, including the compressed and relative point encodings
- [ ] Drawing records: `DrawLines`, `DrawPath`, `FillPath`, `DrawString`, `DrawImage`,
      `DrawImagePoints`, `FillRects`, `FillPolygon`, `DrawCurve`, `DrawBeziers`
- [ ] Text via `DrawString` with `EmfPlusStringFormat` — alignment, wrapping, trimming
- [ ] Colour matrix and image-attribute effects
- [ ] Transform records, and the container save/restore records
      (`Save`/`Restore`/`BeginContainer`/`EndContainer`)
- [ ] Anti-aliasing and pixel-offset modes, insofar as they change geometry rather than just
      quality

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
      alternative is a paint that carries a `DisplayList` tile; that is a Core change and
      should wait until EMF+ texture brushes want the same thing. `PL6019`, and the shape is
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
- [ ] EMF+ path gradients have no direct `GradientPaint` equivalent and will need
      decomposing. Establish how faithful that has to be before building it; the band
      decomposition in `Paperless.Rendering.Fills.Gradients` is the precedent.
- [ ] A picture drawn at several sizes re-uses one `DisplayList`, which is right for geometry
      and indifferent to hinting. Nothing depends on it yet; it will matter when a thumbnail
      and a print rendering share a cache.

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
