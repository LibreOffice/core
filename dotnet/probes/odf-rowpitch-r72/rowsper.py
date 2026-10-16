#!/usr/bin/env python3
import os, re, subprocess, sys
BANK="/home/user/odsgap-work/bank"
def ys(pdf):
    out=subprocess.run(["pdftotext","-bbox","-f","1","-l","1",pdf,"-"],capture_output=True).stdout.decode("utf-8","replace")
    return sorted({round(float(m),3) for m in re.findall(r'yMin="([0-9.]+)"', out)})
for line in open(sys.argv[1],encoding="utf-8"):
    if line.startswith("#") or line.startswith("path\t"): continue
    f=line.rstrip("\n").split("\t")
    if len(f)<7 or f[1]!="ods" or f[3]!="pages-short": continue
    stem=os.path.basename(f[0]).rsplit(".",1)[0]+"__ods"
    o=ys(f"{BANK}/ours-head2/{stem}.pdf"); r=ys(f"{BANK}/ref/{stem}.pdf")
    if not o or not r: print(f"?    {f[4]:10}  {os.path.basename(f[0])[:48]}"); continue
    print(f"rows o={len(o):4} r={len(r):4}  top o={o[0]:8.2f} r={r[0]:8.2f}  bot o={o[-1]:8.2f} r={r[-1]:8.2f}  {f[4]:9} {os.path.basename(f[0])[:44]}")
