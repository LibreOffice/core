#!/usr/bin/env python3
"""Stroke colour of every long axis-parallel segment on a page, by length family."""
import sys, re, collections
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
from pg import page_stream

NUM = rb'-?\d*\.?\d+'
TOK = re.compile(rb'(' + NUM + rb'(?:\s+' + NUM + rb')*\s+[a-zA-Z*\'"]+|[a-zA-Z*\'"]+)')


def run(path, page):
    s = re.sub(rb'BT.*?ET', b'', page_stream(path, page), flags=re.S)
    ctm = [1.0, 0, 0, 1.0, 0, 0]
    col = (0.0, 0.0, 0.0)
    lw = 1.0
    dash = b''
    stack = []
    cur = []
    out = []

    def app(m, x, y):
        return (m[0]*x + m[2]*y + m[4], m[1]*x + m[3]*y + m[5])

    def mul(a, b):
        return [a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
                a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3],
                a[4]*b[0]+a[5]*b[2]+b[4], a[4]*b[1]+a[5]*b[3]+b[5]]

    pos = 0
    for t in TOK.finditer(s):
        parts = t.group(1).split()
        op = parts[-1].decode('latin1')
        try: nums = [float(x) for x in parts[:-1]]
        except ValueError: nums = []
        if op == 'q': stack.append((list(ctm), col, lw, dash))
        elif op == 'Q':
            if stack: ctm, col, lw, dash = stack.pop(); ctm = list(ctm)
        elif op == 'cm' and len(nums) >= 6: ctm = mul(nums[-6:], ctm)
        elif op == 'RG' and len(nums) >= 3: col = tuple(nums[-3:])
        elif op == 'G' and len(nums) >= 1: col = (nums[-1],)*3
        elif op == 'K' and len(nums) >= 4:
            c, m_, y_, k = nums[-4:]
            col = (1-min(1, c+k), 1-min(1, m_+k), 1-min(1, y_+k))
        elif op == 'w' and nums: lw = nums[-1]
        elif op == 'd':
            seg = s[max(0, t.start()-60):t.end()]
            m2 = re.search(rb'\[([^\]]*)\]\s*(' + NUM + rb')\s*d\s*$', seg)
            dash = m2.group(1).strip() if m2 else b''
        elif op == 'm' and len(nums) >= 2: cur = [app(ctm, nums[-2], nums[-1])]
        elif op == 'l' and len(nums) >= 2 and cur: cur.append(app(ctm, nums[-2], nums[-1]))
        elif op in ('S', 's'):
            sc = abs(ctm[0]) or 1.0
            for a, b in zip(cur, cur[1:]):
                out.append((a, b, col, lw*sc, dash))
            cur = []
        elif op in ('f', 'F', 'f*', 'n', 'B', 'b', 'B*', 'b*'): cur = []
    return out


def hexc(c):
    return '#%02X%02X%02X' % tuple(max(0, min(255, round(v*255))) for v in c)


if __name__ == '__main__':
    for lbl, p in (('ours', sys.argv[1]), ('ref', sys.argv[2])):
        segs = run(p, int(sys.argv[3]) - 1)
        agg = collections.Counter()
        for a, b, c, w, d in segs:
            L = max(abs(a[0]-b[0]), abs(a[1]-b[1]))
            if L < 20: continue
            ax = 'H' if abs(a[1]-b[1]) < 0.05 else ('V' if abs(a[0]-b[0]) < 0.05 else '.')
            if ax == '.': continue
            agg[(ax, round(L, 1), hexc(c), round(w, 2), d.decode('latin1'))] += 1
        print(f"--- {lbl}")
        for k, n in sorted(agg.items(), key=lambda kv: -kv[1]):
            print(f"   x{n:4d}  {k[0]} len {k[1]:8.1f}  {k[2]}  w {k[3]}  dash [{k[4]}]")
