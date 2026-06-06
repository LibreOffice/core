<!---
NOTE: This file describes how to build Collabora Online as WASM
without using the Allotropia container.
-->

# Collabora Online as WASM (COWASM)

Before building Collabora Online as WASM you only need to build the
CollaboraOffice core. (POCO, zstd and libpng are built as part of the
CollaboraOffice core engine, so they no longer need to be built separately.)

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

## Build Online itself

    # Update the directories in the command below to match your system.

    ./autogen.sh
    emconfigure ./configure --disable-werror --with-lokit-path=/home/tml/lo/core-cool-wasm/include --with-lo-path=/home/tml/lo/core-cool-wasm/instdir --with-lo-builddir=/home/tml/lo/core-cool-wasm --host=wasm32-local-emscripten
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
