#!/usr/bin/env python3
"""Where the red band of each `make-capture.py` fixture is actually drawn, on both sides.

The band is the only red ink on the page, so its rectangle separates from the body text and the
running heads with no assumption about either — the instrument note in
`probes/frame-area-r85/results.md` §6, *a raster band is a better instrument than
`pdftotext -bbox` for a frame's position, because the band has no font*.

    measure-capture.py <fixture dir> <work dir> [dpi]

Renders each fixture through `$REF_SOFFICE` (falling back to `PATH`) and through
`$XRAY_OFFICE_CLI`, prints the binary's own version first, and writes one row per fixture:

    fixture   ref x0 y0   ours x0 y0   verdict

`verdict` is `agree` within the raster quantum (72/dpi points) and the signed gap otherwise.
"""
import os
import pathlib
import subprocess
import sys
import tempfile

import numpy as np
from PIL import Image

REF = os.environ.get('REF_SOFFICE', 'soffice')
CLI = os.environ.get('XRAY_OFFICE_CLI')


def version():
    out = subprocess.run([REF, '--version'], capture_output=True, text=True, timeout=120)
    return out.stdout.strip() or out.stderr.strip()


def render_all_ref(fixtures, outdir):
    """Convert every fixture in one `soffice` invocation, with one profile made once.

    A fresh `-env:UserInstallation` per document costs more than the conversion and, under
    contention, more than the 240 s bound — the first cut of this probe timed out on document one
    and reported *no band* for it, which reads exactly like the reference declining to draw the
    object. One process, one profile, every file on the command line.
    """
    outdir = outdir.resolve()
    outdir.mkdir(parents=True, exist_ok=True)

    # Absolute, because `-env:UserInstallation=file://relative/path` is a URL whose *host* is the
    # first segment: soffice silently falls back to the shared profile, and where another round
    # holds that it sleeps for ever with no output and no error. The first cut of this probe sat
    # at 0 % CPU for twelve minutes on that, which reads exactly like a slow conversion.
    profile = (outdir.parent / 'prof').resolve()
    for start in range(0, len(fixtures), 25):
        subprocess.run(
            ['timeout', '-k', '30', '900', REF, '--headless',
             f'-env:UserInstallation=file://{profile}',
             '--convert-to', 'pdf', '--outdir', str(outdir)]
            + [str(f.resolve()) for f in fixtures[start:start + 25]],
            capture_output=True, timeout=950)


def render_all_ours(fixtures, outdir):
    outdir.mkdir(parents=True, exist_ok=True)
    for src in fixtures:
        subprocess.run([CLI, 'render', str(src), '--outdir', str(outdir)],
                       capture_output=True, timeout=300)


def band(pdf, dpi):
    """The red band's top-left corner in points, or None when the page carries no red."""
    if not pdf.exists() or pdf.stat().st_size == 0:
        return None
    with tempfile.TemporaryDirectory() as t:
        stem = pathlib.Path(t) / 'p'
        subprocess.run(['pdftoppm', '-r', str(dpi), '-png', '-f', '1', '-l', '1', '-singlefile',
                        str(pdf), str(stem)], capture_output=True)
        f = pathlib.Path(str(stem) + '.png')
        if not f.exists():
            return None
        a = np.asarray(Image.open(f).convert('RGB'), dtype=np.int16)
    red = (a[:, :, 0] > 150) & (a[:, :, 1] < 110) & (a[:, :, 2] < 110)
    if not red.any():
        return None
    rows = np.where(red.any(axis=1))[0]
    cols = np.where(red.any(axis=0))[0]
    s = 72.0 / dpi
    return (cols[0] * s, rows[0] * s)


def main():
    fixtures = pathlib.Path(sys.argv[1])
    work = pathlib.Path(sys.argv[2])
    dpi = int(sys.argv[3]) if len(sys.argv) > 3 else 288
    work.mkdir(parents=True, exist_ok=True)
    quantum = 72.0 / dpi + 0.01

    fixture_list = sorted(fixtures.glob('*.docx'))
    rdir, odir = work / 'ref', work / 'ours'

    # `REUSE_REF=1` scores a second binary against a reference half already rendered here. The
    # fixtures cannot have changed under it -- `make-capture.py` writes them deterministically --
    # and the rulebook's rule applies: reuse the reference rather than re-rendering it whenever the
    # diff under test is confined to `dotnet/src`, which cannot reach `soffice`.
    if os.environ.get('REUSE_REF') != '1':
        render_all_ref(fixture_list, rdir)
    render_all_ours(fixture_list, odir)

    print(f'reference {REF} -- {version()}')
    print(f'ours      {CLI}')
    print(f'{"fixture":<32}{"ref x":>9}{"ref y":>9}{"our x":>9}{"our y":>9}  verdict')

    agree = total = 0
    for src in fixture_list:
        r = band(rdir / (src.stem + '.pdf'), dpi)
        o = band(odir / (src.stem + '.pdf'), dpi)
        total += 1
        if r is None or o is None:
            print(f'{src.stem:<32}{"-" if r is None else "ok":>9}{"":>9}'
                  f'{"-" if o is None else "ok":>9}{"":>9}  NO BAND')
            continue
        dx, dy = o[0] - r[0], o[1] - r[1]
        ok = abs(dx) <= quantum and abs(dy) <= quantum
        agree += ok
        print(f'{src.stem:<32}{r[0]:9.2f}{r[1]:9.2f}{o[0]:9.2f}{o[1]:9.2f}  '
              f'{"agree" if ok else f"dx {dx:+.2f} dy {dy:+.2f}"}')

    print(f'TOTAL agree {agree} of {total}')


main()
