#!/usr/bin/env python3
"""Per-page ink, baseline against a later run, for one document's cmp report."""
import re, sys

def perpage(path):
    out = {}
    for line in open(path, encoding="utf-8", errors="replace"):
        f = line.rstrip("\n").split("\t")
        if len(f) >= 3 and re.fullmatch(r"\d+", f[0]) and re.fullmatch(r"[\d.]+", f[2]):
            out[int(f[0])] = float(f[2])
    return out

a, b = perpage(sys.argv[1]), perpage(sys.argv[2])
pages = sorted(set(a) | set(b))
moved = [(p, a.get(p, 0.0), b.get(p, 0.0)) for p in pages
         if abs(a.get(p, 0.0) - b.get(p, 0.0)) > 0.005]
moved.sort(key=lambda r: -(r[2] - r[1]))
print("%-6s %8s %8s %8s" % ("page", "base", "after", "delta"))
for p, x, y in moved[:20]:
    print("%-6d %8.2f %8.2f %+8.2f" % (p, x, y, y - x))
if len(moved) > 20:
    print("... and %d more" % (len(moved) - 20))
print()
print("pages moved: %d of %d" % (len(moved), len(pages)))
print("base total  %.2f" % sum(a.values()))
print("after total %.2f" % sum(b.values()))
up = sum(y - x for _, x, y in moved if y > x)
down = sum(x - y for _, x, y in moved if y < x)
print("up %.2f   down %.2f" % (up, down))
