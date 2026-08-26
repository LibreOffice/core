#!/usr/bin/env bash
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
