#!/usr/bin/env python3
"""Census the fill and stroke colours a page's content stream sets, ours against the reference.

`pdf-image-diff.py`'s *"the same marks in a different colour"* hint says a region's ink is
in the wrong colour but not which colour, and a themed text colour resolved wrongly looks
exactly like a wrong fill at 512 pixels. The colour operators settle it in one read.

    colour-census.py <ours.pdf> <ref.pdf> <page> [<page> ...]
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', 'slides-r15'))
from pdfops import content, objects, pages  # noqa: E402

OPS = {b'rg': 3, b'RG': 3, b'g': 1, b'G': 1, b'k': 4, b'K': 4}


def colours(path, page_index):
    data = open(path, 'rb').read()
    objs = objects(data)
    order = pages(data, objs)
    cs = content(data, objs, order[page_index - 1]).split()
    out = {}
    for i, tok in enumerate(cs):
        n = OPS.get(tok)
        if not n or i < n:
            continue
        try:
            vals = [float(x) for x in cs[i - n:i]]
        except ValueError:
            continue
        if n == 1:
            rgb = (vals[0],) * 3
        elif n == 3:
            rgb = tuple(vals)
        else:
            rgb = tuple((1.0 - min(1.0, c + vals[3])) for c in vals[:3])
        key = '#%02X%02X%02X' % tuple(int(round(c * 255)) for c in rgb)
        kind = 'stroke' if tok.isupper() else 'fill'
        out[(kind, key)] = out.get((kind, key), 0) + 1
    return out


def main():
    ours, ref = sys.argv[1], sys.argv[2]
    for page in (int(x) for x in sys.argv[3:]):
        a, b = colours(ours, page), colours(ref, page)
        print(f'\npage {page}')
        print(f'  {"kind":7s} {"colour":8s} {"ours":>6} {"ref":>6}')
        for key in sorted(set(a) | set(b), key=lambda k: -(a.get(k, 0) + b.get(k, 0))):
            mark = '   <-- only one side' if (key in a) != (key in b) else ''
            print(f'  {key[0]:7s} {key[1]:8s} {a.get(key, 0):6d} {b.get(key, 0):6d}{mark}')


if __name__ == '__main__':
    main()
