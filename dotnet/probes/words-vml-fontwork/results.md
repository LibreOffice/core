# VML WordArt — `v:textpath` on a `#_x0000_t136`

Round `agent/fontwork2`, base `9ca78d7e1`. Environment: this container, `soffice` = **24.2.7.2**,
`/opt/libreoffice26.2/program/soffice` = **26.2.4.2**, Liberation/Carlito/Caladea installed.
Every figure below was taken here, against both binaries.

## What the corpus holds

Censused over every OOXML container in `/home/user/sample-files` by counting
`type="#_x0000_t136"` references rather than the string `_x0000_t136`, which also matches the
`v:shapetype` definition Word writes ahead of each one:

| document | shapes | where |
|---|---:|---|
| `words/extra-001/…/ABCD-SDE-23-00 - Avionic System Description…docx` | 4 | `header1..4` |
| `words/ceiling-001/…/ABCD-FE-01-00 Flight Envelope…docx` | 3 | `header1..3` |
| `words/extra-001/…/ABCD-WB-08-00 Weight and Balance Report…docx` | 3 | `header1..3` |
| `words/done-015/…/DOA_Template_Form_Type_Certification_Programme.docx` | 3 | `header1..3` |
| `words/done-012/…/technical-architecture.docx` | 2 | `header1..2` |
| **total** | **15** | every one in a header, none in an `mc:Fallback` |

Fourteen carry a `v:textpath` with a string; `DOA_Template`'s `header3` is a 317.5 pt square
`#_x0000_t136` with a `fillcolor` and **no `v:textpath` at all**, which the reference also draws
nothing for.

`WordArt_Shapes_Arrows_Catalog1.docx` holds **zero** `_x0000_t136` and 77 `v:textpath` hits, every
one of them inside the `mc:Fallback` of a DrawingML shape. `OoxmlXml.ResolveAlternateContent`
picks the `mc:Choice` at load, so none of them reaches this reader — the catalogue is unchanged at
52 pages and 2468 raw extracted words.

## The three things the VML path does that DrawingML does not

1. **The text is an attribute.** `v:textpath/@string`, with the face and size as CSS in `@style`.
2. **The declared height is discarded and remeasured** (`oox/source/vml/vmlformatting.cxx:1041-1056`).
3. **`ScaleX` and `SameLetterHeights` are hard-coded `false`** (`vmlformatting.cxx:966-975`),
   whatever the shape type.

### The height, measured

`TextpathModel::pushToPropMap` measures the string in the stated family at 96 units on a
`VirtualDevice` and sets the height to `textHeight / textWidth × shapeWidth`. Only the ratio
survives, so this reproduces it from the face's own design metrics: `hhea`'s ascender less its
descender over the sum of the advances.

Probed on five (family, string) pairs, each isolated in an otherwise empty one-page document so
the watermark's ink box is the only ink on the page (`makeprobe.py`), measured at 300 dpi:

| family → face | string | declared w | reference h | ours | error |
|---|---|---:|---:|---:|---:|
| Arial → Liberation Sans | `EASA example document` | 583.25 | 57.60 | 57.36 | **−0.42%** |
| Calibri → Carlito | `EASA Example Documents` | 556.75 | 64.08 | 64.08 | **0.00%** |
| Calibri → Carlito | `DRAFT` | 412.40 | 186.96 | 187.44 | **+0.26%** |
| Arial → Liberation Sans | `DRAFT` | 412.40 | 138.00 | 138.48 | **+0.35%** |
| Times New Roman → Liberation Serif | `EASA Example Documents` | 556.75 | 55.44 | 54.96 | **−0.87%** |

Worst case 0.9%, which on a 57 pt band is 0.5 pt; the probe's own edge measurement at 300 dpi is
0.24 pt of that. The residual is VCL grid-fitting the outline at 96 ppem where this reads the
unhinted design advance — the same divergence `dotnet/CLAUDE.md` records for text advances.

**`trim` has no default and reading it as though it had is the whole failure mode.**
`lclDecodeBool` yields nothing for an absent attribute and line 1041 tests
`moTrim.has_value() && moTrim.value()`, so an unstated `trim` resizes. The first cut of this read
it the other way, left every watermark at its declared height, and scored *worse* than drawing
nothing: `DOA_Template`'s 53 pt against the reference's 57.5, and `technical-architecture`'s
247.45 pt against 138.

### `gtextFSameHeights` and `gtextFStretch` are not reachable from here

The brief expected this path to expose them. It does not:
`oox/source/vml/vmlformatting.cxx:966-975` writes `ScaleX` and `SameLetterHeights` as literal
`false` into every VML text path's `CustomShapeGeometry`, and nothing later overrides them. The
fixture's `V005 same letter heights` case (`v-same-letter-heights:t` in the textpath style)
confirms it: the reference draws it identically to `V001`.

They are reachable only from **binary Escher**, `filter/source/msfilter/msdffimp.cxx:2516-2600`,
which reads `DFF_Prop_gtextFStrikethrough`'s bits and `IsHardAttribute(DFF_Prop_gtextFStretch)`.
That is the `.doc`/`.ppt` path, not this one.

## Corpus measurement, both references

100 dpi mean absolute grey difference, whole document, and `pdftotext` word counts per page.
`measure.py` in this directory.

| document | pages 24/26/ours | mean ink24 before → after | mean ink26 before → after |
|---|---|---|---|
| `ABCD-FE-01-00 Flight Envelope` | 15 / 16 / 14 | 11.698 → **11.341** | 15.210 → **14.855** |
| `ABCD-SDE-23-00 Avionic System Description` | 29 / 29 / 29 | 5.364 → **5.003** | 6.050 → **5.694** |
| `ABCD-WB-08-00 Weight and Balance` | 12 / 12 / 12 | 7.081 → **6.738** | 8.515 → **8.197** |
| `DOA_Template_Form_Type_Certification_Programme` | 20 / 20 / 20 | 11.322 → *11.602* | 10.960 → *11.239* |
| `technical-architecture` | 8 / 8 / 8 | 4.976 → **4.590** | 5.884 → **5.499** |

Four improve and one gets worse. Net over the five, mean of the document means:
**8.088 → 7.855 against 24.2** and **9.324 → 9.097 against 26.2**.

Page counts and per-page word counts are unchanged on all five, and a whole-track sweep confirms
it: `words/*`, 338 documents, **311 MATCH / 27 MISMATCH before and after, and not one row of
`parity.tsv` differs** — no page, word, font or verdict column moved anywhere in the track.

`technical-architecture` is the cleanest single reading, because only its first two headers carry
a watermark: **page 1 2.19 → 0.77 and page 2 2.64 → 0.97 against 24.2**, with pages 3-8 untouched
to the hundredth.

### Why `DOA_Template` gets worse, exactly

The watermark is drawn, at the right size and the right horizontal position, **34.7 pt too high**.
The cause is not the warp; it is what `mso-position-vertical:center` with
`mso-position-vertical-relative:margin` is centred *in*.

Measured on an isolated one-page probe (`rot0.docx`, empty header, A4, `w:top` = `w:header` = 708
twips): the reference centres the shape at y = 409.08 pt and we centre it at 402.48. The
difference is **6.6 pt, which is half of one empty header line** — so LibreOffice's
`RelOrientation::PAGE_PRINT_AREA` starts *below* the header and ours starts at Word's `w:top`
margin. On `DOA_Template`, whose header is a three-row table, that gap is 76.6 pt and half of it
is the 34.7 pt observed.

This reaches every frame anchored `relativeFrom="margin"` vertically in a document with a header
taller than its top margin — DrawingML frames included, not just VML ones — so it is left for a
round that measures it rather than changed on the strength of this one document.

## The authored fixture

`/home/user/fixtures/fontwork-vml-t136.docx`, 12 shapes varying `fitshape`, `fitpath`, `trim`,
`v-same-letter-heights`, `v-text-kern`, `v-text-spacing`, the face, the fill, the stroke and a
315° rotation. Both references agree on it to 0.294 mean per page.

| | page 1 | page 2 |
|---|---:|---:|
| ink against 26.2.4.2 | 30.82 | 10.42 |
| our ink / reference's | 7.61 / 8.99 | 2.26 / 2.72 |

Words and pages match exactly: 2 pages, and the same 2 `WORDART` tokens (both from the inherited
page header) the reference extracts — every one of the 12 warps is outlines on both sides.

**The residual is one thing and it is not the warp.** Read out of the reference's own content
stream, it strokes each glyph contour with `0 w` — PDF's "thinnest line the device can draw" —
because `StrokeModel::pushToPropMap` defaults an unstated `strokeweight` to **1 EMU**. Eleven of
the twelve fixture shapes state no `stroked`, so the reference outlines all of them and we draw
the same stroke at `DocxVmlFrames.Hairline`, 0.1 pt, which is about one device pixel at 300 dpi
and a seventh of one at the 100 dpi this metric uses. Paired renderings at 110 dpi show the
difference as the reference's dark keyline round each letter.

**No corpus document measures it**: all 15 `#_x0000_t136` shapes state `stroked="f"`. Closing it
means a resolution-independent "hairline" in the drawing IR, which is a `XRay.Office.Rendering`
question and reaches every VML border rather than these.
