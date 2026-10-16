#!/usr/bin/env python3
"""The control: the same string, face and size in a chart and in a plain slide text box.

`chart-gap.py` shows a chart's inter-glyph advance missing the face's design advance by up to
2.9%, in a pattern that follows `round(height_in_100thmm x 96 / 2540)` -- the font height
quantised to a whole pixel of a 96 dpi device. This script puts the identical string in an
ordinary `p:sp` text box on the same slide of the same deck, so the two are drawn by one
binary from one document into one PDF and the only difference is which of the two paths laid
the text out.

Usage:  XRAY_OFFICE_CLI=... python3 chart-vs-textbox.py <workdir>
"""

import os
import re
import subprocess
import sys
import zipfile
from pdftext import Pdf

HERE = os.path.dirname(os.path.abspath(__file__))
DECK = os.path.join(HERE, '..', '..', 'tests', 'corpus', 'features',
                    'chart-face-theme-minor.pptx')
MONO = '/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf'
TARGETS = {'24.2.7.2': '/usr/bin/soffice', '26.2.4.2': '/opt/libreoffice26.2/program/soffice'}

CHART_TITLE = 'oooooooooooooooo'
BOX_TEXT = 'nnnnnnnnnnnnnnnn'

BOX = (
    '<p:sp><p:nvSpPr><p:cNvPr id="9" name="control"/><p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>'
    '<p:spPr><a:xfrm><a:off x="540000" y="5040000"/><a:ext cx="7920000" cy="540000"/></a:xfrm>'
    '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
    '<p:txBody><a:bodyPr wrap="none"><a:noAutofit/></a:bodyPr><a:lstStyle/>'
    '<a:p><a:pPr><a:defRPr/></a:pPr>'
    '<a:r><a:rPr lang="en-US" sz="{sz}" spc="-1"><a:latin typeface="+mn-lt"/></a:rPr>'
    '<a:t>{text}</a:t></a:r></a:p></p:txBody></p:sp>'
)


def design_advance() -> float:
    sys.path.insert(0, os.path.join(HERE, '..', 'advance-ppem'))
    from ttf import Face
    face = Face(MONO)
    return face.advance('o') / face.upem


def variant(path: str, size: int) -> str:
    with zipfile.ZipFile(DECK) as source, zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as out:
        for item in source.infolist():
            data = source.read(item.filename)
            if item.filename == 'ppt/charts/chart1.xml':
                text = data.decode('utf-8')
                head, _, tail = text.partition('</c:title>')
                head = head.replace('>Regional revenue<', f'>{CHART_TITLE}<')
                head = re.sub(r'sz="\d+"', f'sz="{size}"', head)
                data = (head + '</c:title>' + tail).encode('utf-8')
            elif item.filename == 'ppt/slides/slide1.xml':
                box = BOX.format(sz=size, text=BOX_TEXT)
                data = data.decode('utf-8').replace('</p:spTree>', box + '</p:spTree>').encode()
            out.writestr(item, data)
    return path


def render(binary: str, document: str, directory: str) -> str:
    os.makedirs(directory, exist_ok=True)
    subprocess.run([binary, f'-env:UserInstallation=file://{directory}/profile', '--headless',
                    '--convert-to', 'pdf', '--outdir', directory, document],
                   capture_output=True, check=True)
    pdf = os.path.join(directory, os.path.splitext(os.path.basename(document))[0] + '.pdf')
    if not os.path.isfile(pdf):
        raise SystemExit(f'{binary} wrote no PDF for {document} -- nothing to compare')
    return pdf


def main() -> int:
    out = os.path.abspath(sys.argv[1])
    cli = os.environ.get('XRAY_OFFICE_CLI')
    os.makedirs(out, exist_ok=True)
    advance = design_advance()

    print('# chart-vs-textbox. Measured ' + subprocess.run(
        ['date', '-u', '+%Y-%m-%d'], capture_output=True, text=True).stdout.strip())
    for label, binary in TARGETS.items():
        print(f'# {label}: {binary} -> ' + subprocess.run(
            [binary, '--version'], capture_output=True, text=True).stdout.strip())
    print(f'# ours: {cli}')
    print('# fonts: system /usr/share/fonts; the 26.2 tarball\'s Latin metric duplicates, its'
          ' Latin NotoSans/NotoSerif and opens___.ttf are moved aside')
    print(f'# face: {MONO}, design advance {advance:.8f} em')
    print('# both strings are the theme minor face at the same sz, in one deck, one PDF each')
    print('# px96 is height_100thmm x 96 / 2540; predicted = round(px96) / px96')
    print('sz\tstack\twhere\tsize_pt\tgap_pt\tdesign_pt\tratio\tpx96\tpredicted\tadjustments')

    for size in (700, 800, 1000, 1300, 1800, 2000):
        work = os.path.join(out, f'sz-{size}')
        os.makedirs(work, exist_ok=True)
        deck = variant(os.path.join(work, f'sz-{size}.pptx'), size)
        pdfs = {label: render(binary, deck, os.path.join(work, label))
                for label, binary in TARGETS.items()}
        if cli:
            directory = os.path.join(work, 'ours')
            os.makedirs(directory, exist_ok=True)
            subprocess.run([cli, 'render', '--quiet', '--outdir', directory, deck], check=True)
            pdfs['ours'] = os.path.join(directory, f'sz-{size}.pdf')
            if not os.path.isfile(pdfs['ours']):
                raise SystemExit('our CLI wrote no PDF -- nothing to compare')
        for label, pdf in pdfs.items():
            runs = {r['text']: r for r in Pdf(pdf).runs()}
            for where, text in (('chart', CHART_TITLE), ('textbox', BOX_TEXT)):
                run = runs.get(text)
                if run is None or not run['gaps']:
                    print(f'{size}\t{label}\t{where}\t\t\t\t\t\t\tNO RUN')
                    continue
                mean = sum(run['gaps']) / len(run['gaps'])
                design = advance * run['size']
                height = round(run['size'] * 2540.0 / 72.0)
                px96 = height * 96.0 / 2540.0
                print(f'{size}\t{label}\t{where}\t{run["size"]:.3f}\t{mean:.4f}\t{design:.4f}'
                      f'\t{mean / design:.5f}\t{px96:.3f}\t{round(px96) / px96:.5f}'
                      f'\t{sorted(set(run["adjust"]))}')
        sys.stdout.flush()
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
