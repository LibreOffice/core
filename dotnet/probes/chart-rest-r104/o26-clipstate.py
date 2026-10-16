#!/usr/bin/env python3
"""Every painting operator on a page with the bounding box of the clip in force.

`clipops.py` only saw clips built from `re`; a clip built from a general path
(`m l c ... W n`) — which is what `VclMetafileProcessor2D::processMaskPrimitive2D`
emits, since it hands VCL a poly-polygon — was invisible to it.  This walks the
whole stream, keeps the q/Q stack, accumulates a clip bounding box from *any* path
that a `W`/`W*` closes, and tags each painting operator with the clip in force.

    o26-clipstate.py FILE.pdf PAGE
"""
import re, sys
import pymupdf

TOKEN = re.compile(
    rb'(?s)(\[[^\]]*\]|\((?:\\.|[^\\)])*\)|<[^>]*>|/[^\s/\[\]<>()]+|[-+.\d]+|[A-Za-z\'"*]+)')
PAINT = {'S','s','f','F','f*','B','B*','b','b*','n','Do','sh','TJ','Tj',"'",'"'}

def mul(a, b):
    return (a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
            a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3],
            a[4]*b[0]+a[5]*b[2]+b[4], a[4]*b[1]+a[5]*b[3]+b[5])

def apply(ctm, x, y):
    return (x*ctm[0]+y*ctm[2]+ctm[4], x*ctm[1]+y*ctm[3]+ctm[5])

def walk(stream):
    ctm = (1,0,0,1,0,0)
    clip = None                      # bbox of clip in force, None = page
    stack = []
    pend = []                        # device-space points of the path under construction
    cur = (0.0, 0.0)
    wpend = False
    ops = []
    out = []
    for m in TOKEN.finditer(stream):
        t = m.group(1).decode('latin-1')
        if re.fullmatch(r'[-+.\d]+', t):
            ops.append(float(t)); continue
        if t == 'q':
            stack.append((ctm, clip))
        elif t == 'Q':
            if stack: ctm, clip = stack.pop()
        elif t == 'cm' and len(ops) >= 6:
            ctm = mul(tuple(ops[-6:]), ctm)
        elif t in ('m','l') and len(ops) >= 2:
            cur = (ops[-2], ops[-1]); pend.append(apply(ctm, *cur))
        elif t == 'c' and len(ops) >= 6:
            for i in (0,2,4): pend.append(apply(ctm, ops[-6+i], ops[-5+i]))
            cur = (ops[-2], ops[-1])
        elif t in ('v','y') and len(ops) >= 4:
            for i in (0,2): pend.append(apply(ctm, ops[-4+i], ops[-3+i]))
            cur = (ops[-2], ops[-1])
        elif t == 're' and len(ops) >= 4:
            x,y,w,h = ops[-4:]
            for px,py in ((x,y),(x+w,y),(x,y+h),(x+w,y+h)): pend.append(apply(ctm, px, py))
        elif t in ('W','W*'):
            wpend = True
        elif t in PAINT:
            if wpend and pend:
                xs=[p[0] for p in pend]; ys=[p[1] for p in pend]
                nb=(min(xs),min(ys),max(xs),max(ys))
                clip = nb if clip is None else (max(clip[0],nb[0]),max(clip[1],nb[1]),
                                                min(clip[2],nb[2]),min(clip[3],nb[3]))
            if t not in ('n',) or not wpend:
                if pend or t in ('Do','sh','TJ','Tj',"'",'"'):
                    xs=[p[0] for p in pend]; ys=[p[1] for p in pend]
                    bb=(min(xs),min(ys),max(xs),max(ys)) if pend else None
                    out.append((t, clip, bb))
            wpend = False; pend = []
        ops = []
    return out

def main():
    doc = pymupdf.open(sys.argv[1]); page = doc[int(sys.argv[2]) - 1]
    events = walk(page.read_contents())
    seen = {}
    for t, clip, bb in events:
        key = None if clip is None else tuple(round(v,3) for v in clip)
        seen.setdefault(key, []).append((t, bb))
    print(f"page {page.rect.width:.2f} x {page.rect.height:.2f}  {len(events)} painting ops")
    for key in sorted(seen, key=lambda k: (k is not None, k)):
        ev = seen[key]
        label = 'NO CLIP' if key is None else \
            f"({key[0]:8.3f},{key[1]:8.3f})-({key[2]:8.3f},{key[3]:8.3f})"
        kinds = {}
        for t, _ in ev: kinds[t] = kinds.get(t,0)+1
        bbs = [b for _, b in ev if b]
        ext = ''
        if bbs:
            ext = (f"  ink-bbox ({min(b[0] for b in bbs):8.3f},{min(b[1] for b in bbs):8.3f})"
                   f"-({max(b[2] for b in bbs):8.3f},{max(b[3] for b in bbs):8.3f})")
        print(f"  {label}  n={len(ev):4d}  {kinds}{ext}")

main()
