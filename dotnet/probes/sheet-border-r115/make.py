#!/usr/bin/env python3
"""Minimal .xlsx with one border style per cell, at a stated print scale."""
import sys, zipfile

STYLES = ["hair","thin","medium","thick","double","dotted","dashed",
          "dashDot","dashDotDot","mediumDashed","mediumDashDot",
          "mediumDashDotDot","slantDashDot"]

def build(path, scale):
    borders = ['<border><left/><right/><top/><bottom/><diagonal/></border>']
    for s in STYLES:
        borders.append(
            f'<border><left style="{s}"><color rgb="FF000000"/></left>'
            f'<right style="{s}"><color rgb="FF000000"/></right>'
            f'<top style="{s}"><color rgb="FF000000"/></top>'
            f'<bottom style="{s}"><color rgb="FF000000"/></bottom>'
            f'<diagonal/></border>')
    xfs = ['<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>']
    for i in range(1, len(borders)):
        xfs.append(f'<xf numFmtId="0" fontId="0" fillId="0" borderId="{i}" xfId="0" applyBorder="1"/>')
    styles = (
      '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
      '<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>'
      '<fills count="2"><fill><patternFill patternType="none"/></fill>'
      '<fill><patternFill patternType="gray125"/></fill></fills>'
      f'<borders count="{len(borders)}">' + ''.join(borders) + '</borders>'
      '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
      f'<cellXfs count="{len(xfs)}">' + ''.join(xfs) + '</cellXfs>'
      '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>'
      '</styleSheet>')

    rows = []
    for i, s in enumerate(STYLES):
        r = 2 + i * 2
        rows.append(f'<row r="{r}"><c r="B{r}" s="{i+1}"/></row>')
    sc = f' scale="{scale}"' if scale else ''
    sheet = (
      '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
      '<dimension ref="A1:D30"/><sheetViews><sheetView workbookViewId="0"/></sheetViews>'
      '<sheetFormatPr defaultRowHeight="15"/>'
      '<cols><col min="1" max="4" width="12" customWidth="1"/></cols>'
      '<sheetData>' + ''.join(rows) + '</sheetData>'
      f'<pageSetup orientation="portrait"{sc}/>'
      '</worksheet>')

    wb = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
      '<sheets><sheet name="S" sheetId="1" r:id="rId1"/></sheets></workbook>')
    wbrels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
      '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>'
      '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
      '</Relationships>')
    ct = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
      '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
      '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
      '</Types>')
    rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
      '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>'
      '</Relationships>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ct)
        z.writestr('_rels/.rels', rels)
        z.writestr('xl/workbook.xml', wb)
        z.writestr('xl/_rels/workbook.xml.rels', wbrels)
        z.writestr('xl/styles.xml', styles)
        z.writestr('xl/worksheets/sheet1.xml', sheet)

if __name__ == '__main__':
    for s in (None, 100, 75, 50, 42, 25, 10, 200, 400):
        build(f"borders-{s or 'none'}.xlsx", s)
    print("ok")
