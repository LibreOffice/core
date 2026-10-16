#!/usr/bin/env python3
"""Every filled path on a PDF page with its colour, its device-space bounding box and its area.

Written to check a page reading with an instrument: "the reference draws a black chart
background and a grey plot wall and we draw neither" is a claim about fills, and a fill census
either finds them or does not.  Colours are resolved for the three device spaces the two stacks
emit (g / rg / k) and for `sh` shadings only far enough to name them.
"""
import re, sys, collections
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
from pg import page_stream

NUM = rb'-?\d*\.?\d+'
TOK = re.compile(rb'(/[^\s/\[\]<>()]+|' + NUM + rb'|[A-Za-z*\'"]+)')


def mul(a, b):
    return [a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
            a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3],
            a[4]*b[0]+a[5]*b[2]+b[4], a[4]*b[1]+a[5]*b[3]+b[5]]


def hexof(space, v):
    if space == 'g' and len(v) >= 1:
        c = int(round(v[-1]*255)); return '#%02X%02X%02X' % (c, c, c)
    if space == 'rg' and len(v) >= 3:
        return '#%02X%02X%02X' % tuple(int(round(x*255)) for x in v[-3:])
    if space == 'k' and len(v) >= 4:
        c, m, y, k = v[-4:]
        return '#%02X%02X%02X' % tuple(int(round(255*(1-min(1, x+k)))) for x in (c, m, y))
    return '?'


def fills(stream):
    stream = re.sub(rb'BT.*?ET', b'', stream, flags=re.S)
    ctm = [1.0, 0, 0, 1.0, 0, 0]
    stack = []
    col = '#000000'
    cur = []
    out = []
    args = []

    def app(m, x, y):
        return (m[0]*x + m[2]*y + m[4], m[1]*x + m[3]*y + m[5])

    for m in TOK.finditer(stream):
        t = m.group(1)
        if re.fullmatch(rb'[A-Za-z*\'"]+', t):
            op = t.decode('latin1')
            nums = []
            for a in args:
                try: nums.append(float(a))
                except ValueError: pass
            if op == 'q': stack.append((list(ctm), col))
            elif op == 'Q':
                if stack: ctm, col = stack.pop(); ctm = list(ctm)
            elif op == 'cm' and len(nums) >= 6: ctm = mul(nums[-6:], ctm)
            elif op in ('g', 'rg', 'k'): col = hexof(op, nums)
            elif op == 'm' and len(nums) >= 2: cur.append(app(ctm, *nums[-2:]))
            elif op in ('l',) and len(nums) >= 2: cur.append(app(ctm, *nums[-2:]))
            elif op == 'c' and len(nums) >= 6:
                cur += [app(ctm, nums[0], nums[1]), app(ctm, nums[2], nums[3]),
                        app(ctm, nums[4], nums[5])]
            elif op == 're' and len(nums) >= 4:
                x, y, w, h = nums[-4:]
                cur += [app(ctm, x, y), app(ctm, x+w, y), app(ctm, x+w, y+h), app(ctm, x, y+h)]
            elif op in ('f', 'F', 'f*', 'b', 'b*', 'B', 'B*'):
                if cur:
                    xs = [p[0] for p in cur]; ys = [p[1] for p in cur]
                    out.append((col, min(xs), min(ys), max(xs), max(ys), len(cur)))
                cur = []
            elif op in ('S', 's', 'n', 'W', 'W*'):
                if op in ('S', 's', 'n'): cur = []
            elif op == 'sh':
                out.append(('SHADING', 0, 0, 0, 0, 0))
            args = []
        else:
            args.append(t)
    return out


if __name__ == '__main__':
    path, page = sys.argv[1], int(sys.argv[2]) - 1
    rows = fills(page_stream(path, page))
    rows.sort(key=lambda r: -((r[3]-r[1]) * (r[4]-r[2])))
    print(f"{len(rows)} fills")
    for c, x0, y0, x1, y1, n in rows[:int(sys.argv[3]) if len(sys.argv) > 3 else 25]:
        print(f"  {c:9s} {x0:8.2f} {y0:8.2f} {x1:8.2f} {y1:8.2f}  "
              f"{(x1-x0):7.2f}x{(y1-y0):7.2f}  pts{n}")
    tal = collections.Counter(r[0] for r in rows)
    print("  colours:", dict(tal.most_common(12)))
