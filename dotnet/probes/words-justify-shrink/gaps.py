#!/usr/bin/env python3
"""Per-line word-gap widths from a PDF, via pdftotext -bbox.

Prints, for each text line: word count, the sum of the word boxes, the mean gap between
consecutive words, and the line's drawn extent. The last line of a justified paragraph is not
stretched, so its mean gap is the natural blank width the other lines are measured against.
"""
import re, subprocess, sys, collections

def words(pdf):
    xml = subprocess.run(["pdftotext", "-bbox", pdf, "-"], capture_output=True, text=True).stdout
    out = []
    for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', xml):
        x0, y0, x1, y1, t = float(m.group(1)), float(m.group(2)), float(m.group(3)), float(m.group(4)), m.group(5)
        out.append((round(y0, 1), x0, x1, t))
    return out

for pdf in sys.argv[1:]:
    ws = words(pdf)
    if not ws:
        raise SystemExit(f"{pdf}: pdftotext produced no words")
    lines = collections.OrderedDict()
    for y, x0, x1, t in ws:
        lines.setdefault(y, []).append((x0, x1, t))
    print(f"== {pdf}")
    for i, (y, ws2) in enumerate(lines.items(), 1):
        ws2.sort()
        ink = sum(x1 - x0 for x0, x1, _ in ws2)
        gaps = [ws2[j + 1][0] - ws2[j][1] for j in range(len(ws2) - 1)]
        mean = sum(gaps) / len(gaps) if gaps else 0
        print(f"  line {i}: {len(ws2):2d} words  ink {ink:8.3f}  span {ws2[0][0]:7.2f}..{ws2[-1][1]:7.2f}"
              f"  mean gap {mean:6.3f}  last word {ws2[-1][2]!r}")
