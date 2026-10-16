"""Mean absolute grey difference at 72 dpi, page for page, between two renderings.

The project's own ink measure (probes/chart-layout/ink.py), at a resolution high enough that a
connector's bend moving by a few points is visible.
"""
import subprocess, sys, tempfile, pathlib
import numpy as np
from PIL import Image

def pages(pdf, dpi, out):
    subprocess.run(['pdftoppm','-gray','-r',str(dpi),'-png',pdf,str(out/'p')],check=True)
    return sorted(out.glob('p*.png'))

def ink(a, b, dpi=72):
    with tempfile.TemporaryDirectory() as t1, tempfile.TemporaryDirectory() as t2:
        pa=pages(a,dpi,pathlib.Path(t1)); pb=pages(b,dpi,pathlib.Path(t2))
        vals=[]
        for x,y in zip(pa,pb):
            ia=np.asarray(Image.open(x).convert('L'),dtype=np.float64)
            ib=np.asarray(Image.open(y).convert('L'),dtype=np.float64)
            h=min(ia.shape[0],ib.shape[0]); w=min(ia.shape[1],ib.shape[1])
            vals.append(float(np.abs(ia[:h,:w]-ib[:h,:w]).mean()))
        return vals, len(pa), len(pb)

if __name__=='__main__':
    vals,na,nb=ink(sys.argv[1],sys.argv[2])
    print(f'pages {na}/{nb} mean {np.mean(vals):.4f} worst {max(vals):.4f}')
    for i,v in enumerate(vals,1): print(f'  p{i}\t{v:.4f}')
