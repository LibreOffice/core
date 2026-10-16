#!/usr/bin/env python3
"""Every gap of ours the reference bridges, one row per bridge, with the spans either side.

    bridges.py <ours.pdf> <ref.pdf> [ident]

Same shape and thresholds as ink-hunt-r124/gap-census.py -- TOL_Y 1.2, MIN_GAP 8, COVER 0.8 --
but it reports each bridge rather than a per-document total, so a bridge can be looked up
against the table geometry that produced it.
"""
import sys
import pymupdf

TOL_Y, MIN_GAP, COVER = 1.2, 8.0, 0.8


def rules(page):
    out = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 2.0 and r.width >= 4.0 and k in ('f', 'fs', 's'):
                    out.append((r.x0, r.x1, (r.y0 + r.y1) / 2, 'fill'))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 2.0 and abs(a.x - b.x) >= 4.0:
                    out.append((min(a.x, b.x), max(a.x, b.x), (a.y + b.y) / 2, 'stroke'))
    return out


def bands(rs):
    out = {}
    for x0, x1, y, kind in rs:
        out.setdefault(round(y / TOL_Y), []).append((x0, x1, y, kind))
    for k, v in out.items():
        v.sort()
        merged = []
        for a, b, y, kind in v:
            if merged and a <= merged[-1][1] + 0.5:
                merged[-1][1] = max(merged[-1][1], b)
                merged[-1][3] |= {kind}
            else:
                merged.append([a, b, y, {kind}])
        out[k] = merged
    return out


ours, ref = sys.argv[1], sys.argv[2]
ident = sys.argv[3] if len(sys.argv) > 3 else ours
print('ident\tpage\tband_y\tgap_x0\tgap_x1\tgap_pt\tleft_x0\tleft_x1\tright_x0\tright_x1'
      '\tour_kind\tref_x0\tref_x1\tref_y\tref_kind')
with pymupdf.open(ours) as od, pymupdf.open(ref) as rd:
    for i in range(min(od.page_count, rd.page_count)):
        mo, mr = bands(rules(od[i])), bands(rules(rd[i]))
        for key, spans in mo.items():
            other = mr.get(key) or mr.get(key - 1) or mr.get(key + 1)
            if not other or len(spans) < 2:
                continue
            for (a1, b1, y1, k1), (a2, b2, y2, k2) in zip(spans, spans[1:]):
                gap = a2 - b1
                if gap < MIN_GAP:
                    continue
                cov = sum(max(0.0, min(a2, t1) - max(b1, t0)) for t0, t1, _, _ in other)
                if cov < COVER * gap:
                    continue
                best = max(other, key=lambda t: max(0.0, min(a2, t[1]) - max(b1, t[0])))
                print(f'{ident}\t{i+1}\t{y1:.2f}\t{b1:.2f}\t{a2:.2f}\t{gap:.2f}'
                      f'\t{a1:.2f}\t{b1:.2f}\t{a2:.2f}\t{b2:.2f}\t{"+".join(sorted(k1 | k2))}'
                      f'\t{best[0]:.2f}\t{best[1]:.2f}\t{best[2]:.2f}\t{"+".join(sorted(best[3]))}')
