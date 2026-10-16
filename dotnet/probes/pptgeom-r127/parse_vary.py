import re, json, sys
cells=json.load(open('cells.json'))
def parse_svg(path, present):
    s=open(path).read()
    page=s[s.index('class="SlideGroup"'):]
    groups=re.split(r'<g class="com\.sun\.star\.drawing\.CustomShape">', page)[1:]
    out={}
    for g in groups:
        bb=re.search(r'<rect class="BoundingBox"[^>]*x="(-?[\d.]+)" y="(-?[\d.]+)" width="([\d.]+)" height="([\d.]+)"', g)
        if not bb: continue
        cx=float(bb.group(1))+float(bb.group(3))/2
        best=min(present, key=lambda c: abs(c['x']+2000-cx))
        if abs(best['x']+2000-cx)>12000: continue
        ds=re.findall(r'<path[^>]*\sd="([^"]*)"', g)
        for (x1,y1,x2,y2) in re.findall(r'<line[^>]*x1="(-?[\d.]+)"[^>]*y1="(-?[\d.]+)"[^>]*x2="(-?[\d.]+)"[^>]*y2="(-?[\d.]+)"', g):
            ds.append(f'M {x1},{y1} L {x2},{y2} ')
        out.setdefault(best['spt'], []).extend(ds)
    return out
mods=json.load(open('ref_modifiers.json'))
present=[c for c in cells if mods[str(c['spt'])][1]]
res={}
for v in (3000,8000,15000):
    res[str(v)]=parse_svg(f'out/vary{v}.svg', present)
json.dump(res, open('vary_paths.json','w'))
print('types with adjustments:', len(present), 'parsed:', {k:len(v) for k,v in res.items()})
# our side inputs
for v in (3000,8000,15000):
    lines=[]
    for c in present:
        vals=mods[str(c['spt'])][1].split(); vals[0]=str(v)
        lines.append(f"{c['preset']}\t"+','.join(f'{float(x)*100000/21600:.4f}' for x in vals))
    open(f'names_vary{v}.txt','w').write('\n'.join(lines)+'\n')
json.dump([c['spt'] for c in present], open('present.json','w'))
