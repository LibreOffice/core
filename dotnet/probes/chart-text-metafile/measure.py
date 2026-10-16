#!/usr/bin/env python3
"""Measure the 96 dpi round trip on real corpus documents, without needing the font file.

For one run of a LibreOffice PDF the drawn gap at position i is
`(declared_width[i] - adjustment[i]) x size / 1000` and the face's own advance there is
`declared_width[i] x size / 1000` (the declared width is `floor(hmtx x 1000 / upem)`, so it is
the design advance to a thousandth of an em). The ratio the run was drawn at is therefore

    1 - sum(adjustment) / sum(declared_width over the same positions)

with no font file and no assumption about which face was resolved. `reach.py` predicts that
ratio from the declared text size alone as `round(px96) / px96`; this checks the prediction on
whole documents.

Usage:  python3 measure.py <workdir> <document> [document...]
"""

import os
import subprocess
import sys
import re
from pdftext import Pdf

NUMERIC = re.compile(r'^[0-9][0-9 .,%$/:-]*[0-9%]$')
TARGETS = {'24.2.7.2': '/usr/bin/soffice', '26.2.4.2': '/opt/libreoffice26.2/program/soffice'}


def render(binary: str, document: str, directory: str) -> str:
    os.makedirs(directory, exist_ok=True)
    subprocess.run([binary, f'-env:UserInstallation=file://{directory}/profile', '--headless',
                    '--convert-to', 'pdf', '--outdir', directory, document],
                   capture_output=True, check=True)
    pdf = os.path.join(directory, os.path.splitext(os.path.basename(document))[0] + '.pdf')
    if not os.path.isfile(pdf):
        raise SystemExit(f'{binary} wrote no PDF for {document} -- nothing to compare')
    return pdf


def predicted(size_pt: float) -> float:
    height = round(size_pt * 2540.0 / 72.0)
    px = height * 96.0 / 2540.0
    return round(px) / px if px >= 1 else 1.0


def main() -> int:
    out = os.path.abspath(sys.argv[1])
    os.makedirs(out, exist_ok=True)
    print('# measure. ' + subprocess.run(['date', '-u', '+%Y-%m-%d'],
                                         capture_output=True, text=True).stdout.strip())
    for label, binary in TARGETS.items():
        print(f'# {label}: {binary} -> ' + subprocess.run(
            [binary, '--version'], capture_output=True, text=True).stdout.strip())
    print('# fonts: system /usr/share/fonts; the 26.2 tarball\'s Latin metric duplicates, its'
          ' Latin NotoSans/NotoSerif and opens___.ttf are moved aside')
    print('# drawn = 1 - sum(TJ adjustment)/sum(declared /Widths); predicted = round(px96)/px96')
    print('# only runs whose text is entirely digits and separators are reported: an axis'
          ' label carries no kerning pair in any of these faces, so every TJ adjustment in one'
          ' is the round trip and not the font\'s own kerning')
    print('document\tstack\tsize_pt\ttext\tgaps\tdrawn_ratio\tpredicted_ratio\tdelta')

    for document in sys.argv[2:]:
        name = os.path.basename(document)
        for label, binary in TARGETS.items():
            pdf = render(binary, document, os.path.join(out, name, label))
            for run in Pdf(pdf).runs():
                if len(run['adjust']) < 1 or not NUMERIC.match(run['text']):
                    continue
                widths = sum(run['widths'][:len(run['adjust'])])
                if widths <= 0:
                    continue
                drawn = 1.0 - sum(run['adjust']) / widths
                print(f'{name}\t{label}\t{run["size"]:.3f}\t{run["text"]!r}'
                      f'\t{len(run["adjust"])}\t{drawn:.5f}\t{predicted(run["size"]):.5f}'
                      f'\t{drawn - predicted(run["size"]):+.5f}')
            sys.stdout.flush()
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
