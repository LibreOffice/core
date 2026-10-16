#!/usr/bin/env python3
"""Corpus chart parts that put a bar group on a date axis.

    ./census.py [chartdocs.tsv]

A `c:dateAx` places every point at its own date, so a series whose categories are not evenly
spaced -- or whose axis states a `c:min`/`c:max` narrower than the data -- is drawn somewhere
a category-index layout cannot put it. A line group already goes through the date scale here;
a bar group does not, which is what this counts.

Prints one row per document: which plot groups share the axis, how many points the series
cache holds, and what the axis states for its own range.
"""
import pathlib
import re
import sys
import zipfile


def main() -> int:
    docs = sys.argv[1] if len(sys.argv) > 1 else \
        str(pathlib.Path(__file__).parent.parent / "chartsweep-r186" / "chartdocs.tsv")

    found = 0
    for line in open(docs, encoding="utf-8"):
        path = line.rstrip("\n").split("\t")[1]
        try:
            package = zipfile.ZipFile(path)
        except Exception:                                # noqa: BLE001 - not a zip is a datum
            continue
        with package:
            for name in package.namelist():
                if "/charts/" not in name or not name.endswith(".xml"):
                    continue
                body = package.read(name).decode("utf-8", "replace")
                if "<c:dateAx>" not in body:
                    continue
                groups = re.findall(r"<c:(\w+Chart)>", body)
                if not any(g.startswith("bar") for g in groups):
                    continue
                found += 1
                counts = sorted({int(n) for n in re.findall(r'<c:ptCount val="(\d+)"/>', body)})
                low = re.search(r'<c:min val="([^"]*)"', body)
                high = re.search(r'<c:max val="([^"]*)"', body)
                print("%-58s %-28s pts %s  min %s max %s"
                      % (pathlib.Path(path).name[:56], ",".join(groups),
                         counts, low.group(1) if low else "-", high.group(1) if high else "-"))

    print("chart parts with a bar group on a date axis: %d" % found)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
