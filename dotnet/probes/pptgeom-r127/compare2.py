"""Second pass: our preset expanded at the Escher default RESCALED by 100000/21600.

If a preset's only disagreement was which default each vocabulary states, this closes it and the
plain rescale is the right conversion for that preset.  If it does not close, either the
adjustment means a different quantity in the two vocabularies or the path templates differ.
"""
import json
import numpy as np
from compare import parse, sample, chamfer, TRANSFORMS, TOL   # reuse the instrument

cells = json.load(open('cells.json'))
ref   = json.load(open('ref_paths.json'))
own   = [l.rstrip('\n').split('\t') for l in open('ours_rescaled.tsv')]
base  = {}
for l in open('preset-census.tsv'):
    f = l.rstrip('\n').split('\t')
    if f[0] != 'mso_spt': base[int(f[0])] = f

rows = []
for c, o in zip(cells, own):
    spt = c['spt']
    if str(spt) not in ref:
        rows.append((spt, c['preset'], 'no-escher-geometry', '', '')); continue
    rp = []
    for d in ref[str(spt)]: rp += parse(d)
    ra = sample(rp, c['x'], c['y'], 4000.0)
    oa = sample(parse(o[1]), 0.0, 0.0, 4000.0)
    if ra is None or oa is None:
        rows.append((spt, c['preset'], 'empty', '', '')); continue
    scored = {t: chamfer(ra, fn(oa))[0] for t, fn in TRANSFORMS.items()}
    ident = scored['identity']
    bestm = min(('mirror-V', 'mirror-H', 'rotate-180'), key=lambda t: scored[t])
    if abs(scored['mirror-V'] - scored[bestm]) < 1e-6: bestm = 'mirror-V'
    verdict = ('identical' if ident <= TOL
               else bestm if scored[bestm] <= TOL
               else 'differs')
    rows.append((spt, c['preset'], verdict, f'{ident:.4f}', f'{scored[bestm]:.4f}'))

with open('preset-census-rescaled.tsv', 'w') as f:
    f.write('mso_spt\tpreset\tverdict_own_defaults\tverdict_rescaled\tchamfer_identity\tbest_mirror\n')
    for r in rows:
        f.write(f'{r[0]}\t{r[1]}\t{base.get(r[0], ["", "", "", "?"])[3]}\t{r[2]}\t{r[3]}\t{r[4]}\n')

from collections import Counter
print('rescaled:', Counter(r[2] for r in rows))
moved = [r for r in rows if base.get(r[0], ['', '', '', ''])[3] != 'identical' and r[2] == 'identical']
print('closed by the plain rescale:', len(moved))
print([ (r[0], r[1]) for r in moved ])
