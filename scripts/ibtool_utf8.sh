#!/usr/bin/env bash
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -euo pipefail

if [ $# -ne 2 ]; then
    echo "Usage: $0 INPUT.storyboard OUTPUT.strings" >&2
    exit 1
fi

IN="$1"
OUT="$2"
TMP="${OUT}.utf16"

ibtool --generate-strings-file "$TMP" "$IN"
iconv -f UTF-16 -t UTF-8 "$TMP" > "$OUT"
rm -f "$TMP"
