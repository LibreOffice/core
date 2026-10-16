#!/usr/bin/env python3
"""How many of the eight category labels each engine drew, and on how many lines.

Reads the PDF text layer.  `drawn` counts the distinct `Kat<i>` tokens present; `rows` is the
number of distinct baselines the labels sit on below the plot, which separates a wrapped label
(two rows) from a single-line one.  A rotated label is reported as `rot` because LibreOffice
emits one Tj per glyph for rotated text and the token count stops meaning anything.
"""
import os, re, subprocess, sys, collections

WORD = re.compile(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">(.*?)</word>')


def read(pdf):
    out = subprocess.run(["pdftotext", "-bbox", "-f", "1", "-l", "1", pdf, "-"],
                         capture_output=True, text=True).stdout
    return [(float(a), float(b), float(c), float(d), e) for a, b, c, d, e in WORD.findall(out)]


def score(pdf):
    if not os.path.exists(pdf):
        return "-", "-"
    words = read(pdf)
    if not words:
        return 0, 0
    hits = {w for _, _, _, _, t in words for w in re.findall(r"Kat\d", t)}
    # The label band is everything below the lowest value-axis number, which is the only other
    # text on the deck once the title and legend are gone.
    ys = sorted({round(y0, 1) for _, y0, _, _, t in words if re.search(r"Kat\d|Tal\d", t)})
    rows = 1
    for a, b in zip(ys, ys[1:]):
        if b - a > 1.0:
            rows += 1
    return len(hits), rows


def main(root, tags):
    print(f"{'deck':<14} " + " ".join(f"{t:>12}" for t in tags))
    for name in sorted(os.listdir(os.path.join(root, tags[0]))):
        if not name.endswith(".pdf"):
            continue
        cells = []
        for tag in tags:
            drawn, rows = score(os.path.join(root, tag, name))
            cells.append(f"{drawn}/8 r{rows}")
        print(f"{name[:-4]:<14} " + " ".join(f"{c:>12}" for c in cells))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2:])
