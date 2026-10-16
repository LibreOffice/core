# Round 110 — O31/N29: there is no vector 3-D chart in the corpus at all

**Reference throughout:** `/opt/libreoffice26.2/program/soffice` = 26.2.4.2.

## The question this settles

`chart-smooth-r102` §2 censused the six corpus documents holding a 3-D chart and found five
rendered as a raster over the plot area and one — the 3-D pie inside a `.ppt` OLE
`Excel.Sheet.8`, on `slides/done-004/ppt/undp_presentation_revised_17_may.ppt` — rendered as
**vectors, 362 and 309 path items**. It recorded two consequences: that an implementation of
3-D chart geometry would have exactly **one of 947** documents to be measured against, and
that *"why the reference rasterises a scene reached through Writer and Calc and not one
reached through Impress is not established"*.

Both are wrong, and the second question does not exist.

## Leg 1 — source: a live 3-D scene has no vector path

`drawinglayer/source/primitive2d/sceneprimitive2d.cxx`:219-548,
`ScenePrimitive2D::create2DDecomposition`. After the optional 2-D shadow primitives it
computes a discrete size, caps its area at
`officecfg::Office::Common::Drawinglayer::Quadratic3DRenderLimit` (:253-260, which is why the
banked raster sizes are odd numbers like 1370x732 and not a device dpi), runs
`aZBufferProcessor3D.process(getChildren3D())`, converts the pixel raster with
`BPixelRasterToBitmap`, and pushes a **`BitmapPrimitive2D`** (:530-534). Every return path
ends in that bitmap or in an empty group; **there is no branch that emits 2-D geometry.**

`ScenePrimitive2D::getGeometry2D()` (:550) does exist and does return vectors — and its only
caller in the tree is `drawinglayer/source/processor2d/contourextractor2d.cxx`:139, a contour
extractor. No `vclprocessor2d`, `vclmetafileprocessor2d` or `vclpixelprocessor2d` handles
`PRIMITIVE2D_ID_SCENEPRIMITIVE2D` at all, so a scene reaches any device — screen, metafile,
PDF — only through the decomposition above.

So the stream a scene is reached through cannot matter. Writer, Calc and Impress share one
drawinglayer.

## Leg 2 — the binary: the same chart, one variable

`undp_presentation_revised_17_may.ppt` page 19 holds two live 3-D pies. `--convert-to fodp`
resolves them to real `chart:chart chart:class="chart:circle"` with
`chart:three-dimensional="true"` and a `dr3d:transform` matrix on the plot area — the same
shape of model the DOCX witness carries (`021_Unit_Circle_Chart_3D_Pie_Chart` resolves to
`chart:circle` + `three-dimensional="true"` + `dr3d:transform` too, and both documents state
the same fifteen `dr3d:` attributes). The models are not the difference.

Rendering the **same binary** twice, changing only whether the object arrives with its stored
OLE preview:

| leg | what page 19 carries in the chart frame |
|---|---|
| `.ppt` rendered directly | vector paths, **362** and **309** items, (53.3, 359.8)-(234.8, 446.3) |
| the reference's own flat ODP, re-rendered | a **635 x 155 raster** at (40.1, 344.9)-(242.3, 397.9) — and **no paths** |

`page19.tsv`. The flat ODP does carry replacement pictures in general — 30 `draw:image`
elements — but not for this object, so the round trip forces the chart to be drawn live, and
drawn live it is a bitmap.

## What this means

The 362/309 paths are **PowerPoint's stored replacement metafile for the `Excel.Sheet.8`
OLE** — Excel's own drawing of the pie, drawn as a picture. They are not 26.2.4.2 drawing a
3-D chart, and nothing about them can be a target for chart geometry.

- **26.2.4.2 never draws a live 3-D chart scene as vectors, in any stream.** Confirmed in
  source and against the binary.
- The corpus therefore holds **0 of 947** documents against which 3-D chart geometry could be
  measured, not 1. O31's "one document is measurable" is withdrawn.
- N29's open question — why Impress would differ — dissolves: it does not differ, and the
  document that suggested it does was never rendering a chart scene.

`021_Unit_Circle_Chart_3D_Pie_Chart`'s 3.47 `|ink|%` is unchanged and still unreachable: no
vector geometry can match a Z-buffer raster, and there is now no witness anywhere in the
corpus that would tell us whether an implementation was right. This is a scope decision with
nothing left behind it.

## Commands

```
soffice --headless --convert-to fodp   undp_presentation_revised_17_may.ppt
soffice --headless --convert-to fodt   021_Unit_Circle_Chart_3D_Pie_Chart_404247ab.docx
soffice --headless --convert-to pdf    undp_presentation_revised_17_may.ppt
soffice --headless --convert-to pdf:impress_pdf_Export  undp_presentation_revised_17_may.fodp
```
paths read out of each PDF with PyMuPDF `get_drawings()` / `get_image_info()`.
