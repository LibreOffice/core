#!/usr/bin/env python3
"""What the unread `a:blipFill` costs on the page, per workbook.

    blipfill-drawn.py <ours-root> <ref-bank> <blipfill.tsv>

For each workbook whose worksheet shape states a picture as its own fill, report the
reference's image placements, ours, and the area of every placement the reference makes
that we do not -- and whether this tree paints ANYTHING inside that rectangle, which is
what separates "the fill is wrong" from "the shape is not drawn at all".
"""
import pathlib
import sys

import pymupdf

ours_root, refbank, census = sys.argv[1:4]
print('document\timgs_ours\timgs_ref\tmissing\tarea_pt2\tour_marks_inside')
tot = [0, 0, 0.0]
for line in pathlib.Path(census).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    if p[1] not in ('xlsx', 'xlsm') or int(p[3]) == 0:
        continue
    stem = pathlib.PurePosixPath(p[0]).stem
    o = pathlib.Path(ours_root) / stem / f'{stem}.pdf'
    r = pathlib.Path(refbank) / f'{stem}__{p[1]}.pdf'
    if not o.exists() or not r.exists():
        print(f'{stem}\tMISSING')
        continue
    do, dr = pymupdf.open(o), pymupdf.open(r)
    io = sum(len(pg.get_image_info()) for pg in do)
    boxes = []
    for i, pg in enumerate(dr):
        for inf in pg.get_image_info():
            boxes.append((i, inf['bbox']))
    ir = len(boxes)
    # the reference's placements with no placement of ours within 2 pt on the same page
    missing, area, inside = 0, 0.0, 0
    for i, b in boxes:
        if i >= do.page_count:
            continue
        mine = [x['bbox'] for x in do[i].get_image_info()]
        if any(max(abs(m[k] - b[k]) for k in range(4)) <= 2.0 for m in mine):
            continue
        missing += 1
        area += abs(b[2] - b[0]) * abs(b[3] - b[1])
        rect = pymupdf.Rect(b).round()
        for d in do[i].get_drawings():
            if pymupdf.Rect(d['rect']).intersects(rect):
                inside += 1
        for w in do[i].get_text('words'):
            if pymupdf.Rect(w[:4]).intersects(rect):
                inside += 1
    print(f'{stem[:52]}\t{io}\t{ir}\t{missing}\t{area:.0f}\t{inside}', flush=True)
    tot[0] += io
    tot[1] += ir
    tot[2] += area
    do.close()
    dr.close()
print(f'TOTAL\t{tot[0]}\t{tot[1]}\t\t{tot[2]:.0f}')
