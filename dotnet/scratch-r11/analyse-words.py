#!/usr/bin/env python3
"""Cluster a words-track batch-check sweep. Reads rows.tsv, writes a characterisation."""
import sys, collections, os

rows = []
path = sys.argv[1]
for line in open(path, encoding='utf8'):
    f = line.rstrip('\n').split('\t')
    if len(f) < 7:
        continue
    doc, ext, pages, words, fonts, unemb, verdict = f[:7]
    def pair(s):
        try:
            a, b = s.split('/')
            return int(a), int(b)
        except Exception:
            return None, None
    op, rp = pair(pages)
    ow, rw = pair(words)
    rows.append(dict(doc=doc, ext=ext, op=op, rp=rp, ow=ow, rw=rw,
                     fonts=fonts, unemb=unemb, verdict=verdict,
                     batch=doc.split('/')[1] if '/' in doc else '?'))

print(f"rows: {len(rows)}   unique: {len(set(r['doc'] for r in rows))}")
verd = collections.Counter(r['verdict'] for r in rows)
print("verdicts:", dict(verd))

match = [r for r in rows if r['verdict'] == 'match']
fail = [r for r in rows if r['verdict'] != 'match']
print(f"\nMATCH {len(match)}/{len(rows)}")

pagexact = [r for r in rows if r['op'] is not None and r['op'] == r['rp']]
pageerr = sum(abs(r['op'] - r['rp']) for r in rows if r['op'] is not None and r['rp'] is not None)
worderr_short = sum(max(0, r['rw'] - r['ow']) for r in rows if r['ow'] is not None and r['rw'] is not None)
worderr_over = sum(max(0, r['ow'] - r['rw']) for r in rows if r['ow'] is not None and r['rw'] is not None)
print(f"page-exact documents: {len(pagexact)}")
print(f"total absolute page error: {pageerr}")
print(f"total absolute word error: {worderr_short + worderr_over}  ({worderr_short} short / {worderr_over} over)")

print("\nper batch:")
bybatch = collections.defaultdict(lambda: [0, 0])
for r in rows:
    bybatch[r['batch']][1] += 1
    if r['verdict'] == 'match':
        bybatch[r['batch']][0] += 1
for b in sorted(bybatch):
    m, t = bybatch[b]
    print(f"  {b}  {m}/{t}")

print("\nby format (failure rate):")
byfmt = collections.defaultdict(lambda: [0, 0])
for r in rows:
    byfmt[r['ext']][1] += 1
    if r['verdict'] == 'match':
        byfmt[r['ext']][0] += 1
for e in sorted(byfmt):
    m, t = byfmt[e]
    print(f"  {e}  {m}/{t} match, {100*(t-m)/t:.0f}% fail")

print("\npage-delta histogram over failures (ours - ref):")
hist = collections.Counter()
for r in fail:
    if r['op'] is None or r['rp'] is None:
        hist['?'] += 1
    else:
        hist[r['op'] - r['rp']] += 1
for k in sorted(hist, key=lambda x: (isinstance(x, str), x)):
    print(f"  {k:>4}  {hist[k]}")

print("\nfailures, sorted by |page delta| then |word delta|:")
def key(r):
    if r['op'] is None:
        return (999, 0)
    return (-abs(r['op'] - r['rp']), -abs(r['ow'] - r['rw']) if r['ow'] is not None else 0)
for r in sorted(fail, key=key):
    d = '' if r['op'] is None else f"{r['op'] - r['rp']:+d}"
    wd = '' if r['ow'] is None else f"{r['ow'] - r['rw']:+d}"
    pct = ''
    if r['rw']:
        pct = f"{100.0*(r['ow']-r['rw'])/r['rw']:+.1f}%"
    print(f"  {d:>4} pg {r['op']}/{r['rp']}  w {r['ow']}/{r['rw']} {wd:>7} {pct:>8}  {r['verdict']:<10} {os.path.basename(r['doc'])[:64]}  [{r['batch']}]")
