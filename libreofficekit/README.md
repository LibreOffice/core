LibreOfficeKit
**************

LibreOfficeKit can be used for accessing LibreOffice functionality
through C/C++, without any need to use UNO.

For now it only offers document conversion (in addition to an experimental
tiled rendering API).

Integrating LOK into other software
-----------------------------------

LOK functionality can be accessed by including LibreOfficeKit.h[xx] in your
program.

LOK initialisation (lok_init) requires the inclusion of LibreOfficeKitInit.h in
your program. If you use the C++ LibreOfficeKit.hxx header, it already includes
LibreOfficeKitInit.h for you.

(LibreOfficeKit.hxx is a simple and fully inlined C++ wrapper for the same
functionality as in LibreOfficeKit.h.)

An example program can be seen on:
https://gitlab.com/ojwb/lloconv

Tiled Rendering
---------------

To use LOK Tiled Rendering you will need the following before the LOK includes:
#define LOK_USE_UNSTABLE_API

(This must be define before ANY LOK header, i.e. including the Init header.)

Currently only bitmap-buffer rendering is supported, with a 32-bit BGRA
colorspace (further alternatives could feasibly be implemented as needed).
Scanlines are ordered top-down (whereas LibreOffice will internally default
to bottom-up).

Tiled Editing
-------------

On top of the tiled rendering API, a set of new methods have been added to the
lok::Document class to allow basic editing, too. Communication between the LOK
client and LibreOffice is a two-way channel. The client can initiate an action
by calling the above mentioned methods. The most important methods for the
client -> LibreOffice communication are:

- initializeForRendering(), expected to be called right after
  lok::Office::documentLoad() returned a lok::Document*.
- postKeyEvent(), expected to be called when the user provides input on the
  (soft-)keyboard.
- postMouseEvent(), expected to be called when the user generated a touch or
  mouse event.

In general, all coordinates are always in absolute twips (20th of a point, or:
1" = 1440 twips). See lok::Document in LibreOfficeKit.hxx for a full list of
methods and their documentation.

The other way around (LibreOffice -> LOK client) is implemented using a
callback. A LOK client can register a callback using the registerCallback()
method. Whenever editing requires some action on the client side, a callback
event is emitted. The callback types are described using the
LibreOfficeKitCallbackType enumeration in LibreOfficeKitEnums.h, the callback
function signature itself is provided by the LibreOfficeKitCallback typedef in
LibreOfficeKitTypes.h. The most important callback types:

- LOK_CALLBACK_INVALIDATE_TILES: drop all tiles cached on client-side that
  intersect with the provided rectangle
- LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR: need to set the position and/or the
  size of the cursor
- LOK_CALLBACK_TEXT_SELECTION: need to adjust the selection overlay provided
  by the client as the set of rectangles describing the selection overlay
  changed

There are currently two known LOK clients supporting tiled editing:

- gtktiledviewer (see below), which allows testing the LOK core implementation
  on (desktop) Linux
- (LibreOffice on) Android

Core has next to no idea what is the LOK client, so for effective development,
it's recommended that the core part is developed against gtktiledviewer, and
once a feature works there, then implement the Android part, with its slower
development iteration (slow uploading to the device, the need to link all
object files into a single .so, etc).

* Debugging with gdb and gtktiledviewer

To run gtktiledviewer:

    bin/run gtktiledviewer --lo-path=$PWD/instdir/program path/to/test.odt

To receive all incoming events from core use G_MESSAGES_DEBUG=all

    G_MESSAGES_DEBUG=all bin/run gtktiledviewer --lo-path=$PWD/instdir/program ../test.odt

To debug with gdb:

    export LO_TRACE='gdb --tui --args'

before bin/run, this will run gtktiledviewer in the debugger instead.

LibreOfficeKitGtk
*****************

Currently consists of only a very basic GTK+ document viewer widget.

The widget uses g_info() instead of SAL_INFO(), use the 'G_MESSAGES_DEBUG=all'
environment variable to display those messages.
