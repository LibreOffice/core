#!/usr/bin/env python3
"""Every corpus document holding a DrawingML or chartex chart part, and which plots it states.

    ./census-charts.py [/home/user/sample-files] > chartdocs.tsv

Emits `<sorted,comma,separated plot elements>\t<path>`, which is the input `sweep.sh` reads.

Two things this deliberately does:

* it opens the package rather than grepping the bytes, because a `.xlsx` is a zip and
  `c:barChart` is not in it uncompressed;
* it reports the *set* of plot elements, so a combination chart is its own group. Counting a
  two-plot page under both of its types would attribute the same ink twice.

It does **not** see a `.xls` chart, which lives in a BIFF substream rather than a zip part --
that census is `probes/chartbiff-r186/census.py`, and the omission is why an earlier
chart-reach figure was short by eight.
"""
import pathlib
import re
import sys
import zipfile

PLOT = re.compile(
    rb"<c:(bar3DChart|barChart|line3DChart|lineChart|stockChart|radarChart|scatterChart"
    rb"|pie3DChart|pieChart|doughnutChart|ofPieChart|surface3DChart|surfaceChart"
    rb"|area3DChart|areaChart|bubbleChart)\b"
)
LAYOUT = re.compile(rb'<cx:series\b[^>]*layoutId="([A-Za-z0-9]+)"')


def plots(path):
    found = set()
    try:
        package = zipfile.ZipFile(path)
    except Exception:                                    # noqa: BLE001 - not a zip is a datum
        return found
    with package:
        for name in package.namelist():
            if "/charts/" not in name or not name.endswith(".xml"):
                continue
            try:
                data = package.read(name)
            except Exception:                            # noqa: BLE001
                continue
            found.update(m.decode() for m in PLOT.findall(data))
            found.update("cx:" + m.decode() for m in LAYOUT.findall(data))
    return found


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "/home/user/sample-files")
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        found = plots(path)
        if found:
            print("%s\t%s" % (",".join(sorted(found)), path))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
