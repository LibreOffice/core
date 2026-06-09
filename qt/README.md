<!-- build-doc-start -->
# Build Collabora Office

This is the Collabora Office Linux desktop app built on Qt6 WebEngine.

## Requirements

### Fedora

Tested on Fedora 43.

```bash
sudo dnf install autoconf automake cppunit-devel fontconfig-devel gcc gcc-c++ \
    git libcap-devel libpng-devel libtool libzstd-devel make npm openssl-devel \
    pam-devel perl-JSON-PP pkgconf-pkg-config python3-lxml \
    python3-polib qt6-linguist qt6-qtbase-devel qt6-qtwebengine-devel \
    qt6-qtwebsockets-devel
```

### Ubuntu

Tested on Ubuntu 24.04 LTS.

```bash
sudo apt install -y autoconf automake build-essential cmake fontconfig git \
    libcap-dev libcppunit-dev libpam0g-dev libpng-dev libqt6websockets6-dev \
    libssl-dev libtool libzstd-dev npm pkg-config python3-lxml python3-polib \
    qt6-base-dev qt6-tools-dev qt6-tools-dev-tools qt6-webengine-dev
```

### Debian

Tested on Debian 13 (Trixie).

```bash
sudo apt install -y autoconf automake build-essential fontconfig git \
    libcap-dev libcppunit-dev libpam0g-dev libpng-dev \
    libqt6websockets6-dev libssl-dev libtool libzstd-dev npm pkg-config \
    python3-lxml python3-polib qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
    qt6-webengine-dev
```

### Arch Linux

```bash
sudo pacman -Syu autoconf automake base-devel cppunit fontconfig \
    git libcap libpng libtool npm openssl pam perl-json-pp pkgconf \
    python-lxml python-polib qt6-base qt6-tools qt6-webengine \
    qt6-websockets zstd
```

### openSUSE

Tested on openSUSE Leap 16.0.

```bash
PYVER=$(python3 -c 'import sys; print(f"python{sys.version_info.major}{sys.version_info.minor}")')
sudo zypper install autoconf automake cppunit-devel fontconfig-devel gcc-c++ \
    git libcap-devel libopenssl-devel libpng16-compat-devel libpng16-devel \
    libtool libzstd-devel make npm-default pam-devel pkgconf \
    ${PYVER}-lxml ${PYVER}-polib qt6-base-devel qt6-tools-linguist \
    qt6-webenginecore-devel qt6-webenginewidgets-devel qt6-websockets-devel
```

### General notes

A C++ compiler with full C++20 support is required, including `std::format` (GCC 13+ or Clang 17+).

POCO is built as part of the engine (`engine/`) and picked up from its workdir
automatically, so it no longer needs to be installed as a distro package.

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

The engine is the `engine/` subdirectory of the monorepo. For dependency installation, refer to https://wiki.documentfoundation.org/Development/BuildingOnLinux if needed.

If you want a localized (translated) user interface, also clone the translations repository into `engine/translations` from the top of the clone before building; the engine's `--with-lang` picks up the `.po` files from there:

```bash
git clone https://gerrit.collaboraoffice.com/translations engine/translations
```

```bash
cd engine
```

```bash
./autogen.sh --with-distro=CPLinux-LOKit --without-package-format --with-system-nss
make
```

> For debug builds, add `--enable-dbgutil` to the autogen line.

Once done, step back to the top of the monorepo:
```bash
cd ..
```

## Build Collabora Office

From the top of the `collabora-office` clone. The engine is in `engine/`, where
configure looks by default, so no paths need to be passed:

```bash
./autogen.sh
./configure --enable-qtapp --enable-debug
make -j $(nproc)
```

This produces the `coda-qt` executable in `qt/`.

## Running

```bash
./qt/coda-qt                         # start without a document
./qt/coda-qt ../test/data/hello.odt  # open a file
```

### Debug with Chromium DevTools

```bash
export QTWEBENGINE_REMOTE_DEBUGGING=3311
```

Then run `coda-qt` and open [http://localhost:3311](http://localhost:3311)

## Flatpak

Install the runtimes:
```bash
flatpak install org.kde.Sdk//6.10 \
                org.kde.Platform//6.10 \
                org.freedesktop.Sdk.Extension.node20//25.08 \
                io.qt.qtwebengine.BaseApp//6.10
                io.qt.qtwebengine.BaseApp.Debug//6.10
```

Build and export to a local repo:
```bash
flatpak-builder --repo=repo --force-clean --ccache build-dir \
    qt/flatpak/com.collaboraoffice.Office.json
```

Create a `.flatpak` bundle from the repo:
```bash
flatpak build-bundle repo CollaboraOffice.flatpak com.collaboraoffice.Office
```

Or install directly from the repo:
```bash
flatpak --user remote-add --no-gpg-verify --if-not-exists co-local repo
flatpak --user install co-local com.collaboraoffice.Office
```

### Debug symbols

To include debug symbols in the build, add `--keep-build-dirs` and export the debug extension:
```bash
flatpak-builder --repo=repo --force-clean --ccache --keep-build-dirs build-dir \
    qt/flatpak/com.collaboraoffice.Office.json
flatpak build-bundle repo CollaboraOffice.Debug.flatpak \
    com.collaboraoffice.Office.Debug --runtime
```

## Snap

Collabora Office is also packaged as a snap. The `snap/snapcraft.yaml` builds the
engine and the Qt app together, so it does not need the separate engine build
above. From the top of the monorepo, with
[snapcraft](https://snapcraft.io/docs/installing-snapcraft) installed (it builds
in an LXD container by default):

```bash
snapcraft
```

This produces a `collabora-office_<version>_<arch>.snap`. Install your locally
built snap with:

```bash
sudo snap install --dangerous ./collabora-office_*.snap
```

## Pre-built download

If you just want a pre-built package instead of compiling:

* **Snap**, from the `latest/edge` channel:
    ```bash
    sudo snap install collabora-office --channel=latest/edge
    ```
* **Flatpak** — download the **Collabora Office Linux Flatpak snapshots**:
    👉 https://www.collaboraoffice.com/downloads/Collabora-Office-Linux-Snapshot/

# Release helpers

`tools/add-release.py` is a script that allow adding a release to the
appstream file prior to tagging.

Usage

```shell
$ ./tools/add-release.py 25.04.7.8.1 com.collaboraoffice.Office.metainfo.xml
```

This will add release `25.04.7.8.1` with today's date in the appstream
file `com.collaboraoffice.Office.metainfo.xml`

Options are:

- `-d` or `--date` to pass the date string to use.
- `-c` or `--changelog` to use the changelog entry from a file.

The expected changelog data should follow appstream `description`
element.

```xml
<description>
<p>Foo</p>
<ul>
<li>Something</li>
</ul>
</description>
```

# Debugging

Some useful environment variable settings are `QT_LOGGING_RULES=` to always get JS console warnings
and errors (even when running without a `--debug` command line argument) and
`QT_MESSAGE_PATTERN=%{if-category}%{category}: %{endif}%{file}:%{line}: %{message}` to get JS
console warnings and errors that contain file and line information.
<!-- build-doc-end -->
