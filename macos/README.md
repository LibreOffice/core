<!-- build-doc-start -->
# Build Collabora Office

This is the Collabora Office macOS desktop app (`macos/`).

## Requirements

* Node.js — needed for building the JavaScript parts in `browser/`. Install
  Node.js (20.x) by downloading the macOS `.pkg` from the `Prebuilt installer`
  tab at https://nodejs.org/en/download. It provides the `npm` and `node`
  commands.
* libtool — for `./configure`:
    ```bash
    brew install libtool
    ```
* The Python helpers used by the build:
    ```bash
    /opt/homebrew/bin/pip3 install --break-system-packages lxml
    /opt/homebrew/bin/pip3 install --break-system-packages polib
    ```
* The Command Line Tools for Xcode:
    * `xcode-select --install`
    * Afterwards you might need to update them in System Settings > General > Software Update.

## Clone the monorepo

All the source code now lives in a single Gerrit monorepo; the former Collabora Office core is the `engine/` subdirectory of the `online` repo, so there is no separate repository to clone any more. Code review happens on [Gerrit](https://gerrit.collaboraoffice.com/), not GitHub pull requests; see the [first contribution guide](https://forum.collaboraonline.com/t/your-first-pull-request/41) for the full workflow.

For an anonymous read-only clone:
```bash
git clone https://gerrit.collaboraoffice.com/online collabora-office
cd collabora-office
```

If you have a Gerrit account and plan to push changes for review, clone over SSH instead:
```bash
git clone ssh://YOUR_USERNAME@gerrit.collaboraoffice.com:29418/online collabora-office
cd collabora-office
```

## Build the engine

Move into the `engine/` subdirectory of the monorepo and build the `main` branch with the following `autogen.input`.

If you want a localized (translated) user interface, also clone the translations repository into `engine/translations` from the top of the clone before building, and replace `--without-lang` with `--with-lang` (see the `autogen.input` below); the engine picks up the `.po` files from there:

```bash
git clone https://gerrit.collaboraoffice.com/translations engine/translations
```

`autogen.input`:

    # Distro
    --with-distro=CPMacOS-LOKit
    --disable-mergelibs

    # Overrides for the debug builds
    --enable-debug
    #--enable-dbgutil

    --enable-werror
    --enable-symbols
    # For an en-US-only build (no localizations):
    --without-lang
    # For a localized UI, drop the line above and list the languages instead, e.g.:
    #--with-lang=de fr es

For dependency installation, refer to https://wiki.documentfoundation.org/Development/BuildingOnLinux

## Build Collabora Office

### Configure

Run this from the top of the monorepo (one level up from `engine/`):

    ./autogen.sh && ./configure \
    --enable-macosapp \
    --enable-experimental \
    --with-app-name="Collabora Office" \
    --with-app-package-name=com.yourpackage.name \
    --with-vendor="Your Name" \
    --with-lo-path=engine/instdir/your-built-lo.app

POCO, zstd and libpng are built as part of the engine and taken from its
workdir, so they no longer need to be installed (via Homebrew or otherwise) or
passed on the configure line.

Adjust `your-built-lo.app` to match the app bundle name produced by your engine
build.

### Build the JavaScript bits

Just run `make` from the terminal.

### Build the Desktop Edition

Open the `macos/coda/coda.xcodeproj` project in Xcode and choose `Build and Run current scheme`.

That's it!

## Pre-built download

If you just want a pre-built macOS version instead of building locally, please use
the **TestFlight preview build**:
👉 https://testflight.apple.com/join/sbkwMzMt
<!-- build-doc-end -->
