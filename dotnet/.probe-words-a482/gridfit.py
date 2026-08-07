#!/usr/bin/env python3
"""Fit a device-pixel quantisation model to the advances a LibreOffice PDF laid text out with.

Model (a): every glyph advance is an integer number of device pixels at P pixels/em,
           P = round(size_pt * dpi / 72).
Model (b): the run's total is rounded once.

Compares each model's predicted run width against the width the PDF actually used
(nominal /Widths minus the TJ adjustments).
"""
import re, sys, zlib, collections

data = open(sys.argv[1], 'rb').read()
objs = {}
for m in re.finditer(rb'(\d+)\s+(\d+)\s+obj\b', data):
    end = data.find(b'endobj', m.end())
    objs[int(m.group(1))] = data[m.end():end]

fonts = {}
for num, body in objs.items():
    if b'/Font' not in body or b'/Widths' not in body:
        continue
    fc = re.search(rb'/FirstChar\s+(\d+)', body)
    w = re.search(rb'/Widths\s*\[([^\]]*)\]', body)
    if not (fc and w):
        continue
    vals = [int(float(x)) for x in w.group(1).split()]
    fonts[num] = {int(fc.group(1)) + i: v for i, v in enumerate(vals)}

name_to_obj = {}
for num, body in objs.items():
    for m in re.finditer(rb'/(F\d+)\s+(\d+)\s+0\s+R', body):
        name_to_obj[m.group(1).decode()] = int(m.group(2))


def content_streams():
    for m in re.finditer(rb'stream\r?\n', data):
        s = m.end()
        e = data.find(b'endstream', s)
        try:
            d = zlib.decompress(data[s:e])
        except Exception:
            continue
        if b'TJ' in d:
            yield d


runs = []   # (fontname, size, [widths...], actual_thousandths)
for d in content_streams():
    cur = size = None
    for m in re.finditer(rb'/(F\d+)\s+([\d.]+)\s+Tf|\[(.*?)\]\s*TJ', d, re.S):
        if m.group(1):
            cur, size = m.group(1).decode(), float(m.group(2))
            continue
        if cur is None:
            continue
        widths = fonts.get(name_to_obj.get(cur, -1), {})
        if not widths:
            continue
        seq, adj = [], 0.0
        for tok in re.finditer(rb'<([0-9A-Fa-f]+)>|(-?[\d.]+)', m.group(3)):
            if tok.group(1):
                h = tok.group(1)
                for i in range(0, len(h), 2):
                    seq.append(widths.get(int(h[i:i + 2], 16), 0))
            else:
                adj += float(tok.group(2))
        if len(seq) >= 5:
            runs.append((cur, size, seq, sum(seq) - adj))

DPI = int(sys.argv[2]) if len(sys.argv) > 2 else 300

by = collections.defaultdict(lambda: [0.0, 0.0, 0.0, 0.0, 0])
for cur, size, seq, actual in runs:
    P = round(size * DPI / 72.0)
    unit = 1000.0 * (72.0 / DPI) / size      # one device pixel, in 1/1000 em
    pred_a = sum(round(w * P / 1000.0) for w in seq) * unit
    tot = sum(seq)
    pred_b = round(tot * P / 1000.0) * unit
    k = (cur, size)
    by[k][0] += tot
    by[k][1] += actual
    by[k][2] += pred_a
    by[k][3] += pred_b
    by[k][4] += len(seq)

print(f'dpi={DPI}')
print(f'{"font":6} {"size":>5} {"glyphs":>8} {"actual/nom":>11} {"modelA/nom":>11} {"modelB/nom":>11}')
for (f, s), (nom, act, pa, pb, n) in sorted(by.items(), key=lambda kv: -kv[1][4]):
    if n < 200:
        continue
    print(f'{f:6} {s:5.1f} {n:8d} {act/nom:11.6f} {pa/nom:11.6f} {pb/nom:11.6f}')
