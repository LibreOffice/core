#!/usr/bin/env python3
"""How much ink an `iconSet` rule is actually worth, from the reference's own renderings.

An icon is a bitmap `drawIconSets` (`sc/source/ui/view/output.cxx`:960-989) draws into the cell's
bottom-left corner, so it is the only conditional format in the file that arrives in a PDF as an
*image*. Counting the small images in 26.2.4.2's rendering of each document that states such a
rule is therefore an exact reach figure for the family, and it needs no comparison against our
side at all.

**It is not ten points square, and this script's own output is what shows it.** The ten points at
`output.cxx`:967 is a fallback for a null `mnHeight`, and `GetIconSetInfo` always sets that field
from the cell's own `ATTR_FONT_HEIGHT` (`colorscale.cxx`:1222-1224), so the branch at `:969-980`
always wins and the icon is as tall as the cell's font. Hence the per-icon areas below range from
19.4 pt² on `066_Agile_Gantt_chart` to 103.9 pt² on `069_Blue_modern_balance_sheet` rather than
sitting at 100 pt² — divide the area by the count before believing any fixed-size claim.

Reads the banked gate reference at /home/user/gate-orig-r83/ref/.

Usage: icon-ink.py <identity-without-__xlsx> ...
"""
import os
import sys

import pymupdf

total = 0
for name in sys.argv[1:]:
    path = f'/home/user/gate-orig-r83/ref/{name}__xlsx.pdf'
    if not os.path.exists(path):
        print(f'{name} MISSING')
        continue
    doc = pymupdf.open(path)
    icons = 0
    area = 0.0
    for page in doc:
        for info in page.get_image_info():
            # A conditional-format icon is 16 x 16 in the file; anything larger is a picture.
            if info['width'] <= 32 and info['height'] <= 32:
                icons += 1
                box = info['bbox']
                area += (box[2] - box[0]) * (box[3] - box[1])
    print(f'{name[:56]:56} pages {len(doc):4d}  icons {icons:3d}  area {area:8.1f} pt2')
    total += icons
print('total icons the reference draws:', total)
