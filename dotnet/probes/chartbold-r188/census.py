#!/usr/bin/env python3
"""Which corpus chart parts state text properties on a PER-POINT `c:dLbl`.

    ./census.py [/home/user/sample-files]

A `c:dLbl` is one point's label override and sits inside the series' own `c:dLbls`; the
series-level statement is that `c:dLbls`' own `c:txPr`, outside every `c:dLbl`. A reader that
resolves only the series level draws a per-point size, weight or colour at the plot default.

Prints, per document, how many per-point labels state each of `sz`, `b` and a colour, and
whether the enclosing series-level `c:dLbls` states the same thing — because an override that
merely repeats what it inherits cannot be seen on the page.
"""
import pathlib
import re
import sys
import zipfile
from xml.etree import ElementTree

C = "{http://schemas.openxmlformats.org/drawingml/2006/chart}"
A = "{http://schemas.openxmlformats.org/drawingml/2006/main}"


def properties(element):
    """The `a:defRPr` a `c:txPr` states, or None."""
    if element is None:
        return None
    body = element.find(C + "txPr")
    if body is None:
        return None
    for paragraph in body.iter(A + "defRPr"):
        return paragraph
    return None


def describe(run):
    if run is None:
        return set()
    stated = set()
    if run.get("sz"):
        stated.add("sz")
    if run.get("b"):
        stated.add("b")
    if run.find(A + "solidFill") is not None:
        stated.add("colour")
    return stated


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "/home/user/sample-files")
    totals = {"sz": 0, "b": 0, "colour": 0}
    documents = set()
    parts = 0

    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        try:
            package = zipfile.ZipFile(path)
        except Exception:                                # noqa: BLE001 - not a zip is a datum
            continue
        with package:
            for name in package.namelist():
                if "/charts/" not in name or not name.endswith(".xml"):
                    continue
                try:
                    tree = ElementTree.fromstring(package.read(name))
                except ElementTree.ParseError:
                    continue
                parts += 1
                for group in tree.iter():
                    for labels in group.findall(C + "dLbls"):
                        inherited = describe(properties(labels))
                        for one in labels.findall(C + "dLbl"):
                            extra = describe(properties(one)) - inherited
                            for key in extra:
                                totals[key] += 1
                            if extra:
                                documents.add(path.name)

    print("chart parts scanned: %d" % parts)
    print("per-point overrides beyond the series level: %s" % totals)
    print("documents: %d" % len(documents))
    for name in sorted(documents):
        print("   ", name)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
