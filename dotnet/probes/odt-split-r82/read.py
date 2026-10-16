#!/usr/bin/env python3
"""Reads the probe renderings: every text run's origin, direction and content."""
import sys, pathlib
import fitz

for pdf in sorted(pathlib.Path(sys.argv[1]).glob('*.pdf')):
    doc = fitz.open(pdf)
    print(f'== {pdf.stem}  pages={doc.page_count}')
    for pno, page in enumerate(doc):
        d = page.get_text('dict')
        for block in d['blocks']:
            for line in block.get('lines', []):
                txt = ''.join(s['text'] for s in line['spans'])
                if not txt.strip():
                    continue
                x0, y0, x1, y1 = line['bbox']
                print(f'   p{pno} dir={line["dir"][0]:+.3f},{line["dir"][1]:+.3f} '
                      f'bbox=({x0:7.2f},{y0:7.2f})-({x1:7.2f},{y1:7.2f}) {txt!r}')
    doc.close()
