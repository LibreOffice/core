#!/usr/bin/env python3
"""What the vertical rule moves on the chart-bearing corpus subset.

`probes/chart-text-metafile/reach.tsv` is the census of the 168 corpus documents that carry a
chart. The vertical half of the chart device is already applied on the sheets track, so what
this round can move is the **78 slides and words** documents — the same set the horizontal half
moved one round ago.

For each it records what a page-count gate would see and what it would not:

  * how many pages or slides the rendering has,
  * how many glyph runs the chart's page carries and how many of them are turned,
  * the md5 of the whole file, with the PDF's `/CreationDate` masked.

The turned-run count is the axis decision. `ChartAxisLabels.Resolve` reaches for a 45 degree
rotation only after a collision it cannot thin or wrap away, and a taller line is exactly the
kind of thing that can tip that decision — which is a much bigger change than a fraction of a
point and invisible to every column of the gate.

Usage:  XRAY_OFFICE_CLI=... python3 corpus.py <corpus root> <outdir> <label> [--reference]

`--reference` renders through 26.2.4.2 instead, for the screen.
"""

import hashlib
import os
import re
import subprocess
import sys
import urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'chart-text-metafile'))
from pdftext import Pdf  # noqa: E402

REACH = os.path.join(HERE, '..', 'chart-text-metafile', 'reach.tsv')
REFERENCE = '/opt/libreoffice26.2/program/soffice'
CLI = os.environ.get('XRAY_OFFICE_CLI', os.path.join(
    HERE, '..', '..', 'tools', 'XRay.Office.Cli', 'bin', 'Debug', 'net10.0', 'linux-x64',
    'XRay.Office.Cli'))

DATE = re.compile(rb'/CreationDate\s*\([^)]*\)')
PAGE = re.compile(rb'/Type\s*/Page[^s]')
TURN = re.compile(r'(-?[\d.]+) (-?[\d.]+) (-?[\d.]+) (-?[\d.]+) (-?[\d.]+) (-?[\d.]+) (Tm|cm)')
SHOW = re.compile(r'\]\s*TJ|>\s*Tj')


def wanted(corpus: str) -> list[tuple[str, str]]:
    """The slides and words rows of the census, as (family, absolute path).

    The path is taken literally first and only then percent-decoded: one corpus document is
    named `Sylva%20introduction%20session.pptx` **on disk**, so decoding unconditionally loses
    it — and losing one row of a 78-row reach without saying so is exactly the shape of failure
    this file exists to avoid. It refuses rather than returning a short list.
    """
    rows = []
    named = 0
    for line in open(REACH, encoding='utf-8'):
        if line.startswith('#') or not line.strip():
            continue
        parts = line.rstrip('\n').split('\t')
        if parts[0] not in ('slides', 'words'):
            continue
        named += 1
        literal = os.path.join(corpus, parts[1])
        decoded = os.path.join(corpus, urllib.parse.unquote(parts[1]))
        path = literal if os.path.isfile(literal) else decoded
        if not os.path.isfile(path):
            raise SystemExit(f'{parts[1]} is in the census and not on disk under {corpus}')
        rows.append((parts[0], path))

    if len(rows) != named:
        raise SystemExit(f'{len(rows)} of {named} census rows resolved')
    return rows


def render(document: str, directory: str, reference: bool) -> str | None:
    os.makedirs(directory, exist_ok=True)
    stem = os.path.splitext(os.path.basename(document))[0]
    pdf = os.path.join(directory, stem + '.pdf')
    if os.path.isfile(pdf):
        return pdf
    command = ([REFERENCE, f'-env:UserInstallation=file://{directory}/profile', '--headless',
                '--convert-to', 'pdf', '--outdir', directory, document]
               if reference else
               [CLI, 'render', '--quiet', '--outdir', directory, document])
    try:
        subprocess.run(command, capture_output=True, timeout=900, check=False)
    except subprocess.TimeoutExpired:
        return None
    return pdf if os.path.isfile(pdf) else None


def summary(path: str) -> dict:
    """Page count, text-run count, turned-run count and a date-masked md5.

    The two counts are read out of the **decompressed** content streams. Both stacks deflate
    theirs, so counting `BT`/`TJ` in the file's bytes reports whatever the compressor happened to
    emit — a 30-slide deck came back as 61 text operators that way, which is not a number about
    the document at all.
    """
    data = open(path, 'rb').read()
    pdf = Pdf(path)
    content = pdf.content.decode('latin1')

    turned = sum(1 for m in TURN.finditer(content)
                 if abs(float(m.group(2))) > 0.01 or abs(float(m.group(3))) > 0.01)

    # The show operators are counted with a regex rather than through `pdftext.Pdf.runs`, whose
    # `TJ` reader trips over a bare `.` that one corpus deck's reference PDF really does contain.
    # Nothing here needs a run's text or its pen — only how many of them there are.
    return {
        'pages': len(PAGE.findall(data)),
        'shows': len(SHOW.findall(content)),
        'turned': turned,
        'md5': hashlib.md5(DATE.sub(b'/CreationDate()', data)).hexdigest(),
    }


def main() -> int:
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    corpus, out, label = sys.argv[1], sys.argv[2], sys.argv[3]
    reference = '--reference' in sys.argv

    rows = wanted(corpus)
    if not rows:
        raise SystemExit(f'{REACH} named no slides or words document under {corpus}')

    os.makedirs(out, exist_ok=True)
    missing = []
    results = []
    for family, path in rows:
        stem = os.path.splitext(os.path.basename(path))[0]
        pdf = render(path, os.path.join(out, label, stem), reference)
        if pdf is None:
            missing.append(path)
            continue
        results.append((family, path, summary(pdf)))

    written = os.path.join(HERE, f'corpus-{label}.tsv')
    with open(written, 'w', encoding='utf-8') as handle:
        handle.write('family\tpath\tpages\tshows\tturned\tmd5\n')
        for family, path, row in results:
            handle.write(f'{family}\t{os.path.basename(path)}\t{row["pages"]}\t{row["shows"]}\t'
                         f'{row["turned"]}\t{row["md5"]}\n')

    print(f'{label}: {len(results)} of {len(rows)} rendered, {len(missing)} produced no PDF')
    for path in missing:
        print('   no output:', os.path.basename(path))
    print('written', written)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
