#!/usr/bin/env python3
"""Split every `size A vs B` on the .ppt half of the track into ratio bands.

The predecessor's instruction, and the thing nobody had done: band before theorising. One
scale would put every record in one band; quantisation would put them all in the narrowest.
"""
import collections, os, re, subprocess, sys
from pathlib import Path

SWEEP = Path(sys.argv[1])
ONLY = sys.argv[2] if len(sys.argv) > 2 else "ppt"
OPS = "/home/user/wt-slides-rd22/.claude/skills/render-comparison/scripts/pdf-ops.py"
env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000")

BANDS = [(0.0, 0.002, "<0.2%"), (0.002, 0.006, "0.2-0.6%"), (0.006, 0.01, "0.6-1%"),
         (0.01, 0.03, "1-3%"), (0.03, 0.10, "3-10%"), (0.10, 9.0, ">10%")]

def band(r):
    d = abs(r - 1.0)
    for lo, hi, name in BANDS:
        if lo <= d < hi:
            return name
    return ">10%"

counts = collections.Counter()
per_doc = collections.defaultdict(collections.Counter)
whole_ours = collections.Counter()
whole_ref = collections.Counter()
rows = []

ids = sorted(p.stem for p in (SWEEP / "ours").glob("*.pdf") if p.stem.endswith("__" + ONLY))
for n, i in enumerate(ids, 1):
    o, r = SWEEP / "ours" / f"{i}.pdf", SWEEP / "ref" / f"{i}.pdf"
    if not r.exists():
        continue
    out = subprocess.run([OPS, "diff", str(o), str(r)], capture_output=True, text=True,
                         env=env, timeout=900).stdout
    k = 0
    for m in re.finditer(r"size ([\d.]+) vs ([\d.]+)", out):
        a, b = float(m.group(1)), float(m.group(2))
        if b == 0:
            continue
        bd = band(a / b)
        counts[bd] += 1
        per_doc[i][bd] += 1
        # "whole point" within the two decimals pdf-ops prints
        if abs(a - round(a)) <= 0.02:
            whole_ours[bd] += 1
        if abs(b - round(b)) <= 0.02:
            whole_ref[bd] += 1
        rows.append((i, a, b))
        k += 1
    print(f"[{n}/{len(ids)}] {i} {k}", flush=True, file=sys.stderr)

total = sum(counts.values())
print(f"\n{total} differing-size records over {len(per_doc)} {ONLY} documents\n")
print(f"{'band':10s} {'records':>8s} {'share':>7s} {'docs':>5s} {'ours whole pt':>14s} {'ref whole pt':>13s}")
for _, _, name in BANDS:
    c = counts[name]
    if not c:
        continue
    d = sum(1 for v in per_doc.values() if v[name])
    print(f"{name:10s} {c:8d} {c/total:6.1%} {d:5d} {whole_ours[name]:13d} {whole_ref[name]:12d}")

print("\nper document, share of its records in each band")
for i, v in sorted(per_doc.items(), key=lambda kv: -sum(kv[1].values())):
    t = sum(v.values())
    parts = "  ".join(f"{n} {v[n]/t:.0%}" for _, _, n in BANDS if v[n])
    print(f"{t:6d}  {i[:56]:58s} {parts}")

print("\nthe wide band, record by record (|ratio-1| >= 3%)")
wide = collections.Counter()
for i, a, b in rows:
    if abs(a / b - 1) >= 0.03:
        wide[(a, b)] += 1
for (a, b), c in wide.most_common(40):
    print(f"{c:5d}  ours {a:7.2f}  ref {b:7.2f}  ratio {a/b:.4f}")
