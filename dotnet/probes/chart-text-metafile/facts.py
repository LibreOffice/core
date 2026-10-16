#!/usr/bin/env python3
"""Reproduce the three facts `SlideChartFaceComparisonTests` was left carrying, and test a model.

The three, as briefed:

  1. the fixture's monospaced digit measures 6.004 pt as the design metric, ~6.009 in ours,
     ~6.010 under 24.2.7.2 and ~5.839 under 26.2.4.2;
  2. the reference's `TJ` adjustment on that face is 16 at *every* inter-glyph position;
  3. the chart's `Tm` origins move between the two binaries where a Writer document's do not.

And the model this round proposes for all of it: a chart's text is measured on the reference
`VirtualDevice` that `DrawModelWrapper` builds from `Application::GetDefaultDevice()`
(`chart2/source/view/main/DrawModelWrapper.cxx`:88-99), which is **96 dpi** headless
(`SvpSalGraphics::GetResolution`, `vcl/headless/svpgdi.cxx`:44). An `OutputDevice` selects a
font at a whole number of device pixels, so a height of H hundredths of a millimetre is laid
out at `round(H x 96 / 2540)` pixels and every advance in the run is scaled by

    round(H x 96 / 2540) / (H x 96 / 2540)

before it ever reaches the page. On top of that, 24.2.7.2 also snaps each glyph *position* to
a whole 96 dpi pixel; 26.2.4.2 does not (tdf#168002 / subpixel positioning), which is the
whole of the difference between "76 at one position in five" and "16 at every position".

Usage:  XRAY_OFFICE_CLI=... python3 facts.py <workdir>
"""

import os
import subprocess
import sys
from pdftext import Pdf

HERE = os.path.dirname(os.path.abspath(__file__))
CORPUS = os.path.join(HERE, '..', '..', 'tests', 'corpus', 'features')
MONO = '/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf'
TARGETS = {'24.2.7.2': '/usr/bin/soffice', '26.2.4.2': '/opt/libreoffice26.2/program/soffice'}
DECK = 'chart-face-theme-minor.pptx'
WRITER = 'tabbed.docx'


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


def design_advance() -> float:
    sys.path.insert(0, os.path.join(HERE, '..', 'advance-ppem'))
    from ttf import Face
    face = Face(MONO)
    return face.advance('0') / face.upem


def main() -> int:
    out = os.path.abspath(sys.argv[1])
    cli = os.environ.get('XRAY_OFFICE_CLI')
    os.makedirs(out, exist_ok=True)
    advance = design_advance()

    print('# facts. Measured ' + subprocess.run(
        ['date', '-u', '+%Y-%m-%d'], capture_output=True, text=True).stdout.strip())
    for label, binary in TARGETS.items():
        print(f'# {label}: {binary} -> ' + subprocess.run(
            [binary, '--version'], capture_output=True, text=True).stdout.strip())
    print(f'# ours: {cli}')
    print('# fonts: system /usr/share/fonts; the 26.2 tarball\'s Latin metric duplicates, its'
          ' Latin NotoSans/NotoSerif and opens___.ttf are moved aside')
    print(f'# face: {MONO}, digit advance {advance:.8f} em')

    deck = os.path.abspath(os.path.join(CORPUS, DECK))
    writer = os.path.abspath(os.path.join(CORPUS, WRITER))
    pdfs = {label: render(binary, deck, os.path.join(out, label))
            for label, binary in TARGETS.items()}
    if cli:
        pdfs['ours'] = ours(cli, deck, os.path.join(out, 'ours'))
    writers = {label: render(binary, writer, os.path.join(out, label + '-writer'))
               for label, binary in TARGETS.items()}

    # Fact 1 -- the digit advance the fidelity test measures, as the difference between the
    # pens of two right-aligned value labels. That is one digit and nothing else *if* the two
    # labels are aligned on the same edge with the same widths, which is the point at issue.
    print('\n## fact 1: the digit advance of the value axis labels')
    print('# pen("80") - pen("100"), the quantity SlideChartFaceComparisonTests asserts')
    print('stack\tsize_pt\tpen_100\tpen_80\tdigit_pt\tdesign_pt')
    for label, pdf in pdfs.items():
        runs = {r['text']: r for r in Pdf(pdf).runs()}
        if '100' not in runs or '80' not in runs:
            print(f'{label}\t\t\t\t\tNO LABELS')
            continue
        size = runs['100']['size']
        print(f'{label}\t{size:.3f}\t{runs["100"]["x"]:.3f}\t{runs["80"]["x"]:.3f}'
              f'\t{runs["80"]["x"] - runs["100"]["x"]:.3f}\t{advance * size:.3f}')

    # Fact 2 -- the TJ adjustment at every inter-glyph position, and what the drawn gaps are.
    print('\n## fact 2: every inter-glyph gap of every value label')
    print('stack\tlabel\tsize_pt\tadjustments\tgaps_pt\tgaps_px96')
    for label, pdf in pdfs.items():
        for run in Pdf(pdf).runs():
            if run['text'] in ('20', '40', '60', '80', '100', '120', '140', '160', '180'):
                px = [g * 96.0 / 72.0 for g in run['gaps']]
                print(f'{label}\t{run["text"]}\t{run["size"]:.3f}\t{run["adjust"]}'
                      f'\t{[round(g, 4) for g in run["gaps"]]}\t{[round(p, 3) for p in px]}')

    # Fact 3 -- the origins, chart against a Writer control, both binaries.
    print('\n## fact 3: the origins, and whether they move between the binaries')
    print('document\trun\t24.2.7.2\t26.2.4.2\tmoved_pt')
    chart = {label: {r['text']: r for r in Pdf(pdf).runs()}
             for label, pdf in pdfs.items() if label in TARGETS}
    for text in ('0', '20', '40', '60', '80', '100', '120', '140', '160', '180'):
        a, b = chart['24.2.7.2'].get(text), chart['26.2.4.2'].get(text)
        if a and b:
            print(f'{DECK}\t{text}\t{a["x"]:.3f}\t{b["x"]:.3f}\t{b["x"] - a["x"]:+.3f}')
    control = {label: Pdf(pdf).runs() for label, pdf in writers.items()}
    pairs = list(zip(control['24.2.7.2'], control['26.2.4.2']))
    for a, b in pairs[:12]:
        if a['text'] == b['text']:
            print(f'{WRITER}\t{a["text"]!r}\t{a["x"]:.3f}\t{b["x"]:.3f}\t{b["x"] - a["x"]:+.3f}')
    print(f'# {WRITER}: {len(pairs)} runs compared, '
          f'{sum(1 for a, b in pairs if abs(a["x"] - b["x"]) > 0.001)} moved by more than 0.001 pt')

    # The model.
    print('\n## the model: a 96 dpi reference device, and a whole-pixel font height')
    print('stack\tsize_pt\theight_100thmm\tpx96\trounded\tpredicted_ratio\tmeasured_ratio')
    for label, pdf in pdfs.items():
        runs = [r for r in Pdf(pdf).runs() if r['text'] == '180' and r['gaps']]
        if not runs:
            continue
        run = runs[0]
        height = round(run['size'] * 2540.0 / 72.0)
        px96 = height * 96.0 / 2540.0
        mean = sum(run['gaps']) / len(run['gaps'])
        print(f'{label}\t{run["size"]:.3f}\t{height}\t{px96:.3f}\t{round(px96)}'
              f'\t{round(px96) / px96:.5f}\t{mean / (advance * run["size"]):.5f}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
