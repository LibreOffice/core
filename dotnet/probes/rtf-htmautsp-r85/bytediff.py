#!/usr/bin/env python3
"""Taken unchanged from probes/rtf-resid-r80.

Byte-compare two banks of rendered PDFs, masking the creation date.

    bytediff.py <dir-a> <dir-b>

`soffice` and this tree both stamp a timestamp into the file, so a raw byte compare finds a
difference on every document; the mask is what makes the comparison mean anything.
"""
import sys
import pathlib
import re

A, B = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
MASK = re.compile(rb"/(CreationDate|ModDate) *\([^)]*\)|<xmp:(CreateDate|ModifyDate)>[^<]*<")


def body(p):
    return MASK.sub(b"", p.read_bytes())


same = diff = missing = 0
names = sorted(p.name for p in A.glob("*.pdf"))
for n in names:
    other = B / n
    if not other.exists():
        missing += 1
        print(f"  MISSING  {n}")
        continue
    if body(A / n) == body(other):
        same += 1
    else:
        diff += 1
        print(f"  DIFFERS  {n}")
print(f"{len(names)} in {A.name}: identical {same}, different {diff}, missing from {B.name} {missing}")
