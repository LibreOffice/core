#!/usr/bin/env python3
"""How many drawing operations a PDF's pages carry, summed over the document.

Counts the path-painting operators -- `f`, `f*`, `S`, `s`, `B`, `B*`, `b`, `b*` -- which is
what separates "the chart was drawn" from "the frame was left empty". Text is deliberately
not counted: a chart that draws nothing still leaves the sheet's own cells on the page.
"""
import re
import sys

import pymupdf

PAINT = re.compile(rb"(?:^|[\s\]>])(f\*?|s|S|B\*?|b\*?)(?=[\s\r\n]|$)", re.M)


def main() -> int:
    if len(sys.argv) < 2 or not sys.argv[1]:
        print("-")
        return 0
    try:
        document = pymupdf.open(sys.argv[1])
    except Exception:                                    # noqa: BLE001 - "no file" is a datum
        print("-")
        return 0
    total = 0
    for page in document:
        for xref in page.get_contents():
            total += len(PAINT.findall(document.xref_stream(xref)))
    print(total)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
