# LibreOffice for Cactus Analytics Ltd.
This version of LibreOffice was produced specifically for Cactus Analytics
to use as a file type converter mainly for the conversion of
__xlsx__(Microsoft Excel spreadsheet files since 2007) to __HTML__ for
display within the website. And secondarily to converting older
__xls__(Microsoft Excel spreadsheet files pre-2007) to __xlsx__ files.

__LibreOffice__ as delivered contains a bug which effects the conversion
of __xlsx__ to __HTML__ this produces bad formatting where a hidden row
crosses a group of merged cells.

This version of LibreOffice has this issue fixed, except for one
particular rare case.

Secondarily there is a problem with the size of the delivered executable
for the cactus use case.

LibreOffice is intended to be deployed as an AWS Layer file, these
files are zip files with a maximum size of 128MBytes, the standard
deployment of LibreOffice is in excess of 280MBytes when
compressed, so this version contains the necessary scripts to remove the
unused parts of the application, these being help files,
scripting facilities and the GUI.

## Building Cactus version of LibreOffice.

This is a fork of the GitHub mirror of LibreOffice and as such
follows the same initial build as LibreOffice.

* Download this repository
* run autogen and make (As autogen requires a lot of parameters
a script is available for this cactus-make, this calls autogen,
make clean and make)

```bash
$ git clone git@github.com:cactus-bm/libreoffice-core.git libreoffice
$ cd libreoffice
$ ./cactus-make
```
* Note that cactus make can take a very long time to run so is only
really only suitable for an initial build, if you
make changes you should just run make at the top level.

This build process produces a standard libreoffice deliverable archive
__LibreOfficeDev_7.6.0.0.alpha0_Linux_x86-64_archive.tar.gz__
at __workdir/installation/LibreOfficeDev/archive/install/en-US__

The part of the file name __7.6.0.0.alpha0_Linux_x86-64__ is
the LibreOffice version number and may change if

to produce an AWS Layers compatible version and simplify some
manual testing another script is available:

The directory __cactus-build__ contains a number of example files that
need to be converted and a script that will extract the needed parts of
LibreOffice into a directory and compress to the form needed for use
by Cactus Analytics.

There main test files are .xlsx files containing various types of
overlapping merged calls hidden columns and hidden rows, but there are
also __xls__, __ppt__ and __doc__ files which can be converted to
__xlsx__ or __HTML__, __pptx__ or __docx__.

The test script allow you to pick which tests you wish to run.

to run tests:
```bash
$ cd cactus-build
$ ./create-test # this will display use case.
$ ./create-test html # will run conversions from xlsx to html.
$ ./create-test doc # will convert doc files to docx.
$ ./create-test xls-html # will convert xls directly to html.
```

There is no automatic testing of these files the conversions
are just executed producing files in the subdirectory __converted__,
these can be manually checked.

Note that the output files have the same name as the input files with
change file types extensions, one exception to this is when converting
__HTML__ if the source contains images these are separately extracted
but charts are not, for example:

the file spreadsheet-with-chart-image.xlsx contains a small data set
a graph of that data set and an image (a screenshot of the graph)
when converted to html is two files:
```
converted/spreadsheet-with-chart-image.html
converted/spreadsheet-with-chart-image_html_18b2813506ec3b97.png
converted/spreadsheet-with-chart-image_html_75cc8254b4b2486c.png
```
The first of these is the spreadsheet cells and incorporates
references to the others which are the images that were in the
original, note that they are both __png__ files, in the original one
was a __jpg__.
Note also that the chart is lost in the conversion.


# LibreOffice
[![Coverity Scan Build Status](https://scan.coverity.com/projects/211/badge.svg)](https://scan.coverity.com/projects/211) [![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/307/badge)](https://bestpractices.coreinfrastructure.org/projects/307) [![Translation status](https://weblate.documentfoundation.org/widgets/libo_ui-master/-/svg-badge.svg)](https://weblate.documentfoundation.org/engage/libo_ui-master/?utm_source=widget)

<img alt="OSIApproved" width="150" height="200" src="https://opensource.org/files/OSIApproved.png">

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
for which you can read the [API docs](https://api.libreoffice.org/)
and [Developers Guide](https://wiki.documentfoundation.org/Documentation/DevGuide).
This re-uses the (extremely generic) UNO APIs that are also used by
macro scripting in StarBasic.

The best way to add a generally useful feature to LibreOffice
is to work on the code base. Overall this way makes it easier
to compile and build your code, it avoids any arbitrary limitations of
our scripting APIs, and in general is far more simple and intuitive -
if you are a reasonably able C++ programmer.

## The Build Chain and Runtime Baselines

These are the current minimal operating system and compiler versions to
run and compile LibreOffice, also used by the TDF builds:

* Windows:
    * Runtime: Windows 7
    * Build: Cygwin + Visual Studio 2019 version 16.10
* macOS:
    * Runtime: 10.15
    * Build: 11.0 + Xcode 12.5
* Linux:
    * Runtime: RHEL 7 or CentOS 7
    * Build: either GCC 7.0.0; or Clang 8.0.1 with libstdc++ 7.3.0
* iOS (only for LibreOfficeKit):
    * Runtime: 11.4 (only support for newer i devices == 64 bit)
    * Build: Xcode 9.3 and iPhone SDK 11.4
* Android:
    * Build: NDK r19c and SDK 22.6.2
* Emscripten / WASM:
    * Runtime: a browser with SharedMemory support (threads + atomics)
    * Build: Qt 5.15 with Qt supported Emscripten 1.39.8
    * See [README.wasm](static/README.wasm.md)

Java is required for building many parts of LibreOffice. In TDF Wiki article
[Development/Java](https://wiki.documentfoundation.org/Development/Java), the
exact modules that depend on Java are listed.

The baseline for Java is Java Development Kit (JDK) Version 11 or later. It is
possible to build LibreOffice with JDK version 9, but it is no longer supported
by the JDK vendors, thus it should be avoided.

If you want to use Clang with the LibreOffice compiler plugins, the minimal
version of Clang is 12.0.1. Since Xcode doesn't provide the compiler plugin
headers, you have to compile your own Clang to use them on macOS.

You can find the TDF configure switches in the `distro-configs/` directory.

To set up your initial build environment on Windows and macOS, we provide
the LibreOffice Development Environment
([LODE](https://wiki.documentfoundation.org/Development/lode)) scripts.

For more information see the build instructions for your platform in the
[TDF wiki](https://wiki.documentfoundation.org/Development/How_to_build).

## The Important Bits of Code

Each module should have a `README.md` file inside it which has some
degree of documentation for that module; patches are most welcome to
improve those. We have those turned into a web page here:

<https://docs.libreoffice.org/>

However, there are two hundred modules, many of them of only
peripheral interest for a specialist audience. So - where is the
good stuff, the code that is most useful. Here is a quick overview of
the most important ones:

| Module                  | Description|
|-------------------------|-------------------------------------------------|
| [sal/](sal)             | this provides a simple System Abstraction Layer|
| [tools/](tools)         | this provides basic internal types: `Rectangle`, `Color` etc.|
| [vcl/](vcl)             | this is the widget toolkit library and one rendering abstraction|
| [framework/](framework) | UNO framework, responsible for building toolbars, menus, status bars, and the chrome around the document using widgets from VCL, and XML descriptions from `/uiconfig/` files|
| [sfx2/](sfx2)           | legacy core framework used by Writer/Calc/Draw: document model / load/save / signals for actions etc.|
| [svx/](svx)             | drawing model related helper code, including much of Draw/Impress|

Then applications

| Module              | Description                                                                                                                                   |
|---------------------|-----------------------------------------------------------------------------------------------------------------------------------------------|
| [desktop/](desktop) | this is where the `main()` for the application lives, init / bootstrap. the name dates back to an ancient StarOffice that also drew a desktop |
| [sw/](sw)          | Writer                                                                                                                                        |
| [sc/](sc)          | Calc                                                                                                                                          |
| [sd/](sd)          | Draw / Impress                                                                                                                                |

There are several other libraries that are helpful from a graphical perspective:

| Module                        | Description|
|-------------------------------|-------------------------------------------------|
| [basegfx/](basegfx)           | algorithms and data-types for graphics as used in the canvas|
| [canvas/](canvas)             | new (UNO) canvas rendering model with various backends|
| [cppcanvas/](cppcanvas)       | C++ helper classes for using the UNO canvas|
| [drawinglayer/](drawinglayer) | View code to render drawable objects and break them down into primitives we can render more easily.|

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
required) or poke people on IRC `#libreoffice-dev` on irc.libera.chat -
we're a friendly and generally helpful mob. We know the code can be
hard to get into at first, and so there are no silly questions.
