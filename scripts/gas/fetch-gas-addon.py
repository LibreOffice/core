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
# ///

"""Fetch a Google Apps Script add-on as a COOL extension.

Usage:
    fetch-gas-addon.py <source> [--id ID] [--dest PATH] [--subpath PATH] [--use-clasp]

<source> is either

- a local directory path holding the add-on files, or
- an https://github.com/OWNER/REPO[/tree/BRANCH[/SUB/PATH]] URL, or
- any other URL that git clone accepts (in which case --id is required).

For a GitHub URL the extension id is derived as
    com.github.<owner>.<repo>[.<subpath with slashes as dots>]
all lowercased.  For a local directory whose basename already starts with
"com." that basename is taken as the id; otherwise --id is required.

The extension lands at <dest>/<id>/.  <dest> defaults to
$PWD/browser/extensions (i.e. the current directory is assumed to be an online
checkout), from where "make -C browser install-demo-extensions" installs it.  An
existing directory at <dest>/<id>/ is a hard error -- remove it explicitly to
fetch the add-on again, so a half-updated tree cannot be mistaken for a fresh
fetch.

If the add-on carries no appsscript.json, a minimal
    {"timeZone": "UTC", "runtimeVersion": "V8"}
is written in its place.  --use-clasp shells out to `clasp create` instead
(assumes clasp is on PATH and already authenticated); rarely needed since the
fabricated file is what clasp writes for a fresh standalone project anyway.
"""

import argparse
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from urllib.parse import urlparse

MINIMAL_APPSSCRIPT_JSON = {
    "timeZone": "UTC",
    "runtimeVersion": "V8",
}


def parse_github_url(url):
    """Return (owner, repo, subpath) for a github.com URL, else None.

    Accepts:
        https://github.com/OWNER/REPO
        https://github.com/OWNER/REPO.git
        https://github.com/OWNER/REPO/tree/BRANCH
        https://github.com/OWNER/REPO/tree/BRANCH/SUB/PATH
    """
    parsed = urlparse(url)
    if parsed.hostname != "github.com":
        return None
    parts = [seg for seg in parsed.path.split("/") if seg]
    if len(parts) < 2:
        return None
    owner, repo = parts[0], parts[1]
    if repo.endswith(".git"):
        repo = repo[:-4]
    subpath = ""
    if len(parts) >= 4 and parts[2] == "tree":
        subpath = "/".join(parts[4:])
    return owner, repo, subpath


def derive_id_from_github(owner, repo, subpath):
    parts = [owner, repo]
    if subpath:
        parts.extend(seg for seg in subpath.split("/") if seg)
    return "com.github." + ".".join(seg.lower() for seg in parts)


def fetch(source, subpath, workdir):
    """Return the path holding the add-on files as the extension holds them."""
    if Path(source).is_dir():
        base = Path(source).resolve()
    else:
        clone_dir = workdir / "src"
        subprocess.run(
            ["git", "clone", "--depth", "1", source, str(clone_dir)], check=True,
        )
        base = clone_dir
    if subpath:
        base = base / subpath
        if not base.is_dir():
            sys.exit(f"subpath {subpath!r} is not a directory in the fetched source")
    return locate_addon_root(base)


def locate_addon_root(base):
    """Return the directory that actually holds the add-on files.

    Many GitHub add-on repositories keep the .gs and sidebar.html files in a
    subdirectory (commonly named "addon" or "src") next to a README, a LICENSE,
    and other content that must not be shipped as part of the extension.  The
    canonical marker is the appsscript.json; take the directory holding it as
    the add-on root.  If none exists, use the base as-is and rely on the
    fabricated minimal appsscript.json.
    """
    if (base / "appsscript.json").is_file():
        return base
    candidates = sorted(base.glob("*/appsscript.json"))
    if len(candidates) == 1:
        return candidates[0].parent
    if len(candidates) > 1:
        rels = ", ".join(str(c.parent.relative_to(base)) for c in candidates)
        sys.exit(
            f"multiple appsscript.json candidates under {base}: {rels}; "
            "pass --subpath to pick one",
        )
    return base


def copy_into_dest(src, dest):
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(src, dest)
    # Strip .git if the source was a clone; git metadata does not belong in the
    # served extension:
    dot_git = dest / ".git"
    if dot_git.exists():
        shutil.rmtree(dot_git)


def ensure_appsscript_json(dest, use_clasp):
    path = dest / "appsscript.json"
    if path.exists():
        return
    if use_clasp:
        subprocess.run(
            ["clasp", "create", "--type", "standalone", "--title", dest.name],
            cwd=dest, check=True,
        )
        if not path.exists():
            sys.exit("clasp did not produce an appsscript.json")
    else:
        path.write_text(json.dumps(MINIMAL_APPSSCRIPT_JSON, indent=2) + "\n")


def main():
    ap = argparse.ArgumentParser(
        description="Fetch a Google Apps Script add-on as a COOL extension",
    )
    ap.add_argument("source", help="local directory or GitHub / HTTPS URL")
    ap.add_argument("--id", help="extension id (default: derive from URL)")
    ap.add_argument(
        "--dest", default="browser/extensions",
        help="destination extensions dir (default: ./browser/extensions)",
    )
    ap.add_argument("--subpath", help="subdirectory inside the source to fetch")
    ap.add_argument(
        "--use-clasp", action="store_true",
        help="use `clasp create` to generate a missing appsscript.json",
    )
    args = ap.parse_args()

    subpath = args.subpath or ""
    ext_id = args.id
    source = args.source

    parsed = parse_github_url(source)
    if parsed is not None:
        owner, repo, url_subpath = parsed
        if url_subpath:
            if subpath and subpath != url_subpath:
                sys.exit(
                    "--subpath and the URL both carry a subpath, and they differ",
                )
            subpath = url_subpath
        # Strip the /tree/BRANCH/... suffix so git clone gets a plain repo URL:
        source = f"https://github.com/{owner}/{repo}"
        if ext_id is None:
            ext_id = derive_id_from_github(owner, repo, subpath)
    elif ext_id is None:
        base = Path(args.source.rstrip("/")).name
        if base.startswith("com."):
            ext_id = base.lower()
        else:
            sys.exit(
                "--id is required for non-github sources unless the source "
                "directory basename already starts with 'com.'",
            )

    dest_root = Path(args.dest).resolve()
    dest = dest_root / ext_id
    if dest.exists():
        sys.exit(
            f"destination {dest} already exists; remove it first if you meant to "
            "fetch the add-on again",
        )

    with tempfile.TemporaryDirectory(prefix="fetch-gas-addon-") as tmp:
        work = Path(tmp)
        add_on = fetch(source, subpath, work)
        copy_into_dest(add_on, dest)
        ensure_appsscript_json(dest, args.use_clasp)

    print(f"fetched as {ext_id} to {dest}")


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
