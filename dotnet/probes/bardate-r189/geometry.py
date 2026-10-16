#!/usr/bin/env python3
"""Where a chart's bars, its plot edges and its axis ticks land, out of a PDF page.

    ./geometry.py <pdf> <page> [--grey]

Prints the plot rectangle's widest horizontal rule, the tick marks below it, and every filled
rectangle that could be a bar with its pitch. That is enough to solve for the plot's left edge
from two renderings of the same chart at different stated ranges, which is what separates a
category centred on its date from one that runs from it.
"""
import sys

import pymupdf


def main() -> int:
    path = sys.argv[1]
    page = int(sys.argv[2]) - 1
    grey = "--grey" in sys.argv

    document = pymupdf.open(path)
    sheet = document[page]

    rules = set()
    ticks = set()
    bars = []

    for drawing in sheet.get_drawings():
        rect = drawing["rect"]

        if drawing["type"] in ("s", "fs"):
            if rect.height < 2 and rect.width > 50:
                rules.add((round(rect.x0, 2), round(rect.x1, 2)))
            if rect.width < 2 and 2 < rect.height < 8:
                ticks.add(round(rect.x0, 2))

        if drawing["type"] not in ("f", "fs"):
            continue

        fill = drawing.get("fill")
        if not fill or rect.height < 10 or rect.width < 1 or rect.width > 80:
            continue
        if grey and (max(fill) - min(fill) > 0.06 or not 0.3 < sum(fill) / 3 < 0.85):
            continue

        bars.append((round(rect.x0, 2), round(rect.width, 2)))

    order = sorted(rules)
    print("plot rules      : %s" % (order[:2] or "-"))

    marks = sorted(ticks)
    if len(marks) > 1:
        print("ticks           : n=%d first %.2f pitch %.2f"
              % (len(marks), marks[0], marks[1] - marks[0]))

    unique = sorted(set(bars))
    if unique:
        pitch = [round(unique[i + 1][0] - unique[i][0], 2) for i in range(len(unique) - 1)]
        print("bars            : n=%d first %.2f last %.2f width %.2f pitch %s"
              % (len(unique), unique[0][0], unique[-1][0], unique[0][1], pitch[:5]))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
