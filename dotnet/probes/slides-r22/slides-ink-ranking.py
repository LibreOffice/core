#!/usr/bin/env python3
"""The slides track's ink ranking, with the two known ceilings subtracted per page."""
import collections, os, sys
SC = os.path.dirname(os.path.abspath(__file__))

alt = collections.defaultdict(set)
for line in open(f"{SC}/altcontent.tsv"):
    d, p, _ = line.rstrip("\n").split("\t")
    stem = d.split("/")[-1].rsplit(".", 1)[0]; ext = d.rsplit(".", 1)[1].lower()
    alt[f"{stem}__{ext}"].add(int(p))

raster = collections.defaultdict(set)
head = True
for line in open(f"{SC}/inkrank-pages.tsv"):
    if head: head = False; continue
    doc, page, strict, loose = line.rstrip("\n").split("\t")
    if strict == "1": raster[doc].add(int(page))

rows = []
for name in sorted(os.listdir(f"{SC}/sweep-base/cmp")):
    doc = name[:-4]
    pages = {}
    for line in open(f"{SC}/sweep-base/cmp/{name}", errors="replace"):
        f = line.rstrip("\n").split("\t")
        if len(f) >= 6 and f[0].isdigit(): pages[int(f[0])] = (float(f[2]), f[5] == "MAJOR")
    if not pages: continue
    ceil = alt[doc] | raster[doc]
    total = sum(v[0] for v in pages.values())
    resid = sum(v for p, (v, _) in pages.items() if p not in ceil)
    cmaj = sum(1 for p, (_, m) in pages.items() if m and p in ceil)
    maj = sum(1 for _, m in pages.values() if m)
    rows.append((doc, len(pages), total, resid, len(ceil & set(pages)), maj, cmaj))

rows.sort(key=lambda r: -r[3])
print("doc\tpages\tink\tink_residual\tceiling_pages\tmajor\tmajor_on_ceiling")
for r in rows:
    print(f"{r[0]}\t{r[1]}\t{r[2]:.2f}\t{r[3]:.2f}\t{r[4]}\t{r[5]}\t{r[6]}")
print(f"# TOTAL ink {sum(r[2] for r in rows):.2f}  residual {sum(r[3] for r in rows):.2f}  "
      f"ceiling pages {sum(r[4] for r in rows)}  major {sum(r[5] for r in rows)}  "
      f"major on ceiling {sum(r[6] for r in rows)}", file=sys.stderr)
