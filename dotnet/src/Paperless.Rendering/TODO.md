# Paperless.Rendering — TODO

Backends consuming `IDrawingSink`: PDF, Skia raster, SVG.

Reference: `research/06-rendering.md` sections A, E and G.

Status: **the PDF writer and the Skia rasteriser both write files**, and the PDF one is
compared against LibreOffice's own PDF operator for operator. **Both paint every fill the
display list can express** — solid, gradient, tiled bitmap and coloured triangle mesh, with a
spread and a focus on the gradient — and a Skia-backed decoder turns an encoded picture into a
`RasterImage`. The SVG *writer* is still a stub.

**The mesh is the newest of the four and the only one added for a format rather than for a
document model.** A GDI+ path-gradient brush states a colour at every vertex of an arbitrary
boundary, which no stop list says at any length, and both backends turn out to have a native
form that agrees with the other on exactly "vertices with colours, plus index triples": a PDF
`/ShadingType 4` and Skia's `SKVertices`. That agreement is what let `MeshPaint` go into
`Paperless.Core` without either backend leaking into it — see `Graphics/Paint.cs`, and
`src/Paperless.Vector/TODO.md` for what it retired.

## How this library is verified, and why it is verified that way

Worth reading before adding anything here, because it changes what is worth building.

**The fidelity harness reads LibreOffice's PDFs, and nothing in it is specific to
LibreOffice.** `tests/Paperless.TestKit/LibreOffice/PdfTextRuns`, `PdfFills`, `PdfStrokes` and
`PdfPageSizes` were written to pull pen positions, filled rectangles, stroked lines and sheet
sizes out of the reference. Pointed at *our* output they compare two content streams — same
pens, same sizes, same fills, same strokes — which is a far sharper question than an image
diff and a far easier one to attribute when the answer is no.

**A fifth reader, `PdfPaints`, was added for the fills.** It reports the shading dictionaries a
PDF paints — type, coordinates, the colours the function gives at each end of its domain, and the
clip the `sh` landed in — and every image XObject placement with its pixel size, its filter and
whether it carries an `/SMask`. It is a separate reader rather than an extension of `PdfFills`
because `PdfFills` finds a page by inflating every stream and keeping the ones holding `BT`, and
a slide of four filled shapes has no `BT` in it at all; changing that rule would renumber the
pages every existing comparison reports, so nothing about `PdfFills`, `PdfStrokes`,
`PdfTextRuns` or `PdfPageSizes` was touched. `PdfPaints` walks the page objects properly instead
— which is worth doing rather than guessing, because a shading dictionary contains the string
`/ShadingType`, so picking the resource list as "the dictionary mentioning `/Shading`" silently
selects the last shading in the file and every lookup then returns nothing.

**A trap the mesh sprang, in the test reader rather than in the writer.** `PdfFile.ContentStreams`
finds a page's content *by elimination* — a deflated stream with no `/Length1` and no `/Subtype`
— which held for every stream this backend wrote until a `/ShadingType 4` arrived. A mesh
shading is a stream, is deflated, and states neither, so the first mesh silently made every
`ContentStreams().ShouldHaveSingleItem()` in the file fail on a page whose content was correct.
The rule now excludes `/ShadingType` too. **Any future stream that is neither text nor a picture
will do the same**, and the failure names the assertion rather than the cause.

That is why the file is shaped the way it is, and each of these would otherwise be a free
choice:

| Decision | Because |
|---|---|
| Content streams deflated, font programs **not** | The harness finds content by inflating every stream and keeping the ones holding `BT`. A deflated font program inflates fine and contains `BT` often enough to be read as a page. |
| Simple `/TrueType` fonts, one-byte codes | `PdfTextRuns` counts two hex digits per glyph. A composite Identity-H font would report twice the glyphs on our side. It is also what LibreOffice writes. |
| Rectangles as `re` | What `PdfFills` looks for, and what every PDF writes. |
| Pen width immediately before its own path | `PdfStrokes` pairs a line with the nearest preceding `w`, allowing no `m` or `S` in between. |
| No flipping `cm` at the top of the page | The text matrix inherits it, so every glyph would be upside down. Converting each coordinate — `pageHeight - y` — is what LibreOffice does and what makes a `Td` in our stream directly comparable with a `Td` in its. |

**Measured, per line, over ten documents in four formats** (`PdfOutputComparisonTests`):
line counts match exactly, font sizes match exactly, baseline pitch agrees to **0.000 pt**,
and the pen differs by exactly **-0.100 pt** — the two twips LibreOffice's PDF export adds
horizontally, already recorded in `src/Paperless.WordProcessing/TODO.md` and now visible
directly rather than inferred. On the two justified documents the pen difference spreads to
between -0.034 and -0.142 pt, which is its own per-blank rounding moving the start of a line.

## PDF writer

Hand-rolled. The requirement is real text with embedded subset fonts, correct transparency
groups and pass-through of already-compressed images; no C# library covers that combination.
LibreOffice reaches the same conclusion and writes its own.

- [x] Document structure: objects, classic `xref`, trailer. Objects are reserved and filled
      in later, because a PDF is a graph and almost every dictionary names something that
      does not exist yet.
- [x] Content streams: graphics state, paths, clipping, text.
- [x] Font embedding with subsetting via `hb-subset`. **HarfBuzzSharp's managed binding does
      not expose it** — verified against 14.2.1.1, whose only matching export is the unrelated
      `hb_set_is_subset` — but the native library it ships exports all thirty-two entry
      points, so `FontSubsetter` binds them directly.
- [x] `ToUnicode` CMaps from `GlyphRun.ClusterMap`. `pdftotext` gets the same words out of
      our PDF as out of LibreOffice's, on four corpus documents.
- [x] Transparency groups as form XObjects with `/Group << /S /Transparency >>` and an
      `ExtGState`. A real group rather than an alpha on each member, because where two
      members overlap the two are different pictures.
- [x] JPEG pass-through via `DCTDecode`; other rasters as deflated RGB with an `/SMask` when
      any pixel is not opaque.
- [x] Deterministic output: a fixed `CreationDate` and no other varying field. Two writes of
      the same document are byte-identical, pinned by a test.
- [x] **Gradients as shading dictionaries.** Linear is `/ShadingType 2` and radial and
      elliptical are `/ShadingType 3`, both `/Extend [true true]`, painted as a clip and an
      `sh` — the form `PDFWriterImpl::drawGradient` writes
      (`vcl/source/pdf/pdfwriter_impl.cxx:9194`) and *not* the shading pattern the same picture
      could be spelled as. The reason is the transform: a pattern's `/Matrix` maps pattern space
      to the page's default space and so ignores any `cm` in force, which would leave a rotated
      shape's gradient pointing the wrong way, whereas `sh` paints in the current user space and
      inherits it for nothing. A gradient's own `Transform` is then one more `cm` inside the
      clip.
- [x] **Bitmap fills as one image draw per tile**, inside a clip, sharing one XObject however
      many tiles name it. Also LibreOffice's form and for the same reason — measured on its own
      PDF of `tests/corpus/features/paint-fills.fodp`, whose one checkerboard rectangle comes out
      as `re W* n` and 47 `q … cm /Im10 Do Q` groups over a single 8×8 XObject. A
      `/PatternType 1` tiling pattern would state the same picture and inherit no transform.
- [x] **Triangle meshes as `/ShadingType 4`.** A free-form Gouraud triangle mesh: a stream of
      vertices, each an edge flag, a packed coordinate pair and a colour, with flag 0 starting a
      fresh triangle. Painted as a clip and an `sh` exactly as a gradient is, because a mesh is a
      *fill* — a GDI+ path-gradient brush's boundary and the shape it fills need not be the same
      polygon, so the triangles are painted where they lie and the path decides how much shows.
      Three decisions worth keeping:
      **type 4 and not 5**, because a fan built from a boundary is not a lattice and type 5's
      `/VerticesPerRow` would have to be invented;
      **every triangle written as three flag-0 vertices**, rather than as a strip or a fan,
      which triples nothing that matters — consecutive fans share no edge — and removes the one
      thing a reader can get wrong about the format;
      and **`/BitsPerFlag 8`, `/BitsPerCoordinate 32`, `/BitsPerComponent 8`**, which make a
      vertex 1 + 4 + 4 + 3 bytes so the "each vertex begins on a byte boundary" rule holds by
      construction rather than by padding. Sixteen-bit coordinates would have quantised a
      page-wide mesh to a fifteenth of a millimetre, visible as a ragged boundary on the very
      shape the paint exists to draw. `/Decode` spends the whole 32-bit range on the mesh's own
      bounding box rather than on the page.
- [x] **A mesh with a translucent vertex takes the same luminosity soft mask a fading gradient
      does** — a second `/ShadingType 4` in `DeviceGray` whose grey level is each vertex's alpha.
      Sharing `PdfContentSink.SoftMask` between the two is what stops a faded mesh and a faded
      gradient disagreeing; a shading has no alpha channel and there is only one way to say so.
- [x] **A repeating gradient as a lengthened axis.** PDF has no tiling for `sh`: `/Extend` clamps
      the shading's parameter and cannot repeat it ([PDF 32000-1] 8.7.4.5.3 computes `t` from a
      parameter already clipped to 0..1). So a repeat is spelled by extending the *axis* over as
      many whole periods as the shape spans, widening `/Domain` to match, and stitching one copy
      of the ramp per period — with `/Encode [1 0]` on alternate periods for a reflect, which is
      how PDF states a mirrored copy without a second function object. `Fills.Gradients.Periods`
      counts the periods and is the only part of `SpreadMethod` either backend needed to share,
      because Skia states the same thing as a tile mode and repeats for nothing.
- [x] **A focal radial as the two circles `/ShadingType 3` already took.** The inner circle
      collapsed to a point at the focus rather than at the centre. Exact, not an approximation,
      which is why `GradientPaint.Focus` is a coordinate rather than a mesh.
- [x] **Soft masks (`ExtGState` `/SMask`) for a gradient whose stops fade.** A shading's colour
      space is `DeviceRGB` and it has no alpha, so a fade is a second shading in `DeviceGray`
      with each stop's alpha as its grey level, painted into a `/Group << /S /Transparency /CS
      /DeviceGray >>` form that a `/SMask << /S /Luminosity /G … /BC [0] >>` reads. One alpha
      shared by every stop takes a constant `/ca` instead, which costs one small object rather
      than three. Not a refinement: Skia's shader colours carry an alpha for nothing, so without
      this the same `GradientPaint` faded on a PNG and was opaque in a PDF.
- [ ] Optional: tagged PDF, outlines, links. LibreOffice writes `/StructElem` trees and
      `/MarkInfo` and we do not; a comparison never looks at them, but accessibility does.
- [ ] **The whole document is buffered before a byte is written.** That is what makes the
      classic `xref` cheap — every offset is known up front — and it means a thousand-page
      document costs a thousand pages of memory. Incremental writing needs a cross-reference
      *stream* and is worth doing when a document that large turns up, not before.
- [ ] `/ID` in the trailer. Optional for an unencrypted file and omitted, because a
      content-derived one would have to be computed over the buffered body and a random one
      would break byte determinism.

## Skia raster backend

- [x] `IDrawingSink` over `SKCanvas`; EMU→pixel mapping at a chosen DPI. Skia's y already
      grows downwards as a document's does, so no flip — the opposite of the PDF backend.
- [x] Paths, including the non-zero/even-odd distinction.
- [x] Strokes: dash patterns, caps, joins, miter limit, and **hairlines** — a zero width
      means "thinnest the device can draw", which PDF and Skia both spell the same way.
- [x] `DrawGlyphRun` via glyph IDs and explicit positions, never re-shaped.
- [x] Transparency groups as offscreen layers (`SaveLayer`).
- [x] PNG and JPEG encode. PNG is deterministic and a page written twice is byte-identical;
      JPEG is for thumbnails and must not be compared against anything, which a test pins by
      showing a flat fill does not survive the round trip.
- [x] **Gradients as `SKShader`s** — linear and radial, with the gradient's own transform as
      the shader's local matrix, which is also all an elliptical gradient needs. The stops go
      through the same normalisation the PDF backend uses rather than straight into Skia, which
      tolerates more than PDF's stitching function does; normalising in one place is what stops
      two backends drawing different pictures from one list. A `SpreadMethod` is the shader's
      tile mode and a `Focus` makes it a two-point conical, both for nothing.
- [x] **Triangle meshes as `SKVertices`**, drawn inside the path as a clip so the two backends
      agree by construction. Two details that are not optional:
      **`SKBlendMode.Dst` with a shaderless paint**, because `drawVertices` blends the per-vertex
      colours *with the paint's shader* and a mode that reads the source — `Modulate`, the usual
      choice — multiplies them by whatever a shaderless paint supplies and can black the mesh
      out; and **antialiasing off for the triangles**, because adjacent fan triangles share an
      edge exactly and two antialiased edges composited over each other leave a seam of the
      background along every one — a hundred-vertex boundary would be a hundred pale spokes. The
      mesh's own outline is antialiased by the clip instead.
- [x] **Tiled and stretched `BitmapPaint`** as a repeating image shader whose local matrix
      places one cell of the grid `Fills.Tiles` computes — the same grid the PDF backend walks
      tile by tile, so the two agree by construction rather than by inspection. A stretched paint
      is the degenerate grid of one cell and clamps rather than repeats, so a rounding pixel at
      the edge does not wrap the far side of the image into view.
- [x] **Raster image decode**, in `Paperless.Rendering.Images.RasterImageDecoder`. PNG, JPEG,
      GIF, BMP, WebP and ICO, sniffed **by content** rather than by a declared media type, which
      is the same rule format identification follows and for the same reason: a `.png` holding a
      JPEG is routine, and believing the name would cost the `DCTDecode` pass-through or, worse,
      write a JPEG into a PDF claiming to be deflated RGB. Pixels come back as straight
      (non-premultiplied) RGBA, because an `/SMask` is a separate greyscale image whose samples
      *are* the alpha and dividing premultiplied colour back out loses precision exactly where
      the alpha is low.

## Fills: what draws, what it was measured against, and what is left

The corpus document is `tests/corpus/features/paint-fills.fodp` — a slide stating a linear, an
axial and a radial gradient, a one-centimetre tiled checkerboard and, on a second page, an
embedded picture — plus `paint-fills-pptx.pptx`, which is LibreOffice's own export of it and
keeps all three gradients as `a:gradFill` and the tile as `a:blipFill`/`a:tile`, so the OOXML
reader has a file to aim at rather than a hypothesis.

### The comparison had to change shape, and this is why

**LibreOffice writes no shading dictionary for a shape gradient, ever.** Its PDF writer has one
(`PDFWriterImpl::writeGradientFunction`, `vcl/source/pdf/pdfwriter_impl.cxx:7965`) and it is
unreachable from a slide: the drawing layer decomposes every gradient into flat bands before the
writer sees it —

> `// tdf#150551 for PDF export, use the decomposition for better gradient visualization`
> — `drawinglayer/source/processor2d/vclmetafileprocessor2d.cxx`,
> `VclMetafileProcessor2D::processPolyPolygonGradientPrimitive2D`

— and even the metafile path that survives accepts only `LINEAR` and `AXIAL` with no explicit
step count (`lcl_canUsePDFAxialShading`, `pdfwriter_impl2.cxx:1061`). Measured on the corpus
slide: **zero** shading dictionaries on its side, **91602 bytes** of page-one content stream
against our **2570**. So an operator-for-operator comparison of a gradient is not a question that
can be asked, and the file compares pictures instead — twice, once on what we rasterise and once
on what a third-party rasteriser makes of our PDF.

### The numbers, at 150 dpi, per channel

| Comparison | Page 1 (three gradients, one tiled fill) | Page 2 (one picture) |
|---|---|---|
| Our raster against LibreOffice's rendering | mae **0.0016**, ink ratio 1.003 | mae **0.0018**, ink ratio 1.000 |
| Our PDF against its PDF, one rasteriser reading both | mae **0.0007** | mae **0.0000** — identical, pixel for pixel |

The second row is the sharper measurement and worth making separately: the same rasteriser reads
both files, so the antialiasing and image-filtering differences cancel and what is left is the
drawing. `PaintFillComparisonTests` holds the first row under 0.005 and the second under 0.002.

A fading gradient has no LibreOffice reference in the corpus, so it is measured against *itself*
across the two backends: a red-to-transparent ramp over 24 cm, our PDF rasterised by poppler
against our own raster of the same fill, **mae 0.0003**, ink ratio 0.999.

### Two format mappings the readers will need, measured rather than assumed

Neither belongs in this library — a backend takes `GradientPaint.Start` as the centre and stop 0
as the colour there — but both were found here, by building the display list that makes our
picture agree with LibreOffice's, and both are invisible until the colours are compared.

- **ODF's `draw:start-color` on a radial gradient paints the outer edge, not the centre.** A
  `#00c0c0`-to-`#101010` radial renders with a black middle. The reader has to swap the ends.
- **A radial gradient's outer radius is half the shape's diagonal, not half its width.**
  `Gradient::GetBoundRect` builds a square of side `hypot(w, h)` for `GradientStyle_RADIAL`
  (`vcl/source/gdi/gradient.cxx:246-251`); `ELLIPTICAL` instead scales each axis by √2. Using
  half the width moved the mean absolute error on that page from 0.0016 to **0.0054**, which is
  what measuring rather than assuming this was worth.

### What is left, and why

- [x] **A focal radial gradient**, closed when SVG's `fx`/`fy` and EMF+'s path gradient wanted
      Core changes at the same time. `GradientPaint.Focus` is the inner circle's centre; PDF
      writes the two circles it always could and Skia takes a two-point conical shader. Both are
      exact. DrawingML's `a:fillToRect` now has somewhere to go when a reader reads one.
- [ ] **`GradientKind.Conical` and `GradientKind.Rectangular` are bands, not shaders.** Skia's
      sweep gradient would draw a conical one natively and PDF has nothing for either, and using
      the shader in one backend and bands in the other would make a shape's colours depend on
      the output format. So both use the shared decomposition, at LibreOffice's own step count
      (`Gradient::GetMetafileSteps`, `gradient.cxx:336`) — the shorter side in 1/100 mm, clamped
      by the largest channel difference. Nothing emits either kind yet; LibreOffice's own
      `awt::GradientStyle` has no conical at all, so it can only arrive from EMF+ or SVG.
- [ ] **A gradient *stroke* is drawn as its middle stop, a mesh stroke as the mean of its
      vertices, and a bitmap stroke as nothing.** There is no gradient pen operator in PDF and
      LibreOffice's writer has none either. A glyph run is the same case: text is shown in the
      current fill colour.
- [ ] **A mesh's triangles are painted in the order stated and may overlap.** Nothing sorts
      them, nothing merges them, and neither backend is asked to: a fan built from a concave
      boundary is neither convex nor consistently wound, and a mesh that overlaps itself is a
      mesh whose later triangles win. That is what both a type 4 shading and `SkVertices` do, so
      the agreement is free — but a producer that expected blending would get painting.
- [ ] **A tiled fill stops at 8192 tiles.** A one-point tile over an A4 page is half a million
      image draws. At the cap the grid is drawn as far as it goes and the rest is left unpainted,
      which is visible and therefore reportable, unlike stretching the tile.
- [ ] **Nothing in any reader emits a `GradientPaint`, a `BitmapPaint` or a `RasterImage` yet.**
      The decoder is public in this library and `Paperless.WordProcessing`,
      `Paperless.Spreadsheets` and `Paperless.Presentations` have no `ProjectReference` on
      `Paperless.Rendering` — one line each, and the reason a `p:pic`, a `w:drawing` and a
      sheet's logo are still unpainted.

### Two measurements the raster backend rests on

**Glyphs are drawn from their outlines, not through Skia's glyph cache.** The cache
rasterises a mask per glyph and places it at a **whole pixel vertically** — `SKFont.Subpixel`
quantises the horizontal position and nothing quantises the vertical. On `prose-odt.odt` at
150 dpi that moved the page's ink centroid down by 0.56 px, a quarter of a point, and made
`compare-images.py` report a reflow cascade on a page whose layout was exact. Drawing outlines
brings it to 0.087 px. `RasterRenderOptions.GlyphOutlines` turns it off for speed when the
output is a thumbnail. This is not "text as outlines" in the sense the PDF backend forbids:
the display list still carries the glyph ids and the text, and only the pixels are produced
differently.

**The pixel size is the ceiling of the page size, not the nearest whole pixel.** poppler takes
the ceiling: an A4 page 595.304 pt wide is 1240.22 px at 150 dpi and `pdftoppm -r 150`
produces 1241, where rounding to nearest gives 1240. `compare-images.py` stops at
`DIMENSION MISMATCH` before measuring anything, so one pixel costs the whole comparison. Both
rules agree on the height — 841.89 pt is 1753.94 px and both give 1754 — which is why this was
only visible on one axis.

## SVG writer

- [ ] Paths, gradients, clip paths
- [ ] Text as `<text>` where faithful, with a fallback to outlines when shaping cannot be
      expressed
- [ ] Embed images as data URIs; never emit external references

## Font embedding is a contract with the callers, and it went unmet for two families

Worth reading before building anything that produces a `FontReference`, because for a long time
this backend held up its end and nothing else did.

**The key is a path.** `SystemFontResolver` answers a `FontRequest` with a `FontReference` whose
`FaceKey` is `InstalledFace.FaceKey` — the font file, plus `#n` for a face inside a collection
(`Fonts/SystemFontResolver.cs:22`). `FileFontProvider` opens exactly that, `PdfFontCatalogue`
subsets what it opened and writes it as a `/FontFile2` on the descriptor. Give it anything else
and the chain stops at the first link: no bytes, no descriptor entry, and a PDF that *names* a
face it does not carry.

**Why the callers could not simply build one.** The layouters hold an `OpenTypeFace`, which is a
parsed table directory. It knows its family, its weight and its slant and it has no memory
whatever of the file it was read out of — so `new FontReference { FaceKey = face.FamilyName }`
was not laziness, it was the only key those three helpers could produce from what they were
handed. The reference has to be *carried* from the resolution, and each family now does:

| Family | Where the key was lost | Where it comes from now |
|---|---|---|
| Presentations | `SlideTextLayout.Reference(face)`, on every run and every outline bullet | `SlideFonts.Resolve` already returned `(Face, Reference)` and the reference was discarded. It now travels on `RunStyle`, beside the colour, which is where this file already puts what changes the drawing and not the measurement. |
| Spreadsheets | `SheetBandText.Describe()`, on every header, footer and chart label | `Load()` returns the face *and* the reference it resolved through, as one `Lazy`. Cell text was never affected: `SheetFace` had carried the reference all along. |
| Word processing | The list label in the DOCX, DOC and RTF readers | `PageLabel.Font`, which already existed and which only the ODT reader filled in. The body text was never affected. |

**What it looked like.** `pdffonts` on our own output, `deck-features.pptx`:

```
AAAAAA+LiberationSans  TrueType  WinAnsi  emb no     ← before
BAAAAA+OpenSymbol      TrueType  WinAnsi  emb no
```
```
AAAAAA+LiberationSans  TrueType  WinAnsi  emb yes    ← after
BAAAAA+OpenSymbol      TrueType  WinAnsi  emb yes
CAAAAA+LiberationSans  TrueType  WinAnsi  emb yes
DAAAAA+LiberationSans  TrueType  WinAnsi  emb yes
```

Across the corpus: **41 unembedded faces in 33 files, now 0**, with every page count and every
`pdftotext` word count byte-for-byte unchanged — because the text was always right. Rasterised
by poppler, the *before* PDF of that deck is a page of empty boxes; the *after* is
indistinguishable from `soffice --convert-to pdf` of the same file.

**Three faces where there had been one, and that is a second bug going with it.** The deck now
writes three Liberation Sans subsets. Before, `FaceKey` was the family name, so regular, bold and
italic — which live in *different files* — collapsed onto one catalogue entry and every one of
them was drawn with the first face's glyph indices. Nothing noticed, because the face was not
embedded and the reader was substituting its own anyway. Keying on the path separates them by
construction.

**The instrument that was missing, and it is the durable part of this.**
`tests/Paperless.Rendering.Tests/PdfFontEmbeddingTests.cs` renders a corpus document per family
*and per reader*, walks every `/Type/Font` object in the bytes to its `/FontDescriptor`, and
asserts the descriptor carries a `/FontFile2`. It fails 16 of its 18 cases against the code as it
stood — the two that pass are the ODT control, which was already right. Three things about its
shape are deliberate:

- **Descriptor by descriptor, not a count of embedded programs.** A count would pass a file that
  embedded one face twice and another not at all, and would need revising every time a corpus
  document's faces changed.
- **Per reader, not per family.** DOC, DOCX and RTF each build their list labels in their own
  reader; ODT built its correctly and the other three did not. A per-family sweep would have
  called word processing clean.
- **Rendered into memory, never to a file.** `word-features.doc` and `word-features.docx` both
  render to `word-features.pdf`; a test that never names a file cannot silently measure one of
  them twice. That trap has cost three agents an hour each and is warned about at the top of
  `tests/corpus/render-sweep.txt` as well.

**The general shape of the defect is worth remembering more than the defect.** Every check
pointed at a rendered document asked *where the ink went* — page counts, pen positions, glyph
counts, extracted words. A face that is referenced and not embedded moves none of them. When a
backend's output has a property that no existing measurement is a function of, that property will
be wrong and will stay wrong; the fix is an assertion about the property, not a sharper version of
the measurements.

## Known deviations, measured

Each is a place our output differs from LibreOffice's on purpose, with the evidence.

- **`/Widths` are real numbers to four decimals where LibreOffice writes integers.** Carlito's
  `a` is 1084 units on a 2048 grid, which is 529.2969 thousandths of an em and 529 rounded.
  The tenth lost per glyph is 0.004 pt at eleven point, so a sixty-glyph line ends a quarter
  of a point short of where layout put it — over the tenth-of-a-point bound everything else in
  this project is held to. Real widths cost four characters each and remove the drift instead
  of correcting it. `PdfContentSink` still corrects the pen with a `TJ` array whenever it has
  drifted more than 0.0025 pt, which in practice means justified lines and nothing else: an
  unjustified line is one `Tj`, the same shape LibreOffice writes.
- **`GSUB`, `GPOS` and `GDEF` are dropped from every embedded subset.** They describe
  substitutions and positioning that shaping has already applied, so carrying them into the
  PDF embeds rules nothing will run. Measured on Carlito with five glyphs retained: 4964 bytes
  with them, 2664 without.
- **No `/Artifact` marked content, no `/StructElem` tree, no page-background clip.**
  LibreOffice opens every page with `q 0 0.028 595.275 841.861 re W* n` and brackets each
  drawing in `BMC`/`EMC`. None of it is drawing and none of it is read by any comparison.
- **A gradient is one shading where LibreOffice writes hundreds of flat polygons.** Impress
  decomposes every shape gradient before its PDF writer sees it (tdf#150551, cited above), so its
  page-one content stream for `paint-fills.fodp` is 91602 bytes and ours 2570. Ours is smooth,
  resolution-independent and a twentieth of the size; theirs bands at the step count
  `Gradient::GetMetafileSteps` gives. Rasterised at 150 dpi the two agree to a mean absolute
  difference of 0.0007 per channel, so the banding is below one level of an eight-bit channel at
  that resolution and the deviation costs nothing visible. It does mean `PdfFills` reports one
  rectangle for a gradient on our side and hundreds on LibreOffice's, which is why `PdfPaints`
  exists.
- **A magnified picture is interpolated and poppler's rasterisation of the same PDF is not.**
  A 16×12 picture drawn at 8×6 cm comes back from `pdftoppm` as hard blocks and from our raster
  backend as a smooth ramp — mean absolute difference 0.0018 over the page, 0.0779 in the worst
  32×32 tile, all of it on the picture's border. **The deviation is poppler's, not
  LibreOffice's**: neither writer sets `/Interpolate` on the image XObject, so the choice is the
  rasteriser's, and LibreOffice's own renderer interpolates exactly as Skia does — checked by
  exporting the same slide through `impress_png_Export`, which produces a smooth ramp. Our PDF
  rasterised by poppler is pixel-for-pixel identical to LibreOffice's PDF rasterised by poppler,
  which is what settles the attribution.
- **`/Widths` are taken off the display list when a face's file cannot be read.** A
  `FontReference` whose `FaceKey` names a family rather than a path never loads, and the
  alternative is an array of zeros — which places every glyph a whole advance from where the
  stated widths put the pen, so each one takes a `TJ` correction of the entire advance.
  Measured on `sheet-print-xlsx.xlsx`: adjustments of −722 and −556 thousandths between adjacent
  glyphs, and `pdftotext` reporting **13255** words over fourteen pages against LibreOffice's
  **2281**, one per character, because an adjustment that large is how a PDF spells a word break.
  With the run's own advances the same file extracts as 2281. This is a fallback and not the fix:
  the face is still not embedded, and the callers that build the reference this way —
  `SheetText.Describe`, `SlideTextLayout.Reference`, `PageDrawing.Reference` — should carry the
  resolver's own key. **They now do; see "Font embedding is a contract with the callers" below.**
  The fallback stays, because a caller driving the display list by hand still has no key to give.
- **A blank at the end of a wrapped line is not drawn.** LibreOffice draws it as a run of its
  own — eleven extra one-glyph runs on `paginated.fodt`, each at the right-hand end of a line
  it has already drawn. The glyph occupies the margin and marks nothing.

## Findings for other libraries

The first two surfaced only because the PDF comparison reads the content stream rather than word
boxes, and neither is this library's: the layout is `Paperless.WordProcessing`'s.

- **A 16 pt heading's baseline sat 1.95 pt higher than LibreOffice's**, in all four formats.
  Fixed there: the leading proportional line spacing adds above a paragraph's first line belongs
  to the paragraph *above* it. The rule, its citations and the before-and-after numbers are in
  `src/Paperless.WordProcessing/TODO.md`.
- **`footnotes.rtf` draws its note separator 1.286 pt above LibreOffice's** — 757.465 pt down the
  page against 758.751. Settled, and it is LibreOffice's rather than ours: its RTF import loses
  the face the note states and sets the notes in Liberation Serif, which is 13 twips a line
  shorter than the Carlito the file names, so its bottom-aligned note area — and the rule above
  it — starts two lines' worth higher. The evidence and the bisection are in
  `src/Paperless.WordProcessing/TODO.md`; `NoteSeparatorComparisonTests` pins the attribution and
  `PdfOutputComparisonTests` still leaves that one file out of the fill comparison.
- ~~**Three layouters throw the resolver's face key away**~~ — **settled**, in all three families
  plus the three word-processing readers whose list labels had the same hole. Diagnosed here and
  fixed by the callers, which is the only place it could be fixed: this library recovered the
  *metrics* from the run's own advances and could never have recovered the *bytes*. The whole
  account, the before-and-after `pdffonts` and the test that now holds it are under "Font
  embedding is a contract with the callers" above.
- **`Paperless.Vector` was going to need `RasterImageDecoder` and does not, and the reason is
  worth keeping.** The one case standing between that library and a codec dependency was EMF+
  image attributes, recorded there as "a colour matrix, a gamma, a chroma key and a colour remap
  table". Those are fields of GDI+'s `ImageAttributes` **API class**; the object a metafile
  actually serialises is [MS-EMFPLUS] 2.2.1.5, which is twenty-four bytes of wrap mode, clamp
  colour and object clamp — the colour adjustments are applied by the producer before the bitmap
  is written. So nothing there ever needed pixels, no `ProjectReference` was added, and the rule
  that a reader must not pay for a rasteriser holds without an exception. **The general form:
  when a gap is described in terms of an API's capabilities, check what the file format writes
  down before sizing the work.**
- **Two ODF gradient mappings, for whoever reads `draw:gradient`.** Its `draw:start-color` on a
  `radial` gradient paints the *outer* edge, and the radial's outer radius is half the shape's
  **diagonal** — `Gradient::GetBoundRect` builds a square of side `hypot(w, h)`
  (`vcl/source/gdi/gradient.cxx:246-251`), where `ELLIPTICAL` scales each axis by √2 instead.
  Both are invisible until the colours are compared: taking half the width instead moved the
  mean absolute error on the corpus slide's radial page from 0.0016 to 0.0054.

## Open questions

- [ ] Is a Skia-independent "reference" software rasteriser worth having, to keep golden
      images stable across SkiaSharp upgrades? A Skia version bump can change antialiasing,
      and the measured gap between Skia and FreeType is already the dominant term in every
      image comparison here: on `prose-odt.odt` at 150 dpi the mean absolute error against
      poppler is 0.0019 and the ink *mass* ratio 0.994, while a naive count of dark pixels
      differs by 30% on a page of 0.5 pt table borders because Skia spreads a 1.04 px line
      across two rows and poppler snaps it to one.
- [x] Should the PDF writer support incremental page writing for large documents, or is
      buffering acceptable? Buffering, for now, and the cost is written down above.
- [ ] `RasterComparer` in the TestKit is still a stub. `RasterOutputComparisonTests` does its
      own comparison because the two backends' test projects are the only callers so far;
      implementing it properly would give every family's rendering tests the same measures.
