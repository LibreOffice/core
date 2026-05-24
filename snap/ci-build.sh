#!/bin/bash
#
# CI build script for the collabora-office snap.
#
# Requirements: Docker on the CI host (any distro — AlmaLinux, Ubuntu, etc.)
# The build runs entirely inside a Docker container.
#
# Usage:
#   snap/ci-build.sh [--clean]
#
# The "collabora-snap-build" Docker volume caches the LibreOffice core
# build across runs (~6h first build, minutes for incremental).
# Pass --clean to wipe it and start fresh.
#
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

IMAGE=collabora-snap-builder
VOLUME=collabora-snap-build

if [ "$1" = "--clean" ]; then
    echo "==> Wiping build cache..."
    docker volume rm "$VOLUME" 2>/dev/null || true
    shift
fi

echo "==> Building Docker image..."
docker build -t "$IMAGE" "$SCRIPT_DIR"

# configure.ac prefers dist_git_hash over `git log` (which would fail anyway
# inside the container, since the rsync excludes .git). Compute it on the
# host and hand it in via env var.
GIT_HASH=$(cd "$REPO_ROOT" && git log -1 --format=%h --abbrev=10 2>/dev/null || true)

mkdir -p "$SCRIPT_DIR/output" "$SCRIPT_DIR/output/logs"

echo "==> Building snap..."
docker run --rm --privileged \
    -v "$REPO_ROOT":/src:ro \
    -v "$VOLUME":/build \
    -v "$SCRIPT_DIR/output":/output \
    -v "$SCRIPT_DIR/output/logs":/root/.local/state/snapcraft/log \
    -e GIT_HASH="$GIT_HASH" \
    "$IMAGE" \
    bash -c '
        set -e

        echo "==> Syncing source..."
        rsync -a \
            --exclude=.git \
            --exclude=node_modules \
            --exclude=parts \
            --exclude=stage \
            --exclude=prime \
            --exclude=snap/output \
            --exclude=engine/workdir \
            --exclude=engine/instdir \
            --exclude=engine/inst \
            --exclude=engine/autom4te.cache \
            --exclude=engine/translations \
            --exclude=browser/dist \
            --exclude=browser/node_modules \
            --exclude=qt/*.moc.cpp \
            --exclude=qt/*.o \
            --exclude=qt/coda-qt \
            /src/ /build/

        if [ -n "$GIT_HASH" ] && [ ! -f /build/dist_git_hash ]; then
            echo "$GIT_HASH" > /build/dist_git_hash
        fi

        # Purge any *.moc.cpp left in the volume from an earlier run that
        # rsynced them from a host Qt-6.8+ build (their include of
        # <QtCore/qtmochelpers.h> breaks compilation against Qt 6.4 in this
        # container). Sweeps the project copy AND the snapcraft pulled and
        # build-dir copies. Files generated inside the container do not have
        # the qtmochelpers include, so they are left alone.
        find /build -name "*.moc.cpp" 2>/dev/null \
            | xargs -r grep -l qtmochelpers 2>/dev/null \
            | xargs -r rm -f || true

        # Scrub stale Ubuntu libreoffice-core files from stage and the
        # part install dir (see snapcraft.yaml for context). Idempotent.
        for d in /build/stage /build/parts/collabora-office/install; do
            rm -rf "$d/usr/lib/libreoffice" 2>/dev/null || true
            rm -f  "$d/usr/lib/x86_64-linux-gnu/libuno"* 2>/dev/null || true
        done

        cd /build
        snapcraft --destructive-mode --verbosity=verbose

        cp /build/collabora-office_*.snap /output/
    '

echo "==> Done!"
ls -lh "$SCRIPT_DIR/output/"*.snap
