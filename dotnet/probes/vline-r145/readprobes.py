#!/usr/bin/env python3
"""Read each probe's marker rectangle and its v:line out of the PDF's path operators."""
import sys, pymupdf, glob, os
def near(c, t, tol=0.02): return c and all(abs(a-b)<tol for a,b in zip(c[:3], t))
print(f'{"probe":<12} {"marker(x,y)":<20} {"line: (x0,y0) -> (x1,y1)":<44} {"len":>8} {"w":>5}')
for f in sorted(glob.glob(sys.argv[1] if len(sys.argv)>1 else 'probes-ref/*.pdf')):
    doc = pymupdf.open(f); name = os.path.basename(f)[:-4]
    mark = None; lines = []
    for page in doc:
        for d in page.get_drawings():
            if d['type'] not in ('s','fs'): continue
            col = d.get('color')
            for it in d['items']:
                if near(col, (1.0,0.0,0.0)):
                    if it[0]=='l': lines.append((it[1].x,it[1].y,it[2].x,it[2].y,d.get('width')))
                    elif it[0]=='re': lines.append(('RECT',it[1].x0,it[1].y0,it[1].x1,it[1].y1,d.get('width')))
                elif near(col, (0.0,0.627,0.0), 0.02) or (col and col[1]>0.4 and col[0]<0.1 and col[2]<0.1):
                    if it[0]=='re': mark=(it[1].x0,it[1].y0,it[1].x1,it[1].y1)
                    elif it[0]=='l': mark=(it[1].x,it[1].y,it[2].x,it[2].y)
    m = f'({mark[0]:.2f},{mark[1]:.2f})' if mark else '(none)'
    if not lines:
        print(f'{name:<12} {m:<20} {"-- NO RED INK --":<44}')
    for L in lines:
        if L[0]=='RECT':
            print(f'{name:<12} {m:<20} rect ({L[1]:.2f},{L[2]:.2f})-({L[3]:.2f},{L[4]:.2f})')
        else:
            x0,y0,x1,y1,w = L
            ln = ((x1-x0)**2+(y1-y0)**2)**.5
            print(f'{name:<12} {m:<20} ({x0:8.2f},{y0:8.2f}) -> ({x1:8.2f},{y1:8.2f}) {ln:8.2f} {w:5.2f}')
