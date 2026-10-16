#!/usr/bin/env python3
"""Apply batch-check.sh's verdict rule to a pair of half-sweeps.

Rule as of 2026-09-05 (batch-check.sh:261-284): page count, then alphanumeric
characters within max(2%, 15), then unembedded fonts.
"""
import csv, sys

def load(path):
    rows = {}
    with open(path) as fh:
        for line in fh:
            if line.startswith('#'): continue
            break
        r = csv.DictReader(fh, delimiter='\t', fieldnames=line.rstrip('\n').split('\t'))
        for row in r:
            rows[row['path']] = row
    return rows

def verdict(o, r):
    if o is None or o['status'] != 'OK':
        if r is None or r['status'] != 'OK': return 'both-failed'
        return 'ours-failed'
    if r is None or r['status'] != 'OK': return 'ref-failed'
    v = []
    if o['pages'] != r['pages']: v.append('pages')
    og, rg = int(o['glyphs']), int(r['glyphs'])
    if rg > 0:
        if abs(og - rg) > rg * 0.02 and abs(og - rg) > 15: v.append('words')
    elif og > 15:
        v.append('words')
    if int(o['unemb'] or 0) != 0: v.append('unembedded')
    return ','.join(v) if v else 'match'

def main():
    ours, ref, out = sys.argv[1], sys.argv[2], sys.argv[3]
    O, R = load(ours), load(ref)
    paths = sorted(set(O) | set(R))
    import collections
    tally = collections.Counter()
    with open(out, 'w', newline='') as fh:
        fh.write('# ours=%s ref=%s\n' % (ours, ref))
        w = csv.writer(fh, delimiter='\t')
        w.writerow(['path', 'pages', 'glyphs', 'verdict'])
        for p in paths:
            o, r = O.get(p), R.get(p)
            v = verdict(o, r)
            tally[v] += 1
            w.writerow([p,
                        '%s/%s' % (o['pages'] if o else '', r['pages'] if r else ''),
                        '%s/%s' % (o['glyphs'] if o else '', r['glyphs'] if r else ''),
                        v])
    print('TOTAL %d' % len(paths))
    for k, n in tally.most_common(): print('%-16s %d' % (k, n))

main()
