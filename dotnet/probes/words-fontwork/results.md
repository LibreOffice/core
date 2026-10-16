# WordArt text warp — `a:prstTxWarp` / Fontwork

Round `agent/fontwork`, base `b9bb9243b`. Environment: this container, `soffice` = **24.2.7.2**,
`/opt/libreoffice26.2/program/soffice` = **26.2.4.2**, Liberation/Carlito/Caladea installed,
`fc-match "DejaVu Sans"` resolving to DejaVu. Every figure below was taken here; the two
references agree with each other on the catalogue to 0.03 mean over its 52 pages, so nothing in
it is a version artefact.

## Instrument

`ink.py` in this directory: `pdftoppm -r 100 -gray`, per page, mean absolute difference of the
two grey rasters, plus each side's own ink as `(255 - grey).mean() / 255`. It reproduces the
briefed "before" column exactly — 7.31 / 19.83 / 18.41 / 16.93 / 15.66 against the brief's
7.29 / 19.79 / 18.37 / 16.90 / 15.63 — so the after column below is comparable with it.

## `words/drawingset-001/docx/WordArt_Shapes_Arrows_Catalog1.docx`

24 warped shapes, one per preset, on pages 17-21 of 52. Page count 52/52 throughout.

| | pages 17 | 18 | 19 | 20 | 21 | document mean, 52 pages |
|---|---:|---:|---:|---:|---:|---:|
| before, against 26.2.4.2 | 7.31 | 19.83 | 18.41 | 16.93 | 15.66 | **3.748** |
| after, against 26.2.4.2 | **0.47** | **0.55** | **0.50** | **0.48** | **0.39** | **2.292** |
| before, against 24.2.7.2 | 7.31 | 19.83 | 18.41 | 16.94 | 15.66 | — |
| after, against 24.2.7.2 | **0.39** | **0.52** | **0.51** | **0.52** | **0.37** | — |

Every other page of the document is between 0.38 and 7.72 and none of them moved; the five
warped pages are now the *least* divergent pages in the file.

Our own ink against the reference's, at 100 dpi:

| page | 17 | 18 | 19 | 20 | 21 |
|---|---:|---:|---:|---:|---:|
| ours before | 0.66 | 0.63 | 0.64 | 0.62 | 0.50 |
| ours after | 2.95 | 7.80 | 7.27 | 6.60 | 6.08 |
| reference | 2.91 | 7.71 | 7.20 | 6.53 | 6.01 |

Words, `pdftotext | wc -w` over the whole document: **before 2492, after 2468, reference 2468**
— exactly 24 fewer, one per warped shape, landing on the reference.

### The 10.8 pt that was not the warp

The first correct-geometry cut still scored 20.4 mean on pages 17-21 while its *ink quantity*
matched the reference to 0.09. Measured at 200 dpi, the whole warped band was a pure translation
away: ours 121.32..468.72 pt across and 84.96..158.04 down page 18, the reference's
132.12..479.52 and 95.76..168.84. That is (+10.8, +10.8) pt, and `wp:effectExtent` on all 24
shapes is 137160 EMU = 10.8 pt.

An as-character drawing's line box is grown by the effect extent on all four sides and the two
halves of LibreOffice then disagree about where the object sits inside it — a draw shape's fill
and outline at the outer corner *plus* the extent, a `wps:txbx` shape's text at the outer corner
regardless (`probes/words-inline-effectextent/`). A warped body is the case where the second
rule does not apply, because the importer clears `TextBox` and there is no text box left. Moving
the curves by the extent puts our rectangle on the reference's to the pixel at 200 dpi.

**The horizontal half of that looks like a defect on the text side as well, and was left alone.**
The same document's *unwarped* gradient-text boxes on page 3 sit at x 229.68..359.64 for us and
240.84..370.80 for the reference — the same 10.8 pt, with y matching exactly. That reaches every
inline drawing that declares an effect extent, which is most of the corpus, so it is not a change
to make on the strength of one document.

## `slides/extra-001/pptx/FAAAIandtheArtandScienceofV&Vfinal.pptx`

Eight `textArchUp`/`textArchDown` labels round a dial, on pages 13 and 14. All four shapes state
`<a:noFill/>` and a white `a:solidFill` on the run, so the fill has to come from the character
properties or nothing is drawn at all.

| page | before | after | ours after | reference |
|---|---:|---:|---:|---:|
| 13 | 3.05 | **2.18** | 19.34 | 19.34 |
| 14 | 2.16 | **1.29** | 15.62 | 15.60 |

Words unchanged at 1143 against the reference's 1141: the slides side already drew nothing for a
warped body, so no word moved.

One of the four labels is two lines (`Automation` / `Autonomy`, split by an `a:br`), which is the
only corpus use of the parallel-rail arm of `FitTextOutlinesToShapeOutlines`.

## `slides/done-009/pptx/redac-sas-201403-ppt-portfolio-rev-sim.pptx`

Two arches on page 6, three `textPlain` on page 7.

| page | before | after |
|---|---:|---:|
| 6 | 5.83 | **5.81** |
| 7 | 3.73 | **3.47** |

## The other four `docx` that state a warp

Corpus-wide census of `prstTxWarp` in `docx`: 2103 `textNoShape`, 5 `textPlain`, and the
catalogue's 24. The five `textPlain` are on four documents.

| document | reference words | before | after |
|---|---:|---:|---:|
| `words/done-014/…/exhibit-06---technical-architecture-template.docx` | 1089 | 1102 | **1096** |
| `words/chartset-005/…/052_Organogram_Template_Colorful_Flow_Chart…docx` | 42 | 42 | 42 |
| `words/chartset-005/…/054_Organogram_Template_Grey_Vertical_Theme…docx` | 36 | 36 | 36 |
| `words/extra-001/…/ABCD-SDE-23-00 - Avionic System Description…docx` | — | 8431 | 8431 |

`exhibit-06` improved on both columns — mean ink over its 8 pages 15.346 → 14.820 — and its two
`textPlain` shapes are a 412 x 247 pt diagonal `DRAFT` watermark in a header.

**The two organograms are why group members are excluded, and they cost two words each before
that.** Their warped shape sits inside a `wpg:wgp`, and the reference keeps its text as text:
`WpsContext::onEndElement` converts at the end of `wps:bodyPr`, and a group member is not yet an
`SdrObjCustomShape` at that point, so its very first guard fails. `pdftotext` reads
`Organogram Template` from the reference and from us. Warping a group member made us the only
side without those words.

`ABCD-SDE-23-00` is unchanged; its reference does not render here at all, which predates this
round.
