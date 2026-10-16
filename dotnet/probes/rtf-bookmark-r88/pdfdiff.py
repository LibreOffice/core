#!/usr/bin/env python3
"""How many renderings of a bank differ, with the dates masked.

The score sweep does not set SOURCE_DATE_EPOCH, so two runs of one binary differ in the
PDF's /CreationDate, its /ModDate, its /ID and the XMP dates beside them and in nothing
else. Masking those five is what makes a byte comparison mean something.

  pdfdiff.py <dir A> <dir B>
"""
import hashlib
import pathlib
import re
import sys

MASK = [
    re.compile(rb'/CreationDate\s*\([^)]*\)'),
    re.compile(rb'/ModDate\s*\([^)]*\)'),
    re.compile(rb'/ID\s*\[[^]]*\]'),
    re.compile(rb'<xmp:CreateDate>[^<]*</xmp:CreateDate>'),
    re.compile(rb'<xmp:ModifyDate>[^<]*</xmp:ModifyDate>'),
    re.compile(rb'<xmp:MetadataDate>[^<]*</xmp:MetadataDate>'),
]


def digest(path):
    data = path.read_bytes()
    for pattern in MASK:
        data = pattern.sub(b'', data)
    return hashlib.md5(data).hexdigest()


a, b = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
names = sorted({p.name for p in a.glob('*.pdf')} | {p.name for p in b.glob('*.pdf')})
same = different = missing = 0
for name in names:
    pa, pb = a / name, b / name
    if not pa.exists() or not pb.exists():
        missing += 1
        print(f"MISSING  {name}")
        continue
    if digest(pa) == digest(pb):
        same += 1
    else:
        different += 1
        print(f"DIFFERS  {name}")

print(f"\n{len(names)} renderings: {same} identical, {different} different, {missing} missing a side")
