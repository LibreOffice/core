#!/usr/bin/env python3
"""When a numbered item skips a level, what happens to the level it skipped?

Why this exists
---------------

`OM template for non-complex NCC operators_August 2016.docx` opens its `0.2`
section with a `Heading4` before any `Heading3` has appeared. The reference then
numbers the headings that follow `0.2.2`, `0.2.3`, `0.2.4` — and the document's
own stored table of contents, written by Word, agrees. We numbered them `0.2.1`,
`0.2.2`, `0.2.3`: one too low, all the way down.

The deeper item draws the skipped level itself — both engines render it
`0.2.1.1` — so the question is only what that display *does* to the counter.
`WordNumbering.FormatLabel` rendered the missing component from `StartOf` and
threw the value away, leaving the level with no counter at all, so its first real
item took the start value a second time.

What it builds
--------------

Three documents over one four-level `multilevel` list with no `w:start` anywhere,
differing only in the order of the levels their paragraphs sit at:

- `no skip`         — 0, 1, 1, 2, the control, which must not move
- `skip one level`  — 0, **2**, 1, 1
- `skip two levels` — 0, **3**, 1, 2

Reading the output
------------------

The rows to look at are the ones *after* the skip. If a level shown inside a
deeper item's number counts as used, the next item at that level counts on from
it; if it does not, that item takes the start value again and everything under
the parent is one low.

Measured on 26.2.4.2, the answer is that it counts:

    skip one level   ref  0 / 0.0.0 / 0.1 / 0.2
                     ours 0 / 0.0.0 / 0.0 / 0.1     (before the fix)

Note the numbers begin at **nought**, which is correct and separately measured:
a level with no `w:start` starts at zero, not one — see
`probes/numbering-start-default/`. The control matters for the same reason it
always does: a run with no skip in it is identical either way, which is why this
defect went unnoticed for so long. It needs a deeper item to appear before its
own parent does.

Usage
-----

    export XRAY_OFFICE_CLI=<tree>/dotnet/tools/XRay.Office.Cli/…/XRay.Office.Cli
    python3 skipped-level-counter.py [outdir]
"""

import os
import re
import subprocess
import sys
import zipfile

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG = 'http://schemas.openxmlformats.org/package/2006/relationships'

LEVELS = 4

CASES = {
    'no skip': [(0, 'A'), (1, 'mid'), (1, 'mid2'), (2, 'sub')],
    'skip one level': [(0, 'A'), (2, 'deep'), (1, 'mid'), (1, 'mid2')],
    'skip two levels': [(0, 'A'), (3, 'deeper'), (1, 'mid'), (2, 'sub')],
}


def numbering():
    """One multilevel list, every level decimal, every level showing its ancestors."""
    levels = ''.join(
        f'<w:lvl w:ilvl="{at}"><w:numFmt w:val="decimal"/>'
        f'<w:lvlText w:val="{".".join("%" + str(k + 1) for k in range(at + 1))}"/>'
        f'<w:lvlJc w:val="left"/></w:lvl>' for at in range(LEVELS))

    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:numbering {W}>'
            f'<w:abstractNum w:abstractNumId="0"><w:multiLevelType w:val="multilevel"/>{levels}'
            '</w:abstractNum><w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num></w:numbering>')


def build(path, items):
    body = ''.join(
        f'<w:p><w:pPr><w:numPr><w:ilvl w:val="{level}"/><w:numId w:val="1"/></w:numPr></w:pPr>'
        f'<w:r><w:t>{text}</w:t></w:r></w:p>' for level, text in items)

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
        '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720"/></w:sectPr>'
        '</w:body></w:document>')

    types = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
        '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
        'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
        '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
        'officedocument.wordprocessingml.document.main+xml"/>'
        '<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-'
        'officedocument.wordprocessingml.numbering+xml"/></Types>')

    root = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="{PKG}">'
            f'<Relationship Id="rId1" Type="{R}/officeDocument" Target="word/document.xml"/>'
            '</Relationships>')

    parts = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="{PKG}">'
             f'<Relationship Id="rId2" Type="{R}/numbering" Target="numbering.xml"/>'
             '</Relationships>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as package:
        package.writestr('[Content_Types].xml', types)
        package.writestr('_rels/.rels', root)
        package.writestr('word/_rels/document.xml.rels', parts)
        package.writestr('word/document.xml', document)
        package.writestr('word/numbering.xml', numbering())


def labelled(pdf):
    text = subprocess.run(['pdftotext', '-layout', pdf, '-'], capture_output=True, text=True,
                          check=True).stdout
    return [' '.join(line.split()) for line in text.splitlines() if line.strip()]


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/skipped-level-counter'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    failures = 0
    for label, items in CASES.items():
        directory = os.path.join(out, re.sub(r'\W+', '_', label))
        os.makedirs(os.path.join(directory, 'ours'), exist_ok=True)
        docx = os.path.join(directory, 't.docx')
        build(docx, items)

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

        left, right = labelled(reference), labelled(ours)
        if left != right:
            failures += 1

        print(f'--- {label}   (levels {[level for level, _ in items]})')
        print(f'    ref  {left}')
        print(f'    ours {right}' + ('' if left == right else '   <<< DIFFERS'))

    print('\nA level shown inside a deeper item\'s number has been used, and the next item at that')
    print('level counts on from it. The no-skip control must stay identical either way.')
    return 1 if failures else 0


if __name__ == '__main__':
    raise SystemExit(main())
