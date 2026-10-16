#!/usr/bin/env python3
"""What advance does a chart's text actually get drawn with?

Builds variants of `tests/corpus/features/chart-face-theme-minor.pptx` -- whose theme minor
Latin face is Liberation Mono, so every glyph has the same design advance of 1229/2048 em --
changing exactly one thing per variant: the chart title's font size, or the graphic frame the
chart is drawn into. It then reads every inter-glyph gap of the title back out of the PDF each
stack writes (`pdftext.py`) and reports it against the face's own `hmtx`.

A monospaced face is what makes this readable: the design gap is one number for the whole
string, so any per-position structure in the output is the renderer's and not the font's.

Usage:  XRAY_OFFICE_CLI=... python3 chart-gap.py <workdir>
"""

import os
import re
import shutil
import subprocess
import sys
import zipfile
from pdftext import Pdf

HERE = os.path.dirname(os.path.abspath(__file__))
DECK = os.path.join(HERE, '..', '..', 'tests', 'corpus', 'features',
                    'chart-face-theme-minor.pptx')
MONO = '/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf'
TARGETS = {'24.2.7.2': '/usr/bin/soffice', '26.2.4.2': '/opt/libreoffice26.2/program/soffice'}

# Sixteen identical glyphs: fifteen gaps of one known design advance, and no kerning pair,
# no ligature and no space anywhere in it.
TITLE = 'oooooooooooooooo'


def design_advance() -> float:
    """Liberation Mono's advance for every glyph, in em, read from `hmtx`."""
    sys.path.insert(0, os.path.join(HERE, '..', 'advance-ppem'))
    from ttf import Face
    face = Face(MONO)
    return face.advance('o') / face.upem


def variant(path: str, *, title: str, size: int, spc: int | None,
            frame: tuple[int, int] | None) -> str:
    """The deck with the chart title's text and size replaced, and optionally the frame."""
    with zipfile.ZipFile(DECK) as source, zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as out:
        for item in source.infolist():
            data = source.read(item.filename)
            if item.filename == 'ppt/charts/chart1.xml':
                text = data.decode('utf-8')
                head, _, tail = text.partition('</c:title>')
                head = head.replace('>Regional revenue<', f'>{title}<')
                head = re.sub(r'sz="\d+"', f'sz="{size}"', head)
                if spc is not None:
                    head = re.sub(r'spc="-?\d+"', f'spc="{spc}"', head)
                data = (head + '</c:title>' + tail).encode('utf-8')
            elif item.filename == 'ppt/slides/slide1.xml' and frame is not None:
                data = re.sub(rb'<a:ext cx="7920000" cy="4320000"/>',
                              f'<a:ext cx="{frame[0]}" cy="{frame[1]}"/>'.encode(), data)
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


def ours(cli: str, document: str, directory: str) -> str:
    os.makedirs(directory, exist_ok=True)
    subprocess.run([cli, 'render', '--quiet', '--outdir', directory, document], check=True)
    pdf = os.path.join(directory, os.path.splitext(os.path.basename(document))[0] + '.pdf')
    if not os.path.isfile(pdf):
        raise SystemExit(f'{cli} wrote no PDF for {document} -- nothing to compare')
    return pdf


def title_run(pdf: str, text: str) -> dict | None:
    for run in Pdf(pdf).runs():
        if run['text'] == text:
            return run
    return None


def main() -> int:
    out = os.path.abspath(sys.argv[1])
    cli = os.environ.get('XRAY_OFFICE_CLI')
    os.makedirs(out, exist_ok=True)
    advance = design_advance()

    stacks = dict(TARGETS)
    print('# chart-gap. Measured ' + subprocess.run(
        ['date', '-u', '+%Y-%m-%d'], capture_output=True, text=True).stdout.strip()
        + ' on chart-face-theme-minor.pptx variants')
    for label, binary in TARGETS.items():
        print(f'# {label}: {binary} -> ' + subprocess.run(
            [binary, '--version'], capture_output=True, text=True).stdout.strip())
    print(f'# ours: {cli}')
    print('# fonts: system /usr/share/fonts; the 26.2 tarball\'s Latin metric duplicates, its'
          ' Latin NotoSans/NotoSerif and opens___.ttf are moved aside')
    print(f'# face: {MONO}, design advance {advance:.8f} em for every glyph')
    print('# gap_pt is the mean inter-glyph advance of the chart title, read from the PDF\'s'
          ' Tm/TJ; ratio is gap_pt / (advance x size_pt)')
    print('case\tsz\tframe_cx\tstack\tsize_pt\tglyphs\tgap_pt\tdesign_pt\tratio\tadjustments')

    cases: list[tuple[str, int, tuple[int, int] | None, int | None]] = []
    for size in (600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1600, 1800, 2000):
        cases.append((f'size-{size}', size, None, None))
    for cx in (3960000, 5940000, 7920000, 9900000):
        cases.append((f'frame-{cx}', 1000, (cx, 4320000), None))
    cases.append(('spc-0', 1000, None, 0))
    cases.append(('spc-100', 1000, None, 100))

    for name, size, frame, spc in cases:
        work = os.path.join(out, name)
        os.makedirs(work, exist_ok=True)
        deck = variant(os.path.join(work, f'{name}.pptx'),
                       title=TITLE, size=size, spc=spc, frame=frame)
        pdfs = {label: render(binary, deck, os.path.join(work, label))
                for label, binary in stacks.items()}
        if cli:
            pdfs['ours'] = ours(cli, deck, os.path.join(work, 'ours'))
        for label, pdf in pdfs.items():
            run = title_run(pdf, TITLE)
            if run is None:
                print(f'{name}\t{size}\t{frame[0] if frame else 7920000}\t{label}'
                      f'\t\t0\t\t\t\tNO RUN')
                continue
            gaps = run['gaps']
            mean = sum(gaps) / len(gaps)
            design = advance * run['size']
            print(f'{name}\t{size}\t{frame[0] if frame else 7920000}\t{label}'
                  f'\t{run["size"]:.3f}\t{len(gaps)}\t{mean:.4f}\t{design:.4f}'
                  f'\t{mean / design:.5f}\t{sorted(set(run["adjust"]))}')
        sys.stdout.flush()
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
