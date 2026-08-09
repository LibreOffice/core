#!/usr/bin/env python3
"""The plot rectangle a PDF page draws, read off its gridlines and axis lines.

r29's plotrect.py took the *modal* span of each orientation, which on a page whose axis
carries more tick marks than gridlines returns the tick's own 4 pt span. This takes the
longest span that at least three strokes share, which is the grid.
"""
import re, subprocess, sys, collections

OPS = "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
LINE = re.compile(r"^stroke\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)")

def rect(pdf, page, minlen=20.0, minshare=3):
    out = subprocess.run([sys.executable, OPS, "dump", pdf, "--page", str(page)],
                         capture_output=True, text=True).stdout
    vert, horiz = collections.Counter(), collections.Counter()
    for ln in out.splitlines():
        m = LINE.match(ln)
        if not m: continue
        x0, y0, x1, y1 = (round(float(m.group(i)), 2) for i in (2, 3, 4, 5))
        if abs(x1 - x0) < 0.3 and abs(y1 - y0) >= minlen: vert[(min(y0,y1), max(y0,y1))] += 1
        elif abs(y1 - y0) < 0.3 and abs(x1 - x0) >= minlen: horiz[(min(x0,x1), max(x0,x1))] += 1
    def pick(c):
        ok = [(b - a, a, b) for (a, b), n in c.items() if n >= minshare]
        return max(ok)[1:] if ok else None
    v, h = pick(vert), pick(horiz)
    if v is None or h is None: return None
    return (h[0], v[0], h[1], v[1])   # left, bottom, right, top  (PDF coords, y up)

if __name__ == "__main__":
    pdf = sys.argv[1]
    for p in sys.argv[2].split(","):
        print(p, rect(pdf, int(p)))

FILL = re.compile(r"^fill\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(#\w+)")

def fillrect(pdf, page, colour):
    """The plot rectangle read off a stated plot-area fill, for probes that carry one."""
    out = subprocess.run([sys.executable, OPS, "dump", pdf, "--page", str(page)],
                         capture_output=True, text=True).stdout
    for ln in out.splitlines():
        m = FILL.match(ln)
        if m and m.group(6).upper() == colour.upper():
            return tuple(float(m.group(i)) for i in (2, 3, 4, 5))
    return None
