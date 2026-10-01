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

"""The Google login that the scripts in this directory share.

They use the same OAuth client and one cached token, so SCOPES covers what each of them needs.

One-time setup for the Google side:

1. Create a Google Cloud project (or reuse one):
       https://console.cloud.google.com/projectcreate
   Pick any name, "No organization" is fine for a personal account.
   Note the project id it assigns; use it as PROJECT_ID in the URLs
   below.  Every Cloud Console page is scoped to whichever project is
   active in the top-bar picker; appending ?project=PROJECT_ID pins
   the page to that one project explicitly, so a click here can't
   accidentally enable APIs on someone else's project.

2. In that project, turn on these three APIs (click "Enable" on each):
       https://console.cloud.google.com/apis/library/drive.googleapis.com?project=PROJECT_ID
       https://console.cloud.google.com/apis/library/docs.googleapis.com?project=PROJECT_ID
       https://console.cloud.google.com/apis/library/script.googleapis.com?project=PROJECT_ID

3. Visit https://script.google.com/home/usersettings and switch
   "Google Apps Script API" on for your account.  This is a per-user
   consent, separate from step 2's per-project enable.

4. Configure the OAuth consent screen (needed before creating a
   client):
       https://console.cloud.google.com/apis/credentials/consent?project=PROJECT_ID
   On a fresh project this lands on a "Get started" wizard; on an
   existing one, click "Edit App".  Fill in:
     - App name:  any string, this is only the label Google shows on
                  its own consent prompt (step 6).  "gas-test" is
                  fine.
     - User support email:  your own email, picked from the dropdown.
     - Audience:  select "External".  "Internal" is only offered on a
                  Google Workspace domain and does not apply here.
     - Developer contact email:  your own email again.
   Save.  Then under "Audience" (or "Test users" on older layouts)
   add your Google account as a test user.  Leave the app in
   "Testing" mode; that keeps the OAuth grant restricted to the test
   users you list, which is what we want.

5. Create the OAuth client:
       https://console.cloud.google.com/apis/credentials?project=PROJECT_ID
   "Create credentials" -> "OAuth client ID", application type
   "Desktop app", any name.  Download the JSON and save it as
   client.json in the --state-dir directory (~/.gas by default).

6. The first run of a script that logs in opens a browser for OAuth
   consent and caches the refresh token in token.json in the same
   directory.  If SCOPES below changes, delete that file to re-consent.
"""

import json
import pathlib
import sys

from google.auth.exceptions import RefreshError
from google.auth.transport.requests import Request
from google.oauth2.credentials import Credentials
from google_auth_oauthlib.flow import InstalledAppFlow

SCOPES = [
    "https://www.googleapis.com/auth/drive",
    "https://www.googleapis.com/auth/documents",
    "https://www.googleapis.com/auth/presentations",
    "https://www.googleapis.com/auth/script.projects",
    "https://www.googleapis.com/auth/script.deployments",
    "https://www.googleapis.com/auth/script.processes",
    # The test scripts' own UrlFetchApp calls; the Apps Script API only runs a function whose
    # scopes the caller's token covers:
    "https://www.googleapis.com/auth/script.external_request",
]


def add_state_dir_argument(parser):
    parser.add_argument(
        "--state-dir",
        type=pathlib.Path,
        default=pathlib.Path.home() / ".gas",
        help=(
            "the directory that holds the OAuth client in client.json, the cached login in"
            " token.json, and any other state of the scripts (default: ~/.gas)"
        ),
    )


def get_credentials(state_dir):
    client_secret = state_dir / "client.json"
    token_cache = state_dir / "token.json"
    creds = None
    if token_cache.exists():
        # A token granted before SCOPES grew lacks the new scopes, so ask for consent again
        # (from_authorized_user_file takes the scopes from its argument, not from the file):
        granted = json.loads(token_cache.read_text()).get("scopes") or []
        if set(SCOPES) <= set(granted):
            creds = Credentials.from_authorized_user_file(str(token_cache), SCOPES)
    if creds and creds.valid:
        return creds
    if creds and creds.expired and creds.refresh_token:
        try:
            creds.refresh(Request())
        except RefreshError as e:
            # A refresh token that Google has expired (unused for ~7 days on an unverified
            # app) or revoked (a re-consent, or the user removed the app in their Google
            # account) surfaces here as invalid_grant; fall through to the consent flow
            # below rather than fail:
            if "invalid_grant" not in str(e):
                raise
            token_cache.unlink()
            creds = None
        else:
            token_cache.write_text(creds.to_json())
            return creds
    if not client_secret.exists():
        sys.exit(
            "no OAuth client at "
            + str(client_secret)
            + ".  See the top of gas_auth.py for how to create one."
        )
    flow = InstalledAppFlow.from_client_secrets_file(str(client_secret), SCOPES)
    creds = flow.run_local_server(port=0)
    token_cache.write_text(creds.to_json())
    return creds

# vim: set shiftwidth=4 softtabstop=4 expandtab:
