#!/usr/bin/env python3
"""Decide how Calc quantises the space width behind an OOXML `indent`.

An OOXML `indent` level is three spaces of the *workbook's* default font
(`sc/source/filter/oox/stylesbuffer.cxx`:1263), and one space is
`xFont->getCharWidth(' ')` (`sc/source/filter/oox/unitconverter.cxx`:139), which is
`OutputDevice::GetTextWidth` cast to `sal_Int16` -- **whole twips**. So the indent is
`3 * level * q(space)` for some integer quantiser `q`, and the only question is whether `q`
truncates or rounds.

Liberation Sans' space is 569/2048 em, so its twip width is 5.5566 twips per point: the
fractional part sweeps through the sizes and only some of them separate `floor` from
`round`. Each workbook here differs from the next in one thing, the default font size, and
each states the same two cells -- one at indent 0 and one at indent 2 in the same column --
so the pen difference between them is the indent and nothing else.

Measured 2026-09-06 against LibreOffice 24.2.7.2 (`/usr/bin/soffice`) and 26.2.4.2
(`/opt/libreoffice26.2/program/soffice`, bundled Latin duplicates and Latin Noto aside),
system fonts, `fc-match Arial` answering Liberation Sans.

Usage:  XRAY_OFFICE_CLI=... python3 indent-twip-rounding.py <workdir>
"""

import math
import os
import re
import subprocess
import sys
import zipfile

LEVEL = 2
SIZES = [8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 24, 28, 30, 36]
FACE = 'Arial'                  # resolves to Liberation Sans, whose space is 569/2048 em
SPACE_EM = 569 / 2048

CONTENT_TYPES = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
    '<Default Extension="xml" ContentType="application/xml"/>'
    '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-'
    'officedocument.spreadsheetml.sheet.main+xml"/>'
    '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-'
    'officedocument.spreadsheetml.worksheet+xml"/>'
    '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-'
    'officedocument.spreadsheetml.styles+xml"/></Types>')
ROOT_RELS = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
    'relationships/officeDocument" Target="xl/workbook.xml"/></Relationships>')
BOOK_RELS = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
    'relationships/worksheet" Target="worksheets/sheet1.xml"/>'
    '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
    'relationships/styles" Target="styles.xml"/></Relationships>')
NS = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
RNS = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
WORKBOOK = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<workbook xmlns="{NS}" xmlns:r="{RNS}"><sheets>'
            f'<sheet name="S" sheetId="1" r:id="rId1"/></sheets></workbook>')


def styles(size: int) -> str:
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><styleSheet xmlns="{NS}">'
            f'<fonts count="1"><font><sz val="{size}"/><name val="{FACE}"/></font></fonts>'
            '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
            '<borders count="1"><border/></borders>'
            '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/>'
            '</cellStyleXfs>'
            '<cellXfs count="2">'
            '<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyAlignment="1">'
            '<alignment horizontal="left"/></xf>'
            f'<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyAlignment="1">'
            f'<alignment horizontal="left" indent="{LEVEL}"/></xf>'
            '</cellXfs></styleSheet>')


SHEET = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><worksheet xmlns="{NS}">'
         '<sheetData>'
         '<row r="1"><c r="A1" t="inlineStr"><is><t>Zero</t></is></c></row>'
         '<row r="2"><c r="A2" s="1" t="inlineStr"><is><t>Ind</t></is></c></row>'
         '</sheetData></worksheet>')


def build(path: str, size: int) -> None:
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CONTENT_TYPES)
        z.writestr('_rels/.rels', ROOT_RELS)
        z.writestr('xl/workbook.xml', WORKBOOK)
        z.writestr('xl/_rels/workbook.xml.rels', BOOK_RELS)
        z.writestr('xl/styles.xml', styles(size))
        z.writestr('xl/worksheets/sheet1.xml', SHEET)


def pens(pdf: str) -> dict[str, float]:
    text = subprocess.run(['pdftotext', '-bbox', '-f', '1', '-l', '1', pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    found = {}
    for m in re.finditer(r'<word xMin="([\d.]+)"[^>]*>(.*?)</word>', text):
        found.setdefault(m.group(2), float(m.group(1)))
    return found


def main() -> int:
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    targets = {'24.2.7.2': '/usr/bin/soffice', '26.2.4.2': '/opt/libreoffice26.2/program/soffice'}
    cli = os.environ.get('XRAY_OFFICE_CLI')

    version = lambda b: subprocess.run([b, '--version'], capture_output=True, text=True).stdout.strip()
    print(f'# indent-twip-rounding: an OOXML indent of {LEVEL} levels ='
          f' {3 * LEVEL} spaces of the default font. Measured ' + subprocess.run(
              ['date', '-u', '+%Y-%m-%d'], capture_output=True, text=True).stdout.strip())
    for label, binary in targets.items():
        print(f'# {label}: {binary} -> {version(binary)}')
    print('# fonts: system /usr/share/fonts, the tarball\'s Latin duplicates and Latin Noto aside')
    print('# fc-match Arial -> ' + subprocess.run(
        ['fc-match', FACE], capture_output=True, text=True).stdout.strip())
    if cli:
        print(f'# ours: {cli}')
    print('binary\tsize_pt\tspace_twips\tfloor_pt\tround_pt\tmeasured_pt\tmatches')
    for size in SIZES:
        book = os.path.join(out, f'indent-{size}.xlsx')
        build(book, size)
        space = SPACE_EM * size * 20.0
        floor_pt = 3 * LEVEL * math.floor(space) / 20.0
        round_pt = 3 * LEVEL * round(space) / 20.0
        for label, binary in list(targets.items()) + ([('ours', cli)] if cli else []):
            directory = os.path.join(out, label)
            os.makedirs(directory, exist_ok=True)
            if label == 'ours':
                subprocess.run([binary, 'render', '--quiet', '--outdir', directory, book],
                               check=True)
            else:
                subprocess.run([binary, f'-env:UserInstallation=file://{directory}/profile',
                                '--headless', '--convert-to', 'pdf', '--outdir', directory, book],
                               capture_output=True, check=True)
            pdf = os.path.join(directory, f'indent-{size}.pdf')
            if not os.path.isfile(pdf):
                raise SystemExit(f'{label} wrote no PDF for {size} pt -- nothing to compare')
            found = pens(pdf)
            if 'Zero' not in found or 'Ind' not in found:
                raise SystemExit(f'{label} at {size} pt drew {sorted(found)} -- expected both cells')
            measured = found['Ind'] - found['Zero']
            matches = ('floor' if abs(measured - floor_pt) < 0.05 else
                       'round' if abs(measured - round_pt) < 0.05 else 'neither')
            if abs(floor_pt - round_pt) < 1e-9:
                matches += ' (undecided)'
            print(f'{label}\t{size}\t{space:.3f}\t{floor_pt:.3f}\t{round_pt:.3f}\t'
                  f'{measured:.3f}\t{matches}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
