#!/usr/bin/env python3
"""Baseline-to-baseline distance inside one chart label, on a deck and on a Writer document.

Round 60 established the vertical half of the chart device on a *workbook*: `chart2` measures a
chart's text on its own 96 dpi `VirtualDevice`, so a line's height is
`round(asc/upem x hpx) + round(desc/upem x hpx)` device pixels with `hpx = round(size x 96/72)`,
converted back through whole hundredths of a millimetre and with no external leading. This asks
whether the same rule holds when the chart is in a `.pptx` and in a `.docx`, on both installed
binaries.

The instrument is a chart *title* rewritten as three lines joined by `<a:br/>`, which `chart2`
draws as one text shape with three text objects; the PDF states each object's baseline outright
in its `Tm`, so the pitch is a subtraction of two numbers the file contains rather than anything
reconstructed. `pdftotext -bbox` is deliberately not used: its quantum is several times the
effect.

Usage:  python3 pitch.py <workdir>
"""

import os
import re
import subprocess
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'chart-text-metafile'))
from pdftext import Pdf  # noqa: E402
from facemetrics import Metrics, predicted, exact  # noqa: E402

FIXTURES = os.path.join(HERE, '..', '..', 'tests', 'corpus', 'features')
TARGETS = {'24.2.7.2': '/usr/bin/soffice', '26.2.4.2': '/opt/libreoffice26.2/program/soffice'}

# Three faces: Carlito's hhea line gap is zero and both Liberation faces' is not, so the gap
# term is separated by the faces rather than by argument.
FACES = {
    'Liberation Sans': '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf',
    'Liberation Serif': '/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf',
    'Carlito': '/usr/share/fonts/truetype/crosextra/Carlito-Regular.ttf',
}

SIZES = [7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24]

LINE = 'Mg'
LINES = 3

# Anchored on `<c:chart>` so it cannot pick up an axis title: both fixtures carry three
# `<c:title>` elements and only the first is the chart's own.
TITLE_RE = re.compile(
    r'(<c:chart><c:title><c:tx><c:rich><a:bodyPr[^>]*/><a:lstStyle/>)<a:p>.*?</a:p>', re.S)


def paragraph(face: str, hundredths: int) -> str:
    runs = []
    for i in range(LINES):
        if i:
            runs.append('<a:br/>')
        runs.append(
            f'<a:r><a:rPr b="0" sz="{hundredths}" spc="-1" strike="noStrike">'
            f'<a:solidFill><a:srgbClr val="000000"/></a:solidFill>'
            f'<a:latin typeface="{face}"/></a:rPr><a:t>{LINE}</a:t></a:r>')
    return (f'<a:p><a:pPr><a:defRPr b="0" sz="{hundredths}" spc="-1" strike="noStrike">'
            f'<a:latin typeface="{face}"/></a:defRPr></a:pPr>' + ''.join(runs) + '</a:p>')


def variant(source: str, part: str, out: str, face: str, hundredths: int) -> str:
    with zipfile.ZipFile(source) as zin, zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == part:
                text = data.decode('utf-8')
                replaced, count = TITLE_RE.subn(
                    lambda m: m.group(1) + paragraph(face, hundredths), text)
                if count != 1:
                    raise SystemExit(f'{source}: rewrote {count} titles, wanted exactly 1')
                data = replaced.encode('utf-8')
            zout.writestr(item, data)
    return out


def render(binary: str, document: str, directory: str) -> str:
    os.makedirs(directory, exist_ok=True)
    subprocess.run(
        [binary, f'-env:UserInstallation=file://{directory}/profile', '--headless',
         '--convert-to', 'pdf', '--outdir', directory, document],
        capture_output=True, check=True, timeout=600)
    pdf = os.path.join(directory, os.path.splitext(os.path.basename(document))[0] + '.pdf')
    if not os.path.isfile(pdf):
        raise SystemExit(f'{binary} wrote no PDF for {document} — nothing to compare')
    return pdf


def title_runs(pdf_path: str, size_pt: float) -> list[tuple[float, float, float]]:
    """The three title baselines: the runs whose text is the title's and whose size fits."""
    runs = []
    for run in Pdf(pdf_path).runs():
        if run['text'].strip() != LINE:
            continue
        if abs(run['size'] - size_pt) > max(0.35, size_pt * 0.04):
            continue
        runs.append((run['x'], run['y'], run['size']))
    runs.sort(key=lambda r: -r[1])
    return runs


def main() -> int:
    work = sys.argv[1] if len(sys.argv) > 1 else '/home/user/tmp-chartvert/chart-vertical'
    os.makedirs(work, exist_ok=True)

    documents = [
        ('slides', os.path.join(FIXTURES, 'chart-face-theme-minor.pptx'), 'ppt/charts/chart1.xml'),
        ('words', os.path.join(FIXTURES, 'chart-bar-text.docx'), 'word/charts/chart1.xml'),
    ]

    missing = []
    rows = []

    for track, source, part in documents:
        for family, path in FACES.items():
            face = Metrics(path)
            for size in SIZES:
                stem = f'{track}-{family.replace(" ", "")}-{size}'
                extension = os.path.splitext(source)[1]
                document = variant(source, part, os.path.join(work, stem + extension),
                                   family, size * 100)
                for label, binary in TARGETS.items():
                    directory = os.path.join(work, 'r-' + label.replace('.', '_'), stem)
                    pdf = render(binary, document, directory)
                    found = title_runs(pdf, size)
                    if len(found) != LINES:
                        missing.append(f'{stem} @ {label}: {len(found)} runs, wanted {LINES}')
                        continue
                    gaps = [found[i][1] - found[i + 1][1] for i in range(LINES - 1)]
                    rows.append({
                        'track': track, 'binary': label, 'face': family, 'size': size,
                        'drawn': found[0][2], 'pitch': sum(gaps) / len(gaps),
                        'spread': max(gaps) - min(gaps),
                        'predicted': predicted(face, size)['height'],
                        'exact': exact(face, size)['height'],
                    })

    if missing:
        print('REFUSING TO SUMMARISE — cases with no measurable geometry:', file=sys.stderr)
        for line in missing:
            print('   ', line, file=sys.stderr)
        return 2

    with open(os.path.join(HERE, 'pitch.tsv'), 'w', encoding='utf-8') as handle:
        handle.write('track\tbinary\tface\tsize\tdrawn\tpitch\tspread\tpredicted\texact\n')
        for row in rows:
            handle.write('{track}\t{binary}\t{face}\t{size}\t{drawn:.3f}\t{pitch:.4f}\t'
                         '{spread:.4f}\t{predicted:.4f}\t{exact:.4f}\n'.format(**row))

    print(f'{"track":7s} {"binary":9s} {"face":17s} {"sz":>3s} {"hpx":>4s} '
          f'{"pitch":>8s} {"pixel law":>10s} {"err":>7s} {"exact":>8s} {"err":>7s}')
    hits = 0
    for row in rows:
        face = Metrics(FACES[row['face']])
        hpx = predicted(face, row['size'])['hpx']
        dp = row['pitch'] - row['predicted']
        de = row['pitch'] - row['exact']
        if abs(dp) <= 0.05:
            hits += 1
        print(f'{row["track"]:7s} {row["binary"]:9s} {row["face"]:17s} {row["size"]:3d} '
              f'{hpx:4d} {row["pitch"]:8.3f} {row["predicted"]:10.3f} {dp:+7.3f} '
              f'{row["exact"]:8.3f} {de:+7.3f}')

    print(f'\npixel law within 0.05 pt on {hits} of {len(rows)} cases')
    worst_p = max(abs(r['pitch'] - r['predicted']) for r in rows)
    worst_e = max(abs(r['pitch'] - r['exact']) for r in rows)
    print(f'worst |pitch - pixel law| = {worst_p:.3f} pt;  '
          f'worst |pitch - exact scaling| = {worst_e:.3f} pt')
    print(f'worst within-label spread of the two gaps = '
          f'{max(r["spread"] for r in rows):.4f} pt')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
