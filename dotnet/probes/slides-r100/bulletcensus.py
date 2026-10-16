#!/usr/bin/env python3
"""Round 99's bulletcensus2.py, on tfz.py's tables rather than tfy.py's, with a
clean-page subset beside it.

Two changes from round 99, and the first is the reason the sample is seven times
bigger.  (1) tfy.py enumerated pages by regex-scanning `N 0 obj` and testing a
700-byte window for /Type /Page; our writer packs objects tightly, so that window
runs into the next object and a shading dictionary is read as a page -- 28 pages for
Inducement-to-Insurance-Business.ppt's 22 -- and every per-page pairing past the
first artefact is off.  tfz.py enumerates through the real page tree.  (2) A page is
reported twice: once under round 99's filter (the two renderings draw the same
sequence of (family, size) shows) and once under a stricter one -- every BODY show on
the page within 0.10 pt of the reference's baseline, so the page's layout provably
agrees and what is left on a bullet is the bullet's own placement.

    bulletcensus.py <shows-directory>
"""
import collections, glob, os, statistics, sys

def load(p):
    d = collections.defaultdict(list)
    for ln in open(p):
        f = ln.rstrip('\n').split('\t')
        if len(f) < 5: continue
        d[int(f[0])].append((f[1].split('+')[-1], float(f[2]), float(f[4])))
    return d

root = sys.argv[1]
bul, body, pages, docs, tot = [], [], 0, set(), 0
clean = dict(pages=0, body=0)
cbul = []
per = collections.defaultdict(lambda: [[], []])
for op in sorted(glob.glob(os.path.join(root, '*.ours.tsv'))):
    stem = os.path.basename(op)[:-len('.ours.tsv')]
    rp = os.path.join(root, stem + '.ref.tsv')
    if not os.path.exists(rp): continue
    o, r = load(op), load(rp)
    for pg in sorted(o):
        tot += 1
        oo, rr = o[pg], r.get(pg, [])
        if len(oo) != len(rr) or not oo: continue
        if any(a[0] != b[0] or abs(a[1] - b[1]) > 0.06 for a, b in zip(oo, rr)): continue
        if not any('OpenSymbol' in a[0] for a in oo): continue
        pages += 1; docs.add(stem)
        bb = [b[2] - a[2] for a, b in zip(oo, rr) if 'OpenSymbol' not in a[0]]
        uu = [b[2] - a[2] for a, b in zip(oo, rr) if 'OpenSymbol' in a[0]]
        bul += uu; body += bb
        per[stem][0] += uu; per[stem][1] += bb
        if bb and max(abs(v) for v in bb) <= 0.10:
            clean['pages'] += 1; clean['body'] += len(bb); cbul += uu

def out(v):
    return sum(1 for x in v if abs(x) > 0.10)

print(f"shows directory {root}")
print(f"pages scanned {tot}; agreeing pages carrying a bullet: {pages} in {len(docs)} documents")
for name, d in (("BULLET", bul), ("body  ", body)):
    print(f"  {name} n={len(d):5} mean {statistics.mean(d):8.4f} median {statistics.median(d):8.4f}"
          f"  beyond 0.10 pt {out(d):5} ({100 * out(d) / len(d):5.1f}%)")
print(f"\n  of those, pages where EVERY body show is within 0.10 pt: {clean['pages']}"
      f" ({clean['body']} body shows, {len(cbul)} bullets)")
print(f"  BULLET beyond 0.10 pt {out(cbul)} of {len(cbul)} ({100 * out(cbul) / len(cbul):.1f}%)"
      f"  mean {statistics.mean(cbul):.4f} min {min(cbul):.3f} max {max(cbul):.3f}")
print(f"\n{'document':52}{'nbul':>6}{'>0.1':>6}{'%':>7}{'nbody':>7}{'>0.1':>6}{'%':>7}")
for k in sorted(per, key=lambda k: -len(per[k][0])):
    b, y = per[k]
    if not b: continue
    print(f"{k[:52]:52}{len(b):6}{out(b):6}{100 * out(b) / len(b):6.1f}%"
          f"{len(y):7}{out(y):6}{100 * out(y) / max(len(y), 1):6.1f}%")
