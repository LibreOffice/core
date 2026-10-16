#!/usr/bin/env python3
"""Read a PDF text-showing operator and report, per character, the advance the
producer actually intended: the drawn font's own declared width plus the TJ
adjustment.  This is exact -- it does not go through a text extractor's
quantised character boxes.

usage: implied-advances.py <pdf> <page> <substring-of-content-stream>
"""
import sys, re
import pymupdf

path, pno = sys.argv[1], int(sys.argv[2])
marker = sys.argv[3]

doc = pymupdf.open(path)
page = doc[pno]
raw = b"".join(doc.xref_stream(x) for x in page.get_contents()).decode("latin-1")

# resource name -> (basefont, xref)
res = {}
for f in page.get_fonts(full=True):
    xref, ext, ftype, basefont, name, enc = f[:6]
    res[name] = (basefont, xref)

def font_widths(xref):
    """/FirstChar,/Widths off the simple font dict."""
    d = doc.xref_object(xref, compressed=True)
    fc = int(re.search(r"/FirstChar\s+(\d+)", d).group(1))
    w = re.search(r"/Widths\s+(\d+)\s+0\s+R", d)
    if w:
        arr = doc.xref_object(int(w.group(1)), compressed=True)
    else:
        arr = re.search(r"/Widths\s*(\[[^\]]*\])", d).group(1)
    vals = [float(x) for x in re.findall(r"-?[\d.]+", arr.strip()[1:-1])]
    return fc, vals

def tounicode(xref):
    d = doc.xref_object(xref, compressed=True)
    m = re.search(r"/ToUnicode\s+(\d+)\s+0\s+R", d)
    if not m: return {}
    s = doc.xref_stream(int(m.group(1))).decode("latin-1")
    out = {}
    for blk in re.findall(r"beginbfchar(.*?)endbfchar", s, re.S):
        for a, b in re.findall(r"<([0-9A-Fa-f]+)>\s*<([0-9A-Fa-f]+)>", blk):
            out[int(a, 16)] = "".join(chr(int(b[i:i+4], 16)) for i in range(0, len(b), 4))
    for blk in re.findall(r"beginbfrange(.*?)endbfrange", s, re.S):
        for a, b, c in re.findall(r"<([0-9A-Fa-f]+)>\s*<([0-9A-Fa-f]+)>\s*<([0-9A-Fa-f]+)>", blk):
            lo, hi, dst = int(a, 16), int(b, 16), int(c, 16)
            for k in range(lo, hi + 1):
                out[k] = chr(dst + k - lo)
    return out

i = raw.find(marker)
if i < 0:
    sys.exit("marker not found")
# walk back to the enclosing BT
bt = raw.rfind("BT", 0, i)
et = raw.find("ET", i)
block = raw[bt:et]
print("== block ==")
print(block.strip()[:900])

tf = re.search(r"/(F\d+)\s+([\d.]+)\s+Tf", block)
fname, size = tf.group(1), float(tf.group(2))
basefont, xref = res[fname]
fc, widths = font_widths(xref)
tu = tounicode(xref)
print(f"\n== drawn with /{fname} = {basefont} at {size} pt (FirstChar {fc}, {len(widths)} widths)")

# parse the TJ array
tj = re.search(r"\[(.*?)\]\s*TJ", block, re.S).group(1)
toks = re.findall(r"<([0-9A-Fa-f]+)>|(-?[\d.]+)", tj)
seq = []          # (code, adjustment-after)
for hexs, num in toks:
    if hexs:
        for k in range(0, len(hexs), 2):
            seq.append([int(hexs[k:k+2], 16), 0.0])
    else:
        if seq: seq[-1][1] += float(num)

total = 0.0
print(f"\n{'ch':>3} {'code':>4} {'W':>7} {'adj':>7} {'drawn':>8} {'implied':>8}")
rows = []
for code, adj in seq:
    W = widths[code - fc]
    drawn = W * size / 1000.0
    implied = (W - adj) * size / 1000.0
    ch = tu.get(code, "?")
    total += implied
    rows.append((ch, code, W, adj, drawn, implied))
    print(f"{ch!r:>3} {code:>4} {W:>7.1f} {adj:>7.1f} {drawn:>8.3f} {implied:>8.3f}")
print(f"\nstring = {''.join(r[0] for r in rows)!r}")
print(f"implied total = {total:.3f} pt   drawn total = {sum(r[4] for r in rows):.3f} pt")

import json
json.dump([{"ch": r[0], "code": r[1], "W": r[2], "adj": r[3],
            "drawn": r[4], "implied": r[5]} for r in rows],
          open(sys.argv[4], "w") if len(sys.argv) > 4 else sys.stdout, indent=1)
