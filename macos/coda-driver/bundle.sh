#!/bin/bash
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Build coda-driver and wrap it in a .app bundle, signed with the
# user's normal Xcode code-signing identity (typically "Apple
# Development", which Xcode auto-creates when you sign in with an
# Apple ID).  This is the same path Xcode uses to sign coda itself.
#
# Why we sign rather than ad-hoc:
#   - ad-hoc signatures embed the cdhash in the designated requirement,
#     so the macOS TCC Accessibility grant is invalidated on every
#     rebuild even though the bundle identifier is the same.
#   - A real code-signing certificate has a stable team identifier;
#     TCC tracks the grant by that, so it persists across rebuilds.
#
# If you do not have a code-signing identity yet, open Xcode ->
# Settings -> Accounts, add an Apple ID (the free one works), and
# let Xcode generate the "Apple Development" certificate.  On CI,
# whatever signing setup you use to build coda itself applies here.

set -euo pipefail

cd "$(dirname "$0")"

BUNDLE_ID="com.collabora.coda-driver"
APP="coda-driver.app"

# Pick a code-signing identity.  The env var CODA_DRIVER_SIGN_IDENTITY
# wins if set; otherwise prefer "Apple Development", falling back to
# any other code-signing identity in the keychain (covers manual
# self-signed certs and CI setups that use a different identity).
if [ -n "${CODA_DRIVER_SIGN_IDENTITY:-}" ]; then
    SIGNING_IDENTITY="$CODA_DRIVER_SIGN_IDENTITY"
else
    # security find-identity prints lines like
    #   1) ABCD... "Apple Development: Joe (TEAMID)"
    # Pull out the quoted names, drop the placeholder "-" (ad-hoc),
    # de-dup, prefer Apple Development.
    AVAILABLE=$(
        security find-identity -v -p codesigning 2>/dev/null \
            | sed -nE 's/^[[:space:]]*[0-9]+\)[[:space:]]+[A-F0-9]+[[:space:]]+"(.+)"$/\1/p' \
            | grep -v '^-$' \
            | sort -u
    )
    SIGNING_IDENTITY=$(echo "$AVAILABLE" | (grep -m1 '^Apple Development' || head -n1))
fi

if [ -z "$SIGNING_IDENTITY" ]; then
    cat >&2 <<'EOF'

============================================================
ERROR: No code-signing identity found in the Keychain.

The macOS test driver needs a code-signing identity so the
Accessibility grant in System Settings persists across rebuilds.
The easiest path is the same one Xcode uses to sign coda:

  Open Xcode -> Settings -> Accounts, sign in with an Apple ID
  (the free Apple ID is fine), and let Xcode auto-generate the
  "Apple Development" certificate.

After that, re-run the tests.  bundle.sh picks up the cert from
the keychain automatically.

On CI: configure the build environment with whatever Apple
code-signing identity you use to build coda itself.  Override
the chosen identity by setting CODA_DRIVER_SIGN_IDENTITY in
the environment.
============================================================

EOF
    exit 1
fi

echo "Signing identity: $SIGNING_IDENTITY"

# Build (incremental).
swift build -c release

BIN=".build/release/coda-driver"
CONTENTS="$APP/Contents"
MACOS="$CONTENTS/MacOS"

mkdir -p "$MACOS"
cp -f "$BIN" "$MACOS/coda-driver"

cat > "$CONTENTS/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
 "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>coda-driver</string>
    <key>CFBundleIdentifier</key>
    <string>$BUNDLE_ID</string>
    <key>CFBundleName</key>
    <string>coda-driver</string>
    <key>CFBundleDisplayName</key>
    <string>coda-driver</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>13.0</string>
    <key>LSUIElement</key>
    <true/>
</dict>
</plist>
PLIST

codesign --force --sign "$SIGNING_IDENTITY" \
    --identifier "$BUNDLE_ID" \
    "$APP"

echo "Built $APP"
