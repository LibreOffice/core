# Server-side Python macro test for the container images

Checks whether a Collabora Online container can execute server-side Python
macros (the `CallPythonScript` postMessage API, i.e. share-location scripts
under `/opt/collaboraoffice/share/Scripts/python`). The test macro does what
an integrator typically wants: it enables track changes programmatically
(`RecordChanges = True`) and inserts a marker string, which the test then
reads back over the websocket.

## How it works

- `wopi.py` - a minimal WOPI host serving `test.odt`, run on the host; the
  container reaches it via the docker bridge gateway.
- `macros/macro_test.py` - the share-location Python macro under test.
- `wsclient.py` - speaks the COOL websocket protocol: loads the document,
  dispatches `uno vnd.sun.star.script:macro_test.py$enable_track_changes
  ?language=Python&location=share` (exactly what CallPythonScript sends),
  then polls the document text for the marker. Traffic is traced to
  `wsclient-<label>.log`.
- `run-case.sh <image> <label> [--install-pyuno]` - runs one image with
  macro execution enabled (`extra_params=--o:security.enable_macros_execution=
  true --o:security.macro_security_level=0`), the macro bind-mounted into
  the share scripts directory, and reports PASS/FAIL.
  `--install-pyuno` instead first installs `collaboraofficebasis-pyuno` and
  `collaboraofficebasis-python-script-provider` via `docker exec` + `apt-get`
  (the classic enablement path on the Debian-based image; version-pinned to
  the installed collaboraofficebasis-core) and copies the macro in with
  `docker cp`.

## Findings (2026-08-24, CODE 26.04)

| case | image | result |
|---|---|---|
| debian-stock | collabora/code:26.04.2.1.1 (Debian-based) | FAIL - pyuno not installed |
| debian-pyuno | same + `docker exec` apt install of pyuno | PASS - macro runs, TrackChanges=true |
| distroless-published-stock | collabora/code:26.04.3.1.1 (published distroless) | FAIL - pyuno not installed |
| distroless-stock | local distroless from-packages build | FAIL - pyuno not installed |
| distroless exec | `docker exec sh` / `apt-get` | impossible - no shell, no package manager |
| distroless-pyuno | distroless rebuilt with `--build-arg withpython=yes` | PASS - macro runs, TrackChanges=true |

Python macros have never worked out of the box: `coolwsd` does not depend on
`collaboraofficebasis-pyuno` / `collaboraofficebasis-python-script-provider`,
so no image ships them. What the distroless move removed is the *runtime
enablement path* (`docker exec` + `apt-get`); the replacement is to build the
image with `--build-arg withpython=yes`, which installs the two packages in
the builder stage - `assemble-rootfs.sh` picks them and their library closure
up automatically.
