#!/usr/bin/env python3
"""What does a numbering level with no `w:start` count from?

Why this exists
---------------

`ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx` numbers its sections one
higher than LibreOffice does throughout: the reference draws
`0. Introduction`, `1. References`, `2. List of Abbreviations`, and we drew
`1.`, `2.`, `3.`. The document's own table of contents — stored text, written by
Word — agrees with the reference, so Word numbered from zero too.

Its heading list is `numId` 21 → `abstractNum` 9, and that level 0 is

    <w:lvl w:ilvl="0"><w:numFmt w:val="decimal"/><w:lvlText w:val="%1."/>…</w:lvl>

with **no `w:start` element at all**. Nothing else in the file starts a list at
zero: a scan of all 42 `abstractNum` definitions finds level-0 starts of 3 and 4
and no 0, and there is no `w:startOverride` anywhere.

So the question is only what the *default* is, and that is worth measuring
rather than remembering: getting it wrong changes the visible numbering of every
list in every document that omits the element.

What it builds
--------------

Four one-level decimal lists of three items, differing in exactly one thing:

- `absent`  — no `w:start` element
- `start0`  — `<w:start w:val="0"/>`
- `start1`  — `<w:start w:val="1"/>`
- `start3`  — `<w:start w:val="3"/>`

The three explicit cases are the controls. If we agree on those and disagree on
the omission, the disagreement is the default and not the parsing.

What it found, 2026-08-16, LibreOffice 26.2.4.2
-----------------------------------------------

    case      ref                ours (before)      ours (after)
    absent    0. 1. 2.           1. 2. 3.           0. 1. 2.
    start0    0. 1. 2.           0. 1. 2.           0. 1. 2.
    start1    1. 2. 3.           1. 2. 3.           1. 2. 3.
    start3    3. 4. 5.           3. 4. 5.           3. 4. 5.

**An absent `w:start` means zero.** We defaulted it to one
(`WordNumbering.cs`), which is the single line this probe changed.

What it did not fix
-------------------

`ABCD-FE-01-00` stays at 14 pages against 15 afterwards, and its word excess
only falls from +117 to +109. The rest of that document is a different and much
larger thing: it holds 87 `m:oMath` and 33 `m:oMathPara` formulas, and
LibreOffice draws them with **no extractable text at all** while we draw real
text. Measured on page 4, both put `=` at x≈244.6 and `[1/rad]` at x≈519.5 — the
same slot — and the reference leaves the 269.5 pt between them empty of text
where we write `dcLdα`. Ink agrees to a tenth of a percent (7.251 % against
7.359 %), so both *draw* the formula. Across the document 156 tokens are ours
alone against 39 the reference's alone, which is 0.97 per formula site.

Usage
-----

    XRAY_OFFICE_CLI=... python3 numbering-start-default.py /abs/workdir
"""

import os
import subprocess
import sys
import zipfile

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
PKG = 'http://schemas.openxmlformats.org/package/2006/relationships'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'

CONTENT_TYPES = (
    '<?xml version="1.0"?>'
    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
    'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
    '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
    'officedocument.wordprocessingml.document.main+xml"/>'
    '<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-'
    'officedocument.wordprocessingml.numbering+xml"/></Types>')

ROOT_RELS = (f'<?xml version="1.0"?><Relationships xmlns="{PKG}">'
             f'<Relationship Id="rId1" Type="{R}/officeDocument" Target="word/document.xml"/>'
             '</Relationships>')

DOC_RELS = (f'<?xml version="1.0"?><Relationships xmlns="{PKG}">'
            f'<Relationship Id="rId1" Type="{R}/numbering" Target="numbering.xml"/>'
            '</Relationships>')

CASES = [('absent', None), ('start0', 0), ('start1', 1), ('start3', 3)]


def build(path, start):
    element = f'<w:start w:val="{start}"/>' if start is not None else ''

    numbering = (
        f'<?xml version="1.0"?><w:numbering {W}>'
        f'<w:abstractNum w:abstractNumId="0"><w:lvl w:ilvl="0">{element}'
        '<w:numFmt w:val="decimal"/><w:lvlText w:val="%1."/><w:lvlJc w:val="left"/></w:lvl>'
        '</w:abstractNum><w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num></w:numbering>')

    body = ''.join(
        '<w:p><w:pPr><w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr></w:pPr>'
        f'<w:r><w:t>Item {i}</w:t></w:r></w:p>'
        for i in range(3))

    document = (
        f'<?xml version="1.0"?><w:document {W}><w:body>{body}'
        '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
        '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
        '</w:sectPr></w:body></w:document>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as package:
        package.writestr('[Content_Types].xml', CONTENT_TYPES)
        package.writestr('_rels/.rels', ROOT_RELS)
        package.writestr('word/_rels/document.xml.rels', DOC_RELS)
        package.writestr('word/numbering.xml', numbering)
        package.writestr('word/document.xml', document)


def numbers(pdf):
    """The list labels, in order — the only part of the page that varies."""
    text = subprocess.run(['pdftotext', pdf, '-'], capture_output=True, text=True,
                          check=True).stdout
    return ' '.join(w for w in text.split() if w.endswith('.') and w[:-1].isdigit())


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/numbering-start'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    print(f"{'case':8s} {'reference':16s} {'ours':16s} agree?")

    for label, start in CASES:
        directory = os.path.join(out, label)
        os.makedirs(os.path.join(directory, 'ours'), exist_ok=True)
        docx = os.path.join(directory, 't.docx')
        build(docx, start)

        subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', directory,
                        docx], capture_output=True, check=True)
        subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(directory, 'ours'),
                        docx], check=True)

        reference = os.path.join(directory, 't.pdf')
        ours = os.path.join(directory, 'ours', 't.pdf')
        for produced in (reference, ours):
            if not os.path.isfile(produced):
                raise SystemExit(f'{produced} was not written — nothing to compare')

        r, o = numbers(reference), numbers(ours)
        print(f'{label:8s} {r:16s} {o:16s} {"yes" if r == o else "NO"}')

    print('\nthe three explicit cases are the controls: disagreeing only on `absent`')
    print('means the difference is the default, not the parsing.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
