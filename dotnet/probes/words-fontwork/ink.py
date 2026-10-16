import subprocess, sys, tempfile, os
import numpy as np
from PIL import Image

def page(pdf, n, dpi=100):
    with tempfile.TemporaryDirectory() as t:
        subprocess.run(["pdftoppm","-r",str(dpi),"-f",str(n),"-l",str(n),"-gray","-png",pdf,os.path.join(t,"p")],capture_output=True)
        m=sorted(os.listdir(t))
        return None if not m else np.asarray(Image.open(os.path.join(t,m[0])).convert("L")).astype(float)

a,b = sys.argv[1], sys.argv[2]
pages = [int(x) for x in sys.argv[3].split(",")] if len(sys.argv)>3 else range(1,53)
tot=[]
for n in pages:
    x,y = page(a,n), page(b,n)
    if x is None or y is None: print(n,"missing"); continue
    h,w = min(x.shape[0],y.shape[0]), min(x.shape[1],y.shape[1])
    d = round(float(np.abs(x[:h,:w]-y[:h,:w]).mean()),2)
    ia = round(float((255-x).mean())/255*100,2); ib = round(float((255-y).mean())/255*100,2)
    tot.append(d)
    print(f"page {n:3d}  div {d:6.2f}   inkA {ia:6.2f}   inkB {ib:6.2f}")
print(f"mean {sum(tot)/len(tot):.3f}  over {len(tot)}")
