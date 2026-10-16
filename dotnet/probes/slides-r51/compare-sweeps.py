#!/usr/bin/env python3
"""Compare two sweeps document by document, case-folded, on verdict and both ink columns."""
import sys

def read(sweep):
    rows = {}
    with open(f"{sweep}/rows.tsv", encoding="utf-8") as fh:
        for line in fh:
            f = line.rstrip("\n").split("\t")
            if len(f) < 7: continue
            rows[f[0].casefold()] = f
    ink = {}
    with open(f"{sweep}/ink.tsv", encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#") or line.startswith("path\t"): continue
            f = line.rstrip("\n").split("\t")
            if len(f) < 6: continue
            ink[f[0].casefold()] = f
    return rows, ink

(ar, ai), (br, bi) = read(sys.argv[1]), read(sys.argv[2])
keys = sorted(set(ar) | set(br))
print(f"documents (case-folded): before {len(ar)}  after {len(br)}")

am = sum(1 for k in ar if ar[k][6] == "match")
bm = sum(1 for k in br if br[k][6] == "match")
print(f"match: {am} -> {bm}")

moved = [(k, ar[k][6], br[k][6]) for k in keys if k in ar and k in br and ar[k][6] != br[k][6]]
print(f"\nverdict movement: {len(moved)}")
for k, a, b in moved: print(f"   {ar[k][0]}\n      {a} -> {b}")

pages = [(k, ar[k][2], br[k][2]) for k in keys if k in ar and k in br and ar[k][2] != br[k][2]]
print(f"\npage-count movement: {len(pages)}")
for k, a, b in pages: print(f"   {ar[k][0]}  {a} -> {b}")

words = [(k, ar[k][3], br[k][3]) for k in keys if k in ar and k in br and ar[k][3] != br[k][3]]
print(f"word-count movement: {len(words)}")
for k, a, b in words: print(f"   {ar[k][0]}  {a} -> {b}")

def total(ink, col):
    s = 0.0
    for f in ink.values():
        try: s += float(f[col])
        except (ValueError, IndexError): pass
    return s

print(f"\nabs_ink    (unsigned, ranks): {total(ai,2):.2f} -> {total(bi,2):.2f}")
print(f"signed_ink (direction)      : {total(ai,3):.2f} -> {total(bi,3):.2f}")
def majors(ink):
    s=0
    for f in ink.values():
        try: s += int(f[4])
        except (ValueError, IndexError): pass
    return s
print(f"major pages                 : {majors(ai)} -> {majors(bi)}")

rows = []
for k in sorted(set(ai) & set(bi)):
    try:
        a, b = float(ai[k][2]), float(bi[k][2])
        sa, sb = float(ai[k][3]), float(bi[k][3])
    except ValueError:
        continue
    if abs(a - b) >= 0.005:
        rows.append((b - a, ai[k][0], a, b, sa, sb))
print(f"\ndocuments whose abs_ink moved: {len(rows)}  "
      f"(improved {sum(1 for r in rows if r[0] < 0)}, worsened {sum(1 for r in rows if r[0] > 0)})")
for d, name, a, b, sa, sb in sorted(rows):
    print(f"   {d:+8.2f}  abs {a:7.2f} -> {b:7.2f}   signed {sa:7.2f} -> {sb:7.2f}   {name}")
