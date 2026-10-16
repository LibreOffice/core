#!/usr/bin/env python3
"""Chart parts whose legend the reference lists in reverse, and we do not.

`VSeriesPlotter::createLegendEntries` (chart2/source/view/charttypes/VSeriesPlotter.cxx:2432-2447):
with the coordinate system swapped -- a horizontal bar chart -- the entries reverse unless the
series stack in Y; otherwise, and only for a legend at the line start or line end, they reverse
when the series *do* stack in Y.  A top or bottom legend on an unswapped chart never reverses.
"""
import os, re, sys, zipfile
import xml.etree.ElementTree as ET

C = "{http://schemas.openxmlformats.org/drawingml/2006/chart}"


def child(el, n):
    return None if el is None else el.find(C + n)


def val(el, n):
    c = child(el, n)
    return None if c is None else c.get("val")


if __name__ == "__main__":
    seen = set()
    print("doc\tpart\tlegendPos\tbarDir\tgrouping\tseries\treverse")
    docs = set()
    for dirpath, _, names in os.walk(sys.argv[1]):
        for n in sorted(names):
            if not n.lower().endswith((".pptx", ".pptm", ".potx", ".ppsx", ".ppsm",
                                       ".xlsx", ".xlsm", ".xltx", ".xltm",
                                       ".docx", ".docm", ".dotx")):
                continue
            if n.lower() in seen:
                continue
            seen.add(n.lower())
            try:
                z = zipfile.ZipFile(os.path.join(dirpath, n))
            except Exception:
                continue
            for m in sorted(z.namelist()):
                if not re.match(r"(ppt|word|xl)/charts/chart\d+\.xml$", m):
                    continue
                try:
                    root = ET.fromstring(z.read(m))
                except Exception:
                    continue
                chart = child(root, "chart")
                legend = child(chart, "legend")
                if legend is None:
                    continue
                pos = val(legend, "legendPos") or "r"
                plot = child(chart, "plotArea")
                if plot is None:
                    continue
                groups = [e for e in plot if e.tag.startswith(C) and e.tag.endswith("Chart")]
                series = sum(len([s for s in g if s.tag == C + "ser"]) for g in groups)
                if series < 2:
                    continue
                bardirs = {val(g, "barDir") for g in groups if g.tag == C + "barChart"
                           or g.tag == C + "bar3DChart"}
                groupings = {val(g, "grouping") for g in groups}
                swap = "bar" in bardirs
                stacked = bool(groupings & {"stacked", "percentStacked"})
                if swap:
                    rev = not stacked
                elif pos in ("l", "r"):
                    rev = stacked
                else:
                    rev = False
                if rev:
                    print(f"{n}\t{m}\t{pos}\t{sorted(x for x in bardirs if x)}\t"
                          f"{sorted(x for x in groupings if x)}\t{series}\tY")
                    docs.add(n)
    sys.stderr.write(f"{len(docs)} documents\n")
