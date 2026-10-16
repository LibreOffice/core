#!/usr/bin/env python3
"""Census the formatting runs a `.xls` text box states, which the reader does not read.

A `TXO` (0x01B6) declares the string's length and the *run* table's length; the characters
arrive in the CONTINUE that follows and the runs in a second one, eight bytes each — a
character offset and a FONT index (`XclImpDrawing::ReadTxo`,
`sc/source/filter/excel/xiescher.cxx`:4242). `XlsDrawingCollector.ReadText` reads the string
and stops, so every shape's text is drawn at one hardcoded size in one weight.

Usage: census-txo.py <corpus-root>
"""
import pathlib
import struct
import sys

import olefile

TXO = 0x01B6
CONTINUE = 0x003C
BOF = 0x0809


def records(data):
    at = 0
    while at + 4 <= len(data):
        rid, size = struct.unpack_from("<HH", data, at)
        at += 4
        yield rid, data[at:at + size]
        at += size


def main():
    root = pathlib.Path(sys.argv[1])
    boxes = runs = docs = 0
    multi = 0
    multi_docs = set()
    for path in sorted(p for p in root.rglob("*")
                       if p.suffix.lower() == ".xls" and p.is_file()):
        try:
            ole = olefile.OleFileIO(str(path))
            name = "Workbook" if ole.exists("Workbook") else "Book"
            data = ole.openstream(name).read()
        except Exception:  # noqa: BLE001
            continue
        here = 0
        here_multi = 0
        for rid, body in records(data):
            if rid != TXO or len(body) < 18:
                continue
            length = struct.unpack_from("<H", body, 10)[0]
            runbytes = struct.unpack_from("<H", body, 12)[0]
            if length == 0:
                continue
            here += 1
            # Two runs is the minimum a TXO writes: the opening one and the terminator.
            n = runbytes // 8
            runs += n
            if n > 2:
                here_multi += 1
        if here:
            docs += 1
            boxes += here
            multi += here_multi
            if here_multi:
                multi_docs.add(path.name)
    print(f"{boxes} text boxes with text in {docs} .xls; {runs} formatting runs")
    print(f"{multi} of them state more than the opening run, in {len(multi_docs)} documents:")
    for name in sorted(multi_docs):
        print(f"  {name}")


if __name__ == "__main__":
    main()
