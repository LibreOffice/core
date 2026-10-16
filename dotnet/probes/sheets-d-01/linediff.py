#!/usr/bin/env python3
"""Compare two PDFs' axis-aligned line work, per page, without rasterising.

    linediff.py <ours.pdf> <ref.pdf> [--page N]

Reports, for each page and each side:
  lines     - number of stroke records
  distinct  - number of DISTINCT (orient, at, from, to, width, colour) records.  LibreOffice
              emits the far column's and far row's border twice, because
              Array::CreateB2DPrimitiveRange expands its loop one cell beyond the range on
              every side, so a raw count is systematically two higher than the geometry.
  ink       - total inked length as a union of intervals per grid line, in points.  This is
              the number that does NOT flatter a renderer for splitting one run into ten,
              and does not flatter it for double-inking a joint either.
  over      - length covered by more than one stroke on the same grid line at the same width
              and colour: the double-inked overlap.  Zero is what a correct renderer gives
              except where LibreOffice itself overlaps (a run broken by a style change with a
              perpendicular border crossing the break).

The grid-line key rounds `at` to 0.5 pt so the two renderers' sub-tenth-point disagreement
about where a line sits does not split a comparison in two; that tolerance is far below a
row height and far above the observed 0.03 pt offset.
"""
import argparse, collections, importlib.util, sys

spec = importlib.util.spec_from_file_location(
    "strokes", "/c/sandbox/workdir/wt-sheets-d/dotnet/probes/sheets-d-01/strokes.py")
S = importlib.util.module_from_spec(spec)
_argv = sys.argv[:]                     # strokes.py imports argparse at module scope
spec.loader.exec_module(S)
sys.argv = _argv


def collect(pdf, only=None):
    pages = collections.defaultdict(list)
    for i, stream in enumerate(S.page_streams(pdf), 1):
        if only and i != only:
            continue
        for r in S.interpret(stream, i):
            if r[1] != "stroke":
                continue
            pages[i].append(r)
    return pages


def union(spans):
    spans = sorted(spans)
    total = 0.0
    over = 0.0
    cur_a, cur_b = None, None
    for a, b in spans:
        if cur_a is None:
            cur_a, cur_b = a, b
            continue
        if a <= cur_b:
            over += min(b, cur_b) - a
            cur_b = max(cur_b, b)
        else:
            total += cur_b - cur_a
            cur_a, cur_b = a, b
    if cur_a is not None:
        total += cur_b - cur_a
    return total, over


def summarise(records):
    by_line = collections.defaultdict(list)
    distinct = set()
    for (_p, _k, orient, at, fr, to, w, col, _i) in records:
        by_line[(orient, round(at * 2) / 2, w, col)].append((fr, to))
        distinct.add((orient, round(at, 1), round(fr, 1), round(to, 1), w, col))
    ink = over = 0.0
    for spans in by_line.values():
        t, o = union(spans)
        ink += t
        over += o
    return len(records), len(distinct), ink, over


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("ours"); ap.add_argument("ref"); ap.add_argument("--page", type=int)
    a = ap.parse_args()
    mine, theirs = collect(a.ours, a.page), collect(a.ref, a.page)
    print("page\tlines_o\tlines_r\tdist_o\tdist_r\tink_o\tink_r\tover_o\tover_r")
    for p in sorted(set(mine) | set(theirs)):
        lo, do, io, oo = summarise(mine.get(p, []))
        lr, dr, ir, orr = summarise(theirs.get(p, []))
        print(f"{p}\t{lo}\t{lr}\t{do}\t{dr}\t{io:.1f}\t{ir:.1f}\t{oo:.2f}\t{orr:.2f}")


if __name__ == "__main__":
    main()
