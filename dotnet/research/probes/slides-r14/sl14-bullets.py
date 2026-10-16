#!/usr/bin/env python3
"""Compare the bullet glyphs and their boxes on one page, ours against the reference."""
import re, subprocess, sys, collections

def boxes(pdf, page):
    xml = subprocess.run(["pdftotext", "-bbox", "-f", str(page), "-l", str(page), pdf, "-"],
                         capture_output=True, text=True).stdout
    out = []
    for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">(.*?)</word>',
                         xml, re.S):
        x0, y0, x1, y1, w = float(m.group(1)), float(m.group(2)), float(m.group(3)), float(m.group(4)), m.group(5)
        out.append((x0, y0, x1, y1, w))
    return out

ours, ref, page = sys.argv[1], sys.argv[2], int(sys.argv[3])

for label, pdf in (("OURS", ours), ("REF ", ref)):
    bs = boxes(pdf, page)
    syms = [b for b in bs if any(ord(c) > 0x2000 for c in b[4])]
    print("%s  %d words, %d with a non-ASCII glyph" % (label, len(bs), len(syms)))
    counts = collections.Counter()
    for b in syms:
        counts[tuple("U+%04X" % ord(c) for c in b[4])] += 1
    for k, v in counts.most_common(8):
        print("     %-28s x%d" % (" ".join(k), v))
    for b in syms[:4]:
        print("     box %.2f,%.2f - %.2f,%.2f  w=%.2f h=%.2f  %s"
              % (b[0], b[1], b[2], b[3], b[2]-b[0], b[3]-b[1],
                 " ".join("U+%04X" % ord(c) for c in b[4])))
    print()
