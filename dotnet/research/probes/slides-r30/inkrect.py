#!/usr/bin/env python3
"""The plot rectangle a PDF page draws, read off the axis lines it drew.

Three readings were tried and only the third is general.

The *modal* span of each orientation (round twenty-nine's `plotrect.py`) returns the tick
mark's own 4 pt span on any axis carrying more ticks than gridlines. Taking the longest
span at least three strokes share fixes that and still needs a grid, which 47 of the
corpus's 61 chart parts do not draw.

What every cartesian chart does draw is its two axis lines, and they meet: the category
axis runs the width of the plot rectangle along its bottom and the value axis runs its
height up the left, sharing the bottom-left corner. So: find a long horizontal stroke and
a long vertical stroke whose ends coincide, and the rectangle is their two spans. A bar or
a series polyline cannot be mistaken for either, because a rectangle's stroke record spans
both dimensions and is excluded from both lists by construction.
"""
import re, subprocess, sys, collections

OPS = "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
LINE = re.compile(r"^stroke\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)")
FILL = re.compile(r"^fill\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(#\w+)")


def _dump(pdf, page):
    return subprocess.run([sys.executable, OPS, "dump", pdf, "--page", str(page)],
                          capture_output=True, text=True).stdout


def rect(pdf, page, minlen=20.0, tol=0.6, dump=None, within=None):
    """left, bottom, right, top in PDF coordinates, or None.

    `within` is the chart frame in the same coordinates. A slide carrying four charts draws
    four plot rectangles on one page, and without it the reader returns whichever is biggest
    and every chart on that page is scored against it.
    """
    out = dump if dump is not None else _dump(pdf, page)
    vert, horiz = [], []
    for ln in out.splitlines():
        m = LINE.match(ln)
        if not m: continue
        x0, y0, x1, y1 = (float(m.group(i)) for i in (2, 3, 4, 5))
        lo_x, hi_x, lo_y, hi_y = min(x0, x1), max(x0, x1), min(y0, y1), max(y0, y1)
        if within is not None:
            wx0, wy0, wx1, wy1 = within
            if lo_x < wx0 - 1 or hi_x > wx1 + 1 or lo_y < wy0 - 1 or hi_y > wy1 + 1: continue
        if hi_x - lo_x < 0.3 and hi_y - lo_y >= minlen: vert.append((lo_x, lo_y, hi_y))
        elif hi_y - lo_y < 0.3 and hi_x - lo_x >= minlen: horiz.append((lo_y, lo_x, hi_x))

    best = None
    for vx, vy0, vy1 in vert:
        for hy, hx0, hx1 in horiz:
            # The two axis lines meet at the plot rectangle's bottom-left corner.
            if abs(hx0 - vx) > tol or abs(vy0 - hy) > tol: continue
            area = (hx1 - hx0) * (vy1 - vy0)
            if best is None or area > best[0]:
                best = (area, hx0, vy0, hx1, vy1)
    if best: return best[1:]

    # No corner: fall back to the longest span at least three strokes share, which is a grid.
    cv, ch = collections.Counter(), collections.Counter()
    for vx, vy0, vy1 in vert: cv[(round(vy0, 2), round(vy1, 2))] += 1
    for hy, hx0, hx1 in horiz: ch[(round(hx0, 2), round(hx1, 2))] += 1
    def pick(c):
        ok = [(b - a, a, b) for (a, b), n in c.items() if n >= 3]
        return max(ok)[1:] if ok else None
    v, h = pick(cv), pick(ch)
    return None if v is None or h is None else (h[0], v[0], h[1], v[1])


def fillrect(pdf, page, colour):
    """The plot rectangle read off a stated plot-area fill, for probes that carry one."""
    for ln in _dump(pdf, page).splitlines():
        m = FILL.match(ln)
        if m and m.group(6).upper() == colour.upper():
            return tuple(float(m.group(i)) for i in (2, 3, 4, 5))
    return None


if __name__ == "__main__":
    for p in sys.argv[2].split(","):
        print(p, rect(sys.argv[1], int(p)))
