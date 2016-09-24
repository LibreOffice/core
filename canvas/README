UNO-based graphics backend, lesser impedance to modern graphics APIs
than vcl.

== The Canvas Framework ==

The canvas framework is the successor of the system GUI and graphics
backend VCL. Basic functionality is available, supplying just as much
features as necessary to provide a VCL-equivalent feature set (except
proper BiDi/CTL support).

The canvas framework consists of the following two modules, canvas and
cppcanvas. Additionally, a new generic graphics tooling is used (but
not exclusively by the canvas, Armin's drawinglayer module also make
use of it), which resides in basegfx.

The UNO API used by the canvas is primarily under
css::rendering, with css::rendering::XCanvas
being the central interface.

== The slideshow engine ==

The slideshow engine has replaced the former Impress-embedded
presentation framework with a fully independent UNO component, and it
is based on the canvas. Some features used there are only available
from canvas, like double-buffering, and hardware-accelerated
alpha-blending (currently not on all platforms).

== Cairo canvas ==

cairo canvas is one of backends of canvas component. canvas is mostly
used for slideshow rendering and also for emf+ rendering. we hoped it
will even be used by drawing layer, but it didn't happen (yet?) for
API look at offapi/com/sun/star/rendering/, the implementation is in
canvas and cppcanvas modules.

cairo canvas backend uses cairo library for rendering. main advantage
is support of alpha transparency and in some cases accelerated
rendering.

the backend itself is quite old and stable, not many changes in that
area lately, mostly changes for emf+ rendering, communication with
vcl and bugfixes

FUTURE work: look at cairo canvas and situation when it is used
(mostly slideshow). TODO there still might be more cases when we
can save some roundtrips when exchanging data with vcl.
