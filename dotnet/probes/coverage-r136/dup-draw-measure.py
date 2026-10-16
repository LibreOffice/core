#!/usr/bin/env python3
"""Measure what the duplicated `w:object` replacement picture actually costs, on the page.

    dup-draw-measure.py <ours-root> <ref-bank> <objdup.tsv>

For every document holding a `w:object`, pair our rendering with the banked 26.2.4.2 one and
report, per document: how many image placements each side draws, how many of OURS are a second
draw of the same XObject within 1 pt of another draw of it (the shape the defect makes), and
the character difference.  The count of placements is read from `page.get_image_info()`; round
128 established that the BOX it reports is the declared placement rectangle and is not
comparable across the two sides when one replays a metafile and the other rasterises it, so
only the count and the OUR-side self-comparison are used.
"""
import pathlib
import sys

import pymupdf

ours_root, refbank, census = sys.argv[1:4]
print('document\tpg\timgs_ours\timgs_ref\tdup_pairs\tdup_area_pt2\tchars_ours\tchars_ref')
tot = [0, 0, 0.0, 0]
for line in pathlib.Path(census).read_text(encoding='utf-8').splitlines()[1:]:
    rel = line.split('\t')[0]
    stem = pathlib.PurePosixPath(rel).stem
    ext = pathlib.PurePosixPath(rel).suffix[1:]
    o = pathlib.Path(ours_root) / stem / f'{stem}.pdf'
    r = pathlib.Path(refbank) / f'{stem}__{ext}.pdf'
    if not o.exists() or not r.exists():
        print(f'{stem}\t-\tMISSING', flush=True)
        continue
    do, dr = pymupdf.open(o), pymupdf.open(r)
    io = ir = dp = 0
    area = 0.0
    co = cr = 0
    for i in range(max(do.page_count, dr.page_count)):
        if i < do.page_count:
            p = do[i]
            infos = p.get_image_info(xrefs=True)
            io += len(infos)
            co += len(''.join(p.get_text('text').split()))
            for a in range(len(infos)):
                for b in range(a + 1, len(infos)):
                    x, y = infos[a], infos[b]
                    if x.get('xref') != y.get('xref'):
                        continue
                    bx, by = x['bbox'], y['bbox']
                    if max(abs(bx[k] - by[k]) for k in range(4)) <= 1.0:
                        dp += 1
                        area += abs(bx[2] - bx[0]) * abs(bx[3] - bx[1])
        if i < dr.page_count:
            p = dr[i]
            ir += len(p.get_image_info())
            cr += len(''.join(p.get_text('text').split()))
    print(f'{stem}\t{do.page_count}/{dr.page_count}\t{io}\t{ir}\t{dp}\t{area:.0f}\t{co}\t{cr}',
          flush=True)
    tot[0] += io
    tot[1] += ir
    tot[2] += area
    tot[3] += dp
    do.close()
    dr.close()
print(f'TOTAL\t\t{tot[0]}\t{tot[1]}\t{tot[3]}\t{tot[2]:.0f}')
