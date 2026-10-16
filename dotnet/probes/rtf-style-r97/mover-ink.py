#!/usr/bin/env python3
r"""Score two legs of `reach-sweep.py`'s movers against the banked 26.2.4.2 reference.

`reach-sweep.py` says *which* renderings a change moves; this says whether each moved towards the
reference or away from it, as the mean per-page absolute grey difference at 150 dpi -- a distance,
so `before` and `after` are comparable and either can be the smaller. `probes/words-close-r95/movers.py`
is the same measure over a whole sweep; this one takes two directories of already-rendered PDFs,
because the four documents worth scoring here include two of 665 and 746 pages and re-rendering
them per leg costs more than the scoring does.

The reference half is `/home/user/gate-odf-r80/ref`, which holds 336 `__rtf.pdf` of the corpus's
338 -- so a mover with no banked reference is reported as such rather than skipped silently.

  mover-ink.py <dir of before/*.pdf> <dir of after/*.pdf>
"""
import pathlib, subprocess, sys, tempfile
import numpy as np
from PIL import Image
def pagecount(pdf):
    out = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith('Pages:'): return int(line.split()[1])
    return 0
def page(pdf, n, dpi):
    with tempfile.TemporaryDirectory() as t:
        stem = pathlib.Path(t)/'p'
        subprocess.run(['pdftoppm','-r',str(dpi),'-gray','-png','-f',str(n),'-l',str(n),
                        '-singlefile',str(pdf),str(stem)], capture_output=True)
        f = pathlib.Path(str(stem)+'.png')
        return np.asarray(Image.open(f), dtype=np.int16) if f.exists() else None
def ink(a,b):
    h,w = min(a.shape[0],b.shape[0]), min(a.shape[1],b.shape[1])
    return float(np.abs(a[:h,:w]-b[:h,:w]).mean())
def score(ref, ours, dpi=150):
    n = min(pagecount(ref), pagecount(ours))
    if n == 0: return None, 0, 0
    tot = 0.0
    for i in range(1, n+1):
        r, o = page(ref,i,dpi), page(ours,i,dpi)
        if r is None or o is None: continue
        tot += ink(r,o)
    return tot/n, pagecount(ref), pagecount(ours)
refdir = pathlib.Path('/home/user/gate-odf-r80/ref')
base = pathlib.Path(sys.argv[1]); head = pathlib.Path(sys.argv[2])
print(f'{"document":<52} {"pages ref/ours":<16} {"before":>9} {"after":>9}   ')
for p in sorted(base.glob('*.pdf')):
    ref = refdir / (p.stem + '__rtf.pdf')
    if not ref.exists(): print(f'{p.stem:<52} no reference'); continue
    b, rp, op = score(ref, p)
    a, _, op2 = score(ref, head/p.name)
    mark = 'better' if a < b - 1e-9 else ('worse' if a > b + 1e-9 else 'level')
    print(f'{p.stem[:52]:<52} {rp}/{op:<13} {b:9.4f} {a:9.4f}   {mark}')
