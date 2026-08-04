#!/usr/bin/env python3
"""Turn the probe's row heights into line counts and the k at which each line is added."""
import sys, collections

rows = collections.defaultdict(list)
for line in open(sys.argv[1]):
    p = line.rstrip('\n').split('\t')
    if len(p) < 4 or p[3] == '-': continue
    rows[p[0]].append((int(p[1]), float(p[3])))

for sheet in sorted(rows):
    seq = sorted(rows[sheet])
    # one line is (18px+2px)/0.067 for 11pt; derive the pitch from the first two distinct heights
    heights = sorted({h for _, h in seq})
    base = heights[0]
    pitch = heights[1] - heights[0] if len(heights) > 1 else 0
    trans = []
    prev = None
    for idx, h in seq:
        lines = 1 if pitch == 0 else round((h - base) / pitch) + 1
        if prev is not None and lines != prev:
            trans.append((idx + 1, lines))   # k = idx+1
        prev = lines
    print('%s\tbase=%.1f\tpitch=%.2f\ttransitions(k,lines)=%s'
          % (sheet, base, pitch, trans))
