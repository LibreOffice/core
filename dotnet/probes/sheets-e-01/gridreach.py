#!/usr/bin/env python3
"""How often does Calc's `bSingle` branch fire, over the sheets track?

    gridreach.py <reference-dir> <ours-dir>

A grid rule is a 0.1 pt stroke. `bSingle` shows in the PDF as **several** strokes at one x on
one page where the un-split branch writes one, so counting distinct x positions against
distinct strokes measures the reach of the whole `ScOutputData::DrawGrid` per-row branch —
without needing to know which of its three triggers fired. Page 1 only: the question is how
many *documents* are touched, and a document that never splits on its first page is unlikely to
be the seat of anything.
"""
import importlib.util
import os
import sys

spec = importlib.util.spec_from_file_location(
    "strokes", os.path.join(os.path.dirname(os.path.abspath(__file__)),
                            "..", "sheets-d-01", "strokes.py"))
strokes = importlib.util.module_from_spec(spec)
spec.loader.exec_module(strokes)


def rules(pdf, page=1):
    """Distinct x positions and total segment count for 0.1 pt verticals on one page."""
    try:
        streams = strokes.page_streams(pdf)
    except Exception:
        return None
    if len(streams) < page:
        return {}
    out = {}
    # strokes.interpret yields (page, kind, orient, at, from, to, width, colour, idiom)
    for row in strokes.interpret(streams[page - 1], page):
        if row[2] != "V" or abs(float(row[6]) - 0.1) > 0.02:
            continue
        key = round(float(row[3]), 1)
        out[key] = out.get(key, 0) + 1
    return out


def main(ref, ours):
    names = sorted(f for f in os.listdir(ours) if f.endswith(".pdf"))
    split_docs = split_lines = 0
    grid_docs = 0
    print("stem\trefLines\trefSegments\toursLines\toursSegments")
    for n in names:
        stem = n[:-4]
        cands = [f for f in os.listdir(ref)
                 if f.startswith(stem + "__") and f.endswith(".pdf")]
        if not cands:
            continue
        r = rules(os.path.join(ref, cands[0]))
        o = rules(os.path.join(ours, n))
        if r is None or o is None:
            continue
        rl, rs = len(r), sum(r.values())
        ol, os_ = len(o), sum(o.values())
        if rl >= 3:
            grid_docs += 1
        if rs > rl:
            split_docs += 1
            split_lines += sum(1 for v in r.values() if v > 1)
        print("%s\t%d\t%d\t%d\t%d" % (stem, rl, rs, ol, os_))
    print("# documents with >=3 hairline verticals on page 1: %d" % grid_docs, file=sys.stderr)
    print("# documents whose reference splits at least one: %d" % split_docs, file=sys.stderr)
    print("# split lines on those page 1s: %d" % split_lines, file=sys.stderr)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
