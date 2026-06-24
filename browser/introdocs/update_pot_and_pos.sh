#!/usr/bin/env bash
#
# Regenerate the .pot template for every introductory document from its
# "_"-marked strings, and merge those new strings into any existing .po files.
#
set -euo pipefail
cd "$(dirname "$0")"

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
    $PYTHON extract_odf_text.py "$doc" "$pot"

    echo "2) Merging existing $name-*.po files"
    for po in "$PO_DIR/$name"-*.po; do
        [ -f "$po" ] || continue
        echo "   msgmerge: $po"
        $MSGMERGE --update --backup=none "$po" "$pot"
    done
    echo
done

echo "Done."
