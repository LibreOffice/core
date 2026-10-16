#!/usr/bin/env python3
"""How far a pie's labels sit from its centre, as a fraction of its radius.

    ./labelreach.py <pdf> [<pdf> ...]

The pie's centre and radius come from the union of the page's own wedge fills -- a wedge's path
runs centre -> rim -> centre, so together they box the circle -- and each label's distance is
measured to the text object's own origin. Reporting the RATIO rather than the distance is what
makes the two renderers comparable when their pies are different sizes, which on
029_Unit_Circle_Chart_Pie_Theme they are by 2 pt.
"""
import math, re, subprocess, sys

OPS = '/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py'
FILL = re.compile(r'fill\s+p1\s+\(\s*([\d.-]+),\s*([\d.-]+)\)-\(\s*([\d.-]+),\s*([\d.-]+)\)\s+(#\w+)')
TEXT = re.compile(r'text\s+p1\s+\(\s*([\d.-]+),\s*([\d.-]+)\)\s+([\d.]+)pt\s+(\S+)\s+(\d+) glyphs')


def dump(pdf, what):
    return subprocess.run(['python3', OPS, 'dump', pdf, '--page', '1', '--only', what],
                          capture_output=True, text=True).stdout


for pdf in sys.argv[1:]:
    boxes = [tuple(map(float, m.groups()[:4])) + (m.group(5),)
             for m in map(FILL.match, dump(pdf, 'fill').splitlines()) if m]
    wedges = [b for b in boxes
              if b[2] - b[0] > 40 and b[3] - b[1] > 40 and b[4] != '#FFFFFF']
    if not wedges:
        print(f'{pdf}: no wedges'); continue

    left = min(b[0] for b in wedges); right = max(b[2] for b in wedges)
    low = min(b[1] for b in wedges); high = max(b[3] for b in wedges)
    cx, cy, r = (left + right) / 2, (low + high) / 2, (right - left) / 2

    print(f'{pdf}\n  centre ({cx:.2f},{cy:.2f}) radius {r:.2f}  wedge fills {len(wedges)}')
    for m in map(TEXT.match, dump(pdf, 'text').splitlines()):
        if not m: continue
        x, y, n = float(m.group(1)), float(m.group(2)), int(m.group(5))
        d = math.hypot(x - cx, y - cy)
        if n > 2 and d < r * 1.4:
            print(f'    ({x:8.2f},{y:8.2f}) {n:3d} glyphs  d {d:7.2f}  d/r {d / r:5.3f}')
