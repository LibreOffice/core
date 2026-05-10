# Snap package for Collabora Office (coda-qt)

This directory must live at the repository root as `snap/` because snapcraft
expects `snap/snapcraft.yaml` relative to the project directory. If it were
nested deeper (e.g. `qt/snap/`), snapcraft would exclude the project directory
from the source copy of parts that use `source: .`, breaking the build.

## Files

- `snapcraft.yaml` — snap package definition (3 parts: Poco, the engine +
   Collabora Online + coda-qt, branding)
- `Dockerfile` — Ubuntu 24.04 build image with systemd for snapd support
- `docker-build.sh` — local build script (interactive, reusable container)
- `ci-build.sh` — CI build script (disposable container, Jenkins/other CI)
- `Jenkinsfile` — declarative Jenkins pipeline

## Building locally

Requires Docker. The core build takes several hours on first run;
subsequent builds are incremental via a named Docker volume.

    snap/docker-build.sh

The resulting `.snap` file is written to `snap/output/`.

## Building on CI

    snap/ci-build.sh

Or use `snap/Jenkinsfile` for a Jenkins Pipeline job. The only requirement on
the CI host is Docker — works on any distro (AlmaLinux, Ubuntu, etc.).

## Publishing

    snapcraft login
    snapcraft upload snap/output/collabora-office_*.snap --release=edge

## Testing

Testers need to install from the edge channel:

    sudo snap install collabora-office --edge

See also https://snapcraft.io/collabora-office


