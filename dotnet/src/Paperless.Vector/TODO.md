# Paperless.Vector — TODO

Importers for embedded vector graphics: WMF, EMF, EMF+ and SVG.

**Decided: full support for all four.** Not a subset, and no rasterise-via-LibreOffice
shortcut.

That makes this the largest single body of work in the project, so it is worth being clear
why it earns that. Office documents embed these constantly — pasted clip art, chart and
Visio snapshots, equation images, and the fallback rendering that accompanies SmartArt and
OLE objects. Anything less than real vector import shows up as blank rectangles on visually
important pages, and rasterised substitutes look wrong at print resolution.

There is no C# library to build on for EMF/EMF+, so all of it is ours.

Reference: `research/06-rendering.md` section D. LibreOffice's own implementations —
`emfio/` for WMF/EMF/EMF+ and `svgio/` for SVG — are the working reference; port their
record handling rather than reading the specifications cold.

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

## SVG

Full static SVG: the SVG 1.1 static subset plus the SVG 2 features producers actually emit.

- [ ] Document structure: `svg`, `g`, `defs`, `symbol`, `use`, `switch`
- [ ] `viewBox` and `preserveAspectRatio`
- [ ] Shapes: `rect`, `circle`, `ellipse`, `line`, `polyline`, `polygon`, `path`
- [ ] Full path grammar, including arc segments and the implicit-repeat forms
- [ ] Presentation attributes **and** CSS: inline `style`, `<style>` blocks, selector
      matching, specificity, inheritance. Producers use both interchangeably, so
      attribute-only support silently mis-styles a lot of real files.
- [ ] Paint: `fill`, `stroke`, `fill-rule`, dash arrays, caps, joins, `opacity`,
      `fill-opacity`, `stroke-opacity`
- [ ] Gradients (`linearGradient`, `radialGradient`, stops, `gradientUnits`,
      `gradientTransform`, `spreadMethod`) and `pattern`
- [ ] `clipPath`, `mask`, `marker`
- [ ] Transforms, including `transform-origin`
- [ ] Text: `text`, `tspan`, `textPath`, `x`/`y`/`dx`/`dy` lists, `text-anchor`, `font-*`.
      Needs `Paperless.Text` for shaping.
- [ ] Embedded raster via `image` with a data URI
- [ ] Filters (`filter`, `feGaussianBlur`, `feOffset`, `feBlend`, `feColorMatrix`,
      `feComposite`, `feMerge`, ...). **The expensive tail** — schedule it last and
      separately from the rest. A document needing filters is far rarer than one needing
      gradients, and partial filter support is more useful than none.

### Deliberately excluded, permanently

Not scope compromises — security decisions, and they stay excluded even under "full
support":

- **No external references.** No network fetching and no local file reads, for `image`,
  `use`, fonts or anything else. A document parser that makes network requests is an SSRF
  and data-exfiltration vector.
- **No scripting.** `<script>`, event attributes and `javascript:` URLs are ignored.
- **No declarative animation.** `<animate>` and friends: render the initial state, which is
  what a static export shows.

## Open questions

- [ ] Lazy decoding? Documents frequently embed graphics that are never drawn — a slide
      master's unused placeholder art, for instance. Decoding on first draw would avoid that
      cost entirely.
- [ ] Cache decoded output as a display list, given the same image is often drawn on many
      slides?
- [ ] EMF+ path gradients have no direct SkiaSharp equivalent and will need decomposing.
      Establish how faithful that has to be before building it.
