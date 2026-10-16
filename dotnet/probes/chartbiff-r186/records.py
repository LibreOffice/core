#!/usr/bin/env python3
"""Print the CHPIE and CHSCATTER bodies of every BIFF chart substream in a workbook.

    ./records.py <file.xls> [...]

A `.xls` chart lives in a BIFF substream inside an OLE2 stream -- usually the ObjectPool --
and not in a zip part, so nothing that walks a package can see it. This walks every stream
of the compound file, finds the records by id, and prints the two fields that decide the
chart's type but have no record of their own:

    CHPIE     0x1019   anStart : u16   pcDonut : u16   grbit : u16   (BIFF8)
    CHSCATTER 0x101B   pcBubble: u16   wBubble : u16   grbit : u16   (BIFF8)

`pcDonut > 0` is a doughnut rather than a pie and `grbit & 0x0001`
(EXC_CHSCATTER_BUBBLES) is a bubble chart rather than a scatter --
`XclImpChType::Finalize`, sc/source/filter/excel/xichart.cxx:2288-2303.
"""
import struct
import sys

import olefile

CHPIE = 0x1019
CHSCATTER = 0x101B


def records(data):
    at = 0
    while at + 4 <= len(data):
        rid, size = struct.unpack_from("<HH", data, at)
        body = data[at + 4 : at + 4 + size]
        if len(body) != size:
            return
        yield rid, body
        at += 4 + size


def main() -> int:
    for path in sys.argv[1:]:
        found = []
        with olefile.OleFileIO(path) as ole:
            for entry in ole.listdir():
                data = ole.openstream(entry).read()
                for rid, body in records(data):
                    if rid == CHPIE and len(body) >= 4:
                        start, hole = struct.unpack_from("<HH", body)
                        flags = struct.unpack_from("<H", body, 4)[0] if len(body) >= 6 else 0
                        found.append(
                            "CHPIE     anStart=%-4d pcDonut=%-4d grbit=0x%04X  -> %s"
                            % (start, hole, flags, "DONUT" if hole > 0 else "PIE")
                        )
                    elif rid == CHSCATTER and len(body) >= 6:
                        size, kind, flags = struct.unpack_from("<HHH", body)
                        found.append(
                            "CHSCATTER pcBubble=%-3d wBubble=%-3d grbit=0x%04X  -> %s"
                            % (size, kind, flags,
                               "BUBBLES" if flags & 1 else "SCATTER")
                        )
        print("%-34s %s" % (path.rsplit("/", 1)[-1], "; ".join(found) or "(no CHPIE/CHSCATTER)"))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
