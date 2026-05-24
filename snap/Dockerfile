FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LC_ALL=C.UTF-8

# We deliberately avoid running snapd inside the container. Snapd's configure
# hook for the snapcraft snap calls aa_is_enabled(), which fails whenever
# /sys/kernel/security/apparmor isn't visible to the container — true in
# plain Docker on any host, and on WSL2 (no AppArmor in the kernel) it can't
# be fixed at all. Instead we fetch the snapcraft snap from the store and
# unsquashfs it directly: it's classic-confined, so it's just a self-contained
# Python virtualenv that runs fine without snapd's confinement machinery.
RUN apt-get update && \
    apt-get install -y \
        ca-certificates curl jq \
        squashfs-tools \
        patchelf \
        ccache \
        git rsync sudo \
        # Runtime libs the snapcraft snap's bundled binaries need
        # (the snap was linked expecting core24 to provide them).
        libyaml-0-2 \
        # Qt6 dev packages (replace kde-qt6-core24-sdk build-snap)
        qt6-webengine-dev qt6-base-dev qt6-declarative-dev \
        qt6-tools-dev qt6-tools-dev-tools \
    && apt-get clean

# Fetch latest stable snapcraft snap and unpack at the path it hardcodes
# (the bundled venv's pyvenv.cfg and hook shebangs reference
# /snap/snapcraft/current explicitly, so we must extract there).
RUN set -eux; \
    arch=$(dpkg --print-architecture); \
    info=$(curl -fsSL \
            -H "Snap-Device-Series: 16" \
            -H "Snap-Device-Architecture: $arch" \
            "https://api.snapcraft.io/v2/snaps/info/snapcraft?fields=download,version"); \
    url=$(echo "$info" | jq -r --arg a "$arch" \
            '."channel-map"[] | select(.channel.name=="stable" and .channel.architecture==$a) | .download.url' \
          | head -n1); \
    version=$(echo "$info" | jq -r --arg a "$arch" \
            '."channel-map"[] | select(.channel.name=="stable" and .channel.architecture==$a) | .version' \
          | head -n1); \
    test -n "$url"; \
    curl -fsSL "$url" -o /tmp/snapcraft.snap; \
    mkdir -p /snap/snapcraft; \
    unsquashfs -q -n -d /snap/snapcraft/current /tmp/snapcraft.snap; \
    rm /tmp/snapcraft.snap; \
    echo "$version" > /snap/snapcraft/current/.snapcraft-version

# The snapcraft snap's bundled python3 has its ELF interpreter and RPATH
# pinned to /snap/core24/current/lib*/... — paths normally provided by the
# core24 base snap. The container IS Ubuntu 24.04 (binary-identical to
# core24 for the C library, expat, zlib, etc. that the snap's python links
# against), so satisfy those paths with a symlink to / instead of unpacking
# a second 250MB snap.
RUN mkdir -p /snap/core24 && ln -s / /snap/core24/current

# Snapcraft uses snapctl (normally provided by snapd) to read snap config
# keys. We never set any, so return an empty JSON object for `get` and
# treat everything else as a no-op.
RUN printf '#!/bin/sh\ncase "$1" in get) echo "{}" ;; *) exit 0 ;; esac\n' \
        > /usr/bin/snapctl && \
    chmod +x /usr/bin/snapctl

COPY snapcraft-wrapper /usr/local/bin/snapcraft
COPY snap-stub /usr/local/bin/snap
RUN chmod +x /usr/local/bin/snapcraft /usr/local/bin/snap
