#!/usr/bin/env python3
"""Count the *painted* marks on a PDF page: filled paths and stroked paths.

    marks.py <ours.pdf> <ref.pdf>

The gate cannot see a fill — it adds no glyph and no page — so this is the instrument the
round is judged on. `page.get_drawings()` reports one entry per path with a `type` of
`f` (filled), `s` (stroked) or `fs` (both), which is exactly the distinction
`CustomShapeGeometry.FillOutline`/`StrokeOutline` make, so the two sides are counted the same
way and a fill painted as a rectangle still counts as a fill: the number says *whether* ink
was laid, and the pairing with `page-vision` says whether it was laid in the right shape.

Both files must exist; a missing one is an error rather than a zero, because a zero here
reads exactly like the defect under test.
"""
import sys
import pymupdf


def counts(path):
    doc = pymupdf.open(path)
    out = []
    for page in doc:
        f = s = 0
        for d in page.get_drawings():
            if d['type'] in ('f', 'fs'):
                f += 1
            if d['type'] in ('s', 'fs'):
                s += 1
        out.append((f, s))
    return out


def main(ours, ref):
    a, b = counts(ours), counts(ref)
    print(f'{"page":>5} {"ours f":>7} {"ref f":>7} {"ours s":>7} {"ref s":>7}')
    for i in range(max(len(a), len(b))):
        x = a[i] if i < len(a) else ('-', '-')
        y = b[i] if i < len(b) else ('-', '-')
        print(f'{i + 1:>5} {x[0]:>7} {y[0]:>7} {x[1]:>7} {y[1]:>7}')
    print(f'{"TOTAL":>5} {sum(p[0] for p in a):>7} {sum(p[0] for p in b):>7} '
          f'{sum(p[1] for p in a):>7} {sum(p[1] for p in b):>7}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
