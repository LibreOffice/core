#!/usr/bin/env python3
"""Join two sweeps of one column and report what moved, failures excluded.

A row that failed on either side in *either* run is dropped from both totals before
anything is compared, because such a row is evidence about the box rather than about the
tree — see CLAUDE.md, "A gate run under CPU contention undercounts ... on the REFERENCE
side". Here the reference half is banked rather than rendered, so a `ref-failed` row can
only mean the bank has no PDF for it; the exclusion is kept anyway so the two totals are
comparable with `probes/ods-notes-r92/compare.py`'s.

Usage: compare.py <a/rows.tsv> <b/rows.tsv>
"""
import sys
from pathlib import Path


def load(path):
    rows = {}
    for line in Path(path).read_text(encoding='utf8').splitlines():
        if not line.strip():
            continue
        fields = line.split('\t')
        rows[fields[0]] = fields
    return rows


def main():
    a = load(sys.argv[1])
    b = load(sys.argv[2])

    shared = sorted(set(a) & set(b))
    only_a = sorted(set(a) - set(b))
    only_b = sorted(set(b) - set(a))

    bad = {k for k in shared
           if 'failed' in a[k][6] or 'failed' in b[k][6]}

    print(f'A {sys.argv[1]}')
    print(f'B {sys.argv[2]}')
    print(f'rows: A {len(a)}, B {len(b)}, shared {len(shared)}, '
          f'A-only {len(only_a)}, B-only {len(only_b)}')
    print(f'excluded (failed on either side in either run): {len(bad)}')
    for k in sorted(bad):
        print(f'   {k}\tA={a[k][6]}\tB={b[k][6]}')

    scored = [k for k in shared if k not in bad]
    am = sum(1 for k in scored if a[k][6] == 'match')
    bm = sum(1 for k in scored if b[k][6] == 'match')
    print(f'comparable rows: {len(scored)}')
    print(f'match: A {am}, B {bm}')

    gained = [k for k in scored if a[k][6] != 'match' and b[k][6] == 'match']
    lost = [k for k in scored if a[k][6] == 'match' and b[k][6] != 'match']
    changed = [k for k in scored
               if a[k][6] != b[k][6] or a[k][2] != b[k][2] or a[k][8] != b[k][8]]

    print(f'verdicts gained: {len(gained)}')
    for k in gained:
        print(f'   + {k}\t{a[k][6]} -> {b[k][6]}\tpages {a[k][2]} -> {b[k][2]}'
              f'\tglyphs {a[k][8]} -> {b[k][8]}')
    print(f'verdicts lost: {len(lost)}')
    for k in lost:
        print(f'   - {k}\t{a[k][6]} -> {b[k][6]}\tpages {a[k][2]} -> {b[k][2]}'
              f'\tglyphs {a[k][8]} -> {b[k][8]}')
    print(f'rows whose columns moved at all: {len(changed)}')
    for k in changed:
        print(f'   ~ {k}\t{a[k][6]} -> {b[k][6]}\tpages {a[k][2]} -> {b[k][2]}'
              f'\tglyphs {a[k][8]} -> {b[k][8]}')


if __name__ == '__main__':
    main()
