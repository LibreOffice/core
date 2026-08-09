#!/usr/bin/env python3
"""Every probe in a directory: LibreOffice's model rectangle, its ink, ours."""
import sys, os, glob, subprocess, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from region import regions
from inkrect import fillrect

D = sys.argv[1]
print(f"{'probe':<14} {'L.model':>8} {'L.ink':>8} {'ours':>8} {'dL':>6} "
      f"{'B.model':>8} {'B.ink':>8} {'ours':>8} {'dB':>6} {'dR':>6} {'dT':>6}")
for f in sorted(glob.glob(os.path.join(D, "*.pptx"))):
    b = os.path.basename(f)[:-5]
    odp=os.path.join(D,"odp",b+".odp"); ours=os.path.join(D,"ours",b+".pdf"); ref=os.path.join(D,"ref",b+".pdf")
    rs = regions(odp) if os.path.exists(odp) else []
    if not rs: print(f"{b:<14} no region"); continue
    r = rs[0]
    out = subprocess.run(["pdfinfo", ours], capture_output=True, text=True).stdout
    H = float(re.search(r"Page size:\s+([\d.]+) x ([\d.]+)", out).group(2))
    x,y,w,h = r["region"]; model=(x,H-(y+h),x+w,H-y)
    a=fillrect(ours,1,"#D9D9D9"); bb=fillrect(ref,1,"#D9D9D9")
    if not a: print(f"{b:<14} no fill in ours"); continue
    print(f"{b:<14} {model[0]:>8.2f} {(bb[0] if bb else 0):>8.2f} {a[0]:>8.2f} {a[0]-model[0]:>6.2f} "
          f"{model[1]:>8.2f} {(bb[1] if bb else 0):>8.2f} {a[1]:>8.2f} {a[1]-model[1]:>6.2f} "
          f"{a[2]-model[2]:>6.2f} {a[3]-model[3]:>6.2f}")
