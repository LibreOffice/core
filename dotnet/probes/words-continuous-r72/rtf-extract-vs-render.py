#!/usr/bin/env python3
"""Separate a reading gap from a drawing gap on the `.rtf` residual.

    rtf-extract-vs-render.py <list> <ours-pdf-dir> <rows.tsv> [corpus-root]

For each document: the alphanumeric characters `xray-office extract` reads, the ones our own
rendering puts in its PDF's text layer, and the ones the reference's does. Extraction close to
the reference with the rendering far below it is a *drawing* gap and not a reading one — the
distinction round 71 could not make from the gate columns alone.
"""
import re, subprocess, sys
from pathlib import Path

LIST, OURS, ROWS = (Path(a) for a in sys.argv[1:4])
ROOT = Path(sys.argv[4] if len(sys.argv) > 4 else '/home/user/corpus-odf')
CLI = Path('/home/user/wt-words69/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/'
           'XRay.Office.Cli')


def alnum(text):
    return sum(1 for c in text if c.isalnum())


def main():
    rows = {}
    for line in ROWS.read_text().split('\n'):
        if not line or line.startswith('#') or line.startswith('path\t'):
            continue
        f = line.split('\t')
        rows[f[0]] = f

    print('extract\trender\tref\tclass\tpath')
    for rel in [p.strip() for p in LIST.read_text().split('\n') if p.strip()]:
        doc = ROOT / rel
        out = subprocess.run([str(CLI), 'extract', str(doc)],
                             capture_output=True, text=True, timeout=300)
        e = alnum(out.stdout)
        pdf = OURS / (doc.stem + '__rtf.pdf')
        r = alnum(subprocess.run(['pdftotext', str(pdf), '-'],
                                 capture_output=True, text=True).stdout) if pdf.exists() else 0
        ref = int(rows[rel][8].split('/')[1]) if rel in rows else 0

        if ref == 0:
            klass = 'no-reference'
        elif r < ref * 0.95 and e >= ref * 0.95:
            klass = 'drawn-short-read-whole'
        elif e < ref * 0.95:
            klass = 'read-short'
        elif r > ref * 1.05:
            klass = 'drawn-long'
        else:
            klass = 'text-agrees'
        print(f'{e}\t{r}\t{ref}\t{klass}\t{rel}')


if __name__ == '__main__':
    main()
