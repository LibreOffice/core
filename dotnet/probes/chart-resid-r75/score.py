#!/usr/bin/env python3
"""Score a before/after pair of renderings against a reference PDF.

Reports page count, alphanumeric characters (the gate's own second column) and the mean
absolute grey difference per page at 30 dpi.

    score.py <ref.pdf> <before.pdf> <after.pdf>
"""
import os
import re
import subprocess
import sys
import tempfile

import numpy as np
import pymupdf
from PIL import Image

ALNUM = re.compile(r'[^0-9A-Za-z]')


def glyphs(path):
    d = pymupdf.open(path)
    return sum(len(ALNUM.sub('', pg.get_text())) for pg in d)


def pages(path, dpi=30):
    with tempfile.TemporaryDirectory() as t:
        subprocess.run(['pdftoppm', '-r', str(dpi), '-gray', '-png', path,
                        os.path.join(t, 'p')], capture_output=True)
        return [np.asarray(Image.open(os.path.join(t, n)).convert('L')).astype(float)
                for n in sorted(os.listdir(t))]


def ink(a, b):
    out = []
    for k in range(min(len(a), len(b))):
        x, y = a[k], b[k]
        h, w = min(x.shape[0], y.shape[0]), min(x.shape[1], y.shape[1])
        out.append(float(np.abs(x[:h, :w] - y[:h, :w]).mean()))
    return out


def main():
    ref, before, after = sys.argv[1:4]
    pr, pb, pa = pages(ref), pages(before), pages(after)
    ib, ia = ink(pr, pb), ink(pr, pa)
    print(f'pages      ref {len(pr)}  before {len(pb)}  after {len(pa)}')
    print(f'alnum      ref {glyphs(ref)}  before {glyphs(before)}  after {glyphs(after)}')
    print(f'mean ink   before {sum(ib) / len(ib):.4f}  after {sum(ia) / len(ia):.4f}')
    print(f'worst page before {max(ib):.4f}  after {max(ia):.4f}')
    for k, (x, y) in enumerate(zip(ib, ia), 1):
        if abs(x - y) > 0.0005:
            print(f'  page {k:3d}  {x:8.4f} -> {y:8.4f}   {y - x:+.4f}')


if __name__ == '__main__':
    main()
