#!/usr/bin/env python3
"""Histogram the first-divergence sweep, split by whether the document passes the gate."""
import collections
import sys

base = "/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/words-r39"

verdict = {}
fmt = {}
for line in open(f"{base}/base/rows.tsv"):
    f = line.rstrip("\n").split("\t")
    verdict[f[0]] = f[6]
    fmt[f[0]] = f[1]

rows = []
for line in open(f"{base}/div.tsv"):
    f = line.rstrip("\n").split("\t")
    if f[0] == "path":
        continue
    rows.append(f)

print(f"{len(rows)} rows swept, {len(verdict)} in the baseline\n")

groups = {
    "MATCH (control)": [r for r in rows if verdict.get(r[0]) == "match"],
    "FAIL": [r for r in rows if verdict.get(r[0], "match") != "match"],
}


def bucket(page):
    if page in ("", "None"):
        return "none — every common page agrees"
    n = int(page)
    return "page 1" if n == 1 else "pages 2-3" if n <= 3 else "page 4 or later"


for name, sel in groups.items():
    print(f"== {name}: {len(sel)} documents")
    dom = collections.Counter(r[5] or "(no divergent page)" for r in sel)
    for k, v in dom.most_common():
        print(f"   {v:4d}  {k}")
    print("   first divergent page:")
    for k, v in collections.Counter(bucket(r[2]) for r in sel).most_common():
        print(f"   {v:4d}  {k}")
    print()

print("== dominance rate, side by side")
kinds = sorted({r[5] or "(none)" for r in rows})
m = groups["MATCH (control)"]
f = groups["FAIL"]
print(f"{'kind':28s} {'match':>12s} {'fail':>12s}")
for k in kinds:
    a = sum(1 for r in m if (r[5] or "(none)") == k)
    b = sum(1 for r in f if (r[5] or "(none)") == k)
    print(f"{k:28s} {a:5d}/{len(m):<6d} {b:5d}/{len(f):<6d}")

print("\n== failing documents that are not the glyph artefact, by kind")
for r in sorted(f, key=lambda r: r[5]):
    if r[5] in ("glyphs",):
        continue
    print(f"  {r[5]:12s} p{r[2]:>3s}/{r[3]:<4s} {r[1]:>8s} {verdict[r[0]]:18s} {fmt[r[0]]:5s} {r[0]}")

print("\n== glyph-dominant failing documents (read as leads only)")
for r in f:
    if r[5] == "glyphs":
        print(f"  gdelta {r[6]:>5s} p{r[2]:>3s}/{r[3]:<4s} {r[1]:>8s} {verdict[r[0]]:18s} {r[0]}")
