#!/usr/bin/env python3
"""Authors `table-style-bands.docx` and reads 26.2.4.2's own answer for it back out.

Six rows by three columns under one table style that states a `w:tcPr/w:shd` on four layers at
once — `firstRow`, `firstCol`, `band1Horz` and `band2Horz` — with `w:tblStyleRowBandSize="2"`, so
the fixture separates four rules that a one-layer fixture cannot:

  * a heading row takes `firstRow` and **no band at all**, which is what fixes that a row inside an
    edge region is excluded from the band count rather than counted as band nought;
  * with a band size of two, body rows pair up — rows 2-3 in band 1 and rows 4-5 in band 2 — so a
    reader that ignores `w:tblStyleRowBandSize` gets four rows wrong out of five;
  * the leading column takes `firstCol` over either band, because an edge layer is more specific;
  * a cell stating its own `w:shd` beats every layer, and the fixture's last cell does.

Every colour is distinct, so the reference's own fill operators name which layer won, cell by cell.

    make-band-fixture.py <outdir>
"""
import os
import subprocess
import sys
import zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

STYLES = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="{W}">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/><w:szCs w:val="24"/>
</w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr></w:pPrDefault>
</w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
<w:style w:type="table" w:styleId="BandedCells">
  <w:name w:val="Banded Cells"/>
  <w:tblPr><w:tblStyleRowBandSize w:val="2"/><w:tblStyleColBandSize w:val="1"/></w:tblPr>
  <w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="EDEDED"/></w:tcPr>
  <w:tblStylePr w:type="firstRow"><w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="4472C4"/></w:tcPr></w:tblStylePr>
  <w:tblStylePr w:type="firstCol"><w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="FFF2CC"/></w:tcPr></w:tblStylePr>
  <w:tblStylePr w:type="band1Horz"><w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="D9E2F3"/></w:tcPr></w:tblStylePr>
  <w:tblStylePr w:type="band2Horz"><w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="FBE4D5"/></w:tcPr></w:tblStylePr>
</w:style>
</w:styles>
'''


def cell(text, own=''):
    shd = f'<w:shd w:val="clear" w:color="auto" w:fill="{own}"/>' if own else ''
    return (f'<w:tc><w:tcPr><w:tcW w:w="2600" w:type="dxa"/>{shd}</w:tcPr>'
            f'<w:p><w:r><w:t>{text}</w:t></w:r></w:p></w:tc>')


def document():
    rows = []
    for r in range(6):
        cells = []
        for c in range(3):
            name = 'R%dC%d' % (r, c)
            own = '00B0F0' if (r, c) == (5, 2) else ''
            cells.append(cell(name, own))
        rows.append('<w:tr>%s</w:tr>' % ''.join(cells))
    return f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="{W}"><w:body>
<w:tbl>
<w:tblPr><w:tblStyle w:val="BandedCells"/><w:tblW w:w="0" w:type="auto"/>
<w:tblLook w:val="0480" w:firstRow="1" w:lastRow="0" w:firstColumn="1" w:lastColumn="0"
           w:noHBand="0" w:noVBand="1"/></w:tblPr>
<w:tblGrid><w:gridCol w:w="2600"/><w:gridCol w:w="2600"/><w:gridCol w:w="2600"/></w:tblGrid>
{''.join(rows)}
</w:tbl>
<w:p/>
<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720"/>
</w:sectPr></w:body></w:document>
'''


RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>
'''
DOCRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>
'''
TYPES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>
'''


def main(outdir):
    os.makedirs(outdir, exist_ok=True)
    path = os.path.join(outdir, 'table-style-bands.docx')
    # A fixed date on every entry, so re-running this script reproduces the package byte for byte
    # and a committed fixture never shows up as changed for having been regenerated.
    stamp = (2026, 1, 1, 0, 0, 0)
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, text in [('[Content_Types].xml', TYPES), ('_rels/.rels', RELS),
                           ('word/_rels/document.xml.rels', DOCRELS),
                           ('word/styles.xml', STYLES), ('word/document.xml', document())]:
            z.writestr(zipfile.ZipInfo(name, stamp), text)
    print('wrote', path)

    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000', TZ='UTC')
    subprocess.run(['soffice', '--headless',
                    '-env:UserInstallation=file://%s' % os.path.join(outdir, 'prof'),
                    '--convert-to', 'pdf', '--outdir', outdir, path],
                   check=True, capture_output=True, env=env, timeout=600)
    here = os.path.dirname(os.path.abspath(__file__))
    subprocess.run([sys.executable, os.path.join(here, 'fillcount.py'),
                    os.path.join(outdir, 'table-style-bands.pdf'), '1'], check=True)


if __name__ == '__main__':
    main(sys.argv[1])
