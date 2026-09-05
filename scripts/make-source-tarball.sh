#!/bin/sh
# Create the release source tarball for a tag: the monorepo tree at that tag
# with the translations repository, which carries the same tag, placed at
# engine/translations. The result is what the FreeBSD port and other
# from-source packagers build from, so it must contain everything the build
# needs without network access: the engine, the browser sources with the npm
# shrinkpack, and the translations. The engine's external tarballs and the
# branding are fetched separately by the packaging recipes.
#
# Usage: scripts/make-source-tarball.sh <tag> [output-dir]
#
# Run from a checkout of the online repository that has the translations
# repository cloned at engine/translations. The tag name minus its product
# prefix becomes the version, so cp-26.04.3-2 gives
# collabora-office-26.04.3-2.tar.xz.

set -e

tag=$1
outdir=${2:-.}

if [ -z "$tag" ]; then
    echo "usage: $0 <tag> [output-dir]" >&2
    exit 1
fi

top=$(git rev-parse --show-toplevel)
translations=$top/engine/translations

if [ ! -d "$translations/.git" ]; then
    echo "$translations is not a git checkout; clone the translations repository there first" >&2
    exit 1
fi

for repo in "$top" "$translations"; do
    if ! git -C "$repo" rev-parse -q --verify "refs/tags/$tag" >/dev/null; then
        echo "tag $tag does not exist in $repo" >&2
        exit 1
    fi
done

version=$(echo "$tag" | sed -E 's/^[a-z]+-//')
name=collabora-office-$version
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

git -C "$top" archive --format=tar --prefix="$name/" -o "$tmp/online.tar" "$tag"
git -C "$translations" archive --format=tar --prefix="$name/engine/translations/" \
    -o "$tmp/translations.tar" "$tag"

# GNU tar appends one archive to another; the translations tree lands under
# engine/translations inside the same top directory.
tar -Af "$tmp/online.tar" "$tmp/translations.tar"

mkdir -p "$outdir"
xz -T0 -c "$tmp/online.tar" > "$outdir/$name.tar.xz"

echo "$outdir/$name.tar.xz"
sha256sum "$outdir/$name.tar.xz"
