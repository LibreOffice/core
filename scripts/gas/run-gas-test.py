#!/usr/bin/env -S uv run --script --quiet
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# /// script
# requires-python = ">=3.9"
# dependencies = [
#   "google-api-python-client",
#   "google-auth-oauthlib",
# ]
# ///

"""Run one of the JS tests under engine/scriptinterop/qa/unit/data/ on
Google Docs, to verify GAS parity of the test.  The single positional
argument names a stem: "document" runs document-test.js against
document-test.rtf, "utilities" runs utilities-test.js (no RTF needed),
and so on for any future <stem>-test.js / <stem>-test.rtf pair.  Every
such test file names its entry function test.  A stem starting with
"slides" runs against a Google Slides presentation instead of a Doc,
and needs a <stem>-test.pptx fixture: "slidesapp" runs
slidesapp-test.js against slidesapp-test.pptx.

For a stem that has a fixture, uploads it to Drive (asking Drive to
convert it to a Google Doc or presentation on the way in), creates a
bound Apps Script project holding the .js file, executes test()
server-side through the Apps Script API, and prints PASS or FAIL with
the exception text.  For a Doc stem without an .rtf fixture, reuses
the cached Doc as-is (its content is not observed by the test).

The Doc or presentation and the script are created on first run and
cached in test-state.json in the --state-dir directory (~/.gas by
default), so every subsequent run just overwrites the file's content
(via a fresh fixture upload that Drive re-converts in place) and
pushes the current <stem>-test.js source, then invokes test.  The one-time GCP
project pairing chore in step 2 below is done once, ever.  Use
--reset to throw the cache away and start over.

uv reads the inline script metadata above, keeps its own on-disk cache
of an isolated venv with those packages, and hands us a matching
interpreter.  No global user site-packages install happens.

One-time setup for the Google side:

1. Set up the Google login, as described at the top of gas_auth.py.

2. On the first run, this script creates the reusable Doc + bound
   script and prints the script's URL.  Before the second run can
   invoke test, pair the script with your GCP project:
     1. Open the script URL printed by the first run.
     2. Click the gear icon in the left rail ("Project Settings").
     3. Under "Google Cloud Platform (GCP) Project" click "Change
        project" and enter the project number that the first run
        prints (not the project id).  It is the digits before the
        first "-" of the client_id in client.json, as an OAuth client
        id starts with the number of the project that owns it.
     4. "Set project".  This pairing sticks with the script, so this
        step is one-time-per-cache-lifetime rather than per-run.

Usage:
    scripts/gas/run-gas-test.py document
    scripts/gas/run-gas-test.py utilities
    scripts/gas/run-gas-test.py slidesapp
    scripts/gas/run-gas-test.py document --reset   # throw away the cache
"""

import argparse
import json
import pathlib
import sys

from googleapiclient.discovery import build
from googleapiclient.errors import HttpError
from googleapiclient.http import MediaFileUpload

from gas_auth import add_state_dir_argument, get_credentials

DATA_DIR = (
    pathlib.Path(__file__).resolve().parents[2]
    / "engine" / "scriptinterop" / "qa" / "unit" / "data"
)


MANIFEST = {
    "timeZone": "Etc/GMT",
    "exceptionLogging": "STACKDRIVER",
    "runtimeVersion": "V8",
    "executionApi": {"access": "MYSELF"},
}


def load_state(state_file):
    if state_file.exists():
        return json.loads(state_file.read_text())
    return {}


def save_state(state_file, s):
    state_file.write_text(json.dumps(s, indent=2))


def gcp_project_number(state_dir):
    """The number of the GCP project that owns the OAuth client, which starts its client id."""
    client = json.loads((state_dir / "client.json").read_text())
    return next(iter(client.values()))["client_id"].split("-", 1)[0]


# The fixture of a Doc test is RTF and the fixture of a presentation test is PPTX.  Each entry
# maps the fixture's file extension to the media type of the upload and to the Google file type
# that Drive converts the upload into.
FIXTURE_KINDS = {
    "rtf": ("application/rtf", "application/vnd.google-apps.document"),
    "pptx": (
        "application/vnd.openxmlformats-officedocument.presentationml.presentation",
        "application/vnd.google-apps.presentation",
    ),
}


def upload_fixture(drive, fixture_path):
    """Upload the fixture as a new file, converted by Drive into a Doc or a presentation."""
    media_type, google_type = FIXTURE_KINDS[fixture_path.suffix[1:]]
    body = {"name": "gas-test " + fixture_path.name, "mimeType": google_type}
    media = MediaFileUpload(str(fixture_path), mimetype=media_type, resumable=False)
    result = drive.files().create(
        body=body, media_body=media, fields="id"
    ).execute()
    return result["id"]


def replace_file_content(drive, file_id, fixture_path):
    """Replace the target file's content with a fresh fixture upload; Drive
    re-runs its conversion on the media, so the resulting Doc or presentation
    has the same content it would have if we had uploaded the fixture as a new
    file."""
    media_type, _ = FIXTURE_KINDS[fixture_path.suffix[1:]]
    media = MediaFileUpload(str(fixture_path), mimetype=media_type, resumable=False)
    drive.files().update(fileId=file_id, media_body=media).execute()


def push_script_content(script, script_id, js_path):
    files = [
        {
            "name": "appsscript",
            "type": "JSON",
            "source": json.dumps(MANIFEST, indent=2),
        },
        {
            "name": "Code",
            "type": "SERVER_JS",
            "source": js_path.read_text(encoding="utf-8"),
        },
    ]
    script.projects().updateContent(
        scriptId=script_id, body={"files": files}
    ).execute()


def create_bound_script(script, doc_id, js_path):
    project = script.projects().create(
        body={"title": "gas-test", "parentId": doc_id}
    ).execute()
    script_id = project["scriptId"]
    push_script_content(script, script_id, js_path)
    return script_id


def run_function(script, script_id, function_name):
    resp = script.scripts().run(
        scriptId=script_id,
        body={"function": function_name, "devMode": True},
    ).execute()
    if "error" in resp:
        return False, format_error(resp["error"])
    return True, resp.get("response", {}).get("result")


def format_error(err):
    """Turn an Apps Script scripts.run error blob into a human-readable
    multi-line message.  The scriptStackTraceElements list carries the frame
    that actually raised, so pull line numbers out of it."""
    details = (err.get("details") or [{}])[0]
    parts = []
    msg = details.get("errorMessage") or err.get("message")
    if msg:
        parts.append(msg)
    for frame in details.get("scriptStackTraceElements") or []:
        line = frame.get("lineNumber")
        func = frame.get("function") or "<anonymous>"
        parts.append(
            "  at " + func + (" (line " + str(line) + ")" if line else "")
        )
    return "\n".join(parts) or json.dumps(err, indent=2)


def delete_quietly(drive, file_id):
    try:
        drive.files().delete(fileId=file_id).execute()
    except HttpError:
        pass


def main():
    ap = argparse.ArgumentParser(
        description=(
            "Run a Google Apps Script test named by its stem, against a"
            " Doc built from an RTF fixture when the stem has one, or"
            " against a presentation built from a PPTX fixture for a"
            " stem that starts with slides."
        )
    )
    ap.add_argument(
        "stem",
        help=(
            "test stem: run <stem>-test.js, invoke test, refresh"
            " the Doc from <stem>-test.rtf when that fixture exists,"
            " or the presentation from <stem>-test.pptx"
        ),
    )
    ap.add_argument(
        "--reset",
        action="store_true",
        help=(
            "delete the cached Doc and script, then start over (creates a"
            " fresh pair and prints the GCP-pairing chore again)"
        ),
    )
    add_state_dir_argument(ap)
    args = ap.parse_args()
    state_file = args.state_dir / "test-state.json"

    args.js = DATA_DIR / (args.stem + "-test.js")
    args.function = "test"
    is_presentation = args.stem.startswith("slides")
    fixture_candidate = DATA_DIR / (
        args.stem + "-test." + ("pptx" if is_presentation else "rtf")
    )
    args.fixture = fixture_candidate if fixture_candidate.exists() else None
    if is_presentation and args.fixture is None:
        sys.exit("missing fixture: " + str(fixture_candidate))
    file_key, script_key = (
        ("presentation_id", "presentation_script_id") if is_presentation
        else ("doc_id", "script_id")
    )

    if not args.js.exists():
        sys.exit("missing fixture: " + str(args.js))

    creds = get_credentials(args.state_dir)
    drive = build("drive", "v3", credentials=creds, cache_discovery=False)
    script = build("script", "v1", credentials=creds, cache_discovery=False)

    state = load_state(state_file)

    if args.reset and (file_key in state or script_key in state):
        print("dropping cached file + script...", flush=True)
        delete_quietly(drive, state.pop(file_key, ""))
        delete_quietly(drive, state.pop(script_key, ""))
        save_state(state_file, state)

    doc_id = state.get(file_key)
    script_id = state.get(script_key)

    if doc_id is None or script_id is None:
        if args.fixture is None:
            sys.exit(
                "first-run bootstrap needs a fixture at "
                + str(fixture_candidate)
                + " to create the file; run with a stem that has one first"
                " (e.g. document)"
            )
        print(
            "first-run bootstrap: creating "
            + ("presentation" if is_presentation else "Doc")
            + " and bound script...",
            flush=True,
        )
        doc_id = upload_fixture(drive, args.fixture)
        print("  file id: " + doc_id)
        script_id = create_bound_script(script, doc_id, args.js)
        print("  script id: " + script_id)
        save_state(state_file, dict(state, **{file_key: doc_id, script_key: script_id}))
        print()
        print(
            ("Presentation: https://docs.google.com/presentation/d/" if is_presentation
             else "Doc:          https://docs.google.com/document/d/")
            + doc_id + "/edit"
        )
        print("Script:       https://script.google.com/d/" + script_id + "/edit")
        print()
        print(
            "Now pair the script with GCP project number "
            + gcp_project_number(args.state_dir)
            + " once (see step 2 in the header of this script), then rerun."
        )
        return 2

    try:
        if args.fixture is not None:
            print(
                "refreshing "
                + ("presentation" if is_presentation else "Doc")
                + " content from " + args.fixture.name + "...",
                flush=True,
            )
            replace_file_content(drive, doc_id, args.fixture)
        print("pushing " + args.js.name + " to the script...", flush=True)
        push_script_content(script, script_id, args.js)
    except HttpError as e:
        if e.resp.status != 404:
            raise
        print(
            "cached Doc, presentation or script is gone from Drive.  Run with"
            " --reset to rebootstrap."
        )
        return 1

    print("running " + args.function + "()...", flush=True)
    ok, detail = run_function(script, script_id, args.function)
    if ok:
        print("PASS")
        if detail is not None:
            print("  result: " + str(detail))
        return 0
    print("FAIL")
    print(detail)
    return 1


if __name__ == "__main__":
    sys.exit(main())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
