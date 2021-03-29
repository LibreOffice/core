# Extensions for the Impress and Draw Applications

`source/pdfimport/` - PDF import

Uses an external poppler process to parse and handle PDF
import as draw shapes.

`source/minimizer/` - Presentation Minimizer

Shrinks presentations by down-scaling images, and removing
extraneous eg. embedded OLE content.

`source/presenter/` - Impress / Presenter Console.

This couples to `sd/` in rather strange ways. Its design is
heavily mangled by an attempt to use only UNO interfaces
which are highly inadequate. This leads to somewhat
ridiculous situations. Activating in response to
configuration keys (for example), and the `XPresenterHelper`
interface inside `sd/` used to create and manage windows.

The main screen uses a hardware-accelerated
canvas (e.g. cairo canvas), while the entire secondary screen
uses a VCL-canvas that is created in
`sd::framework::FullScreenPane::CreateCanvas()`.

The secondary screen contains 3 `Pane`s which each have
2 `XWindows` for the border area & the actual content,
and each content Pane is backed by a `sd::presenter::PresenterCanvas`
that wraps the `FullScreenPane`'s canvas and does clipping.
