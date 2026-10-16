#!/usr/bin/env python3
"""How far a bank's renderings draw outside their own page — the coarse witness for
unclipped chart geometry. Reports one row per document that leaves the page by >1 pt."""
import pathlib, sys
import pymupdf
root = pathlib.Path(sys.argv[1])
rows = []
for d in sorted(root.iterdir()):
    if not d.is_dir(): continue
    name = (d / 'name.txt').read_text().strip()
    for pdf in sorted(d.glob('*.pdf')):
        doc = pymupdf.open(pdf)
        worst = 0.0; page = 0
        for i, pg in enumerate(doc):
            w, h = pg.rect.width, pg.rect.height
            for dr in pg.get_drawings():
                for it in dr['items']:
                    pts = []
                    if it[0] == 'l': pts = [it[1], it[2]]
                    elif it[0] == 'c': pts = list(it[1:5])
                    elif it[0] == 're': r = it[1]; pts = [pymupdf.Point(r.x0, r.y0), pymupdf.Point(r.x1, r.y1)]
                    elif it[0] == 'qu': q = it[1]; pts = [q.ul, q.ur, q.ll, q.lr]
                    for p in pts:
                        over = max(-p.x, -p.y, p.x - w, p.y - h)
                        if over > worst: worst, page = over, i + 1
        if worst > 1.0:
            rows.append((worst, name, page))
rows.sort(reverse=True)
for worst, name, page in rows:
    print(f'{worst:10.2f} pt  p{page:<4d} {name}')
print(f'# {len(rows)} of the bank leave their page by more than 1 pt')
