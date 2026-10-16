#!/usr/bin/env python3
"""One document's half of `size-census-2.py`, so a sweep can run the census inline.

    size-census-doc.py <ours.pdf> <ref.pdf> <name>

Round twenty-one lost its census to exactly the sequence this script exists to remove: the
whole-track sweep finished, its rendered PDFs were freed to make room, and the census — which
needs *both* renderings on disk — could then not be resumed. Running it after the sweep is what
makes it cancellable; running it *inside* the sweep's comparison pass, per document, next to
the image diff, makes it cost one pass over PDFs that are open anyway and parallelise across the
sweep's own workers instead of running serially afterwards under whatever load is left.

Emits one tab-separated row per disagreeing page —

    name  page  ours  ref  verdict

— and nothing at all for a page whose dominant size agrees, which is most of them. Aggregate
with `size-census-fold.py`. The classification is `size-census-2.py`'s, unchanged and imported
rather than copied, so the two cannot drift.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'slides-r15'))
sys.path.insert(0, os.path.join(HERE, '..', 'slides-r18'))

import importlib.util

spec = importlib.util.spec_from_file_location(
    'size_census_2', os.path.join(HERE, '..', 'slides-r18', 'size-census-2.py'))
census = importlib.util.module_from_spec(spec)
spec.loader.exec_module(census)


def main():
    if len(sys.argv) != 4:
        print(__doc__, file=sys.stderr)
        return 2

    ours_pdf, ref_pdf, name = sys.argv[1:4]
    try:
        a = census.sizes(ours_pdf)
        b = census.sizes(ref_pdf)
    except Exception as exc:                            # noqa: BLE001 — a probe
        print(f'{name}\t-\t-\t-\tunreadable: {exc}')
        return 0

    common = sorted(set(a) & set(b))
    # The page total is what the aggregate divides by, so it has to be emitted even when every
    # page agrees — otherwise a document that is entirely correct vanishes from the denominator.
    print(f'{name}\t0\t0\t0\tPAGES {len(common)}')

    for p in common:
        verdict = census.classify(a[p], b[p])
        if verdict is None:
            continue
        da, db, kind = verdict
        print(f'{name}\t{p}\t{da:.2f}\t{db:.2f}\t{kind}')

    return 0


if __name__ == '__main__':
    sys.exit(main())
