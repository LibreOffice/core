#!/usr/bin/env python3
"""Census: pivot tables whose row-label cells repeat down the sheet."""
import os, re, csv, zipfile, collections

ROOT = "/c/sandbox/workdir/sample-files"

def main():
    rows = []
    with open(os.path.join(ROOT, 'MANIFEST.tsv')) as fh:
        for r in csv.reader(fh, delimiter='\t'):
            if len(r) < 8 or r[0] == 'family': continue
            rows.append((r[0], r[7], r[2]))
    docs = 0; withpivot = []; withfill = []
    for fam, status, rel in sorted(rows):
        p = os.path.join(ROOT, rel)
        if not os.path.exists(p): continue
        try: z = zipfile.ZipFile(p)
        except Exception: continue
        docs += 1
        parts = [n for n in z.namelist() if re.match(r'xl/pivotTables/pivotTable\d*\.xml$', n)]
        if not parts: continue
        fill = 0; locs = []
        for n in parts:
            try: s = z.read(n).decode('utf8', 'replace')
            except Exception: continue
            fill += len(re.findall(r'fillDownLabels="1"', s))
            loc = re.search(r'<location[^>]*ref="([^"]+)"[^>]*firstDataRow="(\d+)"[^>]*firstDataCol="(\d+)"', s)
            rf = re.search(r'<rowFields count="(\d+)"', s)
            locs.append((loc.group(1) if loc else '?', rf.group(1) if rf else '0'))
        withpivot.append((fam, status, rel, len(parts), fill, locs))
        if fill: withfill.append((fam, status, rel, len(parts), fill, locs))
    print(f"zip documents scanned: {docs}")
    print(f"with a pivotTable part: {len(withpivot)}")
    print(f"with fillDownLabels=1 : {len(withfill)}")
    print("--- all pivot documents ---")
    for fam, status, rel, n, fill, locs in withpivot:
        print(f"  [{fam}/{status}] parts={n} fillDown={fill} {rel.split('/')[-1][:60]} {locs[:3]}")
main()
