"""Does the extra factor w/ss close the four `rescale-correct-square-only` presets and the
trapezoid?  Same instrument, wide box only, with the value scaled by `w/ss` as well."""
import json, os, re, subprocess
import numpy as np
from compare import parse, sample as _sample, chamfer, TRANSFORMS, TOL

def sample(subs, ox, oy, w, h, cap=700):
    scaled=[[((x-ox)/w,(y-oy)/h) for x,y in poly] for poly in subs]
    a=_sample(scaled,0.0,0.0,1.0)
    return None if a is None else a[np.linspace(0,len(a)-1,min(cap,len(a))).astype(int)]

cells={c['spt']:c for c in json.load(open('cells.json'))}
present=json.load(open('present.json'))
mods=json.load(open('ref_modifiers.json'))
CAND=[7,8,9,23,57]
W,H=8000,4000
FACTOR=W/min(W,H)
CLI=['dotnet','run','--project','/home/user/wt-pptgeom/dotnet/probes/pptgeom-r127/PresetDump/PresetDump.csproj','--no-build','--']

def parse_svg(path,w):
    s=open(path).read(); page=s[s.index('class="SlideGroup"'):]
    out={}
    for g in re.split(r'<g class="com\.sun\.star\.drawing\.CustomShape">', page)[1:]:
        bb=re.search(r'<rect class="BoundingBox"[^>]*x="(-?[\d.]+)" y="(-?[\d.]+)" width="([\d.]+)" height="([\d.]+)"', g)
        if not bb: continue
        cx=float(bb.group(1))+float(bb.group(3))/2
        best=min(present,key=lambda s2: abs(cells[s2]['x']+w/2-cx))
        if abs(cells[best]['x']+w/2-cx)>12000: continue
        ds=re.findall(r'<path[^>]*\sd="([^"]*)"',g)
        out.setdefault(best,[]).extend(ds)
    return out

for v in (2000,5000,9000):
    lines=[]
    for spt in present:
        vals=mods[str(spt)][1].split(); vals[0]=str(v)
        conv=[float(x)*100000/21600 for x in vals]
        if spt in CAND: conv[0]*=FACTOR
        lines.append(f"{cells[spt]['preset']}\t"+','.join(f'{x:.4f}' for x in conv))
    open(f'in_aw{v}.txt','w').write('\n'.join(lines)+'\n')
    if not os.path.exists(f'ours_aw{v}.tsv'):
        with open(f'ours_aw{v}.tsv','w') as fh:
            subprocess.run(CLI+[os.path.abspath(f'in_aw{v}.txt'),str(W),str(H)],stdout=fh,check=True)

refs={v: parse_svg(f'out/vwide{v}.svg',W) for v in (2000,5000,9000)}
ours={v: [l.rstrip('\n').split('\t') for l in open(f'ours_aw{v}.tsv')] for v in (2000,5000,9000)}
print(f'{"spt":<5}{"preset":<18}{"identity(2000,5000,9000)":<30}best-mirror')
for i,spt in enumerate(present):
    if spt not in CAND: continue
    c=cells[spt]; ids=[]; mirs=[]
    for v in (2000,5000,9000):
        rp=[]
        for d in refs[v].get(spt,[]): rp+=parse(d)
        ra=sample(rp,c['x'],1000,W,H); oa=sample(parse(ours[v][i][1]),0,0,W,H)
        sc={t: chamfer(ra,fn(oa))[0] for t,fn in TRANSFORMS.items()}
        ids.append(sc['identity']); mirs.append(min(sc['mirror-V'],sc['mirror-H'],sc['rotate-180']))
    print(f'{spt:<5}{c["preset"]:<18}{",".join(f"{x:.4f}" for x in ids):<30}{",".join(f"{x:.4f}" for x in mirs)}')
