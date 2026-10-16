#!/usr/bin/env python3
"""Every corpus document carrying a chart, counted both ways.

A `c:chartSpace` zip part misses every `.xls` chart, which lives in a BIFF substream — a
`BOF` (0x0809) whose substream type is 0x0020, in *any* OLE2 stream, the ObjectPool
included. Counting only one way is how a reach figure loses the legacy binaries.

    census.py [corpus] > census.tsv
"""
import struct, sys, zipfile
from pathlib import Path
import olefile

CORPUS = Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files')


def ooxml_parts(path):
    try:
        with zipfile.ZipFile(path) as z:
            n = 0
            for info in z.infolist():
                if not info.filename.lower().endswith('.xml'):
                    continue
                if 'chart' not in info.filename.lower():
                    continue
                try:
                    head = z.read(info.filename)[:4096]
                except Exception:
                    continue
                if b'chartSpace' in head:
                    n += 1
            return n
    except Exception:
        return 0


def biff_substreams(path):
    try:
        ole = olefile.OleFileIO(path)
    except Exception:
        return 0
    n = 0
    for entry in ole.listdir(streams=True, storages=False):
        try:
            data = ole.openstream('/'.join(entry)).read()
        except Exception:
            continue
        p, end = 0, len(data)
        while p + 4 <= end:
            rid, ln = struct.unpack_from('<HH', data, p)
            if rid == 0x0809 and ln >= 4 and p + 4 + ln <= end:
                if struct.unpack_from('<H', data, p + 6)[0] == 0x0020:
                    n += 1
            if ln == 0 and rid == 0:
                break
            p += 4 + ln
    try:
        ole.close()
    except Exception:
        pass
    return n


print('path\tooxml\tbiff')
for path in sorted(CORPUS.rglob('*')):
    if not path.is_file() or path.suffix.lower() not in (
            '.docx', '.doc', '.xlsx', '.xls', '.xlsm', '.pptx', '.ppt'):
        continue
    with open(path, 'rb') as fh:
        magic = fh.read(8)
    ooxml = ooxml_parts(path) if magic[:2] == b'PK' else 0
    biff = biff_substreams(path) if magic == b'\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1' else 0
    if ooxml or biff:
        print(f'{path.relative_to(CORPUS)}\t{ooxml}\t{biff}')
