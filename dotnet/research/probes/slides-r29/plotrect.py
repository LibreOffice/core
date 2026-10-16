#!/usr/bin/env python3
"""Plot-area rectangle of a chart page, read off the drawn gridlines.

The plot area is the band every gridline spans: vertical gridlines share one (y0,y1)
pair and horizontal ones share one (x0,x1) pair, so the modal span among strokes of
each orientation is the rectangle both renderers agree to call the plot area. Reading
it from the ink rather than from either model is what makes it comparable across the
two.
"""
import re, subprocess, sys, collections, os

OPS = "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
LINE = re.compile(r"^stroke\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)")

def rect(pdf, page):
    out = subprocess.run([sys.executable, OPS, "dump", pdf, "--page", str(page)],
                         capture_output=True, text=True).stdout
    vert, horiz = collections.Counter(), collections.Counter()
    for ln in out.splitlines():
        m = LINE.match(ln)
        if not m: continue
        x0, y0, x1, y1 = (round(float(m.group(i)), 2) for i in (2, 3, 4, 5))
        if abs(x1 - x0) < 0.3 and abs(y1 - y0) > 2: vert[(y0, y1)] += 1
        elif abs(y1 - y0) < 0.3 and abs(x1 - x0) > 2: horiz[(x0, x1)] += 1
    if not vert or not horiz: return None
    (y0, y1), nv = vert.most_common(1)[0]
    (x0, x1), nh = horiz.most_common(1)[0]
    if nv < 3 or nh < 3: return None
    return (x0, y0, x1, y1, nv, nh)

if __name__ == "__main__":
    ours, ref = sys.argv[1], sys.argv[2]
    pages = [int(p) for p in sys.argv[3].split(",")]
    print(f"{'page':>4} {'edge':>7} {'ours':>9} {'ref':>9} {'ours-ref':>9}")
    for p in pages:
        a, b = rect(ours, p), rect(ref, p)
        if a is None or b is None:
            print(f"{p:>4}  no gridline rectangle (ours={a is not None} ref={b is not None})")
            continue
        for i, name in enumerate(("left", "bottom", "right", "top")):
            print(f"{p:>4} {name:>7} {a[i]:>9.2f} {b[i]:>9.2f} {a[i]-b[i]:>9.2f}")
