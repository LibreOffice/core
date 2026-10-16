#!/usr/bin/env python3
"""Diff two batch-check rows.tsv files, flagging C13 volatile-date rows as
not comparable when the two gates straddle a UTC date boundary."""
import sys, os

def load(p):
    d = {}
    for ln in open(p):
        f = ln.rstrip("\n").split("\t")
        if len(f) < 9: continue
        d[f[0]] = f
    return d

def main(a, b, volfile, la, lb):
    A, B = load(a), load(b)
    vol = set()
    if volfile and os.path.isfile(volfile):
        for ln in open(volfile):
            ln = ln.strip()
            if not ln: continue
            vol.add(ln.split("\t")[-1])          # basename

    only_a = sorted(set(A) - set(B))
    only_b = sorted(set(B) - set(A))
    print(f"{la}: {len(A)} rows   {lb}: {len(B)} rows")
    for k in only_a: print(f"  only in {la}: {k}")
    for k in only_b: print(f"  only in {lb}: {k}")

    vcount = {c: 0 for c in (2, 6, 8)}
    changed = []
    volatile_changed = []
    for k in sorted(set(A) & set(B)):
        x, y = A[k], B[k]
        isvol = os.path.basename(k) in vol
        diffs = []
        for col, name in ((2, "pages"), (6, "verdict"), (8, "glyphs")):
            if x[col] != y[col]:
                diffs.append(f"{name} {x[col]} -> {y[col]}")
                if not isvol: vcount[col] += 1
        if diffs:
            (volatile_changed if isvol else changed).append((k, diffs))

    print(f"\n=== changed, comparable ({len(changed)}) ===")
    for k, d in changed: print(f"  {k}\n      {'; '.join(d)}")
    print(f"\n=== changed, C13 volatile-date - NOT COMPARABLE ({len(volatile_changed)}) ===")
    for k, d in volatile_changed: print(f"  {os.path.basename(k)}\n      {'; '.join(d)}")

    print(f"\nverdict changes (comparable only): {vcount[6]}")
    print(f"page   changes (comparable only): {vcount[2]}")
    print(f"glyph  changes (comparable only): {vcount[8]}")
    for lbl, p in ((la, a), (lb, b)):
        m = sum(1 for f in load(p).values() if f[6] == "match")
        print(f"{lbl}: MATCH {m} / {len(load(p))}")

if __name__ == "__main__":
    main(*sys.argv[1:6])
