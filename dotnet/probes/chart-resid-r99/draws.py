import sys, pymupdf
doc = pymupdf.open(sys.argv[1]); pg = doc[int(sys.argv[2])-1]
lo = float(sys.argv[3]) if len(sys.argv)>3 else -1e9
hi = float(sys.argv[4]) if len(sys.argv)>4 else 1e9
seen=set()
for d in pg.get_drawings():
    r=d['rect']; k=(round(r.x0,2),round(r.y0,2),round(r.x1,2),round(r.y1,2),d['type'])
    if k in seen: continue
    seen.add(k)
    if r.x1 < lo or r.x0 > hi: continue
    print(f"{r.x0:9.2f} {r.y0:8.2f} {r.x1:9.2f} {r.y1:8.2f}  w={r.x1-r.x0:7.2f} h={r.y1-r.y0:7.2f} {d['type']} fill={d.get('fill')} col={d.get('color')}")
