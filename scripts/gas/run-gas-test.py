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
one that has a single blank slide at the start of every run.

For a stem that has an .rtf fixture, uploads it to Drive (asking Drive
to convert it to a Google Doc on the way in), creates a bound Apps
Script project holding the .js file, executes test() server-side
through the Apps Script API, and prints PASS or FAIL with the
exception text.  For a stem without an .rtf fixture, reuses the cached
Doc as-is (its content is not observed by the test).

The Doc and the script are created on first run and cached in
test-state.json in the --state-dir directory (~/.gas by default),
so every subsequent run just overwrites the Doc's content (via a fresh
RTF upload that Drive re-converts in place) and pushes the current
<stem>-test.js source, then invokes test.  The one-time GCP
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
        project" and enter your project NUMBER (not id -- find it at
        the top of  https://console.cloud.google.com/home/dashboard?project=PROJECT_ID  ).
     4. "Set project".  This pairing sticks with the script, so this
        step is one-time-per-cache-lifetime rather than per-run.

Usage:
    scripts/gas/run-gas-test.py document
    scripts/gas/run-gas-test.py utilities
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


def upload_rtf(drive, rtf_path):
    body = {
        "name": "gas-test " + rtf_path.name,
        "mimeType": "application/vnd.google-apps.document",
    }
    media = MediaFileUpload(
        str(rtf_path), mimetype="application/rtf", resumable=False
    )
    result = drive.files().create(
        body=body, media_body=media, fields="id"
    ).execute()
    return result["id"]


PPTX_TYPE = "application/vnd.openxmlformats-officedocument.presentationml.presentation"


def create_presentation(drive, snapshot_path):
    """Create a blank presentation, and save its PPTX export at snapshot_path for
    replace_presentation_content."""
    result = drive.files().create(
        body={
            "name": "gas-test presentation",
            "mimeType": "application/vnd.google-apps.presentation",
        },
        fields="id",
    ).execute()
    snapshot_path.write_bytes(
        drive.files().export(fileId=result["id"], mimeType=PPTX_TYPE).execute()
    )
    return result["id"]


def replace_presentation_content(drive, presentation_id, snapshot_path):
    media = MediaFileUpload(str(snapshot_path), mimetype=PPTX_TYPE, resumable=False)
    drive.files().update(fileId=presentation_id, media_body=media).execute()


def replace_doc_content(drive, doc_id, rtf_path):
    """Replace the target Docs file's content with a fresh RTF upload; Drive
    re-runs its RTF-to-Docs conversion on the media, so the resulting Doc has
    the same content it would have if we had uploaded the RTF as a new file."""
    media = MediaFileUpload(
        str(rtf_path), mimetype="application/rtf", resumable=False
    )
    drive.files().update(fileId=doc_id, media_body=media).execute()


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
            " Doc built from an RTF fixture when the stem has one."
        )
    )
    ap.add_argument(
        "stem",
        help=(
            "test stem: run <stem>-test.js, invoke test, refresh"
            " the Doc from <stem>-test.rtf when that fixture exists"
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
    rtf_candidate = DATA_DIR / (args.stem + "-test.rtf")
    args.rtf = rtf_candidate if rtf_candidate.exists() else None
    is_presentation = args.stem.startswith("slides")
    if is_presentation and args.rtf is not None:
        sys.exit("a presentation test cannot have a fixture: " + str(args.rtf))
    file_key, script_key = (
        ("presentation_id", "presentation_script_id") if is_presentation
        else ("doc_id", "script_id")
    )
    snapshot_path = args.state_dir / "test-presentation.pptx"

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
        if is_presentation:
            print(
                "first-run bootstrap: creating presentation and bound script...",
                flush=True,
            )
            doc_id = create_presentation(drive, snapshot_path)
        else:
            if args.rtf is None:
                sys.exit(
                    "first-run bootstrap needs an .rtf fixture at "
                    + str(rtf_candidate)
                    + " to create the Doc; run with a stem that has one first"
                    " (e.g. document)"
                )
            print("first-run bootstrap: creating Doc and bound script...", flush=True)
            doc_id = upload_rtf(drive, args.rtf)
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
            "Now pair the script's GCP project once (see step 2 in the"
            " header of this script), then rerun."
        )
        return 2

    try:
        if is_presentation:
            print("resetting the presentation to one blank slide...", flush=True)
            replace_presentation_content(drive, doc_id, snapshot_path)
        if args.rtf is not None:
            print("refreshing Doc content from " + args.rtf.name + "...", flush=True)
            replace_doc_content(drive, doc_id, args.rtf)
        print("pushing " + args.js.name + " to the script...", flush=True)
        push_script_content(script, script_id, args.js)
    except HttpError as e:
        if e.resp.status != 404:
            raise
        print(
            "cached Doc or script is gone from Drive.  Run with --reset to"
            " rebootstrap."
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
