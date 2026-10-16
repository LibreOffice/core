"""Per-preset verdict on `value x 100000/21600`, at three IN-RANGE values of
DFF_Prop_adjustValue (the Escher handle range for a one-handle preset is 0..10800) and at two
aspect ratios -- square, where `ss = w = h`, and 2:1, where `ss = h` and the two vocabularies'
choice of base separates."""
import json, re, sys
import numpy as np
from compare import parse, sample as _sample, chamfer, TRANSFORMS, TOL

def sample(subs, ox, oy, w, h, cap=700):
    scaled=[[((x-ox)/w, (y-oy)/h) for x,y in poly] for poly in subs]
    a=_sample(scaled, 0.0, 0.0, 1.0)
    if a is None: return None
    return a[np.linspace(0, len(a)-1, min(cap, len(a))).astype(int)]

cells={c['spt']:c for c in json.load(open('cells.json'))}
present=json.load(open('present.json'))
mods=json.load(open('ref_modifiers.json'))
VALUES=[2000,5000,9000]
BOXES={'sq':(4000,4000),'wide':(8000,4000)}

def parse_svg(path, w):
    s=open(path).read(); page=s[s.index('class="SlideGroup"'):]
    groups=re.split(r'<g class="com\.sun\.star\.drawing\.CustomShape">', page)[1:]
    out={}
    for g in groups:
        bb=re.search(r'<rect class="BoundingBox"[^>]*x="(-?[\d.]+)" y="(-?[\d.]+)" width="([\d.]+)" height="([\d.]+)"', g)
        if not bb: continue
        cx=float(bb.group(1))+float(bb.group(3))/2
        best=min(present, key=lambda s2: abs(cells[s2]['x']+w/2-cx))
        if abs(cells[best]['x']+w/2-cx)>12000: continue
        ds=re.findall(r'<path[^>]*\sd="([^"]*)"', g)
        for (x1,y1,x2,y2) in re.findall(r'<line[^>]*x1="(-?[\d.]+)"[^>]*y1="(-?[\d.]+)"[^>]*x2="(-?[\d.]+)"[^>]*y2="(-?[\d.]+)"', g):
            ds.append(f'M {x1},{y1} L {x2},{y2} ')
        out.setdefault(best, []).extend(ds)
    return out

import subprocess, os
CLI=['dotnet','run','--project','/home/user/wt-pptgeom/dotnet/probes/pptgeom-r127/PresetDump/PresetDump.csproj','--no-build','--']
def ours_for(tag, v, w, h):
    lines=[]
    for spt in present:
        vals=mods[str(spt)][1].split(); vals[0]=str(v)
        lines.append(f"{cells[spt]['preset']}\t"+','.join(f'{float(x)*100000/21600:.4f}' for x in vals))
    fn=f'in_{tag}{v}.txt'; open(fn,'w').write('\n'.join(lines)+'\n')
    outn=f'ours_{tag}{v}.tsv'
    if not os.path.exists(outn):
        with open(outn,'w') as fh:
            subprocess.run(CLI+[os.path.abspath(fn), str(w), str(h)], stdout=fh, check=True)
    return [l.rstrip('\n').split('\t') for l in open(outn)]

rows=[]
for tag,(w,h) in BOXES.items():
    refs={v: parse_svg(f'out/v{tag}{v}.svg', w) for v in VALUES}
    ours={v: ours_for(tag,v,w,h) for v in VALUES}
    for i,spt in enumerate(present):
        c=cells[spt]; ids=[]; mirs=[]
        for v in VALUES:
            rp=[]
            for d in refs[v].get(spt, []): rp+=parse(d)
            ra=sample(rp, c['x'], 1000, w, h)
            oa=sample(parse(ours[v][i][1]), 0.0, 0.0, w, h)
            if ra is None or oa is None: ids.append(9.0); mirs.append(9.0); continue
            sc={t: chamfer(ra, fn(oa))[0] for t,fn in TRANSFORMS.items()}
            ids.append(sc['identity']); mirs.append(min(sc['mirror-V'],sc['mirror-H'],sc['rotate-180']))
        rows.append((tag,spt,c['preset'],max(ids),max(mirs),ids,mirs))
json.dump([[r[0],r[1],r[2],r[3],r[4]] for r in rows], open('conv_raw.json','w'))
by={}
for tag,spt,preset,wid,wmir,ids,mirs in rows: by.setdefault(spt,{})[tag]=(wid,wmir,ids,mirs)
with open('adjust-conversion.tsv','w') as f:
    f.write('mso_spt\tpreset\tverdict\tworst_identity_square\tworst_identity_wide\tworst_mirror_square\tworst_mirror_wide\tsquare_values\twide_values\n')
    for spt in present:
        s=by[spt]['sq']; wd=by[spt]['wide']
        if s[0]<=TOL and wd[0]<=TOL: verdict='rescale-correct'
        elif s[0]<=TOL and wd[0]>TOL: verdict='rescale-correct-square-only'
        elif s[1]<=TOL and wd[1]<=TOL: verdict='rescale-correct-but-mirrored'
        elif s[1]<=TOL: verdict='mirrored-square-only'
        else: verdict='rescale-wrong'
        f.write(f"{spt}\t{cells[spt]['preset']}\t{verdict}\t{s[0]:.4f}\t{wd[0]:.4f}\t{s[1]:.4f}\t{wd[1]:.4f}\t"
                f"{','.join(f'{x:.4f}' for x in s[2])}\t{','.join(f'{x:.4f}' for x in wd[2])}\n")
from collections import Counter
print(Counter(l.split('\t')[2] for l in open('adjust-conversion.tsv').readlines()[1:]))
