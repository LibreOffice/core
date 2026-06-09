<!-- build-doc-start -->
# Build Collabora Office

This is the Collabora Office Windows desktop app (`windows/`).

## Requirements

Collabora Office for Windows is built with **Visual Studio 2026**, plus a
Unix-style toolchain reached through WSL and Git Bash. The `windows/.config/`
directory in the repo ships
[WinGet configuration](https://learn.microsoft.com/windows/package-manager/configuration/)
files that set up a build machine for you; apply them, or follow them as a
checklist if you prefer to install things by hand.

### Visual Studio 2026

Install Visual Studio 2026 Community with the "Desktop development with C++" and
".NET desktop development" workloads. The exact component list is pinned in
`windows/.config/2026.vsconfig`. To install Visual Studio and the required
components in one step:

```
winget configure windows/.config/configuration.winget
```

(`windows/.config/2026_cross.vsconfig` lists the extra components for
cross-compiling to ARM64.)

### Build tools and WSL

Two more WinGet configurations finish the setup. The first must be run **as
administrator** — it downloads `jom`, `make`, `clang-format`, `pkgconf` and
Strawberry Perl Portable, makes sure WSL is installed, and enables Developer
Mode (so `tar` can create native symlinks while unpacking tarballs):

```
winget configure windows/.config/admin_deps.winget
```

The second runs as your normal user. It places `make`, `jom`, `clang-format`
and `pkgconf` in `~\bin`, extracts Strawberry Perl Portable to `~\co\spp`, sets
the git options the build needs (`protocol.version=2`, `core.autocrlf=false`)
and `MSYS=winsymlinks:nativestrict`, drops a sample `~\co\autogen.input`, and
installs an Ubuntu 24.04 WSL distro with the packages the engine and online
build need (autotools, `build-essential`, the Python `lxml`/`polib` helpers, and
Node.js 20 from NodeSource, since 24.04 only ships Node.js 18):

```
winget configure windows/.config/user_steps.winget
```

## Clone the monorepo

All the source code now lives in a single Gerrit monorepo; the former Collabora Office core is the `engine/` subdirectory of the `online` repo, so there is no separate repository to clone any more. Code review happens on [Gerrit](https://gerrit.collaboraoffice.com/), not GitHub pull requests; see the [first contribution guide](https://forum.collaboraonline.com/t/your-first-pull-request/41) for the full workflow.

From a **Git Bash** terminal, clone with protocol v2 and CRLF translation turned off:

```bash
git clone --config protocol.version=2 --config core.autocrlf=false \
    ssh://YOUR_USERNAME@gerrit.collaboraoffice.com:29418/online collabora-office
cd collabora-office
```

(If you applied `user_steps.winget`, those two git options are already set globally.) For an anonymous read-only clone, use `https://gerrit.collaboraoffice.com/online` in place of the SSH URL.

## Build the engine

If you want a localized (translated) user interface, first clone the translations repository into `engine/translations` from the top of the clone; the engine's `--with-lang` picks up the `.po` files from there:

```bash
git clone https://gerrit.collaboraoffice.com/translations engine/translations
```

Change to the `engine` subdirectory and create an `autogen.input`.
`user_steps.winget` writes a template to `~\co\autogen.input`; a typical Visual
Studio 2026 configuration looks like:

```
--with-distro=CODAWindows
--host=x86_64-pc-cygwin
--with-visual-studio=2026
--with-strawberry-perl-portable=C:\Users\<you>\co\spp
--with-external-tar=C:\Users\<you>\co\co-externaltar
```

Adjust the paths to match where Strawberry Perl Portable was extracted and where
you keep downloaded tarballs. Then run autogen through WSL and build with `make`
from Git Bash:

```bash
cd engine
wsl ./autogen.sh
make
```

If you build the Collabora Office project in Visual Studio in the Debug
configuration, you *must* use an engine build with either `--enable-dbgutil` or
`--enable-msvc-debug-runtime`.

You will not end up with a runnable Collabora Office Classic — running
`instdir/program/soffice.exe` just prints "no suitable windowing system found,
exiting". You can attempt `make check`, but it will probably hit some false
positives.

## Build Collabora Office

### Configure

From the top of the clone, run autogen and configure through WSL. POCO, libpng
and zstd are built as part of the engine and linked from its workdir by the
Visual Studio project, so they no longer need to be built or passed separately
(zlib likewise comes from the engine):

```bash
wsl --exec bash -c "./autogen.sh"
wsl --exec bash -c "./configure --enable-windowsapp --with-app-name='Collabora Office' --with-lo-builddir='/mnt/c/Users/<you>/collabora-office/engine' --with-lo-path='C:\Users\<you>\collabora-office\engine\instdir' --with-info-url=https://example.com/coda/info.html"
```

`--with-lo-builddir` is a WSL (Unix) path, while `--with-lo-path` is a Windows
path — this matters. Change `--with-info-url` as appropriate; that is the web
page shown when clicking the leftmost button in the toolbar.

### Build the JavaScript bits

```bash
wsl --exec bash -c "(cd browser && make clean && make)"
```

### Build the app

From a **Developer Command Prompt for VS 2026**, build the solution (`/restore`
fetches the NuGet/.NET dependencies):

```
msbuild /restore /p:Configuration=Release /p:Platform=x64 windows\coda\CODA.sln
```

To clean first, run the same command with `/t:Clean`. You can also build from a
WSL shell, as long as PATH has what is needed and you quote the msbuild
parameters.

## Pre-built download

If you just want a **pre-built snapshot for Windows**, you can download it here:
👉 https://www.collaboraoffice.com/downloads/Collabora-Office-Windows-Snapshot/
<!-- build-doc-end -->
