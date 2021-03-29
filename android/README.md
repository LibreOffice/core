# LibreOffice for Android

## Bootstrap

Contains common code for all projects on Android to bootstrap LibreOffice. In
addition it is a home to `LibreOfficeKit` (LOK - see `libreofficekit/README.md`) JNI
classes.

## Stuff in Source Directory

LibreOffice Android application - the code is based on Fennec (Firefox for Android).
It uses OpenGL ES 2 for rendering of the document tiles which are gathered from
LibreOffice using LOK. The application contains the LibreOffice core in one shared
library: `liblo-native-code.so`, which is bundled together with the application.

## Architecture and Threading

The application implements editing support using 4 threads:

1. The Android UI thread, we can't perform anything here that would take a considerable
   amount of time.
2. An OpenGL thread which contains the OpenGL context and is responsible for drawing
   all layers (including tiles) to the screen.
3. A thread (`LOKitThread`), that performs `LibreOfficeKit` calls, which may take more time
   to complete. In addition it also receives events from the soffice thread (see below)
   when the callback emits an event. Events are stored in a blocking queue (thread
   processes events in FCFS order, goes to sleep when no more event is available and
   awakens when there are events in the queue again).
4. A native thread created by LibreOfficeKit (we call it the soffice thread), where
   LibreOffice itself runs. It receives calls from `LOKitThread`, and may emit callback
   events as necessary.

## LOKitThread

`LOKitThread` (`org.libreoffice.LOKitThread`) communicates with LO via JNI (this can
be done only for one thread) and processes events (defined in `org.libreoffice.LOEvent`)
triggered from UI.

## Application Overview

LibreOfficeMainActivity (`org.libreoffice.LibreOfficeMainActivity`) is the entry point
of the application - everything starts up and tears down from here (`onCreate`, `onResume`,
`onPause`, `onStart`, `onStop`, `onDestroy`).

### Document View

From here on one of the most interesting pieces are the classes around document view,
which includes listening to touch events, recalculating the viewport, tiled handling
and rendering the layers to the document.

`Viewport` - the viewport is the currently visible part of the document. It is defined
           by view rectangle and zoom.

`Layers` - document view is rendered using many layers. Such layers are: document
         background, scroll handles, and also the document tiles.

### Document View Classes

- `LayerView` (`org.mozilla.gecko.gfx.LayerView`) is the document view of the application.
  It uses the `SurfaceView` (`android.view.SurfaceView`) as the main surface to draw on
  using OpenGL ES 2.

- `GLController` (`org.mozilla.gecko.gfx.GLController`) - holder of the OpenGL context.

- `RenderControllerThread` (`org.mozilla.gecko.gfx.RenderControllerThread`) executes the
  rendering requests through LayerRenderer.

- `LayerRenderer` (`org.mozilla.gecko.gfx.LayerRenderer`) renders all the layers.

- `GeckoLayerClient` (`org.mozilla.gecko.gfx.GeckoLayerClient`) is the middle man of the
  application, which connects all the bits together. It is the document view layer
  holder so the any management (including tiled rendering) usually go through this
  class. It listens to draw requests and viewport changes from `PanZoomController`
  (see "Touch events").

### Touch Events, Scrolling and Zooming

The main class that handles the touch event, scrolling and zooming is `JavaPanZoomController`
`org.mozilla.gecko.gfx.JavaPanZoomController` (implementation of `PanZoomController` interface).
When the user performs a touch action, the document view needs to change, which means the
viewport changes. `JavaPanZoomController` changes the viewport and signals the change through
`PanZoomTarget` (`org.mozilla.gecko.gfx.PanZoomTarget`).

### Tiled Rendering

Tiled rendering is a technique that splits the document to bitmaps of same size (typically
256x256) which are fetched on demand.

In the application the `ComposedTileLayer` (`org.mozilla.gecko.gfx.ComposedTileLayer`) is the
layer responsible for tracking and managing the tiles. Tiles are in this case also layers
(sub layers?) implemented in `SubTile` (`org.mozilla.gecko.gfx.SubTile`), where each one is
responsible for one tile bitmap (actually OpenGL texture once it has been uploaded).

When the viewport changes, the request for tile rechecking is send to `LOKitThread` (see
LOKitThread#tileReevaluationRequest), where the tiles are rechecked, add and removed if
necessary.

`CompositeTileLayer` is actually an abstract class, which has two implementations. One is
`DynamicTileLayer` (`org.mozilla.gecko.gfx.DynamicTileLayer`), which is used for main tile
view of the document, and `FixedZoomTileLayer` (`org.mozilla.gecko.gfx.FixedZoomTileLayer`),
which just renders the tiles at a fixed zoom level. This is then used as a background
low resolution layer.

### Tile Invalidation

Tile can change in LibreOffice when user changes the content (adds, removes text or changes
the properties). In this case, an invalidation rectangle is signaled from LibreOffice, which
includes a rectangle that needs to be invalidated. In this case `LOKitThread` gets this request
via callback, and rechecks all tiles if they need to be invalidated. For more details see
LOKitThread#tileInvalidation).

## Editing

For editing there are 2 coarse tasks that the LibreOffice app must do:

1. send input events to LibreOffice core (keyboard, touch and mouse)
2. listen to messages (provided via callback) from LibreOffice core and react accordingly

In most cases when an input event happens and is send to the LO core, then a message from
LO core follows. For example: when the user writes to the keyboard, key event is sent and
an invalidation request from LO core follows. When user touches an image, a mouse event is
sent, and a "new graphic selection" message from LO core follows.

All keyboard and touch events are sent to `LOKitThread` as `LOEvents`. In `LOKitThread` they are
processed and sent to LibreOffice core. The touch events originate in `JavaPanZoomController`,
the keyboard events in `LOKitInputConnectionHandler` (`org.libreoffice.LOKitInputConnectionHandler`),
however there are other parts too - depending on the need.

`InvalidationHandler` (`org.libreoffice.InvalidationHandler`) is the class that is responsible
to process messages from LibreOffice core and to track the state.

## Overlay

Overlay elements like cursor and selections aren't drawn by the LO core, instead the core
only provides data (cursor position, selection rectangles) and the app needs to draw them.
`DocumentOverlay` (`org.libreoffice.overlay.DocumentOverlay`) and `DocumentOverlayView`
(`org.libreoffice.overlay.DocumentOverlayView`) are the classes that provide the overlay over
the document, where selections and the cursor is drawn.


## Icons

App uses material design icons available at [1].


[1] - <https://www.google.com/design/icons/>

## Emulator and Debugging Notes

For instructions on how to build for Android, see `README.cross`.

### Getting Something Running

Attach your device, so 'adb devices' shows it. Then run:

        cd android/source
        make install
        adb logcat

and if all goes well, you should have some nice debug output to enjoy when you
start the app.

### Using the Emulator

Create an AVD in the android UI, don't even try to get the data partition size
right in the GUI, that is doomed to producing an AVD that doesn't work.
Instead start it from the console:

        LD_LIBRARY_PATH=$(pwd)/lib emulator-arm -avd <Name> -partition-size 500

where <Name> is the literal name of the AVD that you entered.

[ In order to have proper acceleration, you need the 32-bit `libGL.so`:

        sudo zypper in Mesa-libGL-devel-32bit

and run emulator-arm after the installation. ]

Then you can run `ant`/`adb` as described above.

After a while of this loop you might find that you have lost a lot of
space on your emulator's or device's `/data` volume. You can do:

        adb shell stop; adb shell start

## Debugging

First of all, you need to configure the build with `--enable-debug` or
`--enable-dbgutil`.  You may want to provide `--enable-symbols` to limit debuginfo,
like `--enable-symbols="sw/"` or so, in order to fit into the memory
during linking.

Building with all symbols is also possible but the linking is currently
slow (around 10 to 15 minutes) and you need lots of memory (around 16GB + some
swap).

### Using ndk-gdb

Direct support for using `ndk-gdb` has been removed from the build system. It is
recommended that you give the lldb debugger a try that has the benefit of being
nicely integrated into Android Studio (see below for instructions).
If you nevertheless want to continue using `ndk-gdb`, use the following steps
that are described in more detail here: <https://stackoverflow.com/a/10539883>

- add `android:debuggable="true"` to `AndroidManifest.xml`
- push `gdbserver` to device, launch and attach to application
- forward debugging port from host to device
- launch matching gdb on host and run following setup commands:
        - set solib-search-path obj/local/<appAbi>
        - file obj/local/<appAbi>/liblo-native-code.so
        - target remote :<portused>

Pretty printers aren't loaded automatically due to the single shared
object, but you can still load them manually. E.g. to have a pretty-printer for
`rtl::OString`, you need:

        (gdb) python sys.path.insert(0, "/master/solenv/gdb")
        (gdb) source /master/instdir/program/libuno_sal.so.3-gdb.py

### Using Android Studio (and Thus lldb)

Note that lldb might not yield the same results as `ndk-gdb`. If you suspect a
problem with `lldb`, you can try to manually use `ndk-gdb` as described above.
Using `lldb` from within Android Studio is more comfortable though and works like this:

- open `android/source/build.gradle` in Android Studio via File|New → Import Project
- make sure you select the right build variant (`strippedUIDebug` is what you want)
- use Run|Edit Configurations to create a new configuration of type "Android Native"
	- on tab "General" pick module "source"
	- on tab "Native Debugger" add `android/obj/local/<hostarch>` to
	the Symbol directories
	- on the LLDB startup commands tab add
	"command script import `/path/to/solenv/lldb/libreoffice/LO.py`"
	to get some pretty printing hooks for the various string classes

Then you can select your new configuration and use Run | Debug to launch it.
Note that `lldb` doesn't initially stop execution, so if you want to add
breakpoints using lldb prompt, you manually have to pause execution, then you
can switch to the lldb tab and add your breakpoints. However making use of the
editor just using File|Open .. to open the desired file in Android Studio and
then toggling the breakpoint by clicking on the margin is more comfortable.

### Debugging the Java Part

Open `android/source/build.gradle` in Android studio via File|New → Import
Project and you can use Android Studio's debugging interface.
Just make sure you pick the correct build variant (strippedUIDebug)

The alternative is to use the jdb command-line debugger. Steps to use it:

1. Find out the JDWP ID of a debuggable application:

        adb jdwp

    From the list of currently active JDWP processes, the last number is the just
started debuggable application.

2. Forward the remote JDWP port/process ID to a local port:

        adb forward tcp:7777 jdwp:31739

3. Connect to the running application:

        jdb -sourcepath src/java/ -attach localhost:7777

Assuming that you're already in the LOAndroid3 directory in your shell.

### Debugging the Missing Services

Android library only include essential services that are compiled for
LibreOffice in order to reduce the size of the apk. When developing,
some services might become useful and we should add those services
to the combined library.

In order to identify missing services, we need to be able to receive
`SAL_INFO` from `cppuhelper/source/shlib.cxx` in logcat and therefore identify
what services are missing. To do so, you may want add the following
when configuring the build.

    --enable-symbols="cppuhelper/ sal/"

[TODO: This is nonsense. `--enable-symbols` enables the `-g` option, not `SAL_INFO`.
Perhaps this was a misunderstanding of meaning of `--enable-selective-debuginfo`,
the old name for the option.]

Which services are combined in the android lib is determined by

    solenv/bin/native-code.py

### Common Errors / Gotchas

    lo_dlneeds: Could not read ELF header of /data/data/org.libreoffice...libfoo.so
This (most likely) means that the install quietly failed, and that
the file is truncated; check it out with `adb shell ls -l /data/data/...`

### Startup Details

All Android apps are basically Java programs. They run "in" a Dalvik
(or on Android 5 or newer - ART) virtual machine. Yes, you can also
have apps where all *your* code is native code, written in a compiled
language like C or C++. But also such apps are actually started
by system-provided Java bootstrapping code (`NativeActivity`) running
in a Dalvik VM.

Such a native app (or actually, "activity") is not built as a
executable program, but as a shared object. The Java `NativeActivity`
bootstrapper loads that shared object with dlopen.

Anyway, our current "experimental" apps are not based on `NativeActivity`.
They have normal Java code for the activity, and just call out to a single,
app-specific native library (called `liblo-native-code.so`) to do all the
heavy lifting.
