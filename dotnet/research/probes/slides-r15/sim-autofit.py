#!/usr/bin/env python3
"""Simulate 24.2.7's `SdrTextObj::autoFitTextForCompatibility` and check it against the binary.

The height model is not assumed: line counts and natural line heights come from
`make-autofit-probe.py --sizes`, an unshrunk rendering of the same text at every size, so
the only thing being tested is the search.

    sim-autofit.py --grid 12 --grid 20      # which EE_CHAR_FONTHEIGHT reproduces the binary
"""
import argparse

MM100_PER_PT = 2540.0 / 72.0

# lines(size in whole pt) for the probe's text at 360 pt wide, three paragraphs.
# Measured on both renderers' unshrunk output; they agree at every size.
LINES = {s: 6 for s in range(1, 13)}
LINES.update({s: 9 for s in range(13, 18)})
LINES.update({s: 12 for s in range(18, 25)})


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
    if prop >= 1.0:
        line = fround(natural * (spacing_pct / 100.0))
    else:
        line = fround(natural * prop * (spacing_pct / 100.0))
    return LINES[max(1, min(24, size_pt))] * line


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
            trace.append((round(font_scale, 4), spacing, round(fit, 4)))
            if spacing == 100.0:
                if fit > 1.0:
                    min_y = font_scale
                else:
                    max_y = font_scale
            if (best_fit < 1.0 and fit > best_fit) or (fit >= 1.0 and fit < best_fit):
                best_font, best_spacing, best_fit = font_scale, spacing, fit

    return best_font, best_spacing, best_fit, trace


# What the binary actually produced, read off fit80-ref.pdf: (font scale %, spacing %).
REF = [
    (39.970, 90), (50.030, 80), (45.070, 90), (45.070, 100), (50.030, 100),
    (54.990, 100), (59.955, 100), (59.955, 100), (59.955, 100), (59.955, 100),
    (65.055, 80), (65.055, 80), (65.055, 90), (74.975, 80), (70.015, 90),
    (65.055, 100), (70.015, 100), (70.015, 100), (74.975, 100), (85.040, 90),
    (79.935, 100), (79.935, 100), (85.040, 100), (85.040, 100), (85.040, 100),
    (90.000, 80), (90.000, 80), (94.960, 80), (94.960, 80), (94.960, 80),
    (90.000, 90), (90.000, 90), (94.960, 90),
]


def brute(box_mm100, base_pt, prop=0.8):
    """The tightest fit over every whole-point size and every spacing the search allows.

    Not an algorithm anyone implements — it is the yardstick that says whether the
    bisection's answer is the best one available, or merely the best one it looked at.
    """
    best = None
    for size_pt in range(1, int(base_pt) + 1):
        scale = size_pt / base_pt * 100.0
        for spacing in (100.0, 90.0, 80.0):
            h = text_height(base_pt, scale, spacing, prop) - 50
            if h <= 0:
                continue
            fit = box_mm100 / h
            if fit < 1.0:
                continue
            if best is None or fit < best[2]:
                best = (size_pt, spacing, fit)
    return best


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--brute', action='store_true',
                    help='compare the binary against the tightest fit that exists')
    ap.add_argument('--grid', type=float, action='append', default=None)
    ap.add_argument('--trace', type=int, help='1-based page to print the search trace for')
    a = ap.parse_args()
    if a.brute:
        agree = 0
        print('page\thpt\tbest size/spacing\tfit\tbinary size/spacing\tsame')
        for i, (rf, rs) in enumerate(REF):
            h_pt = 40.0 + 5.0 * i
            box = fround(h_pt * 12700 / 360) + 1
            size, spacing, fit = brute(box, 20.0)
            same = size == round(20.0 * rf / 100.0) and abs(spacing - rs) < 0.5
            agree += same
            print(f'{i + 1}\t{h_pt:.0f}\t{size:6d} {spacing:5.0f}\t\t{fit:.4f}\t'
                  f'{round(20.0 * rf / 100.0):6d} {rs:5.0f}\t\t{"ok" if same else "NO"}')
        print(f'agree {agree}/{len(REF)}')
        raise SystemExit(0)

    grids = a.grid or [12.0, 20.0]

    for grid in grids:
        agree = 0
        print(f'--- EE_CHAR_FONTHEIGHT grid = {grid} pt ---')
        print('page\thpt\tsim font/spacing\tbinary font/spacing\tsame')
        for i, (rf, rs) in enumerate(REF):
            h_pt = 40.0 + 5.0 * i
            box = fround(h_pt * 12700 / 360) + 1
            f, s, fit, trace = solve(box, 20.0, grid)
            # The simulated scale is a percentage of the stated size; the binary's is read
            # back through the mm100 round trip, so compare the whole points they land on.
            same = (round(scaled_size_mm100(20.0, f) / MM100_PER_PT) ==
                    round(20.0 * rf / 100.0)) and abs(s - rs) < 0.5
            agree += same
            print(f'{i + 1}\t{h_pt:.0f}\t{f:8.3f} {s:5.0f}\t\t{rf:8.3f} {rs:5.0f}\t\t{"ok" if same else "NO"}')
            if a.trace == i + 1:
                for t in trace:
                    print('    ', t)
        print(f'agree {agree}/{len(REF)}\n')
