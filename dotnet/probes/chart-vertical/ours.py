#!/usr/bin/env python3
"""The same two vertical measurements, taken off our own renderer.

`pitch.py` and `tickoffset.py` read the reference binaries; this reads `XRay.Office.Cli` on the
identical variant documents, through the identical instrument, so a before/after run of it is
directly comparable with the reference columns those two produce.

Usage:  XRAY_OFFICE_CLI=... python3 ours.py <workdir> <label>

`<label>` names the run — `before` or `after` — and decides the output file, `ours-<label>.tsv`.
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
import pitch as pitch_probe  # noqa: E402
import tickoffset as tick_probe  # noqa: E402

CLI = os.environ.get('XRAY_OFFICE_CLI', os.path.join(
    HERE, '..', '..', 'tools', 'XRay.Office.Cli', 'bin', 'Debug', 'net10.0', 'linux-x64',
    'XRay.Office.Cli'))


TRANSLATION = re.compile(
    r'(-?[\d.]+) (-?[\d.]+) (-?[\d.]+) (-?[\d.]+) (-?[\d.]+) (-?[\d.]+) cm')


def text_shift(pdf: Pdf) -> float:
    """How far our own writer translates a text block's space below the page's.

    `pdftext` reads the `Td`/`Tm` a text object states and knows nothing about the CTM. The
    reference binaries emit no `cm` at all, so their stated origin *is* the page position; ours
    wraps every glyph run in `q 1 0 0 1 tx ty cm`, and a tick read straight off the path
    operators is then in a different space from the label beside it — a constant 42.52 pt on
    this deck. This recovers that constant, and refuses rather than guessing if the file holds
    more than one axis-aligned translation.
    """
    shifts = {(float(m.group(5)), float(m.group(6)))
              for m in TRANSLATION.finditer(pdf.content.decode('latin1'))
              if (float(m.group(1)), float(m.group(2)), float(m.group(3)), float(m.group(4)))
              == (1.0, 0.0, 0.0, 1.0)}
    if len(shifts) > 1:
        raise SystemExit(f'{len(shifts)} distinct text translations — the instrument assumes one')
    return next(iter(shifts))[1] if shifts else 0.0


def tick_offsets(pdf_path: str, size_pt: float) -> list[float]:
    """`tickoffset.offsets`, with our writer's own text translation folded back in."""
    return tick_probe.offsets(pdf_path, size_pt, text_shift(Pdf(pdf_path)))


LATIN = re.compile(r'<a:latin typeface="[^"]*"/>')


def one_face(document: str, part: str, out: str, face: str) -> str:
    """The same document with *every* stated Latin face in the chart part replaced.

    Our reader carries one face for a whole chart — `ChartFace` is a family, and
    `DrawingChartPlot.FamilyOf` searches the part — so a variant that names a face on the axes
    alone leaves us drawing the *title's* face in the labels. That is the same documented
    one-answer-per-chart simplification as the label size, and it makes a face comparison out of
    what is meant to be a metric one: at 7 pt the Carlito variant came back on Liberation Sans'
    ascent to the last thousandth. Naming the face everywhere removes the confound instead of
    absorbing it.
    """
    with zipfile.ZipFile(document) as zin, zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == part:
                data = LATIN.sub(f'<a:latin typeface="{face}"/>',
                                 data.decode('utf-8')).encode('utf-8')
            zout.writestr(item, data)
    return out


def ours(document: str, directory: str) -> str:
    os.makedirs(directory, exist_ok=True)
    cached = os.path.join(directory, os.path.splitext(os.path.basename(document))[0] + '.pdf')
    if os.path.isfile(cached):
        return cached
    subprocess.run([CLI, 'render', '--quiet', '--outdir', directory, document],
                   check=True, capture_output=True, timeout=600)
    pdf = os.path.join(directory, os.path.splitext(os.path.basename(document))[0] + '.pdf')
    if not os.path.isfile(pdf):
        raise SystemExit(f'{CLI} wrote no PDF for {document} — nothing to compare')
    return pdf


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    work, label = sys.argv[1], sys.argv[2]
    os.makedirs(work, exist_ok=True)

    documents = [
        ('slides', os.path.join(pitch_probe.FIXTURES, 'chart-face-theme-minor.pptx'),
         'ppt/charts/chart1.xml'),
        ('words', os.path.join(pitch_probe.FIXTURES, 'chart-bar-text.docx'),
         'word/charts/chart1.xml'),
    ]

    missing = []
    rows = []

    for track, source, part in documents:
        extension = os.path.splitext(source)[1]
        for family, path in pitch_probe.FACES.items():
            face = Metrics(path)
            for size in pitch_probe.SIZES:
                stem = f'{track}-{family.replace(" ", "")}-{size}'

                title = one_face(
                    pitch_probe.variant(
                        source, part, os.path.join(work, 'q-' + stem + extension), family,
                        size * 100),
                    part, os.path.join(work, 'p-' + stem + extension), family)
                found = pitch_probe.title_runs(
                    ours(title, os.path.join(work, 'o-pitch', stem)), size)
                if len(found) != pitch_probe.LINES:
                    missing.append(f'pitch {stem}: {len(found)} runs')
                    continue

                axis = one_face(
                    tick_probe.variant(
                        source, part, os.path.join(work, 'a-' + stem + extension), family,
                        size * 100, every_axis=True),
                    part, os.path.join(work, 't-' + stem + extension), family)
                pairs = tick_offsets(
                    ours(axis, os.path.join(work, 'o-tick', stem)), size)
                if len(pairs) < 3:
                    missing.append(f'tick {stem}: {len(pairs)} pairs')
                    continue

                gaps = [found[i][1] - found[i + 1][1] for i in range(pitch_probe.LINES - 1)]
                p = predicted(face, size)
                e = exact(face, size)
                rows.append({
                    'track': track, 'face': family, 'size': size,
                    'pitch': sum(gaps) / len(gaps),
                    'pitch_pixel': p['height'], 'pitch_exact': e['height'],
                    'offset': sum(pairs) / len(pairs),
                    'offset_pixel': p['ascent'] - p['height'] / 2.0,
                    'offset_exact': e['ascent'] - e['height'] / 2.0,
                })

    if missing:
        print('REFUSING TO SUMMARISE — cases with no measurable geometry:', file=sys.stderr)
        for line in missing:
            print('   ', line, file=sys.stderr)
        return 2

    path = os.path.join(HERE, f'ours-{label}.tsv')
    with open(path, 'w', encoding='utf-8') as handle:
        handle.write('track\tface\tsize\tpitch\tpitch_pixel\tpitch_exact\toffset\t'
                     'offset_pixel\toffset_exact\n')
        for row in rows:
            handle.write('{track}\t{face}\t{size}\t{pitch:.4f}\t{pitch_pixel:.4f}\t'
                         '{pitch_exact:.4f}\t{offset:.4f}\t{offset_pixel:.4f}\t'
                         '{offset_exact:.4f}\n'.format(**row))

    print(f'{"track":7s} {"face":17s} {"sz":>3s} {"pitch":>8s} {"pixel":>8s} {"err":>7s} '
          f'{"offset":>8s} {"pixel":>8s} {"err":>7s}')
    for row in rows:
        print(f'{row["track"]:7s} {row["face"]:17s} {row["size"]:3d} {row["pitch"]:8.3f} '
              f'{row["pitch_pixel"]:8.3f} {row["pitch"] - row["pitch_pixel"]:+7.3f} '
              f'{row["offset"]:8.3f} {row["offset_pixel"]:8.3f} '
              f'{row["offset"] - row["offset_pixel"]:+7.3f}')

    worst_pitch = max(abs(r['pitch'] - r['pitch_pixel']) for r in rows)
    worst_offset = max(abs(r['offset'] - r['offset_pixel']) for r in rows)
    inside = sum(1 for r in rows if abs(r['pitch'] - r['pitch_pixel']) <= 0.05)
    print(f'\n{label}: {len(rows)} cases; pitch within 0.05 pt of the pixel law on {inside}; '
          f'worst pitch {worst_pitch:.3f} pt, worst tick offset {worst_offset:.3f} pt')
    print(f'{label}: against exact scaling, worst pitch '
          f'{max(abs(r["pitch"] - r["pitch_exact"]) for r in rows):.3f} pt, worst offset '
          f'{max(abs(r["offset"] - r["offset_exact"]) for r in rows):.3f} pt')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
