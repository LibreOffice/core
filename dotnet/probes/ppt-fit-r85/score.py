#!/usr/bin/env python3
"""Score a base and a head sweep against each other and against the banked reference.

    score.py gate  <base/rows.tsv> <head/rows.tsv>
    score.py sizes <ref.tsv> <base.tsv> <head.tsv>

`gate` reads `sweep-ours.sh`'s columns: 1 path, 2 ext, 3 pages, 4 words, 5 fonts,
6 unembedded, 7 verdict, 8 rawwords, 9 glyphs, 10 md5. **The verdict is decided on column 9**,
the alphanumeric character count, not on column 4 -- see `batch-check.sh`:279-295.

`sizes` reads `size-sweep.sh`'s: id, page, dominant size, characters at it.
"""
import csv, sys, collections


def load_rows(path):
    return {r[0]: r for r in csv.reader(open(path), delimiter='\t')}


def load_sizes(path):
    out = {}
    for line in open(path):
        f = line.rstrip('\n').split('\t')
        if len(f) < 4 or not f[2] or not f[3]:
            continue
        out[(f[0], int(f[1]))] = float(f[2])
    return out


def gate(base, head):
    b, h = load_rows(base), load_rows(head)
    keys = sorted(set(b) & set(h))
    moved = [k for k in keys if b[k][9] != h[k][9]]
    print(f'rows {len(keys)}   renderings differing {len(moved)}')
    print(f'gate match: base {sum(1 for k in keys if b[k][6] == "match")}'
          f'  head {sum(1 for k in keys if h[k][6] == "match")}')
    for k in keys:
        if b[k][6] != h[k][6]:
            print(f'  VERDICT {k}  {b[k][6]} -> {h[k][6]}')
    print('pages moved', sum(1 for k in moved if b[k][2] != h[k][2]))
    for k in moved:
        if b[k][8] != h[k][8]:
            print(f'  glyphs {k.split("/")[-1]}  {b[k][8]} -> {h[k][8]}')


def sizes(ref, base, head):
    R, B, H = load_sizes(ref), load_sizes(base), load_sizes(head)
    common = sorted(set(R) & set(B) & set(H))
    db = {k for k in common if abs(B[k] - R[k]) > 0.15}
    dh = {k for k in common if abs(H[k] - R[k]) > 0.15}
    print(f'pages compared {len(common)}')
    print(f'dominant size off 26.2.4.2 by >0.15 pt: base {len(db)} in '
          f'{len({k[0] for k in db})} docs, head {len(dh)} in {len({k[0] for k in dh})} docs')
    print(f'fixed {len(db - dh)}   newly wrong {len(dh - db)}')
    print('fixed by ext', collections.Counter(k[0].rsplit("__", 1)[-1] for k in db - dh))
    print('total |size error| pt: base %.2f  head %.2f'
          % (sum(abs(B[k] - R[k]) for k in common), sum(abs(H[k] - R[k]) for k in common)))


if __name__ == '__main__':
    if sys.argv[1] == 'gate':
        gate(*sys.argv[2:])
    else:
        sizes(*sys.argv[2:])
