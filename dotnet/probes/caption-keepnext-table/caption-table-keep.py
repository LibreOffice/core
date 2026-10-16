#!/usr/bin/env python3
"""Does a `keepNext` caption drag its whole table to the next page, or does the table split?

Why this exists
---------------

`AC-150-5370-10G-updated-201604.docx` renders 693 pages against 696, and the
**first** of its three lost pages is exactly at reference page 187. Both
renderings agree line for line down to y=583.0 on page 186; then the reference
stops, leaving about 120 pt free, and opens page 187 with the caption
"Requirements for Gradation of Mixture" and the whole ten-row table under it.
We put the caption at y=601.8 and split the table after two data rows.

The caption is the built-in `Caption` style, whose `<w:pPr>` is
`<w:keepNext/><w:jc w:val="center"/>`, and its next sibling is `<w:tbl>`.
So the shape is: **a keep-with-next paragraph whose successor is a table.**

Our paginator cannot act on that shape at all. `Paginator.cs`:1357 requires
`Laid(paragraphIndex).Paragraph is { } next` — null for a table block — and
`MoveTrailingGroupToNextPage` stops its backward walk at a table on purpose
("keep-with-next is a paragraph property, and a paragraph cannot be kept with a
table it does not know about"). So the caption is placed and the table splits
under it.

**But a second explanation fits the same evidence and must be excluded before
anything is changed.** The table's first two rows are `<w:tblHeader/>`, and
Writer has its own rules about splitting a table with repeated headings — it may
be refusing the split for that reason, with the caption merely following the
table it is attached to. The document has 176 `tblHeader` and 110 `cantSplit`,
so picking the wrong one would be a plausible, wrong, corpus-wide change.

The two are separable by varying them independently, which is what this does.

What it builds
--------------

One page per case, each: filler lines to leave a chosen amount of room, then a
caption paragraph, then a ten-row table. Four families of case:

- `keep+hdr`  — caption has `keepNext`, table's first two rows are `tblHeader`
                (the AC-150 shape exactly)
- `keep`      — caption has `keepNext`, no repeated headers
- `hdr`       — no `keepNext`, first two rows are `tblHeader`
- `plain`     — neither

crossed with the room left before the caption, swept from "the whole table
fits" down to "not even the caption fits".

Reading the output
------------------

For each case the reference is asked two questions:

- `capPage`  — is the caption on the filler's page, or the next one?
- `split`    — do the table's data rows appear on *both* pages?

If `keep+hdr` and `keep` both move the caption and refuse to split while `hdr`
and `plain` split, the rule is **keepNext across a paragraph/table boundary**
and the fix is in `Paginator.cs`. If `keep+hdr` and `hdr` behave alike against
`keep` and `plain`, it is the repeated-heading rule and the fix is in
`TableLayouter`. If both matter, both are needed and the sweep says at which
room each takes over.

Usage
-----

    XRAY_OFFICE_CLI=... python3 caption-table-keep.py /abs/workdir
"""

import os
import re
import subprocess
import sys
import zipfile

FACE = 'Liberation Serif'
HALF_POINTS = 22

# US Letter with one-inch margins: body is 12240-2880 = 9360 twips wide and
# 15840-2880 = 12960 twips (648 pt) tall.
PAGE_W, PAGE_H, MARGIN = 12240, 15840, 1440
BODY_PT = (PAGE_H - 2 * MARGIN) / 20.0

DATA_ROWS = 8
HEADER_ROWS = 2

# Room to leave above the caption, in points. The table is about 10 rows of
# roughly 17 pt plus a 19 pt caption, so ~190 pt; the sweep straddles that and
# runs down past the point where even the caption cannot fit.
ROOMS = [200, 160, 120, 90, 60, 34]

# 'style' puts keepNext in a style whose w:name is the built-in `caption`, which is how
# AC-150 carries it; 'direct' puts it in the paragraph's own pPr, where nothing can
# merge it away; 'none' is the control.
CASES = [(k, h) for k in ('direct', 'style', 'none') for h in (True, False)]

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
PKG = 'http://schemas.openxmlformats.org/package/2006/relationships'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
      'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.document.main+xml"/>'
      '<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.styles+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f'<Relationships xmlns="{PKG}"><Relationship Id="rId1" Type="{R}/officeDocument"'
        ' Target="word/document.xml"/></Relationships>')
DOCRELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<Relationships xmlns="{PKG}"><Relationship Id="rId1" Type="{R}/styles"'
           ' Target="styles.xml"/></Relationships>')

# `Caption` carries keepNext exactly as the built-in style in AC-150 does; the
# `NoKeep` twin is identical without it, so the two differ in one bit only.
STYLES = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    f'<w:styles {W}>'
    '<w:style w:type="paragraph" w:styleId="Caption"><w:name w:val="caption"/>'
    '<w:pPr><w:keepNext/><w:jc w:val="center"/></w:pPr></w:style>'
    '<w:style w:type="paragraph" w:styleId="NoKeep"><w:name w:val="nokeep"/>'
    '<w:pPr><w:jc w:val="center"/></w:pPr></w:style>'
    '</w:styles>')


def build(path):
    run = f'<w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}"/><w:sz w:val="{HALF_POINTS}"/>'
    body, labels = '', []

    for keep, hdr in CASES:
        for room in ROOMS:
            tag = f'{keep}{"H" if hdr else "N"}{room}'
            labels.append((tag, keep, hdr, room))

            # One filler line is 12 pt of type on a single-spaced line; measured
            # rather than assumed below, since only the *ordering* of the cases
            # matters to the verdict.
            lines = max(1, int((BODY_PT - room) / 13.8))
            filler = ''.join(
                f'<w:p><w:pPr><w:rPr>{run}</w:rPr></w:pPr><w:r><w:rPr>{run}</w:rPr>'
                f'<w:t>{tag} filler line {i}</w:t></w:r></w:p>'
                for i in range(lines))

            rows = ''
            for i in range(HEADER_ROWS + DATA_ROWS):
                head = '<w:tblHeader/>' if (hdr and i < HEADER_ROWS) else ''
                kind = 'HEAD' if i < HEADER_ROWS else 'DATA'
                cells = ''.join(
                    f'<w:tc><w:tcPr><w:tcW w:w="3000" w:type="dxa"/></w:tcPr>'
                    f'<w:p><w:pPr><w:rPr>{run}</w:rPr></w:pPr><w:r><w:rPr>{run}</w:rPr>'
                    f'<w:t>{tag}{kind}{i}c{c}</w:t></w:r></w:p></w:tc>'
                    for c in range(2))
                rows += f'<w:tr><w:trPr>{head}</w:trPr>{cells}</w:tr>'

            style = 'Caption' if keep == 'style' else 'NoKeep'
            direct = '<w:keepNext/>' if keep == 'direct' else ''
            caption = (
                f'<w:p><w:pPr><w:pStyle w:val="{style}"/>{direct}<w:rPr>{run}</w:rPr></w:pPr>'
                f'<w:r><w:rPr>{run}</w:rPr><w:t>{tag}CAPTION</w:t></w:r></w:p>')

            body += (filler + caption
                     + '<w:tbl><w:tblPr><w:tblW w:w="6000" w:type="dxa"/></w:tblPr>'
                     '<w:tblGrid><w:gridCol w:w="3000"/><w:gridCol w:w="3000"/></w:tblGrid>'
                     + rows + '</w:tbl>'
                     + '<w:p><w:pPr><w:pageBreakBefore/></w:pPr></w:p>')

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
        f'<w:sectPr><w:pgSz w:w="{PAGE_W}" w:h="{PAGE_H}"/>'
        f'<w:pgMar w:top="{MARGIN}" w:right="{MARGIN}" w:bottom="{MARGIN}" w:left="{MARGIN}"/>'
        '</w:sectPr></w:body></w:document>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/_rels/document.xml.rels', DOCRELS)
        z.writestr('word/styles.xml', STYLES)
        z.writestr('word/document.xml', document)

    return labels


def per_page(pdf):
    """Every page's text, so a marker can be located by page rather than by offset."""
    text = subprocess.run(['pdftotext', pdf, '-'], capture_output=True, text=True,
                          check=True).stdout
    return [re.sub(r'\s+', ' ', p) for p in text.split('\f')]


def verdict(pages, tag):
    """Which page the caption landed on, and whether the table's data rows straddle."""
    cap = next((i for i, p in enumerate(pages) if f'{tag}CAPTION' in p), None)
    data = [i for i, p in enumerate(pages) if f'{tag}DATA' in p]
    if cap is None or not data:
        return None, None, None

    filler = [i for i, p in enumerate(pages) if f'{tag} filler line 0' in p]
    base = filler[0] if filler else cap
    return cap - base, len(set(data)) > 1, min(data) - base


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/caption-keep'
    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)

    docx = os.path.join(out, 'caption.docx')
    labels = build(docx)

    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', out, docx],
                   capture_output=True, check=True)
    reference = os.path.join(out, 'caption.pdf')
    if not os.path.isfile(reference):
        raise SystemExit(f'{reference} was not written — nothing to compare')
    ref = per_page(reference)

    cli = os.environ.get('XRAY_OFFICE_CLI')
    ours = None
    if cli:
        subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(out, 'ours'), docx],
                       check=True)
        path = os.path.join(out, 'ours', 'caption.pdf')
        ours = per_page(path) if os.path.isfile(path) else None

    print(f'body {BODY_PT:.0f} pt; table is {HEADER_ROWS} header + {DATA_ROWS} data rows\n')
    print(f'{"case":14s} {"room":>5s} | {"ref cap":>7s} {"ref split":>9s} '
          f'| {"our cap":>7s} {"our split":>9s}  agree?')

    for tag, keep, hdr, room in labels:
        rc, rs, _ = verdict(ref, tag)
        oc, os_, _ = verdict(ours, tag) if ours else (None, None, None)
        name = f'{keep}+{"hdr" if hdr else "plain"}'
        same = '' if ours is None else ('yes' if (rc, rs) == (oc, os_) else 'NO')
        print(f'{name:14s} {room:5d} | {str(rc):>7s} {str(rs):>9s} '
              f'| {str(oc):>7s} {str(os_):>9s}  {same}')

    print('\nref cap 0 = caption stayed on the filler page, 1 = it moved with the table.')
    print('ref split True = the table\'s data rows appear on two pages.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
