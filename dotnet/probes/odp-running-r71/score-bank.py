#!/usr/bin/env python3
"""Score a fresh half-sweep of the original corpus against /home/user/gate-2f47's banked rows.

The bank's own verdict column is not reused: both the baseline and the fresh verdict are
computed here with batch-check.sh's rule of 2026-09-05 -- page count, then alphanumeric
characters within max(2%, 15), then unembedded fonts -- so the two sides are the same
instrument and the only thing that differs is our half of the rendering.

Bank columns (rows.tsv, no header): path, ext, pages, words, fonts, unemb, verdict,
rawwords, glyphs -- each of pages/words/fonts/rawwords/glyphs as `ours/ref`.
"""
import csv, sys, collections

def band(og, rg):
    if rg > 0:
        return abs(og - rg) > rg * 0.02 and abs(og - rg) > 15
    return og > 15

def verdict(op, og, ounemb, rp, rg):
    v = []
    if op != rp: v.append('pages')
    if band(og, rg): v.append('words')
    if ounemb != 0: v.append('unembedded')
    return ','.join(v) if v else 'match'

def main():
    bankpath, freshpath = sys.argv[1], sys.argv[2]
    bank = {}
    with open(bankpath) as fh:
        for row in csv.reader(fh, delimiter='\t'):
            if not row or row[0].startswith('#'): continue
            bank[row[0]] = row
    fresh = {}
    with open(freshpath) as fh:
        lines = [l for l in fh if not l.startswith('#')]
    for r in csv.DictReader(lines, delimiter='\t'):
        fresh[r['path']] = r

    before = collections.Counter(); after = collections.Counter(); moved = []
    n = 0
    for path, r in sorted(fresh.items()):
        b = bank.get(path)
        if b is None:
            print('NOT IN BANK', path); continue
        n += 1
        bop, brp = (int(x) for x in b[2].split('/'))
        bog, brg = (int(x) for x in b[8].split('/'))
        bunemb = int(b[5])
        v0 = verdict(bop, bog, bunemb, brp, brg)
        if r['status'] != 'OK':
            v1 = 'ours-failed'
        else:
            v1 = verdict(int(r['pages']), int(r['glyphs']), int(r['unemb'] or 0), brp, brg)
        before[v0] += 1; after[v1] += 1
        if v0 != v1 or (r['status'] == 'OK' and (int(r['pages']) != bop or int(r['glyphs']) != bog)):
            moved.append((path, v0, v1, '%s/%s' % (bop, brp),
                          '%s/%s' % (r['pages'], brp), '%s/%s' % (bog, brg),
                          '%s/%s' % (r['glyphs'], brg)))
    print('rows scored %d' % n)
    print('before:', dict(before))
    print('after :', dict(after))
    print('rows whose pages, glyphs or verdict moved: %d' % len(moved))
    for m in moved:
        print('  %-70s %-10s -> %-10s pages %-10s -> %-10s glyphs %-14s -> %-14s'
              % (m[0][-70:], m[1], m[2], m[3], m[4], m[5], m[6]))

main()
