#!/usr/bin/env python3
"""Score one rendering of an autofit probe against the reference's, box by box.

Reports the chosen font size and spacing scale per page and how many of them agree, which
is the only thing an autofit change is allowed to be judged on.

    compare-fit.py ours.pdf ref.pdf --size 20 [--first-height 40] [--step 5]
"""
import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pdfops import dump  # noqa: E402

NATURAL = 1.20031


def answers(path, prop, ceiling=500.0):
    out = []
    page = 1
    while True:
        try:
            ops = [o for o in dump(path, page) if o[4] and o[4][1] <= ceiling]
        except (IndexError, KeyError):
            break
        if not ops:
            break
        size = max(o[2] for o in ops)
        ys = sorted({o[4][1] for o in ops if abs(o[2] - size) < 0.01}, reverse=True)
        gaps = [ys[i] - ys[i + 1] for i in range(len(ys) - 1)]
        pitch = min(gaps) if gaps else 0.0
        spacing = pitch / (NATURAL * size * prop) if pitch else 1.0
        out.append((size, spacing))
        page += 1
    return out


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('ours')
    ap.add_argument('ref')
    ap.add_argument('--spacing', type=int, default=80)
    ap.add_argument('--first-height', type=float, default=40.0)
    ap.add_argument('--step', type=float, default=5.0)
    a = ap.parse_args()

    prop = a.spacing / 100.0
    o, r = answers(a.ours, prop), answers(a.ref, prop)
    n = min(len(o), len(r))
    agree = 0
    print('page\thpt\tours pt/sp\tref pt/sp\tsame')
    for i in range(n):
        h = a.first_height + a.step * i
        same = (abs(o[i][0] - r[i][0]) < 0.05
                and abs(round(o[i][1] * 10) - round(r[i][1] * 10)) == 0)
        agree += same
        print(f'{i + 1}\t{h:.0f}\t{o[i][0]:6.3f} {o[i][1] * 100:5.1f}\t'
              f'{r[i][0]:6.3f} {r[i][1] * 100:5.1f}\t{"ok" if same else "NO"}')
    print(f'agree {agree}/{n}')
