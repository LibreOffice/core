#!/usr/bin/env python3
"""Row-by-row disagreement between two renderings inside an x band, for one page.

Answers "is our bullet in the same place as the reference's, and is it the same size"
without trusting pdftotext's font-derived word boxes, which are grouped per line and so
report the line's height rather than the glyph's.

    sl14-band.py <page> <x0pt> <x1pt> <y0pt> <y1pt> <ref.pdf> <label:pdf> [label:pdf ...]
"""
import subprocess, sys, tempfile, os

DPI = 150
K = DPI / 72.0

def pgm(pdf, page):
    with tempfile.TemporaryDirectory() as d:
        subprocess.run(["pdftoppm", "-gray", "-r", str(DPI), "-f", str(page), "-l", str(page),
                        pdf, os.path.join(d, "p")], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        names = sorted(os.listdir(d))
        data = open(os.path.join(d, names[0]), "rb").read()
    parts, i = [], 0
    while len(parts) < 4:
        while data[i:i+1].isspace(): i += 1
        if data[i:i+1] == b"#":
            while data[i:i+1] != b"\n": i += 1
            continue
        j = i
        while not data[j:j+1].isspace(): j += 1
        parts.append(data[i:j]); i = j
    i += 1
    w, h = int(parts[1]), int(parts[2])
    return w, h, data[i:i + w*h]

page = int(sys.argv[1])
x0, x1, y0, y1 = (float(v) for v in sys.argv[2:6])
ref = sys.argv[6]
others = sys.argv[7:]

rw, rh, rp = pgm(ref, page)
cx0, cx1 = max(0, int(x0*K)), min(rw, int(x1*K))
cy0, cy1 = max(0, int(y0*K)), min(rh, int(y1*K))

print("page %d, x %.0f-%.0f pt, y %.0f-%.0f pt, %d dpi" % (page, x0, x1, y0, y1, DPI))
print()

profiles = {}
for spec in others:
    label, pdf = spec.split(":", 1)
    w, h, p = pgm(pdf, page)
    if (w, h) != (rw, rh):
        print("%s: page size differs (%dx%d vs %dx%d)" % (label, w, h, rw, rh))
        continue
    rows = []
    total = 0
    for y in range(cy0, cy1):
        n = 0
        base = y*rw
        for x in range(cx0, cx1):
            if abs(p[base+x] - rp[base+x]) > 40:
                n += 1
        rows.append(n)
        total += n
    profiles[label] = rows
    print("%-8s differing pixels in band: %d" % (label, total))

print()
print("row profile (y in pt, differing pixel count per label)")
labels = list(profiles)
print("  %-8s %s" % ("y", "  ".join("%-8s" % l for l in labels)))
for idx in range(cy1 - cy0):
    vals = [profiles[l][idx] for l in labels]
    if max(vals) == 0:
        continue
    print("  %-8.1f %s" % ((cy0+idx)/K, "  ".join("%-8d" % v for v in vals)))
