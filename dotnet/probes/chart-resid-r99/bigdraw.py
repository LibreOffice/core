import sys, pymupdf
d=pymupdf.open(sys.argv[1]); p=d[int(sys.argv[2])-1]
rows=[]
seen=set()
for g in p.get_drawings():
    r=g['rect']; k=(round(r.x0,2),round(r.y0,2),round(r.x1,2),round(r.y1,2),g['type'])
    if k in seen: continue
    seen.add(k)
    rows.append(((r.x1-r.x0)*(r.y1-r.y0), k, g.get('fill')))
rows.sort(reverse=True)
for a,k,f in rows[:6]:
    print(f"  area={a:10.1f} {k}  w={k[2]-k[0]:7.2f} h={k[3]-k[1]:7.2f} fill={f}")
