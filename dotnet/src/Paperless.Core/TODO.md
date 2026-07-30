# Paperless.Core — TODO

Abstractions everything else agrees on. **Must keep zero external dependencies**: anything
added here is inherited by every consumer.

## Done

- [x] `Length` — exact EMU-based length with conversions to/from twips, 1/100 mm, points,
      inches, pixels
- [x] Geometry: `DocPoint`, `DocSize`, `DocRect`, `Margins`, `AffineTransform`
- [x] `Colour` — straight (non-premultiplied) 8-bit sRGB with alpha
- [x] `DocumentFormat` enum covering all in-scope formats; `DocumentFamily`, `ContainerKind`
- [x] `FormatInfo`, `IFormatCatalogue`, `IFormatIdentifier`, `FormatIdentification`
- [x] `IDocument`, `IPaginatedDocument`, `IPageSequence`, `IPage`, `LayoutOptions`
- [x] `IDocumentReader`, `DocumentSource`
- [x] `DocumentMetadata`, `DocumentStatistics`
- [x] Extraction content tree
- [x] `IDrawingSink` and its supporting types (`GraphicsPath`, `Paint`, `Stroke`,
      `GlyphRun`, `FontReference`, `RasterImage`)
- [x] `Diagnostic` / `DiagnosticLocation`; the exception hierarchy

## To do

### Format catalogue

- [ ] Populate the full catalogue from the table in
      `research/01-formats-and-detection.md`: extensions, media types, display names,
      template and macro flags, per-format read support.
- [ ] `FindByExtension` with ambiguity handled properly — some extensions map to several
      formats.
- [ ] Keep `IsReadSupported` honest as readers land. It is what `paperless identify`
      reports, so a wrong value is worse than a missing feature.

### Drawing IR

- [ ] A recording sink that captures a command list, for testing and for caching a laid-out
      page.
- [ ] A bounds-computing sink — cheap way to answer "what area did this page actually
      touch", useful for cropping and for the fidelity harness.
- [ ] Decide whether `IDrawingSink` needs a soft-mask/luminance-mask operation. OOXML
      picture effects and some gradients want one, and retrofitting it later would touch
      every backend. Check `research/06-rendering.md` before deciding.
- [ ] Consider a text-decoration primitive (underline, strikethrough) rather than making
      each caller synthesise rectangles: PDF and SVG can express decoration natively and
      more accurately.

### Content tree

- [ ] A visitor, so consumers can walk the tree without type switches.
- [ ] JSON serialisation with a stable schema — the CLI emits it and the comparison
      harness parses it, so it is effectively public API.
- [ ] Decide whether to keep character-offset provenance (which byte range of which part a
      run came from). Extremely useful for debugging extraction; costs memory. Probably
      opt-in.

### Open questions

- [ ] `Length` currently rounds half-away-from-zero. Confirm that matches what the office
      formats do at every conversion boundary; a systematic half-unit bias would show up as
      one-pixel drift everywhere.
- [ ] Should `IDocument` expose the raw container for callers wanting embedded objects
      Paperless does not interpret? Useful, but it leaks a layer.
