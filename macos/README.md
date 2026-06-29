<!-- build-doc-start -->
# Build Collabora Office

This is the Collabora Office macOS desktop app (`macos/`).

## Requirements

* Homebrew - install Homebrew from https://brew.sh/ to make getting all the dependencies easier.
* Node.js — needed for building the JavaScript parts in `browser/`. Install
  Node.js (20.x) by downloading the macOS `.pkg` from the `Prebuilt installer`
  tab at https://nodejs.org/en/download. It provides the `npm` and `node`
  commands.
* Install various build dependencies by running
    ```bash
    brew install automake autoconf libtool make gperf meson ninja pkg-conf
    ```
* Python 3 plus the `lxml` and `polib` modules, used by the Python build
  helpers:
    ```bash
    brew install python3
    /opt/homebrew/bin/pip3 install --break-system-packages lxml polib
    ```
    Modern pip (23+) treats the Homebrew Python environment as externally
    managed (PEP 668), so `--break-system-packages` is required; alternatively,
    install the two modules into a virtual environment.
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

    # See footnote [0]
    PKG_CONFIG=/opt/homebrew/bin/pkg-config
    # See footnote [1]
    GNUMAKE=gmake
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

Save the above as `engine/autogen.input`, then run the build from within
`engine/`:

```bash
cd engine
./autogen.sh
make
cd ..
```

The first build takes a long time and downloads a number of third-party
tarballs. When it finishes, the built app bundle is under `engine/instdir/`.

## Build Collabora Office

### Configure

Run this from the top of the monorepo (one level up from `engine/`):

    ./autogen.sh && ./configure \
    --enable-macosapp \
    --enable-experimental \
    --with-app-name="Collabora Office" \
    --with-app-package-name=com.yourpackage.name \
    --with-vendor="Your Name" \
    --with-lo-path=engine/instdir/CollaboraOffice.app

POCO, zstd and libpng are built as part of the engine and taken from its
workdir, so they no longer need to be installed (via Homebrew or otherwise) or
passed on the configure line.

With the `CPMacOS-LOKit` distro above, the engine produces
`CollaboraOffice.app`. If you change the branding/product name, adjust the
bundle name in `--with-lo-path` to match what ends up in `engine/instdir/`.

### Build the JavaScript bits

Just run `make` from the terminal.

### Build the Desktop Edition

Open the `macos/coda/coda.xcodeproj` project in Xcode and choose `Build and Run current scheme`.

That's it!

## Pre-built download

If you just want a pre-built macOS version instead of building locally, please use
the **TestFlight preview build**:
👉 https://testflight.apple.com/join/sbkwMzMt

## Footnotes

[0] Without PKG_CONFIG=... in autogen.input, our configure.ac gets a little confused and tries to build internal python, which is
    both unwanted, and will fail.

[1] by default, autogen.sh finds /usr/bin/make instead of the newer version of make that brew installs (brew calls it gmake),
    and complains that the version is too old.

<!-- build-doc-end -->
