#!/usr/bin/env python3
"""Which region signatures recur across a document's pages, and on how many.

A defect in a deck's page furniture shows up as the *same* box with the same hint on
page after page. Grouping the report's region lines by (hint, box) turns a long thin ink
figure into "one thing, repeated N times", which is the difference between a tail and a
lead.
"""
import re, sys, collections

path = sys.argv[1]
top = int(sys.argv[2]) if len(sys.argv) > 2 else 12

groups = collections.Counter()
pages = collections.defaultdict(set)
page = None
for line in open(path, encoding="utf-8", errors="replace"):
    f = line.rstrip("\n").split("\t")
    if len(f) >= 3 and re.fullmatch(r"\d+", f[0]):
        page = int(f[0])
        continue
    m = re.search(r"^\s+(.*?) \((\d+\.\d+)% of page, (x [\d.]+-[\d.]+, y [\d.]+-[\d.]+)\)", line)
    if m and page is not None:
        key = (m.group(1), m.group(3))
        groups[key] += 1
        pages[key].add(page)

print("%-6s %-7s %s" % ("pages", "share", "region"))
for (hint, box), n in groups.most_common(top):
    print("%-6d %-7s %s" % (n, box.split(",")[0][:0] or "", hint))
    print("       %s" % box)
