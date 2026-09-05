# FreeBSD port

`editors/collabora-office` is a FreeBSD ports overlay for the Collabora Office
desktop app. It builds the engine and the Qt app from one source tarball, the
GitHub mirror snapshot named by `GH_TAGNAME`, with the engine configured
through `engine/distro-configs/CPFreeBSDQt.conf`. Everything the ports tree
carries is taken from packages; the engine externals it still builds are
listed in `DISTFILES` and fetched from their upstream sites, so the build
itself runs without network access, as poudriere requires. The browser build
installs its npm packages from the shrinkpack in `browser/node_shrinkpack`.

## Using the overlay

The directory is laid out as a ports overlay, so it works next to a normal
ports tree without being copied into it. With `/usr/ports` checked out:

```sh
cd qt/freebsd/editors/collabora-office
make -DPORTSDIR=/usr/ports makesum      # after any change to DISTFILES or GH_TAGNAME
make -DPORTSDIR=/usr/ports stage
make -DPORTSDIR=/usr/ports makeplist > pkg-plist   # then review the header line
make -DPORTSDIR=/usr/ports package
```

For a clean package build, point poudriere at the overlay:

```sh
poudriere bulk -j 151amd64 -p default -O /path/to/online/qt/freebsd editors/collabora-office
```

## Layout of the installed package

- `bin/coda-qt`, the app, plus the desktop file, icons, metainfo and Qt
  translations from the app's `make install`.
- `share/coolwsd/browser/dist`, the web user interface the app shows.
- `lib/collaboraoffice`, the engine installation the app points at through
  `--with-lo-path`, produced by the engine's `distro-pack-install`.

## Keeping it current

- **Snapshot.** `GH_TAGNAME` names the commit of the mirror to build. Bump it
  and run `makesum`.
- **Externals.** After a change to `engine/download.lst`, regenerate the
  tarball list from a configured engine tree with
  `gmake -f Makefile.fetch fetch-names` and update `ENGINE_SRC_TARBALLS` and
  the other `DISTFILES` entries, then run `makesum`. The opens___.ttf font,
  the OpenDyslexic font and POCO come from their own sites; everything else
  is on the document foundation's source mirror.
- **Ports libraries.** The `LIB_DEPENDS` entries use the versioned library
  names the ports packages install, so a major bump of a port such as
  liborcus needs the entry updated.

## Not done yet

- `pkg-plist` is not committed; generate it with `makeplist` once the stage
  step works.
- The engine is built with `--with-lang=en-US` because the translations live
  in a separate repository that this distfile does not include.
- The version is the tree's `AC_INIT` version, not a release tag.
