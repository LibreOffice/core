#!/usr/bin/env python3
"""Filled rectangles on one page of two PDFs, side by side, with their colours."""
import sys, pymupdf, collections
a,b,pno=sys.argv[1],sys.argv[2],int(sys.argv[3])
for tag,path in (('ours',a),('ref',b)):
    d=pymupdf.open(path); p=d[pno-1]
    out=[]
    for dr in p.get_drawings():
        if dr['type'] not in ('f','fs'): continue
        r=dr['rect']
        col=dr.get('fill')
        out.append((round(r.x0,1),round(r.y0,1),round(r.x1,1),round(r.y1,1),
                    tuple(round(c,3) for c in col) if col else None, round(r.get_area(),1)))
    print('== %s: %d fills, area %.1f'%(tag,len(out),sum(o[5] for o in out)))
    byc=collections.Counter()
    for o in out: byc[o[4]]+=o[5]
    for c,ar in byc.most_common(10): print('    %-28s area %.1f'%(str(c),ar))
    for o in sorted(out,key=lambda o:-o[5])[:12]:
        print('    %8.1f %8.1f %8.1f %8.1f  %-24s %.1f'%o)
    d.close()
