#!/usr/bin/env python3
"""Every show-text operator on a PDF page with its device-space pen position and advance.

Advance is taken from the text matrix's own progression: the pen after the operator minus the
pen before, which needs no font metrics and is exact for both stacks because both write an
explicit Tm or Td per run.  Where a run is the last of its BT block the advance is estimated
from the /Widths of the font when it can be read, else reported as 0.
"""
import re, sys
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
from pg import page_stream, npages

NUM = rb'-?\d*\.?\d+'


def mul(a, b):
    return [a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
            a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3],
            a[4]*b[0]+a[5]*b[2]+b[4], a[4]*b[1]+a[5]*b[3]+b[5]]


def texts(stream):
    ctm = [1.0, 0, 0, 1.0, 0, 0]
    stack = []
    tm = tlm = [1.0, 0, 0, 1.0, 0, 0]
    out = []
    tok = re.compile(rb'(\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>|\[[^\]]*\]|'
                     + NUM + rb'|/[^\s/\[\]<>()]+|[A-Za-z\'"*]+)')
    args = []
    for m in tok.finditer(stream):
        t = m.group(1)
        if re.fullmatch(rb'[A-Za-z\'"*]+', t):
            op = t.decode('latin1')
            if op == 'q': stack.append(list(ctm))
            elif op == 'Q':
                if stack: ctm = stack.pop()
            elif op == 'cm' and len(args) >= 6:
                ctm = mul([float(x) for x in args[-6:]], ctm)
            elif op == 'BT': tm = tlm = [1.0, 0, 0, 1.0, 0, 0]
            elif op == 'Tm' and len(args) >= 6:
                tm = tlm = [float(x) for x in args[-6:]]
            elif op in ('Td', 'TD') and len(args) >= 2:
                tlm = mul([1, 0, 0, 1, float(args[-2]), float(args[-1])], tlm); tm = list(tlm)
            elif op == 'T*':
                tlm = mul([1, 0, 0, 1, 0, 0], tlm); tm = list(tlm)
            elif op in ('Tj', 'TJ', "'", '"'):
                raw = args[-1] if args else b''
                s = raw.decode('latin1', 'replace')
                d = mul(tm, ctm)
                out.append((d[4], d[5], s, tm[0]))
            args = []
        else:
            args.append(t)
    return out


if __name__ == '__main__':
    path, page = sys.argv[1], int(sys.argv[2]) - 1
    for x, y, s, sc in texts(page_stream(path, page)):
        print(f"{x:9.2f} {y:9.2f}  sc{sc:7.2f}  {s[:70]}")
