import zipfile, sys, os

# A worksheet with N text boxes, each a rect wide enough that the text wraps to
# several lines, at a stated size and face. No print scale, so the drawn size is
# the stated one and a baseline pitch is directly readable.
CASES = []
for face in ["Liberation Serif", "Liberation Sans", "DejaVu Sans", "Carlito"]:
    for size in [8, 11, 16, 24]:
        CASES.append((face, size))

WORDS = "alpha bravo charlie delta echo foxtrot golf hotel india juliett kilo lima mike"

def emu_cm(x): return int(x * 360000)

shapes = []
for i, (face, size) in enumerate(CASES):
    col = (i % 4) * 5
    row = (i // 4) * 10
    shapes.append(f'''<xdr:twoCellAnchor editAs="oneCell">
<xdr:from><xdr:col>{col}</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>{row}</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>
<xdr:to><xdr:col>{col+4}</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>{row+9}</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>
<xdr:sp macro="" textlink=""><xdr:nvSpPr><xdr:cNvPr id="{i+2}" name="t{i}"/><xdr:cNvSpPr txBox="1"/></xdr:nvSpPr>
<xdr:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="1000000" cy="1000000"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></xdr:spPr>
<xdr:txBody><a:bodyPr wrap="square"/><a:lstStyle/><a:p><a:r><a:rPr lang="en-US" sz="{size*100}"><a:latin typeface="{face}"/></a:rPr><a:t>{WORDS}</a:t></a:r></a:p></xdr:txBody>
</xdr:sp><xdr:clientData/></xdr:twoCellAnchor>''')

drawing = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"'
 ' xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">' + "".join(shapes) + '</xdr:wsDr>')

sheet = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"'
 ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
 '<sheetData/><pageMargins left="0.25" right="0.25" top="0.25" bottom="0.25" header="0" footer="0"/>'
 '<pageSetup orientation="landscape" paperSize="9"/>'
 '<drawing r:id="rId1"/></worksheet>')

parts = {
 '[Content_Types].xml':
   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
   '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
   '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
   '<Default Extension="xml" ContentType="application/xml"/>'
   '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
   '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
   '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
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
   '<sheets><sheet name="probe" sheetId="1" r:id="rId1"/></sheets></workbook>',
 'xl/_rels/workbook.xml.rels':
   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>'
   '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
   '</Relationships>',
 'xl/styles.xml':
   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
   '<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
   '<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>'
   '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
   '<borders count="1"><border/></borders>'
   '<cellStyleXfs count="1"><xf/></cellStyleXfs><cellXfs count="1"><xf/></cellXfs></styleSheet>',
 'xl/worksheets/sheet1.xml': sheet,
 'xl/worksheets/_rels/sheet1.xml.rels':
   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing" Target="../drawings/drawing1.xml"/>'
   '</Relationships>',
 'xl/drawings/drawing1.xml': drawing,
}
out = sys.argv[1]
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED) as z:
    for n,c in parts.items(): z.writestr(n,c)
print(out, len(CASES), 'boxes')
