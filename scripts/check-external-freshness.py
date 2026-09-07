#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Report bundled third-party components that are behind their latest upstream
# release, by asking release-monitoring.org (Anitya) what upstream currently
# ships.
#
# A vulnerability scan does not answer this question: it reports the CVEs a
# version is known to be affected by, so a component can be several releases
# behind and still scan clean - which is exactly what happened with expat
# 2.8.2 (no CVEs, two releases old). Being behind is not a vulnerability, but
# it is how one becomes one, and it is the lead time for a bump that turns
# out to need work (a build system migration, patch rebases).
#
# Usage:
#   scripts/check-external-freshness.py [--component NAME]... <sbom.spdx.json>...
#
# The SBOM is the inventory: name, version and homepage come from it, so this
# covers whatever the SBOM covers, engine and online alike. Version
# comparison is deliberately conservative - it only reports a component when
# the upstream version is unambiguously newer under a numeric comparison, and
# says so when it cannot decide.

import argparse
import json
import re
import sys
import urllib.parse
import urllib.request

API = "https://release-monitoring.org/api/v2/projects/?name="


def numeric(version):
    """Version as a comparable tuple, or None if it is not purely numeric."""
    if not version or not re.fullmatch(r"\d+(\.\d+)*", version):
        return None
    return tuple(int(part) for part in version.split("."))


def collect(paths):
    """(name, version, homepage) of every versioned SBOM component."""
    found = {}
    for path in paths:
        with open(path, encoding="utf-8") as f:
            document = json.load(f)
        for element in document.get("@graph", []):
            if element.get("type") != "software_Package":
                continue
            name = element.get("name")
            version = element.get("software_packageVersion")
            if not name or not version:
                continue
            found.setdefault(name, (version, element.get("software_homePage")))
    return found


def upstream(name, homepage):
    """Latest upstream version per Anitya, and whether the pick was clear."""
    url = API + urllib.parse.quote(name)
    try:
        with urllib.request.urlopen(url, timeout=30) as response:
            items = json.load(response).get("items", [])
    except Exception as error:
        return None, f"lookup failed: {error}"
    items = [i for i in items if i.get("name") == name and i.get("version")]
    if not items:
        return None, "not tracked by Anitya"
    if len(items) > 1 and homepage:
        host = urllib.parse.urlparse(homepage).netloc.removeprefix("www.")
        preferred = [i for i in items
                     if host and host in (i.get("homepage") or "") + (i.get("ecosystem") or "")]
        if preferred:
            items = preferred
    if len(items) > 1:
        versions = ", ".join(f"{i.get('version')} ({i.get('ecosystem')})"
                             for i in items[:4])
        return None, f"ambiguous project name, candidates: {versions}"
    return items[0]["version"], None


def main():
    parser = argparse.ArgumentParser(
        description="Report bundled components behind their latest upstream release")
    parser.add_argument("sboms", nargs="+")
    parser.add_argument("--component", action="append", default=[],
                        help="only check these components (default: all)")
    args = parser.parse_args()

    components = collect(args.sboms)
    if args.component:
        components = {k: v for k, v in components.items()
                      if k in args.component}

    behind, unknown = [], []
    for name, (version, homepage) in sorted(components.items()):
        latest, problem = upstream(name, homepage)
        if latest is None:
            unknown.append((name, version, problem))
            continue
        ours, theirs = numeric(version), numeric(latest)
        if ours and theirs and theirs > ours:
            behind.append((name, version, latest))
        elif not (ours and theirs) and version != latest:
            unknown.append((name, version,
                            f"upstream says {latest}, cannot compare"))

    for name, version, latest in behind:
        print(f"BEHIND    {name}: we bundle {version}, upstream is {latest}")
    for name, version, problem in unknown:
        print(f"UNCHECKED {name} ({version}): {problem}")
    print(f"\n{len(components)} components, {len(behind)} behind, "
          f"{len(unknown)} unchecked")
    sys.exit(1 if behind else 0)


if __name__ == "__main__":
    main()
