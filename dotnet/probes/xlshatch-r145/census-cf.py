#!/usr/bin/env python3
"""Census the pattern every `CF` record's area block states, over the corpus's BIFF workbooks.

The blocks are optional and sized, so reaching the area block means walking past whichever of
the number-format, font, alignment and border blocks the record's flags say are present -- the
same walk `XlsConditionalFormats.ReadRule` makes, and the same one `XclImpCondFormat::ReadCF`
makes (`sc/source/filter/excel/xicontent.cxx`).
"""
import collections
import glob
import os
import struct
import sys

import olefile

BOF, EOF = 0x0809, 0x000A
CONDFMT, CF = 0x01B0, 0x01B1

BLOCK_NUMFMT = 0x02000000
BLOCK_FONT = 0x04000000
BLOCK_ALIGNMENT = 0x08000000
BLOCK_BORDER = 0x10000000
BLOCK_AREA = 0x20000000
NUMFMT_IS_USER = 0x00000001


def records(stream):
    at = 0
    while at + 4 <= len(stream):
        rid, size = struct.unpack_from('<HH', stream, at)
        yield rid, stream[at + 4:at + 4 + size]
        at += 4 + size


def workbook(path):
    ole = olefile.OleFileIO(path)
    for name in ('Workbook', 'Book'):
        if ole.exists(name):
            return ole.openstream(name).read()
    return None


def area_of(body):
    """(pattern, foreground, background, flags) for one CF record, or None."""
    if len(body) < 12:
        return None
    flags = struct.unpack_from('<I', body, 6)[0]
    if not flags & BLOCK_AREA:
        return None

    at = 12
    if flags & BLOCK_NUMFMT:
        # The user-format form is a length-prefixed string; the built-in form is two bytes.
        if flags & NUMFMT_IS_USER:
            if at >= len(body):
                return None
            size = body[at]
            at += 1 + size * 2 + 1          # a rough walk; a miss shows up as a silly pattern
        else:
            at += 2
    if flags & BLOCK_FONT:
        at += 118
    if flags & BLOCK_ALIGNMENT:
        at += 8
    if flags & BLOCK_BORDER:
        at += 8
    if at + 4 > len(body):
        return None

    packed_pattern, packed_colour = struct.unpack_from('<HH', body, at)
    return ((packed_pattern >> 10) & 0x3F,
            packed_colour & 0x7F,
            (packed_colour >> 7) & 0x7F,
            flags)


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files'
    patterns = collections.Counter()
    documents = collections.defaultdict(set)
    records_seen = 0
    files = 0

    seen = set()
    for found in glob.glob(os.path.join(root, '**', '*.*'), recursive=True):
        if os.path.splitext(found)[1].lower() not in ('.xls', '.xlt'):
            continue
        real = os.path.realpath(found)
        if real in seen:
            continue
        seen.add(real)
        try:
            stream = workbook(found)
        except Exception:
            continue
        if stream is None:
            continue
        files += 1
        for rid, body in records(stream):
            if rid != CF:
                continue
            records_seen += 1
            area = area_of(body)
            if area is None:
                continue
            patterns[area[0]] += 1
            documents[area[0]].add(found)

    print('BIFF workbooks scanned: %d' % files)
    print('CF records: %d' % records_seen)
    for pattern in sorted(patterns):
        print('  pattern %2d: %3d records in %d documents'
              % (pattern, patterns[pattern], len(documents[pattern])))
        if pattern not in (0, 1):
            for path in sorted(documents[pattern]):
                print('       ', path)


if __name__ == '__main__':
    main()
