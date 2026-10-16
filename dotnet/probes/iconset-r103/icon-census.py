#!/usr/bin/env python3
"""Icon for icon, both sides: what 26.2.4.2 draws and what this tree draws.

The reference draws each icon as a 16x16 bitmap, identified here by the SHA-1 of its composited
RGBA pixels (see reference-glyphs.py). This tree draws the same glyph as vector paths, so it is
recovered by colour: every path painted in one of the colibre palette's eight colours is
clustered with its touching neighbours, and the set of palette colours in a cluster names the
glyph -- `pole+red` is a red flag, a bare `red` a diamond, `red+white` a cross in a red disc.

Usage: icon-census.py <ours-root> <identity-without-__xlsx> ...
"""
import collections
import hashlib
import os
import sys

import pymupdf

ASSET = {'f892a0427061': 'flag-red', '6246434f64a7': 'flag-amber', 'f8365039aff6': 'flag-green',
         'dbb047b02069': 'diamond', '5da002bf1b32': 'tick-green', 'e604cb94bfd8': 'cross-red',
         '43175f417030': 'excl-amber'}

PALETTE = {(0xD4, 0x23, 0x14): 'red', (0xFF, 0x91, 0x98): 'red',
           (0xED, 0x87, 0x33): 'amber', (0xF8, 0xDB, 0x8F): 'amber',
           (0x30, 0x90, 0x48): 'green', (0xA1, 0xDD, 0xAA): 'green',
           (0x3A, 0x3A, 0x38): 'pole', (0xFA, 0xFA, 0xFA): 'white'}

OURS = {'pole+red': 'flag-red', 'amber+pole': 'flag-amber', 'green+pole': 'flag-green',
        'red': 'diamond', 'green+white': 'tick-green', 'red+white': 'cross-red',
        'amber+white': 'excl-amber'}


def classify(colour):
    if colour is None:
        return None
    r, g, b = (int(round(v * 255)) for v in colour[:3])
    for key, name in PALETTE.items():
        if abs(r - key[0]) + abs(g - key[1]) + abs(b - key[2]) <= 6:
            return name
    return None


def reference(path):
    counts = collections.Counter()
    doc = pymupdf.open(path)
    for page in doc:
        for info in page.get_image_info(xrefs=True):
            if info['width'] > 32 or info['height'] > 32:
                continue
            pix = pymupdf.Pixmap(doc, info['xref'])
            smask = doc.extract_image(info['xref']).get('smask', 0)
            if smask:
                pix = pymupdf.Pixmap(pix, pymupdf.Pixmap(doc, smask))
            digest = hashlib.sha1(pix.tobytes('png')).hexdigest()[:12]
            counts[ASSET.get(digest, digest)] += 1
    return counts


def ours(path):
    counts = collections.Counter()
    doc = pymupdf.open(path)
    for page in doc:
        boxes = []
        for drawing in doc[page.number].get_drawings():
            name = classify(drawing.get('fill')) or classify(drawing.get('color'))
            if name:
                boxes.append((tuple(drawing['rect']), name))
        clusters = []
        for box, name in sorted(boxes, key=lambda t: (t[0][1], t[0][0])):
            for cluster in clusters:
                r = cluster[0]
                if not (box[0] > r[2] + 0.05 or box[2] < r[0] - 0.05
                        or box[1] > r[3] + 0.05 or box[3] < r[1] - 0.05):
                    cluster[0] = [min(r[0], box[0]), min(r[1], box[1]),
                                  max(r[2], box[2]), max(r[3], box[3])]
                    cluster[1].add(name)
                    break
            else:
                clusters.append([list(box), {name}])
        for cluster in clusters:
            key = '+'.join(sorted(cluster[1]))
            if key == 'white':
                continue          # not an icon: page furniture in the mark colour
            counts[OURS.get(key, key)] += 1
    return counts


root = sys.argv[1]
print(f'{"document":58} {"26.2.4.2":34} {"ours":34}')
for name in sys.argv[2:]:
    ref = reference(f'/home/user/gate-orig-r83/ref/{name}__xlsx.pdf')
    our = ours(os.path.join(root, name, f'{name}.pdf'))
    mark = '=' if ref == our else '!'
    print(f'{mark} {name[:56]:56} {sum(ref.values()):3d} '
          f'{", ".join(f"{k} x{v}" for k, v in sorted(ref.items())) or "(none)":30} '
          f'{sum(our.values()):3d} '
          f'{", ".join(f"{k} x{v}" for k, v in sorted(our.items())) or "(none)"}')
