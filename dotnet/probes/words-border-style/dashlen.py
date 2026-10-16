"""Ink and gap lengths *along* a rule, read at 600 dpi. The companion to `rules.py`, which
measures across it. Usage: python3 dashlen.py <pdf>... -- the band is the probe's own y."""
import subprocess, sys, os, tempfile
import numpy as np
from PIL import Image
DPI=600
def runs(pdf, ytop_pt):
    with tempfile.TemporaryDirectory() as t:
        subprocess.run(["pdftoppm","-r",str(DPI),"-f","1","-l","1","-gray","-png",pdf,os.path.join(t,"p")],capture_output=True)
        m=sorted(os.listdir(t))
        a=np.asarray(Image.open(os.path.join(t,m[0])).convert("L")).astype(float)
    # find the densest row in a band around ytop
    y0=int((ytop_pt-2)*DPI/72); y1=int((ytop_pt+6)*DPI/72)
    band=a[y0:y1]
    row=band[(band<128).sum(axis=1).argmax()]
    on=row<128
    out=[];cur=on[0];n=0
    for v in on:
        if v==cur: n+=1
        else: out.append((("ink" if cur else "gap"), round(n*72/DPI,3))); cur=v; n=1
    out.append((("ink" if cur else "gap"), round(n*72/DPI,3)))
    return [p for p in out if p[1]>0.02][:14]
for a in sys.argv[1:]:
    print(a.split("/")[-1], runs(a, 85.4))
