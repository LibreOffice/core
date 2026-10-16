#!/usr/bin/env python3
"""How often LibreOffice's own PDF draws text horizontally stretched.

A PDF text show carries two independent scales: the `Tf` operand, which is the font's
height, and the text matrix's `a`, which scales it horizontally.  LibreOffice emits a
non-unit `a` whenever the glyph advances it has do not match the ones the font would give
at that height — which is what happens to text inside a metafile that has been scaled by
different factors across and down.

`pdf-ops.py` reports the *effective* size, `a * Tf`, so a stretched run reads there as a
run at some other size.  That is the shape of the "chart text residual is neither a
constant offset nor a constant factor" item: the residual is anisotropic, and half of it
is in a column no instrument in this tree was reading.

Prints, per PDF, the records whose |a - 1| exceeds the threshold, grouped by (a, Tf).
"""
import re, sys, zlib
from collections import Counter
from pathlib import Path

# `a 0 0 d e f Tm` immediately followed by `/Fn size Tf`, which is the shape LibreOffice's
# PDF export writes for every show. A run whose matrix is anything else is skipped rather
# than guessed at.
SHOW = re.compile(
    rb'([\d.]+) 0 0 ([\d.]+) (-?[\d.]+) (-?[\d.]+) Tm\s*/F(\d+) ([\d.]+) Tf')


def streams(data):
    for m in re.finditer(rb'stream\r?\n', data):
        start = m.end()
        end = data.find(b'endstream', start)
        if end < 0:
            continue
        try:
            yield zlib.decompress(data[start:end])
        except Exception:
            continue


def census(path, tolerance):
    seen = Counter()
    for body in streams(open(path, 'rb').read()):
        for m in SHOW.finditer(body):
            a = float(m.group(1))
            size = float(m.group(6))
            seen[(round(a, 6), round(size, 3))] += 1
    total = sum(seen.values())
    stretched = sum(n for (a, _), n in seen.items() if abs(a - 1.0) > tolerance)
    return total, stretched, seen


def main(argv):
    tolerance = 0.002
    paths = [Path(p) for p in argv]
    if len(paths) == 1 and paths[0].is_dir():
        paths = sorted(paths[0].glob('*.pdf'))

    grand = grand_stretched = 0
    hit = 0
    for p in paths:
        total, stretched, seen = census(p, tolerance)
        grand += total
        grand_stretched += stretched
        if stretched:
            hit += 1
            worst = sorted(((n, a, s) for (a, s), n in seen.items()
                            if abs(a - 1.0) > tolerance), reverse=True)[:4]
            detail = '  '.join(f'{n}x a={a:.6f} Tf={s:g}' for n, a, s in worst)
            print(f"{p.name}\t{stretched}/{total}\t{detail}")

    print()
    print(f"documents                {len(paths)}")
    print(f"documents with stretch   {hit}")
    print(f"text shows               {grand}")
    print(f"stretched                {grand_stretched}"
          f"  ({100.0 * grand_stretched / grand:.2f}%)" if grand else "")


if __name__ == '__main__':
    main(sys.argv[1:])
