#!/usr/bin/env python3
"""Where does an extra page come from: a blank page inserted, or content shifted?

For one document, prints per page the shape, the extracted word count, and the first words
on the page, for ours and the reference side by side. An *inserted* page shows as a page
with no words and the same first-word sequence resuming after it; an *overflow* shows as
every page's first word walking backwards by a page's worth of text.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

WORD = re.compile(r"[^\W_]+", re.UNICODE)


def pages(pdf: Path):
    out = subprocess.run(["pdfinfo", "-l", "100000", str(pdf)],
                         capture_output=True, text=True).stdout
    sizes = []
    for line in out.splitlines():
        m = re.match(r"Page\s+\d+ size:\s+([\d.]+) x ([\d.]+)", line)
        if m:
            sizes.append("L" if float(m.group(1)) > float(m.group(2)) else "P")
    rows = []
    for i in range(1, len(sizes) + 1):
        t = subprocess.run(["pdftotext", "-f", str(i), "-l", str(i), str(pdf), "-"],
                           capture_output=True, text=True).stdout
        w = WORD.findall(t)
        lines = [ln for ln in t.splitlines() if ln.strip()]
        rows.append((sizes[i - 1], len(w), len(lines), " ".join(w[:6])))
    return rows


if __name__ == "__main__":
    a, b = Path(sys.argv[1]), Path(sys.argv[2])
    ra, rb = pages(a), pages(b)
    print(f"{'#':>3} | ours: shp  wds lns  head                          "
          f"| ref: shp  wds lns  head")
    for i in range(max(len(ra), len(rb))):
        x = ra[i] if i < len(ra) else ("-", 0, 0, "")
        y = rb[i] if i < len(rb) else ("-", 0, 0, "")
        mark = "  " if x[3] == y[3] else " *"
        print(f"{i + 1:>3}{mark}| {x[0]:<3} {x[1]:>5} {x[2]:>4}  {x[3][:32]:<32} | "
              f"{y[0]:<3} {y[1]:>5} {y[2]:>4}  {y[3][:32]}")
