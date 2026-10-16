#!/usr/bin/env python3
"""The x of a level's bullet, its first line and its continuation lines, off a rendered page.

A histogram of text-record x is enough and is robust: a level's continuation lines are the most
common x in the block, its bullet glyph is a one-glyph record at `marL + indent`, and its first
lines are the one remaining cluster between them and the right margin.
"""
import collections
import re
import subprocess
import sys

OPS = ('/home/user/libreoffice-core/.claude/skills/render-comparison'
       '/scripts/pdf-ops.py')
TEXT = re.compile(r'text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)')

for pdf in sys.argv[1:]:
    out = subprocess.run(['python3', OPS, 'dump', pdf, '--page', '13', '--only', 'text'],
                         capture_output=True, text=True).stdout
    rows = [(round(float(m.group(1)), 2), float(m.group(3))) for m in TEXT.finditer(out)]
    seen = collections.Counter(x for x, _ in rows)
    sizes = {s for _, s in rows}
    print(f'{pdf}\n    x {sorted(x for x, _ in seen.most_common(8))}  sizes {sorted(sizes)}')
