"""Classify each name-mapped preset: is the Escher shape 26.2.4.2 draws the same shape as the
DrawingML preset of that name, a mirror of it, or something else?

Both sides are expanded in a SQUARE box, so `ss = min(w,h)` equals both edges and the two
vocabularies' view-box anisotropy cannot enter the comparison.  Each side is resampled to points
in box-fractions and the two clouds compared by symmetric Chamfer distance under four rigid
transforms.
"""
import json, math, re, sys
import numpy as np

STEP = 0.004          # resample step, in box fractions
TOL  = 0.010          # a match is a mean Chamfer distance within 1% of the box side

def tokens(d):
    return re.findall(r'([MLCQZmlcqz])|(-?\d+(?:\.\d+)?)', d)

def parse(d):
    """SVG subset -> list of polylines (flattened), in the path's own units."""
    toks = re.findall(r'[MLCQZmlcqz]|-?\d+(?:\.\d+)?', d)
    i, cmd, cur, start = 0, None, (0.0, 0.0), (0.0, 0.0)
    subs, poly = [], []
    def flush():
        nonlocal poly
        if len(poly) > 1: subs.append(poly)
        poly = []
    while i < len(toks):
        t = toks[i]
        if t.isalpha():
            cmd = t; i += 1
            if cmd in 'Zz':
                if poly: poly.append(start)
                flush(); cur = start
            continue
        if cmd in ('M', 'm'):
            x, y = float(toks[i]), float(toks[i+1]); i += 2
            flush(); cur = start = (x, y); poly = [cur]; cmd = 'L' if cmd == 'M' else 'l'
        elif cmd in ('L', 'l'):
            x, y = float(toks[i]), float(toks[i+1]); i += 2
            if not poly: poly = [cur]
            cur = (x, y); poly.append(cur)
        elif cmd in ('C', 'c'):
            p1 = (float(toks[i]), float(toks[i+1])); p2 = (float(toks[i+2]), float(toks[i+3]))
            p3 = (float(toks[i+4]), float(toks[i+5])); i += 6
            if not poly: poly = [cur]
            for k in range(1, 17):
                s = k / 16.0; u = 1 - s
                poly.append((u*u*u*cur[0] + 3*u*u*s*p1[0] + 3*u*s*s*p2[0] + s*s*s*p3[0],
                             u*u*u*cur[1] + 3*u*u*s*p1[1] + 3*u*s*s*p2[1] + s*s*s*p3[1]))
            cur = p3
        else:
            i += 1
    flush()
    return subs

def sample(subs, ox, oy, side):
    pts = []
    for poly in subs:
        p = [((x - ox) / side, (y - oy) / side) for x, y in poly]
        for a, b in zip(p, p[1:]):
            dx, dy = b[0] - a[0], b[1] - a[1]
            n = max(1, int(math.hypot(dx, dy) / STEP))
            for k in range(n):
                s = k / n
                pts.append((a[0] + dx*s, a[1] + dy*s))
        if p: pts.append(p[-1])
    if not pts: return None
    a = np.array(pts, dtype=np.float64)
    if len(a) > 4000:
        a = a[np.linspace(0, len(a)-1, 4000).astype(int)]
    return a

def chamfer(a, b):
    d = np.sqrt(((a[:, None, :] - b[None, :, :]) ** 2).sum(-1))
    return float((d.min(1).mean() + d.min(0).mean()) / 2), float(max(d.min(1).max(), d.min(0).max()))

TRANSFORMS = {
    'identity':  lambda p: p,
    'mirror-V':  lambda p: np.stack([p[:, 0], 1 - p[:, 1]], 1),
    'mirror-H':  lambda p: np.stack([1 - p[:, 0], p[:, 1]], 1),
    'rotate-180':lambda p: np.stack([1 - p[:, 0], 1 - p[:, 1]], 1),
}

cells = json.load(open('cells.json'))
ref   = json.load(open('ref_paths.json'))
ours = {}
for line in open('ours_paths.tsv'):
    f = line.rstrip('\n').split('\t')
    ours[f[0]] = f[1]          # the STROKED subpaths only: the reference leg is a stroke-only render

rows = []
for c in cells:
    spt, preset = c['spt'], c['preset']
    key = str(spt)
    if key not in ref:
        rows.append((spt, preset, c['odf'], 'no-escher-geometry', '', '', '', ''))
        continue
    rp = []
    for d in ref[key]: rp += parse(d)
    ra = sample(rp, c['x'], c['y'], 4000.0)
    oa = sample(parse(ours.get(preset, '')), 0.0, 0.0, 4000.0)
    if ra is None or oa is None:
        rows.append((spt, preset, c['odf'], 'empty', '', '', '', '')); continue
    scored = {}
    for tname, fn in TRANSFORMS.items():
        scored[tname] = chamfer(ra, fn(oa))
    ident = scored['identity'][0]
    mirrors = ('mirror-V', 'mirror-H', 'rotate-180')
    bestm = min(mirrors, key=lambda t: scored[t][0])
    # A vertical mirror and a 180 turn are the same map on a left-right symmetric shape, so a tie
    # is reported as the simpler of the two rather than by float noise.
    if abs(scored['mirror-V'][0] - scored[bestm][0]) < 1e-6: bestm = 'mirror-V'
    if ident <= TOL:
        verdict = 'identical'
    elif scored[bestm][0] <= TOL:
        verdict = bestm
    elif scored[bestm][0] * 3 <= ident:
        verdict = bestm + '-approx'
    else:
        verdict = 'differs'
    rows.append((spt, preset, c['odf'], verdict, f'{ident:.4f}',
                 f'{scored["mirror-V"][0]:.4f}', f'{scored["mirror-H"][0]:.4f}',
                 f'{scored["rotate-180"][0]:.4f}'))

with open('preset-census.tsv', 'w') as f:
    f.write('mso_spt\tpreset\todf_type\tverdict\tchamfer_identity\tchamfer_mirrorV\tchamfer_mirrorH\tchamfer_rot180\n')
    for r in rows: f.write('\t'.join(str(x) for x in r) + '\n')

from collections import Counter
print(Counter(r[3] for r in rows))
