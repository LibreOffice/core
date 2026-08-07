#!/usr/bin/env python3
"""Read the autofit answer back off a probe rendering: font scale and spacing scale.

Per page of `make-autofit-probe.py`'s deck: the measured box's `/Tf` size gives the font
scale directly, and the baseline pitch divided by `natural x proportion x size` gives the
spacing scale. `natural` is the face's own line ratio, measured once off an unshrunk box
rather than assumed — 1.20031 for DejaVu Sans here, which is close enough to the 1.2 of
Impress's font-independent line spacing that the two cannot be told apart on this face.

    read-autofit.py <pdf> --spacing 80 --size 20 [--natural 1.20031]
"""
import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pdfops import dump  # noqa: E402


def rows(path, size, prop, natural, ceiling=500.0):
    """`ceiling` separates the spacer from the measured box by position, not by size.

    Filtering the spacer out by its 12 pt height silently drops every page whose fit
    landed on 12 pt too — six of this probe's thirty-three — and the loop then stops at
    the first of them looking like a short document.
    """
    out = []
    page = 1
    while True:
        try:
            ops = dump(path, page)
        except (IndexError, KeyError):
            break
        ys, fs = [], None
        for _font, _base, sz, _tm, td in ops:
            if sz is None or td is None or td[1] > ceiling:
                continue
            fs = sz if fs is None else fs
            ys.append(td[1])
        if fs is None:
            page += 1
            if page > 500:
                break
            continue
        ys = sorted(set(ys), reverse=True)
        gaps = [round(ys[i] - ys[i + 1], 4) for i in range(len(ys) - 1)]
        pitch = min(gaps) if gaps else 0.0
        out.append((page, fs, len(ys), pitch,
                    fs / size,
                    pitch / (natural * fs * prop) if fs and pitch else 0.0))
        page += 1
    return out


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('pdf')
    ap.add_argument('--spacing', type=int, default=80)
    ap.add_argument('--size', type=float, default=20.0)
    ap.add_argument('--natural', type=float, default=1.20031)
    a = ap.parse_args()
    print('page\tTf\tlines\tpitch\tfontScale\tspacingScale')
    for r in rows(a.pdf, a.size, a.spacing / 100.0, a.natural):
        print(f'{r[0]}\t{r[1]:.4f}\t{r[2]}\t{r[3]:.4f}\t{r[4]:.5f}\t{r[5]:.5f}')
