#!/usr/bin/env python3
"""Separate kerning from base-advance divergence, and locate what actually drifts.

**Superseded 2026-09-06.** The conclusion this probe fed -- that the reference does not
draw the design advance -- is withdrawn: it was read out of the PDF's glyph positioning,
which is quantised to whole thousandths of an em, and that is larger than the effect. The
measurements below stand; the reading of them does not. See `probes/advance-ppem/`.

Why
---

`dotnet/CLAUDE.md`'s third absolute rule carried, for several rounds, the claim that
**"LibreOffice kerns 19% harder"**. It came from one line of one document and it is
wrong: kerning agrees to better than 1%, and the ~0.1% advance divergence is still
there with kerning switched off entirely.

This probe is the thing that should have been built before that sentence was written.
It sets the same string three ways so the two effects cannot be confused:

    KERNED     w:kern val="1"   pair kerning on
    UNKERNED   w:kern val="0"   pair kerning off
    spaced     a space between every letter, so no pair can form at all

The third is the control that matters. Turning a feature off asks the renderer to
agree with you about what "off" means; putting a space between every letter removes
the pairs from the *text*, and no shaper can kern across it.

Two faces are used deliberately. **Liberation Serif** carries its own advances.
**Carlito** is a metric-compatible substitute, drawn to reproduce Calibri's advances,
which is precisely where a quantisation rule would show itself. They behave
differently, and that difference is the finding.

What it measured, 2026-08-15, against LibreOffice 26.2.4.2
----------------------------------------------------------

Kerning's own contribution, as (unkerned width - kerned width):

    Liberation Serif 12 pt   ref 16.500 pt   ours 16.588 pt   ours/ref 1.0053
    Liberation Serif 24 pt       32.904          33.176                1.0083
    Carlito          12 pt       10.344          10.412                1.0066
    Carlito          24 pt       20.616          20.825                1.0101

Whole-line width divergence with every pair broken by a space:

    Liberation Serif   +0.011%      Carlito   +0.115%

Per-glyph pen positions on that line: Liberation Serif holds a constant 0.06-0.10 pt
offset and does not accumulate; Carlito starts 0.100 pt behind and is 0.063 pt ahead
by the fourteenth glyph, about +0.0125 pt per glyph. Per-glyph ink widths differ by
at most 0.010 pt and do not accumulate, so the ink is not the driver.

Reading it
----------

`pdftotext -bbox` gives a `<word>` per whitespace-delimited token and **no `<line>`
element** — an earlier cut of this script matched on `<line>` and reported zero rows
for both sides, which reads exactly like "the two agree". Group by `yMin` instead.

Note also that `xMax - xMin` on a `<word>` is the *ink* box, not the advance. The
advance is the difference between successive `xMin` values, which is why the pen
column below is the one to read.

Usage
-----

    python3 dotnet/probes/advance-divergence/kerning-vs-advance.py /abs/workdir

Needs `soffice` and `XRAY_OFFICE_CLI` set. Writes its docx and both PDFs under the
given directory.
"""

import os
import re
import subprocess
import sys
import zipfile
from collections import defaultdict

CONTENT_TYPES = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
    '<Default Extension="xml" ContentType="application/xml"/>'
    '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
    'officedocument.wordprocessingml.document.main+xml"/></Types>')

RELS = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
    'relationships/officeDocument" Target="word/document.xml"/></Relationships>')

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

# Pairs chosen because both faces kern them and the kern is large enough to survive
# the 0.001 pt resolution pdftotext reports.
PAIRS = 'AV AW AT Ta To Te Wa We Ye P. F, LT rq vy'
SPACED = ' '.join(PAIRS.replace(' ', ''))

FACES = ('Liberation Serif', 'Carlito')
SIZES = (24, 48)   # half-points: 12 pt and 24 pt


def paragraph(text: str, face: str, half_points: int, kern: int) -> str:
    run_properties = (
        f'<w:rFonts w:ascii="{face}" w:hAnsi="{face}"/>'
        f'<w:sz w:val="{half_points}"/><w:kern w:val="{kern}"/>')
    return (f'<w:p><w:pPr><w:jc w:val="left"/><w:rPr>{run_properties}</w:rPr></w:pPr>'
            f'<w:r><w:rPr>{run_properties}</w:rPr>'
            f'<w:t xml:space="preserve">{text}</w:t></w:r></w:p>')


def build(path: str) -> list[str]:
    body, labels = '', []
    for face in FACES:
        for half in SIZES:
            points = half // 2
            body += paragraph(PAIRS, face, half, 1)
            body += paragraph(SPACED, face, half, 1)
            body += paragraph(PAIRS, face, half, 0)
            labels += [f'{face} {points}pt KERNED',
                       f'{face} {points}pt spaced',
                       f'{face} {points}pt UNKERNED']

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
        '<w:sectPr><w:pgSz w:w="16838" w:h="11906" w:orient="landscape"/>'
        '<w:pgMar w:top="1134" w:right="567" w:bottom="1134" w:left="567"/></w:sectPr>'
        '</w:body></w:document>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CONTENT_TYPES)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)
    return labels


def words(pdf: str) -> list[tuple[float, float, float, str]]:
    text = subprocess.run(['pdftotext', '-bbox', '-f', '1', '-l', '1', pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    found = []
    for m in re.finditer(
            r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="[\d.]+">([^<]*)</word>',
            text):
        found.append((round(float(m.group(2)), 0), float(m.group(1)), float(m.group(3)), m.group(4)))
    if not found:
        raise SystemExit(f'no <word> elements in {pdf} — did it render?')
    return found


def lines(pdf: str) -> list[tuple[float, float]]:
    """(y, width) per line, width being last xMax minus first xMin."""
    rows: dict[float, list[float]] = defaultdict(lambda: [1e9, -1e9])
    for y, x0, x1, _ in words(pdf):
        rows[y][0] = min(rows[y][0], x0)
        rows[y][1] = max(rows[y][1], x1)
    return [(y, v[1] - v[0]) for y, v in sorted(rows.items())]


def main() -> int:
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/advance-divergence'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)
    docx = os.path.join(out, 'kern.docx')
    labels = build(docx)

    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', out, docx],
                   capture_output=True, check=True)
    subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(out, 'ours'), docx],
                   check=True)

    reference = os.path.join(out, 'kern.pdf')
    ours = os.path.join(out, 'ours', 'kern.pdf')
    # The guard an earlier probe of mine lacked: the CLI rejects -o in favour of --outdir, so a
    # mistyped invocation leaves no file and a glob happily picks up the reference's own output.
    for p in (reference, ours):
        if not os.path.isfile(p):
            raise SystemExit(f'{p} was not written — nothing to compare')

    r, o = lines(reference), lines(ours)
    print(f"{'case':30s} {'REF':>9s} {'OURS':>9s} {'delta':>8s} {'%':>8s}")
    width = {}
    for i in range(min(len(r), len(o))):
        label = labels[i] if i < len(labels) else f'line {i}'
        delta = o[i][1] - r[i][1]
        print(f'{label:30s} {r[i][1]:9.3f} {o[i][1]:9.3f} {delta:+8.3f} '
              f'{(100 * delta / r[i][1] if r[i][1] else 0):+7.3f}%')
        width[label] = (r[i][1], o[i][1])

    print('\nkerning\'s own contribution — unkerned minus kerned, which is what the pairs are worth:')
    for face in FACES:
        for half in SIZES:
            points = half // 2
            k = width.get(f'{face} {points}pt KERNED')
            u = width.get(f'{face} {points}pt UNKERNED')
            if not (k and u) or u[0] == k[0]:
                continue
            ref_kern, our_kern = u[0] - k[0], u[1] - k[1]
            print(f'  {face} {points}pt: ref {ref_kern:6.3f} pt  ours {our_kern:6.3f} pt'
                  f'  ours/ref {our_kern / ref_kern:.4f}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
