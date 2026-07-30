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

## Shared groundwork

Do this before any individual format.

- [ ] A device-context model: current transform, clip stack, and the selected pen/brush/
      font/palette state. All three metafile formats are stateful command streams over
      essentially this same model, so build it once.
- [ ] A graphics-object table with correct handle-reuse and delete semantics — real files
      reuse handles aggressively and leak others.
- [ ] Mapping-mode and window/viewport arithmetic. **The most common source of
      wrongly-scaled or mirrored output**, and it is shared between WMF and EMF.
- [ ] Bounded replay: a record count and time cap. These streams come from untrusted
      documents and can be crafted to loop or allocate unboundedly.

## WMF

Oldest and smallest. Good first target — it exercises the shared groundwork on the simplest
of the three.

- [ ] Placeable header (`D7 CD C6 9A`) and the bare metafile header
- [ ] Object records: pen, brush, font, palette, region creation and selection
- [ ] Drawing records: `TextOut`, `ExtTextOut`, `Polygon`, `PolyPolygon`, `Polyline`,
      `Rectangle`, `RoundRect`, `Ellipse`, `Arc`, `Chord`, `Pie`, `LineTo`/`MoveTo`
- [ ] Bitmap records: `StretchDIBits`, `SetDIBitsToDevice`, `BitBlt`, `StretchBlt`
- [ ] Clipping and region records
- [ ] `ESCAPE` records, which is where some producers hide EMF payloads

## EMF

- [ ] Header, bounds, frame, and the reference-device fields that set the coordinate scale
- [ ] Path construction (`BeginPath`/`EndPath`/`StrokePath`/`FillPath`/`StrokeAndFillPath`)
- [ ] World-transform records (`SetWorldTransform`, `ModifyWorldTransform`)
- [ ] Text: `EMR_EXTTEXTOUTW`/`A`, including per-glyph DX arrays — honour the DX array
      rather than re-measuring, or text spacing drifts from what the producer intended
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

## SVG — use Svg.SceneGraph, do not hand-roll

**Decided: reuse the Svg.Skia family's parser rather than writing our own.** Verified by
building against it on .NET 10.

Take `Svg.SceneGraph` + `Svg.Model` + `ShimSkiaSharp` but **not** `Svg.Skia` itself. That
combination:

- pulls in **no SkiaSharp at all** — only `ExCSS`, `ShimSkiaSharp`, `Svg.Custom`,
  `Svg.Model` — so there is no clash with our own SkiaSharp version and no second native
  dependency;
- produces a **device-independent canvas-command list**, not pixels, so the PDF backend
  gets real vectors;
- is permissively licensed (MIT, except `Svg.Custom` which is MS-PL).

Hand-rolling SVG would have meant reimplementing a CSS cascade, the full path grammar,
gradients, masks and filters — for no fidelity gain, since the command list is exactly what
we would have produced anyway.

`ShimSkiaSharp`'s command set maps almost one-to-one onto `IDrawingSink`, which is why the
translation is small:

| ShimSkiaSharp command | `IDrawingSink` |
|---|---|
| `SaveCanvasCommand` / `RestoreCanvasCommand` | `Save` / `Restore` |
| `SetMatrixCanvasCommand` | `Transform` |
| `ClipPathCanvasCommand` / `ClipRectCanvasCommand` | `ClipPath` |
| `DrawPathCanvasCommand` | `FillPath` / `StrokePath`, per the paint's style |
| `DrawImageCanvasCommand` | `DrawImage` |
| `SaveLayerCanvasCommand` | `BeginTransparencyGroup` / `EndTransparencyGroup` |
| `DrawTextCanvasCommand`, `DrawPositionedTextRunCanvasCommand`, `DrawTextBlobCanvasCommand`, `DrawTextOnPathCanvasCommand` | `DrawGlyphRun` |
| `DrawPictureCanvasCommand` | recurse |

### Work items

- [ ] Open an SVG through `Svg.Model.Services.SvgService`, build the scene, and record it to
      a `ShimSkiaSharp` picture.
- [ ] Translate the command list to `IDrawingSink` per the table above.
- [ ] Convert the shim's geometry and paint types to ours: `SKPath` → `GraphicsPath`,
      `SKShader` gradients → `GradientPaint`, `SKPaint` stroke state → `Stroke`. Note the
      shim's coordinates are floats in SVG user units — scale into EMUs once, at the
      boundary.
- [ ] **Plug our own text stack in** via `ISvgTextRunTypefaceResolver`,
      `ISvgTextGlyphRunResolver` and `ISvgAssetLoader`, so SVG text is resolved and shaped by
      `Paperless.Text` rather than by a second, divergent text path. This is the main reason
      to prefer this library over a rasterising one, and it is what keeps SVG text
      consistent with document text.
- [ ] Route image loading through `ISvgAssetLoader` to our own decoder.
- [ ] Assess filter coverage empirically against real documents before deciding how much
      further to take it.

### Enforce these at the boundary

The library will happily do things a document parser must not. None of these are its fault;
they are our responsibility to prevent:

- [ ] **No external references.** Implement `ISvgAssetLoader` so it refuses network and
      local-filesystem access outright — only data URIs and package-internal parts resolve. A
      document parser that fetches URLs is an SSRF and data-exfiltration vector.
- [ ] **No scripting.** Confirm `<script>`, event attributes and `javascript:` URLs are
      inert, and add a test asserting it rather than trusting the default.
- [ ] **Bound the work.** Cap element count, nesting depth and total time. SVG supports
      recursive `<use>` and enormous filter chains; both are trivially weaponisable.
- [ ] Render the initial state of any declarative animation, which is what a static export
      shows.

## Open questions

- [ ] Lazy decoding? Documents frequently embed graphics that are never drawn — a slide
      master's unused placeholder art, for instance. Decoding on first draw would avoid that
      cost entirely.
- [ ] Cache decoded output as a display list, given the same image is often drawn on many
      slides?
- [ ] EMF+ path gradients have no direct SkiaSharp equivalent and will need decomposing.
      Establish how faithful that has to be before building it.
- [ ] `ShimSkiaSharp`'s text commands carry typeface plus string, not resolved glyph ids.
      Check whether `ISvgTextGlyphRunResolver` gives us enough control to emit a real
      `GlyphRun`, or whether SVG text needs re-shaping on our side after translation.
