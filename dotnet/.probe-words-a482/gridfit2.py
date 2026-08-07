#!/usr/bin/env python3
"""Fit a device-pixel quantisation model to the advances a PDF laid text out with.

Resolves /Font resources per page, which the naive version does not: LibreOffice reuses
the names /F1../Fn with different font objects on different pages, so a global name map
silently attributes one face's widths to another's glyph codes.
"""
import re, sys, zlib, collections

data = open(sys.argv[1], 'rb').read()
DPI = int(sys.argv[2]) if len(sys.argv) > 2 else 300

objs = {}
for m in re.finditer(rb'(\d+)\s+(\d+)\s+obj\b', data):
    end = data.find(b'endobj', m.end())
    objs[int(m.group(1))] = data[m.end():end]


def stream_of(num):
    body = objs.get(num, b'')
    m = re.search(rb'stream\r?\n', body)
    if not m:
        return b''
    e = body.find(b'endstream', m.end())
    raw = body[m.end():e]
    try:
        return zlib.decompress(raw)
    except Exception:
        return raw


def widths_of(num):
    body = objs.get(num, b'')
    if b'/Widths' not in body:
        return None
    fc = re.search(rb'/FirstChar\s+(\d+)', body)
    w = re.search(rb'/Widths\s*\[([^\]]*)\]', body)
    if not w:
        wr = re.search(rb'/Widths\s+(\d+)\s+\d+\s+R', body)
        if wr:
            w = re.search(rb'\[([^\]]*)\]', objs.get(int(wr.group(1)), b''))
    if not (fc and w):
        return None
    vals = [int(float(x)) for x in w.group(1).split()]
    base = re.search(rb'/BaseFont\s*/([^\s/\]>]+)', body)
    return (base.group(1).decode() if base else '?',
            {int(fc.group(1)) + i: v for i, v in enumerate(vals)})


pages = []
for num, body in objs.items():
    if not re.search(rb'/Type\s*/Page\b', body):
        continue
    fontmap = {}
    res = re.search(rb'/Resources\s+(\d+)\s+\d+\s+R', body)
    resbody = objs.get(int(res.group(1)), b'') if res else body
    fdict = re.search(rb'/Font\s*<<(.*?)>>', resbody, re.S)
    if not fdict:
        fr = re.search(rb'/Font\s+(\d+)\s+\d+\s+R', resbody)
        fdict = re.search(rb'<<(.*?)>>', objs.get(int(fr.group(1)), b''), re.S) if fr else None
    if fdict:
        for m in re.finditer(rb'/(\w+)\s+(\d+)\s+\d+\s+R', fdict.group(1)):
            w = widths_of(int(m.group(2)))
            if w:
                fontmap[m.group(1).decode()] = w
    contents = [int(x) for x in re.findall(rb'/Contents\s+(\d+)\s+\d+\s+R', body)]
    if not contents:
        arr = re.search(rb'/Contents\s*\[(.*?)\]', body, re.S)
        if arr:
            contents = [int(x) for x in re.findall(rb'(\d+)\s+\d+\s+R', arr.group(1))]
    pages.append((fontmap, contents))

runs = []
for fontmap, contents in pages:
    for c in contents:
        d = stream_of(c)
        cur = size = None
        for m in re.finditer(rb'/(\w+)\s+([\d.]+)\s+Tf|\[(.*?)\]\s*TJ|\((.*?)\)\s*Tj', d, re.S):
            if m.group(1):
                cur, size = m.group(1).decode(), float(m.group(2))
                continue
            if m.group(3) is None or cur not in fontmap:
                continue
            base, widths = fontmap[cur]
            seq, adj = [], 0.0
            for tok in re.finditer(rb'<([0-9A-Fa-f]+)>|(-?[\d.]+)', m.group(3)):
                if tok.group(1):
                    h = tok.group(1)
                    for i in range(0, len(h), 2):
                        seq.append(widths.get(int(h[i:i + 2], 16), 0))
                else:
                    adj += float(tok.group(2))
            if len(seq) >= 8:
                runs.append((base, size, seq, sum(seq) - adj))

by = collections.defaultdict(lambda: [0.0, 0.0, 0.0, 0.0, 0])
for base, size, seq, actual in runs:
    P = round(size * DPI / 72.0)
    unit = 1000.0 * (72.0 / DPI) / size
    pa = sum(round(w * P / 1000.0) for w in seq) * unit
    pb = round(sum(seq) * P / 1000.0) * unit
    k = (base, size)
    by[k][0] += sum(seq)
    by[k][1] += actual
    by[k][2] += pa
    by[k][3] += pb
    by[k][4] += len(seq)

print(f'dpi={DPI}   (P = round(size*dpi/72) pixels per em)')
print(f'{"font":34} {"size":>5} {"glyphs":>7} {"actual":>10} {"perGlyph":>10} {"perRun":>10} {"P/exact":>9}')
for (b, s), (nom, act, pa, pb, n) in sorted(by.items(), key=lambda kv: -kv[1][4]):
    if n < 150:
        continue
    P = round(s * DPI / 72.0)
    print(f'{b[:34]:34} {s:5.1f} {n:7d} {act/nom:10.6f} {pa/nom:10.6f} {pb/nom:10.6f} '
          f'{P/(s*DPI/72.0):9.6f}')
