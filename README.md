A quick overview of the LibreOffice code structure.

## Overview

You can develop for LibreOffice in one of two ways, one
recommended and one much less so. First the somewhat less recommended
way: it is possible to use the SDK to develop an extension,
for which you can read the API docs [here](http://api.libreoffice.org/)
and [here](http://wiki.services.openoffice.org/wiki/Documentation/DevGuide).
This re-uses the (extremely generic) UNO APIs that are also used by
macro scripting in StarBasic.

The best way to add a generally useful feature to LibreOffice
is to work on the code base however. Overall this way makes it easier
to compile and build your code, it avoids any arbitrary limitations of
our scripting APIs, and in general is far more simple and intuitive -
if you are a reasonably able C++ programmer.


## The important bits of code

Each module should have a `README` file inside it which has some
degree of documentation for that module; patches are most welcome to
improve those. We have those turned into a web page here:

http://docs.libreoffice.org/

However, there are two hundred modules, many of them of only
peripheral interest for a specialist audience. So - where is the
good stuff, the code that is most useful. Here is a quick overview of
the most important ones:

Module    | Description
----------|-------------------------------------------------
sal/      | this provides a simple System Abstraction Layer
tools/    | this provides basic internal types: 'Rectangle', 'Color' etc.
vcl/      | this is the widget toolkit library and one rendering abstraction
framework | UNO framework, responsible for building toolbars, menus, status bars, and the chrome around the document using widgets from VCL, and XML descriptions from */uiconfig/* files
sfx2/     | legacy core framework used by Writer/Calc/Draw: document model / load/save / signals for actions etc.
svx/      | drawing model related helper code, including much of Draw/Impress

Then applications

Module    | Description
----------|-------------------------------------------------
desktop/  | this is where the 'main' for the application lives, init / bootstrap. the name dates back to an ancient StarOffice that also drew a desktop
sw/       | Writer
sc/       | Calc
sd/       | Draw / Impress

There are several other libraries that are helpful from a graphical perspective:

Module    | Description
----------|-------------------------------------------------
basebmp/  | enables a VCL compatible rendering API to render to bitmaps, as used for LibreOffice Online, Android, iOS, etc.
basegfx/  | algorithms and data-types for graphics as used in the canvas
canvas/   | new (UNO) canvas rendering model with various backends
cppcanvas/ | C++ helper classes for using the UNO canvas
drawinglayer/ | View code to render drawable objects and break them down into primitives we can render more easily.


## Finding out more

Beyond this, you can read the `README` files, send us patches, ask
on the mailing list libreoffice@lists.freedesktop.org (no subscription
required) or poke people on IRC `#libreoffice-dev` on irc.freenode.net -
we're a friendly and generally helpful mob. We know the code can be
hard to get into at first, and so there are no silly questions.
