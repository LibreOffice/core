#!/usr/bin/env python3
"""Total both of `pdf-image-diff.py`'s ink columns per document, and diff two sweeps.

`track-ink-sweep.sh` sums column 3 -- the *signed* figure, which cancels a region where
we draw more against one where we draw less and is therefore blind to reflow by design.
Column 4, `|ink|%`, sums the same regions unsigned. It cannot tell reflow from loss, so it
is the wrong verdict metric and the right one for ranking and for comparing two runs of
the same round: a correct fix that fills a deficit can *raise* the signed figure by
uncancelling a surplus elsewhere, and has nowhere to hide in the unsigned one.

    ink-columns.py <sweepdir>                 one row per document, both columns
    ink-columns.py <before> <after>           only the documents that moved
"""
import os
import sys


def totals(outdir):
    """path -> (signed ink, unsigned ink, major pages, pages)."""
    out = {}
    cmp_dir = os.path.join(outdir, 'cmp')
    ink = os.path.join(outdir, 'ink.tsv')
    ident = {}
    for line in open(ink, encoding='utf-8'):
        f = line.rstrip('\n').split('\t')
        if len(f) < 5:
            continue
        base = os.path.basename(f[0])
        stem, ext = base.rsplit('.', 1)
        ident[f'{stem}__{ext.lower()}'] = f[0]
    for name in sorted(os.listdir(cmp_dir)):
        if not name.endswith('.txt'):
            continue
        key = name[:-4]
        signed = unsigned = 0.0
        major = 0
        pages = 0
        for line in open(os.path.join(cmp_dir, name), encoding='utf-8'):
            f = line.rstrip('\n').split('\t')
            if len(f) >= 5 and f[0].isdigit():
                try:
                    signed += float(f[2])
                    unsigned += float(f[3])
                except ValueError:
                    continue
                pages += 1
                if len(f) > 5 and 'MAJOR' in f[5]:
                    major += 1
            elif 'with major differences' in line:
                try:
                    major = int(line.split()[2])
                except (IndexError, ValueError):
                    pass
        out[ident.get(key, key)] = (signed, unsigned, major, pages)
    return out


def show(outdir):
    t = totals(outdir)
    si = sum(v[0] for v in t.values())
    ui = sum(v[1] for v in t.values())
    mj = sum(v[2] for v in t.values())
    print(f'{"ink%":>9} {"|ink|%":>9} {"major":>6}  document')
    for path, v in sorted(t.items(), key=lambda kv: -kv[1][1]):
        print(f'{v[0]:9.2f} {v[1]:9.2f} {v[2]:6d}  {path}')
    print(f'\nTOTAL ink% {si:.2f}   |ink|% {ui:.2f}   major pages {mj}   documents {len(t)}')


def diff(before, after):
    a, b = totals(before), totals(after)
    common = sorted(set(a) & set(b))
    print(f'{"ink% a>b":>18} {"|ink|% a>b":>20} {"major":>12}  document')
    moved = 0
    for p in common:
        if abs(a[p][1] - b[p][1]) < 0.005 and abs(a[p][0] - b[p][0]) < 0.005 \
                and a[p][2] == b[p][2]:
            continue
        moved += 1
        print(f'{a[p][0]:8.2f} -> {b[p][0]:6.2f} {a[p][1]:9.2f} -> {b[p][1]:7.2f} '
              f'{a[p][2]:5d} -> {b[p][2]:4d}  {p}')
    for label, i in (('ink%', 0), ('|ink|%', 1), ('major', 2)):
        sa = sum(x[i] for x in a.values())
        sb = sum(x[i] for x in b.values())
        print(f'{label:8s} {sa:10.2f} -> {sb:10.2f}   ({sb - sa:+.2f})')
    print(f'documents compared {len(common)}, moved {moved}')


if __name__ == '__main__':
    if len(sys.argv) == 2:
        show(sys.argv[1])
    elif len(sys.argv) == 3:
        diff(sys.argv[1], sys.argv[2])
    else:
        raise SystemExit(__doc__)
