"""Per-preset verdict on the plain rescale `value x 100000/21600`, measured at three values of
DFF_Prop_adjustValue against the reference's own drawn coordinates."""
import json
from compare import parse, sample, chamfer, TRANSFORMS, TOL

cells={c['spt']:c for c in json.load(open('cells.json'))}
present=json.load(open('present.json'))
vary=json.load(open('vary_paths.json'))
base={}
for l in open('preset-census.tsv'):
    f=l.rstrip('\n').split('\t')
    if f[0]!='mso_spt': base[int(f[0])]=f[3]

ours={}
for v in (3000,8000,15000):
    ours[v]=[l.rstrip('\n').split('\t') for l in open(f'ours_vary{v}.tsv')]

rows=[]
for i,spt in enumerate(present):
    c=cells[spt]
    ids=[]; mirs=[]
    ok=True
    for v in (3000,8000,15000):
        rp=[]
        for d in vary[str(v)].get(str(spt), []): rp+=parse(d)
        ra=sample(rp, c['x'], c['y'], 4000.0)
        oa=sample(parse(ours[v][i][1]), 0.0, 0.0, 4000.0)
        if ra is None or oa is None: ok=False; break
        sc={t: chamfer(ra, fn(oa))[0] for t,fn in TRANSFORMS.items()}
        ids.append(sc['identity'])
        mirs.append(min(sc['mirror-V'], sc['mirror-H'], sc['rotate-180']))
    if not ok:
        rows.append((spt, c['preset'], base[spt], 'empty', '', '')); continue
    worst_id=max(ids); worst_mir=max(mirs)
    if worst_id<=TOL: verdict='rescale-correct'
    elif worst_mir<=TOL: verdict='rescale-correct-but-mirrored'
    else: verdict='rescale-wrong'
    rows.append((spt, c['preset'], base[spt], verdict,
                 ','.join(f'{x:.4f}' for x in ids), ','.join(f'{x:.4f}' for x in mirs)))

with open('adjust-conversion.tsv','w') as f:
    f.write('mso_spt\tpreset\tshape_verdict\tconversion_verdict\tchamfer_identity_3000_8000_15000\tchamfer_bestmirror\n')
    for r in rows: f.write('\t'.join(str(x) for x in r)+'\n')

from collections import Counter
print(Counter(r[3] for r in rows))
print('among the 92 shape-identical presets:', Counter(r[3] for r in rows if r[2]=='identical'))
