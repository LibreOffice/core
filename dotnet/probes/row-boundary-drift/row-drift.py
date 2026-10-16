#!/usr/bin/env python3
"""Measure where a table's row boundaries drift against the reference's.

What it found, 2026-08-15, on `SPA-02_mcar_part-2_and_IS_v2.9.docx` page 5
(26 rules on both sides, so the rows correspond one for one):

    row   ref pitch   our pitch     delta    cum ref   cum ours    cum Δ
      2      29.400      29.400    +0.000     58.080     58.080   +0.000
      3      29.280      29.400    +0.120     87.360     87.480   +0.120
      4      29.400      29.400    +0.000    116.760    116.880   +0.120
      5      29.280      29.400    +0.120    146.040    146.280   +0.240
      …
     23      29.400      29.400    +0.000    686.880    687.960   +1.080
     24      19.680      18.600    -1.080    706.560    706.560   +0.000

**The reference alternates 29.280 and 29.400 on rows that are identical in the
file; we always give 29.400.** The step between them is 0.120 pt, which is
exactly one pixel at 600 dpi — the printer reference device this container
resolves (see `CLAUDE.md`, and `LineSpacing.cs`'s `RefDevMode`).

**CORRECTION, made before anything was built on the first reading.** The
paragraph that stood here said LibreOffice snaps row *boundaries* while we snap
each row *height*, and that the rounding order was the defect. Half of that is
right and the half that matters is not.

The reference does snap boundaries — every one is a whole number of 600 dpi
pixels (58.080/0.12 = 484, 87.360/0.12 = 728, 706.560/0.12 = 5888). But
**rounding order alone oscillates; it cannot accumulate monotonically.** Ours
grows in one direction to +1.080 pt over 24 rows, and that is only possible if
the height itself differs.

Averaging the reference's sixteen uniform body rows gives its exact height:

    reference mean pitch   29.3475 pt   (alternating 29.280 and 29.400)
    our pitch              29.4000 pt
    per-row excess         +0.0525 pt   ->  +1.26 pt over 24 rows

29.3475 pt is **244.56** pixels at 600 dpi, which is exactly why the reference
alternates between 244 and 245 while ours is 245 every time.

So **our auto row height is about 0.05 pt too tall**, and the quantisation makes
that visible rather than causing it. These rows carry `w:spacing w:line="233"
w:lineRule="auto"` — proportional spacing at 97.08%, *below* 100 — so the seat is
a line height under sub-unity proportional spacing, adjacent to the
base-membership rule fixed the same day but not the same bug. Only 12 of the
document's 305 rows declare a `w:trHeight`; the rest are auto, so the line height
is the whole of it.

Two things make this worth chasing rather than filing as cosmetic:

- **The table total is right.** Both sides end at 706.560 — the last row
  absorbs the whole accumulated error. So nothing about the table's own extent
  is wrong, and no measurement of a *complete* table would ever show it. It is
  visible only inside the table, which is why it has survived.
- **It decides pagination.** A table cut across a page boundary is cut at a row
  boundary, and a boundary that sits 1 pt low is a row that does not fit. That
  is exactly the shape of the remaining `mcar` failures — `02_mcar` 314/312 and
  `SPA-02` 268/266, both pages-only with words in band, both diverging first
  inside a long table.

Distinct from the advance-width divergence in `CLAUDE.md`'s third absolute
rule. That one is horizontal, per-glyph and face-dependent, and needs a hinted
advance; this is vertical, per-row, and is a **line height that is 0.05 pt too
large under sub-unity proportional spacing** — a value, not a rounding rule.
Both are small per unit and both reach whole pages by accumulating, which is
what makes them look alike from the page count and is why the direction of the
accumulation is the thing to check first.

Usage
-----

    XRAY_OFFICE_CLI=... python3 row-drift.py <document.docx> <our.pdf> <page>

Reads full-width horizontal rules out of a 600 dpi rasterisation of one page —
600 because the step being measured is one pixel at that resolution, and a
coarser raster cannot see it. An earlier pass at 150 dpi showed the same rows
as `61, 61, 62, 61` and looked like noise.
"""

import glob
import os
import subprocess
import sys

DPI = 600


def rules(pdf: str, page: int, tag: str) -> list[float]:
    """The y of every full-width horizontal rule on a page, in points."""
    for stale in glob.glob(f'/tmp/{tag}-*.pgm'):
        os.remove(stale)

    subprocess.run(
        ['pdftoppm', '-r', str(DPI), '-f', str(page), '-l', str(page), '-gray', pdf, f'/tmp/{tag}'],
        check=True)

    rendered = sorted(glob.glob(f'/tmp/{tag}-*.pgm'))
    if not rendered:
        raise SystemExit(f'no raster produced for {pdf} page {page}')

    data = open(rendered[0], 'rb').read()
    at, header = 0, []
    while len(header) < 4:
        end = data.index(b'\n', at)
        line = data[at:end]
        at = end + 1
        if not line.startswith(b'#'):
            header += line.split()

    width, height = int(header[1]), int(header[2])
    pixels = data[at:]

    dark = [y for y in range(height)
            if sum(1 for v in pixels[y * width:(y + 1) * width] if v < 128) > width * 0.45]

    runs: list[list[int]] = []
    for y in dark:
        if runs and y - runs[-1][-1] <= 3:
            runs[-1].append(y)
        else:
            runs.append([y])

    return [run[0] * 72.0 / DPI for run in runs]


def main() -> int:
    if len(sys.argv) < 4:
        raise SystemExit(__doc__)

    document, ours, page = sys.argv[1], sys.argv[2], int(sys.argv[3])
    stem = os.path.splitext(os.path.basename(document))[0]
    extension = os.path.splitext(document)[1].lstrip('.').lower()
    reference = (f'/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/'
                 f'{stem}__{extension}.pdf')

    for path in (reference, ours):
        if not os.path.isfile(path):
            raise SystemExit(f'{path} does not exist — nothing to compare')

    r, o = rules(reference, page, 'rowref'), rules(ours, page, 'rowours')
    print(f'rules: ref {len(r)}, ours {len(o)}')
    if len(r) != len(o):
        print('  (different counts — the rows do not correspond, so the drift below is not '
              'row-for-row)')

    print(f"{'row':>4s} {'ref pitch':>10s} {'our pitch':>10s} {'delta':>8s} "
          f"{'cum ref':>9s} {'cum ours':>9s} {'cum Δ':>8s}")

    for i in range(min(len(r), len(o)) - 1):
        print(f'{i:4d} {r[i + 1] - r[i]:10.3f} {o[i + 1] - o[i]:10.3f} '
              f'{(o[i + 1] - o[i]) - (r[i + 1] - r[i]):+8.3f} '
              f'{r[i + 1] - r[0]:9.3f} {o[i + 1] - o[0]:9.3f} '
              f'{(o[i + 1] - o[0]) - (r[i + 1] - r[0]):+8.3f}')

    step = 72.0 / DPI
    print(f'\none device pixel at {DPI} dpi is {step:.3f} pt — the step the reference alternates by')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
