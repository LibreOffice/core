#!/usr/bin/env python3
"""How narrow must the band beside a float get before the reference stops wrapping?

Why this exists
---------------

`band-clearance.py` refuted the width hypotheses: against a float leaving a
298 pt band, the reference wraps a paragraph with a right tab at the full text
width beside it, and one with no right indent beside it too. So neither the tab
stop nor the declared width is what makes `absrc-pac-01-info-note-en.doc`'s
contents list clear its float.

One variable was held fixed there and is the obvious remaining candidate: the
band was 298 pt wide, and `absrc-pac`'s is about 204 pt. This sweeps the float's
width — and so the band's — with the paragraph shape held constant, and reads
off where the reference changes its mind.

The paragraph is the `absrc-pac` contents-entry shape throughout: a number, a
title, a dot leader and a right tab at the text width.

It is also now established that the divergence on that document is *exactly* at
the heading-to-list transition and nothing earlier. Our heading lands at
y=332.1 x=310.9 and the reference's at y=334.1 x=311.0 — the same place — so
everything above agrees, and the "the empty paragraphs before it are taller in
the reference" explanation is dead: there is no room for them to differ.

What it found, 2026-08-15 — band width is refuted too
-----------------------------------------------------

    float pt  band pt   ref x  reference
       160.0    378.6   192.1     beside
       240.0    298.6   272.1     beside
       300.0    238.6   332.1     beside
       340.0    198.6   372.1     beside
       400.0    138.6   432.1     beside

**No threshold.** The reference wraps the contents-entry shape beside the float
at every band width down to 138.6 pt — including 198.6 pt, which is narrower
than the band in `absrc-pac`. So the band's width is not what decides either,
and it lays the paragraph out in the band with its tab stop clamped rather than
moving it below.

An arithmetic correction that came out of this, because it was used above and
was wrong: `absrc-pac`'s band is about **314 pt**, not 204. Its heading sits at
x=311.0, and a string of that width centred in a band starting at the float's
right edge puts the edge at about 220 pt — so the band runs 220 to 534. The 204
figure was read off a rendered image rather than computed, and the sweep would
have been pointed at the wrong range if it had been trusted.

So four readings are now dead for this document: the paragraph's declared width,
its tab stop, its right indent, and the band's width. What is left points at the
float's own properties rather than at the text — and `absrc-pac` is a `.doc`,
so its WW8 positioned table may carry a wrap mode this DOCX probe cannot
express. That is where the next attempt should start, not with another text
property.

Reading the output
------------------

`beside` means the paragraph's first word landed to the right of the float,
`cleared` means it landed at the left margin. A threshold band width, with
`beside` above it and `cleared` below, is the rule. No threshold means the band
width is not what decides either, and the next variable has to come from the
float's own properties rather than from the text.

Our own column is not comparable yet — we do not float a DOCX `w:tblpPr` table
at all (task #65), so every case reads `cleared` for us regardless. The
reference column is the measurement here.

Usage
-----

    XRAY_OFFICE_CLI=... python3 band-width-sweep.py /abs/workdir
"""

import os
import re
import subprocess
import sys
import zipfile

FACE = 'Liberation Serif'
HALF_POINTS = 22
TEXT_WIDTH_TWIPS = 10772
FLOAT_ROWS = 26

# Float widths in twips; the band is TEXT_WIDTH_TWIPS minus these, so this walks
# the band from about 380 pt down to about 90 pt and straddles absrc-pac's 204.
FLOAT_WIDTHS = [3200, 4000, 4800, 5200, 5600, 6000, 6400, 6800, 7200, 8000]

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


def build(path):
    run = f'<w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}"/><w:sz w:val="{HALF_POINTS}"/>'
    body = ''

    for index, width in enumerate(FLOAT_WIDTHS):
        rows = ''.join(
            f'<w:tr><w:tc><w:tcPr><w:tcW w:w="{width}" w:type="dxa"/></w:tcPr>'
            f'<w:p><w:pPr><w:rPr>{run}</w:rPr></w:pPr><w:r><w:rPr>{run}</w:rPr>'
            f'<w:t>float {i}</w:t></w:r></w:p></w:tc></w:tr>'
            for i in range(FLOAT_ROWS))

        brk = '<w:p><w:pPr><w:pageBreakBefore/></w:pPr></w:p>' if index else ''
        body += (
            f'{brk}<w:tbl><w:tblPr><w:tblW w:w="{width}" w:type="dxa"/>'
            '<w:tblpPr w:leftFromText="180" w:rightFromText="180" w:vertAnchor="text"'
            ' w:horzAnchor="margin" w:tblpX="0" w:tblpY="1"/></w:tblPr>'
            f'<w:tblGrid><w:gridCol w:w="{width}"/></w:tblGrid>{rows}</w:tbl>'
            f'<w:p><w:pPr>'
            f'<w:tabs><w:tab w:val="right" w:leader="dot" w:pos="{TEXT_WIDTH_TWIPS}"/></w:tabs>'
            f'<w:rPr>{run}</w:rPr></w:pPr>'
            f'<w:r><w:rPr>{run}</w:rPr><w:t xml:space="preserve">CASE{index} ENTRY TITLE</w:t></w:r>'
            f'<w:r><w:rPr>{run}</w:rPr><w:tab/><w:t>9</w:t></w:r></w:p>')

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
        '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="567" w:right="567" w:bottom="567" w:left="567"/></w:sectPr>'
        '</w:body></w:document>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)


def where(pdf, page, word):
    text = subprocess.run(['pdftotext', '-bbox', '-f', str(page), '-l', str(page), pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    for x, _, found in re.findall(
            r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="[\d.]+" yMax="[\d.]+">([^<]*)</word>',
            text):
        if found.startswith(word):
            return float(x)

    return None


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/band-sweep'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)
    docx = os.path.join(out, 'sweep.docx')
    build(docx)

    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', out, docx],
                   capture_output=True, check=True)
    subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(out, 'ours'), docx],
                   check=True)

    reference, ours = os.path.join(out, 'sweep.pdf'), os.path.join(out, 'ours', 'sweep.pdf')
    for path in (reference, ours):
        if not os.path.isfile(path):
            raise SystemExit(f'{path} was not written — nothing to compare')

    margin = 567 / 20.0
    print(f"{'float pt':>9s} {'band pt':>8s} {'ref x':>7s} {'reference':>10s} {'our x':>7s}")

    for index, width in enumerate(FLOAT_WIDTHS):
        edge = margin + width / 20.0
        band = (TEXT_WIDTH_TWIPS - width) / 20.0
        r = where(reference, index + 1, f'CASE{index}')
        o = where(ours, index + 1, f'CASE{index}')
        verdict = 'missing' if r is None else ('beside' if r > edge - 5 else 'cleared')

        print(f'{width / 20.0:9.1f} {band:8.1f} {(r if r else -1):7.1f} {verdict:>10s} '
              f'{(o if o else -1):7.1f}')

    print('\nabsrc-pac\'s band is about 204 pt; a threshold between beside and cleared is the rule.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
