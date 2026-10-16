#!/usr/bin/env python3
"""Where the first line of a level-2 bullet starts, ours against 26.2.4.2.

`Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx` page 13 is the top of that
deck's diff% ranking at 15.93 and holds no chart. Its master's body list states
lvl2 `marL="742950" indent="-285750"` with an en-dash bullet in Arial, so the bullet sits 22.5 pt
left of a 58.5 pt margin. Level 1 agrees exactly; level 2's FIRST line does not.
"""
import re
import subprocess

OPS = ('/home/user/libreoffice-core/.claude/skills/render-comparison'
       '/scripts/pdf-ops.py')
TEXT = re.compile(r'text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)')


def runs(pdf: str, page: str) -> list[tuple[float, float]]:
    out = subprocess.run(['python3', OPS, 'dump', pdf, '--page', page, '--only', 'text'],
                         capture_output=True, text=True).stdout
    return [(float(m.group(1)), float(m.group(2))) for m in TEXT.finditer(out)]


for label, pdf in (('ours', 'ss/ours.pdf'), ('ref', 'ss/ref.pdf')):
    at = {y: x for x, y in runs(pdf, '13')}
    print(label, {round(y): round(x, 2) for y, x in sorted(at.items(), reverse=True)})
