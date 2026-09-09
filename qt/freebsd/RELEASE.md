# Collabora Office on FreeBSD: release process

This document describes how Collabora Office, the Qt desktop app built from the online monorepo, is released for FreeBSD through the official FreeBSD ports tree. It covers the concepts a reader without FreeBSD background needs, the one-time work for the first release, and the routine for every later release. Everything described here is implemented in the `qt/freebsd/` directory of the online repository and in `engine/distro-configs/CPFreeBSDQt.conf`, and every command below has been run on FreeBSD 15.1 with the port as it stands.

## 1. FreeBSD concepts in five minutes

**Base system and ports.** FreeBSD separates the operating system, called the base system, from third-party software. Third-party software is installed under `/usr/local` and comes from the ports collection. The ports tree is a git repository (https://cgit.freebsd.org/ports/) of about 36000 small directories, one per piece of software, organized as `category/name`, for example `editors/libreoffice`. Our port will be `editors/collabora-office`.

**A port.** A port is a recipe, not the software itself. It consists of a `Makefile` written for BSD make that declares the source tarballs to fetch, the dependencies, the build steps and the install steps; a `distinfo` file with the size and SHA256 of every tarball; a `pkg-descr` with a short description; and a `pkg-plist` listing every installed file. The ports framework (`/usr/ports/Mk`) provides the machinery: it fetches the tarballs, verifies checksums, extracts, patches, builds, installs into a staging directory, compares the staged files against `pkg-plist`, runs quality checks and finally produces a binary package.

**Packages and pkg.** The binary result of a port is a `.pkg` file, installed and managed by the `pkg` tool, the FreeBSD equivalent of apt or dnf. Users normally never build ports; they run `pkg install collabora-office` and get the package from the FreeBSD package repositories, which the FreeBSD project's own build cluster builds from the ports tree.

**Quarterly and latest.** The FreeBSD package builders publish two repositories. `latest` follows the head of the ports tree and is rebuilt every few days. `quarterly` is a branch of the ports tree cut every three months that receives only security and bug fixes; most desktop users are on quarterly by default. A new port lands in `latest` within days of being committed and appears in `quarterly` at the next branch point, or earlier if a committer merges it on request.

**No binary compatibility across the tree.** FreeBSD packages are linked against the exact library versions of the package set they were built with. When a library such as Qt or liborcus is updated in the ports tree, everything depending on it is rebuilt by the FreeBSD builders. This is the reason for going through the official ports tree instead of publishing our own package: a package we host ourselves would break every time one of its seventy dependencies changed, and we would have to rebuild it for every branch, release and architecture ourselves.

**poudriere.** poudriere is the tool the FreeBSD project and all serious port maintainers use to build packages. It creates a clean FreeBSD installation in a jail (a lightweight container) with no network access during the build, installs exactly the declared dependencies into it, builds the port, and runs a set of quality checks. A port that passes `poudriere testport` is considered correct; a build that only worked on a developer's machine is not, and this is not a formality: the first three jail runs of our port each failed on something no local build could have caught, because the developer machine happened to have the missing pieces installed.

**Maintainer.** Every port has a maintainer address. The maintainer receives bug reports and is expected to respond within two weeks; otherwise committers may act without them. Our maintainer address is releng@collaboraoffice.com.

**Bugzilla.** Changes to the ports tree by non-committers go through https://bugs.freebsd.org/bugzilla/ as attached patches. A committer reviews and commits them. There is no way to push directly.

## 2. What our port consists of

The port lives in the monorepo as a ports overlay at `qt/freebsd/editors/collabora-office/`. An overlay is a directory with the same `category/name` layout as the ports tree, which poudriere and the ports framework can use next to a real ports tree without copying files into it. Once the port is in the FreeBSD tree, the overlay in our repository stays the place where each update is prepared before it is submitted.

The port builds two things from one source tarball: the engine, configured with `--with-distro=CPFreeBSDQt`, and the Qt app. The engine is installed under `/usr/local/lib/collaboraoffice`, the app as `/usr/local/bin/coda-qt` with its web bundle under `/usr/local/share/coolwsd/browser/dist`, plus its desktop file, icons, metainfo and Qt translations. The installed package is about 440 MB from a 115 MB `.pkg`, some 6600 files, with about seventy dependencies.

The distro-config takes every library the ports tree carries from packages and builds only the few externals the engine needs in its own form: the patched cairo and pixman, libjpeg-turbo, pdfium, POCO, argon2, dragonbox, libfixmath, zxcvbn, quickjs, afdko with antlr4, meson, and the OpenSymbol font. Fonts are not bundled; the port declares fourteen font packages as run-time dependencies instead. The engine's externals are listed as additional distfiles so the build needs no network access, which poudriere enforces. The browser build installs its npm packages from the shrinkpack committed in `browser/node_shrinkpack`, so npm needs no network either.

The port fetches three kinds of files: the release source tarball from the Collabora download site, the branding tarball from the Collabora download site, and the engine externals, POCO included, from our own mirror at gerrit.collaboraoffice.com.

## 3. Deliverables of a release, in one list

- A release tag, the same name in the online repository and in the translations repository, for example `coda-26.04.4.1-1`.
- `collabora-office-<version>.tar.xz`, made by `scripts/make-source-tarball.sh` from that tag and published at `https://www.collaboraoffice.com/downloads/collabora-office/src/`. The version is the tag without its product prefix, so `coda-26.04.4.1-1` gives `collabora-office-26.04.4.1-1.tar.xz`.
- The branding tarball `collabora-office-brand-<version>.tar.gz` at `https://www.collaboraoffice.com/downloads/collabora-office-brand/`, which already exists for the snap and flatpak and is shared with them.
- An updated port: `Makefile` with the new `DISTVERSION` and, when the branding changed, `BRAND_VERSION`; regenerated `distinfo`; regenerated `pkg-plist` if the installed file set changed.
- A `poudriere testport` log showing a clean build.
- A Bugzilla report with the patch, or for the first release the new port directory.

## 4. Initial release: one-time setup

These steps prepare the machine that produces the port submissions. Any FreeBSD 15.x machine or VM with at least 16 GB of RAM, 80 GB of free disk and ZFS works; the numbers below come from a four-core virtual machine with exactly that. The steps assume a user in the `wheel` group with `sudo`.

### 4.1 Tools

```sh
sudo pkg install git poudriere portlint portfmt
git clone --depth 1 -b 2026Q3 https://git.FreeBSD.org/ports.git ~/ports
```

Expected result: `~/ports/Mk/bsd.port.mk` exists, and `portclippy`, `portfmt` and `portlint` are on the path (portclippy comes with portfmt).

Clone the **quarterly branch**, not the head of the tree, and keep it on that branch. poudriere can install a dependency as a binary package only when the version in the local ports tree matches the version in the FreeBSD package repository. The quarterly branch changes rarely, so nearly everything is fetched; a checkout of the tree's head is usually a few days ahead of the builders, and every mismatched package is then built from source. In our case a head checkout meant building Qt WebEngine, a Chromium build of eight hours on four cores, purely because ffmpeg had been updated in the tree that week.

### 4.2 The source checkout

```sh
git clone https://gerrit.collaboraoffice.com/online collabora-office
cd collabora-office
git clone https://gerrit.collaboraoffice.com/translations engine/translations
```

The translations clone at `engine/translations` is required by the tarball script. The port itself does not need the engine to be built on this machine.

### 4.3 poudriere

Edit `/usr/local/etc/poudriere.conf` and set:

```sh
ZPOOL=zroot                                  # name of the ZFS pool, from "zpool list"
FREEBSD_HOST=https://download.FreeBSD.org
DISTFILES_CACHE=/home/<user>/ports/distfiles # any writable directory
USE_TMPFS=no                                 # the work directory does not fit in 16 GB of RAM
ALLOW_MAKE_JOBS=yes                          # let the engine build use all cores
```

Then create a build jail for the FreeBSD release to build for, register the ports tree, and register our overlay:

```sh
sudo poudriere jail -c -j 151amd64 -v 15.1-RELEASE
sudo poudriere ports -c -p default -m null -M /home/<user>/ports
sudo poudriere ports -c -p collabora -m null -M /home/<user>/collabora-office/qt/freebsd
```

Expected result: `poudriere jail -l` lists `151amd64`, `poudriere ports -l` lists `default` and `collabora`. Creating the jail downloads the FreeBSD base system, a few hundred megabytes, and takes about three minutes. Repeat the jail step with `-j 143amd64 -v 14.3-RELEASE` for each other supported FreeBSD release the port should be tested on.

Both ports trees are registered with method `null`, which means poudriere uses the directories in place instead of managing its own copies. Every change committed to `qt/freebsd/` in the checkout is immediately visible to poudriere.

## 5. Initial release: producing and submitting the port

### 5.1 Tag

The FreeBSD work must be on the release branch that is being tagged. Tag the online repository and the translations repository with the same name, in the usual way for a release.

### 5.2 Source tarball

```sh
cd collabora-office
git fetch --tags && git -C engine/translations fetch --tags
scripts/make-source-tarball.sh coda-26.04.4.1-1 /tmp/out
```

Expected result: the script prints the path and the SHA256 of `/tmp/out/collabora-office-26.04.4.1-1.tar.xz`. The tarball is about 580 MB and takes three minutes on a fast machine; it contains the monorepo tree at the tag, including the engine sources and the npm shrinkpack, plus the translations under `engine/translations`. The script refuses to run if the tag is missing in either repository.

Publish the file at `https://www.collaboraoffice.com/downloads/collabora-office/src/collabora-office-26.04.4.1-1.tar.xz`. The URL must be permanent: the FreeBSD builders fetch it every time they rebuild the package, which happens whenever a dependency changes, for years.

### 5.3 Port Makefile

In `qt/freebsd/editors/collabora-office/Makefile`, set `DISTVERSION` to the version, `26.04.4.1-1` in the example, and `BRAND_VERSION` to the branding tarball version in use. The ports framework turns `26.04.4.1-1` into the package version `26.04.4.1.1`.

If `engine/download.lst` changed since the last release, regenerate the list of engine externals: configure an engine tree with `--with-distro=CPFreeBSDQt`, run `gmake -f Makefile.fetch fetch-names` in it, and update `ENGINE_SRC_TARBALLS` and the other `DISTFILES` entries accordingly. The README in `qt/freebsd/` says which download site each entry belongs to.

### 5.4 Checksums

```sh
cd qt/freebsd/editors/collabora-office
make PORTSDIR=~/ports makesum
```

Expected result: `distinfo` is rewritten with a `SHA256` and `SIZE` line for every distfile, 15 files at the time of writing, and all of them are fetched into the distfiles cache. A failure here means a URL is wrong or a file is not published yet. Pass `PORTSDIR` unless the ports tree is at `/usr/ports`.

### 5.5 Packing list

`pkg-plist` is committed in the repository and only changes when the set of installed files changes, which happens when the engine gains or loses a library, a language is added or removed, or the app installs a new file. It contains no version numbers, so a plain version bump never touches it. To regenerate it, a staged build is needed:

```sh
make PORTSDIR=~/ports NO_DEPENDS=yes stage
make PORTSDIR=~/ports makeplist | tail -n +2 > pkg-plist
make PORTSDIR=~/ports generate-plist check-plist
```

Expected result: `check-plist` reports `No pkg-plist issues found`. Notes on the three commands: `NO_DEPENDS=yes` tells the framework not to install missing dependencies itself, which would need root and would build them from source, so the build tools must already be installed for this shortcut; the first line of `makeplist` output is a reminder to review the list, which `tail` drops; and `check-plist` reads a temporary list generated during `stage`, so `generate-plist` has to run after `pkg-plist` is written. The staged build takes about three hours. Skip this step when the file set cannot have changed; a `check-plist` failure in the poudriere run tells you when it was needed after all.

### 5.6 Lint

```sh
portclippy Makefile
portfmt Makefile | diff -u Makefile -
PORTSDIR=~/ports portlint -AC
```

Expected result: portclippy prints the variable layout with no `+` or `-` lines, and the portfmt diff is empty. portfmt is the authority on the order and wrapping of the dependency lists; do not sort those lists by hand, because a mistake there silently merges two blocks, and take `portfmt Makefile` output as the file whenever it differs.

portlint is older than the other two and its warnings need judgement. These four are expected and can be ignored, because `editors/libreoffice` produces the same ones: the claim that `RUN_DEPENDS` and `USES` have to appear earlier, which contradicts portclippy and current practice; the suggestion of an NLS knob; and the suggestion of `USE_LDCONFIG` for the engine's private shared libraries, which are loaded by rpath and must not go into the system linker path. On a development machine portlint also reports two fatal items that are artifacts of testing, an untracked `distinfo` and a leftover `work` directory.

### 5.7 poudriere testport

```sh
sudo poudriere testport -S -j 151amd64 -p default -O collabora -b quarterly editors/collabora-office
```

The options: `-j` picks the build jail, `-p default` the ports tree, `-O collabora` adds our overlay, `-b quarterly` lets poudriere fetch dependencies as binary packages from the FreeBSD quarterly repository instead of building them, and `-S` stops poudriere from rebuilding everything that depends on a package it did have to build. Use `-b latest` only with a ports checkout that tracks the head of the tree. Always pass `-S`.

Expected result: the run ends with `Built: 1` and the port's own log ends with `No pkg-plist issues found` and `build time: 03:01:57` or thereabouts. Our port takes just about three hours in the jail on four cores. The dependency phase takes an hour or so the first time, mostly downloads, and is nearly instant afterwards because poudriere caches the packages. Logs are under `/usr/local/poudriere/data/logs/bulk/151amd64-default/`, with the port's own log in the `logs/` subdirectory of the run; keep it, it is attached to the submission.

Read the Q/A section of the port's log even when the run succeeds. It lists every library the staged binaries link without a declaration, in the form `Error: ... is linked to ... but it is not declared as a dependency` followed by `Warning: you need LIB_DEPENDS+=...`, and this is the authoritative source for the dependency list; the final port was built by following it until the section was empty. It also prints `you might not need LIB_DEPENDS on ...` advisories, which are not always right: two of ours name libraries that the binaries demonstrably link, checked with `readelf -d`, so verify before removing anything.

`testport` deinstalls its package at the end and does not keep it. To get a package to install and try, use `poudriere bulk` with the same options, which leaves it in `/usr/local/poudriere/data/packages/151amd64-default/All/`.

### 5.8 Submit

Produce a patch against a ports tree checkout. The submission for a new port contains the port directory and one added line in the category Makefile:

```sh
cd ~/ports
git checkout main && git pull
cp -R ~/collabora-office/qt/freebsd/editors/collabora-office editors/
# add "    SUBDIR += collabora-office" to editors/Makefile, in alphabetical order
git add editors/collabora-office editors/Makefile
git commit -m "editors/collabora-office: add Collabora Office desktop application"
git format-patch -1
```

Expected result: a file `0001-editors-collabora-office-add-Collabora-Office-desktop.patch`. Make sure the copied directory contains only `Makefile`, `distinfo`, `pkg-descr` and `pkg-plist`; no `work` directory and no editor backups.

Open a report at https://bugs.freebsd.org/bugzilla/ with product `Ports & Packages`, component `Individual Port(s)`, and summary `[NEW PORT] editors/collabora-office: Collabora Office desktop application`. Attach the patch with the `patch` flag set, and attach the poudriere log. In the description state the maintainer address, that the port passes `poudriere testport` on the releases you tested, that portclippy and portfmt are clean, and one sentence on what the software is. Mention that Collabora publishes the source tarball at a permanent URL.

Expected result: a committer, most likely from the office@FreeBSD.org team that maintains `editors/libreoffice`, reviews the port. Reviews of new ports of this size take from days to a few weeks. Questions to expect: why cairo, pixman, libjpeg-turbo, pdfium and POCO are bundled (cairo is our patched copy, needed for the cairo-rgba mode; pdfium has no system option; the others are versions we build and test against), and whether the tarball URL is permanent. When the port is committed, the package appears in `latest` within days. Ask in the report for a merge to the quarterly branch if the release should reach quarterly users before the next branch point.

## 6. Subsequent releases

Once the port is in the FreeBSD tree, a release is a patch that bumps it.

1. Tag both repositories and publish the source tarball, as in 5.1 and 5.2. Publish a new branding tarball only if the branding changed.
2. In the overlay, set `DISTVERSION`, and `BRAND_VERSION` if needed. Update the engine externals list if `engine/download.lst` changed, as in 5.3.
3. `make makesum` to regenerate `distinfo`.
4. Regenerate `pkg-plist` as in 5.5 if the installed file set may have changed. For a pure version bump this can be skipped.
5. `portclippy` and `portfmt` as in 5.6.
6. `poudriere testport` as in 5.7, on each supported release, and read the Q/A section.
7. Produce the patch: copy the changed files over `~/ports/editors/collabora-office/` and use `git format-patch`. A version bump patch typically touches `Makefile` and `distinfo`, sometimes `pkg-plist`.
8. Open a Bugzilla report with summary `editors/collabora-office: update to 26.04.5.1.1`, attach the patch and the testport log. Since the report comes from the maintainer address, committers apply it with little review; a day or two is typical. If the release must reach quarterly users promptly, add a line `MFH: 2026Q4`, naming the current quarterly branch, and say why the merge is warranted; committers merge bug and security fixes.

Between releases, expect mail to the maintainer address from the FreeBSD builders and from users. The common case is a dependency that changed its library version, for example a new liborcus with a different soname, which makes a `LIB_DEPENDS` entry stale; the fix is a one-line patch through Bugzilla. The ports tree also runs an automatic check, portscout, which mails the maintainer when a newer version appears at the download URL; for us that is harmless noise, since we drive updates from our own release process.

## 7. FreeBSD-specific pitfalls already fixed

These were found by the jail builds and are fixed in the tree. They are listed because they explain otherwise puzzling parts of the port and of the FreeBSD build, and because the same classes of problem will come back with new code.

- **Argument limits.** FreeBSD allows 512 KiB of command arguments, a quarter of what Linux allows. The engine's merged library links about 4800 object files by absolute path, and under the long paths of a ports work directory the link command exceeded the limit. gbuild now passes the object list to the linker through a response file.
- **BSD versus GNU tools.** `readlink -f` prints nothing for a path that does not exist yet, which emptied the engine path compiled into the app, since the engine is not installed when the app is configured. BSD `sed` rejects GNU's `\|` alternation and `\s`, which silently emptied a generated component list. BSD `make` is not GNU make, so the app's configure needs `MAKE=gmake`.
- **What a clean jail does not have.** The unversioned `python3` command comes from a separate meta package, so configure now looks the interpreter up and accepts a `PYTHON` variable. `libtoolize` needs an explicit dependency on devel/libtool, because the framework's autoreconf helper does not imply it. rsync is needed by the browser install step.
- **File modes.** The `zip` tool in ports creates archives readable only by the owner, unlike the Linux one, and the engine's installer keeps the modes it finds, which left 59 data files unreadable in a package owned by root. The port opens them up after installing.

## 8. Where things are

- Port and overlay: `qt/freebsd/editors/collabora-office/` in the online repository; `qt/freebsd/README.md` describes maintenance of the recipe itself.
- Tarball script: `scripts/make-source-tarball.sh`.
- Engine configuration for FreeBSD: `engine/distro-configs/CPFreeBSDQt.conf`.
- Build requirements for a developer machine: the FreeBSD section of `qt/README.md`.
- FreeBSD ports tree: https://cgit.freebsd.org/ports/, browsable with metadata at https://www.freshports.org/.
- Bugzilla: https://bugs.freebsd.org/bugzilla/, product `Ports & Packages`.
- Porter's Handbook, the reference for everything in the port: https://docs.freebsd.org/en/books/porters-handbook/.
- poudriere documentation: https://github.com/freebsd/poudriere/wiki.
