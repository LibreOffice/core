#!/usr/bin/env python3
"""Remove every embedded chart from a workbook, leaving the sheet untouched.

The question a `chartset-*` gate row poses is whether its divergence is the chart's
or the sheet's, and nothing in a rendered page separates the two reliably.  A
workbook whose `xdr:graphicFrame` anchors are gone renders the same sheet with no
chart on it, so what remains of the divergence is the sheet's by construction.
"""
import re, sys, zipfile
from pathlib import Path

FRAME = re.compile(rb'<xdr:graphicFrame.*?</xdr:graphicFrame>', re.S)
ANCHOR = re.compile(rb'<xdr:(twoCellAnchor|oneCellAnchor|absoluteAnchor)\b.*?</xdr:\1>', re.S)


def strip(src, dst):
    removed = 0
    with zipfile.ZipFile(src) as zf, zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as out:
        for i in zf.infolist():
            d = zf.read(i.filename)
            if i.filename.startswith('xl/drawings/drawing') and i.filename.endswith('.xml'):
                def keep(m):
                    nonlocal removed
                    if FRAME.search(m.group(0)):
                        removed += 1
                        return b''
                    return m.group(0)
                d = ANCHOR.sub(keep, d)
            out.writestr(i, d)
    return removed


if __name__ == '__main__':
    print(f'{Path(sys.argv[1]).name}\t{strip(sys.argv[1], sys.argv[2])}')
