# Environment variables in VCL

## General

These are the general environment variables used in the VCL:

* `SAL_USE_VCLPLUGIN` - use a VCL plugin
* `SAL_RTL_ENABLED` - Enable RTL UI
* `SAL_NO_NWF` - disable native widgets
* `SAL_FORCEDPI` - force a specific DPI (gen, gtk3/gtk4, qt5/qt6/kf5/kf6 plugins only)
* `SAL_FORCE_HC` - force high-contrast mode
* `SAL_USE_SYSTEM_LOOP` - calls std::abort on nested event loop calls. Currently just for Qt with many crashes. WIP.

* `SAL_NO_FONT_LOOKUP` - disable font search and fallback and always use a hard-coded font name (for some unit tests)
* `SAL_NON_APPLICATION_FONT_USE` - control use of non-bundled fonts, values are `deny` or `abort`;
  for now only works on platforms using fontconfig (i.e., on Linux, but neither on macOS nor on
  Windows); also see gb_CppunitTest_set_non_application_font_use for using it in unit tests
* `SAL_ALLOW_DEFAULT_HINTING` - use default font hinting for the platform. Enables medium/full hinting 
  style which is otherwise reverted to (s)light.

* `LO_COLLECT_UIINFO` - enable the uitesting logging, value is expected to be a relative file name that
will be used to write the log under `instdir/uitest/`.

* `VCL_DOUBLEBUFFERING_AVOID_PAINT` - don't paint the buffer, useful to see where we do direct painting
* `VCL_DOUBLEBUFFERING_FORCE_ENABLE` - enable double buffered painting
* `VCL_DOUBLEBUFFERING_ENABLE` - enable a safe subset of double buffered painting (currently in Writer, not in any other applications)

* `VCL_DEBUG_DISABLE_PDFCOMPRESSION` - disable compression in the PDF writer

* `SAL_DISABLE_WATCHDOG` - don't start the thread that watches for broken GL/Vulkan/OpenCL drivers

* `SAL_NO_MOUSEGRABS` - for debugging - stop blocking UI if a breakpoint is hit

## Gtk+

* `VCL_GTK3_PAINTDEBUG` - in debug builds, if set to `1` then holding down `shift+0` forces a redraw event, `shift+1` repaints everything, and
`shift+2` dumps cairo frames to pngs as `/tmp/frame<n>.png`
* `GDK_SCALE=2` - for HiDPI scaling (just supports integers)

## Bitmap

* `VCL_NO_THREAD_SCALE` - disable threaded bitmap scale
* `VCL_NO_THREAD_IMPORT` - disable threaded bitmap import
* `EMF_PLUS_DISABLE` - use EMF rendering and ignore EMF+ specifics

## OpenGL

* `SAL_DISABLEGL` - disable OpenGL use
* `SAL_GL_NO_SWAP` - disable buffer swapping if set (should show nothing)
* `SAL_GL_SLEEP_ON_SWAP` - sleep for half a second on each swap-buffers.

## Skia

* `SAL_DISABLESKIA=1` - force disabled Skia
* `SAL_ENABLESKIA=1` - enable Skia, unless denylisted (and if the VCL backend supports Skia)
* `SAL_FORCESKIA=1` - force using Skia, even if denylisted
* `SAL_SKIA=raster|vulkan|metal` - select Skia's drawing method, by default Vulkan or Metal are used if available
* `SAL_DISABLE_SKIA_CACHE=1` - disable caching of complex images
* `SAL_SKIA_KEEP_BITMAP_BUFFER=1` - `SkiaSalBitmap` will keep its bitmap buffer even after storing in `SkImage`

## OpenGL,Skia

* `SAL_WITHOUT_WIDGET_CACHE` - disable LRU caching of native widget textures

## Qt

* `QT_SCALE_FACTOR=2` - for HiDPI testing (also supports float)
* `SAL_VCL_QT_NO_FONTCONFIG` - ignore fontconfig provided font substitutions
* `SAL_VCL_QT_NO_NATIVE` - disable `QStyle`'d controls
* `SAL_VCL_QT_USE_QFONT` - use `QFont` for text layout and rendering (default is to use cairo)
* `SAL_VCL_QT_USE_WELDED_WIDGETS` - enable use of welded Qt widgets also for dialogs created from .ui files
* `SAL_VCL_QT_NO_WELDED_WIDGETS` - disable the use of welded Qt widgets completely (also for simple message
  dialogs), reverts to previous VCL widget behavior

## Mac

* `SAL_FORCE_HIDPI_SCALING` - set to 2 to fake HiDPI drawing (useful for unittests, windows may draw only top-left 1/4 of the content scaled)
