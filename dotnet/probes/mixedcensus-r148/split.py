#!/usr/bin/env python3
"""census-pdf.mixed() split into its shape (a) and shape (b) contributions, with samples."""
import sys, os, types, collections
import pymupdf

R146 = '/home/user/libreoffice-core/dotnet/probes/tablerow-r146'
src = open(os.path.join(R146, 'census-pdf.py')).read()
mod = types.ModuleType('cpdf')
exec(compile(src[:src.index('rows = []')], 'census-pdf.py', 'exec'), mod.__dict__)
TOL = mod.TOL
cover = mod.cover


def mixed_split(H, samples, page):
    g = collections.defaultdict(list)
    for lo, hi, x0, x1, th in H:
        key = round(lo / TOL)
        seg = (round(x0, 2), round(x1, 2), round(th, 3))
        if seg not in g[key]:
            g[key].append(seg)
    keys = sorted(g)
    na = nb = nsame = 0
    flagged = set()
    for k in keys:
        v = sorted(g[k])
        if len(v) >= 2:
            side = all(v[i][1] - v[i + 1][0] <= 1.0 for i in range(len(v) - 1))
            if side and len({t for _, _, t in v}) > 1:
                na += 1; flagged.add(k)
                if len(samples['a']) < 6:
                    samples['a'].append((page, k * TOL, v))
                continue
            if side:
                nsame += 1
    for i, k in enumerate(keys):
        if k in flagged:
            continue
        cv = cover(g[k])
        for k2 in keys[i + 1:]:
            dy = (k2 - k) * TOL
            if dy <= 0 or dy > 6.0:
                break
            if cover(g[k2]) < cv - 1.0 and all(
                    any(a[0] >= b[0] - 1.0 and a[1] <= b[1] + 1.0 for b in g[k]) for a in g[k2]):
                nb += 1; flagged.add(k)
                if len(samples['b']) < 8:
                    samples['b'].append((page, k * TOL, sorted(g[k]), k2 * TOL, sorted(g[k2])))
                break
    return na, nb, nsame


for path in sys.argv[1:]:
    doc = pymupdf.open(path)
    A = B = S = 0
    samples = {'a': [], 'b': []}
    for p in range(doc.page_count):
        H, _ = mod.segs(doc[p])
        a, b, s = mixed_split(H, samples, p)
        A += a; B += b; S += s
    print(f'{path}\n  pages={doc.page_count}  shape(a)={A}  shape(b)={B}  total={A+B}  same-width base={S}')
    for s in samples['a']:
        print('   (a) page %d y~%.3f  %s' % (s[0], s[1], s[2][:6]))
    for s in samples['b']:
        print('   (b) page %d upper y~%.3f %s' % (s[0], s[1], s[2][:4]))
        print('       lower y~%.3f %s' % (s[3], s[4][:4]))
    print()
