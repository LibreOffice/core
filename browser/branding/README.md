# Branding

The themes of Collabora Online, CODE (Collabora Online Development Edition) and
Collabora Office (the desktop apps, CODA). The files in this directory are licensed
under the Apache License, Version 2.0, see [LICENSE](LICENSE).

The Nextcloud themes and the customer brands are not here, they are still in the
online-branding repository.

## Layout

- `brand.sh`: generates a theme into a `browser/dist` directory
- `sass/`: the stylesheets
  - `def/`: variables and mixins; `_base.scss` holds the defaults, `_CO-theme-1.1.scss`
    the Collabora Online theme
  - `desktop/`: the rules per UI component
  - `mobile/`: the phone and tablet overrides
  - `main.scss`, `device-mobile.scss`, `device-tablet.scss`: the entry points for
    `branding.css`, `branding-mobile.css` and `branding-tablet.css`
- `themes/collabora-online-brand/`: the Collabora Online theme
  - `branding.js`, `branding-unsupported.js`: the brand's script, and its variant shown
    without a valid support key
  - `images/`: the icons and logos the stylesheets reference
  - `styleguide/`: the document type icons, converted for the macOS app
- `themes/CODE/branding.js`: the product name and links of CODE
- `themes/CODA/branding.js`: the product name and link of Collabora Office
- `generate-document-icons.sh`: the macOS document type icons, called by `brand.sh`
- `sync-online-branding.py`: compares and synchronizes with online-branding, see below

## Themes

All three themes compile the same stylesheet and copy the same images; they differ in
`branding.js`, which names the product.

| Theme      | Product name                                | `branding.js`             | Extra                    |
|------------|---------------------------------------------|---------------------------|--------------------------|
| `CO-theme` | Collabora Online                            | collabora-online-brand    | `branding-unsupported.*` |
| `CODE`     | Collabora Online Development Edition (CODE) | CODE                      |                          |
| `CODA`     | Collabora Office                            | CODA                      |                          |

The welcome slideshow of Collabora Office is part of the product, not of the branding;
it lives in [../welcome-slideshow](../welcome-slideshow) and every desktop app build
bundles it.

## In the build

A debug build applies a theme to `browser/dist` by itself: the desktop app builds
(`--enable-macosapp`, `--enable-windowsapp`, `--enable-qtapp --disable-server`) get CODA,
everything else gets CODE. A tree that builds the server and the qt app together gets
CODE in `browser/dist`, and the qt app page loads `branding-app.js`, the script of the
CODA theme, so that the server is CODE and the app is Collabora Office. Configure
options:

- `--disable-branding`: build an unbranded tree
- `--enable-branding=CO-theme|CODE|CODA`: pick the theme, also in a release build

Release builds, the Android and iOS builds and builds with `--with-app-branding` are
not branded from here unless asked to, so the `coolwsd` package stays unbranded and
the brand packages are made separately, as before. Switching the theme or turning it
off leaves the generated files of the previous one in `browser/dist`; `make clean` in
`browser/` removes them.

The build uses the dart-sass in `browser/node_modules` (a devDependency of
`browser/package.json`, vendored in `browser/node_shrinkpack`). `make` in `browser/`
regenerates the theme after a change here; coolwsd reads `browser/dist` at startup,
restart it to see the new CSS.

## Generating a theme by hand

    browser/branding/brand.sh OUTDIR THEME

for example

    browser/branding/brand.sh browser/dist CODE

`THEME` is `CO-theme`, `CODE` or `CODA`; the theme ids of online-branding's `brand.sh`
(1, 3, 10) work too. The script needs
[dart-sass](https://sass-lang.com/dart-sass/) (`sass` on the `PATH`, or
`SASS=browser/node_modules/.bin/sass`). Unlike the online-branding script, this one
never modifies its sources.

## Keeping online-branding in sync

During the transition both repositories are developed. Pair the branches: `main` here
with `master` there, `distro/collabora/co-XX.YY` with the branch of the same name.
`sync-online-branding.py` knows which files correspond and ignores the differences that
are expected (the "All Rights Reserved" notice in the SVG files there, the license
header of the scripts here):

    export ONLINE_BRANDING_DIR=~/online-branding
    browser/branding/sync-online-branding.py status      # what differs
    browser/branding/sync-online-branding.py diff        # how
    browser/branding/sync-online-branding.py pull -n     # what pull would change here
    browser/branding/sync-online-branding.py pull        # take the changes from there
    browser/branding/sync-online-branding.py push        # bring the changes there

`status` exits with 1 when something differs, so it can run in a CI job. `pull` and
`push` copy only, they never delete; a file that exists on one side only is reported.
Only the images that the generated CSS references are kept here, so `status` lists the
others as only in online-branding. Commit the result on each side as usual.

To check that the two `brand.sh` scripts still generate the same theme:

    browser/branding/sync-online-branding.py compare-output CODE

It runs both on a copy and compares the outputs, ignoring the header comment of the
generated files.
