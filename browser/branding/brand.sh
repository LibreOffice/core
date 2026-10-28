#!/bin/bash
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: Apache-2.0
#
# Generate a theme into a browser/dist directory.
#
# Usage: brand.sh [--pure] OUTDIR THEME
#
#   OUTDIR   the browser/dist directory to brand
#   THEME    CO-theme (Collabora Online), CODE (Collabora Online Development
#            Edition) or CODA (Collabora Office); the theme ids of
#            online-branding's brand.sh (1, 3, 10) work too
#   --pure   for the mobile apps, which load the technology logo of the About
#            dialog from the web instead of the server
#
# Environment: SASS (default: sass), SASS_FLAGS (extra sass options, like
# --quiet), BRAND_VERSION (shown in the header of the generated files,
# optional).
#
# The sources are not modified; the theme is selected in a copy of sass/.

set -e

usage()
{
    cat 1>&2 << EOF
Usage: brand.sh [--pure] OUTDIR THEME

  OUTDIR   the browser/dist directory to brand
  THEME    CO-theme, CODE or CODA (or the online-branding ids 1, 3, 10)
  --pure   for the mobile apps, the About dialog loads the technology logo
           from the web
EOF
    exit 1
}

PURE=
if [ "$1" = "--pure" ]; then
    PURE=1
    shift
fi
[ $# -eq 2 ] || usage

OUTDIR="$1"
THEME="$2"

SRCDIR=$(cd "$(dirname "$0")" && pwd)
SASS="${SASS:-sass}"

# All three themes share the Collabora Online look (CO-theme-1.1); they differ
# in their branding.js, which names the product.
BASE_DIR="$SRCDIR/themes/collabora-online-brand"
SCSS_THEME=CO-theme-1.1
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
    *)
        echo "brand.sh: unknown theme '$THEME', use CO-theme, CODE or CODA" 1>&2
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
{ echo "@import '$SCSS_THEME';"; cat "$WORK/sass/def/_base.scss"; } > "$WORK/base.scss"
mv "$WORK/base.scss" "$WORK/sass/def/_base.scss"

set_iconset()
{
    sed "s/iconsetgenerated: .*/iconsetgenerated: $1;/" "$WORK/sass/def/_icons.scss" > "$WORK/icons.scss"
    mv "$WORK/icons.scss" "$WORK/sass/def/_icons.scss"
}

HEADER="/* Copyright the Collabora Online contributors. SPDX-License-Identifier: Apache-2.0${BRAND_VERSION:+ (version $BRAND_VERSION)} */"

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

# The desktop-only rules are for the Nextcloud flavours, which are not here.
: > "$OUTDIR/branding-desktop.css"

compile device-mobile.scss branding-mobile.css
compile device-tablet.scss branding-tablet.css
add_header "$OUTDIR/branding-mobile.css" "$OUTDIR/branding-tablet.css"

# Copy the images that the CSS references.
for i in $(grep -ho "images/.*svg" "$OUTDIR"/branding*.css | sed -e "s/images\///" | grep -v lokit-extra-img.svg | sort | uniq); do
    cp "$BASE_DIR/images/$i" "$OUTDIR/images/"
done

if [ -n "$PURE" ]; then
    sed "s|remote/static/lokit-extra-img.svg|https://rating.collaboraonline.com/static/lokit-extra-img.svg|" \
        "$OUTDIR/branding.css" > "$WORK/branding.css"
    mv "$WORK/branding.css" "$OUTDIR/branding.css"
fi

# Document type icons for the macOS app
if [ "$(uname)" = Darwin ] && command -v sips > /dev/null 2>&1; then
    "$SRCDIR/generate-document-icons.sh" "$BASE_DIR/styleguide" "$OUTDIR/images"
fi
