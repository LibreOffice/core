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
# To make the tarball reproducible, use a timestamp of a file inside the tarball (they all seem to have the same mtime).
if ! test -f "$tmpdir/$dir/README.md"; then
    echo timestamp retrieval failed, check the script
    rm -rf "$tmpdir"
    exit 1
fi
# Many of the options are to make the tarball reproducible.
LC_ALL=C tar c -C "$tmpdir" --xz -f "$(pwd)/$name" --format=gnu --sort=name --owner=0 --group=0 --mode=go=rX,u=rwX --mtime "$tmpdir/$dir/README.md" "$dir"
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
