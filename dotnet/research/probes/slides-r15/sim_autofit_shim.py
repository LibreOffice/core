#!/usr/bin/env python3
"""Simulate 24.2.7's `SdrTextObj::autoFitTextForCompatibility` and check it against the binary.

The height model is not assumed. Line counts come from `make-autofit-probe.py --sizes`, an
unshrunk rendering of the same text at every size, so the only thing under test is the
search — and specifically the one quantity the search reads that is not a length:
`fFontHeightPt`, the grid each candidate scale is floored to a tenth of a point of.

    sim-autofit.py                       # both probes, the grid candidates that matter
    sim-autofit.py --grid 12 --trace 20  # why one box comes out where it does
"""
import argparse

MM100_PER_PT = 2540.0 / 72.0

# lines(size in whole pt) for the probe's text at 360 pt wide, three paragraphs.
# Measured on unshrunk renderings at every size from 5 to 40; both renderers agree.
LINES = {}
for _lo, _hi, _n in ((1, 6, 3), (7, 12, 6), (13, 17, 9), (18, 22, 12),
                     (23, 29, 15), (30, 34, 18), (35, 41, 21)):
    for _s in range(_lo, _hi + 1):
        LINES[_s] = _n


def fround(x):
    return int(x + 0.5) if x >= 0 else -int(-x + 0.5)


def round_to_nearest_pt(mm100):
    return fround(mm100 * 72.0 / 2540.0) * MM100_PER_PT


def scaled_size_mm100(base_pt, scale_pct):
    """`SeekCursor`'s two roundings, and only when the scale is not 100."""
    base = fround(base_pt * MM100_PER_PT)
    if scale_pct == 100.0:
        return base
    h = round_to_nearest_pt(base) * (scale_pct / 100.0)
    return fround(round_to_nearest_pt(h))


def text_height(base_pt, scale_pct, spacing_pct, prop=0.8):
    """Height of the whole body, in 1/100 mm, at one (font scale, spacing scale)."""
    mm = scaled_size_mm100(base_pt, scale_pct)
    size_pt = int(round(mm / MM100_PER_PT))
    natural = fround(1.2 * mm)
    line = fround(natural * (prop if prop < 1.0 else 1.0) * (spacing_pct / 100.0))
    return LINES[max(1, min(41, size_pt))] * line


def solve(box_mm100, base_pt, grid_pt, prop=0.8, iterations=10):
    """`autoFitTextForCompatibility`, 24.2.7, transcribed."""
    trace = []
    height = text_height(base_pt, 100.0, 100.0, prop) - 50
    fit = box_mm100 / height
    if fit >= 1.0:
        return 100.0, 100.0, fit, trace

    min_y, max_y = 0.0, 100.0
    best_font, best_spacing, best_fit = 0.0, 100.0, fit

    for _ in range(iterations):
        scale_y = min_y + (max_y - min_y) * 0.5
        rounded = int(grid_pt * (scale_y / 100.0) * 10.0) / 10.0
        font_scale = (rounded / grid_pt) * 100.0

        fit = 0.0
        for spacing in (100.0, 90.0, 80.0):
            if fit >= 1.0:
                continue
            h = text_height(base_pt, font_scale, spacing, prop) - 50
            fit = box_mm100 / h if h > 0 else 0.0
            trace.append((round(font_scale, 4),
                          round(scaled_size_mm100(base_pt, font_scale) / MM100_PER_PT),
                          spacing, round(fit, 4)))
            if spacing == 100.0:
                if fit > 1.0:
                    min_y = font_scale
                else:
                    max_y = font_scale
            if (best_fit < 1.0 and fit > best_fit) or (fit >= 1.0 and fit < best_fit):
                best_font, best_spacing, best_fit = font_scale, spacing, fit

    return best_font, best_spacing, best_fit, trace


# What the binary produced, read off the two probe renderings with read-autofit.py:
# (stated size, first box height, step, [(font scale %, spacing %), ...]).
PROBES = {
    '20pt': (20.0, 40.0, 5.0, [
        (39.970, 90), (50.030, 80), (45.070, 90), (45.070, 100), (50.030, 100),
        (54.990, 100), (59.955, 100), (59.955, 100), (59.955, 100), (59.955, 100),
        (65.055, 80), (65.055, 80), (65.055, 90), (74.975, 80), (70.015, 90),
        (65.055, 100), (70.015, 100), (70.015, 100), (74.975, 100), (85.040, 90),
        (79.935, 100), (79.935, 100), (85.040, 100), (85.040, 100), (85.040, 100),
        (90.000, 80), (90.000, 80), (94.960, 80), (94.960, 80), (94.960, 80),
        (90.000, 90), (90.000, 90), (94.960, 90),
    ]),
    '40pt': (40.0, 80.0, 10.0, [
        (29.978, 100), (32.527, 80), (32.527, 90), (35.008, 90), (35.008, 100),
        (37.487, 100), (39.968, 100), (42.520, 100), (42.520, 100), (45.000, 80),
        (47.480, 80), (45.000, 90), (47.480, 90), (50.033, 90), (47.480, 100),
        (50.033, 100), (50.033, 100), (52.512, 100), (54.992, 100), (57.473, 80),
        (60.023, 80), (62.505, 80), (57.473, 90), (60.023, 90), (60.023, 90),
        (57.473, 100), (57.473, 100), (60.023, 100), (62.505, 100), (62.505, 100),
        (64.985, 100), (67.535, 100), (67.535, 100),
    ]),
}

# The grid candidates worth separating. Each is EE_CHAR_FONTHEIGHT read as hundredths of a
# millimetre and converted to points, which is what autoFitTextForCompatibility does with
# whatever number the object's item set holds.
GRIDS = {
    '240 mm100 (EditEngine default, read as mm100)': 240 * 18.0 / 635.0,
    '423 mm100 (twelve points)': 423 * 18.0 / 635.0,
    'the run\'s own size, 20 pt': 706 * 18.0 / 635.0,
    'the run\'s own size, 40 pt': 1411 * 18.0 / 635.0,
    'a round 12.0 pt (what we ship)': 12.0,
}


def run(name, grid, verbose=False):
    base, first, step, ref = PROBES[name]
    agree = 0
    for i, (rf, rs) in enumerate(ref):
        h_pt = first + step * i
        box = fround(h_pt * 12700 / 360) + 1
        f, s, fit, trace = solve(box, base, grid)
        same = (round(scaled_size_mm100(base, f) / MM100_PER_PT) ==
                round(base * rf / 100.0)) and abs(s - rs) < 0.5
        agree += same
        if verbose and not same:
            print(f'  page {i + 1} h={h_pt:.0f}: sim '
                  f'{round(scaled_size_mm100(base, f) / MM100_PER_PT)}pt/{s:.0f} vs binary '
                  f'{round(base * rf / 100.0)}pt/{rs}')
    return agree, len(ref)


