#!/usr/bin/env python3
"""Before against after for the affected documents, joined to the reference where known."""
import sys

def load_ours(p):
    out = {}
    for line in open(p, encoding='utf8'):
        f = line.rstrip('\n').split('\t')
        if len(f) == 3:
            out[f[0]] = (int(f[1]), int(f[2]))
    return out


def load_ref(paths):
    out = {}
    for p in paths:
        try:
            lines = open(p, encoding='utf8')
        except OSError:
            continue
        for line in lines:
            f = line.rstrip('\n').split('\t')
            if len(f) >= 7 and '/' in f[2]:
                out[f[0]] = (int(f[2].split('/')[1]), int(f[3].split('/')[1]))
    return out


before = load_ours(sys.argv[1])
after = load_ours(sys.argv[2])
ref = load_ref(sys.argv[3:])

moved = same = 0
print(f"{'pages b/a/ref':>22}  {'words b/a/ref':>28}  verdict  document")
for path in sorted(before):
    b, a = before[path], after.get(path)
    if a is None:
        continue
    r = ref.get(path)
    if b == a:
        same += 1
        continue
    moved += 1
    rp = r[0] if r else '?'
    rw = r[1] if r else '?'
    if r:
        pb, pa = abs(b[0] - r[0]), abs(a[0] - r[0])
        wb, wa = abs(b[1] - r[1]), abs(a[1] - r[1])
        verdict = ('better' if (pa, wa) < (pb, wb) else
                   'WORSE' if (pa, wa) > (pb, wb) else 'level')
    else:
        verdict = 'no-ref'
    print(f"{b[0]:>6}/{a[0]:<5}/{str(rp):<8} {b[1]:>8}/{a[1]:<8}/{str(rw):<8}  {verdict:7}  {path}")
print(f"\n{moved} of {len(before)} affected documents moved; {same} identical")
