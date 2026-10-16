#!/usr/bin/env python3
"""Face-set distance to the reference, before against after.

    facedist.py <before-dir> <after-dir>

The instrument a font change is actually about. The gate's font column counts faces and cannot see
that they are the wrong ones; this reads the names `pdffonts` reports for our PDF and for the banked
reference, strips the subset prefix, and reports the symmetric difference — so "we draw a DejaVu
Serif the reference draws in DejaVu Sans" scores 2 and going to an exact match scores 0.

It also reports the verdict movement from the same pair of sweeps, because the two answer different
questions and a round needs both: a font fix routinely moves a face set on thirty documents and a
page count on one.
"""
import sys
import os


def read(path, index, fields):
    out = {}
    with open(path, encoding='utf8') as handle:
        for line in handle:
            parts = line.rstrip('\n').split('\t')
            if len(parts) <= max(fields):
                continue
            out[parts[0]] = tuple(parts[f] for f in fields)
    return out


def faces(cell):
    return {f for f in cell.split(',') if f}


def main(before_dir, after_dir):
    bf = read(os.path.join(before_dir, 'faces.tsv'), 0, [1, 2])
    af = read(os.path.join(after_dir, 'faces.tsv'), 0, [1, 2])
    bp = read(os.path.join(before_dir, 'parity.tsv'), 0, [2, 6])
    ap = read(os.path.join(after_dir, 'parity.tsv'), 0, [2, 6])

    closer = unchanged = further = 0
    changed = []
    for doc in sorted(af):
        if doc not in bf:
            continue
        bours, bref = faces(bf[doc][0]), faces(bf[doc][1])
        aours, aref = faces(af[doc][0]), faces(af[doc][1])
        if bours == aours:
            continue                                   # the rendering did not move at all
        db = len(bours ^ bref)
        da = len(aours ^ aref)
        changed.append((doc, db, da, sorted(bours ^ bref), sorted(aours ^ aref)))
        if da < db:
            closer += 1
        elif da == db:
            unchanged += 1
        else:
            further += 1

    print(f"renderings whose face set moved: {len(changed)}")
    print(f"  closer {closer}   unchanged {unchanged}   further {further}")
    print()
    for doc, db, da, bset, aset in changed:
        mark = 'closer  ' if da < db else ('same    ' if da == db else 'FURTHER ')
        print(f"{mark} {db}->{da}  {doc}")
        if da >= db:
            print(f"           before-diff {bset}")
            print(f"           after-diff  {aset}")

    print()
    print("--- verdicts ---")
    gained, lost, moved = [], [], []
    for doc in sorted(ap):
        if doc not in bp:
            continue
        (bpages, bv), (apages, av) = bp[doc], ap[doc]
        if bv != av:
            (gained if av == 'match' else lost if bv == 'match' else moved).append(
                (doc, bv, av))
        if bpages != apages:
            print(f"pages   {bpages} -> {apages}  {doc}")
    for label, rows in (("gained", gained), ("LOST", lost), ("changed", moved)):
        for doc, bv, av in rows:
            print(f"{label:8} {bv} -> {av}  {doc}")
    print(f"gained {len(gained)}  lost {len(lost)}  other verdict changes {len(moved)}")


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
