#!/usr/bin/env python3
"""Which corpus documents state a doughnut or a bubble chart in BIFF.

    ./census.py [/home/user/sample-files]

Walks every OLE2 file, not every `.xls` -- a chart substream can sit in a `.doc` or a `.ppt`
ObjectPool too, and a zip walk cannot see any of them. This is the census a `.xlsx`-keyed
one misses entirely.
"""
import pathlib
import struct
import sys

import olefile

OLE2 = (".xls", ".xlsm", ".xlsx", ".doc", ".docx", ".ppt", ".pptx")


def kinds(path):
    found = set()
    with olefile.OleFileIO(str(path)) as ole:
        for entry in ole.listdir():
            try:
                data = ole.openstream(entry).read()
            except Exception:                            # noqa: BLE001 - a broken stream is a datum
                continue
            at = 0
            while at + 4 <= len(data):
                rid, size = struct.unpack_from("<HH", data, at)
                if at + 4 + size > len(data):
                    break
                if rid == 0x1019 and size >= 4:
                    found.add("donut" if struct.unpack_from("<H", data, at + 6)[0] else "pie")
                elif rid == 0x101B and size >= 6:
                    found.add("bubble" if struct.unpack_from("<H", data, at + 8)[0] & 1
                              else "scatter")
                at += 4 + size
    return found


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "/home/user/sample-files")
    scanned = 0
    for path in sorted(root.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in OLE2:
            continue
        try:
            if not olefile.isOleFile(str(path)):
                continue
            found = kinds(path)
        except Exception:                                # noqa: BLE001
            continue
        scanned += 1
        if found:
            print("%-12s %s" % (",".join(sorted(found)), path.name))
    print("OLE2 files scanned: %d" % scanned)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
