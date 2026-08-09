#!/usr/bin/env python3
"""Documents embedding a JPEG that is not three-component colour.

The pass-through branch in `PdfImages` declared `/DeviceRGB` for every JPEG it handed to
`DCTDecode`, so a one-component (greyscale) or four-component (CMYK) JPEG was drawn against the
wrong colour space. This counts, per track, the documents that carry one.

Unlike a markup census this one is exact for both halves of a track: it scans the whole file for
JPEG frame headers, so a zip container and an OLE2 container are read the same way. What it cannot
say is whether the picture is ever *drawn* — a document may carry an unused image — so read it as a
ceiling.
"""
import os, struct, sys, collections

def components(d, at):
    i = at + 2
    while i + 3 < len(d):
        if d[i] != 0xFF: return None
        m = d[i+1]
        if m == 0xFF: i += 1; continue
        if m == 0x01 or 0xD0 <= m <= 0xD9: i += 2; continue
        if m == 0xDA: return None
        ln = struct.unpack('>H', d[i+2:i+4])[0]
        if ln < 2: return None
        if 0xC0 <= m <= 0xCF and m not in (0xC4, 0xC8, 0xCC):
            return d[i+9] if i + 9 < len(d) else None
        i += 2 + ln
    return None

def scan(path):
    d = open(path, 'rb').read()
    found = collections.Counter()
    at = d.find(b'\xff\xd8\xff')
    while at >= 0:
        n = components(d, at)
        if n: found[n] += 1
        at = d.find(b'\xff\xd8\xff', at + 3)
    return found

root = sys.argv[1]
for track in ('words', 'slides', 'sheets'):
    total = odd = 0
    kinds = collections.Counter()
    base = os.path.join(root, track)
    for dirpath, _, files in os.walk(base):
        for f in sorted(files):
            total += 1
            found = scan(os.path.join(dirpath, f))
            bad = sum(v for k, v in found.items() if k in (1, 4))
            if bad:
                odd += 1
                for k, v in found.items():
                    if k in (1, 4): kinds[k] += v
    print(f'{track:8s} {odd:4d} of {total:4d} documents carry a 1- or 4-component JPEG'
          f'  ({dict(kinds)} images by component count)')
