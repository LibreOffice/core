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

"""Fetch a Google Apps Script add-on as a COOL extension.

Usage:
    fetch-gas-addon.py <source> [--id ID] [--dest PATH] [--subpath PATH] [--use-clasp] [--force]

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

Then the Apps Script libraries that dependencies.libraries in the add-on's
appsscript.json declares are fetched in the declared version, through the Apps
Script API, and then the libraries that those libraries declare in turn.  Each
one lands in

    <dest>/<id>/_cool-gas-libraries/<libraryId>/<version>/

as one file per file of the library project (<name>.gs for code, <name>.html,
appsscript.json), plus _cool-gas.json, whose "scripts" lists the code files in
the order of the project, as GAS runs them.  (That is the same file as the
_cool-gas.json at the top of an add-on, which also lists the add-on's code files
under "scripts".)  A library declared with "developmentMode": true is fetched
in its current state, into a HEAD directory.  The Google login is the one that
run-gas-test.py uses (see gas_auth.py), and a library can only be fetched by
an account that may read it, which for a public library is anyone.

A <source> that is a directory directly in <dest> is an add-on already in place,
for which only the libraries are fetched.  A library already in place is kept
unless --force is given.
"""

import argparse
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from urllib.parse import urlparse

from googleapiclient.discovery import build

from gas_auth import add_state_dir_argument, get_credentials

MINIMAL_APPSSCRIPT_JSON = {
    "timeZone": "UTC",
    "runtimeVersion": "V8",
}


EXTENSIONS = {"SERVER_JS": ".gs", "HTML": ".html", "JSON": ".json"}


def declared_libraries(manifest):
    return (manifest.get("dependencies") or {}).get("libraries") or []


# The files are written next to the target first, so that an interrupted fetch leaves no library
# directory that looks complete:
def write_library(content, target):
    partial = target.with_name(target.name + ".partial")
    if partial.exists():
        shutil.rmtree(partial)
    scripts = []
    for f in content.get("files", []):
        name = f["name"] + EXTENSIONS[f["type"]]
        if name.startswith("/") or ".." in pathlib.PurePosixPath(name).parts:
            sys.exit("refusing library file name " + name)
        path = partial / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(f.get("source", ""), encoding="utf-8")
        if f["type"] == "SERVER_JS":
            scripts.append(name)
    (partial / "_cool-gas.json").write_text(json.dumps({"scripts": scripts}, indent=2) + "\n")
    if target.exists():
        shutil.rmtree(target)
    partial.rename(target)


def fetch_library(script, library, root, force, done):
    library_id = library["libraryId"]
    version = "HEAD" if library.get("developmentMode") else str(library["version"])
    if (library_id, version) in done:
        return
    done.add((library_id, version))
    target = root / library_id / version
    if target.exists() and not force:
        print("have  " + library.get("userSymbol", "?") + " " + library_id + " " + version)
    else:
        target.parent.mkdir(parents=True, exist_ok=True)
        request = script.projects().getContent(scriptId=library_id)
        if version != "HEAD":
            request = script.projects().getContent(scriptId=library_id, versionNumber=int(version))
        write_library(request.execute(), target)
        print("fetched " + library.get("userSymbol", "?") + " " + library_id + " " + version)
    manifest_path = target / "appsscript.json"
    if manifest_path.exists():
        for dependency in declared_libraries(json.loads(manifest_path.read_text())):
            fetch_library(script, dependency, root, force, done)


def fetch_libraries(add_on, force, state_dir):
    libraries = declared_libraries(json.loads((add_on / "appsscript.json").read_text()))
    if not libraries:
        return
    script = build("script", "v1", credentials=get_credentials(state_dir), cache_discovery=False)
    root = add_on / "_cool-gas-libraries"
    done = set()
    for library in libraries:
        fetch_library(script, library, root, force, done)


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
    ap.add_argument(
        "--force", action="store_true", help="fetch libraries again that are already in place")
    add_state_dir_argument(ap)
    args = ap.parse_args()

    in_place = Path(args.source).resolve()
    if in_place.is_dir() and in_place.parent == Path(args.dest).resolve():
        fetch_libraries(in_place, args.force, args.state_dir)
        return

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
    fetch_libraries(dest, args.force, args.state_dir)

    print(f"fetched as {ext_id} to {dest}")


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
