#!/usr/bin/env python3
"""Median row pitch on page 1 of two banked PDFs, for the ods rows of a group file."""
import os, re, subprocess, sys, statistics

BANK="/home/user/odsgap-work/bank"
def ys(pdf):
    out=subprocess.run(["pdftotext","-bbox","-f","1","-l","1",pdf,"-"],capture_output=True).stdout.decode("utf-8","replace")
    v=sorted({round(float(m),3) for m in re.findall(r'yMin="([0-9.]+)"', out)})
    return v
def pitch(v):
    d=[round(b-a,3) for a,b in zip(v,v[1:]) if 3 < b-a < 60]
    return statistics.median(d) if len(d)>=4 else None

rows=[]
for line in open(sys.argv[1],encoding="utf-8"):
    if line.startswith("#") or line.startswith("path\t"): continue
    f=line.rstrip("\n").split("\t")
    if len(f)<7 or f[1]!="ods" or f[3]!="pages-short": continue
    stem=os.path.basename(f[0]).rsplit(".",1)[0]+"__ods"
    o=pitch(ys(f"{BANK}/ours-head2/{stem}.pdf")); r=pitch(ys(f"{BANK}/ref/{stem}.pdf"))
    rows.append((os.path.basename(f[0]), f[4], o, r))
same=0; short=0; other=0
for n,p,o,r in rows:
    if o is None or r is None: tag="?"; other+=1
    elif abs(o-r) < 0.05: tag="pitch-equal"; same+=1
    elif o < r: tag="pitch-SHORT"; short+=1
    else: tag="pitch-long"; other+=1
    print(f"{tag:12} ours={o} ref={r} pages={p}  {n}")
print(f"\nequal {same}  ours-shorter {short}  other {other}  of {len(rows)}")
