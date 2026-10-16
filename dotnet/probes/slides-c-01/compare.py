#!/usr/bin/env python3
"""Count how many renderings differ between two sweeps of the same corpus track.

    compare.py <base-dir> <after-dir> [--list]

Reports both the raw byte comparison and the comparison with every /CreationDate
and /ModDate string replaced, because the difference between those two numbers is
itself a check: with SOURCE_DATE_EPOCH pinned they must agree, and if they do not,
the dates were not pinned and every other number in the sweep is suspect.
"""
import re
import sys
from pathlib import Path

DATE = re.compile(rb'/(?:Creation|Mod)Date\s*\((?:[^()\\]|\\.)*\)')


def normalised(path: Path) -> bytes:
    return DATE.sub(b'/Date(NORMALISED)', path.read_bytes())


def main() -> int:
    base, after = Path(sys.argv[1]), Path(sys.argv[2])
    show = '--list' in sys.argv[3:]

    names = sorted({p.name for p in base.glob('*.pdf')} | {p.name for p in after.glob('*.pdf')})
    raw_changed, norm_changed, missing = [], [], []

    for name in names:
        a, b = base / name, after / name
        if not a.exists() or not b.exists():
            missing.append(name)
            continue
        if a.read_bytes() != b.read_bytes():
            raw_changed.append(name)
            if normalised(a) != normalised(b):
                norm_changed.append(name)

    print(f'{base.name} -> {after.name}: {len(names)} renderings, '
          f'{len(raw_changed)} differ raw, {len(norm_changed)} differ after date normalisation'
          + f', {len(missing)} present in only one leg')

    if show:
        for name in norm_changed:
            print('  CHANGED', name)
        for name in missing:
            print('  MISSING', name)

    return 0


if __name__ == '__main__':
    sys.exit(main())
