#!/usr/bin/env bash
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: Apache-2.0
#
# Generate a theme into a browser/dist directory.
#
# Usage: brand.sh [--overlay DIR] OUTDIR THEME
#
#   --overlay DIR  build a brand on top of THEME: the files in DIR/sass
#            replace or add to the stylesheets (for the Nextcloud theme,
#            sass/def/_NC-brand.scss sets its options and sass/def/_NC-icons.scss
#            its icons), DIR/branding.js replaces the script and DIR/images
#            override the images
#   OUTDIR   the browser/dist directory to brand
#   THEME    CO-theme (Collabora Online), CODE (Collabora Online Development
#            Edition), CODA (Collabora Office) or NC-theme-community
#            (Nextcloud Office, generated into OUTDIR/nextcloud); the theme
#            ids of online-branding's brand.sh (1, 3, 10, 5) work too
#
# Environment: SASS (default: sass), SASS_FLAGS (extra sass options, like
# --quiet), BRAND_VERSION (shown in the header of the generated files,
# optional), BRAND_HEADER (replaces the header line of the generated files,
# for a brand under other terms).
#
# The sources are not modified; the theme is selected in a copy of sass/.

set -e

usage()
{
    cat 1>&2 << EOF
Usage: brand.sh [--overlay DIR] OUTDIR THEME

  OUTDIR   the browser/dist directory to brand
  THEME    CO-theme, CODE, CODA or NC-theme-community
           (or the online-branding ids 1, 3, 10, 5)
EOF
    exit 1
}

OVERLAY=
if [ "$1" = --overlay ]; then
    [ -d "$2" ] || usage
    OVERLAY=$(cd "$2" && pwd)
    shift 2
fi
[ $# -eq 2 ] || usage

OUTDIR="$1"
THEME="$2"

SRCDIR=$(cd "$(dirname "$0")" && pwd)
SASS="${SASS:-sass}"

# CO-theme, CODE and CODA share the Collabora Online look (CO-theme-1.1); they
# differ in their branding.js, which names the product. The Nextcloud theme is a
# flavour: it goes into a subdirectory that the integration selects at runtime,
# and its images override those of the Collabora Online theme.
BASE_DIR="$SRCDIR/themes/collabora-online-brand"
SCSS_THEME=CO-theme-1.1
FLAVOUR_DIR=
case "$THEME" in
    1 | CO-theme)
        JS_DIR="$BASE_DIR"
        ;;
    3 | CODE)
        JS_DIR="$SRCDIR/themes/CODE"
        ;;
    10 | CODA)
        JS_DIR="$SRCDIR/themes/CODA"
        ;;
    5 | NC-theme-community)
        SCSS_THEME=NC-theme
        FLAVOUR_DIR="$SRCDIR/themes/nextcloud"
        JS_DIR="$FLAVOUR_DIR"
        OUTDIR="$OUTDIR/nextcloud"
        ;;
    *)
        echo "brand.sh: unknown theme '$THEME', use CO-theme, CODE, CODA or NC-theme-community" 1>&2
        exit 1
        ;;
esac

if ! command -v "$SASS" > /dev/null 2>&1; then
    echo "brand.sh: sass not found, set SASS or install it with: npm install -g sass" 1>&2
    exit 1
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cp -R "$SRCDIR/sass" "$WORK/sass"
if [ -n "$OVERLAY" ] && [ -d "$OVERLAY/sass" ]; then
    cp -R "$OVERLAY/sass/." "$WORK/sass/"
fi
if [ -n "$OVERLAY" ] && [ -f "$OVERLAY/branding.js" ]; then
    JS_DIR="$OVERLAY"
fi
{ echo "@import '$SCSS_THEME';"; cat "$WORK/sass/def/_base.scss"; } > "$WORK/base.scss"
mv "$WORK/base.scss" "$WORK/sass/def/_base.scss"

set_iconset()
{
    sed "s/iconsetgenerated: .*/iconsetgenerated: $1;/" "$WORK/sass/def/_icons.scss" > "$WORK/icons.scss"
    mv "$WORK/icons.scss" "$WORK/sass/def/_icons.scss"
}

HEADER="${BRAND_HEADER:-/* Copyright the Collabora Office contributors. SPDX-License-Identifier: Apache-2.0${BRAND_VERSION:+ (version $BRAND_VERSION)} */}"

add_header()
{
    for f in "$@"; do
        { echo "$HEADER"; echo; cat "$f"; } > "$WORK/header.tmp"
        mv "$WORK/header.tmp" "$f"
    done
}

# The theme still uses @import; quieten its deprecation warnings where sass knows them.
FLAGS="--no-source-map $SASS_FLAGS"
if "$SASS" --help 2>&1 | grep -q silence-deprecation; then
    FLAGS="$FLAGS --silence-deprecation=import"
fi

compile()
{
    "$SASS" $FLAGS "$WORK/sass/$1" "$OUTDIR/$2"
}

mkdir -p "$OUTDIR/images"
rm -f "$OUTDIR/branding.css" "$OUTDIR/branding-unsupported.css" "$OUTDIR/branding-unsupported.js"

set_iconset false
compile main.scss branding.css
set_iconset true

cp "$JS_DIR/branding.js" "$OUTDIR/branding.js"
GENERATED="$OUTDIR/branding.css $OUTDIR/branding.js"
if [ -f "$JS_DIR/branding-unsupported.js" ]; then
    cp "$JS_DIR/branding-unsupported.js" "$OUTDIR/branding-unsupported.js"
    cp "$OUTDIR/branding.css" "$OUTDIR/branding-unsupported.css"
    GENERATED="$GENERATED $OUTDIR/branding-unsupported.css $OUTDIR/branding-unsupported.js"
fi
add_header $GENERATED

if [ -n "$FLAVOUR_DIR" ]; then
    compile desktop/_desktop-only.scss branding-desktop.css
    add_header "$OUTDIR/branding-desktop.css"
else
    : > "$OUTDIR/branding-desktop.css"
fi

compile device-mobile.scss branding-mobile.css
compile device-tablet.scss branding-tablet.css
add_header "$OUTDIR/branding-mobile.css" "$OUTDIR/branding-tablet.css"

# Copy the images that the CSS references.
for i in $(grep -ho "images/.*svg" "$OUTDIR"/branding*.css | sed -e "s/images\///" | sort | uniq); do
    if [ -n "$OVERLAY" ] && [ -f "$OVERLAY/images/$i" ]; then
        cp "$OVERLAY/images/$i" "$OUTDIR/images/"
    elif [ -n "$FLAVOUR_DIR" ] && [ -f "$FLAVOUR_DIR/images/$i" ]; then
        cp "$FLAVOUR_DIR/images/$i" "$OUTDIR/images/"
    else
        cp "$BASE_DIR/images/$i" "$OUTDIR/images/"
    fi
done

# Document type icons for the macOS app
if [ -z "$FLAVOUR_DIR" ] && [ "$(uname)" = Darwin ] && command -v sips > /dev/null 2>&1; then
    "$SRCDIR/generate-document-icons.sh" "$BASE_DIR/styleguide" "$OUTDIR/images"
fi
