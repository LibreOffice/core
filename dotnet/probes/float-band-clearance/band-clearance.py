#!/usr/bin/env python3
"""Which paragraphs wrap beside a floating table, and which clear it?

Why this exists
---------------

`absrc-pac-01-info-note-en.doc` renders 6 pages against 7 with words at
1301/1303, so it is purely placement. Measured from the two PDFs' geometry on
page 1, the reference wraps **partially** past a floating icon table:

    reference   y=334.1 x=311.0   "INFORMATION HIGHLIGHTS"    beside the float
                y=700.0 x= 76.6   "1. OPENING AND REGISTRATION..."   below it
    ours        y=432.5 x=256.5   "5. GENERAL INFORMATION..."  beside, interleaved

The heading is about 130 pt wide and fits the roughly 204 pt band to the right
of the table, so it sits there. The contents entries need about 457 pt — their
dot leaders run to a right tab at the text width — and drop below instead. We
wrap all of them into the band, which fits the whole list on page 1 and is where
the missing page goes.

**Two observations do not fix a rule.** "A tab stop beyond the band" fits them,
and so does "a declared right indent wider than the band", and so does "the
paragraph's minimum width". This varies each of those independently against a
float of known width and reads which paragraphs land in the band.

What it found, 2026-08-15
-------------------------

    left margin 28.4 pt, float right edge about 268.4 pt, band about 298.6 pt

    case                 ref x      ref   our x      our  agree?
    short-plain          272.1   beside    28.4  cleared  NO
    long-plain           272.1   beside    28.4  cleared  NO
    tab-beyond-band      272.1   beside    28.4  cleared  NO
    tab-inside-band      272.1   beside    28.4  cleared  NO
    indent-right-zero    272.1   beside    28.4  cleared  NO
    indent-right-wide    272.1   beside    28.4  cleared  NO

**The hypothesis this was written to test is refuted.** The reference wrapped
`tab-beyond-band` — a right tab at the full text width with dot leaders, which
is exactly the `absrc-pac` contents-entry shape — *beside* the float. So "a tab
stop beyond the band makes a paragraph clear the float" is not the rule, and
neither is "a right indent wider than the band": `indent-right-zero` wrapped
too. Whatever makes `absrc-pac`'s contents list clear, it is not the paragraph's
declared width.

**And the probe found something it was not looking for: we do not float a DOCX
`w:tblpPr` table at all.** All six cases landed at the left margin, which is
where a paragraph goes when the table above it was stacked rather than floated.
That is a defect in its own right and it is the reason this probe cannot decide
the original question — our column is uninformative until it is fixed.

Note that this is the *opposite* of what `absrc-pac` does, and the two are
different code paths: that document is a `.doc`, and its WW8 positioned table
*is* floated by us, with the contents entries shifted right by 179.9 pt into the
band. So one path floats and the other does not, and at most one of them is
right.

Reading the output
------------------

Every case is one paragraph beside the same float. `x` is where its first word
landed:

- **x in the band** (greater than the float's right edge) — it wrapped beside.
- **x at the left margin** — it cleared the float.

A case where the reference clears and we wrap is a case the rule has to
explain; a case where both wrap is a control that stops the rule being written
too broadly.

Usage
-----

    XRAY_OFFICE_CLI=... python3 band-clearance.py /abs/workdir
"""

import os
import re
import subprocess
import sys
import zipfile

FACE = 'Liberation Serif'
HALF_POINTS = 22

# The float: a one-cell table pinned to the left, 240 pt wide and tall enough
# that every case below is beside it rather than under it.
FLOAT_WIDTH_TWIPS = 4800
FLOAT_ROWS = 24

# The text area is 11906 - 2*567 = 10772 twips; the band beside the float is
# what is left of that, about 5972 twips or 298 pt.
TEXT_WIDTH_TWIPS = 10772
BAND_TWIPS = TEXT_WIDTH_TWIPS - FLOAT_WIDTH_TWIPS

CASES = [
    # (label, what varies, the paragraph's pPr, its runs)
    ('short-plain', 'a short paragraph, nothing declared', '', 'SHORT'),
    ('long-plain', 'long enough to need several lines, nothing declared',
     '', 'LONG ' + ' '.join(f'w{i}' for i in range(40))),
    ('tab-beyond-band', 'a right tab at the full text width — the absrc-pac shape',
     f'<w:tabs><w:tab w:val="right" w:leader="dot" w:pos="{TEXT_WIDTH_TWIPS}"/></w:tabs>',
     'TABFAR\t9'),
    ('tab-inside-band', 'the same, with the tab inside the band',
     f'<w:tabs><w:tab w:val="right" w:leader="dot" w:pos="{TEXT_WIDTH_TWIPS - 500}"/></w:tabs>',
     'TABNEAR\t9'),
    ('indent-right-zero', 'no right indent, so the paragraph declares the full width',
     '<w:ind w:left="0" w:right="0"/>', 'INDNONE'),
    ('indent-right-wide', 'a right indent that pulls it inside the band',
     f'<w:ind w:left="0" w:right="{FLOAT_WIDTH_TWIPS}"/>', 'INDWIDE'),
]

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
    """One page per case: the same float, then that case's single paragraph."""
    run = f'<w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}"/><w:sz w:val="{HALF_POINTS}"/>'

    def floated():
        rows = ''.join(
            f'<w:tr><w:tc><w:tcPr><w:tcW w:w="{FLOAT_WIDTH_TWIPS}" w:type="dxa"/></w:tcPr>'
            f'<w:p><w:pPr><w:rPr>{run}</w:rPr></w:pPr><w:r><w:rPr>{run}</w:rPr>'
            f'<w:t>float row {i}</w:t></w:r></w:p></w:tc></w:tr>'
            for i in range(FLOAT_ROWS))
        return (
            '<w:tbl><w:tblPr>'
            f'<w:tblW w:w="{FLOAT_WIDTH_TWIPS}" w:type="dxa"/>'
            '<w:tblpPr w:leftFromText="180" w:rightFromText="180" w:vertAnchor="text"'
            ' w:horzAnchor="margin" w:tblpX="0" w:tblpY="1"/>'
            '</w:tblPr>'
            f'<w:tblGrid><w:gridCol w:w="{FLOAT_WIDTH_TWIPS}"/></w:tblGrid>{rows}</w:tbl>')

    body, labels = '', []
    for index, (label, _, ppr, text) in enumerate(CASES):
        if index:
            body += '<w:p><w:pPr><w:pageBreakBefore/></w:pPr></w:p>'

        body += floated()
        runs = ''.join(
            f'<w:r><w:rPr>{run}</w:rPr><w:t xml:space="preserve">{part}</w:t></w:r>'
            if i == 0 else f'<w:r><w:rPr>{run}</w:rPr><w:tab/><w:t>{part}</w:t></w:r>'
            for i, part in enumerate(text.split('\t')))
        body += f'<w:p><w:pPr>{ppr}<w:rPr>{run}</w:rPr></w:pPr>{runs}</w:p>'
        labels.append(label)

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
        '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="567" w:right="567" w:bottom="567" w:left="567"/></w:sectPr>'
        '</w:body></w:document>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)
    return labels


def marker(pdf, page, word):
    """Where a case's marker word landed on its page, or None when it is elsewhere."""
    text = subprocess.run(['pdftotext', '-bbox', '-f', str(page), '-l', str(page), pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    for x, y, found in re.findall(
            r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="[\d.]+" yMax="[\d.]+">([^<]*)</word>',
            text):
        if found.startswith(word):
            return float(x), float(y)

    return None


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/float-band'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)
    docx = os.path.join(out, 'band.docx')
    build(docx)

    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', out, docx],
                   capture_output=True, check=True)
    subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(out, 'ours'), docx],
                   check=True)

    reference, ours = os.path.join(out, 'band.pdf'), os.path.join(out, 'ours', 'band.pdf')
    for path in (reference, ours):
        if not os.path.isfile(path):
            raise SystemExit(f'{path} was not written — nothing to compare')

    margin = 567 / 20.0
    edge = margin + FLOAT_WIDTH_TWIPS / 20.0

    print(f'left margin {margin:.1f} pt, float right edge about {edge:.1f} pt, '
          f'band about {BAND_TWIPS / 20.0:.1f} pt wide\n')
    print(f"{'case':18s} {'ref x':>7s} {'ref':>8s} {'our x':>7s} {'our':>8s}  agree?")

    for index, (label, _, _, text) in enumerate(CASES):
        word = text.split('\t')[0].split(' ')[0]
        r = marker(reference, index + 1, word)
        o = marker(ours, index + 1, word)

        def side(hit):
            if hit is None:
                return 'missing'
            return 'beside' if hit[0] > edge - 5 else 'cleared'

        rs, os_ = side(r), side(o)
        print(f'{label:18s} {(r[0] if r else -1):7.1f} {rs:>8s} '
              f'{(o[0] if o else -1):7.1f} {os_:>8s}  {"yes" if rs == os_ else "NO"}')

    return 0


if __name__ == '__main__':
    raise SystemExit(main())
