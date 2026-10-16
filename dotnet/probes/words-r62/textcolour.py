#!/usr/bin/env python3
"""Every text-showing operator on a PDF page, with the fill colour in force when it ran.

`pdf-ops.py` reports position, size, face and glyph count and deliberately not colour. This
answers one question it cannot: *what colour is this run drawn in*. It is the instrument for the
`COL_AUTO` question — a run that states no colour is resolved by the renderer, and the two
renderers can disagree without a single glyph or a single coordinate moving, which is exactly the
shape of defect no gate column and no ink metric will show.

    textcolour.py <pdf> [page] [--near-top PT]

Colour is tracked through `g`, `rg`, `k` and `sc`/`scn` in a DeviceGray/RGB/CMYK space; a pattern
or an ICC space reports as `?`. `q`/`Q` save and restore it, as they must — a run drawn inside a
form's own graphics state is otherwise attributed the colour of whatever ran before the `q`.
"""
import re
import sys
import zlib


def page_streams(data, page_index):
    """Content bytes of the page at `page_index` (1-based), in document order."""
    # Object table by naive scan; enough for the two renderers this project compares, both of
    # which write plain (non-object-stream) page trees for the documents in the corpus.
    objs = {}
    for m in re.finditer(rb'(\d+)\s+(\d+)\s+obj(.*?)endobj', data, re.S):
        objs[int(m.group(1))] = m.group(3)

    pages = []
    for num, body in objs.items():
        if re.search(rb'/Type\s*/Page\b(?!s)', body):
            pages.append((num, body))
    if not pages:
        raise SystemExit('no /Page objects found')

    # Document order: the /Kids arrays of the /Pages nodes.
    order = []
    for num, body in objs.items():
        if re.search(rb'/Type\s*/Pages\b', body):
            kids = re.search(rb'/Kids\s*\[(.*?)\]', body, re.S)
            if kids:
                order += [int(x) for x in re.findall(rb'(\d+)\s+\d+\s+R', kids.group(1))]
    order = [n for n in order if n in dict(pages)] or [n for n, _ in pages]

    num = order[page_index - 1]
    body = dict(pages)[num]
    refs = re.search(rb'/Contents\s*(\[(.*?)\]|(\d+)\s+\d+\s+R)', body, re.S)
    if not refs:
        return b''
    ids = [int(x) for x in re.findall(rb'(\d+)\s+\d+\s+R', refs.group(0))]

    out = b''
    for i in ids:
        raw = objs.get(i, b'')
        s = re.search(rb'stream\r?\n(.*?)\s*endstream', raw, re.S)
        if not s:
            continue
        chunk = s.group(1)
        if b'/FlateDecode' in raw:
            try:
                chunk = zlib.decompress(chunk)
            except zlib.error:
                continue
        out += chunk + b'\n'
    return out


TOK = re.compile(rb'''
      \((?:\\.|[^()\\])*\)          # literal string
    | <[0-9A-Fa-f\s]*>              # hex string
    | \[|\]                          # array
    | /[^\s/\[\]()<>]+               # name
    | [-+0-9.]+                      # number
    | [A-Za-z'"*]+                   # operator
''', re.X)


def shows(content):
    """(y, colour, glyphs) for every Tj/TJ/'/" on the page."""
    stack = []
    fill = None
    identity = [1.0, 0.0, 0.0, 1.0, 0.0, 0.0]
    tm = list(identity)
    tlm = list(identity)
    ctm = list(identity)
    out = []
    args = []

    def mul(a, b):
        """a x b, both 2x3 affine matrices in PDF order."""
        return [
            a[0] * b[0] + a[1] * b[2],
            a[0] * b[1] + a[1] * b[3],
            a[2] * b[0] + a[3] * b[2],
            a[2] * b[1] + a[3] * b[3],
            a[4] * b[0] + a[5] * b[2] + b[4],
            a[4] * b[1] + a[5] * b[3] + b[5],
        ]

    def hexcol(vals, kind):
        if kind == 'g':
            v = int(round(vals[0] * 255))
            return '#%02X%02X%02X' % (v, v, v)
        if kind == 'rg':
            return '#%02X%02X%02X' % tuple(int(round(v * 255)) for v in vals[:3])
        if kind == 'k':
            c, m, y, k = vals[:4]
            return '#%02X%02X%02X' % tuple(
                int(round(255 * (1 - min(1.0, x + k)))) for x in (c, m, y))
        return '?'

    for m in TOK.finditer(content):
        t = m.group(0)
        if t[:1] in b'-+0123456789.':
            try:
                args.append(float(t))
            except ValueError:
                args.append(0.0)
            continue
        if t[:1] in b'(<[/':
            args.append(t)
            continue
        op = t.decode('latin-1')
        nums = [a for a in args if isinstance(a, float)]
        if op == 'q':
            stack.append((fill, list(ctm)))
        elif op == 'Q':
            if stack:
                fill, ctm = stack.pop()
        elif op == 'g' and len(nums) >= 1:
            fill = hexcol(nums[-1:], 'g')
        elif op == 'rg' and len(nums) >= 3:
            fill = hexcol(nums[-3:], 'rg')
        elif op == 'k' and len(nums) >= 4:
            fill = hexcol(nums[-4:], 'k')
        elif op in ('sc', 'scn') and nums:
            fill = hexcol(nums, {1: 'g', 3: 'rg', 4: 'k'}.get(len(nums), '?'))
        elif op == 'cm' and len(nums) >= 6:
            ctm = mul(nums[-6:], ctm)
        elif op == 'BT':
            tm = list(identity)
            tlm = list(identity)
        elif op == 'Tm' and len(nums) >= 6:
            tm = list(nums[-6:])
            tlm = list(tm)
        elif op in ('Td', 'TD') and len(nums) >= 2:
            tlm = mul([1.0, 0.0, 0.0, 1.0, nums[-2], nums[-1]], tlm)
            tm = list(tlm)
        elif op == 'T*':
            tlm = mul([1.0, 0.0, 0.0, 1.0, 0.0, 0.0], tlm)
            tm = list(tlm)
        elif op in ('Tj', 'TJ', "'", '"'):
            glyphs = 0
            for a in args:
                if isinstance(a, bytes) and a[:1] in b'(<':
                    glyphs += len(a) - 2 if a[:1] == b'(' else max(0, (len(a) - 2) // 2)
            device = mul(tm, ctm)
            out.append((round(device[5], 2), fill or '#000000', glyphs))
        if op not in ('q', 'Q'):
            args = []
        else:
            args = []
    return out


def main():
    pdf = sys.argv[1]
    page = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    data = open(pdf, 'rb').read()
    rows = shows(page_streams(data, page))
    counts = {}
    for _y, col, _g in rows:
        counts[col] = counts.get(col, 0) + 1
    print('%s page %d — %d shows' % (pdf, page, len(rows)))
    for col, n in sorted(counts.items(), key=lambda kv: -kv[1]):
        print('   %-9s %4d' % (col, n))
    print('   by y (descending, top of the page first):')
    for y, col, g in sorted(rows, key=lambda r: -r[0])[:24]:
        print('      y %8.2f  %-9s %3d glyphs' % (y, col, g))


if __name__ == '__main__':
    main()
