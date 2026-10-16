#!/usr/bin/env python3
"""How often does the reference draw a rule in a gap BETWEEN two rules of ours at the same y?

    gap-census.py <rows.tsv> <ours-dir> <ref-dir> <out.tsv> [ext-filter]

That shape is specific. Two of our rules on one baseline with clear air between them, and a
reference rule bridging exactly that air, is an underline that continued across something we
stopped at -- a tab, or the glue a justified line stretches. It is not a thickness question and
not C16's fill-versus-stroke question, because both sides are measured on both shapes and the
comparison is of horizontal COVER, never of object count.

A bridge counts only when the reference's rule covers at least 80% of the gap and the gap is at
least 8 pt, so a rounding step at the join of two abutting rules cannot make one.

The same script gives the other direction -- our rule where the reference has clear air -- so a
round can read the two rates side by side rather than quoting one.
"""
import pathlib
import sys

import pymupdf

TOL_Y = 1.2
MIN_GAP = 8.0
COVER = 0.8


def rules(page):
    out = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 2.0 and r.width >= 4.0 and k in ('f', 'fs', 's'):
                    out.append((r.x0, r.x1, (r.y0 + r.y1) / 2))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 2.0 and abs(a.x - b.x) >= 4.0:
                    out.append((min(a.x, b.x), max(a.x, b.x), (a.y + b.y) / 2))
    return out


def bands(rs):
    """Group rules into y-bands and merge overlapping spans inside each."""
    out = {}
    for x0, x1, y in rs:
        key = round(y / TOL_Y)
        out.setdefault(key, []).append((x0, x1))
    for k, v in out.items():
        v.sort()
        merged = []
        for a, b in v:
            if merged and a <= merged[-1][1] + 0.5:
                merged[-1][1] = max(merged[-1][1], b)
            else:
                merged.append([a, b])
        out[k] = merged
    return out


def bridges(mine, theirs):
    """Total length of gap in `mine` that `theirs` covers, per band."""
    total, count = 0.0, 0
    for key, spans in mine.items():
        other = theirs.get(key) or theirs.get(key - 1) or theirs.get(key + 1)
        if not other or len(spans) < 2:
            continue
        for (a1, b1), (a2, _) in zip(spans, spans[1:]):
            gap = a2 - b1
            if gap < MIN_GAP:
                continue
            cov = sum(max(0.0, min(b1 + gap, y1) - max(b1, y0)) for y0, y1 in other)
            if cov >= COVER * gap:
                total += gap
                count += 1
    return count, total


rowsfile, oursdir, refdir, out = sys.argv[1:5]
extf = sys.argv[5].split(',') if len(sys.argv) > 5 else None
oursdir, refdir = pathlib.Path(oursdir), pathlib.Path(refdir)

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\text\tpath\tnpages\tref_bridges\tref_bridge_pt\tref_bridge_pages'
             '\tours_bridges\tours_bridge_pt\tours_bridge_pages\tworst_page\n')
    for line in pathlib.Path(rowsfile).read_text(encoding='utf-8').splitlines():
        p = line.split('\t')
        if len(p) < 7 or p[6] != 'match':
            continue
        if extf and p[1] not in extf:
            continue
        ident = f'{pathlib.PurePosixPath(p[0]).stem}__{p[1]}'
        o, r = oursdir / f'{ident}.pdf', refdir / f'{ident}.pdf'
        if not (o.exists() and r.exists()):
            continue
        rb = rp = ob = op = npages = 0
        rbl = obl = 0.0
        worst, worstpg = 0.0, 0
        try:
            with pymupdf.open(o) as od, pymupdf.open(r) as rd:
                npages = min(od.page_count, rd.page_count)
                for i in range(npages):
                    mo, mr = bands(rules(od[i])), bands(rules(rd[i]))
                    c, t = bridges(mo, mr)
                    d, u = bridges(mr, mo)
                    rb += c
                    rbl += t
                    ob += d
                    obl += u
                    rp += 1 if c else 0
                    op += 1 if d else 0
                    if t > worst:
                        worst, worstpg = t, i + 1
        except Exception as exc:                      # noqa: BLE001
            print(f'{ident}: {type(exc).__name__}', file=sys.stderr)
            continue
        if rb or ob:
            fh.write(f'{ident}\t{p[1]}\t{p[0]}\t{npages}'
                     f'\t{rb}\t{rbl:.0f}\t{rp}\t{ob}\t{obl:.0f}\t{op}\t{worstpg}\n')
print('done')
