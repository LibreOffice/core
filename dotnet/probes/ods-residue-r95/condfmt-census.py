#!/usr/bin/env python3
"""Census of BIFF CONDFMT (0x01B0) records over the corpus's `.xls`.

Walks every OLE2 workbook stream, splits it into BIFF records, and for each
worksheet substream counts CONDFMT records and the ranges each covers.

The record layout is LibreOffice's own read of it — `XclImpCondFormat::ReadCondfmt`,
`sc/source/filter/excel/xicontent.cxx`:516-524:

    ccf      uint16                     how many CF records follow
    ignore   10 bytes                   grbit (2) + the enclosing Ref8U (8)
    sqref    uint16 count, then that
             many 8-byte ranges         rwFirst, rwLast, colFirst, colLast, all uint16

Usage: condfmt-census.py <corpus-root> [...]
"""
import struct
import sys
from pathlib import Path

import olefile

BOF = 0x0809
EOF_ = 0x000A
CONDFMT = 0x01B0
CF = 0x01B1
CONTINUE = 0x003C
BOUNDSHEET = 0x0085


def records(data):
    off = 0
    n = len(data)
    while off + 4 <= n:
        rid, size = struct.unpack_from('<HH', data, off)
        off += 4
        if off + size > n:
            break
        yield rid, data[off:off + size]
        off += size


def condfmt_ranges(body):
    if len(body) < 14:
        return None
    ccf = struct.unpack_from('<H', body, 0)[0]
    off = 2 + 10
    if off + 2 > len(body):
        return None
    count = struct.unpack_from('<H', body, off)[0]
    off += 2
    out = []
    for _ in range(count):
        if off + 8 > len(body):
            break
        r0, r1, c0, c1 = struct.unpack_from('<HHHH', body, off)
        off += 8
        out.append((r0, r1, c0, c1))
    return ccf, out


def workbook_stream(ole):
    for name in ('Workbook', 'Book'):
        if ole.exists(name):
            return ole.openstream(name).read()
    return None


def scan(path):
    try:
        ole = olefile.OleFileIO(str(path))
    except Exception as exc:                       # noqa: BLE001
        return ('not-ole2', str(exc), [])
    try:
        data = workbook_stream(ole)
    finally:
        ole.close()
    if data is None:
        return ('no-workbook-stream', '', [])

    found = []
    substream = None            # BOF dt of the substream we are inside
    sheet_index = -1
    for rid, body in records(data):
        if rid == BOF:
            dt = struct.unpack_from('<H', body, 2)[0] if len(body) >= 4 else -1
            substream = dt
            if dt == 0x0010:
                sheet_index += 1
        elif rid == EOF_:
            substream = None
        elif rid == CONDFMT and substream == 0x0010:
            parsed = condfmt_ranges(body)
            if parsed is not None:
                ccf, ranges = parsed
                found.append((sheet_index, ccf, ranges))
    return ('ok', '', found)


def main():
    roots = [Path(a) for a in sys.argv[1:]] or [Path('/home/user/sample-files/sheets')]
    paths = sorted(
        {p.resolve() for root in roots for p in root.rglob('*')
         if p.is_file() and p.suffix.lower() == '.xls'})
    print('document\tstatus\tcondfmt\tranges\tcells')
    docs = 0
    for p in paths:
        status, err, found = scan(p)
        if not found:
            print(f'{p.name}\t{status}{(":" + err) if err else ""}\t0\t0\t0')
            continue
        docs += 1
        nranges = sum(len(r) for _, _, r in found)
        cells = sum((r1 - r0 + 1) * (c1 - c0 + 1)
                    for _, _, rs in found for (r0, r1, c0, c1) in rs)
        print(f'{p.name}\t{status}\t{len(found)}\t{nranges}\t{cells}')
    print(f'# {docs} of {len(paths)} .xls state a CONDFMT', file=sys.stderr)


if __name__ == '__main__':
    main()
