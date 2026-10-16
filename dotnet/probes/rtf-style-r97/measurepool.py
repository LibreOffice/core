#!/usr/bin/env python3
"""Render one probe directory both sides and read HEAD's size, face and the gaps either side.

`probes/rtf-bookmark-r88/readpool.py` reads one side; this pairs them, because the whole point of
a pool probe is *agreement* and a single column cannot show it. The reference half is rendered in
one `soffice` invocation over the whole directory -- ~2 s a document rather than the ~8 s a fresh
`-env:UserInstallation` per document costs, which is what makes 116 probes affordable.

  XRAY_OFFICE_CLI=... REF_SOFFICE=... measurepool.py <probe dir> <work dir>
"""
import os
import pathlib
import subprocess
import sys

import pymupdf

FX = pathlib.Path(sys.argv[1])
WORK = pathlib.Path(sys.argv[2])
CLI = os.environ['XRAY_OFFICE_CLI']
REF = os.environ.get('REF_SOFFICE', '/opt/libreoffice26.2/program/soffice')

(WORK / 'ref').mkdir(parents=True, exist_ok=True)
(WORK / 'ours').mkdir(parents=True, exist_ok=True)
probes = sorted(FX.glob('p_*.rtf'))

if not list((WORK / 'ref').glob('*.pdf')):
    subprocess.run(['timeout', '-k', '30', '1800', REF, '--headless',
                    '-env:UserInstallation=file://' + str((WORK / 'prof').absolute()),
                    '--convert-to', 'pdf', '--outdir', str(WORK / 'ref')]
                   + [str(p) for p in probes], capture_output=True, check=False)
for p in probes:
    out = WORK / 'ours' / (p.stem + '.pdf')
    if out.exists():
        continue
    subprocess.run(['timeout', '-k', '30', '240', CLI, 'render', str(p),
                    '--format', 'pdf', '--outdir', str(WORK / 'ours')], capture_output=True)


def read(path):
    if not path.exists():
        return None
    spans = {}
    for block in pymupdf.open(path)[0].get_text('dict')['blocks']:
        for line in block.get('lines', []):
            for span in line['spans']:
                text = span['text'].strip()
                if text:
                    spans.setdefault(text.split()[0], span)
    if 'HEAD' not in spans:
        return None
    head = spans['HEAD']
    row = {
        'size': round(head['size'], 2),
        'italic': 'talic' in head['font'] or 'Oblique' in head['font'],
        'bold': 'Bold' in head['font'],
    }
    for key, other, sign in (('above', 'AAA', 1), ('below', 'BBB', -1)):
        if other in spans:
            row[key] = round(sign * (head['bbox'][3] - spans[other]['bbox'][3]), 2)
    return row


def cell(row):
    if row is None:
        return '-'
    flags = ('i' if row['italic'] else '.') + ('b' if row['bold'] else '.')
    return (f"{row['size']:>6} {flags} above={row.get('above', '-'):<7} "
            f"below={row.get('below', '-'):<7}")


width = max(len(p.stem) for p in probes)
agree = differ = 0
for p in probes:
    ref = read(WORK / 'ref' / (p.stem + '.pdf'))
    ours = read(WORK / 'ours' / (p.stem + '.pdf'))
    same = ref == ours
    agree += same
    differ += not same
    print(f"{p.stem:<{width}}  {cell(ref)} | {cell(ours)}  {'' if same else '  <-- differs'}")
print(f'\n{agree} agree, {differ} differ, of {len(probes)}')
