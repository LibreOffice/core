#!/usr/bin/env python3
"""Every rotated text run on a page, with its device-space origin and its device rotation.

Round 60 abandoned a rotated-label census because "our rotated runs carry the rotation in the
CTM and the reference's in the text matrix", so a detector that reads either one alone sees
only one stack's labels.  The fix is to stop reading either: the matrix that maps text space to
device space is Tm x CTM, and the rotation of a run is atan2 of that product's first row
whichever factor it came out of.  The origin is the same product's translation, which is the
pen -- the point the run is anchored at -- in both conventions.
"""
import math, re, sys
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
sys.path.insert(0, '/c/sandbox/workdir/scratch-r62-slides')
from pg import page_stream
from tfpos import mul

NUM = rb'-?\d*\.?\d+'
TOK = re.compile(rb'(\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>|\[[^\]]*\]|' + NUM
                 + rb'|/[^\s/\[\]<>()]+|[A-Za-z\'"*]+)')


def runs(stream):
    ctm = [1.0, 0, 0, 1.0, 0, 0]
    stack = []
    tm = tlm = [1.0, 0, 0, 1.0, 0, 0]
    size = 0.0
    out = []
    args = []
    for m in TOK.finditer(stream):
        t = m.group(1)
        if re.fullmatch(rb'[A-Za-z\'"*]+', t):
            op = t.decode('latin1')
            if op == 'q': stack.append(list(ctm))
            elif op == 'Q':
                if stack: ctm = stack.pop()
            elif op == 'cm' and len(args) >= 6: ctm = mul([float(x) for x in args[-6:]], ctm)
            elif op == 'BT': tm = tlm = [1.0, 0, 0, 1.0, 0, 0]
            elif op == 'Tm' and len(args) >= 6: tm = tlm = [float(x) for x in args[-6:]]
            elif op in ('Td', 'TD') and len(args) >= 2:
                tlm = mul([1, 0, 0, 1, float(args[-2]), float(args[-1])], tlm); tm = list(tlm)
            elif op == 'Tf' and len(args) >= 2: size = float(args[-1])
            elif op in ('Tj', 'TJ', "'", '"'):
                d = mul(tm, ctm)
                ang = math.degrees(math.atan2(d[1], d[0]))
                em = size * math.hypot(d[0], d[1])
                out.append((d[4], d[5], ang, em, (args[-1] if args else b'').decode('latin1', 'replace')))
            args = []
        else:
            args.append(t)
    return out


if __name__ == '__main__':
    path, page = sys.argv[1], int(sys.argv[2]) - 1
    lo = float(sys.argv[3]) if len(sys.argv) > 3 else 5.0
    rs = [r for r in runs(page_stream(path, page)) if abs(r[2]) >= lo]
    print(f"{len(rs)} rotated runs (|angle| >= {lo} deg)")
    for x, y, a, em, s in sorted(rs, key=lambda r: r[0]):
        print(f"  x{x:8.2f} y{y:8.2f}  {a:+7.2f} deg  em{em:6.2f}  {s[:50]}")
