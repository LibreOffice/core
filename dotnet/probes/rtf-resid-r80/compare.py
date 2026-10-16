#!/usr/bin/env python3
"""Two scored columns side by side: the verdict counts, the aggregate, and what moved.

    compare.py <before.tsv> <after.tsv>
"""
import sys
import csv


def load(path):
    return {r["name"]: r for r in csv.DictReader(open(path), delimiter="\t")}


b, a = load(sys.argv[1]), load(sys.argv[2])
for tag, d in (("before", b), ("after", a)):
    ok = [r for r in d.values() if r["ours_pages"] != "-"]
    print(f"{tag:6s} match={sum(1 for r in d.values() if r['verdict']=='match')}"
          f" page-exact={sum(1 for r in ok if r['ours_pages']==r['ref_pages'])}"
          f" sum|dpages|={sum(abs(int(r['ours_pages'])-int(r['ref_pages'])) for r in ok)}"
          f" sum|dglyphs|={sum(abs(int(r['ours_glyphs'])-int(r['ref_glyphs'])) for r in ok)}"
          f" of {len(d)}")

for label, rows in (
        ("GAINED", [n for n in a if a[n]["verdict"] == "match" != b[n]["verdict"]]),
        ("LOST", [n for n in a if b[n]["verdict"] == "match" != a[n]["verdict"]])):
    print(f"\n{label} {len(rows)}")
    for n in sorted(rows):
        print(f"  {b[n]['ours_pages']:>5}/{b[n]['ref_pages']:<5} -> "
              f"{a[n]['ours_pages']:>5}/{a[n]['ref_pages']:<5} "
              f"{b[n]['verdict']:>12} -> {a[n]['verdict']:<12} {n}")

moved = [n for n in a if a[n]["ours_pages"] != b[n]["ours_pages"]]
print(f"\npage count moved on {len(moved)} documents")
worse = sum(1 for n in moved
            if abs(int(a[n]['ours_pages']) - int(a[n]['ref_pages']))
            > abs(int(b[n]['ours_pages']) - int(b[n]['ref_pages'])))
print(f"  closer {len(moved)-worse}, further {worse}")
for n in sorted(moved, key=lambda n: -abs(int(a[n]['ours_pages']) - int(b[n]['ours_pages'])))[:25]:
    print(f"  {b[n]['ours_pages']:>5} -> {a[n]['ours_pages']:>5} (ref {a[n]['ref_pages']:>5}) {n}")
