#!/usr/bin/env python3
"""Compare two whole-track sweeps: which documents moved, and by how much."""
import sys, os

S = "/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad"

def ink(name):
    out = {}
    for line in open(os.path.join(S, name, "ink.tsv"), encoding="utf-8"):
        f = line.rstrip("\n").split("\t")
        if len(f) < 5 or f[2] in ("-", "?"):
            continue
        out[f[0]] = (float(f[2]), int(f[3]), f[4])
    return out

def rows(name):
    out = {}
    for line in open(os.path.join(S, name, "rows.tsv"), encoding="utf-8"):
        f = line.rstrip("\n").split("\t")
        if len(f) >= 7:
            out[f[0]] = f[1:]
    return out

a, b = ink(sys.argv[1]), ink(sys.argv[2])
ra, rb = rows(sys.argv[1]), rows(sys.argv[2])

moved = [(p, a[p][0], b[p][0], a[p][1], b[p][1]) for p in a if p in b
         and abs(a[p][0] - b[p][0]) > 0.005]
moved.sort(key=lambda r: r[1] - r[2], reverse=True)

print("%-58s %8s %8s %8s   major" % ("document", "base", "after", "delta"))
for p, x, y, mx, my in moved:
    print("%-58s %8.2f %8.2f %+8.2f   %d -> %d" % (os.path.basename(p)[:58], x, y, y - x, mx, my))

print()
print("documents whose ink moved: %d   down %d   up %d"
      % (len(moved), sum(1 for r in moved if r[2] < r[1]), sum(1 for r in moved if r[2] > r[1])))
print("sum of falls %.2f   sum of rises %.2f"
      % (sum(r[1] - r[2] for r in moved if r[2] < r[1]),
         sum(r[2] - r[1] for r in moved if r[2] > r[1])))
print("track ink %.2f -> %.2f  (%+.2f)"
      % (sum(v[0] for v in a.values()), sum(v[0] for v in b.values()),
         sum(v[0] for v in b.values()) - sum(v[0] for v in a.values())))
print("major pages %d -> %d" % (sum(v[1] for v in a.values()), sum(v[1] for v in b.values())))

changed = [p for p in ra if p in rb and ra[p] != rb[p]]
print()
print("gate rows that changed a column: %d" % len(changed))
for p in changed:
    print("  %s" % os.path.basename(p))
    print("    base  %s" % "  ".join(ra[p]))
    print("    after %s" % "  ".join(rb[p]))
