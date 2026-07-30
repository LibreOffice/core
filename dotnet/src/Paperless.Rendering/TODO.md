# Paperless.Rendering — TODO

Backends consuming `IDrawingSink`: Skia raster, PDF, SVG.

Reference: `research/06-rendering.md` sections A, E and G.

## Skia raster backend

The default, because Skia is permissively licensed and gets the awkward parts —
arbitrary-path clipping, gradients, transparency groups — right without extra work.

- [ ] `IDrawingSink` over `SKCanvas`; EMU→pixel mapping at a chosen DPI
- [ ] Paths, including the non-zero/even-odd distinction
- [ ] Solid, gradient and tiled-bitmap paints. Gradient geometry must match LibreOffice's
      (`basegfx` gradient tools) or fills are subtly wrong everywhere.
- [ ] Strokes: dash patterns, caps, joins, miter limit, and **hairlines** — a zero width
      means "thinnest the device can draw", which is a real concept in the office formats
      and not the same as invisible
- [ ] `DrawGlyphRun` via glyph IDs and explicit positions. Must **not** re-shape: layout
      already committed to these advances when it chose line breaks.
- [ ] Transparency groups as offscreen surfaces — a group at 50% opacity differs from each
      member at 50%, and shape groups use group-level transparency
- [ ] PNG encode; deterministic output so golden images are checksummable

## PDF writer

Hand-rolled. The requirement is real text with embedded subset fonts, correct transparency
groups, and pass-through of already-compressed images; no C# library covers that
combination. LibreOffice reaches the same conclusion and writes its own.

- [ ] Document structure: objects, xref, trailer; both classic and stream xref
- [ ] Content streams: graphics state, paths, clipping, text
- [ ] Font embedding with subsetting via `hb-subset` (HarfBuzzSharp) — the one part with a
      ready answer, and the same tool LibreOffice uses
- [ ] `ToUnicode` CMaps from `GlyphRun.ClusterMap`, so output text is searchable and
      copyable. This is why the IR carries text and a cluster map at all.
- [ ] Gradients as shading dictionaries; tiling patterns for bitmap fills
- [ ] Transparency groups and soft masks (ExtGState `/SMask`)
- [ ] JPEG pass-through via `DCTDecode` — faster and lossless
- [ ] Deterministic output: a fixed `CreationDate` (`PdfRenderOptions.CreationDate`) and no
      other varying field, so a PDF can be byte-compared in tests
- [ ] Optional: tagged PDF, outlines, links

## SVG writer

- [ ] Paths, gradients, clip paths
- [ ] Text as `<text>` where faithful, with a fallback to outlines when shaping cannot be
      expressed
- [ ] Embed images as data URIs; never emit external references

## Open questions

- [ ] Is a Skia-independent "reference" software rasteriser worth having, to keep golden
      images stable across SkiaSharp upgrades? A Skia version bump can change antialiasing.
- [ ] Should the PDF writer support incremental page writing for large documents, or is
      buffering the whole document acceptable?
