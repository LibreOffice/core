# COKit

COKit can be used for accessing CollaboraOffice functionality
through C/C++, without any need to use UNO.

For now it only offers document conversion (in addition to an experimental
tiled rendering API).

## Integrating COKit Into Other Software

COKit functionality can be accessed by including `COKit.h[xx]` in your
program.

COKit initialisation (`cok_init`) requires the inclusion of `COKitInit.h` in
your program. If you use the C++ `COKit.hxx` header, it already includes
`COKitInit.h` for you.

(`COKit.hxx` is a simple and fully inlined C++ wrapper for the same
functionality as in `COKit.h`.)

An example program can be seen on:
<https://gitlab.com/ojwb/lloconv>

## Tiled Rendering

To use COKit Tiled Rendering you will need the following before the COKit includes:

    #define KIT_USE_UNSTABLE_API

(This must be define before ANY COKit header, i.e. including the Init header.)

Currently only bitmap-buffer rendering is supported, with a 32-bit BGRA
colorspace (further alternatives could feasibly be implemented as needed).
Scanlines are ordered top-down (whereas CollaboraOffice will internally default
to bottom-up).

## Tiled Editing

On top of the tiled rendering API, a set of new methods have been added to the
`kit::Document` class to allow basic editing, too. Communication between the COKit
client and CollaboraOffice is a two-way channel. The client can initiate an action
by calling the above mentioned methods. The most important methods for the
client -> CollaboraOffice communication are:

- `initializeForRendering()`, expected to be called right after
  `kit::Office::documentLoad()` returned a `kit::Document*`.
- `postKeyEvent()`, expected to be called when the user provides input on the
  (soft-)keyboard.
- `postMouseEvent()`, expected to be called when the user generated a touch or
  mouse event.

In general, all coordinates are always in absolute twips (20th of a point, or:
1" = 1440 twips). See `kit::Document` in `COKit.hxx` for a full list of
methods and their documentation.

The other way around (CollaboraOffice -> COKit client) is implemented using a
callback. A COKit client can register a callback using the registerCallback()
method. Whenever editing requires some action on the client side, a callback
event is emitted. The callback types are described using the
`COKitCallbackType` enumeration in `COKitEnums.h`, the callback
function signature itself is provided by the COKitCallback typedef in
`COKitTypes.h`. The most important callback types:

- `KIT_CALLBACK_INVALIDATE_TILES`: drop all tiles cached on client-side that
  intersect with the provided rectangle
- `KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR`: need to set the position and/or the
  size of the cursor
- `KIT_CALLBACK_TEXT_SELECTION`: need to adjust the selection overlay provided
  by the client as the set of rectangles describing the selection overlay
  changed

Core has next to no idea what is the COKit client, so for effective development,
it's recommended that the core part is developed against `gtktiledviewer`, and
once a feature works there, then implement the Android part, with its slower
development iteration (slow uploading to the device, the need to link all
object files into a single `.so`, etc).

### COKit API guidelines

Introducing explicit new API under `include/COKit/` adds type safety but listing each &
every micro-feature in those headers don't scale. Before extending those headers, consider using one
of the following alternatives, which require no changes to these headers:

- COKit client → core direction: use `postUnoCommand()` to dispatch an UNO command, optionally with
  parameters.
- core → COKit client direction:
  - Use `getCommandValues()` when this is initiated by the COKit client.
  - Use `KIT_CALLBACK_STATE_CHANGED` with a JSON payload when this is initiated by core.

It's useful to stick to these if possible, only add new C++ API when these are not a good fit.

* Debugging with gdb and `gtktiledviewer`

To run `gtktiledviewer`:

    bin/run gtktiledviewer --lo-path=$PWD/instdir/program path/to/test.odt

To receive all incoming events from core use `G_MESSAGES_DEBUG=all`

    G_MESSAGES_DEBUG=all bin/run gtktiledviewer --lo-path=$PWD/instdir/program ../test.odt

To debug with `gdb`:

    export LO_TRACE='gdb --tui --args'

before `bin/run`, this will run gtktiledviewer in the debugger instead.

### Building and running gtktiledviewer on Windows

A pre-requisite is pre-built GTK3 libraries. See [official GTK documentation](https://www.gtk.org/docs/installations/windows/).
Building of gtktiledviewer on Windows is enabled by passing

    --with-gtk3-build=<path/to/GTK3/build/directory>

to configure.

Running the compiled executable requires GTK's bin in PATH:

    PATH=${PATH}:/cygdrive/c/gtk-build/gtk/x64/release/bin bin/run gtktiledviewer --lo-path=$(cygpath -am $PWD/instdir/program) ../test.odt

## COKitGtk

Currently consists of only a very basic GTK document viewer widget.

The widget uses `g_info()` instead of `SAL_INFO()`, use the `G_MESSAGES_DEBUG=all`
environment variable to display those messages.
