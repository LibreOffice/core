#!/usr/bin/env python3
"""What does a page numbered nought print, in each of the five sequences?

Why this exists
---------------

`EHEST-SMS-Safety-Management-Manual-V2.docx` declares `<w:pgNumType w:start="0"/>`
and its reference footer reads `Page 0 of 82`. Ours read `Page 1 of 80` — on the
first page only, because from the second page on the two agree exactly (1, 2, 3,
… 41 on both). A first page alone being wrong reads as a counter fault and is
not one: the counter is right and the *formatting* clamped it.

`NoteNumbering.Render` raised every value to one, with the reasoning that "none
of the sequences has a zeroth term". That is true of four of the five and false
of the one that matters, and the clamp is invisible until a document asks for
page nought — three in the corpus do (`EHEST-SMS…`,
`final-technical-report-template`, `Technical_Issue_Report_Form`).

The clamp is safe for a *note*, which is what it was written for:
`NoteNumbering.Citation` clamps its own start before calling, so a footnote never
arrives here below one. It is only the page-number path that reaches it.

What it builds
--------------

Ten three-page documents, one per (`w:pgNumType/@w:fmt`, `w:start`) pair, each
with a `PAGE` field in the footer bracketed by `[` and `]` so the empty result is
still findable in the extracted text. Both engines render each; the table is the
answer.

Reading the output
------------------

`decimal` is the only sequence with a zeroth term, and it prints one. The other
four print **nothing at all** for nought — not their first term, which is what
clamping would give, and not a zero. Any row where the two columns differ is a
defect; every row agreeing is what this file exists to keep true.

Usage
-----

    export XRAY_OFFICE_CLI=<tree>/dotnet/tools/XRay.Office.Cli/…/XRay.Office.Cli
    python3 page-number-zero.py [outdir]
"""

import os
import re
import subprocess
import sys
import zipfile

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'

# The <Relationships> element is in the PACKAGE namespace, not the one its Type attributes
# point into. Using R for both writes a file LibreOffice refuses with no further detail.
PKG = 'http://schemas.openxmlformats.org/package/2006/relationships'

FORMATS = ('decimal', 'lowerRoman', 'upperRoman', 'lowerLetter', 'upperLetter')
STARTS = ('0', '1')


def build(path, page_numbering):
    """Three pages, each footing a bracketed PAGE field."""
    footer = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:ftr {W}>'
        '<w:p><w:r><w:t xml:space="preserve">[</w:t></w:r>'
        '<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
        '<w:r><w:instrText xml:space="preserve"> PAGE </w:instrText></w:r>'
        '<w:r><w:fldChar w:fldCharType="separate"/></w:r><w:r><w:t>1</w:t></w:r>'
        '<w:r><w:fldChar w:fldCharType="end"/></w:r>'
        '<w:r><w:t xml:space="preserve">]</w:t></w:r></w:p></w:ftr>')

    body = ''.join(
        f'<w:p><w:r><w:t>Body {at}</w:t></w:r></w:p>'
        '<w:p><w:r><w:br w:type="page"/></w:r></w:p>' for at in range(3))

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
        f'<w:sectPr><w:footerReference w:type="default" r:id="rId2" xmlns:r="{R}"/>'
        f'{page_numbering}'
        '<w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:footer="360"/>'
        '</w:sectPr></w:body></w:document>')

    types = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
        '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
        'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
        '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
        'officedocument.wordprocessingml.document.main+xml"/>'
        '<Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-'
        'officedocument.wordprocessingml.footer+xml"/></Types>')

    root = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="{PKG}">'
            f'<Relationship Id="rId1" Type="{R}/officeDocument" Target="word/document.xml"/>'
            '</Relationships>')

    parts = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="{PKG}">'
             f'<Relationship Id="rId2" Type="{R}/footer" Target="footer1.xml"/></Relationships>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as package:
        package.writestr('[Content_Types].xml', types)
        package.writestr('_rels/.rels', root)
        package.writestr('word/_rels/document.xml.rels', parts)
        package.writestr('word/document.xml', document)
        package.writestr('word/footer1.xml', footer)


def printed(pdf):
    """What each page's footer field came out as, brackets stripped."""
    text = subprocess.run(['pdftotext', pdf, '-'], capture_output=True, text=True,
                          check=True).stdout
    found = []
    for page in text.split('\f')[:-1]:
        match = re.search(r'\[(.*?)\]', page)
        found.append(match.group(1) if match else '?')

    return found


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/page-number-zero'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    print(f"{'fmt':12s} {'start':6s} {'reference':24s} ours")

    failures = 0
    for fmt in FORMATS:
        for start in STARTS:
            directory = os.path.join(out, f'{fmt}-{start}')
            os.makedirs(os.path.join(directory, 'ours'), exist_ok=True)
            docx = os.path.join(directory, 't.docx')
            build(docx, f'<w:pgNumType w:start="{start}" w:fmt="{fmt}"/>')

            subprocess.run(
                ['soffice', '--headless', '--convert-to', 'pdf', '--outdir', directory, docx],
                capture_output=True, check=True)
            subprocess.run(
                [cli, 'render', '--quiet', '--outdir', os.path.join(directory, 'ours'), docx],
                check=True)

            reference = os.path.join(directory, 't.pdf')
            ours = os.path.join(directory, 'ours', 't.pdf')
            for produced in (reference, ours):
                if not os.path.isfile(produced):
                    raise SystemExit(f'{produced} was not written — nothing to compare')

            left, right = printed(reference), printed(ours)
            if left != right:
                failures += 1

            print(f'{fmt:12s} {start:6s} {str(left):24s} {right}'
                  + ('' if left == right else '   <<< DIFFERS'))

    print('\ndecimal is the only sequence with a zeroth term and prints one; the other four print')
    print('nothing at all for nought — never their first term, which is what clamping would give.')
    return 1 if failures else 0


if __name__ == '__main__':
    raise SystemExit(main())
