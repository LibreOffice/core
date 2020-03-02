#! /bin/sh

# Repack the boost tarball as xz (much faster to unpack) and remove
# a lot of needless files such as generated html docs.

tarball="$1"

if test -z "$tarball" -o ! -f "$tarball"; then
    echo "Usage: $0 <tarball>"
    exit 1
fi

tmpdir=$(mktemp -d)

if ! test -d "$tmpdir"; then
    echo mktemp failed
    exit 1
fi

echo Unpacking "$tarball" ...
tar x -C "$tmpdir" -f "$tarball"
if test $? -ne 0; then
    echo tar x failed
    rm -rf "$tmpdir"
    exit 1
fi

echo Removing unnecessary files ...
find "$tmpdir" \( -name doc -o -name test -o -name example \) -type d -prune -print0 | xargs -0 rm -r
if test $? -ne 0; then
    echo file removal failed
    rm -rf "$tmpdir"
    exit 1
fi

name="$(basename "$tarball" | sed 's/\.tar.*$//').tar.xz"
dir=$(ls "$tmpdir")

echo Creating "$name" ...
tar c -C "$tmpdir" --xz -f "$(pwd)/$name" "$dir"
if test $? -ne 0; then
    echo tar c failed
    rm -rf "$tmpdir"
    exit 1
fi

echo Cleaning up ...
rm -rf "$tmpdir"

sha256sum "$name"

echo Done.
exit 0
