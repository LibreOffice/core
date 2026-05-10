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
CONTAINER="snap-builder-$$"
VOLUME=collabora-snap-build
EXEC="docker exec -e PATH=/snap/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

cleanup() {
    echo "==> Cleaning up container..."
    docker stop "$CONTAINER" 2>/dev/null || true
    docker rm "$CONTAINER" 2>/dev/null || true
}
trap cleanup EXIT

if [ "$1" = "--clean" ]; then
    echo "==> Wiping build cache..."
    docker volume rm "$VOLUME" 2>/dev/null || true
fi

# --- build the Docker image ---
echo "==> Building Docker image..."
docker build -t "$IMAGE" "$SCRIPT_DIR"

# --- start container with systemd (for snapd) ---
echo "==> Starting container..."
docker run -d --privileged \
    --name "$CONTAINER" \
    -v "$REPO_ROOT":/src:ro \
    -v "$VOLUME":/build \
    "$IMAGE"

# --- wait for snapd ---
echo "==> Waiting for snapd..."
for i in $(seq 1 60); do
    if docker exec "$CONTAINER" snap version &>/dev/null; then
        echo "    snapd is ready"
        break
    fi
    sleep 1
done

# --- install snapcraft ---
if ! $EXEC "$CONTAINER" bash -c 'command -v snapcraft' &>/dev/null; then
    echo "==> Installing snapcraft..."
    docker exec "$CONTAINER" snap install snapcraft --classic
fi

# --- sync source ---
echo "==> Syncing source..."
$EXEC "$CONTAINER" rsync -a \
    --exclude='.git' \
    --exclude='node_modules' \
    --exclude='parts' \
    --exclude='stage' \
    --exclude='prime' \
    --exclude='snap/output' \
    --exclude='engine/workdir' \
    --exclude='engine/instdir' \
    --exclude='engine/inst' \
    --exclude='engine/autom4te.cache' \
    --exclude='engine/translations' \
    --exclude='browser/dist' \
    --exclude='browser/node_modules' \
    /src/ /build/

# --- build ---
echo "==> Building snap..."
$EXEC -w /build "$CONTAINER" \
    snapcraft --destructive-mode --verbosity=verbose

# --- extract result ---
mkdir -p "$SCRIPT_DIR/output"
docker cp "$CONTAINER:/build/collabora-office_*.snap" "$SCRIPT_DIR/output/"

echo "==> Done!"
ls -lh "$SCRIPT_DIR/output/"*.snap
