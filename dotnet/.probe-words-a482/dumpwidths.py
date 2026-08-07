#!/usr/bin/env python3
"""Print every simple font in a PDF with its BaseFont name and /Widths histogram."""
import re, sys, collections

data = open(sys.argv[1], 'rb').read()
objs = {}
for m in re.finditer(rb'(\d+)\s+(\d+)\s+obj\b', data):
    end = data.find(b'endobj', m.end())
    objs[int(m.group(1))] = data[m.end():end]

for num, body in sorted(objs.items()):
    if b'/Font' not in body or b'/Widths' not in body:
        continue
    base = re.search(rb'/BaseFont\s*/([^\s/\]>]+)', body)
    fc = re.search(rb'/FirstChar\s+(\d+)', body)
    w = re.search(rb'/Widths\s*\[([^\]]*)\]', body)
    if not w:
        wr = re.search(rb'/Widths\s+(\d+)\s+\d+\s+R', body)
        if wr:
            w = re.search(rb'\[([^\]]*)\]', objs.get(int(wr.group(1)), b''))
    if not (fc and w):
        continue
    vals = [int(float(x)) for x in w.group(1).split()]
    print(f'obj {num:4d} {base.group(1).decode() if base else "?":40s} first={fc.group(1).decode():4s} n={len(vals)}')
    print('   ', vals[:40])
