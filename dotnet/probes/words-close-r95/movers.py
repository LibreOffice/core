#!/usr/bin/env python3
"""Which renderings two of our own sweeps disagree on, and how far each is from the reference.

Two `frame-area-r85/sweep-ours.py` runs at two binaries, both under `SOURCE_DATE_EPOCH`, so a
byte difference is a real difference and nothing else — `/CreationDate` and the header fields are
both pinned by it. The mover list is the whole reach of a change no gate column can see.

    movers.py <before/ours> <after/ours> [<reference dir> [dpi]]

With a reference directory it also prints, per mover, the mean per-page absolute grey difference
from the banked 26.2.4.2 rendering on each side — a *distance*, so it can fall as well as rise,
which is what makes `after` comparable with `before` at all.
"""
import pathlib
import subprocess
import sys
import tempfile

import numpy as np
from PIL import Image


def pagecount(pdf):
    out = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith('Pages:'):
            return int(line.split()[1])
    return 0


def page(pdf, n, dpi):
    with tempfile.TemporaryDirectory() as t:
        stem = pathlib.Path(t) / 'p'
        subprocess.run(['pdftoppm', '-r', str(dpi), '-gray', '-png', '-f', str(n), '-l', str(n),
                        '-singlefile', str(pdf), str(stem)], capture_output=True)
        f = pathlib.Path(str(stem) + '.png')
        return np.asarray(Image.open(f), dtype=np.int16) if f.exists() else None


def ink(a, b):
    h, w = min(a.shape[0], b.shape[0]), min(a.shape[1], b.shape[1])
    return float(np.abs(a[:h, :w] - b[:h, :w]).mean())


def score(ref, ours, dpi):
    n = min(pagecount(ref), pagecount(ours))
    if n == 0:
        return None
    total = 0.0
    for i in range(1, n + 1):
        r, o = page(ref, i, dpi), page(ours, i, dpi)
        if r is None or o is None:
            continue
        total += ink(r, o)
    return total / n


def main():
    before = pathlib.Path(sys.argv[1])
    after = pathlib.Path(sys.argv[2])
    ref = pathlib.Path(sys.argv[3]) if len(sys.argv) > 3 else None
    dpi = int(sys.argv[4]) if len(sys.argv) > 4 else 150

    names = sorted({p.name for p in before.glob('*.pdf')} | {p.name for p in after.glob('*.pdf')})
    moved = []
    missing = []
    for name in names:
        b, a = before / name, after / name
        if not b.exists() or not a.exists():
            missing.append(name)
            continue
        if b.read_bytes() != a.read_bytes():
            moved.append(name)

    print(f'{len(names)} renderings compared, {len(moved)} differ, {len(missing)} on one side only')
    for name in missing:
        print(f'  ONE SIDE ONLY  {name}')
    if not ref:
        for name in moved:
            print(f'  {name}')
        return

    print(f'{"before":>9}{"after":>9}{"delta":>9}  document')
    sb = sa = 0.0
    for name in moved:
        r = ref / name
        if not r.exists():
            print(f'{"-":>9}{"-":>9}{"-":>9}  {name}  (no banked reference)')
            continue
        vb, va = score(r, before / name, dpi), score(r, after / name, dpi)
        if vb is None or va is None:
            print(f'{"-":>9}{"-":>9}{"-":>9}  {name}  (unscoreable)')
            continue
        sb += vb
        sa += va
        print(f'{vb:9.3f}{va:9.3f}{va - vb:+9.3f}  {name}')
    print(f'{sb:9.3f}{sa:9.3f}{sa - sb:+9.3f}  TOTAL over the movers')


main()
