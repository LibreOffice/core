#!/usr/bin/env python3
"""O50 -- which number format a data label's `[CATEGORY NAME]` field is written through.

`055_Project_timeline_with_milestones` draws thirteen milestone labels, each a
`c:dLbl/c:tx/c:rich` holding a `CELLRANGE` field and a `CATEGORYNAME` field. Its categories
are the serials of `'Project timeline'!$C$20:$C$36`, whose cells are formatted `m/d/yyyy`;
its `c:dateAx` states `numFmt formatCode="[$-409]d\ mmm;@" sourceLinked="0"`.

`ExplicitCategoriesProvider::convertCategoryAnysToText`
(`chart2/source/tools/ExplicitCategoriesProvider.cxx`:186-227) takes the number format ONCE,
before the loop, from `getAxisByDimension2(0, 0)` through
`AxisHelper::getExplicitNumberFormatKeyForAxis` -- the primary category axis -- and writes
every numeric category through it. `VSeriesPlotter::getCategoryName` (`:2213-2224`) is
`getSimpleCategories()[n]`, and `:511-513` is the `CATEGORYNAME` field's own case. So the
axis' format decides the field, and the source cell's decides nothing.

The variants change exactly one thing each:

  axis-<code>   the `c:dateAx`'s own formatCode (sourceLinked stays 0)
  axis-linked   `sourceLinked="1"` with the code left alone, which sends
                `getExplicitNumberFormatKeyForAxis` down its DATE branch to the data sequence
  cell-<code>   the number format of the C20:C36 cells, the axis untouched

    catname-055.py <outdir>
    soffice --headless --convert-to pdf --outdir <outdir>/ref <outdir>/055_*.xlsx
    catname-055.py --read <outdir>/ref
"""
import glob
import os
import re
import sys
import zipfile
from pathlib import Path

SRC = ("/home/user/sample-files/sheets/chartset-008/xlsx/"
       "055_Project_timeline_with_milestones_Use_this_template_546cecc0.xlsx")
CHART = "xl/charts/chart11.xml"
STYLES = "xl/styles.xml"
SHEET_FMT = "m/d/yyyy"          # the numFmtId the C20:C36 cells resolve to
AXIS = '<c:numFmt formatCode="[$-409]d\\ mmm;@" sourceLinked="0"/>'


def build(out):
    out.mkdir(parents=True, exist_ok=True)

    def write(name, chart=None, styles=None):
        with zipfile.ZipFile(SRC) as zin:
            path = out / ("055_%s.xlsx" % name)
            with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zout:
                for item in zin.infolist():
                    data = zin.read(item.filename)
                    if item.filename == CHART and chart is not None:
                        text = data.decode("utf-8")
                        assert text.count(AXIS) == 1, "the date axis' numFmt moved"
                        data = text.replace(AXIS, chart).encode("utf-8")
                    elif item.filename == STYLES and styles is not None:
                        text = data.decode("utf-8")
                        assert text.count(SHEET_FMT) >= 1, "the cells' format moved"
                        data = text.replace(SHEET_FMT, styles).encode("utf-8")
                    zout.writestr(item.filename, data)
        print(path)

    write("c0")
    for tag, code in (("yyyy", "yyyy"), ("mmmm", "mmmm"), ("hash", "0.00")):
        write("axis_" + tag,
              chart='<c:numFmt formatCode="%s" sourceLinked="0"/>' % code)
    write("axis_linked", chart='<c:numFmt formatCode="[$-409]d\\ mmm;@" sourceLinked="1"/>')
    for tag, code in (("yyyy", "yyyy"), ("hash", "0.00")):
        write("cell_" + tag, styles=code)


def read(refdir):
    """The thirteen milestone labels of page 1, as drawn."""
    import pymupdf
    for path in sorted(glob.glob(os.path.join(refdir, "055_*.pdf"))):
        name = os.path.basename(path)[4:-4]
        with pymupdf.open(path) as doc:
            texts = []
            for block in doc[0].get_text("dict")["blocks"]:
                if block["type"] != 0:
                    continue
                for line in block["lines"]:
                    for span in line["spans"]:
                        t = span["text"].strip()
                        if t:
                            texts.append(t)
        # a milestone label is the line after a `[CELLRANGE]` expansion; report the
        # distinct shapes of everything that parses as a date or a number
        shapes = []
        for t in texts:
            s = re.sub(r"\d", "9", t)
            if any(c.isdigit() for c in t) and s not in shapes:
                shapes.append(s)
        print("%-12s %s" % (name, " | ".join(shapes[:14])))


if __name__ == "__main__":
    if sys.argv[1] == "--read":
        read(sys.argv[2])
    else:
        build(Path(sys.argv[1]))
