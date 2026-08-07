#!/usr/bin/env python3
"""Does the fit's candidate grid follow the body's *first* character height or its largest?

`fit40lead.pptx` is `fit40.pptx` with a one-word 20 pt paragraph in front of three 40 pt
ones, so the two readings of `EE_CHAR_FONTHEIGHT` give different grids — 20.0126 pt and
39.99685 pt — on a body whose height model is still exactly computable.
"""
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sim_autofit_shim import LINES, MM100_PER_PT, fround, scaled_size_mm100  # noqa: E402

# Read off fit40lead-ref.pdf: (body /Tf in pt, spacing scale per cent).
REF = [
    (11.991, 100), (11.991, 100), (13.011, 80), (14.995, 80), (13.011, 100),
    (14.003, 100), (17.008, 90), (15.987, 100), (17.008, 100), (17.008, 100),
    (18.000, 80), (18.992, 80), (18.000, 90), (18.992, 90), (18.000, 100),
    (18.992, 100), (20.013, 100), (20.013, 100), (21.005, 100), (21.997, 100),
    (22.989, 80), (24.009, 80), (25.002, 80), (22.989, 90), (22.989, 90),
    (24.009, 90), (22.989, 100), (22.989, 100), (24.009, 100), (25.002, 100),
    (25.002, 100), (25.994, 100), (25.994, 100),
]


def line_mm(size_mm, spacing_pct, prop=0.8):
    return fround(fround(1.2 * size_mm) * prop * (spacing_pct / 100.0))


def height(scale_pct, spacing_pct):
    body_mm = scaled_size_mm100(40.0, scale_pct)
    lead_mm = scaled_size_mm100(20.0, scale_pct)
    body_pt = int(round(body_mm / MM100_PER_PT))
    return (line_mm(lead_mm, spacing_pct)
            + LINES[max(1, min(41, body_pt))] * line_mm(body_mm, spacing_pct))


def solve(box, grid_pt):
    fit = box / (height(100.0, 100.0) - 50)
    if fit >= 1.0:
        return 100.0, 100.0
    min_y, max_y = 0.0, 100.0
    best_font, best_spacing, best_fit = 0.0, 100.0, fit
    for _ in range(10):
        scale_y = min_y + (max_y - min_y) * 0.5
        rounded = int(grid_pt * (scale_y / 100.0) * 10.0) / 10.0
        font_scale = (rounded / grid_pt) * 100.0
        fit = 0.0
        for spacing in (100.0, 90.0, 80.0):
            if fit >= 1.0:
                continue
            h = height(font_scale, spacing) - 50
            fit = box / h if h > 0 else 0.0
            if spacing == 100.0:
                if fit > 1.0:
                    min_y = font_scale
                else:
                    max_y = font_scale
            if (best_fit < 1.0 and fit > best_fit) or (fit >= 1.0 and fit < best_fit):
                best_font, best_spacing, best_fit = font_scale, spacing, fit
    return best_font, best_spacing


if __name__ == '__main__':
    grids = {'first run, 20 pt as 706 mm100': 706 * 18.0 / 635.0,
             'largest run, 40 pt as 1411 mm100': 1411 * 18.0 / 635.0,
             'a round 12.0 pt (what we ship)': 12.0}
    for label, grid in grids.items():
        agree, misses = 0, []
        for i, (rtf, rs) in enumerate(REF):
            box = fround((80.0 + 10.0 * i) * 12700 / 360) + 1
            f, s = solve(box, grid)
            size_pt = round(scaled_size_mm100(40.0, f) / MM100_PER_PT)
            if size_pt == round(rtf) and abs(s - rs) < 0.5:
                agree += 1
            else:
                misses.append(f'p{i + 1} sim {size_pt}pt/{s:.0f} vs {round(rtf)}pt/{rs}')
        print(f'{label:36s} grid {grid:9.5f}  {agree}/{len(REF)}')
        for m in misses:
            print(f'    {m}')
