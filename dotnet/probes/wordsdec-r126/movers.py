#!/usr/bin/env python3
"""Which of our renderings moved between two legs, byte for byte.

    movers.py <base-dir> <after-dir> <out.tsv>

Both legs are rendered under the same pinned `SOURCE_DATE_EPOCH`, so a byte difference is the
diff under test and not the clock (`CLAUDE.md`, *The signature of a forgotten pin is a TOTAL*).
"""
import hashlib
import pathlib
import sys

base, after, out = (pathlib.Path(p) for p in sys.argv[1:4])


def digest(d):
    pdfs = sorted(d.glob('*.pdf'))
    if not pdfs:
        return None
    return hashlib.sha256(pdfs[0].read_bytes()).hexdigest()


keys = sorted({p.name for p in base.iterdir() if p.is_dir()}
              | {p.name for p in after.iterdir() if p.is_dir()})
moved, same, missing = 0, 0, 0
with open(out, 'w') as fh:
    fh.write('identity\tverdict\n')
    for k in keys:
        a, b = digest(base / k), digest(after / k)
        if a is None or b is None:
            v = 'missing'
            missing += 1
        elif a == b:
            v = 'same'
            same += 1
        else:
            v = 'moved'
            moved += 1
        fh.write('%s\t%s\n' % (k, v))
print('%d documents: %d moved, %d byte-identical, %d missing' % (len(keys), moved, same, missing))
