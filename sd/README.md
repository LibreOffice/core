# LibreOffice Impress / Draw Application

The core directory for the impress/draw applications.

Think of impress as a hack on top of draw.

`sd` module contains impress/draw specific code, non-shared UI and part
of `ppt` and `pptx` filter, few other filters too.

the slideshow UI lives here as well, the slideshow engine is in
`slideshow` module though (including the 3D transitions engine
`slideshow/source/engine/opengl`).

the most used filters are ODF's `odp`, binary ppt and OOXML's
`pptx`. their locations are listed below:

 * `odp` import and export filters are in `xmloff` module (mostly `xmloff/source/draw`)

 * `ppt` import is in `sd/source/filter/ppt` (big shared chunks are also in `svx`)
 * `ppt` export is in `sd/source/filter/eppt` (big shared chunks are also in `svx`)

 * `pptx` import is in `oox/source/ppt` (and uses a lot of
   `oox/source/drawingml` and `oox/source/*`)
 * `pptx` export is in `sd/source/filter/eppt` (mostly in `pptx-*` source
   files) and shared part is in `oox/source/export`

## PPTX Export / Import Filters

PPTX export filter is split into 2 parts. Impress related part is in
`sd/source/filter/eppt/pptx-*` and the other part is in
`oox/source/export/` because it contains mostly code related to
`DrawingML`, which is shared with writer and calc ooxml export.

The export filter was written in 2009 IIRC and was not much extended
feature-wise lately.

## Presenter Console
The main screen uses a hardware-accelerated
canvas (e.g. cairo canvas), while the entire secondary screen
uses a VCL-canvas that is created in
`sd::framework::FullScreenPane::CreateCanvas()`.

The secondary screen contains 3 `Pane`s which each have
2 `XWindows` for the border area & the actual content,
and each content Pane is backed by a `sd::presenter::PresenterCanvas`
that wraps the `FullScreenPane`'s canvas and does clipping.

## Future Works
Add custom shapes export (see below). enhance text
output, we don't write text style for indentation levels now, need to
export `a:lvl1pPr`, `a:lvl2pPr`, ... elements.

`PPTX` import was written by Sun/Oracle and then extended in LibreOffice
a lot during bug fixing. It is located in `oox/source/ppt` and
`oox/source/drawingml`. The areas with most bugs (at least until today)
were shape placeholders and text style inheritance.
