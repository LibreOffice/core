#!/usr/bin/env python3
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
# The SBOM is the inventory: name, version, homepage and purl come from it, so
# this covers whatever the SBOM covers, engine and online alike. Version
# comparison is deliberately conservative - it only reports a component when
# the upstream version is unambiguously newer under a numeric comparison, and
# says so when it cannot decide.
#
# BEHIND means a newer release in our own series - the same version but for
# its last part, as openssl 3.5.9 is for 3.5.8 - which is the update that
# should be taken. A newer series (openssl 4.1) is reported as NEWER-SERIES,
# for information only: staying on a maintained branch is a decision, not an
# oversight, and the scan reports the CVEs a version is affected by in any
# case. Exit code 1 if anything is BEHIND.

import argparse
import json
import re
import sys
import time
import urllib.parse
import urllib.request

API = "https://release-monitoring.org/api/v2/projects/?name="


def numeric(version):
    """Version as a comparable tuple, or None if it is not purely numeric."""
    if not version or not re.fullmatch(r"\d+(\.\d+)*", version):
        return None
    return tuple(int(part) for part in version.split("."))


def collect(paths):
    """name -> (version, homepage, purl) of every versioned third-party
    component.

    Our own packages - the document roots, the online branding - carry no
    identifiers and are not upstream releases. Dictionaries are versioned
    from their own description.xml, which no release tracker knows, so the
    lookups would only add noise."""
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
            identifiers = (element.get("externalIdentifier")
                           or element.get("externalIdentifiers") or [])
            if not identifiers:
                continue
            if name.startswith("dict-"):
                continue
            purl = next((i.get("identifier") for i in identifiers
                         if i.get("externalIdentifierType")
                         in ("packageUrl", "packageURL")), None)
            found.setdefault(name, (version, element.get("software_homePage"),
                                    purl))
    return found


def lookup(name):
    """Anitya's projects of exactly this name; raises if the lookup fails."""
    url = API + urllib.parse.quote(name)
    # a full sweep is a burst of requests, and the server resets a
    # connection now and then; retry before giving up
    for attempt in range(3):
        try:
            with urllib.request.urlopen(url, timeout=30) as response:
                items = json.load(response).get("items", [])
            break
        except Exception:
            if attempt == 2:
                raise
            time.sleep(5 * (attempt + 1))
    return [i for i in items if i.get("name") == name and i.get("version")]


def site(url):
    """Host and path of a URL, for comparing homepages."""
    parsed = urllib.parse.urlparse(url or "")
    host = parsed.netloc.lower()
    if host.startswith("www."):
        host = host[len("www."):]
    return host, parsed.path.rstrip("/").lower()


def pick(items, homepage):
    """The candidate whose homepage is ours: the same page if one is (two
    projects on github.com differ only in the path), else the same host."""
    if not homepage:
        return None
    host, path = site(homepage)
    if not host:
        return None
    same_page = [i for i in items if site(i.get("homepage")) == (host, path)]
    if len(same_page) == 1:
        return same_page[0]
    same_host = [i for i in items
                 if host in (i.get("homepage") or "") + (i.get("ecosystem") or "")]
    if len(same_host) == 1:
        return same_host[0]
    return None


def purl_names(purl):
    """Project names a repository purl suggests, for when Anitya knows the
    project by another name: pkg:git/github.com/quickjs-ng/quickjs gives
    quickjs-ng and quickjs."""
    match = re.match(r"pkg:(git|github)/([^?@#]+)", purl or "")
    if not match:
        return []
    parts = [part for part in match.group(2).split("/") if part]
    if match.group(1) == "git":
        parts = parts[1:]  # the host
    names = []
    for part in parts[-2:]:
        part = re.sub(r"\.git$", "", part)
        if part not in names:
            names.append(part)
    return names


def upstream(name, homepage, purl):
    """Anitya's project for a component, or None and the reason."""
    try:
        items = lookup(name)
        chosen = pick(items, homepage)
        if chosen is None and len(items) != 1:
            # untracked or ambiguous under our name: Anitya may know the
            # project by its repository's name (quickjs-ng, zxing-cpp)
            for other in purl_names(purl):
                if other == name:
                    continue
                chosen = pick(lookup(other), homepage)
                if chosen is not None:
                    break
    except Exception as error:
        return None, f"lookup failed: {error}"
    if chosen is None and len(items) == 1:
        chosen = items[0]
    if chosen is None:
        if not items:
            return None, "not tracked by Anitya"
        versions = ", ".join(f"{i.get('version')} ({i.get('ecosystem')})"
                             for i in items[:4])
        return None, f"ambiguous project name, candidates: {versions}"
    return chosen, None


def compare(version, project):
    """(newest release in our series, newest release), as version strings,
    from Anitya's stable versions - never a pre-release; None where there is
    no such newer release. Raises ValueError if the versions do not compare."""
    ours = numeric(version)
    stable = [v for v in project.get("stable_versions") or [] if numeric(v)]
    if not stable and numeric(project.get("version")):
        stable = [project["version"]]
    if not ours or not stable:
        raise ValueError(f"upstream says {project.get('version')}, "
                         f"cannot compare")
    newest = max(stable, key=numeric)
    series = ours[:-1]
    in_series = [v for v in stable if numeric(v)[:len(series)] == series]
    newest_in_series = max(in_series, key=numeric) if in_series else None
    behind = (newest_in_series if newest_in_series
              and numeric(newest_in_series) > ours else None)
    newer = newest if numeric(newest) > ours else None
    return behind, newer


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

    behind, newer_series, unknown = [], [], []
    for name, (version, homepage, purl) in sorted(components.items()):
        project, problem = upstream(name, homepage, purl)
        if project is None:
            unknown.append((name, version, problem))
            continue
        try:
            in_series, newest = compare(version, project)
        except ValueError as error:
            unknown.append((name, version, str(error)))
            continue
        if in_series:
            behind.append((name, version, in_series, newest))
        elif newest:
            newer_series.append((name, version, newest))

    for name, version, in_series, newest in behind:
        also = (f" (newest series {newest})"
                if newest and newest != in_series else "")
        print(f"BEHIND    {name}: we bundle {version}, {in_series} is out{also}")
    for name, version, newest in newer_series:
        print(f"NEWER-SERIES {name}: we bundle {version}, upstream is {newest}")
    for name, version, problem in unknown:
        print(f"UNCHECKED {name} ({version}): {problem}")
    print(f"\n{len(components)} components, {len(behind)} behind, "
          f"{len(newer_series)} with a newer series, {len(unknown)} unchecked")
    sys.exit(1 if behind else 0)


if __name__ == "__main__":
    main()
