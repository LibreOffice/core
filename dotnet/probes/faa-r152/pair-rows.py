#!/usr/bin/env python3
"""Reproduce round 131's page-82 row pairing from the two PDFs' own ink.

Method (r131 results.md §4): take every horizontal rule on the page, drop the page
furniture (the 2.25 pt header/footer rules), merge rules whose y agree within MERGE pt
into one grid line -- the reference draws two of them doubled, 0.25 pt apart, and so do
we -- then pair the two lists positionally and difference consecutive gaps.

    pair-rows.py <ref.pdf> <ours.pdf> <1-based page> [thickness-to-drop]
"""
import sys, pymupdf

MERGE = 0.4
DROP_THICK = 2.0          # the page's own header/footer rules

def lines(path, pno):
    doc = pymupdf.open(path)
    page = doc[pno - 1]
    out = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 3.0 and r.width >= 2.0 and k in ('f', 'fs', 's'):
                    out.append(((r.y0 + r.y1) / 2, r.height))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 3.0 and abs(a.x - b.x) >= 2.0:
                    out.append(((a.y + b.y) / 2, d.get('width') or 0.0))
    out = [(y, w) for y, w in out if w < DROP_THICK]
    out.sort()
    merged = []
    for y, w in out:
        if merged and y - merged[-1][-1][0] <= MERGE:
            merged[-1].append((y, w))
        else:
            merged.append([(y, w)])
    # one grid line per cluster, at the mean of its members' y -- the same rule on both sides
    return [sum(y for y, _ in c) / len(c) for c in merged]

ref = lines(sys.argv[1], int(sys.argv[3]))
our = lines(sys.argv[2], int(sys.argv[3]))
print(f'# grid lines: ref {len(ref)}  ours {len(our)}', file=sys.stderr)
if len(ref) != len(our):
    print('# UNEQUAL -- not paired', file=sys.stderr)
print('row\tref_top\tref_bot\tref_height_pt\tours_top\tours_bot\tours_height_pt\tdelta_pt')
n = min(len(ref), len(our)) - 1
tot = 0.0
for i in range(n):
    rh = ref[i + 1] - ref[i]
    oh = our[i + 1] - our[i]
    d = oh - rh
    tot += d
    print(f'{i}\t{ref[i]:.2f}\t{ref[i+1]:.2f}\t{rh:.2f}\t{our[i]:.2f}\t{our[i+1]:.2f}\t{oh:.2f}\t{d:+.2f}')
print(f'# first line: ref {ref[0]:.2f} ours {our[0]:.2f} delta {our[0]-ref[0]:+.2f}', file=sys.stderr)
print(f'# last  line: ref {ref[-1]:.2f} ours {our[-1]:.2f} delta {our[-1]-ref[-1]:+.2f}', file=sys.stderr)
print(f'# sum of row deltas {tot:+.2f}', file=sys.stderr)
