## Where POCO comes from

POCO is no longer a separately-installed dependency. It is built as one of the
engine's third-party libraries (`engine/external/poco/`), unconditionally,
alongside zstd / libpng / openssl etc. Only the components the code uses are
built, as static libraries:

+ always: PocoFoundation, PocoXML, PocoJSON, PocoUtil, PocoNet
+ server platforms only (`ENABLE_OPENSSL=TRUE` and `$(OS)` not one of ANDROID /
  iOS / EMSCRIPTEN / WNT / MACOSX): PocoCrypto, PocoNetSSL

The bundled zlib, pcre2, utf8proc, pdjson and double-conversion copies are
compiled into those archives; expat and OpenSSL are taken from the engine's own
externals (so `PocoXML` references the engine's expat).

PocoCrypto/PocoNetSSL need OpenSSL and are only used by the COOL server (SSL)
build. None of the CODA app targets (Android, iOS, Emscripten, Windows, macOS)
build with SSL or link them, and some ship a reduced OpenSSL (e.g. wasm's lacks
OCSP, which PocoNetSSL uses), so `Module_poco.mk` excludes those `$(OS)` values
and builds the two libraries only on the server platforms (Linux/FreeBSD/...)
that have OpenSSL. COOL links `-lPocoNetSSL -lPocoCrypto` only when `ENABLE_SSL`
is true. The two static libraries compile against OpenSSL **headers** only
(`use_externals openssl_headers` — an archive does not link), so the engine's
`workdir/UnpackedTarball/openssl/include` is on the include path; the actual
`libssl`/`libcrypto` are linked by COOL itself. (Using the plain `openssl`
external instead would miss that `-I`, since for an internally-built OpenSSL the
include path is only provided by `openssl_headers`; it merely appeared to work
where system OpenSSL headers happened to be in the default search path.)

After an engine build the artefacts live at:

+ headers: `<core-build>/workdir/UnpackedTarball/poco/include`
+ archives: `<core-build>/workdir/LinkTarget/StaticLibrary/libPoco*.a`
  (plus `libexpat.a` in the same directory)

## Fetching the tarball / bumping the version

POCO is not on the LibreOffice tarball mirror, so it has its own fetch bucket in
`engine/Makefile.fetch` (`fetch_POCO_TARBALLS`), downloaded from
`https://pocoproject.org/releases/poco-<version>`. Bumping the POCO version
therefore means editing three places: `engine/download.lst` (`POCO_TARBALL` +
`POCO_SHA256SUM`), the version in the `Makefile.fetch` download URL, and the
version in `cool-sbom-template.spdx.json`. (`make fetch-names` must list the
tarball, otherwise the CI tarball cache via `bin/cached-fetch.sh` won't include
it and the build fails with "No rule to make target .../poco-...tar.bz2".)

## How configure finds it

`configure` looks for POCO in the engine workdir first, then falls back to a
system POCO. The engine build tree is the one `--with-lo-builddir` points at,
defaulting to `engine` for in-tree builds. So a normal in-tree build needs no
POCO options at all.

`--with-poco-includes` and `--with-poco-libs` are **obsolete**: they are
accepted but ignored (with a warning). There is no longer any `/opt/poco`.

## Packaging builds (rpm/deb) — important

The `coolwsd-*.tar.gz` source tarball does **not** contain the engine, so a
package build cannot rely on the default `engine` build dir. The build host
does have the core build tree (the same one used for zstd/png/openssl), so
point configure at it explicitly and drop the old POCO options. In the
`config_options` passed to the spec:

+ remove: `--with-poco-includes=/opt/poco/include --with-poco-libs=/opt/poco/lib`
+ add:    `--with-lo-builddir=/path/to/.../engine`

This is safe: the spec already passes `--with-lokit-path=bundled/include` and
`--with-lo-path=/opt/collaboraoffice`, which override the lokit/lo-path defaults
that `--with-lo-builddir` would otherwise set, so `--with-lo-builddir` ends up
feeding only the POCO lookup. POCO and its expat are then linked statically from
the engine workdir, so neither `/opt/poco` nor a system expat-devel package is
needed.

Do **not** instead inject the paths via `CPPFLAGS`/`LDFLAGS`: that leaves
`POCOLIB` unset, configure then does not add `-lexpat`, and because the engine
builds `PocoXML` unbundled the final link fails on `XML_Parse`. Use
`--with-lo-builddir`, which sets `POCOLIB` and wires up `-lexpat` from the same
directory.

## Windows (CODA-W) Visual Studio projects

On Windows, PocoUtil additionally compiles `WinService`, `WinRegistryKey` and
`WinRegistryConfiguration` (`ifeq ($(OS),WNT)` in `StaticLibrary_PocoUtil.mk`);
`ServerApplication` references `WinService` for service registration.

Windows has no `<syslog.h>`, so Foundation compiles `WindowsConsoleChannel`
(POCO's `ConsoleChannel`/`ColorConsoleChannel` on Windows) instead of
`SyslogChannel` there (`ifeq ($(OS),WNT)` in `StaticLibrary_PocoFoundation.mk`).
`EventLogChannel` is deliberately **not** built: it needs a message-compiler-
generated `pocomsg.h` and is unused by COOL/CODA, so
`external/poco/no-eventlogchannel.patch` drops its include and its
`LoggingFactory` registration on Windows.

On Windows POCO defaults to a DLL build (`Foundation.h` defines `POCO_DLL`
unless `POCO_STATIC` is set), which marks the `*_API` symbols `__declspec(dllimport)`
and makes the `.cpp` files fail to define their static members (C2491). So all
the engine's POCO static libraries are compiled with `-DPOCO_STATIC` (a no-op on
non-Windows), and the consuming Windows projects define `POCO_STATIC` too.

The MSVC POCO headers auto-link via `#pragma comment(lib, "Poco...md.lib")`
(name decorated with runtime/debug suffix). The engine builds **undecorated**
static libs (`PocoFoundation.lib` etc.), so the projects under `windows/coda/`
(`CODA.vcxproj`, `COConvert.vcxproj`) now:

+ define `POCO_NO_AUTOMATIC_LIBS` (so no decorated auto-link is emitted), and
+ list the engine's Poco libs and `expat.lib` explicitly in
  `AdditionalDependencies`, next to `zlib.lib`/`zstd.lib`/`libpng.lib`.

CODA-W is a `mobile_app`, so `ENABLE_SSL` is false: it links PocoNet, PocoUtil,
PocoJSON, PocoXML, PocoFoundation and expat — **not** PocoNetSSL/PocoCrypto, and
no OpenSSL.

## Other app targets (iOS, macOS, Android, wasm)

These are all `mobile_app` builds (no SSL), so each links PocoNet, PocoUtil,
PocoJSON, PocoXML, PocoFoundation and the engine's expat, taken from the engine
workdir (`@POCOINCLUDE@`/`@POCOLIB@`, derived from `--with-lo-builddir`):

+ iOS: `ios/Mobile.xcodeproj` — dropped the old debug `d` suffix
  (`-lPocoFoundationd` -> `-lPocoFoundation`) and added `-lexpat`. The engine's
  iOS platform defines a global `-DIOS` (`solenv/gbuild/platform/iOS.mk`), which
  collides with POCO using `IOS` as a C++ identifier in its stream headers; the
  POCO static libraries are therefore compiled with `-UIOS` (POCO never uses the
  `IOS` macro, so this is a no-op on every other platform).
+ macOS: `macos/coda` and `macos/coolforkit` xcode projects gained `-lexpat`
  (`macos/coolwsd` has empty `OTHER_LDFLAGS`, nothing to do). macOS builds
  against the in-tree engine like the desktop build, so configure derives its
  POCOINCLUDE/POCOLIB from `--with-lo-builddir` (default `engine`), not from
  `$LOBUILDDIR` (which is only set for the iOS/Android/Windows/wasm cross
  builds).
+ Windows: PocoNet emulates epoll with POCO's bundled `wepoll`
  (SocketImpl/PollSet `#include "wepoll.h"`), so on Windows
  `StaticLibrary_PocoNet.mk` adds `-Idependencies/wepoll/src` and compiles
  `dependencies/wepoll/src/wepoll.c`. Other platforms use real epoll/poll.
+ Android: `android/lib/src/main/cpp/CMakeLists.txt.in` — added `libexpat.a`.
  The Foundation static library is also compiled with `-DPOCO_NO_SHAREDMEMORY`
  on Android (`ifeq ($(OS),ANDROID)` in `StaticLibrary_PocoFoundation.mk`)
  because Bionic has no POSIX `shm_open`/`shm_unlink`.
+ wasm: `wasm/Makefile.am` — added `libexpat.a`. Emscripten looks like Linux to
  POCO but lacks some Linux APIs, so the Foundation/Net libraries need two
  tweaks (most of the older `wasm/poco-*-emscripten.patch` is already upstream in
  1.15.3): `external/poco/emscripten-no-epoll.patch` excludes Emscripten from
  `POCO_HAVE_FD_EPOLL` (PollSet/SocketImpl fall back to `select()`),
  `external/poco/emscripten-no-prctl.patch` skips `<sys/prctl.h>`/`prctl` (used
  for thread naming on Linux; a no-op on Emscripten), and Foundation is compiled
  with `-DPOCO_NO_INOTIFY` on Emscripten (no `<sys/inotify.h>`).
  PocoCrypto/PocoNetSSL are not built for Emscripten anyway (see above).

In all of them the previously-linked `libPocoEncodings` was removed: the engine
does not build the Encodings component (nothing in the code uses it).

## Packaging / CI pipelines

Every pipeline that used to build POCO separately and pass `--with-poco-*` was
migrated to the engine workdir:

+ **snap** (`snap/snapcraft.yaml`): dropped the `poco:` part and `after: [poco]`;
  the engine is built in the same part (`./engine`), so configure auto-detects
  POCO. Engine uses `--with-system-expat`, so `-lexpat` resolves to the runtime
  expat (already a stage-package).
+ **docker/from-source** (`build.sh`, `build-alpine.sh`,
  `from-source-gh-action/build.sh` + `Dockerfile`): dropped the separate POCO
  build / `libpoco-dev`; the engine is built in-tree at `online/engine`, so
  `--with-lo-builddir` defaults to `engine` and POCO is found. If a build uses a
  prebuilt `ENGINE_ASSETS` tarball instead of building the engine, that tarball
  must now also contain `workdir/UnpackedTarball/poco/include` and
  `workdir/LinkTarget/StaticLibrary/libPoco*.a`.
+ **flatpak** (`qt/flatpak/com.collaboraoffice.Office.json`): the `libreoffice`
  (engine) module's build dir is not visible to the later `collabora-office`
  module, so it now stages POCO into `/app/core/workdir/...` (matching the
  engine workdir layout, alongside the existing COKit copy), the standalone
  `poco` module was removed, and COOL configures with
  `--with-lo-builddir=/app/core`. The flatpak build sandbox has **no network**,
  so (like every other engine tarball) `poco-1.15.3-all.tar.bz2` is declared as
  a pre-fetched `file` source on the `libreoffice` module (`dest:
  external/tarballs`); flatpak-builder downloads it before the offline build.
+ **wasm** (`wasm/README*`): the example commands already pass
  `--with-lo-builddir`, so the redundant `--with-poco-*` (and the POCO bind
  mount) were dropped.

The rpm/deb package jobs (out of tree, building from the `coolwsd-*.tar.gz`
source tarball) must add `--with-lo-builddir=<core-build>/engine` and drop their
`--with-poco-*`, as described above.

The CodeQL workflow (`.github/workflows/codeql-analysis.yml`) is a special case:
it consumes a prebuilt engine-assets tarball (no engine workdir) and still
installs a system POCO, so it relies on configure's **system-POCO fallback**
(engine-workdir lookup fails, system POCO is used). It keeps working unchanged;
to migrate it fully, the engine-assets tarball would need to ship the POCO
workdir. The SBOM template (`cool-sbom-template.spdx.json`) records the bundled
POCO version (now 1.15.3).
