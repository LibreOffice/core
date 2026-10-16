#!/usr/bin/env python3
"""Per-page direction: did a page's line work move toward the reference, or away?

    direction.py <before-dir> <after-dir> <ref-dir> <ids-file> [--max-pages N]

Vector rather than raster, deliberately. `pdf-image-diff.py` rasterises at 512 px, which on A4
is ~1.6 pt per pixel; the whole effect here is a 0.75 pt overlap on a 0.75 pt line. A round that
quoted only `diff%` would score this as noise. Pixels are the secondary check, not this one.

Two numbers per page, each with a sign:

  dist  |distinct lines ours - distinct lines reference|.  DISTINCT, because LibreOffice draws
        the far column's and far row's border twice (Array::CreateB2DPrimitiveRange expands its
        loop one cell past the range on every side), so its raw count is systematically higher
        than its geometry and a raw comparison would punish a correct renderer.

  over  the length, in points, covered more than once on one grid line at one width and colour,
        counted AFTER exactly-coincident repeats are folded together.  This is the *overhang*
        double-ink -- a segment running past its cell into its neighbour's -- and it is what the
        reader sees as a heavier, slightly ragged rule.

  dup   the length of exactly-coincident repeats.  Kept apart from `over` because it is a
        different act: LibreOffice draws the far column's right edge and the far row's bottom
        edge twice, at identical coordinates, and no renderer should copy that.  Folding it into
        `over` made the reference look like the heavier-inking side, which it is not.

A page is `closer` when a measure moved toward the reference's own value and `further` when it
moved away.  Pages are compared position for position and only for documents whose page count
equals the reference's, because past a pagination divergence page N is not page N.
"""
import argparse, collections, importlib.util, os, sys

_here = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("strokes", os.path.join(_here, "strokes.py"))
_argv = sys.argv[:]
S = importlib.util.module_from_spec(spec)
spec.loader.exec_module(S)
sys.argv = _argv


def union(spans):
    spans = sorted(spans)
    total = over = 0.0
    ca = cb = None
    for a, b in spans:
        if ca is None:
            ca, cb = a, b
            continue
        if a <= cb:
            over += min(b, cb) - a
            cb = max(cb, b)
        else:
            total += cb - ca
            ca, cb = a, b
    if ca is not None:
        total += cb - ca
    return total, over


def measure(pdf, max_pages):
    out = {}
    for i, stream in enumerate(S.page_streams(pdf), 1):
        if max_pages and i > max_pages:
            break
        seen = collections.Counter()
        raw = 0
        dup = 0.0
        for (_p, kind, orient, at, fr, to, w, col, _i) in S.interpret(stream, i):
            if kind != "stroke":
                continue
            raw += 1
            key = (orient, round(at, 1), round(fr, 1), round(to, 1), w, col)
            if seen[key]:
                dup += abs(to - fr)          # an exactly coincident repeat, not an overhang
            seen[key] += 1
        by_line = collections.defaultdict(list)
        for (orient, at, fr, to, w, col) in seen:
            by_line[(orient, round(at * 2) / 2, w, col)].append((fr, to))
        over = 0.0
        for spans in by_line.values():
            over += union(spans)[1]
        classes = collections.Counter()
        for (orient, at, fr, to, w, col) in seen:
            classes[(w, col)] += 1
        out[i] = (len(seen), over, dup, raw, classes)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("before"); ap.add_argument("after"); ap.add_argument("ref")
    ap.add_argument("ids"); ap.add_argument("--max-pages", type=int, default=0)
    a = ap.parse_args()
    print("id\tpage\tdist_b\tdist_a\tdist_r\tover_b\tover_a\tover_r\tdup_b\tdup_a\tdup_r"
          "\tshr_b\tshr_a\tshr_r")
    for line in open(a.ids):
        pid = line.strip()
        if not pid:
            continue
        paths = [os.path.join(d, pid) for d in (a.before, a.after, a.ref)]
        if not all(os.path.exists(p) for p in paths):
            print(f"# missing {pid}", file=sys.stderr)
            continue
        try:
            b, af, r = (measure(p, a.max_pages) for p in paths)
        except Exception as exc:                                   # noqa: BLE001
            print(f"# failed {pid}: {exc}", file=sys.stderr)
            continue
        if len(b) != len(r):
            print(f"# pagecount {pid} {len(b)} vs {len(r)}", file=sys.stderr)
            continue
        for p in sorted(b):
            db, ob, ub, _, cb = b[p]; da, oa, ua, _, ca = af[p]; dr, orr, ur, _, cr = r[p]
            # Restricted to the (width, colour) classes BOTH sides draw on this page. A class
            # only one side draws is a different defect -- TK-Syllabus's reference carries red
            # 0.794 pt strike-through rules we never draw at all -- and folding it into a
            # coalescing measurement would score that as over-merging.
            shared = set(ca) & set(cr)
            sb = sum(v for k, v in cb.items() if k in shared)
            sa = sum(v for k, v in ca.items() if k in shared)
            sr = sum(v for k, v in cr.items() if k in shared)
            print(f"{pid}\t{p}\t{db}\t{da}\t{dr}\t{ob:.2f}\t{oa:.2f}\t{orr:.2f}"
                  f"\t{ub:.2f}\t{ua:.2f}\t{ur:.2f}\t{sb}\t{sa}\t{sr}")


if __name__ == "__main__":
    main()
