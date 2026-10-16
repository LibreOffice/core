#!/usr/bin/env python3
"""Score two renderings of the same document against a reference, two ways.

`ink` is the share of page one where exactly one of the two has ink. It is what
a first-page ink ranking measures, and it is **displacement-sensitive**: a glyph
that moves an eighth of a point can cross a pixel boundary and flip every pixel
of its own outline, so it reports a document as worse when the geometry moved
closer. That is not hypothetical -- see `results.md`, where it called
`FO.FCTOA.00010` 0.76 worse while its first body line moved 0.08 pt nearer the
reference with the page count unchanged.

`dev` is the mean |dy| over page-one words paired by text and document order. It
cannot be flipped by a sub-pixel shift, and it is the column to read when the
question is whether a change moved a document towards the reference.

    score.py ours.pdf ref.pdf              # both metrics for one rendering
    score.py before.pdf after.pdf ref.pdf  # both, for a change
"""
import os
import re
import subprocess
import sys
import tempfile

WORD = re.compile(
    r'<word xMin="([\d.eE+-]+)" yMin="([\d.eE+-]+)" '
    r'xMax="([\d.eE+-]+)" yMax="([\d.eE+-]+)">(.*?)</word>', re.S)


def _words(pdf):
    """(y, text) for every word on page one, from poppler's own text layer."""
    out = subprocess.run(['pdftotext', '-bbox', '-f', '1', '-l', '1', pdf, '-'],
                         capture_output=True, text=True).stdout
    return [(float(m.group(2)), m.group(5)) for m in WORD.finditer(out)]


def _mask(pdf, dpi=110):
    """Page one as a 1-bit ink mask. Rec.601 luma, threshold 200 -- averaging the
    channels instead would count saturated yellow as paper."""
    from PIL import Image
    directory = tempfile.mkdtemp()
    base = os.path.join(directory, 'p')
    subprocess.run(['pdftoppm', '-r', str(dpi), '-f', '1', '-l', '1', '-png',
                    '-singlefile', pdf, base], capture_output=True)
    if not os.path.exists(base + '.png'):
        return None
    return Image.open(base + '.png').convert('L').point(
        lambda v: 255 if v < 200 else 0, mode='1')


def ink(ours, ref):
    """Share of page one where exactly one side has ink, as a percentage."""
    from PIL import ImageChops
    a, b = _mask(ours), _mask(ref)
    if a is None or b is None or a.size != b.size:
        return None
    diff = ImageChops.logical_xor(a, b)
    return 100.0 * diff.convert('L').histogram()[255] / (diff.size[0] * diff.size[1])


def dev(ours, ref):
    """Mean |dy| in points over page-one words that pair up by text and order."""
    a, b = _words(ours), _words(ref)
    pairs = [(a[i][0], b[i][0]) for i in range(min(len(a), len(b))) if a[i][1] == b[i][1]]
    if len(pairs) < 5:
        return None
    return sum(abs(p - q) for p, q in pairs) / len(pairs)


def _fmt(value):
    return 'n/a' if value is None else f'{value:.3f}'


if __name__ == '__main__':
    if len(sys.argv) == 3:
        ours, ref = sys.argv[1], sys.argv[2]
        print(f'ink {_fmt(ink(ours, ref))}  dev {_fmt(dev(ours, ref))}')
    elif len(sys.argv) == 4:
        before, after, ref = sys.argv[1], sys.argv[2], sys.argv[3]
        for name, fn in (('ink', ink), ('dev', dev)):
            b, a = fn(before, ref), fn(after, ref)
            delta = 'n/a' if b is None or a is None else f'{a - b:+.3f}'
            print(f'{name}  before {_fmt(b)}  after {_fmt(a)}  delta {delta}')
    else:
        sys.exit(__doc__)
