#!/usr/bin/env python3
"""One property, one arm: c:crossBetween on the crossing value axis of three corpus charts.

`oox/source/drawingml/chart/axisconverter.cxx:292-301` decides ScaleData::ShiftedCategoryPosition
from the *crossing* value axis' c:crossBetween ahead of the chart type, and falls back to the
first type group's type only when the element is absent.  Three chart types x three arms
(between / midCat / element deleted) is the smallest set that separates the two rules: the
absent arm is the only one whose answer differs BETWEEN the types.

Everything else in each deck is byte-identical across its own three arms.
"""
import os, re, shutil, sys, zipfile

SRC = {
    "area": ("/c/sandbox/workdir/sample-files/slides/chartset-002/pptx/"
             "006_advanced_powerpoint_area.pptx", "ppt/charts/chart6.xml"),
    "line": ("/c/sandbox/workdir/sample-files/slides/chartset-003/pptx/"
             "003_advanced_powerpoint_line.pptx", None),
    "column": ("/c/sandbox/workdir/sample-files/slides/chartset-003/pptx/"
               "002_advanced_powerpoint_column.pptx", "ppt/charts/chart2.xml"),
}

ARMS = {
    "between": lambda s: re.sub(r'<c:crossBetween val="\w+"/>',
                                '<c:crossBetween val="between"/>', s),
    "midcat": lambda s: re.sub(r'<c:crossBetween val="\w+"/>',
                               '<c:crossBetween val="midCat"/>', s),
    "absent": lambda s: re.sub(r'<c:crossBetween val="\w+"/>', '', s),
}


def chart_part(z):
    for n in z.namelist():
        if re.match(r'ppt/charts/chart\d*\.xml$', n):
            return n
    raise SystemExit("no chart part")


def write(src, part, arm, out):
    zin = zipfile.ZipFile(src)
    part = part or chart_part(zin)
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == part:
                s = data.decode("utf-8")
                before = s.count("<c:crossBetween")
                s = ARMS[arm](s)
                data = s.encode("utf-8")
                print(f"  {os.path.basename(out)}: {before} crossBetween -> "
                      f"{s.count('<c:crossBetween')}")
            zout.writestr(item, data)


if __name__ == "__main__":
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    for kind, (src, part) in SRC.items():
        for arm in ARMS:
            write(src, part, arm, os.path.join(out, f"{kind}-{arm}.pptx"))
