#!/usr/bin/env python3
"""Count long thin horizontal strokes per page in two renderings, which is what a `v:line` in a
header or footer draws.

A rule is counted as a stroked path whose bounding box is at least 100 pt wide and under 3 pt
tall. Both a stroke and a thin filled rectangle qualify: LibreOffice draws a hairline either
way and PyMuPDF reports the two differently, so counting only `stroke` items undercounts one
side and reads as a missing rule."""
import sys

import pymupdf


def rules(path):
    document = pymupdf.open(path)
    out = []
    for number, page in enumerate(document, start=1):
        for item in page.get_drawings():
            box = item['rect']
            if box.width >= 100 and box.height <= 3:
                out.append((number, round(box.x0, 1), round(box.y0, 1),
                            round(box.width, 1), round(box.height, 2)))
    return out


def main():
    ours, reference = rules(sys.argv[1]), rules(sys.argv[2])
    print('%-8s ours %d   reference %d' % (sys.argv[3], len(ours), len(reference)))
    only_reference = [r for r in reference if r not in ours]
    only_ours = [r for r in ours if r not in reference]
    print('   in the reference and not in ours: %d' % len(only_reference))
    for r in only_reference[:10]:
        print('      page %d  x %.1f  y %.1f  w %.1f  h %.2f' % r)
    print('   in ours and not in the reference: %d' % len(only_ours))
    for r in only_ours[:10]:
        print('      page %d  x %.1f  y %.1f  w %.1f  h %.2f' % r)


if __name__ == '__main__':
    main()
