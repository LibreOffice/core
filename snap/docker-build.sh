#!/bin/bash
#
# Build the collabora-office snap inside a Docker container.
#
# Requirements: Docker (works on WSL2).
#
# Uses systemd as PID 1 so snapd can run and install snapcraft.
# Build state persists in the "collabora-snap-build" named volume,
# so re-runs after a failure pick up where they left off.
#
# To start completely fresh:
#   docker rm -f snap-builder
#   docker volume rm collabora-snap-build
#
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

IMAGE=collabora-snap-builder
CONTAINER=snap-builder
VOLUME=collabora-snap-build
EXEC="docker exec -e PATH=/snap/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

echo "==> Building Docker image..."
docker build -t "$IMAGE" "$SCRIPT_DIR"

# Start (or reuse) the persistent container
if docker ps -a --format '{{.Names}}' | grep -qx "$CONTAINER"; then
    if ! docker ps --format '{{.Names}}' | grep -qx "$CONTAINER"; then
        echo "==> Starting existing container..."
        docker start "$CONTAINER"
    else
        echo "==> Container already running."
    fi
else
    echo "==> Creating container..."
    mkdir -p "$SCRIPT_DIR/output"
    docker run -d --privileged \
        --name "$CONTAINER" \
        -v "$REPO_ROOT":/src:ro \
        -v "$VOLUME":/build \
        -v "$SCRIPT_DIR/output":/output \
        "$IMAGE"
fi

# Wait for snapd to be ready
echo "==> Waiting for snapd..."
for i in $(seq 1 60); do
    if docker exec "$CONTAINER" snap version &>/dev/null; then
        echo "    snapd is ready"
        break
    fi
    sleep 1
done

# Install snapcraft if needed
if ! $EXEC "$CONTAINER" bash -c 'command -v snapcraft' &>/dev/null; then
    echo "==> Installing snapcraft (one-time)..."
    docker exec "$CONTAINER" snap install snapcraft --classic
fi

# Sync source and build
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

# Carry the host's commit hash into the build. configure.ac prefers
# dist_git_hash over `git log` (which would fail anyway, since the
# rsync excludes .git), so without this file the About dialog ends
# up showing "git hash: <version-string>".
# Skip if /build already has a dist_git_hash from a release tarball,
# and skip if the host isn't a git checkout (e.g. tarball build).
if ! $EXEC "$CONTAINER" test -f /build/dist_git_hash; then
    GIT_HASH=$(cd "$REPO_ROOT" && git log -1 --format=%h --abbrev=10 2>/dev/null)
    if [ -n "$GIT_HASH" ]; then
        $EXEC "$CONTAINER" bash -c "echo '$GIT_HASH' > /build/dist_git_hash"
    fi
fi

echo "==> Building snap (this will take a long time)..."
$EXEC -w /build "$CONTAINER" \
    snapcraft --destructive-mode "$@"

# Copy result out
echo "==> Copying .snap to output/..."
$EXEC "$CONTAINER" bash -c 'cp /build/*.snap /output/ 2>/dev/null'
echo "==> Done!"
ls -lh "$SCRIPT_DIR/output/"*.snap 2>/dev/null || echo "(no .snap found — build may have failed)"
