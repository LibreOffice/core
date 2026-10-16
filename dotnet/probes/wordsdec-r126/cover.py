#!/usr/bin/env python3
"""Merged horizontal rule cover per page, both shapes, duplicates collapsed (C16).

    cover.py <pdf> [first] [last]
"""
import sys
import pymupdf

TOL_Y = 1.2


def rules(page):
    out = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 3.0 and r.width >= 0.5 and k in ('f', 'fs', 's'):
                    out.append((r.x0, r.x1, (r.y0 + r.y1) / 2))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 3.0 and abs(a.x - b.x) >= 0.5:
                    out.append((min(a.x, b.x), max(a.x, b.x), (a.y + b.y) / 2))
    return out


def cover(page):
    bands = {}
    for x0, x1, y in rules(page):
        bands.setdefault(round(y / TOL_Y), []).append((x0, x1))
    total = 0.0
    for spans in bands.values():
        spans.sort()
        cur = None
        for a, b in spans:
            if cur and a <= cur[1] + 0.5:
                cur[1] = max(cur[1], b)
            else:
                if cur:
                    total += cur[1] - cur[0]
                cur = [a, b]
        if cur:
            total += cur[1] - cur[0]
    return total


doc = pymupdf.open(sys.argv[1])
first = int(sys.argv[2]) if len(sys.argv) > 2 else 1
last = int(sys.argv[3]) if len(sys.argv) > 3 else doc.page_count
grand = 0.0
for i in range(first - 1, min(last, doc.page_count)):
    c = cover(doc[i])
    grand += c
    if last - first < 30:
        print('page %d\t%.2f' % (i + 1, c))
print('TOTAL %s pages %d-%d\t%.2f' % (sys.argv[1].split('/')[-1], first, min(last, doc.page_count), grand))
