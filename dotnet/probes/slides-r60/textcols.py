#!/usr/bin/env python3
"""The fill colour in force at every show-text operator on a page, with the text's pen.

A page reading that says "the reference's title is white and ours is black" is a claim about
the colour operator in force inside the BT/ET block, which neither an ink diff nor a fill
census reports -- text is not a filled path.
"""
import re, sys, collections
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
from pg import page_stream

NUM = rb'-?\d*\.?\d+'
TOK = re.compile(rb'(\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>|\[[^\]]*\]|' + NUM +
                 rb'|/[^\s/\[\]<>()]+|[A-Za-z\'"*]+)')


def hexof(op, v):
    if op == 'g' and v: c = int(round(v[-1]*255)); return '#%02X%02X%02X' % (c, c, c)
    if op == 'rg' and len(v) >= 3: return '#%02X%02X%02X' % tuple(int(round(x*255)) for x in v[-3:])
    if op == 'k' and len(v) >= 4:
        c, m, y, k = v[-4:]
        return '#%02X%02X%02X' % tuple(int(round(255*(1-min(1, x+k)))) for x in (c, m, y))
    return '?'


def runs(stream):
    ctm = [1.0, 0, 0, 1.0, 0, 0]; stack = []
    tm = tlm = [1.0, 0, 0, 1.0, 0, 0]
    col = '#000000'
    out = []; args = []

    def mul(a, b):
        return [a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
                a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3],
                a[4]*b[0]+a[5]*b[2]+b[4], a[4]*b[1]+a[5]*b[3]+b[5]]

    for m in TOK.finditer(stream):
        t = m.group(1)
        if re.fullmatch(rb'[A-Za-z\'"*]+', t):
            op = t.decode('latin1'); nums = []
            for a in args:
                try: nums.append(float(a))
                except ValueError: pass
            if op == 'q': stack.append((list(ctm), col))
            elif op == 'Q':
                if stack: c2, col = stack.pop(); ctm = list(c2)
            elif op == 'cm' and len(nums) >= 6: ctm = mul(nums[-6:], ctm)
            elif op in ('g', 'rg', 'k'): col = hexof(op, nums)
            elif op == 'BT': tm = tlm = [1.0, 0, 0, 1.0, 0, 0]
            elif op == 'Tm' and len(nums) >= 6: tm = tlm = nums[-6:]
            elif op in ('Td', 'TD') and len(nums) >= 2:
                tlm = mul([1, 0, 0, 1, nums[-2], nums[-1]], tlm); tm = list(tlm)
            elif op in ('Tj', 'TJ', "'", '"'):
                d = mul(tm, ctm)
                out.append((col, d[4], d[5], round(abs(tm[0]), 1)))
            args = []
        else:
            args.append(t)
    return out


if __name__ == '__main__':
    for path in sys.argv[1:-1]:
        rs = runs(page_stream(path, int(sys.argv[-1]) - 1))
        print(f"--- {path.split('/')[-1]}  {len(rs)} runs")
        print("   ", dict(collections.Counter(r[0] for r in rs).most_common(8)))
        for c, x, y, sc in sorted(rs, key=lambda r: -r[3])[:8]:
            print(f"    {c}  size{sc:6.1f}  at {x:7.2f},{y:7.2f}")
