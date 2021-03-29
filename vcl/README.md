# Visual Class Library (VCL)

Visual Class Library (VCL) is responsible for the widgets (windowing, buttons, controls,
file-pickers etc.), operating system abstraction, including basic rendering (e.g. the output device).

It should not be confused with Borland's Visual Component Library, which is entirely unrelated.

VCL provides a graphical toolkit similar to gtk+, Qt, SWING etc.

+ source/
	+ the main cross-platform chunk of source

+ inc/
	+ cross-platform abstraction headers

+ headless/
	+ a backend renderer that draws to bitmaps

+ android/
	+ Android backend

+ osx/
	+ macOS backend

+ ios/
	+ iOS backend

+ quartz/
	+ code common to macOS and iOS

+ win/
	+ Windows backend

+ qt5/
	+ Qt5 (under construction)

+ unx/
	+ X11 backend and its sub-platforms
		+ gtk3/
			+ GTK3 support
		+ kf5/
			+ KF5 support (based on qt5 VCL plugin mentioned above)
		+ gtk3_kde5/
			+ GTK3 support with KDE5 file pickers (alternative to native kf5 one)
		+ generic/
			+ raw X11 support

		+ plugadapt/
			+ pluggable framework to select correct unx backend

		+ dtrans/
			+ "data transfer" - clipboard handling
			+ http://stackoverflow.com/questions/3261379/getting-html-source-or-rich-text-from-the-x-clipboard
			  for tips how to show the current content of the
			  clipboard


+ How the platform abstraction works

	+ InitVCL calls 'CreateSalInstance'
		+ this is implemented by the compiled-in platform backend
		+ it stores various bits of global state in the
		  'SalData' (inc/saldatabasic.hxx) structure but:
	+ the SalInstance vtable is the primary outward facing gateway
	  API for platform backends
		+ It is a factory for:
		  SalFrames, SalVirtualDevices, SalPrinters,
		  Timers, the SolarMutex, Drag&Drop and other
		  objects, as well as the primary event loop wrapper.

Note: references to "SV" in the code mean StarView, which was a
portable C++ class library for GUIs, with very old roots, that was
developed by StarDivision. Nowadays it is not used by anything except
LibreOffice (and OpenOffice).

"svp" stands for "StarView Plugin".

## COM Threading

The way COM is used in LO generally:
- vcl puts main thread into Single-threaded Apartment (STA)
- oslWorkerWrapperFunction() puts every thread spawned via `oslCreateThread()`
  into MTA (free-threaded)

# GDIMetafile

GDIMetafile is a vector drawing representation that corresponds directly
to the SVM (StarView Metafile) format; it is extremely important as
an intermediate format in all sorts of drawing and printing operations.

There is a class `MetafileXmlDump` in `include/vcl/mtfxmldump.hxx` that
can store a GDIMetafile as XML, which makes debugging much easier
since you can just use "diff" to see changes.

## EMF+

emf+ is vector file format used by MSO and is successor of wmf and
emf formats. see
http://msdn.microsoft.com/en-us/library/cc230724.aspx for
documentation. note that we didn't have this documentation from
start, so part of the code predates to the time when we had guessed
some parts and can be enhanced today. there also still many thing not
complete

emf+ is handled a bit differently compared to original emf/wmf files,
because GDIMetafile is missing features we need (mostly related to
transparency, argb colors, etc.)

emf/wmf is translated to GDIMetafile in import filter
`vcl/source/filter/wmf` and so special handling ends here

emf+ is encapsulated into GDIMetafile inside comment records and
parsed/rendered later, when it reaches cppcanvas. It is parsed and
rendered in cppcanvas/source/mtfrenderer. also note that there are
emf+-only and emf+-dual files. dual files contains both types of
records (emf and emf+) for rendering the images. these can used also
in applications which don't know emf+. in that case we must ignore
emf records and use emf+ for rendering. for more details see
the documentation.

## Parsing

 wmf/emf filter --> GDI metafile with emf+ in comments --> cppcanvas metafile renderer

lately the GDIMetafile rendering path changed which also influenced
emf+ rendering. now many things happen in drawing layer, where
GDIMetafile is translated into drawing layer primitives. for
metafiles with emf+ we let the mtfrenderer render them into bitmap
(with transparency) and use this bitmap in drawinlayer. cleaner
solution for current state would be to extend the drawing layer for
missing features and move parsing into drawing layer (might be quite
a lot of work). intermediary enhancement would be to know better the
needed size/resolution of the bitmap, before we render emf+ into
bitmap in drawing layer. Thorsten is working on the same problem with
svg rendering, so hopefully his approach could be extended for emf+ as
well. the places in drawing layer where we use canvas mtfrenderer to
render into bitmaps can be found when you grep for GetUseCanvas. also
look at vcl/source/gdi/gdimetafile.cxx where you can look for
UseCanvas again. moving the parsing into drawinglayer might also have
nice side effect for emf+-dual metafiles. in case the emf+ records
are broken, it would be easier to use the duplicit emf
rendering. fortunately we didn't run into such a broken emf+ file
yet. but there were already few cases where we first though that the
problem might be because of broken emf+ part. so far it always turned
out to be another problem.

## Rendering

 before

 vcl --> cppcanvas metafile renderer --> vcl

 now

 drawing layer --> vcl --> cppcanvas metafile renderer --> vcl --> drawing layer

another interesting part is actual rendering into canvas bitmap and
using that bitmap later in code using vcl API.

EMF+ implementation has some extensive logging, best if you do a dbgutil
build, and then

    export SAL_LOG=+INFO.cppcanvas.emf+INFO.vcl.emf

before running LibreOffice; it will give you lots of useful hints.

You can also fallback to EMF (from EMF+) rendering via

    export EMF_PLUS_DISABLE=1


## Printing/PDF Export

Printing from Writer works like this:

1) individual pages print by passing an appropriate OutputDevice to XRenderable
2) in drawinglayer, a VclMetafileProcessor2D is used to record everything on
   the page (because the OutputDevice has been set up to record a GDIMetaFile)
3) the pages' GDIMetaFiles are converted to PDF by the vcl::PDFWriter
   in `vcl/source/gdi/pdfwriter*`

Creating the ODF thumbnail for the first page works as above except step 3 is:

3) the GDIMetaFile is replayed to create the thumbnail

On-screen display differs in step 1 and 2:

1) the VCL Window gets invalidated somehow and paints itself
2) in drawinglayer, a `VclPixelProcessor2D` is used to display the content


### Debugging PDF export

Debugging the PDF export becomes much easier when
compression is disabled (so the PDF file is directly readable) and
the MARK function puts comments into the PDF file about which method
generated the following PDF content.

The compression can be disabled even using an env. var:

    export VCL_DEBUG_DISABLE_PDFCOMPRESSION=1

To de-compress the contents of a PDF file written by a release build or
other programs, use the "pdfunzip" tool:

    bin/run pdfunzip input.pdf output.pdf

### SolarMutexGuard

The solar mutex is the "big kernel lock" of LibreOffice, a global one. It's a
recursive mutex, so it's allowed to take the lock on the same thread multiple
times, and only the last unlock will actually release the mutex.

UNO methods on components can be called from multiple threads, while the
majority of the codebase is not prepared for multi-threading. One way to get
around this mismatch is to create a SolarMutexGuard instance at the start of
each & every UNO method implementation, but only when it is necessary:

- Only acquire the SolarMutex if you actually need it (e.g., not in functions
  that return static information).

- Only around the code that actually needs it (i.e., never call out with it
  locked).

This way you ensure that code (not prepared for multithreading) is still
executed only on a single thread.

In case you expect that your caller takes the solar mutex, then you can use
the `DBG_TESTSOLARMUTEX()` macro to assert that in dbgutil builds.

Event listeners are a special (but frequent) case of the "never call out with
a mutex (`SolarMutex` or other) locked" fundamental rule:

- UNO methods can be called from multiple threads, so most implementations
  take the solar mutex as their first action when necessary.

- This can be problematic if later calling out (an event handler is called),
  where the called function may be an UNO method implementation as well and
  may be invoked on a different thread.

- So we try to not own the solar mutex, whenever we call out (invoke event
  listeners).

In short, never hold any mutex unless necessary, especially not when calling
out.
