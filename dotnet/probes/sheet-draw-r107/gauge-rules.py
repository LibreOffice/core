#!/usr/bin/env python3
"""Every full-width horizontal rule of one page, ours beside 26.2.4.2's.

    gauge-rules.py <ours.pdf> <reference.pdf> <page> [<page> ...]

O41 says `EHEST-Pre-departure-checklist`'s *row grid* is about 2.5 pt out on pages 13 and 15.
This separates the page's two populations of horizontal rule — the sheet's own cell borders,
which run the full printed width, and the embedded gauge chart's value-axis gridlines, which
run only between the plot rectangle's own edges — and pairs each of ours with the nearest of
the reference's inside its own population.

The two populations answer the seat differently, which is the whole point of splitting them.

Coordinates are the PDF's own, bottom-left origin, as `pdf-ops.py dump` prints them.
"""
import re
import subprocess
import sys

OPS = "/home/user/wt-sheetdraw/.claude/skills/render-comparison/scripts/pdf-ops.py"

# The gauge's plot rectangle is inset from the printed area on both sides in both renderings;
# no sheet rule starts right of this or ends left of that.
GAUGE_LEFT, GAUGE_RIGHT = 60.0, 490.0

# A rule is horizontal within this, and has to be at least this long to be a rule at all.
FLAT = 0.06
MINIMUM_LENGTH = 200.0

RECORD = re.compile(
    r"stroke\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(\S+)")


def rules(pdf, page):
    """{y: (x0, x1, colour, is_gauge)} for every full-width horizontal stroke on one page."""
    dump = subprocess.run(
        ["python3", OPS, "dump", pdf, "--page", str(page)],
        capture_output=True, text=True, check=False).stdout

    out = {}
    for line in dump.splitlines():
        found = RECORD.match(line)
        if not found:
            continue
        x0, y0, x1, y1 = (float(v) for v in found.groups()[:4])
        if abs(y0 - y1) > FLAT or (x1 - x0) < MINIMUM_LENGTH:
            continue
        out[round(y0, 2)] = (
            round(x0, 2), round(x1, 2), found.group(5), x0 > GAUGE_LEFT and x1 < GAUGE_RIGHT)
    return out


def main(ours, reference, pages):
    print("page\tband\tours_y\tref_y\tdy\tours_x0\tours_x1\tref_x0\tref_x1\tours_rgb\tref_rgb")
    for page in pages:
        mine, theirs = rules(ours, page), rules(reference, page)
        for y, (x0, x1, colour, gauge) in sorted(mine.items()):
            same = [z for z in theirs if theirs[z][3] == gauge]
            if not same:
                print(f"{page}\t{'gauge' if gauge else 'sheet'}\t{y}\t-\t-")
                continue
            near = min(same, key=lambda z: abs(z - y))
            rx0, rx1, rcolour, _ = theirs[near]
            print(f"{page}\t{'gauge' if gauge else 'sheet'}\t{y:.2f}\t{near:.2f}"
                  f"\t{abs(near - y):.3f}\t{x0}\t{x1}\t{rx0}\t{rx1}\t{colour}\t{rcolour}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], [int(p) for p in sys.argv[3:]])
