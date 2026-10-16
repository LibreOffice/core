#!/usr/bin/env python3
"""Author minimal .xlsx workbooks for the 24.2.7.2 re-check probes."""
import os, zipfile

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"

def workbook(path, *, font, size, cols, rows):
    """cols: [(min,max,width)]; rows: [(rowidx, [(colletter, kind, value)])]"""
    colxml = "".join(
        f'<col min="{a}" max="{b}" width="{w}" customWidth="1"/>' for a, b, w in cols)
    rowxml = ""
    for r, cells in rows:
        cs = ""
        for ref, kind, value in cells:
            if kind == "s":
                cs += f'<c r="{ref}{r}" t="inlineStr"><is><t>{value}</t></is></c>'
            else:
                cs += f'<c r="{ref}{r}"><v>{value}</v></c>'
        rowxml += f'<row r="{r}">{cs}</row>'

    styles = (
        f'<styleSheet xmlns="{NS}">'
        f'<fonts count="1"><font><sz val="{size}"/><name val="{font}"/></font></fonts>'
        '<fills count="2"><fill><patternFill patternType="none"/></fill>'
        '<fill><patternFill patternType="gray125"/></fill></fills>'
        '<borders count="1"><border/></borders>'
        '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
        '<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/></cellXfs>'
        '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>'
        '<dxfs count="0"/>'
        '</styleSheet>')

    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml",
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
            '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
            '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
            '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
            '</Types>')
        z.writestr("_rels/.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/officeDocument" Target="xl/workbook.xml"/>'
            '</Relationships>')
        z.writestr("xl/_rels/workbook.xml.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/worksheet" Target="worksheets/sheet1.xml"/>'
            f'<Relationship Id="rId2" Type="{RNS}/styles" Target="styles.xml"/>'
            '</Relationships>')
        z.writestr("xl/workbook.xml",
            f'<workbook xmlns="{NS}" xmlns:r="{RNS}"><sheets>'
            '<sheet name="Probe" sheetId="1" r:id="rId1"/></sheets></workbook>')
        z.writestr("xl/styles.xml", styles)
        z.writestr("xl/worksheets/sheet1.xml",
            f'<worksheet xmlns="{NS}" xmlns:r="{RNS}">'
            f'<cols>{colxml}</cols><sheetData>{rowxml}</sheetData></worksheet>')
    return path
