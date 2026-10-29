#!/bin/bash
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: Apache-2.0
#
# Build the brand packages of Collabora Online Development Edition (code-brand)
# and Collabora Online (collabora-online-brand) from browser/branding.
#
# Usage: make-brand-packages.sh [options] OUTDIR
#
#   --version V-R     package version and revision, like 26.04.6.0-1
#                     (default: the version of configure.ac with revision 1)
#   --package NAME    build only code-brand or collabora-online-brand
#   --format FORMAT   deb, rpm or all (default: all)
#   --overlay DIR     copy the contents of DIR into the dist directory of
#                     code-brand: the Nextcloud community theme, which the
#                     online-branding repository generates as nextcloud/
#
# Environment: SASS (the dart-sass binary; default: browser/node_modules/.bin/sass
# when present, else sass on the PATH), PYTHON (default: python3).
#
# The packages are not signed; the repository jobs sign them.

set -e

usage()
{
    sed -n '/^# Usage/,/^# The packages/p' "$0" | sed 's/^# \{0,1\}//' 1>&2
    exit 1
}

SRCDIR=$(cd "$(dirname "$0")" && pwd)
BRANDING_DIR=$(dirname "$SRCDIR")
TOPDIR=$(cd "$BRANDING_DIR/../.." && pwd)

VERSION=
PACKAGES="code-brand collabora-online-brand"
FORMAT=all
OVERLAY=
while [ $# -gt 0 ]; do
    case "$1" in
        --version) VERSION="$2"; shift 2 ;;
        --package) PACKAGES="$2"; shift 2 ;;
        --format) FORMAT="$2"; shift 2 ;;
        --overlay) OVERLAY=$(cd "$2" && pwd); shift 2 ;;
        -*) usage ;;
        *) break ;;
    esac
done
[ $# -eq 1 ] || usage
mkdir -p "$1"
OUTDIR=$(cd "$1" && pwd)

for p in $PACKAGES; do
    case "$p" in
        code-brand|collabora-online-brand) ;;
        *) echo "make-brand-packages.sh: unknown package '$p'" 1>&2; exit 1 ;;
    esac
done
case "$FORMAT" in
    deb|rpm|all) ;;
    *) echo "make-brand-packages.sh: unknown format '$FORMAT'" 1>&2; exit 1 ;;
esac
if [ -n "$OVERLAY" ] && [ "$PACKAGES" = collabora-online-brand ]; then
    echo "make-brand-packages.sh: the overlay goes into code-brand only" 1>&2
    exit 1
fi

if [ -z "$VERSION" ]; then
    VERSION=$(sed -n 's/^AC_INIT(\[coolwsd\], \[\([^]]*\)\].*/\1/p' "$TOPDIR/configure.ac")
    [ -n "$VERSION" ] || { echo "make-brand-packages.sh: no version in configure.ac, use --version" 1>&2; exit 1; }
    VERSION="$VERSION-1"
fi
case "$VERSION" in
    *-*) ;;
    *) echo "make-brand-packages.sh: the version needs a revision, like $VERSION-1" 1>&2; exit 1 ;;
esac
UPSTREAM_VERSION=${VERSION%-*}
REVISION=${VERSION##*-}

if [ -z "$SASS" ]; then
    if [ -x "$TOPDIR/browser/node_modules/.bin/sass" ]; then
        SASS="$TOPDIR/browser/node_modules/.bin/sass"
    else
        SASS=sass
    fi
fi
export SASS
export SASS_FLAGS="${SASS_FLAGS:---quiet}"
PYTHON="${PYTHON:-python3}"

DATE=$(date -R)
YEAR=$(date +%Y)
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

build_package()
{
    local package="$1" theme="$2" rpm_name="$3"
    local stage="$WORK/$package"
    local dist="$stage/usr/share/coolwsd/browser/dist"
    local doc="$stage/usr/share/doc/$package"

    echo "Building $package $VERSION ($theme)"
    mkdir -p "$dist" "$doc"
    BRAND_VERSION="$VERSION" "$BRANDING_DIR/brand.sh" "$dist" "$theme"
    if [ -n "$OVERLAY" ] && [ "$package" = code-brand ]; then
        cp -a "$OVERLAY/." "$dist/"
    fi

    cp "$BRANDING_DIR/LICENSE" "$doc/LICENSE"
    sed -e "s/@PACKAGE@/$package/g" -e "s/@YEAR@/$YEAR/g" "$SRCDIR/copyright.in" > "$doc/copyright"
    if [ -d "$dist/nextcloud" ]; then
        sed -e "s/@YEAR@/$YEAR/g" "$SRCDIR/copyright-nextcloud.in" >> "$doc/copyright"
    fi
    "$PYTHON" "$TOPDIR/scripts/create-sbom.py" --srcdir "$TOPDIR" \
        --brand-package "$package" --package-root "$stage" --version "$VERSION" \
        --output "$doc/$package-sbom.spdx.json"
    find "$stage" -type d -exec chmod 755 {} +
    find "$stage" -type f -exec chmod 644 {} +

    if [ "$FORMAT" != deb ]; then
        local rpmtop="$WORK/rpmbuild-$package"
        mkdir -p "$rpmtop/SOURCES" "$rpmtop/SPECS"
        tar czf "$rpmtop/SOURCES/$package.tar.gz" -C "$stage" usr
        sed -e "s/@VERSION@/$UPSTREAM_VERSION/" -e "s/@RELEASE@/$REVISION/" \
            "$SRCDIR/$rpm_name.spec.in" > "$rpmtop/SPECS/$rpm_name.spec"
        rpmbuild -bb --quiet --define "_topdir $rpmtop" "$rpmtop/SPECS/$rpm_name.spec"
        cp "$rpmtop/RPMS/noarch/$rpm_name-$VERSION.noarch.rpm" "$OUTDIR/"
    fi

    if [ "$FORMAT" != rpm ]; then
        {
            echo "$package ($VERSION) unstable; urgency=medium"
            echo
            echo "  * Built from browser/branding of Collabora Online $UPSTREAM_VERSION."
            echo
            echo " -- Collabora Productivity <hello@collaboraoffice.com>  $DATE"
        } | gzip -9n > "$doc/changelog.Debian.gz"
        chmod 644 "$doc/changelog.Debian.gz"
        mkdir -p "$stage/DEBIAN"
        sed -e "s/@VERSION@/$VERSION/" "$SRCDIR/$package.control.in" > "$stage/DEBIAN/control"
        (cd "$stage" && find . -type f -printf '%P\n' | grep -v '^DEBIAN/' | sort | xargs md5sum > DEBIAN/md5sums)
        dpkg-deb --root-owner-group --build "$stage" "$OUTDIR/${package}_${VERSION}_all.deb"
    fi
}

for p in $PACKAGES; do
    case "$p" in
        code-brand) build_package code-brand CODE CODE-brand ;;
        collabora-online-brand) build_package collabora-online-brand CO-theme collabora-online-brand ;;
    esac
done

echo "Packages in $OUTDIR:"
ls -1 "$OUTDIR"
