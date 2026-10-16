#!/usr/bin/env python3
"""Which renderings actually moved between two sweeps, with the conversion date masked out.

`soffice` and this tree both stamp the wall clock into a PDF's /CreationDate and its XMP, so a
raw byte diff of two sweeps reports every document as changed.  Masking the four date carriers
leaves a comparison that means what it says.
"""
import hashlib, os, re, sys

PAT = re.compile(
    rb'/CreationDate\s*\([^)]*\)'
    rb'|/ModDate\s*\([^)]*\)'
    rb'|/ID\s*\[[^\]]*\]'
    rb'|<xmp:CreateDate>[^<]*</xmp:CreateDate>'
    rb'|<xmp:ModifyDate>[^<]*</xmp:ModifyDate>'
    rb'|<xmp:MetadataDate>[^<]*</xmp:MetadataDate>'
    rb'|<dc:date>[^<]*</dc:date>')

def digest(path):
    return hashlib.md5(PAT.sub(b'', open(path, 'rb').read())).hexdigest()

a, b = sys.argv[1], sys.argv[2]
names = sorted(set(os.listdir(a)) & set(os.listdir(b)))
moved = [n for n in names if digest(os.path.join(a, n)) != digest(os.path.join(b, n))]
print(f"{len(names)} renderings compared with the date masked, {len(moved)} differ")
for n in moved:
    print("  ", n)
