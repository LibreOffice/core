#!/usr/bin/env bash
#
# Cached wrapper around `make fetch` for CI source tarball generation.
#
# Instead of `rm -rf $TARFILE_LOCATION/* && make fetch` (which re-downloads
# ~500MB every time), this script uses a persistent cache directory to avoid
# redundant downloads and then prunes stale tarballs.
#
# Usage:
#   bin/cached-fetch.sh [CACHE_DIR]
#
# CACHE_DIR defaults to $HOME/.cache/lo-tarballs.
# TARFILE_LOCATION is read from config_host.mk (or defaults to external/tarballs).

set -euo pipefail

SRCDIR=$(cd "$(dirname "$0")/.." && pwd)
CACHE_DIR="${1:-${HOME}/.cache/lo-tarballs}"

# Get TARFILE_LOCATION from build config
if [ -f "$SRCDIR/config_host.mk" ]; then
    TARFILE_LOCATION=$(sed -n 's/^export TARFILE_LOCATION=//p' "$SRCDIR/config_host.mk")
fi
TARFILE_LOCATION="${TARFILE_LOCATION:-$SRCDIR/external/tarballs}"

echo "Cache dir:        $CACHE_DIR"
echo "Tarfile location: $TARFILE_LOCATION"

mkdir -p "$CACHE_DIR" "$TARFILE_LOCATION/tmp"

# Ask make for the list of needed tarball filenames
NEEDED=$(make -C "$SRCDIR" fetch-names)

# Clean target dir and pre-populate needed files from cache
rm -rf "$TARFILE_LOCATION"/*
mkdir -p "$TARFILE_LOCATION/tmp"

cached=0
for f in $NEEDED; do
    if [ -f "$CACHE_DIR/$f" ]; then
        # Hardlink from cache (fast, no extra disk).
        # Fall back to copy if on a different filesystem.
        ln "$CACHE_DIR/$f" "$TARFILE_LOCATION/$f" 2>/dev/null ||
            cp "$CACHE_DIR/$f" "$TARFILE_LOCATION/$f"
        cached=$((cached + 1))
    fi
done
echo "Restored from cache: $cached"

# Fetch anything not already present (new/updated tarballs only)
make -C "$SRCDIR" fetch

# Update cache with any newly downloaded tarballs
new=0
for f in $NEEDED; do
    if [ -f "$TARFILE_LOCATION/$f" ] && [ ! -f "$CACHE_DIR/$f" ]; then
        cp "$TARFILE_LOCATION/$f" "$CACHE_DIR/$f"
        new=$((new + 1))
    fi
done
[ "$new" -gt 0 ] && echo "Added to cache: $new new tarballs"

echo "Done. $TARFILE_LOCATION is ready for source tarball packaging."
