#!/usr/bin/env python3
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
"""Drop optional packages from the shrinkwrap and the shrinkpack tarballs.

    prune_shrinkwrap.py LOCKFILE TARBALL_DIR PACKAGE...

A PACKAGE is removed wherever it is nested, together with the packages
that only it depended on, and their tarballs are deleted. The entries
that listed it as an optional dependency stop doing so, since npm 11
resolves optional dependencies of the lockfile's entries and refuses a
lockfile that lacks them; a package that something requires cannot be
pruned. The update-shrinkpack target uses it for the platform binaries
of sass's file watcher, which only sass --watch needs.
"""

import json
import os
import sys

DEPENDENCY_KINDS = ("dependencies", "optionalDependencies", "peerDependencies")


def name_of(path):
    return path.rsplit("node_modules/", 1)[-1]


def main():
    if len(sys.argv) < 4:
        print(__doc__, file=sys.stderr)
        return 1
    lockfile, tarball_dir, targets = sys.argv[1], sys.argv[2], set(sys.argv[3:])
    with open(lockfile) as f:
        lock = json.load(f)
    packages = lock["packages"]
    root = packages[""]
    root_deps = set()
    for kind in ("dependencies", "devDependencies", "optionalDependencies"):
        root_deps |= set(root.get(kind, {}))

    for path, entry in packages.items():
        for kind in ("dependencies", "peerDependencies"):
            required = targets & set(entry.get(kind, {}))
            if required:
                print(f"{path or 'the root'} requires {', '.join(sorted(required))}",
                      file=sys.stderr)
                return 1

    removed = {}
    pruned_names = set(targets)
    while targets:
        drop = {path for path in packages if path and name_of(path) in targets}
        drop |= {
            path
            for path in packages
            if any(path.startswith(d + "/node_modules/") for d in drop)
        }
        if not drop:
            break
        for path in drop:
            removed[path] = packages.pop(path).get("version")
            pruned_names.add(name_of(path))
        used = set()
        for entry in packages.values():
            for kind in DEPENDENCY_KINDS:
                used |= set(entry.get(kind, {}))
        targets = {
            name_of(path)
            for path in packages
            if path and name_of(path) not in used and name_of(path) not in root_deps
        }

    for path, entry in packages.items():
        optional = entry.get("optionalDependencies", {})
        for name in sorted(pruned_names & set(optional)):
            del optional[name]
            print(f"unreferenced {name} in {path}")
        if "optionalDependencies" in entry and not optional:
            del entry["optionalDependencies"]

    with open(lockfile, "w") as f:
        json.dump(lock, f, indent=2)
        f.write("\n")

    kept = {(name_of(path), entry.get("version")) for path, entry in packages.items()}
    for path in sorted(removed):
        name, version = name_of(path), removed[path]
        print("removed", path)
        if (name, version) in kept:
            continue
        tarball = os.path.join(tarball_dir, f"{name.replace('/', '_')}-{version}.tar")
        if os.path.exists(tarball):
            os.remove(tarball)
            print("deleted", tarball)
    return 0


if __name__ == "__main__":
    sys.exit(main())
