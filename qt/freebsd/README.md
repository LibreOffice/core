# FreeBSD port

`editors/collabora-office` is the FreeBSD port of the Collabora Office desktop
app, kept here as a ports overlay until it is in the FreeBSD ports tree. It
builds the engine and the Qt app from one release source tarball, with the
engine configured through `engine/distro-configs/CPFreeBSDQt.conf`.
Everything the ports tree carries is taken from packages, fonts included; the
few engine externals it still builds are listed in `DISTFILES` and fetched from
their upstream sites, and the branding comes from its own tarball on the
Collabora download site. The build itself runs without network access, as
poudriere requires; the browser build installs its npm packages from the
shrinkpack in `browser/node_shrinkpack`.

## Release source tarball

The port builds `collabora-office-<version>.tar.xz`, made from the release tag
with `scripts/make-source-tarball.sh`. The tarball is the monorepo tree at the
tag plus the translations repository, which carries the same tag, at
`engine/translations`. Run it from a checkout that has the translations cloned
there:

```sh
scripts/make-source-tarball.sh cp-26.04.3-2 /tmp/out
```

The tag's product prefix is dropped for the version, so the example gives
`collabora-office-26.04.3-2.tar.xz`, and the port's `DISTVERSION` is
`26.04.3-2`. Publish the tarball under
`https://www.collaboraoffice.com/downloads/collabora-office/src/`, the
`MASTER_SITES` entry the port fetches from.

## Branding

The port fetches `collabora-office-brand-<BRAND_VERSION>.tar.gz` from the
Collabora download site, the same tarball the snap and flatpak use, and applies
it the same way: the app is configured with `--with-app-branding` pointing at
the extracted tree, which puts the CSS, images and welcome files into the
browser bundle, and `online-theme` is installed as the engine's `online`
theme definition. Bump `BRAND_VERSION` and rerun `makesum` when a new branding
tarball is published.

## Fonts

The engine is built with `--without-fonts`, so the package ships none of the
bundled fonts. The families they covered come from ports packages listed as
`RUN_DEPENDS`, the way the LibreOffice port does it.

## Using the overlay

The directory is laid out as a ports overlay, so it works next to a normal
ports tree without being copied into it. With a ports tree at `/usr/ports`:

```sh
cd qt/freebsd/editors/collabora-office
make makesum                     # after any change to DISTFILES or DISTVERSION
make stage
make makeplist > pkg-plist       # then review the header line
make package
```

Pass `PORTSDIR=/path/to/ports` when the tree is elsewhere. For a clean package
build, point poudriere at the overlay:

```sh
poudriere bulk -j 151amd64 -p default -O /path/to/online/qt/freebsd editors/collabora-office
```

To test a tarball that is not published yet, put it into
`${DISTDIR}/collabora-office/` by hand; fetch skips files that are already
there, and `makesum` records their checksums.

## Layout of the installed package

- `bin/coda-qt`, the app, plus the desktop file, icons, metainfo and Qt
  translations from the app's `make install`.
- `share/coolwsd/browser/dist`, the web user interface the app shows, with the
  branding applied.
- `lib/collaboraoffice`, the engine installation the app points at through
  `--with-lo-path`, produced by the engine's `distro-pack-install`, with the
  `online` theme definition added.

## Keeping it current

- **Release.** Set `DISTVERSION` to the new version, publish the tarball, run
  `makesum`.
- **Externals.** After a change to `engine/download.lst`, regenerate the
  tarball list from a configured engine tree with
  `gmake -f Makefile.fetch fetch-names` and update `ENGINE_SRC_TARBALLS` and
  the other `DISTFILES` entries, then run `makesum`. The opens___.ttf font and
  POCO come from their own sites; everything else is on the document
  foundation's source mirror.
- **Ports libraries.** The `LIB_DEPENDS` entries use the versioned library
  names the ports packages install, so a major bump of a port such as
  liborcus needs the entry updated.

## Not done yet

- `distinfo` and `pkg-plist` are generated once the first release tarball is
  published; until then they are made locally from a test tarball.
- Submission to the FreeBSD ports tree as `editors/collabora-office`, with
  releng@collaboraoffice.com as maintainer.
