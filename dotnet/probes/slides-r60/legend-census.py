#!/usr/bin/env python3
"""Our legend's key and text against the reference's, on a page both stacks draw a legend on.

The residue the round left: after `c:crossBetween`, seventeen of the fifty-seven chart pages
the plot-rect census can measure sit at dRight -2.71, -2.73 or -2.88, one number across bar,
column, line and area alike.  This says where it comes from -- the legend's left edge and the
plot's right edge move together by the same amount, and the key's size and the key-to-text gap
agree between the two stacks to within 0.02 pt, so the surplus is in the legend box's own width.
"""
import sys
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
sys.path.insert(0, '/c/sandbox/workdir/scratch-r60-slides')
from fills import fills
from textpos import texts
from pg import page_stream


def legend(path, page, floor):
    stream = page_stream(path, page)
    keys = [f for f in fills(stream)
            if 0 < (f[3] - f[1]) < 12 and 0 < (f[4] - f[2]) < 12 and f[1] > floor]
    runs = [t for t in texts(stream) if t[0] > floor]
    if not keys or not runs:
        return None
    left = min(f[1] for f in keys)
    width = max(f[3] - f[1] for f in keys)
    pen = min(t[0] for t in runs if t[0] > left)
    return left, width, pen


if __name__ == '__main__':
    ours_dir, ref_dir, floor = sys.argv[1], sys.argv[2], float(sys.argv[3])
    print("doc\tkeyLeftOurs\tkeyLeftRef\tdKeyLeft\tkeyWOurs\tkeyWRef\tdTextPen")
    for name in sorted(sys.argv[4:]):
        a = legend(f"{ours_dir}/{name}.pdf", 0, floor)
        b = legend(f"{ref_dir}/{name}.pdf", 0, floor)
        if a is None or b is None:
            continue
        print("%s\t%.2f\t%.2f\t%+.2f\t%.2f\t%.2f\t%+.2f"
              % (name, a[0], b[0], a[0] - b[0], a[1], b[1], a[2] - b[2]))
