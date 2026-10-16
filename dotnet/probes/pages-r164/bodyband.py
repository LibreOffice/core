#!/usr/bin/env python3
"""Empirical body band: per page, the highest and lowest text baseline once the
running head and foot are excluded by clustering on y across pages."""
import re, subprocess, sys, collections
PDFOPS="/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
pdf=sys.argv[1]; lo=int(sys.argv[2]); hi=int(sys.argv[3])
rows=[]
for p in range(lo,hi+1):
    out=subprocess.run(["python3",PDFOPS,"dump",pdf,"--page",str(p)],capture_output=True,text=True).stdout
    ys=set()
    for ln in out.splitlines():
        m=re.match(r'text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)',ln)
        if not m: continue
        txt=ln.split('show(s)',1)[1].strip() if 'show(s)' in ln else ''
        if not txt.strip('" '): continue
        ys.add(round(float(m.group(2)),2))
    rows.append((p,sorted(ys)))
# a y that appears on >70% of pages is running head/foot furniture
cnt=collections.Counter()
for p,ys in rows:
    for y in ys: cnt[y]+=1
n=len(rows); furn={y for y,c in cnt.items() if c> 0.7*n}
print("furniture y:",sorted(furn))
print(f"{'pg':>4} {'topbody':>8} {'botbody':>8} {'nlines':>6}")
mins=[]
for p,ys in rows:
    b=[y for y in ys if y not in furn]
    if not b: print(f"{p:4d}    (none)"); continue
    print(f"{p:4d} {max(b):8.2f} {min(b):8.2f} {len(b):6d}")
    mins.append(min(b))
print("LOWEST body baseline over the range:", min(mins))
