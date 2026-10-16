# ODF fontwork — the census, the verdict, and the half that was worth building

Round `agent/fontwork2`. Environment: this container, `soffice` = 24.2.7.2,
`/opt/libreoffice26.2/program/soffice` = 26.2.4.2.

## The census, which is stronger than the brief's

The brief reported no `draw:text-path="true"` and no `draw:type="fontwork-*"` in the corpus's ODF
files. Re-run here by extension over the whole tree, the true statement is one step stronger:

| extension | files |
|---|---:|
| `docx` | 272 |
| `pptx` | 251 |
| `xlsx` | 241 |
| `doc` | 66 |
| `xls` | 64 |
| `ppt` | 51 |
| **any ODF form** (`odt odp ods fodt fodp fods sxw sxi sxc ott otp ots odg otg`) | **0** |

**The corpus holds no OpenDocument file of any kind.** So ODF fontwork does not have "zero measured
reach" in the sense of a feature nothing happens to use — the gate structurally cannot see any ODF
path at all, and no amount of corpus work will change that. Anything measured here is measured
against authored or converted evidence, and is worth exactly what that evidence is worth.

## What the evidence showed, on both families

Two documents, both produced by LibreOffice 26.2.4.2 from OOXML originals, so their markup is
genuine ODF rather than hand-built.

### Impress: the text was drawn, unwarped, and that is worse than not drawing it

`FAAAIandtheArtandScienceofV&Vfinal.pptx` converted to `.odp` carries **8 `draw:text-path="true"`
shapes** on slides 13 and 14 — the four dial labels twice, `fontwork-arch-up-curve` and
`fontwork-arch-down-curve`, with `draw:text-path-scale="shape"` and `draw:modifiers` in WordArt
units. Before this round X-Ray.Office read `draw:enhanced-geometry` for the shape but knew nothing
about text-path mode, so it laid the labels out as ordinary text: overlapping, unwarped, and
tokenised per glyph by the rotation.

| | slide 13 | slide 14 | whole deck, 30 pages | words |
|---|---:|---:|---:|---:|
| before | 8.63 | 7.32 | 8.699 | 1289 |
| after | **5.06** | **3.74** | **8.461** | **1213** |
| reference (26.2.4.2) | — | — | — | 1219 |

100 dpi mean absolute grey difference against 26.2.4.2. A paired reading of the dial confirms the
shape: the labels now run round the arcs as filled outlines where they were flat overlapping text.

**One residual worth stating rather than smoothing over.** The reference's own ODP text layer still
holds three of the four labels as split fragments — `A|na|ly|si|s`, `Au|gm|en|ta|tio|n`,
`ta|s|i|s|As` — which is why we now extract 1213 against its 1219 rather than landing on it. Before
the change we were 70 *over*; we are now 6 under. The rendered page shows each label exactly once
on both sides, so those fragments are not a second drawing of them, and both the ink and the
picture say suppressing our copy is right; but the word count does not land exactly and this says
so.

### Writer: the gap is not fontwork, and building fontwork there would be building a leaf

`fontwork-presets-adjusted.odt` — the 41-preset fixture converted by 26.2.4.2 — renders like this:

| | pages | words | mean ink, 7 comparable pages |
|---|---:|---:|---:|
| reference 26.2.4.2 | 9 | 228 | — |
| reference 24.2.7.2 | 9 | 228 | — |
| ours | 7 | 213 | **15.78** |

And the reason is not the warp. `OdfFrames.Read` takes a frame's text only from a
`draw:text-box`, and a `draw:custom-shape` holds its `text:p` children directly; it reads no
geometry for one either. So **every one of the 41 shapes draws nothing at all** — not the unwarped
`textNoShape` one's text, not any shape's fill, not any shape's outline; only a bare border
rectangle on the one shape whose style states one. The captions between them also sit 22 pt closer
together than the reference's, so the inline frames are not reserving the right height either.

Implementing ODF fontwork on the Writer side would put warped outlines into frames that are the
wrong height and that draw nothing else, and no part of the improvement could be attributed to the
warp. **What that side needs first is `draw:custom-shape` in a Writer body: its geometry, its fill,
its outline and its text.** Fontwork is one leaf of that branch and is not the thing to build
first.

There is a second reason to leave it, and it is structural rather than a matter of effort. The
Fontwork model lives in `XRay.Office.Ooxml.DrawingML`, and `XRay.Office.OpenDocument` — the only layer
both Writer and Impress can reach — sits beside `XRay.Office.Ooxml` rather than above it. So a
*shared* ODF fontwork reader has nowhere to live under the current layering; the one written this
round is in `XRay.Office.Presentations.OpenDocument`, beside the `OdfEnhancedGeometry` it belongs
with, and a Writer implementation would either duplicate it or force that question.

## What the reader reads, and what it does not

`draw:type` is the LibreOffice Fontwork type itself — `fontwork-arch-up-curve`, `mso-spt157` —
which is exactly what `FontworkPresets` is keyed by, so nothing is mapped. `draw:modifiers` is
already in the 21600 viewbox the tables are written in, so nothing is converted; the DrawingML side
needs `fontworkhelpers.cxx:95-150` for both of those. `draw:text-path-scale` is `TextPathScaleX`
outright.

**And it corroborates the previous round's four-preset list independently.** LibreOffice writes
`draw:text-path-scale="shape"` on exactly four of the forty — `fontwork-arch-up-curve`,
`fontwork-arch-down-curve`, `fontwork-circle-curve`, `fontwork-open-circle-curve` — which is the
same set `fontworkhelpers.cxx:173-179` derives it for and the same set the previous round found
keeps its stated font size.

Two of the four `TextPath` members are not read:

- **`draw:text-path-mode`** decides `NORMAL` / `PATH` / `SHAPE`, and
  `EnhancedCustomShapeFontWork` never consults it — the fit is decided by how many rails the
  geometry makes and by `ScaleX`.
- **`draw:text-path-same-letter-heights`** *is* honoured, at
  `EnhancedCustomShapeFontWork.cxx:488`, and is not implemented. LibreOffice writes it only when
  true, and nothing this project has measured sets it: the corpus holds no ODF file, and all five
  of its binary Escher WordArt shapes leave bit `0x80` of `DFF_Prop_gtextFStrikethrough` clear.
