# Snap package for Collabora Office (coda-qt)

This directory must live at the repository root as `snap/` because snapcraft
expects `snap/snapcraft.yaml` relative to the project directory. If it were
nested deeper (e.g. `qt/snap/`), snapcraft would exclude the project directory
from the source copy of parts that use `source: .`, breaking the build.

## Files

- `snapcraft.yaml` — snap package definition. Four parts: `local-parts` (the
  command-chain wrapper), `collabora-office` (the engine plus Collabora Online
  and coda-qt in one build — POCO is built within the engine), `branding`, and
  `mythes-pl`.
- `local/bin/set-chromium-flags` — command-chain wrapper that assembles
  `QTWEBENGINE_CHROMIUM_FLAGS` at launch
- `hooks/configure` — initialises the `enable-gpu` snap option
- `qtpaths6-wrapper` — shadows the SDK snap's `qtpaths6`, whose compiled-in
  multiarch defaults do not match its own layout

The following are **unmaintained** and not used for releases. They describe a
Docker-based build that differs from the manifest in ways that matter — the
Dockerfile substitutes the distribution's `qt6-*-dev` packages for the
`kde-qt6-core24-sdk` build-snap and stubs out `snap` and `snapctl` — so a
result obtained through them does not necessarily hold for the shipped build:

- `Dockerfile`, `docker-build.sh`, `ci-build.sh`, `Jenkinsfile`,
  `snapcraft-wrapper`, `snap-stub`

## Building

Requires snapcraft and LXD on an Ubuntu host. From the repository root:

    snapcraft pack

snapcraft builds inside an LXD container; the source tree is left clean. The
first build takes several hours, dominated by the engine.

To rebuild only part of it, clean the specific part rather than deleting
`stage/` or `prime/` by hand — removing those wipes every part's staged state
and forces a full rebuild:

    snapcraft clean collabora-office

## Publishing

    snapcraft login
    snapcraft upload collabora-office_*.snap --release=latest/edge

## Testing

    sudo snap install collabora-office --edge

See also https://snapcraft.io/collabora-office

## Known issues

### WSL2: first launch may abort once

Recorded before the startup abort fixed in "snap: fix the intermittent abort
at startup" was understood, and **not re-verified since**. It reads:

> On WSL2 (the snap works fine on real Ubuntu 24.04 and other distros) the
> very first launch after install sometimes aborts silently, and re-running
> succeeds. The abort is in the Mesa shader-compilation path used by
> QtWebEngine's GL init on WSL2's dxgkrnl-backed `llvmpipe` stack.

Treat that with suspicion. Its premise is now known to be false: the snap did
*not* work reliably on real Ubuntu 24.04 — roughly half of all launches
aborted in Chromium's user-namespace probe, which produces the same
"launch fails once, works on retry" shape. The two may well be the same bug.
Re-test on WSL2 before spending any effort on the Mesa theory.
