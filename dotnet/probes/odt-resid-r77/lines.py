#!/usr/bin/env python3
"""Group a PDF page's words into lines with their bounding boxes.

`pdftotext -bbox` emits words, not lines, so every comparison of "where did this
line start and end" has to reconstruct them.  Words are bucketed on `yMin` to a
tenth of a point, which is finer than any line pitch in the corpus and coarser
than the sub-point jitter a justified line carries.

usage: lines.py <pdf> [page]
"""
import re
import subprocess
import sys


def lines_of(pdf: str, page: int):
    out = subprocess.run(
        ['pdftotext', '-f', str(page), '-l', str(page), '-bbox', pdf, '-'],
        capture_output=True, text=True).stdout
    words = re.findall(
        r'<word xMin="([\d.-]+)" yMin="([\d.-]+)" xMax="([\d.-]+)" yMax="([\d.-]+)">(.*?)</word>',
        out)
    rows: dict[float, list] = {}
    for xmin, ymin, xmax, ymax, text in words:
        rows.setdefault(round(float(ymin), 1), []).append(
            (float(xmin), float(xmax), float(ymax), text))
    for y in sorted(rows):
        ws = sorted(rows[y])
        yield (y, min(w[0] for w in ws), max(w[1] for w in ws),
               ' '.join(w[3] for w in ws))


def pages(pdf: str) -> int:
    info = subprocess.run(['pdfinfo', pdf], capture_output=True, text=True).stdout
    return int(info.split('Pages:')[1].split()[0])


if __name__ == '__main__':
    pdf = sys.argv[1]
    pp = [int(sys.argv[2])] if len(sys.argv) > 2 else range(1, pages(pdf) + 1)
    for p in pp:
        for y, x0, x1, text in lines_of(pdf, p):
            print(f'{p:4d} y{y:8.2f} x{x0:7.2f}-{x1:7.2f} {text}')
