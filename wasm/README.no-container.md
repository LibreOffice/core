<!---
NOTE: This file describes how to build Collabora Online as WASM
without using the Allotropia container.
-->

# Collabora Online as WASM (COWASM)

Before building Collabora Online as WASM you need to build three
dependencies: CollaboraOffice core, Poco, and zstd.

The toolchain used is Emscripten. Versions >= 3.1.58 should generally work.

Below we assume that the Emscripten environment is already set up,
that you have sourced the emsdk_env.sh file in your shell.

## Build CollaboraOffice core

Use the master branch of CollaboraOffice core. The feature/wasm branch is
not needed any longer, all commits to that branch are currently also
present in the master branch.

See the static/README.wasm.md file in CollaboraOffice core, especially the
section "Building headless CollaboraOffice as WASM for use in another
product".

## Build other Online dependencies

### zstd

Build libzstd  with assembly code disable, and using the Makefile (didn't try its other build systems):

    tar -xzvf ~/Downloads/zstd-1.5.2.tar.gz
    cd zstd-1.5.2/
    emmake make CC='emcc -pthread' CXX='em++ -pthread' lib-mt V=1 ZSTD_NO_ASM=1 PREFIX=/opt/zstd.emsc.3.1.30
    (cd lib && emmake make install-static install-includes ZSTD_NO_ASM=1 PREFIX=/opt/zstd.emsc.3.1.30)

This will install the zstd headers and libraries in `/opt/zstd.emsc.3.1.30`.

### Poco

Poco requires two patches plus renaming one source file (which
actually is from expat but Poco includes). Note that the header
Poco/Platform.h maps EMSCRIPTEN to POCO_OS_LINUX. It has both
Makefiles and CMake but I couldn't get CMake to work. Online requires
a single include directory so "make install" must be used.

Here we assume that the online repo is cloned at `$HOME/lo/online`,
adapt as necessary.

    tar -xzvf ~/Downloads/poco-1.12.4-release.tar.gz
    cd poco-poco-1.12.4-release
    patch -p1 < $HOME/lo/online/wasm/poco-1.12.4-emscripten.patch
    mv XML/src/xmlparse.cpp XML/src/xmlparse.c
    patch -p0 < $HOME/lo/online/wasm/poco-no-special-expat-sauce.diff
    emconfigure ./configure --static --no-samples --no-tests --omit=Crypto,NetSSL_OpenSSL,JWT,Data,Data/SQLite,Data/ODBC,Data/MySQL,Data/PostgreSQL,Zip,PageCompiler,PageCompiler/File2Page,MongoDB,Redis,ActiveRecord,ActiveRecord/Compiler,Prometheus
    emmake make CC=emcc CXX=em++  CXXFLAGS="-DPOCO_NO_LINUX_IF_PACKET_H -DPOCO_NO_INOTIFY -pthread -s USE_PTHREADS=1 -fwasm-exceptions"
    make install INSTALLDIR=/opt/poco.emsc.3.1.30

This will install into `/opt/poco.emsc.3.1.30`.

## Build Online itself

    # Update the directories in the command below to match your system.

    ./autogen.sh
    emconfigure ./configure --disable-werror --with-lokit-path=/home/tml/lo/core-cool-wasm/include --with-lo-path=/home/tml/lo/core-cool-wasm/instdir --with-lo-builddir=/home/tml/lo/core-cool-wasm --with-zstd-includes=/opt/zstd.emsc.3.1.30/include --with-zstd-libs=/opt/zstd.emsc.3.1.30/lib --with-poco-includes=/opt/poco.emsc.3.1.30/include --with-poco-libs=/opt/poco.emsc.3.1.30/lib --host=wasm32-local-emscripten
    emmake make CC=emcc CXX=em++

## Running WASM Online

There are two modes in which you can use the Wasm binary, either served from a COOL instance or
served stand-alone from a plain web server.

The below steps should get one up and running.

### Served from a COOL instance

Once the build is done, copy the full (source and) build directory to a safe locataion.
E.g. cp -a . ../wasm
Next, re-configure Online and rebuild with normal config/settings (i.e. without WASM) plus
--enable-wasm-fallback=... specifying an absolute path to the above WASM build directory.

You may need to increase
MaxFileSizeToCacheInBytes in wsd/FileServer.cpp to avoid
```
[ coolwsd ] ERR  Failed to read file [.../browser/dist/wasm/online.wasm] or is too large to cache and serve| wsd/FileServer.cpp:1285
```
failures.

Alternatively to re-configuring the existing Online build directory, you may have opted to build
WASM in a separate directory.

Now point your browser to some https://localhost:9980/browser/c85d8681f3/wasm.html?file_path=/some/existing/document
(where /some/existing/document denotes some exisiting document made available by that server).

### Served stand-alone from a plain web server

Once the build is done, you can serve the browser/dist tree from a web server (e.g., via `emrun
--no-browser browser/dist/cool.html`) and point your browser at that served cool.html page with a
file_path=/some/exisiting/document query paramter, where /some/exisiting/document is the absolute
path of an exisiting document in the Emscripten file system (cf. online's
--with-wasm-additional-files configure option; e.g., some
<http://localhost:6931/cool.html?file_path=/TODO/example.odt> for the emrun example).
