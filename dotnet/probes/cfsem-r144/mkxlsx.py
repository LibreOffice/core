"""Minimal .xlsx builder for conditional-format / formula probes.

Builds a single-sheet workbook by hand (zipfile + a few XML parts) so that each
probe can isolate one arm of the semantics question.
"""
import zipfile, re

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>"""

WBRELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>"""

def esc(s):
    return s.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;').replace('"', '&quot;')

def styles(dxf_colors):
    """dxf_colors: list of ARGB strings -> one solid-fill dxf each."""
    dxfs = "".join(
        '<dxf><fill><patternFill patternType="solid">'
        '<fgColor rgb="%s"/><bgColor rgb="%s"/></patternFill></fill></dxf>' % (c, c)
        for c in dxf_colors)
    return """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="2"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill></fills>
<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>
<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
<dxfs count="%d">%s</dxfs>
</styleSheet>""" % (len(dxf_colors), dxfs)

def colname(n):
    s = ""
    while n > 0:
        n, r = divmod(n - 1, 26)
        s = chr(65 + r) + s
    return s

def workbook(names, sheetname="Sheet1"):
    dn = ""
    if names:
        dn = "<definedNames>" + "".join(
            '<definedName name="%s">%s</definedName>' % (n, esc(f)) for n, f in names) + "</definedNames>"
    return """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="%s" sheetId="1" r:id="rId1"/></sheets>%s</workbook>""" % (sheetname, dn)

def sheet(cells, cfs=(), dim="A1:Z100"):
    """cells: dict 'A1' -> value.  number -> <v>, str starting '=' -> formula,
    other str -> inline string.  cfs: list of (sqref, [formula, ...]) with dxfId
    allocated in listed order across all cfs."""
    rows = {}
    for ref, val in cells.items():
        m = re.match(r'([A-Z]+)(\d+)$', ref)
        rows.setdefault(int(m.group(2)), []).append((m.group(1), ref, val))
    out = []
    for r in sorted(rows):
        cs = sorted(rows[r], key=lambda t: (len(t[0]), t[0]))
        body = ""
        for _, ref, val in cs:
            if isinstance(val, str) and val.startswith('='):
                body += '<c r="%s"><f>%s</f></c>' % (ref, esc(val[1:]))
            elif isinstance(val, str):
                body += '<c r="%s" t="inlineStr"><is><t>%s</t></is></c>' % (ref, esc(val))
            else:
                body += '<c r="%s"><v>%s</v></c>' % (ref, val)
        out.append('<row r="%d">%s</row>' % (r, body))
    cfxml = ""
    dxf = 0
    prio = 1
    for sqref, formulas in cfs:
        rules = ""
        for f in formulas:
            rules += ('<cfRule type="expression" dxfId="%d" priority="%d"><formula>%s</formula></cfRule>'
                      % (dxf, prio, esc(f)))
            dxf += 1
            prio += 1
        cfxml += '<conditionalFormatting sqref="%s">%s</conditionalFormatting>' % (sqref, rules)
    return """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="%s"/><sheetData>%s</sheetData>%s</worksheet>""" % (dim, "".join(out), cfxml)

def build(path, cells, names=(), cfs=(), dxf_colors=(), sheetname="Sheet1", dim="A1:Z100"):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('xl/_rels/workbook.xml.rels', WBRELS)
        z.writestr('xl/workbook.xml', workbook(names, sheetname))
        z.writestr('xl/styles.xml', styles(list(dxf_colors)))
        z.writestr('xl/worksheets/sheet1.xml', sheet(cells, cfs, dim))
    return path
