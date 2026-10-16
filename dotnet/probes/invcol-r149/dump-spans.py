#!/usr/bin/env python3
"""Dump every span of a PDF: page, y0, x0, x1, width, font, size, text.

Deliberately does NOT merge, pair or sort anything -- it is the raw record the
pairing instruments are checked against.
"""
import sys, glob, pymupdf
f = glob.glob(sys.argv[1])[0]
d = pymupdf.open(f)
print('\t'.join(['page','y0','y1','x0','x1','w','font','size','text']))
for pn, pg in enumerate(d):
    for b in pg.get_text('dict')['blocks']:
        for l in b.get('lines', []):
            for s in l['spans']:
                t = s['text'].strip()
                if not t: continue
                x0, y0, x1, y1 = s['bbox']
                print('\t'.join([str(pn), f'{y0:.2f}', f'{y1:.2f}', f'{x0:.2f}', f'{x1:.2f}',
                                 f'{x1-x0:.2f}', s['font'], f"{s['size']:.2f}", t]))
