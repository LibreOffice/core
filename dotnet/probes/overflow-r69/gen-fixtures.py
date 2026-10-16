#!/usr/bin/env python3
"""Build the two fixtures round 69 added to `tests/corpus/features`.

Both are minimal by design: one thing each, and nothing else on the sheet that could
move what the test measures.

  sheet-print-centred-overflow.xlsx
      Two sheets under `printOptions/@horizontalCentered`. `wide` is a single column far
      wider than the printable area — a single column cannot be split across page columns,
      so the block overflows rather than paginating — and `narrow` is a single column that
      fits. Each holds one centred marker string, so the drawn run's own centre is the
      column's centre and therefore the block's, with no char-width conversion in the way.

  sheet-shape-line-separator.xlsx
      One sheet, two text boxes, one 12 pt Liberation Sans run each. The first states
      `AAA\nBBB` inside a single `a:t` and the second `CCC\n\nDDD`, so the second's gap is
      the first's plus one empty paragraph. Boxes wide enough not to wrap and tall enough
      not to clip.
"""
import zipfile, sys

CT_HEAD = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
 '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
 '<Default Extension="xml" ContentType="application/xml"/>'
 '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
 '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>')

STYLES = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
 '<fonts count="1"><font><sz val="11"/><name val="Liberation Sans"/></font></fonts>'
 '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
 '<borders count="1"><border/></borders>'
 '<cellStyleXfs count="1"><xf/></cellStyleXfs>'
 '<cellXfs count="2"><xf/>'
 '<xf applyAlignment="1"><alignment horizontal="center"/></xf></cellXfs>'
 '</styleSheet>')

def write(out, parts):
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        for n, c in parts.items():
            z.writestr(n, c)
    print(out)


def centred(out):
    def sheet(width, marker):
        return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
         '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
         f'<cols><col min="1" max="1" width="{width}" customWidth="1"/></cols>'
         f'<sheetData><row r="1"><c r="A1" s="1" t="inlineStr"><is><t>{marker}</t></is></c></row></sheetData>'
         '<printOptions horizontalCentered="1"/>'
         '<pageMargins left="0.7" right="0.7" top="0.75" bottom="0.75" header="0.3" footer="0.3"/>'
         '<pageSetup orientation="portrait" paperSize="1"/></worksheet>')

    parts = {
     '[Content_Types].xml': CT_HEAD +
       '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
       '<Override PartName="/xl/worksheets/sheet2.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
       '</Types>',
     '_rels/.rels':
       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
       '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>'
       '</Relationships>',
     'xl/workbook.xml':
       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"'
       ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
       '<sheets><sheet name="wide" sheetId="1" r:id="rId1"/>'
       '<sheet name="narrow" sheetId="2" r:id="rId2"/></sheets></workbook>',
     'xl/_rels/workbook.xml.rels':
       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
       '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>'
       '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet2.xml"/>'
       '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
       '</Relationships>',
     'xl/styles.xml': STYLES,
     'xl/worksheets/sheet1.xml': sheet('160', 'WIDEBLOCK'),
     'xl/worksheets/sheet2.xml': sheet('20', 'NARROWBLOCK'),
    }
    write(out, parts)


def separator(out):
    def box(i, col, text):
        return (f'<xdr:twoCellAnchor editAs="oneCell">'
         f'<xdr:from><xdr:col>{col}</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>0</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>'
         f'<xdr:to><xdr:col>{col+4}</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>20</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>'
         f'<xdr:sp macro="" textlink=""><xdr:nvSpPr><xdr:cNvPr id="{i+2}" name="box{i}"/><xdr:cNvSpPr txBox="1"/></xdr:nvSpPr>'
         '<xdr:spPr><a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></xdr:spPr>'
         '<xdr:txBody><a:bodyPr wrap="square"/><a:lstStyle/><a:p><a:r>'
         '<a:rPr lang="en-US" sz="1200"><a:latin typeface="Liberation Sans"/></a:rPr>'
         f'<a:t>{text}</a:t></a:r></a:p></xdr:txBody></xdr:sp><xdr:clientData/></xdr:twoCellAnchor>')

    drawing = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
     '<xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"'
     ' xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
     + box(0, 0, 'AAA\nBBB') + box(1, 6, 'CCC\n\nDDD') + '</xdr:wsDr>')

    sheet = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
     '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"'
     ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
     '<sheetData/>'
     '<pageMargins left="0.25" right="0.25" top="0.25" bottom="0.25" header="0" footer="0"/>'
     '<pageSetup orientation="landscape" paperSize="1"/>'
     '<drawing r:id="rId1"/></worksheet>')

    parts = {
     '[Content_Types].xml': CT_HEAD +
       '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
       '<Override PartName="/xl/drawings/drawing1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawing+xml"/>'
       '</Types>',
     '_rels/.rels':
       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
       '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>'
       '</Relationships>',
     'xl/workbook.xml':
       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"'
       ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
       '<sheets><sheet name="boxes" sheetId="1" r:id="rId1"/></sheets></workbook>',
     'xl/_rels/workbook.xml.rels':
       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
       '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>'
       '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
       '</Relationships>',
     'xl/styles.xml': STYLES,
     'xl/worksheets/sheet1.xml': sheet,
     'xl/worksheets/_rels/sheet1.xml.rels':
       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
       '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing" Target="../drawings/drawing1.xml"/>'
       '</Relationships>',
     'xl/drawings/drawing1.xml': drawing,
    }
    write(out, parts)


if __name__ == '__main__':
    into = sys.argv[1].rstrip('/')
    centred(into + '/sheet-print-centred-overflow.xlsx')
    separator(into + '/sheet-shape-line-separator.xlsx')
