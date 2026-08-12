#!/usr/bin/env bash
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Builds a drag-to-Applications .dmg from an already signed, notarized and
# stapled .app bundle, then signs, notarizes and staples the .dmg itself.
#
# Usage:
#   IDENTITY="Developer ID Application: ... (TEAMID)" \
#   KEYCHAIN_PROFILE=Collabora \
#       macos/make-dmg.sh coda.export/CollaboraOffice.app [output.dmg]
#
# The product name, used for the volume name, is taken from the bundle's
# CFBundleDisplayName, which the build fills in from --with-app-name. Set
# PRODUCT_NAME to override it. Rebranded builds get the untitled background
# variant unless DMG_BACKGROUND points at their own artwork; see
# dmg-artwork/make-dmg-background.py.
#
# Requires dmgbuild, which is pure Python: only ds_store and mac_alias, no
# compiled extensions, nothing to build. Installing it for the build user is
# enough, and PATH does not matter, because we fall back to running it as a
# module:
#
#   python3 -m pip install --user dmgbuild
#
# On the Python 3.9 that comes with the command line tools that resolves to
# dmgbuild 1.6.5, the last release supporting 3.9; it has every setting used
# here, and copies the payload with ditto just like the current ones.
#
# To keep it out of the build user's site-packages instead, put it in a venv
# and point DMGBUILD at it:
#
#   python3 -m venv /opt/dmgbuild && /opt/dmgbuild/bin/pip install dmgbuild
#   DMGBUILD=/opt/dmgbuild/bin/dmgbuild macos/make-dmg.sh ...
#
# dmgbuild writes the .DS_Store holding the window layout itself, so unlike the
# AppleScript-driven tools it needs neither Finder nor a GUI login session, and
# runs on a headless build node.

set -e
set -u
set -o pipefail

if [ "$(uname)" != Darwin ]; then
    echo "This is for macOS only" >&2
    exit 1
fi

if [ $# -lt 1 ]; then
    echo "Usage: $0 signed-stapled-app-bundle [output.dmg]" >&2
    exit 1
fi

APP=$1
if [ ! -d "$APP" ] || [[ "$APP" != *.app ]]; then
    echo "Not an app bundle: $APP" >&2
    exit 1
fi

OUT=${2:-$(basename "$APP" .app).dmg}
IDENTITY=${IDENTITY:-}
KEYCHAIN_PROFILE=${KEYCHAIN_PROFILE:-}
PYTHON=${PYTHON:-python3}

ARTDIR=$(cd "$(dirname "$0")"/dmg-artwork && /bin/pwd)
. "$ARTDIR"/geometry.env

# Running dmgbuild as a module covers a pip install --user, where the script
# itself lands somewhere outside the build user's PATH.
if [ -n "${DMGBUILD:-}" ]; then
    DMGBUILD_CMD=("$DMGBUILD")
elif command -v dmgbuild >/dev/null; then
    DMGBUILD_CMD=(dmgbuild)
elif "$PYTHON" -c 'import dmgbuild' 2>/dev/null; then
    DMGBUILD_CMD=("$PYTHON" -m dmgbuild)
else
    echo "dmgbuild not found -- $PYTHON -m pip install --user dmgbuild" >&2
    echo "(or set DMGBUILD to the dmgbuild of a venv)" >&2
    exit 1
fi
echo "Using: ${DMGBUILD_CMD[*]}"

plist_value() {
    /usr/libexec/PlistBuddy -c "Print :$1" "$APP"/Contents/Info.plist 2>/dev/null || true
}

PRODUCT_NAME=${PRODUCT_NAME:-}
if [ -z "$PRODUCT_NAME" ]; then
    PRODUCT_NAME=$(plist_value CFBundleDisplayName)
fi
if [ -z "$PRODUCT_NAME" ]; then
    PRODUCT_NAME=$(plist_value CFBundleName)
fi
if [ -z "$PRODUCT_NAME" ]; then
    PRODUCT_NAME=$(basename "$APP" .app)
fi
echo "Product name: $PRODUCT_NAME"

# The .app must already carry its own notarization ticket, so that it keeps
# validating once the user has dragged it out of the .dmg.
xcrun stapler validate "$APP"

WORK=$(mktemp -d "${TMPDIR:-/tmp}/dmg-build.XXXXXX")
trap 'rm -rf "$WORK"' EXIT

# The titled background only fits the product it names.
if [ -z "${DMG_BACKGROUND:-}" ]; then
    if [ "$PRODUCT_NAME" = "$DMG_TITLE_NAME" ]; then
        DMG_BACKGROUND="$ARTDIR"/dmg-background.png
    else
        DMG_BACKGROUND="$ARTDIR"/dmg-background-generic.png
    fi
fi
if [ ! -f "$DMG_BACKGROUND" ]; then
    echo "No such background image: $DMG_BACKGROUND" >&2
    exit 1
fi
echo "Background: $DMG_BACKGROUND"

DEFINES=(
    -Dapp="$APP"
    -Dbackground="$DMG_BACKGROUND"
    -Dicon_size="$DMG_ICON_SIZE"
    -Dwin_w="$DMG_WINDOW_WIDTH"
    -Dwin_h="$DMG_WINDOW_HEIGHT"
    -Dapp_x="$DMG_APP_X"
    -Dapp_y="$DMG_APP_Y"
    -Ddrop_x="$DMG_DROP_X"
    -Ddrop_y="$DMG_DROP_Y"
)

if [ -f "$APP"/Contents/Resources/AppIcon.icns ]; then
    cp "$APP"/Contents/Resources/AppIcon.icns "$WORK"/VolumeIcon.icns
    DEFINES+=(-Dvolicon="$WORK"/VolumeIcon.icns)
fi

rm -f "$OUT"
"${DMGBUILD_CMD[@]}" -s "$ARTDIR"/dmgbuild-settings.py "${DEFINES[@]}" \
    "$PRODUCT_NAME" "$OUT"

if [ -n "$IDENTITY" ]; then
    codesign --force --sign "$IDENTITY" --timestamp "$OUT"
fi

if [ -n "$KEYCHAIN_PROFILE" ]; then
    xcrun notarytool submit "$OUT" --keychain-profile "$KEYCHAIN_PROFILE" --wait
    xcrun stapler staple "$OUT"
    xcrun stapler validate "$OUT"
    spctl -a -vvv -t open --context context:primary-signature "$OUT"
else
    echo
    echo "Warning: $OUT is NOT notarized. Run:"
    echo "  xcrun notarytool submit '$OUT' --keychain-profile <profile> --wait"
    echo "  xcrun stapler staple '$OUT'"
fi

echo "Created $OUT"
