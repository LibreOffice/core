#!/usr/bin/env python3
"""The bullet offset isolated: only pages where the two renderings draw the SAME
sequence of (font family, size) shows, so the page's layout agrees and what is left
on a bullet is the bullet's own placement."""
import collections, glob, os, statistics

def load(p):
    d = collections.defaultdict(list)
    if not os.path.exists(p): return d
    for ln in open(p):
        f = ln.rstrip('\n').split('\t')
        if len(f) < 5: continue
        d[int(f[1])].append((f[2].split('+')[-1], float(f[3]), float(f[4])))
    return d

bul, body, pages, docs = [], [], 0, set()
tot_pages = 0
for op in sorted(glob.glob('shows/*.ours.tsv')):
    stem = os.path.basename(op)[:-len('.ours.tsv')]
    o, r = load(op), load(f'shows/{stem}.ref.tsv')
    if not r: continue
    for pg in sorted(o):
        tot_pages += 1
        oo, rr = o[pg], r.get(pg, [])
        if len(oo) != len(rr) or not oo: continue
        if any(a[0] != b[0] or abs(a[1]-b[1]) > 0.06 for a, b in zip(oo, rr)): continue
        if not any('OpenSymbol' in a[0] for a in oo): continue
        pages += 1; docs.add(stem)
        for a, b in zip(oo, rr):
            (bul if 'OpenSymbol' in a[0] else body).append((stem, pg, a[1], b[1], b[2]-a[2]))

print(f"pages scanned {tot_pages}; pages whose show sequence agrees and carries a bullet: {pages} in {len(docs)} documents")
for name, rows in (("BULLET", bul), ("body on the same pages", body)):
    if not rows: continue
    d = [x[4] for x in rows]
    print(f"\n{name}: n={len(d)} mean {statistics.mean(d):.4f} median {statistics.median(d):.4f} "
          f"min {min(d):.3f} max {max(d):.3f}")
    print(f"   |dy| <= 0.01 : {sum(1 for v in d if abs(v)<=0.01):5d}"
          f"   <= 0.10 : {sum(1 for v in d if abs(v)<=0.10):5d}"
          f"   >  0.10 : {sum(1 for v in d if abs(v)>0.10):5d}")
g = collections.defaultdict(list)
for stem,pg,so,sr,dy in bul: g[round(sr,2)].append(dy)
print("\n   ref em    n    mean dy    dy/em   (bullets, agreeing pages only)")
for k in sorted(g):
    v = g[k]
    print(f"{k:9.2f} {len(v):4d} {statistics.mean(v):10.4f} {statistics.mean(v)/k:8.5f}")
