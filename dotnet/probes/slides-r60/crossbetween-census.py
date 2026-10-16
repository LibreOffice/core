#!/usr/bin/env python3
"""Every OOXML chart part in the corpus, with its c:crossBetween and its type group.

Reads the part rather than the rendering.  Reports, per chart part, the chart-type elements
present, whether a c:catAx exists, and the c:crossBetween value stated on each c:valAx --
which is what `oox/source/drawingml/chart/axisconverter.cxx:292-301` uses to decide
ScaleData::ShiftedCategoryPosition, ahead of the chart type.
"""
import os, re, sys, zipfile, collections

TYPES = ("barChart", "bar3DChart", "lineChart", "line3DChart", "areaChart", "area3DChart",
         "scatterChart", "pieChart", "pie3DChart", "doughnutChart", "radarChart",
         "stockChart", "bubbleChart", "surfaceChart", "ofPieChart")


def parts(path):
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return
    for n in z.namelist():
        if re.match(r'(ppt|xl|word)/charts/chart\d*\.xml$', n):
            try:
                yield n, z.read(n).decode("utf-8", "replace")
            except Exception:
                pass


if __name__ == "__main__":
    root = sys.argv[1]
    fams = sys.argv[2:] or ["slides", "sheets", "words"]
    tally = collections.Counter()
    for fam in fams:
        base = os.path.join(root, fam)
        for dirpath, _, names in os.walk(base):
            for nm in sorted(names):
                if not nm.lower().endswith((".pptx", ".xlsx", ".docx", ".xlsm", ".pptm", ".docm")):
                    continue
                p = os.path.join(dirpath, nm)
                for part, s in parts(p):
                    kinds = [t for t in TYPES if "<c:%s>" % t in s or "<c:%s " % t in s]
                    cb = re.findall(r'<c:crossBetween val="(\w+)"', s)
                    bardir = re.findall(r'<c:barDir val="(\w+)"', s)
                    row = (fam, nm, part, "+".join(kinds), "+".join(bardir), "+".join(cb) or "-")
                    print("\t".join(row))
                    tally[(fam, "+".join(kinds), "+".join(cb) or "-")] += 1
    print("# --- tally", file=sys.stderr)
    for k, v in sorted(tally.items()):
        print("#\t%s\t%s\t%s\t%d" % (k[0], k[1], k[2], v), file=sys.stderr)
