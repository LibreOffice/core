#!/usr/bin/env python3
"""How far `ChartLayout.IntervalsThatFit` reaches: every chart with an AUTOMATIC value axis.

`IntervalsThatFit` returns `MaximumAutoIntervalCount` unchanged when the plot states a
`c:majorUnit`, so a stated interval is inert to it. Everything else -- every value axis whose
major unit is automatic -- goes through `available / needed`, and a rule that governs that many
axes across three tracks wants its reach on the table before a line of it is changed.

`ChartLayout` lives in **XRay.Office.Core**, so this counts all 946 corpus documents and not the
sheets track alone.

Blind spots, stated:
  * A `c:valAx` is counted whether or not its plot is one this code lays out with axes -- a pie
    or a doughnut chart part carries no `c:valAx` at all, so that is mostly self-correcting, but
    a hidden axis (`c:delete val="1"`) is counted here and reaches nothing.
  * `.xls`/`.ppt`/`.doc` binary charts are not counted: their axes come through the Escher chart
    records, not through any XML part.
  * It counts axes, not the axes where the rule actually *bites* -- which is the ones where
    `available / needed` comes out below ten. That number needs the layout run, not the file.
"""
import csv, os, re, sys, zipfile
import xml.etree.ElementTree as ET
from collections import Counter

CORPUS = "/c/sandbox/workdir/sample-files"
C = "{http://schemas.openxmlformats.org/drawingml/2006/chart}"


def main():
    per_family = Counter()
    docs = Counter()
    parts = 0
    auto = stated = 0
    binary = Counter()
    for r in csv.DictReader(open(os.path.join(CORPUS, "MANIFEST.tsv"), newline=""), delimiter="\t"):
        p = os.path.join(CORPUS, r["path"])
        if not os.path.exists(p):
            continue
        if r["path"].lower().endswith((".xls", ".ppt", ".doc")):
            binary[r["family"]] += 1
            continue
        try:
            z = zipfile.ZipFile(p)
        except Exception:
            continue
        hit = False
        with z:
            for name in z.namelist():
                if "chart" not in name or not name.endswith(".xml"):
                    continue
                try:
                    root = ET.fromstring(z.read(name))
                except Exception:
                    continue
                if not root.tag.endswith("}chartSpace"):
                    continue
                parts += 1
                for ax in root.iter(C + "valAx"):
                    if ax.find(C + "majorUnit") is None:
                        auto += 1
                        hit = True
                    else:
                        stated += 1
        if hit:
            per_family[r["family"]] += 1
        docs[r["family"]] += 1
    print("OOXML packages opened: %s" % dict(docs))
    print("chartSpace parts: %d" % parts)
    print("value axes with an AUTOMATIC major unit: %d" % auto)
    print("value axes with a STATED major unit:     %d" % stated)
    print("documents holding at least one automatic value axis, by track: %s" % dict(per_family))
    print("binary documents not visible to this census (their axes are Escher records): %s"
          % dict(binary))


main()
