import re, json, sys
cells=json.load(open('cells.json'))
s=open('out/census.svg').read()
page=s[s.index('class="SlideGroup"'):]
groups=re.split(r'<g class="com\.sun\.star\.drawing\.CustomShape">', page)[1:]
out={}
for g in groups:
    bb=re.search(r'<rect class="BoundingBox"[^>]*x="(-?[\d.]+)" y="(-?[\d.]+)" width="([\d.]+)" height="([\d.]+)"', g)
    if not bb: continue
    bx,bw=float(bb.group(1)),float(bb.group(3))
    cx=bx+bw/2
    best=min(cells, key=lambda c: abs(c['x']+2000-cx))
    if abs(best['x']+2000-cx)>12000:
        print('UNMATCHED', bx, file=sys.stderr); continue
    ds=re.findall(r'<path[^>]*\sd="([^"]*)"', g)
    for (x1,y1,x2,y2) in re.findall(r'<line[^>]*x1="(-?[\d.]+)"[^>]*y1="(-?[\d.]+)"[^>]*x2="(-?[\d.]+)"[^>]*y2="(-?[\d.]+)"', g):
        ds.append(f'M {x1},{y1} L {x2},{y2} ')
    for (x1,y1,x2,y2) in re.findall(r'<rect[^>]*class="Border"', g):
        pass
    out.setdefault(best['spt'], []).extend(ds)
json.dump(out, open('ref_paths.json','w'))
print('groups', len(groups), 'shapes with paths', len(out), file=sys.stderr)
for c in cells:
    if c['spt'] not in out: print('MISSING', c['spt'], c['preset'], c['odf'], file=sys.stderr)
