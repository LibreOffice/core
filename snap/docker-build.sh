#!/bin/bash
#
# Build the collabora-office snap inside a Docker container.
#
# Requirements: Docker (works on WSL2).
#
# Build state persists in the "collabora-snap-build" named volume,
# so re-runs after a failure pick up where they left off.
#
# To start completely fresh:
#   docker volume rm collabora-snap-build
#
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

IMAGE=collabora-snap-builder
VOLUME=collabora-snap-build

echo "==> Building Docker image..."
docker build -t "$IMAGE" "$SCRIPT_DIR"

# configure.ac prefers dist_git_hash over `git log` (which would fail anyway
# inside the container, since the rsync excludes .git). Compute it on the
# host and hand it in via env var, so the About dialog shows a real commit.
GIT_HASH=$(cd "$REPO_ROOT" && git log -1 --format=%h --abbrev=10 2>/dev/null || true)

mkdir -p "$SCRIPT_DIR/output" "$SCRIPT_DIR/output/logs"

echo "==> Building snap (this will take a long time)..."
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

        # Skip if /build already has a dist_git_hash from a release tarball.
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

        # Scrub stale Ubuntu libreoffice-core files. Snapcraft would put
        # $CRAFT_STAGE/usr/lib/x86_64-linux-gnu/ on LD_LIBRARY_PATH during
        # build, and a stale libuno_sal.so.3 there gets loaded by engine
        # build tools (svidl) in preference to the freshly-built one,
        # producing undefined-symbol errors for any sal symbol added
        # after the Ubuntu release. The yaml fix (extracting mythes-pl as
        # a separate part) removes the source of the libreoffice-core
        # dep, but a prior run may have already deposited the tree into
        # /build/stage and /build/parts/*/install. Idempotent: no-op if
        # the files were never staged or are already gone.
        for d in /build/stage /build/parts/collabora-office/install; do
            rm -rf "$d/usr/lib/libreoffice" 2>/dev/null || true
            rm -f  "$d/usr/lib/x86_64-linux-gnu/libuno"* 2>/dev/null || true
        done

        cd /build
        snapcraft --destructive-mode "$@"

        echo "==> Copying .snap to output/..."
        cp /build/*.snap /output/ 2>/dev/null || true
    ' bash "$@"

echo "==> Done!"
ls -lh "$SCRIPT_DIR/output/"*.snap 2>/dev/null || echo "(no .snap found — build may have failed)"
