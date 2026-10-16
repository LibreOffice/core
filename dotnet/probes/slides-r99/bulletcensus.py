#!/usr/bin/env python3
"""Every OpenSymbol bullet the two renderings both draw, paired within a page, on the
pages where the two agree on how many there are AND on the size of each.  The offset
is read from the content stream's own text-matrix, never from a bounding box."""
import collections, glob, os, statistics, sys

def load(p):
    d = collections.defaultdict(list)
    if not os.path.exists(p): return d
    for ln in open(p):
        f = ln.rstrip('\n').split('\t')
        if len(f) < 5: continue
        d[int(f[1])].append((f[2], float(f[3]), float(f[4])))
    return d

pairs, skipped_count, skipped_size, docs = [], 0, 0, set()
allpages = 0
for op in sorted(glob.glob('shows/*.ours.tsv')):
    stem = os.path.basename(op)[:-len('.ours.tsv')]
    o, r = load(op), load(f'shows/{stem}.ref.tsv')
    if not r: continue
    for pg in sorted(o):
        allpages += 1
        ob = [x for x in o[pg] if 'OpenSymbol' in x[0]]
        rb = [x for x in r.get(pg, []) if 'OpenSymbol' in x[0]]
        if not ob and not rb: continue
        if len(ob) != len(rb):
            skipped_count += 1; continue
        if any(abs(a[1]-b[1]) > 0.35 for a, b in zip(ob, rb)):
            skipped_size += 1; continue
        for (fo,so,yo),(fr,sr,yr) in zip(ob, rb):
            pairs.append((stem, pg, so, sr, yo, yr))
            docs.add(stem)

print(f"pages scanned {allpages}")
print(f"pages skipped, bullet count differs {skipped_count}")
print(f"pages skipped, a bullet size differs by more than 0.35 pt {skipped_size}")
print(f"bullets paired {len(pairs)} in {len(docs)} documents")
dy = [(yr-yo) for _,_,_,_,yo,yr in pairs]
ds = [(sr-so) for _,_,so,sr,_,_ in pairs]
print(f"\nbaseline offset ref-ours (pt): mean {statistics.mean(dy):.4f} "
      f"median {statistics.median(dy):.4f} min {min(dy):.3f} max {max(dy):.3f}")
print(f"  |dy| <= 0.01 : {sum(1 for v in dy if abs(v)<=0.01)}")
print(f"  |dy| <= 0.10 : {sum(1 for v in dy if abs(v)<=0.10)}")
print(f"  |dy| >  0.10 : {sum(1 for v in dy if abs(v)>0.10)}")
print(f"size difference ref-ours (pt): mean {statistics.mean(ds):.4f} "
      f"min {min(ds):.3f} max {max(ds):.3f}")
print(f"  exactly equal        : {sum(1 for v in ds if abs(v)<0.0005)}")
print(f"  one mm100 unit apart : {sum(1 for v in ds if 0.0005<=abs(v)<=0.030)}")
print(f"  more than that       : {sum(1 for v in ds if abs(v)>0.030)}")

# offset as a fraction of the bullet em, grouped
g = collections.defaultdict(list)
for stem,pg,so,sr,yo,yr in pairs:
    g[round(sr,2)].append(yr-yo)
print("\n   ref em    n    mean dy    dy/em")
for k in sorted(g):
    v=g[k]
    if len(v) < 3: continue
    print(f"{k:9.2f} {len(v):4d} {statistics.mean(v):10.4f} {statistics.mean(v)/k:8.5f}")
