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
# Turns the log of CppunitTest_sd_roundtrip_diff into a ranked list of what the
# filter loses.  A property is counted once per document, so that a document
# with a thousand shapes does not outweigh all the others.

import argparse
import collections
import sys


# A difference in one of these means the objects on the two sides are no longer
# the same objects, so every property below that point compares unrelated things.
STRUCTURAL = ("ShapeCount", "ShapeType")


def read(path):
    """Reads the log into the per document results it holds."""
    results = {}
    raw = {"D1": [], "D2": []}
    for line in open(path, errors="replace"):
        parts = line.rstrip("\n").split("\t")
        if parts[0] == "FILE" and len(parts) >= 3:
            results[parts[1]] = parts[2]
        elif parts[0] in ("D1", "D2") and len(parts) >= 6:
            raw[parts[0]].append(parts[1:6])

    diffs = {}
    structural = {}
    for stage, entries in raw.items():
        # Where a shape count or a shape type changed, every shape after it on that
        # page has moved up or down the order, so each index now names a different
        # object and every property below reads as changed. One such difference
        # spoils the whole page, so the page leaves the ranking and the document
        # counts as structurally changed instead.
        broken = collections.defaultdict(set)
        for document, kind, key, before, after in entries:
            path_part, prop = key.rsplit("@", 1)
            if prop in STRUCTURAL:
                broken[document].add(path_part.split("/")[0])
        structural[stage] = set(broken)

        counts = collections.defaultdict(dict)
        for document, kind, key, before, after in entries:
            path_part, prop = key.rsplit("@", 1)
            if path_part.split("/")[0] in broken.get(document, ()):
                continue
            # One example per property and document is enough to act on.
            counts[prop].setdefault(document, (kind, key, before, after))
        diffs[stage] = counts
    return results, diffs, structural


def table(title, counts, total, limit, examples):
    print()
    print(title)
    print("-" * len(title))
    if not counts:
        print("  nothing")
        return
    for prop, documents in sorted(counts.items(), key=lambda item: -len(item[1]))[:limit]:
        share = 100.0 * len(documents) / total if total else 0.0
        kinds = collections.Counter(entry[0] for entry in documents.values())
        print("%5d docs (%4.1f%%)  %-34s %s"
              % (len(documents), share, prop,
                 " ".join("%s=%d" % (kind.lower(), count) for kind, count in kinds.most_common())))
        if examples:
            document, (kind, key, before, after) = sorted(documents.items())[0]
            print("        %s" % document)
            print("        %s" % key)
            print("        %s  ->  %s" % (before[:90], after[:90]))


def main():
    parser = argparse.ArgumentParser(description="rank what the pptx round trip loses")
    parser.add_argument("log", help="report written by CppunitTest_sd_roundtrip_diff")
    parser.add_argument("--limit", type=int, default=30, help="how many properties to list")
    parser.add_argument("--examples", action="store_true", help="show one example per property")
    arguments = parser.parse_args()

    results, diffs, structural = read(arguments.log)
    if not results:
        print("no documents in %s" % arguments.log, file=sys.stderr)
        return 1

    failed = [name for name, status in results.items() if status != "OK"]
    good = len(results) - len(failed)
    lossy = set()
    for documents in diffs["D1"].values():
        lossy.update(documents)
    unstable = set()
    for documents in diffs["D2"].values():
        unstable.update(documents)

    print("documents         %d" % len(results))
    print("  processed       %d" % good)
    print("  failed to load  %d" % len(failed))
    print("  changed by one round trip          %d" % len(lossy))
    print("  changed again by a second one      %d" % len(unstable))
    print("  shapes added, dropped or retyped   %d" % len(structural["D1"]))
    print("  the same on the second round trip  %d" % len(structural["D2"]))

    table("Lost or changed by the round trip, by document count",
          diffs["D1"], good, arguments.limit, arguments.examples)
    table("Still changing on the second round trip, by document count",
          diffs["D2"], good, arguments.limit, arguments.examples)

    if failed:
        print()
        print("Documents that did not get through:")
        for name in sorted(failed)[:40]:
            print("  %s" % name)
    return 0


if __name__ == "__main__":
    sys.exit(main())
