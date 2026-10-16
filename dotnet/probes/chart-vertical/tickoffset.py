#!/usr/bin/env python3
"""How far a value-axis label's baseline sits from its own tick, over a size series.

The pitch probe (`pitch.py`) measures the line *height*. This one measures the *ascent*, which
is the other half of the vertical rule and the half that decides where a label sits relative to
the mark it belongs to. A value-axis label is centred on its tick, so

    tick_y - baseline_y = ascent - height/2

and both terms on the right are predicted with no free parameter once the device is known:
`hpx = round(size x 96/72)`, `ascent = round(asc/upem x hpx)` device pixels, `height` the
taller of the two roundings of ascent-plus-descent — or, on the competing reading, the face's
metrics scaled exactly with no device at all.

**The tick is read from the PDF's own path operators**, not inferred from the plot area: the
axis draws each major tick as one `m`/`l`/`S` segment and the content stream carries no `cm`,
so the tick's `y` is stated in the same user space as the label's `Tm`. That is what makes the
constant term vanish instead of having to be fitted.

Usage:  python3 tickoffset.py <workdir>
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

FACES = {
    'Liberation Sans': '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf',
    'Liberation Serif': '/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf',
    'Carlito': '/usr/share/fonts/truetype/crosextra/Carlito-Regular.ttf',
}

SIZES = [7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24]

SEGMENT = re.compile(r'([-\d.]+) ([-\d.]+) m\s+([-\d.]+) ([-\d.]+) l\s+S')
VALUE_AXIS = re.compile(r'(<c:valAx>.*?</c:valAx>)', re.S)
EVERY_AXIS = re.compile(r'(<c:(?:cat|val|date|ser)Ax>.*?</c:(?:cat|val|date|ser)Ax>)', re.S)


def value_axis_text(part: str, face: str, hundredths: int, every_axis: bool = False) -> str:
    """The chart part with the value axis' *label* text rewritten.

    `every_axis` rewrites the category axis' as well, and exists for one reason: our own reader
    carries **one** label size for the whole chart, taken from the first axis that states one
    (`DrawingChartPlot.AxisLabelSizeOf`), which is a deliberate simplification of the model and
    not a defect — so a variant that states a size on `c:valAx` alone leaves our renderer drawing
    the category axis' 10 pt. The quantity this probe reads is unaffected either way, because the
    tick is taken from the path operators rather than from the plot area: moving the category
    labels moves where the plot sits and not how far a value label is from its own tick.
    """
    def rewrite(match: re.Match) -> str:
        axis = match.group(1)
        head, marker, tail = axis.partition('<c:txPr>')
        if not marker:
            raise SystemExit('the value axis states no <c:txPr> — nothing to rewrite')
        tail = re.sub(r'sz="\d+"', f'sz="{hundredths}"', tail, count=1)
        if '<a:latin' in tail:
            tail = re.sub(r'<a:latin typeface="[^"]*"/>', f'<a:latin typeface="{face}"/>',
                          tail, count=1)
        else:
            tail = tail.replace('</a:defRPr>', f'<a:latin typeface="{face}"/></a:defRPr>', 1)
        return head + marker + tail

    pattern = EVERY_AXIS if every_axis else VALUE_AXIS
    rewritten, count = pattern.subn(rewrite, part)
    if count != (2 if every_axis else 1):
        raise SystemExit(f'rewrote {count} axes, which is not what this fixture holds')
    return rewritten


def variant(source: str, part: str, out: str, face: str, hundredths: int,
            every_axis: bool = False) -> str:
    with zipfile.ZipFile(source) as zin, zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == part:
                data = value_axis_text(
                    data.decode('utf-8'), face, hundredths, every_axis).encode('utf-8')
            zout.writestr(item, data)
    return out


def render(binary: str, document: str, directory: str) -> str:
    os.makedirs(directory, exist_ok=True)
    cached = os.path.join(directory, os.path.splitext(os.path.basename(document))[0] + '.pdf')
    if os.path.isfile(cached):
        return cached
    subprocess.run(
        [binary, f'-env:UserInstallation=file://{directory}/profile', '--headless',
         '--convert-to', 'pdf', '--outdir', directory, document],
        capture_output=True, check=True, timeout=600)
    pdf = os.path.join(directory, os.path.splitext(os.path.basename(document))[0] + '.pdf')
    if not os.path.isfile(pdf):
        raise SystemExit(f'{binary} wrote no PDF for {document} — nothing to compare')
    return pdf


def horizontal_ticks(pdf: Pdf) -> list[tuple[float, float]]:
    """The value axis' major ticks: short horizontal segments, as (x_right, y)."""
    found = []
    for x1, y1, x2, y2 in SEGMENT.findall(pdf.content.decode('latin1')):
        x1, y1, x2, y2 = float(x1), float(y1), float(x2), float(y2)
        if abs(y1 - y2) > 0.01 or not 0.5 < abs(x2 - x1) < 12.0:
            continue
        found.append((max(x1, x2), y1))
    return found


def offsets(pdf_path: str, size_pt: float, text_shift: float = 0.0) -> list[float]:
    """`tick_y - baseline_y` for every value-axis label, paired with its nearest tick.

    `text_shift` is added to each label's stated baseline before anything is paired. The
    reference binaries need none — they emit no `cm`, so a `Td` *is* a page position — and our
    own writer wraps every glyph run in a translation, which has to be undone before the pairing
    and not after it: a label a whole tick spacing out of place pairs with the wrong tick.

    Nearest rather than "the only one within a window": at the larger sizes a label's own
    height exceeds the tick spacing, so a window wide enough to hold the offset catches the
    neighbouring tick as well. The pairing is checked rather than assumed — every label must
    take a distinct tick, and the run is discarded if two claim one.
    """
    pdf = Pdf(pdf_path)
    ticks = horizontal_ticks(pdf)
    if not ticks:
        return []

    axis_x = min(x for x, _ in ticks)
    labels = [(run['x'], run['y'] + text_shift) for run in pdf.runs()
              if run['text'].strip().isdigit()
              and abs(run['size'] - size_pt) <= max(0.35, size_pt * 0.04)
              and run['x'] < axis_x + 0.5]

    found = []
    taken = set()
    for _, baseline in labels:
        best = min(ticks, key=lambda tick: abs(tick[1] - baseline))
        if best[1] in taken:
            return []
        taken.add(best[1])
        found.append(best[1] - baseline)
    return found


def main() -> int:
    work = sys.argv[1] if len(sys.argv) > 1 else '/home/user/tmp-chartvert/chart-tick'
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
                    found = offsets(pdf, size)
                    if len(found) < 3:
                        missing.append(f'{stem} @ {label}: {len(found)} tick/label pairs')
                        continue
                    p = predicted(face, size)
                    e = exact(face, size)
                    rows.append({
                        'track': track, 'binary': label, 'face': family, 'size': size,
                        'pairs': len(found),
                        'offset': sum(found) / len(found),
                        'spread': max(found) - min(found),
                        'predicted': p['ascent'] - p['height'] / 2.0,
                        'exact': e['ascent'] - e['height'] / 2.0,
                    })

    if missing:
        print('REFUSING TO SUMMARISE — cases with no measurable geometry:', file=sys.stderr)
        for line in missing:
            print('   ', line, file=sys.stderr)
        return 2

    with open(os.path.join(HERE, 'tickoffset.tsv'), 'w', encoding='utf-8') as handle:
        handle.write('track\tbinary\tface\tsize\tpairs\toffset\tspread\tpredicted\texact\n')
        for row in rows:
            handle.write('{track}\t{binary}\t{face}\t{size}\t{pairs}\t{offset:.4f}\t'
                         '{spread:.4f}\t{predicted:.4f}\t{exact:.4f}\n'.format(**row))

    print(f'{"track":7s} {"binary":9s} {"face":17s} {"sz":>3s} {"n":>3s} {"offset":>8s} '
          f'{"pixel":>8s} {"err":>7s} {"exact":>8s} {"err":>7s} {"spread":>7s}')
    hits = exacts = 0
    for row in rows:
        dp = row['offset'] - row['predicted']
        de = row['offset'] - row['exact']
        if abs(dp) <= 0.02:
            hits += 1
        if abs(de) <= 0.02:
            exacts += 1
        print(f'{row["track"]:7s} {row["binary"]:9s} {row["face"]:17s} {row["size"]:3d} '
              f'{row["pairs"]:3d} {row["offset"]:8.3f} {row["predicted"]:8.3f} {dp:+7.3f} '
              f'{row["exact"]:8.3f} {de:+7.3f} {row["spread"]:7.3f}')

    print(f'\nwithin 0.02 pt: pixel law {hits} of {len(rows)}, exact scaling {exacts}')
    print(f'worst |offset - pixel| {max(abs(r["offset"] - r["predicted"]) for r in rows):.3f} pt; '
          f'worst |offset - exact| {max(abs(r["offset"] - r["exact"]) for r in rows):.3f} pt')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
