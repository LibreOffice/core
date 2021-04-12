# LibreOffice
[![Coverity Scan Build Status](https://scan.coverity.com/projects/211/badge.svg)](https://scan.coverity.com/projects/211) [![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/307/badge)](https://bestpractices.coreinfrastructure.org/projects/307) [![Translation status](https://weblate.documentfoundation.org/widgets/libo_ui-master/-/svg-badge.svg)](https://weblate.documentfoundation.org/engage/libo_ui-master/?utm_source=widget)

<img align="right" width="150" height="200" src="https://opensource.org/files/OSIApproved.png">

LibreOffice is an integrated office suite based on copyleft licenses
and compatible with most document formats and standards. Libreoffice
is backed by The Document Foundation, which represents a large
independent community of enterprises, developers and other volunteers
moved by the common goal of bringing to the market the best software
for personal productivity. LibreOffice is open source, and free to
download, use and distribute.

A quick overview of the LibreOffice code structure.

## Overview

You can develop for LibreOffice in one of two ways, one
recommended and one much less so. First the somewhat less recommended
way: it is possible to use the SDK to develop an extension,
for which you can read the API docs [here](https://api.libreoffice.org/)
and [here](https://wiki.openoffice.org/wiki/Documentation/DevGuide).
This re-uses the (extremely generic) UNO APIs that are also used by
macro scripting in StarBasic.

The best way to add a generally useful feature to LibreOffice
is to work on the code base however. Overall this way makes it easier
to compile and build your code, it avoids any arbitrary limitations of
our scripting APIs, and in general is far more simple and intuitive -
if you are a reasonably able C++ programmer.

## The Build Chain and Runtime Baselines

These are the current minimal operating system and compiler versions to
run and compile LibreOffice, also used by the TDF builds:

* Windows:
    * Runtime: Windows 7
    * Build: Cygwin + Visual Studio 2019 version 16.5
* macOS:
    * Runtime: 10.10
    * Build: 10.14.4 + Xcode 11.3
* Linux:
    * Runtime: RHEL 7 or CentOS 7
    * Build: either GCC 7.0.0; or Clang 5.0.2 with libstdc++ 7.3.0
* iOS (only for LibreOfficeKit):
    * Runtime: 11.4 (only support for newer i devices == 64 bit)
    * Build: Xcode 9.3 and iPhone SDK 11.4
* Android:
    * Build: NDK r19c and SDK 22.6.2

If you want to use Clang with the LibreOffice compiler plugins, the minimal
version of Clang is 5.0.2. Since Xcode doesn't provide the compiler plugin
headers, you have to compile your own Clang to use them on macOS.

You can find the TDF configure switches in the `distro-configs/` directory.

To setup your initial build environment on Windows and macOS, we provide
the LibreOffice Development Environment
([LODE](https://wiki.documentfoundation.org/Development/lode)) scripts.

For more information see the build instructions for your platform in the
[TDF wiki](https://wiki.documentfoundation.org/Development).

## The Important Bits of Code

Each module should have a `README.md` file inside it which has some
degree of documentation for that module; patches are most welcome to
improve those. We have those turned into a web page here:

<https://docs.libreoffice.org/>

However, there are two hundred modules, many of them of only
peripheral interest for a specialist audience. So - where is the
good stuff, the code that is most useful. Here is a quick overview of
the most important ones:

Module    | Description
----------|-------------------------------------------------
[sal/](sal)             | this provides a simple System Abstraction Layer
[tools/](tools)         | this provides basic internal types: `Rectangle`, `Color` etc.
[vcl/](vcl)             | this is the widget toolkit library and one rendering abstraction
[framework/](framework) | UNO framework, responsible for building toolbars, menus, status bars, and the chrome around the document using widgets from VCL, and XML descriptions from `/uiconfig/` files
[sfx2/](sfx2)           | legacy core framework used by Writer/Calc/Draw: document model / load/save / signals for actions etc.
[svx/](svx)             | drawing model related helper code, including much of Draw/Impress

Then applications

Module    | Description
----------|-------------------------------------------------
[desktop/](desktop)  | this is where the `main()` for the application lives, init / bootstrap. the name dates back to an ancient StarOffice that also drew a desktop
[sw/](sw/)           | Writer
[sc/](sc/)           | Calc
[sd/](sd/)           | Draw / Impress

There are several other libraries that are helpful from a graphical perspective:

Module    | Description
----------|-------------------------------------------------
[basegfx/](basegfx)  | algorithms and data-types for graphics as used in the canvas
[canvas/](canvas)   | new (UNO) canvas rendering model with various backends
[cppcanvas/](cppcanvas) | C++ helper classes for using the UNO canvas
[drawinglayer/](drawinglayer) | View code to render drawable objects and break them down into primitives we can render more easily.

## Rules for #include Directives (C/C++)

Use the `"..."` form if and only if the included file is found next to the
including file. Otherwise, use the `<...>` form. (For further details, see the
mail [Re: C[++]: Normalizing include syntax ("" vs
<>)](https://lists.freedesktop.org/archives/libreoffice/2017-November/078778.html).)

The UNO API include files should consistently use double quotes, for the
benefit of external users of this API.

`loplugin:includeform (compilerplugins/clang/includeform.cxx)` enforces these rules.


## Finding Out More

Beyond this, you can read the `README.md` files, send us patches, ask
on the mailing list libreoffice@lists.freedesktop.org (no subscription
required) or poke people on IRC `#libreoffice-dev` on irc.freenode.net -
we're a friendly and generally helpful mob. We know the code can be
hard to get into at first, and so there are no silly questions.
