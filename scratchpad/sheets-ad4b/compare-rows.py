#!/usr/bin/env python3
"""Join our computed row heights against LibreOffice's, per sheet and row."""
import sys, collections

lo_path, our_path = sys.argv[1], sys.argv[2]
lo = {}
for line in open(lo_path):
    p = line.rstrip('\n').split('\t')
    if len(p) < 5: continue
    sheet, row, rep, h, opt = p[0], int(p[1]), int(p[2]), p[3], p[4]
    if h == '-': continue
    if rep > 2000: continue
    for i in range(rep):
        lo[(sheet, row + i)] = (float(h), opt)

ours = {}
for line in open(our_path):
    p = line.rstrip('\n').split('\t')
    if len(p) < 4: continue
    ours[(p[0], int(p[1]))] = (float(p[2]), p[3])

keys = sorted(set(lo) & set(ours), key=lambda k: (k[0], k[1]))
print('# common rows: %d   (lo %d, ours %d)' % (len(keys), len(lo), len(ours)))

tot_lo = tot_our = 0.0
buckets = collections.Counter()
detail = []
for k in keys:
    l, lopt = lo[k]
    o, oopt = ours[k]
    tot_lo += l; tot_our += o
    d = o - l
    if abs(d) < 1.0:
        buckets['exact'] += 1
    else:
        buckets['short' if d < 0 else 'tall'] += 1
        detail.append((k[0], k[1], l, o, d, lopt, oopt))

print('# total twips: lo %.0f  ours %.0f   ratio %.5f' % (tot_lo, tot_our, tot_our / tot_lo))
print('# exact %d  ours-taller %d  ours-shorter %d' % (buckets['exact'], buckets['tall'], buckets['short']))

# worst contributors
detail.sort(key=lambda r: -abs(r[4]))
print('# worst 40 by |delta|')
for r in detail[:40]:
    print('%s\trow %d\tlo %.1f\tours %.1f\tdelta %+.1f\tloopt=%s\touropt=%s' % r)

# aggregate delta per sheet
per = collections.defaultdict(lambda: [0.0, 0.0, 0])
for k in keys:
    per[k[0]][0] += lo[k][0]; per[k[0]][1] += ours[k][0]; per[k[0]][2] += 1
print('# per sheet: name, rows, lo, ours, delta, ratio')
for s, (l, o, n) in sorted(per.items(), key=lambda x: -(x[1][1] - x[1][0])):
    print('%s\t%d\t%.0f\t%.0f\t%+.0f\t%.4f' % (s, n, l, o, o - l, o / l if l else 0))
