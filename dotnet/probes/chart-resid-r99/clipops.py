#!/usr/bin/env python3
"""Every clip operator on a page, in stream order, with its rectangle in page coordinates,
and whether it was reached with an intervening `Q` (a replace) or nested (an intersect).

Usage: clipops.py FILE.pdf PAGE
"""
import re, sys
import pymupdf

TOKEN = re.compile(
    rb'(?s)(\[[^\]]*\]|\((?:\\.|[^\\)])*\)|<[^>]*>|/[^\s/\[\]<>()]+|[-+.\d]+|[A-Za-z\'"*]+)')

def mul(a, b):
    return (a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
            a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3],
            a[4]*b[0]+a[5]*b[2]+b[4], a[4]*b[1]+a[5]*b[3]+b[5])

def main():
    doc = pymupdf.open(sys.argv[1]); page = doc[int(sys.argv[2]) - 1]
    stream = page.read_contents()
    print(f"page {page.rect.width:.2f} x {page.rect.height:.2f}")
    ctm, saved, ops, last, depth = (1,0,0,1,0,0), [], [], None, 0
    for m in TOKEN.finditer(stream):
        t = m.group(1).decode('latin-1')
        if re.fullmatch(r'[-+.\d]+', t):
            ops.append(float(t)); continue
        if t == 'q':
            saved.append(ctm); depth += 1
        elif t == 'Q':
            ctm = saved.pop() if saved else ctm; depth -= 1
        elif t == 'cm' and len(ops) >= 6:
            ctm = mul(tuple(ops[-6:]), ctm)
        elif t == 're' and len(ops) >= 4:
            last = tuple(ops[-4:])
        elif t in ('W', 'W*') and last:
            x, y, w, h = last
            pts = [(x,y),(x+w,y),(x,y+h),(x+w,y+h)]
            mv = [(px*ctm[0]+py*ctm[2]+ctm[4], px*ctm[1]+py*ctm[3]+ctm[5]) for px,py in pts]
            xs=[p[0] for p in mv]; ys=[p[1] for p in mv]
            print(f"  depth={depth} {t}  ({min(xs):8.3f},{min(ys):8.3f})-({max(xs):8.3f},{max(ys):8.3f})"
                  f"  w={max(xs)-min(xs):8.3f} h={max(ys)-min(ys):8.3f}")
        ops = []

main()
