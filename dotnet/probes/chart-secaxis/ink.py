#!/usr/bin/env python3
"""Mean and worst-page ink against 26.2.4.2, for the documents this round can reach.

Ink is the mean absolute grey difference at 30 dpi, page for page, over the shared pages —
the same measure as probes/words-apo-table/inkcheck.py and probes/chart-layout/ink.py. The
reference render is cached so the same documents can be scored twice.

    ink.py <affected.txt> <out-dir> <tag>
"""
import subprocess, sys, os
from pathlib import Path

CORPUS = Path('/home/user/sample-files')
SOFFICE = '/opt/libreoffice26.2/program/soffice'
CLI = os.environ.get('XRAY_OFFICE_CLI',
                     '/home/user/wt-secaxis/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/'
                     'linux-x64/XRay.Office.Cli')

def render_ref(src, into):
    out = into / (src.stem + '.pdf')
    if out.exists():
        return out
    into.mkdir(parents=True, exist_ok=True)
    subprocess.run([SOFFICE, '--headless', '--norestore',
                    '-env:UserInstallation=file:///home/user/tmp-secaxis/lo-ink',
                    '--convert-to', 'pdf', '--outdir', str(into), str(src)],
                   capture_output=True, timeout=900)
    return out if out.exists() else None

def render_ours(src, into):
    into.mkdir(parents=True, exist_ok=True)
    out = into / (src.stem + '.pdf')
    if out.exists():
        return out
    subprocess.run([CLI, 'render', str(src), '--format', 'pdf', '--outdir', str(into)],
                   capture_output=True, timeout=900)
    return out if out.exists() else None

def raster(pdf, into):
    into.mkdir(parents=True, exist_ok=True)
    subprocess.run(['pdftoppm', '-r', '30', '-gray', '-png', str(pdf), str(into / 'p')],
                   capture_output=True, timeout=900)
    return sorted(into.glob('p*.png'))

def ink(a, b):
    from PIL import Image
    import numpy as np
    ia = np.asarray(Image.open(a).convert('L'), dtype=float)
    ib = np.asarray(Image.open(b).convert('L'), dtype=float)
    h = min(ia.shape[0], ib.shape[0]); w = min(ia.shape[1], ib.shape[1])
    return float(np.abs(ia[:h, :w] - ib[:h, :w]).mean()) * 100.0 / 255.0

def main():
    listing, out, tag = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3]
    print('document\tpages\tmean\tworst')
    # splitlines, not split: two corpus paths hold a space and a whitespace split
    # turned each into five documents that do not exist.
    for rel in (l.strip() for l in listing.read_text().splitlines() if l.strip()):
        src = CORPUS / rel
        if not src.exists():
            print(f'{rel}\tMISSING'); continue
        ref = render_ref(src, out / 'ref')
        mine = render_ours(src, out / tag)
        if ref is None or mine is None:
            print(f'{rel}\tNO-RENDER'); continue
        ra = raster(ref, out / 'ras-ref' / src.stem)
        rb = raster(mine, out / f'ras-{tag}' / src.stem)
        n = min(len(ra), len(rb))
        if n == 0:
            print(f'{rel}\tNO-PAGES'); continue
        values = [ink(ra[i], rb[i]) for i in range(n)]
        print(f'{rel}\t{n}\t{sum(values)/n:.2f}\t{max(values):.2f}')
        sys.stdout.flush()

main()
