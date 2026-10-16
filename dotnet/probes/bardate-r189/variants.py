#!/usr/bin/env python3
"""One workbook per stated date-axis range, everything else left alone.

    ./variants.py <source.xlsx> <outdir>

The two variants exist to solve for the plot's left edge, which is the only unknown standing
between "the category is centred on its date" and "the category runs from its date". One
rendering cannot separate them -- both fit, with different left edges -- and two with different
stated ranges can, because the left edge must come out the same.
"""
import pathlib
import re
import sys
import zipfile

# The first point of 044_Cash_flow_forecast is serial 44958 and the last 45292.
CASES = {
    "exact": ("44958", "45292"),   # the range the automatic axis chooses
    "lo": ("44927", "45323"),      # a month of margin either side
}

SCALING = ('<c:dateAx><c:axId val="688037200"/>'
           '<c:scaling><c:orientation val="minMax"/></c:scaling>')


def main() -> int:
    src = pathlib.Path(sys.argv[1])
    out = pathlib.Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)

    with zipfile.ZipFile(src) as package:
        entries = [(i, package.read(i.filename)) for i in package.infolist()]

    at = next((k for k, (i, _) in enumerate(entries)
               if "/charts/chart" in i.filename and i.filename.endswith(".xml")), None)
    if at is None:
        print("no chart part in %s" % src, file=sys.stderr)
        return 1

    body = entries[at][1].decode()
    if SCALING not in body:
        print("the chart's date axis is not the shape this script patches", file=sys.stderr)
        return 1

    for name, (low, high) in CASES.items():
        patched = body.replace(
            SCALING,
            SCALING.replace("</c:scaling>",
                            '<c:max val="%s"/><c:min val="%s"/></c:scaling>' % (high, low)),
            1)
        target = out / ("044-%s.xlsx" % name)
        with zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as w:
            for k, (info, data) in enumerate(entries):
                w.writestr(info, patched.encode() if k == at else data)
        print("wrote %s" % target)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
