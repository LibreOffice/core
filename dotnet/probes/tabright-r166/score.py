#!/usr/bin/env python3
"""Does the page number stay on the entry's first line? One `#` or `.` per arm.

`pdftotext` emits one output line per line the renderer drew, so whether `2-123`
shares an arm's tag line is read straight off it. No coordinate is used: the question
is grouping, not position, and `-bbox`'s `yMin` is a font-descriptor ink box anyway.
"""
import os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from build import RIGHTS, TITLES


def table(pdf):
    text = subprocess.run(["pdftotext", "-nopgbrk", pdf, "-"], capture_output=True, text=True).stdout
    out = {(r, n): "?" for r in RIGHTS for n in TITLES}
    current = None
    for line in text.splitlines():
        m = re.search(r'\br(\d+)n(\d+)\b', line)
        if m:
            current = (int(m.group(1)), int(m.group(2)))
            # The tag's own line is the entry's first: the number is on it or it is not.
            # `2-?123`: where the number overlaps the leader the extractor loses the hyphen, and a
            # literal match then reads an entry that did fit as one that did not.
            out[current] = "#" if re.search(r'2-?123', line) else "."
            continue
        if current is not None and re.search(r'2-?123', line):
            current = None
    return out


def show(name, t):
    print(f"{name:>6} | " + "".join(f"{r:>5}" for r in RIGHTS))
    for n in TITLES:
        print(f"{n:>6} | " + "".join(f"{t[(r, n)]:>5}" for r in RIGHTS))


if __name__ == "__main__":
    for arm in sys.argv[1:]:
        print(f"=== {arm} ===")
        show(arm, table(os.path.join(HERE, arm, "toc-righttab.pdf")))
        print()
