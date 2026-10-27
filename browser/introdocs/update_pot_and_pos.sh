#!/usr/bin/env bash
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Regenerate the .pot template for every introductory document from its
# "_"-marked strings, and merge those new strings into any existing .po files.
# With a directory argument it does the same for the documents in that
# directory, like ../welcome-slideshow.
#
set -euo pipefail
TOOLDIR="$(cd "$(dirname "$0")" && pwd)"
cd "${1:-$TOOLDIR}"

PYTHON="${PYTHON:-python3}"
MSGMERGE="${MSGMERGE:-msgmerge}"
POT_DIR="po/templates"
PO_DIR="po"

mkdir -p "$POT_DIR" "$PO_DIR"
shopt -s nullglob

for doc in *.odt *.ods *.odp; do
    name="${doc%.*}"
    pot="$POT_DIR/$name.pot"

    echo "1) Extracting strings from $doc -> $pot"
    $PYTHON "$TOOLDIR/extract_odf_text.py" "$doc" "$pot"

    echo "2) Merging existing $name-*.po files"
    for po in "$PO_DIR/$name"-*.po; do
        [ -f "$po" ] || continue
        echo "   msgmerge: $po"
        $MSGMERGE --update --backup=none "$po" "$pot"
    done
    echo
done

echo "Done."
