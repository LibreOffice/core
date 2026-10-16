#!/usr/bin/env python3
"""Batch 2: where 26.2.4.2 stops drawing a shape body that outgrows its shape.

One break run kept in each body; the break run's `w:sz` is swept across the threshold, at each
of the three `anchor` values, with a tall-shape control for every arm that draws nothing so the
drop can be attributed to the height and to nothing else.
"""
import pathlib
import re
import sys
import zipfile

import mutate

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'fixtures2')
SIZES = (16, 18, 19, 20, 21, 22, 24, 28, 32, 40)


def build():
    arms = {}
    for sz in SIZES:
        for anc in ('t', 'ctr', 'b'):
            arms['s%02d-%s' % (sz, anc)] = (
                lambda d, sz=sz, anc=anc:
                mutate.anchor(mutate.break_size(mutate.keep(d, 1), sz), anc))
        # the control: the same body in a shape 1 000 000 EMU taller
        arms['s%02d-tall' % sz] = (
            lambda d, sz=sz:
            mutate.cy(mutate.anchor(mutate.break_size(mutate.keep(d, 1), sz), 't'), 1414655))
    # vertOverflow, measured at a size that DOES overflow
    for val, fn in (('overflow', lambda d: d),
                    ('clip', lambda d: mutate.overflow(d, 'clip')),
                    ('absent', lambda d: mutate.overflow(d, None))):
        arms['ovf40-%s' % val] = (
            lambda d, fn=fn:
            mutate.anchor(mutate.break_size(mutate.keep(fn(d), 1), 40), 't'))
    # four breaks, size swept: the witness's own shape
    for sz in (4, 8, 10, 12, 14, 16):
        arms['q%02d' % sz] = (lambda d, sz=sz: mutate.break_size(d, sz))
    return arms


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(mutate.SRC) as z:
        names = z.namelist()
        blobs = {n: z.read(n) for n in names}
    doc = blobs['word/document.xml'].decode('utf8')
    for name, fn in sorted(build().items()):
        mutated = fn(doc)
        assert mutated != doc or name == 'q04', name
        target = OUT / ('%s.docx' % name)
        with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as z:
            for n in names:
                z.writestr(n, mutated.encode('utf8') if n == 'word/document.xml' else blobs[n])
        print('%-14s %8d' % (name, target.stat().st_size))


if __name__ == '__main__':
    main()
