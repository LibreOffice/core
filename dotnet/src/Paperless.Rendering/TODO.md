# Paperless.Rendering — TODO

Backends consuming `IDrawingSink`: PDF, Skia raster, SVG.

Reference: `research/06-rendering.md` sections A, E and G.

Status: **the PDF writer and the Skia rasteriser both write files**, and the PDF one is
compared against LibreOffice's own PDF operator for operator. SVG is still a stub, and so are
gradients, tiling patterns and vector import in both backends.

## How this library is verified, and why it is verified that way

Worth reading before adding anything here, because it changes what is worth building.

**The fidelity harness reads LibreOffice's PDFs, and nothing in it is specific to
LibreOffice.** `tests/Paperless.TestKit/LibreOffice/PdfTextRuns`, `PdfFills`, `PdfStrokes` and
`PdfPageSizes` were written to pull pen positions, filled rectangles, stroked lines and sheet
sizes out of the reference. Pointed at *our* output they compare two content streams — same
pens, same sizes, same fills, same strokes — which is a far sharper question than an image
diff and a far easier one to attribute when the answer is no.

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
- [ ] **Gradients as shading dictionaries, tiling patterns for bitmap fills.** Not written,
      and deliberately not written blind: nothing in the display list produces either yet —
      word-processing layout emits only `SolidPaint` — so the code would be unexercised and
      unverifiable. A `GradientPaint` currently draws as its middle stop and a `BitmapPaint`
      as nothing (`PdfContentSink.Flatten`). Land these with the first feature that emits one,
      which is shape fills in slide rendering.
- [ ] Soft masks (`ExtGState` `/SMask`) beyond the constant alpha now written. Needed for a
      shape with a gradient transparency, which is the same unblocking as above.
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
- [ ] Solid paints only, as in the PDF backend: gradients map to Skia shaders and are
      written but unexercised, and a tiled `BitmapPaint` draws nothing. Same reason, same
      unblocking.
- [ ] Raster image decode. `DrawImage` draws a decoded `RasterImage`; nothing decodes a
      picture out of a document yet.

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
- **A blank at the end of a wrapped line is not drawn.** LibreOffice draws it as a run of its
  own — eleven extra one-glyph runs on `paginated.fodt`, each at the right-hand end of a line
  it has already drawn. The glyph occupies the margin and marks nothing.

## Findings for other libraries

Both surfaced only because the PDF comparison reads the content stream rather than word boxes,
and neither is this library's: the layout is `Paperless.WordProcessing`'s.

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
