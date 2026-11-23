#!/bin/bash
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Put the SBOM of the Mac app into its bundle, in Contents/Resources/sbom.
#
# Run it on the app as it will ship, before it is signed for distribution:
# for a release, on the universal bundle that merge-app-bundles made, before
# xcodebuild -exportArchive signs it. Signing changes the Mach-O files, so
# their hashes in the SBOM are those before signing (the SBOM says so); the
# code signature is what verifies them. A file added after signing would
# break the seal.
#
# Usage: macos/add-sbom.sh <the .app> <online build dir> [<engine build dir>]
#            [<build number>] [<side-car path prefix>]
#
# The engine build dir defaults to the online build's ENGINE_BUILDDIR. The
# side-car prefix, when given, also gets <prefix>.cdx.json / .spdx.json.

set -e

APP="$1"
BUILDDIR="$2"
ENGINE_BUILDDIR="$3"
BUILD="$4"
SIDECAR="$5"

if [ ! -d "$APP/Contents" ] || [ ! -f "$BUILDDIR/Makefile" ]; then
    echo "Usage: $0 <the .app> <online build dir> [<engine build dir>] [<build number>] [<side-car prefix>]" >&2
    exit 1
fi

SRCDIR=$(sed -n 's/^abs_top_srcdir = //p' "$BUILDDIR/Makefile")
if [ -z "$ENGINE_BUILDDIR" ]; then
    ENGINE_BUILDDIR=$(sed -n 's/^ENGINE_BUILDDIR = //p' "$BUILDDIR/Makefile")
fi
PYTHON=$(sed -n 's/^PYTHON = //p' "$BUILDDIR/Makefile")

make -C "$BUILDDIR/browser" sbom-shipped-js.txt

STAGING=$(mktemp -d)
trap 'rm -rf "$STAGING"' EXIT

"${PYTHON:-python3}" "$SRCDIR/scripts/create-product-sbom.py" all \
    --platform macos \
    --tree "$APP" \
    --staging "$STAGING/staging" \
    --srcdir "$SRCDIR" \
    --builddir "$BUILDDIR" \
    --engine-builddir "$ENGINE_BUILDDIR" \
    --engine-root "CollaboraOffice.app/=" \
    --hash-basis pre-signing \
    ${BUILD:+--build "$BUILD"} \
    ${SIDECAR:+--sidecar "$SIDECAR"}
