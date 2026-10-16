#!/usr/bin/env python3
"""Per-document fill-operator agreement with the reference, before and against after.

A cell shade moves no gate column, so the only way to say whether 42 changed renderings improved
or worsened is to count the thing that changed and compare both sides to the reference. Reported
per document and never netted: improved, worsened, unchanged.
"""
import collections
import os
import sys

sys.path.insert(0, '/c/sandbox/workdir/wt-words-r50/dotnet/probes/words-r63')
from fillcount import counts  # noqa: E402
from textcolour import page_streams  # noqa: E402
import re  # noqa: E402


def npages(path):
    data = open(path, 'rb').read()
    n = 0
    for m in re.finditer(rb'/Type\s*/Page\b(?!s)', data):
        n += 1
    return n


def total(path):
    """Every fill operator in the document, by colour, over all pages."""
    out = collections.Counter()
    for page in range(1, npages(path) + 1):
        try:
            fills, _ = counts(path, page)
        except Exception:                                          # noqa: BLE001
            continue
        out.update(fills)
    return out


def distance(a, b):
    keys = set(a) | set(b)
    return sum(abs(a.get(k, 0) - b.get(k, 0)) for k in keys)


def main(before, after, ref, names):
    better = worse = same = 0
    for name in names:
        r = total(os.path.join(ref, name))
        d0 = distance(total(os.path.join(before, name)), r)
        d1 = distance(total(os.path.join(after, name)), r)
        mark = 'improved' if d1 < d0 else ('WORSENED' if d1 > d0 else 'unchanged')
        better += d1 < d0
        worse += d1 > d0
        same += d1 == d0
        print('  %-9s  |ours-ref| %5d -> %5d   %s' % (mark, d0, d1, name))
    print('\nimproved %d   worsened %d   unchanged %d' % (better, worse, same))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4:])
