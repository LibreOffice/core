#!/usr/bin/env python3
"""The x of the text record that follows each level-2 bullet, in draw order.

More robust than a histogram once the autofit re-fits: the bullet is a one-glyph record at
`marL + indent` and its paragraph's first line is the record drawn immediately after it.
"""
import re
import subprocess
import sys

OPS = ('/home/user/libreoffice-core/.claude/skills/render-comparison'
       '/scripts/pdf-ops.py')
ROW = re.compile(r'text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)[^0-9]*(\d+) glyphs')
BULLET_X = float(sys.argv[1])

for pdf in sys.argv[2:]:
    out = subprocess.run(['python3', OPS, 'dump', pdf, '--page', '13', '--only', 'text'],
                         capture_output=True, text=True).stdout
    rows = [(float(m.group(1)), float(m.group(2)), float(m.group(3)), int(m.group(4)))
            for m in ROW.finditer(out)]
    found = []
    for at, (x, y, size, glyphs) in enumerate(rows):
        if abs(x - BULLET_X) > 0.2 or glyphs != 1 or at + 1 >= len(rows):
            continue
        nxt = rows[at + 1]
        found.append((round(nxt[0] - x, 2), size, nxt[2]))
    print(f'{pdf.split("/")[-1]:24s} offset/size/textsize {found}')
