#!/usr/bin/env python3
"""Count Escher WordArt shapes in a corpus of binary Office documents.

A WordArt shape stores its string in the `msofbtOPT` property `gtextUNICODE` (0x00C0), which is
what `SvxMSDffManager::ApplyAttributes` keys on (`filter/source/msfilter/msdffimp.cxx`). Neither
VML nor DrawingML is involved, so this is the whole population the Escher readers would have to
grow a path for.

Scans for the `msofbtOPT` record header anywhere in the file — Escher containers are embedded in
several different streams across DOC, PPT and XLS, and walking each format's stream layout would
be three readers to write for a census. A header is accepted only when its version nibble is 3,
its instance is a plausible property count, and its stated length covers that many six-byte
entries, which is what keeps the byte scan from reporting noise.

    census.py <corpus-root>
"""
import collections
import glob
import os
import struct
import sys

BINARY = ('.doc', '.dot', '.ppt', '.pot', '.pps', '.xls', '.xlt')
GTEXT_UNICODE = 192


def shapes_in(path):
    data = open(path, 'rb').read()
    found = 0
    i = 0
    while True:
        i = data.find(b'\x0b\xf0', i + 1)
        if i < 2:
            break
        version_instance = struct.unpack_from('<H', data, i - 2)[0]
        count = version_instance >> 4
        if (version_instance & 0xF) != 3 or not 0 < count <= 200:
            continue
        length = struct.unpack_from('<I', data, i + 2)[0]
        if length < count * 6 or i + 6 + length > len(data):
            continue
        for k in range(count):
            identifier = struct.unpack_from('<H', data, i + 6 + k * 6)[0]
            if (identifier & 0x3FFF) == GTEXT_UNICODE and (identifier & 0x8000):
                found += 1
    return found


def main(root):
    per_document = collections.Counter()
    scanned = 0
    for path in sorted(glob.glob(os.path.join(root, '**', '*'), recursive=True)):
        if not os.path.isfile(path) or not path.lower().endswith(BINARY):
            continue
        scanned += 1
        found = shapes_in(path)
        if found:
            per_document[os.path.relpath(path, root)] = found

    print(f'{scanned} binary documents scanned')
    print(f'{len(per_document)} carry Escher WordArt, {sum(per_document.values())} shapes in all')
    for name, count in per_document.most_common():
        print(f'  {count}  {name}')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '.')
