#!/usr/bin/env python3
"""Measure, from a LibreOffice PDF, the ratio between the advances it actually laid
text out with and the advances the embedded font nominally has.

Each show-text array is  [<glyphs> adj <glyphs> adj ...] TJ.  The nominal advance of a
glyph is the /Widths entry for its code; the adjustment shifts the pen by -adj/1000 em.
So  actual = sum(W) - sum(adj), and the ratio actual/sum(W) is what the layout device did
to the font.
"""
import re, sys, zlib, collections

data = open(sys.argv[1], 'rb').read()


def objects(buf):
    out = {}
    for m in re.finditer(rb'(\d+)\s+(\d+)\s+obj\b', buf):
        num = int(m.group(1))
        end = buf.find(b'endobj', m.end())
        out[num] = buf[m.end():end]
    return out


objs = objects(data)


def widths_for(fontobj):
    """Return {code: width} for a simple font object body."""
    fc = re.search(rb'/FirstChar\s+(\d+)', fontobj)
    w = re.search(rb'/Widths\s*\[([^\]]*)\]', fontobj)
    if fc and w:
        first = int(fc.group(1))
        vals = [int(float(x)) for x in w.group(1).split()]
        return {first + i: v for i, v in enumerate(vals)}
    # indirect Widths
    w = re.search(rb'/Widths\s+(\d+)\s+\d+\s+R', fontobj)
    if fc and w:
        first = int(fc.group(1))
        body = objs.get(int(w.group(1)), b'')
        arr = re.search(rb'\[([^\]]*)\]', body)
        vals = [int(float(x)) for x in arr.group(1).split()]
        return {first + i: v for i, v in enumerate(vals)}
    return {}


# map resource name -> widths, by scanning every font object and every /Font dict
fonts = {}
for num, body in objs.items():
    if b'/Type' in body and b'/Font' in body and b'/Widths' in body:
        fonts[num] = widths_for(body)

# resource dictionaries: /F1 12 0 R
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


TF = re.compile(rb'/(F\d+)\s+([\d.]+)\s+Tf')
TJ = re.compile(rb'\[(.*?)\]\s*TJ', re.S)

per_size = collections.defaultdict(lambda: [0.0, 0.0, 0])
for d in content_streams():
    cur = None
    size = None
    pos = 0
    for m in re.finditer(rb'/(F\d+)\s+([\d.]+)\s+Tf|\[(.*?)\]\s*TJ', d, re.S):
        if m.group(1):
            cur = m.group(1).decode()
            size = float(m.group(2))
            continue
        if cur is None:
            continue
        widths = fonts.get(name_to_obj.get(cur, -1), {})
        if not widths:
            continue
        arr = m.group(3)
        nominal = 0.0
        adj = 0.0
        n = 0
        for tok in re.finditer(rb'<([0-9A-Fa-f]+)>|(-?[\d.]+)', arr):
            if tok.group(1):
                h = tok.group(1)
                for i in range(0, len(h), 2):
                    code = int(h[i:i + 2], 16)
                    nominal += widths.get(code, 0)
                    n += 1
            else:
                adj += float(tok.group(2))
        key = (cur, size)
        per_size[key][0] += nominal
        per_size[key][1] += adj
        per_size[key][2] += n

print(f'{"font":8} {"size":>6} {"glyphs":>8} {"nominal":>12} {"adj":>10} {"actual/nominal":>16}')
for (f, s), (nom, adj, n) in sorted(per_size.items(), key=lambda kv: -kv[1][2]):
    if n < 50:
        continue
    print(f'{f:8} {s:6.1f} {n:8d} {nom:12.0f} {adj:10.0f} {(nom - adj) / nom:16.6f}')
