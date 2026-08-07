#!/usr/bin/env python3
"""Predict the reference render from ours under several picture-recolour hypotheses.

The fit in fit-washout.py could not separate them: our page-one background is nearly a
constant dark, so `in` has almost no range and a least-squares slope is meaningless. This
applies each candidate transform to our pixels instead and reports how far the result is
from the reference's, which needs no range at all.
"""
import sys

sys.path.insert(0, __file__.rsplit('/', 1)[0])
import importlib.util as _util

_spec = _util.spec_from_file_location(
    'fit_washout', __file__.rsplit('/', 1)[0] + '/fit-washout.py')
_module = _util.module_from_spec(_spec)
_spec.loader.exec_module(_module)
read_png = _module.read_png


def lum_con(luminance, contrast):
    """basegfx::BColorModifier_RGBLuminanceContrast, as a function of one channel."""
    if contrast >= 0.0:
        slope = 128.0 / (128.0 - (contrast * 127.0))
    else:
        slope = (128.0 + (contrast * 127.0)) / 128.0
    offset = luminance + ((128.0 - slope * 128.0) / 255.0)
    return lambda v: min(max(v * slope + offset, 0.0), 1.0)


CANDIDATES = {
    'identity': lambda v: v,
    'washout (lum +0.5, con -0.7)': lum_con(0.5, -0.7),
    'bright 70% alone': lambda v: min(max(v + 0.7, 0.0), 1.0),
    'bright +0.7 then contrast -0.7': lum_con(0.7, -0.7),
}


def main():
    ours, ref = sys.argv[1], sys.argv[2]
    w1, h1, c1, r1 = read_png(ours)
    w2, h2, c2, r2 = read_png(ref)
    w, h = min(w1, w2), min(h1, h2)

    samples = []
    for y in range(h):
        for x in range(w):
            a = r1[y][x * c1:x * c1 + 3]
            b = r2[y][x * c2:x * c2 + 3]
            samples.append((a, b))

    print(f'{len(samples)} pixels over {w}x{h}')
    for name, f in CANDIDATES.items():
        total = 0.0
        for a, b in samples:
            for k in range(3):
                total += abs(f(a[k] / 255.0) - b[k] / 255.0)
        print(f'  {name:32s} mean |error| {total / (3 * len(samples)) * 255:6.2f} of 255')


if __name__ == '__main__':
    main()
