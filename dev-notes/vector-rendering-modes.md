# Vector rendering: what each application draws itself

With vector rendering on, the client picks one render manager by the document type. Three
methods of RenderManagerBase say how much of the drawing and of the object handling the client
does itself, and each manager answers them differently.

| Manager | Document type | drawsSlidesFromVectors | drawsFromObjects | drawsFromPrimitives |
|---|---|---|---|---|
| ImpressVectorManager | presentation, drawing | true | true | true |
| CalcVectorManager | spreadsheet | false | true | false |
| WriterVectorManager | text | false | false | true |
| BitmapTileManager | any other type, and every type with vector rendering off | false | false | false |

A text document whose status names no page ids is drawn by BitmapTileManager too.

## drawsFromPrimitives

The client draws the pages of the document from vector primitives and asks for no tiles of
them. The pages are the slides of a presentation or a drawing, and the pages of a text
document. VectorPartManagerBase answers true, so Impress and Writer do.

Calc answers false. Its cells are not pages of primitives. A section of their own draws them.

## drawsFromObjects

The client holds the drawing objects, such as shapes, pictures and charts, and selects, hit
tests and moves them itself. The engine is told the result. The default is the answer of
drawsSlidesFromVectors, and Calc answers true on its own.

Writer answers false. The engine selects and moves the drawing objects of a text document, the
same as with tiles.

The engine view has a flag of the same name, SfxViewShell::setDrawsFromObjects. A view of a
presentation, a drawing or a spreadsheet sets it when it asks for its first vector primitives,
and clears it with .uno:DrawFromTiles.

## drawsSlidesFromVectors

The document is a presentation or a drawing in vector mode. It is the widest of the three, and
only Impress answers true. On top of the two above, the client does what only Draw and Impress
do in vector mode: the workspace around the slide, the table handles, editing a chart or a
formula in place, and the slide show drawn from the primitives. dev-notes/vector-mode-guide.md
describes what that looks like for someone using it.
