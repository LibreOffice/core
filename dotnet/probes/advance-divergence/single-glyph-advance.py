#!/usr/bin/env python3
"""Isolate one glyph's advance and compare both stacks against the font's own hmtx.

**Superseded 2026-09-06.** The conclusion this probe fed -- that the reference does not
draw the design advance -- is withdrawn: it was read out of the PDF's glyph positioning,
which is quantised to whole thousandths of an em, and that is larger than the effect. The
measurements below stand; the reading of them does not. See `probes/advance-ppem/`.

The trick is differencing two repeat counts. Rendering a character N times gives an ink
box of (N-1) advances plus one glyph's ink width; rendering it M times and subtracting
cancels that trailing term exactly, so (w_N - w_M) / (N - M) is the advance and nothing
else. An earlier cut divided a single width by (N-1) and carried that ink term as a
silent bias.

What it established, 2026-08-15, at 12 pt against LibreOffice 26.2.4.2:

    face             ch  hmtx   exact      ours       ref        ref/exact
    Liberation Serif 'o' 1024   6.000000   6.000000   6.000000   1.000000
    Liberation Serif '.'  512   3.000000   3.000000   3.000000   1.000000
    Liberation Serif 'A' 1479   8.666016   8.666016   8.663520   0.999712
    Liberation Serif 'i'  569   3.333984   3.333984   3.324480   0.997149
    Carlito          'A' 1185   6.943359   6.943360   6.931680   0.998318
    Carlito          '.'  517   3.029297   3.029297   3.020880   0.997222

**Ours is exactly hmtx * size / upem on every glyph tested — the unhinted design
advance. The reference's is not**, differing per glyph by up to 0.3%. It agrees only
where the design advance is a clean fraction of the em: `o` and `n` are 1024 = upem/2
and `.` is 512 = upem/4. Carlito, whose advances are drawn to match Calibri rather than
to sit on round fractions, agrees on none of its six.

That pattern looks like a quantisation grid and **is not one**. Searching every N from
16 to 4000 units per em for a grid reproducing all twelve reference advances from hmtx
leaves a best-case maximum error of 0.007 pt — the same order as the defect. So the
reference is grid-fitting the outline, which is per-glyph and not derivable from hmtx
at all, and closing this means reproducing a hinted advance rather than rounding ours.

Two controls, both of which have cost a round elsewhere:

- `pdffonts` both PDFs before believing any comparison. If a face did not resolve the
  way the document asked, the numbers describe a substitution and not an advance.
- Assert both PDFs exist. The CLI rejects `-o` in favour of `--outdir`, and a glob over
  the output directory will happily pick up the reference's own file — which reads as
  a perfect match.

Usage:  XRAY_OFFICE_CLI=... python3 single-glyph-advance.py /abs/workdir
"""

import os
import re
import subprocess
import sys
import zipfile
from collections import defaultdict

CHARS = 'AoiMn.'
REPEATS = (10, 60)
POINTS = 12
FACES = ('Liberation Serif', 'Carlito')

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.document.main+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
        'relationships/officeDocument" Target="word/document.xml"/></Relationships>')
W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'


def build(path: str) -> list[tuple[str, str, int]]:
    body, labels = '', []
    for face in FACES:
        for ch in CHARS:
            for n in REPEATS:
                run = (f'<w:rFonts w:ascii="{face}" w:hAnsi="{face}"/>'
                       f'<w:sz w:val="{POINTS * 2}"/><w:kern w:val="0"/>')
                body += (f'<w:p><w:pPr><w:jc w:val="left"/><w:rPr>{run}</w:rPr></w:pPr>'
                         f'<w:r><w:rPr>{run}</w:rPr>'
                         f'<w:t xml:space="preserve">{ch * n}</w:t></w:r></w:p>')
                labels.append((face, ch, n))

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
        '<w:sectPr><w:pgSz w:w="16838" w:h="23812"/>'
        '<w:pgMar w:top="567" w:right="567" w:bottom="567" w:left="567"/></w:sectPr>'
        '</w:body></w:document>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)
    return labels


def widths(pdf: str) -> list[float]:
    text = subprocess.run(['pdftotext', '-bbox', '-f', '1', '-l', '1', pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    rows: dict[float, list[float]] = defaultdict(lambda: [1e9, -1e9])
    for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)"', text):
        y = round(float(m.group(2)), 0)
        rows[y][0] = min(rows[y][0], float(m.group(1)))
        rows[y][1] = max(rows[y][1], float(m.group(3)))
    if not rows:
        raise SystemExit(f'no <word> elements in {pdf} — did it render?')
    return [v[1] - v[0] for _, v in sorted(rows.items())]


def main() -> int:
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/single-glyph-advance'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)
    docx = os.path.join(out, 'adv.docx')
    labels = build(docx)

    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', out, docx],
                   capture_output=True, check=True)
    subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(out, 'ours'), docx],
                   check=True)

    reference, ours = os.path.join(out, 'adv.pdf'), os.path.join(out, 'ours', 'adv.pdf')
    for path in (reference, ours):
        if not os.path.isfile(path):
            raise SystemExit(f'{path} was not written — nothing to compare')
        listing = subprocess.run(['pdffonts', path], capture_output=True, text=True).stdout
        print(f'--- faces embedded in {path} ---')
        print('\n'.join(listing.splitlines()[2:]))

    from fontTools.ttLib import TTFont
    files = {face: subprocess.run(['fc-match', '-f', '%{file}', face],
                                  capture_output=True, text=True).stdout.strip()
             for face in FACES}

    r, o = widths(reference), widths(ours)
    span = REPEATS[1] - REPEATS[0]
    print(f"\n{'face':17s} {'ch':4s} {'hmtx':>6s} {'exact':>10s} {'ours':>10s} "
          f"{'ref':>10s} {'ref/exact':>10s}")
    for i in range(0, min(len(r), len(o), len(labels)) - 1, len(REPEATS)):
        face, ch, _ = labels[i]
        our_advance = (o[i + 1] - o[i]) / span
        ref_advance = (r[i + 1] - r[i]) / span
        font = TTFont(files[face])
        units = font['hmtx'][font.getBestCmap()[ord(ch)]][0]
        exact = units * POINTS / font['head'].unitsPerEm
        print(f'{face:17s} {ch!r:4s} {units:6d} {exact:10.6f} {our_advance:10.6f} '
              f'{ref_advance:10.6f} {ref_advance / exact:10.6f}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
