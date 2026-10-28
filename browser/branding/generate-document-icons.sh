#!/bin/sh
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: Apache-2.0
#
# Generate macOS document type icons from SVGs
# This script converts x-office-*.svg files to PNG format for use in macOS apps
set -e

# Check arguments
if [ -z "$1" ]; then
    echo "Usage: $0 <svg-source-dir> <output-dir>"
    echo "  svg-source-dir: Directory containing x-office-*.svg files"
    echo "  output-dir: Directory where PNG files will be created"
    exit 1
fi

SVG_DIR="$1"
OUTPUT_DIR="$2"

# Validate directories
if [ ! -d "$SVG_DIR" ]; then
    echo "Error: SVG source directory '$SVG_DIR' not found"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

# Convert SVGs to PNGs using sips (macOS built-in tool)
# Generate 512x512 PNG files suitable for macOS document icons
# Handle both standard names and _branding suffix variants
for svg_file in "$SVG_DIR"/x-office-*.svg; do
    if [ -f "$svg_file" ]; then
        basename=$(basename "$svg_file" .svg)

        # Remove _branding suffix if present
        basename_clean=$(echo "$basename" | sed 's/_branding$//')

        # Normalize x-office-text to x-office-document for macOS
        basename_clean=$(echo "$basename_clean" | sed 's/x-office-text/x-office-document/')

        png_file="$OUTPUT_DIR/${basename_clean}.png"

        echo "  Converting ${basename}.svg -> ${basename_clean}.png"
        sips -s format png -z 512 512 "$svg_file" --out "$png_file" >/dev/null 2>&1
    fi
done

echo "Document type icons generated successfully in $OUTPUT_DIR"
