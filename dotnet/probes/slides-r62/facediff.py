#!/usr/bin/env python3
"""Face histogram disagreement, ours against the reference, over every chart page of a sweep.

One number per page: the number of text runs whose (face, size) our side draws and the
reference does not, summed over the multiset difference in both directions.  Zero means the two
pages set the same runs in the same faces at the same sizes.
"""
import collections, sys
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
sys.path.insert(0, '/c/sandbox/workdir/scratch-r62-slides')
from facecensus import faces


def hist(path, page):
    return collections.Counter((f, s) for _, _, f, s, _ in faces(path, page - 1))


if __name__ == '__main__':
    ours, ref, pagefile = sys.argv[1], sys.argv[2], sys.argv[3]
    total = 0
    rows = []
    for line in open(pagefile, encoding='utf-8'):
        name, pages = line.rstrip('\n').split('\t')
        for p in [int(x) for x in pages.split(',')]:
            try:
                a, b = hist(f"{ours}/{name}.pdf", p), hist(f"{ref}/{name}.pdf", p)
            except Exception as e:
                rows.append((name, p, -1, str(e)[:40]))
                continue
            d = sum(((a - b) + (b - a)).values())
            total += d
            if d:
                rows.append((name, p, d, "; ".join(
                    f"+{f}@{s}x{n}" for (f, s), n in sorted((a - b).items()))
                    + " | " + "; ".join(
                    f"-{f}@{s}x{n}" for (f, s), n in sorted((b - a).items()))))
    rows.sort(key=lambda r: -r[2])
    for name, p, d, why in rows:
        print(f"{d:5d}  {name} p{p}\t{why}")
    print(f"TOTAL {total} mismatched runs over {sum(1 for _ in open(pagefile))} documents")
