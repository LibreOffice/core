# Status

As work-in-progress, some of the online C++ artifacts are now built with engine's gbuild system.
(The plan is to extend that to at least all of online's C++ artifacts, and then retire at least the
no longer used parts of online's Automake-based build system.)  However, for now, the use of gbuild
as part of the online build is more of an implementation detail (with a bespoke invocation of
online's `gbuild/Makefile` from within online's Automake `Makefile.am`) than something user-facing.

The configuration and build is still two-step:  Configure and build engine first, then configure and
build online.  Building any part of online, even the gbuild-built parts, still relies on a
completely finished engine build.  The gbuild-built parts of online do not automatically rebuild any
of their engine-side dependencies.

But the gbuild-built parts of online are now largely controlled by the engine configuration:  For
example, it is engine's `--enable-debug`/`--enable-dbgutil` (rather than online's `--enable-debug`)
that controls whether those parts are built in debug mode.  Similarly, these parts now use engine's
`loplugin` Clang compiler plugin rather than online's `coplugin`.  (The plan, again, is to combine
this further, and eventually end up with a single, coherent configuration for the whole system.)

All of this works for in-tree and out-of-tree builds (for both a combined build directory with the
engine build nested in the online one, as well as for two independent build directories and the
traditional usage of `--with-lo-builddir`).

# Linking

One area where Automake and gbuild widely differ in concepts is linking.  For one, while Automake
just lists all the source files that shall be included in an executable (and compiles each of those
source files afresh for each executable it ends up in), gbuild needs to model that with static
libraries.

And for another, gbuild carefully tries to restrict the set of exported symbols of its executables
and dynamic libraries to a minimum.  For example, it generally uses hidden symbol visibility as the
default.  The Automake approach, on the other hand, was to let the executables export as much as
possible (e.g., no hidden visibility as the default, `-Wl,-E` to add all symbols to an executable's
dynamic symbol table, linking static libraries as whole-archive), so that test code can access all
the innards of these executables.  Mimicking that in gbuild is a source of complications:

* `Executable_coolwsd` links `StaticLibrary_shared` as whole-archive, because some test code
(`unit-typing`, expecting to be able to run `dlopen`'ed within a `coolwsd` process) uses `KitQueue`
from `StaticLibrary_shared`, but which no code from `Executable_coolwsd` references.

* `StaticLibrary_PocoFoundation` bundles its own zlib (`adler32`, `crc32`, `deflate`, ...), while
the online executables also pull engine's `libzlib.a` transitively through `libpng`, so two copies
of every zlib symbol reach the link.  Normally harmless (whole-archived Poco defines them first, so
the engine copy is never pulled), but fatal under ASan, whose `__odr_asan_gen_*` indicators defeat
that lazy selection and drag both copies in as multiple-definition errors.  The proper fix will be
to de-bundle:  Drop the zlib objects from `engine/external/poco/StaticLibrary_PocoFoundation.mk` and
build Poco against the engine's `external/zlib`.  It is not self-contained, though: the Automake
`coolwsd-inproc` also links `libPocoFoundation.a` with a hand-ordered link line (`AM_LDFLAGS =
${PNG_LIBS} $(ZSTD_LIBS) $(ZLIB_LIBS)`) that assumes a self-contained Poco, so zlib would have to
move after the Poco libraries there too.

* Also, a few libraries that online's `configure` drops into the global `LIBS` are invisible to
gbuild and re-added per program:  `librt` (Poco `SharedMemory`'s `shm_open`/`shm_unlink` on glibc
before 2.34) on `coolwsd` and the `coolforkit` variants; and `libcap` (forkit/kit capability calls)
on the `coolforkit` variants, forwarded as `ONLINE.CAP_LIBS`, empty unless `configure` found it.

# TODO

* Switch the remaining parts of the top-level `Makefile.am` (i.e., `coolwsd-inproc` which in gbuild
cannot share translation units with `coolwsd` that want to differ in passed-in macro definitions).

* Switch the mobile apps, the CODA apps, and Wasm.

* Switch the C++ unit tests.

* Once there are no remaining uses of `engine/external/poco` from `Automake`,
`efbd551d499c84327a033a4d053e2d4d9d8c0023` "engine: build the POCO static libraries without the
debug STL ABI" can be reverted again.

* Merge `coplugin` (`clang/`) into `loplugin` (`engine/compilerplugins/`).
