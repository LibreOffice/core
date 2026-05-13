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

## Known issues

### WSL2: first launch may abort once

On WSL2 (the snap works fine on real Ubuntu 24.04 and other distros) the
very first launch after install or after `rm -rf ~/snap/collabora-office`
sometimes aborts silently:

    $ collabora-office
    ... (Qt warnings)
    Aborted

Re-running succeeds and the app behaves normally from that point on. The
abort is in the Mesa shader-compilation path used by QtWebEngine's GL
init on WSL2's dxgkrnl-backed `llvmpipe` stack; the partial Mesa shader
cache that survives the crash is enough to let subsequent launches skip
the doomed code path. Diagnosed but not worked around in the snap
because the fix paths (pre-bake the Mesa cache; force QtWebEngine onto
SwiftShader) are fragile against noble Mesa/LLVM updates and the issue
doesn't affect non-WSL2 environments. If a clean first launch matters
to you on WSL2, run the app once, ignore the abort, run it again.
