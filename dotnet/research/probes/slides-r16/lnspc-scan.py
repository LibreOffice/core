#!/usr/bin/env python3
"""How many slides-track documents state a proportional *line* spacing, and how low it goes.

The question is narrow: `LineSpacingRule.Apply` clamps a proportion up to 50% — Writer's
`SwTextFormatter::CalcRealHeight` rule — and EditEngine, which is what lays out a slide, has
no such clamp. Dropping the clamp is only safe if little in the track sits below it.

The trap this script exists to avoid: `a:spcPct` is the child of `a:spcBef` and `a:spcAft`
as well as of `a:lnSpc`, and paragraph space-before of 20% is in every stock Office theme.
Matching `a:spcPct` alone reports 84 documents "under 50% line spacing" and essentially all
of it is space-before. Only `a:lnSpc/a:spcPct` counts.
"""
import collections
import os
import re
import sys
import zipfile

ROOT = sys.argv[1] if len(sys.argv) > 1 else '/workspace/sample-files/slides'

# a:lnSpc wraps exactly one child; keep the match tight so a sibling spcBef cannot be caught.
LNSPC = re.compile(rb'<a:lnSpc>\s*<a:spcPct\s+val="(-?\d+)"')

hist = collections.Counter()
docs_with = 0
below50 = []

for dirpath, _dirs, files in os.walk(ROOT):
    for name in sorted(files):
        path = os.path.join(dirpath, name)
        vals = []
        try:
            with zipfile.ZipFile(path) as z:
                for entry in z.namelist():
                    if not entry.endswith('.xml'):
                        continue
                    for m in LNSPC.finditer(z.read(entry)):
                        vals.append(int(m.group(1)))
        except (zipfile.BadZipFile, OSError):
            continue          # .ppt — binary, not reachable this way
        if not vals:
            continue
        docs_with += 1
        for v in vals:
            hist[v] += 1
        positive = [v for v in vals if v > 0]
        if positive and min(positive) < 50000:
            below50.append((path, min(positive)))

print(f'pptx documents stating a:lnSpc/a:spcPct: {docs_with}')
print(f'distinct percentages: {len(hist)}, {sum(hist.values())} occurrences')
print('\nby percentage:')
for v, n in sorted(hist.items()):
    print(f'  {v / 1000:9.3f}%  {n}')

nonwhole = {v: n for v, n in hist.items() if v % 1000 != 0}
print(f'\nnon-whole percentages: {len(nonwhole)} distinct, {sum(nonwhole.values())} occurrences')

print(f'\ndocuments whose lowest stated line proportion is under 50%: {len(below50)}')
for path, low in below50:
    print(f'  {low / 1000:8.3f}%  {path}')
