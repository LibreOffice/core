#!/usr/bin/env bash
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Extracts the NSLocalizedString() keys and comments of the given source
# files into a UTF-8 Localizable.strings, the way ibtool_utf8.sh does it
# for a storyboard.

set -euo pipefail

if [ $# -lt 2 ]; then
    echo "Usage: $0 OUTPUT.strings SOURCE..." >&2
    exit 1
fi

OUT="$1"
shift

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

# The extractor warns about a key it cannot read, such as one built with
# string interpolation. Such a string would ship in English only, so stop.
if ! xcrun extractLocStrings -u -o "$TMP" "$@" 2> "$TMP/warnings"; then
    cat "$TMP/warnings" >&2
    exit 1
fi
if [ -s "$TMP/warnings" ]; then
    cat "$TMP/warnings" >&2
    echo "$0: the extractor warned, see above" >&2
    exit 1
fi

iconv -f UTF-16 -t UTF-8 "$TMP/Localizable.strings" > "$OUT"
