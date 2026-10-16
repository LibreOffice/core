#!/usr/bin/env python3
"""How far our drawn series curve is from 26.2.4.2's, before and after the spline.

Pairs each of the reference's long stroked polylines with ours by stroke colour and overlapping
bounding box, and reports the mean distance from every reference vertex to our polyline — raw,
and again after removing the constant offset between the two centroids, which separates *the
curve is the wrong shape* from *the curve is in the wrong place*.

Run: python3 curve-deviation.py <before.pdf> <after.pdf> <reference.pdf> <page,page,...>
"""
import math, sys, pymupdf


def polylines(path, pno, minitems=6):
    pg = pymupdf.open(path)[pno]
    out = []
    for dr in pg.get_drawings():
        if dr['type'] != 's':
            continue
        items = dr['items']
        if len(items) < minitems or any(i[0] != 'l' for i in items):
            continue
        pts = [(items[0][1].x, items[0][1].y)] + [(i[2].x, i[2].y) for i in items]
        out.append((dr.get('color'), pts, dr['rect']))
    return out


def to_segment(p, a, b):
    ax, ay = a
    bx, by = b
    dx, dy = bx - ax, by - ay
    length = (dx * dx) + (dy * dy)
    if length == 0:
        return math.hypot(p[0] - ax, p[1] - ay)
    t = max(0.0, min(1.0, (((p[0] - ax) * dx) + ((p[1] - ay) * dy)) / length))
    return math.hypot(p[0] - (ax + (t * dx)), p[1] - (ay + (t * dy)))


def to_polyline(p, poly):
    return min(to_segment(p, poly[i], poly[i + 1]) for i in range(len(poly) - 1))


def mean_distance(reference, ours):
    return sum(to_polyline(p, ours) for p in reference) / len(reference)


def same_colour(a, b):
    return a is not None and b is not None and all(abs(x - y) < 0.02 for x, y in zip(a, b))


def compare(ours_path, ref_path, pno):
    reference = polylines(ref_path, pno)
    ours = polylines(ours_path, pno)
    used = set()
    rows = []
    for colour, pts, rect in reference:
        best = None
        for at, (ocolour, opts, orect) in enumerate(ours):
            if at in used or not same_colour(colour, ocolour):
                continue
            if not pymupdf.Rect(rect).intersects(pymupdf.Rect(orect)):
                continue
            score = abs(rect[0] - orect[0]) + abs(rect[2] - orect[2])
            if best is None or score < best[0]:
                best = (score, at, opts)
        if best is None:
            rows.append(None)
            continue
        used.add(best[1])
        opts = best[2]
        dx = (sum(p[0] for p in pts) / len(pts)) - (sum(p[0] for p in opts) / len(opts))
        dy = (sum(p[1] for p in pts) / len(pts)) - (sum(p[1] for p in opts) / len(opts))
        shifted = [(p[0] + dx, p[1] + dy) for p in opts]
        rows.append((len(pts) - 1, len(opts) - 1,
                     mean_distance(pts, opts), mean_distance(pts, shifted)))
    return rows


def main(before, after, reference, pages):
    print('page\tseries\tref segs\tour segs before/after\tmean pt before/after'
          '\tsame after removing the offset')
    for pno in pages:
        b = compare(before, reference, pno)
        a = compare(after, reference, pno)
        for at, (rb, ra) in enumerate(zip(b, a)):
            if rb is None or ra is None:
                print(f'{pno + 1}\t{at}\t(unmatched)')
                continue
            print(f'{pno + 1}\t{at}\t{rb[0]}\t{rb[1]} / {ra[1]}\t'
                  f'{rb[2]:.2f} / {ra[2]:.2f}\t{rb[3]:.2f} / {ra[3]:.2f}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], sys.argv[3],
         [int(x) - 1 for x in sys.argv[4].split(',')])
