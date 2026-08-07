#!/usr/bin/env python3
"""Total diff% and ink% over a document's pages, for two cmp reports.

diff% is the raw share of pixels that differ and is computed per page, so it is not
affected by how the tool groups differing pixels into regions. ink% is measured over
regions, so a change that merges or grows regions can raise it while the page gets
strictly closer. Reporting both separates those.
"""
import re, sys

def read(path):
    diff, ink, major = {}, {}, {}
    for line in open(path, encoding="utf-8", errors="replace"):
        f = line.rstrip("\n").split("\t")
        if len(f) >= 3 and re.fullmatch(r"\d+", f[0]) and re.fullmatch(r"[\d.]+", f[2]):
            p = int(f[0])
            diff[p] = float(f[1])
            ink[p] = float(f[2])
            major[p] = "MAJOR" in line
    return diff, ink, major

ad, ai, am = read(sys.argv[1])
bd, bi, bm = read(sys.argv[2])
pages = sorted(set(ad) | set(bd))

print("%-28s %10s %10s %10s" % ("", "base", "after", "delta"))
print("%-28s %10.2f %10.2f %+10.2f"
      % ("total diff% (pixels)", sum(ad.values()), sum(bd.values()),
         sum(bd.values()) - sum(ad.values())))
print("%-28s %10.2f %10.2f %+10.2f"
      % ("total ink% (regions)", sum(ai.values()), sum(bi.values()),
         sum(bi.values()) - sum(ai.values())))
print("%-28s %10d %10d %+10d"
      % ("major pages", sum(am.values()), sum(bm.values()),
         sum(bm.values()) - sum(am.values())))
better = sum(1 for p in pages if bd.get(p, 0) < ad.get(p, 0) - 0.005)
worse = sum(1 for p in pages if bd.get(p, 0) > ad.get(p, 0) + 0.005)
print("%-28s %10d %10d" % ("pages closer / further (diff%)", better, worse))
