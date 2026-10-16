#!/usr/bin/env python3
"""Ink between two banks of renderings and a reference bank, document by document.

Ink is the mean absolute grey difference at 30 dpi, page for page, over the shared pages —
the same measure as `probes/chart-layout/ink.py`, taken here from banked PDFs rather than
rendered on the spot, so the same documents can be scored twice without re-rendering.

    ink.py --ref <dir> --before <dir> --after <dir> < paths
"""
import argparse, hashlib, os, subprocess, sys, tempfile
from pathlib import Path
import numpy as np
from PIL import Image

ap = argparse.ArgumentParser()
ap.add_argument('--ref', required=True)
ap.add_argument('--before', required=True)
ap.add_argument('--after', required=True)
ap.add_argument('--corpus', default='/home/user/sample-files')
ap.add_argument('--dpi', type=int, default=30)
ap.add_argument('--work', default=None)
args = ap.parse_args()

CORPUS = Path(args.corpus)


def key(rel):
    return hashlib.sha1(rel.encode('utf-8')).hexdigest()[:16]


def pdf(bank, rel):
    p = Path(bank) / key(rel) / (Path(rel).stem + '.pdf')
    return p if p.exists() else None


def pages(path):
    with tempfile.TemporaryDirectory(dir=args.work) as t:
        subprocess.run(['pdftoppm', '-r', str(args.dpi), '-gray', '-png', str(path),
                        os.path.join(t, 'p')], capture_output=True)
        names = sorted(os.listdir(t))
        return [np.asarray(Image.open(os.path.join(t, n)).convert('L')).astype(float)
                for n in names]


def ink(a, b):
    n = min(len(a), len(b))
    vals = []
    for k in range(n):
        x, y = a[k], b[k]
        h, w = min(x.shape[0], y.shape[0]), min(x.shape[1], y.shape[1])
        vals.append(float(np.abs(x[:h, :w] - y[:h, :w]).mean()))
    return vals


rows = [l.strip() for l in sys.stdin if l.strip() and not l.startswith('#')]
print('path\tpages_ref\tpages_before\tpages_after\tmean_before\tmean_after'
      '\tworst_before\tworst_after\tworstpage')
for rel in rows:
    r, b, a = pdf(args.ref, rel), pdf(args.before, rel), pdf(args.after, rel)
    if r is None or b is None or a is None:
        print(f'{rel}\tMISSING\t{r is not None}\t{b is not None}\t{a is not None}\t-\t-\t-\t-')
        sys.stdout.flush()
        continue
    rp, bp, ap_ = pages(r), pages(b), pages(a)
    vb, va = ink(bp, rp), ink(ap_, rp)
    if not vb or not va:
        print(f'{rel}\t{len(rp)}\t{len(bp)}\t{len(ap_)}\tNO-PAGES\t-\t-\t-\t-')
        sys.stdout.flush()
        continue
    worst = max(range(len(va)), key=lambda k: va[k])
    print(f'{rel}\t{len(rp)}\t{len(bp)}\t{len(ap_)}'
          f'\t{sum(vb)/len(vb):.3f}\t{sum(va)/len(va):.3f}'
          f'\t{max(vb):.3f}\t{max(va):.3f}\t{worst+1}')
    sys.stdout.flush()
