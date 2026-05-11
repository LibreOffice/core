This directory contains the host-driven from-source Docker build. `build.sh`
builds the engine (from the `engine/` subdir of the online monorepo) and
coolwsd on the host, then `docker build` packages the result using the
Dockerfile matching the host distribution (Ubuntu, Debian, Fedora, ArchLinux,
openSUSE).

`build-alpine.sh` plus the `Alpine` Dockerfile do a self-contained multi-stage
build inside the container (musl needs the engine to be built from source --
the prebuilt engine assets are glibc-only).
