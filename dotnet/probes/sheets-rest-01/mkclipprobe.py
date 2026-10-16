#!/usr/bin/env python3
"""Build an XLSX probe of cells too wide for their column, with the neighbour occupied.

Every `cellXf` states `applyFont="1"` and the workbook carries a `cellStyles`, because the
first cut of this probe omitted both: `fontId="0"` with no flag left LibreOffice using its own
application default of Liberation Sans 10 for the plain rows while the rich rows took the face
their `rPr` named, so plain and rich were never compared in the same font at all.

`ScOutputData::LayoutStrings` sends a cell to `DrawEdit` rather than to `DrawStrings` when
`aCell.getType() == CELLTYPE_EDIT` (`sc/source/ui/view/output2.cxx:1710-1712`), and the two
paths disagree about a string that does not fit: `DrawStrings` shortens it to what the clip
rectangle can show, `DrawEdit` keeps every character and clips the ink. Only the second
leaves the hidden tail in the PDF's text layer, which is the half `pdftotext` scores.

Which cells are `CELLTYPE_EDIT` is the question. Each row here differs in one thing only:

  1  plain     a plain shared string
  2  rich      the same string with a formatting run over its first word
  3  break     the same string with a hard line break in it, cell not wrapping
  4  richbreak both
  5  plainwrap the plain string in a cell that wraps

Column B is occupied throughout, so nothing may spill; the whole difference has to be the
path. Read the answer off `pdftotext`, not off the image — the clip hides the tail either way.
"""
import os
import sys
import zipfile

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

WBRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>
</Relationships>'''

WB = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="S1" sheetId="1" r:id="rId1"/></sheets>
</workbook>'''

STYLES_TEMPLATE = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="%(pt)s"/><name val="%(face)s"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="2">
<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/>
<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1" applyAlignment="1"><alignment wrapText="1"/></xf>
</cellXfs>
<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
</styleSheet>'''

# The face and size matter, and the first cut of this probe got that wrong: at Arial 9 every
# row came back at 12.82 pt — `ScGlobal::nStdRowHeight`, 256 twips — so the floor hid the very
# difference the probe was built to show. Anything whose line is taller than the floor works;
# Calibri 11 is what `SIL_TDB648.xlsx` uses and is the reason for the default.
BODY = ('Run the following command to enable the tmp mount service '
        'systemctl enable tmp.mount Ensure the proper settings for your tmp mount')

def _cases(face, pt):
    """The five rows, with the rich runs set in the face the probe is parameterised on."""
    rpr = '<rPr><sz val="%s"/><rFont val="%s"/></rPr>' % (pt, face)
    bold = '<rPr><b/><sz val="%s"/><rFont val="%s"/></rPr>' % (pt, face)
    broken = BODY[:20] + '&#10;' + BODY[20:]

    def plain(text):
        return '<si><t xml:space="preserve">%s</t></si>' % text

    def rich(text):
        return ('<si><r>%s<t xml:space="preserve">Run</t></r>'
                '<r>%s<t xml:space="preserve">%s</t></r></si>' % (bold, rpr, text[3:]))

    return [
        ('plain', plain(BODY), 0),
        ('rich', rich(BODY), 0),
        ('break', plain(broken), 0),
        ('richbreak', rich(broken), 0),
        ('plainwrap', plain(BODY), 1),
    ]


def build(path, stated_height=True, face='Calibri', pt='11'):
    styles = STYLES_TEMPLATE % {'face': face, 'pt': pt}
    cases = _cases(face, pt)
    sst = ''.join(x for _, x, _ in cases) + '<si><t>X</t></si>'
    sstxml = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
              'count="%d" uniqueCount="%d">%s</sst>'
              % (2 * len(cases), len(cases) + 1, sst))

    # With `customHeight` the row is a statement and is honoured; without it the row is a hint
    # and Calc recomputes it, which is the second half of the same question — does an edit cell
    # get EditEngine's line height rather than the arithmetic one?
    height = ' ht="12" customHeight="1"' if stated_height else ''
    rows = ''.join(
        '<row r="%d"%s><c r="A%d" s="%d" t="s"><v>%d</v></c>'
        '<c r="B%d" t="s"><v>%d</v></c></row>'
        % (i + 1, height, i + 1, style, i, i + 1, len(cases))
        for i, (_, _, style) in enumerate(cases))

    sheet = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
             '<cols><col min="1" max="1" width="20" customWidth="1"/>'
             '<col min="2" max="2" width="20" customWidth="1"/></cols>'
             '<sheetData>%s</sheetData>'
             '<pageSetup paperSize="9" orientation="landscape"/></worksheet>' % rows)

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('xl/workbook.xml', WB)
        z.writestr('xl/_rels/workbook.xml.rels', WBRELS)
        z.writestr('xl/styles.xml', styles)
        z.writestr('xl/sharedStrings.xml', sstxml)
        z.writestr('xl/worksheets/sheet1.xml', sheet)


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    build(os.path.join(out, 'clipprobe.xlsx'))
    build(os.path.join(out, 'clipprobe-auto.xlsx'), stated_height=False)
    for i, (name, _, style) in enumerate(_cases('Calibri', '11')):
        print('%d\t%s\twrap=%d' % (i + 1, name, style))
