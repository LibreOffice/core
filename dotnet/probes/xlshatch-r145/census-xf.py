#!/usr/bin/env python3
"""Census the fill pattern of every BIFF XF in the corpus's `.xls`, and of every `CF` record.

A cell's fill pattern is a six-bit field, and only 0 (none) and 1 (solid) are what the reader
takes whole today; 2..18 are the hatches `XclTools::GetPatternColor` mixes. The question this
answers is not how many XF records state one -- it is how many of them a *cell* can reach,
which needs the XF index a cell states as well.

Usage: census-xf.py [root]
"""
import collections
import glob
import os
import struct
import sys

import olefile

BOF = 0x0809
EOF = 0x000A
XF = 0x00E0
CONDFMT = 0x01B0
CF = 0x01B1
BLANK, NUMBER, LABEL, BOOLERR, RK = 0x0201, 0x0203, 0x0204, 0x0205, 0x027E
LABELSST, FORMULA, MULRK, MULBLANK, ROW = 0x00FD, 0x0006, 0x00BD, 0x00BE, 0x0208

# Records whose first six bytes are row, column, XF index.
CELL = {BLANK, NUMBER, LABEL, BOOLERR, RK, LABELSST, FORMULA}


def records(stream):
    at = 0
    while at + 4 <= len(stream):
        rid, size = struct.unpack_from('<HH', stream, at)
        body = stream[at + 4:at + 4 + size]
        yield rid, body
        at += 4 + size


def workbook(path):
    ole = olefile.OleFileIO(path)
    for name in ('Workbook', 'Book'):
        if ole.exists(name):
            return ole.openstream(name).read()
    return None


def scan(path):
    """(patterns stated by XFs, patterns a cell reaches, patterns a CF states)."""
    stream = workbook(path)
    if stream is None:
        return None

    xfs = []
    used = collections.Counter()
    stated = collections.Counter()
    cf = collections.Counter()
    version = None

    for rid, body in records(stream):
        if rid == BOF and len(body) >= 2:
            version = struct.unpack_from('<H', body, 0)[0]
        elif rid == XF:
            # BIFF8 XF is 20 bytes: the pattern is bits 26..31 of the second border dword,
            # which starts at offset 14. BIFF5's XF is 16 bytes with the pattern at bits
            # 16..21 of the area dword at offset 12.
            if version == 0x0600 and len(body) >= 20:
                border2 = struct.unpack_from('<I', body, 14)[0]
                pattern = (border2 >> 26) & 0x3F
            elif len(body) >= 16:
                area = struct.unpack_from('<I', body, 12)[0]
                pattern = (area >> 16) & 0x3F
            else:
                pattern = 0
            xfs.append(pattern)
            stated[pattern] += 1
        elif rid in CELL and len(body) >= 6:
            index = struct.unpack_from('<H', body, 4)[0]
            if index < len(xfs):
                used[xfs[index]] += 1
        elif rid == MULRK and len(body) >= 6:
            first = struct.unpack_from('<H', body, 2)[0]
            count = (len(body) - 6) // 6
            for i in range(count):
                index = struct.unpack_from('<H', body, 4 + i * 6)[0]
                if index < len(xfs):
                    used[xfs[index]] += 1
        elif rid == MULBLANK and len(body) >= 6:
            count = (len(body) - 6) // 2
            for i in range(count):
                index = struct.unpack_from('<H', body, 4 + i * 2)[0]
                if index < len(xfs):
                    used[xfs[index]] += 1
        elif rid == CF and len(body) >= 6:
            # The `CF` record's area block is optional and sits after the font and border
            # blocks; its pattern is bits 10..15 of the first area word. Counting it needs the
            # block sizes, so this only records that the file HAS a CF at all.
            cf['records'] += 1

    return stated, used, cf


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files'
    stated = collections.Counter()
    used = collections.Counter()
    docs_stated, docs_used = set(), set()
    files = 0

    # Case-folded rather than a lower-case glob: four corpus documents wear an upper-case
    # extension, and this mount will also materialise a case-variant alias of any name a tool
    # looks up, so the walk folds case and keeps one entry per resolved path.
    seen = set()
    candidates = []
    for found in glob.glob(os.path.join(root, '**', '*.*'), recursive=True):
        if os.path.splitext(found)[1].lower() not in ('.xls', '.xlt'):
            continue
        real = os.path.realpath(found)
        if real in seen:
            continue
        seen.add(real)
        candidates.append(found)

    for path in sorted(candidates):
        try:
            result = scan(path)
        except Exception as error:                       # a corrupt file is data, not a crash
            print('SKIP %s (%s)' % (path, error), file=sys.stderr)
            continue
        if result is None:
            print('NOT-BIFF %s' % path, file=sys.stderr)
            continue
        files += 1
        s, u, _ = result
        for pattern, count in s.items():
            if pattern not in (0, 1):
                stated[pattern] += count
                docs_stated.add(path)
        for pattern, count in u.items():
            if pattern not in (0, 1):
                used[pattern] += count
                docs_used.add(path)

    print('BIFF workbooks scanned: %d' % files)
    print('XF records stating a hatch: %d in %d documents %s'
          % (sum(stated.values()), len(docs_stated), dict(sorted(stated.items()))))
    print('cells reaching one:         %d in %d documents %s'
          % (sum(used.values()), len(docs_used), dict(sorted(used.items()))))
    for path in sorted(docs_used):
        print('   ', path)


if __name__ == '__main__':
    main()
