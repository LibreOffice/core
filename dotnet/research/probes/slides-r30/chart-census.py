#!/usr/bin/env python3
"""Which corpus slide documents carry chart parts, and how many."""
import sys, os, zipfile, re
root = "/workspace/sample-files/slides"
rows = []
for dirpath, _, names in os.walk(root):
    for n in sorted(names):
        p = os.path.join(dirpath, n)
        cnt = 0
        try:
            with zipfile.ZipFile(p) as z:
                cnt = sum(1 for m in z.namelist()
                          if re.match(r"ppt/charts/chart\d+\.xml$", m))
        except Exception:
            # binary .ppt — chart parts live in an embedded OLE, not countable here
            cnt = -1
        if cnt != 0:
            rows.append((os.path.relpath(p, root), cnt))
for r, c in sorted(rows, key=lambda t: -t[1]):
    if c > 0: print(f"{c}\t{r}")
print(f"# {sum(1 for _,c in rows if c>0)} documents with OOXML chart parts, "
      f"{sum(c for _,c in rows if c>0)} parts")
