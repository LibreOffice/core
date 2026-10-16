#!/usr/bin/env python3
"""Read the pool probes: HEAD's size, face, x and the gaps either side of it.

  readpool.py <dir of rendered pdfs> [<second dir>]

Prints one row per probe: size, font, left edge, and the AAA->HEAD and HEAD->BBB
baseline gaps, which carry the pool's space above and below.
"""
import pathlib
import sys

import pymupdf


def read(path):
    page = pymupdf.open(path)[0]
    spans = {}
    for block in page.get_text('dict')['blocks']:
        for line in block.get('lines', []):
            for span in line['spans']:
                text = span['text'].strip()
                if text:
                    spans[text] = span
    if 'HEAD' not in spans:
        return None
    head = spans['HEAD']
    row = {
        'size': round(head['size'], 2),
        'font': head['font'],
        'x': round(head['bbox'][0], 2),
        'italic': 'talic' in head['font'] or 'Oblique' in head['font'],
    }
    if 'AAA' in spans:
        row['above'] = round(head['bbox'][3] - spans['AAA']['bbox'][3], 2)
    if 'BBB' in spans:
        row['below'] = round(spans['BBB']['bbox'][3] - head['bbox'][3], 2)
    return row


dirs = [pathlib.Path(d) for d in sys.argv[1:]] or [pathlib.Path('.')]
names = sorted({p.stem for d in dirs for p in d.glob('p_*.pdf')})
width = max(len(n) for n in names) if names else 10

for name in names:
    cells = []
    for d in dirs:
        path = d / f'{name}.pdf'
        row = read(path) if path.exists() else None
        cells.append('-' if row is None else
                     f"{row['size']:>5} {row['font']:<22} x={row['x']:<6} "
                     f"above={row.get('above', '-'):<7} below={row.get('below', '-'):<7}")
    print(f"{name:<{width}}  " + " | ".join(cells))
