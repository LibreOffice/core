import zipfile, os, sys

NS = ('xmlns:wpc="http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas" '
      'xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" '
      'xmlns:o="urn:schemas-microsoft-com:office:office" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math" '
      'xmlns:v="urn:schemas-microsoft-com:vml" '
      'xmlns:wp14="http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing" '
      'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
      'xmlns:w10="urn:schemas-microsoft-com:office:word" '
      'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" '
      'xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" '
      'xmlns:wpi="http://schemas.microsoft.com/office/word/2010/wordprocessingInk" '
      'xmlns:wne="http://schemas.microsoft.com/office/word/2006/wordml" '
      'xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape"')

FIELD_PARA = (
 '<w:p><w:pPr><w:jc w:val="right"/></w:pPr>'
 '<w:r><w:t xml:space="preserve">Page </w:t></w:r>'
 '<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
 '<w:r><w:instrText>PAGE</w:instrText></w:r>'
 '<w:r><w:fldChar w:fldCharType="separate"/></w:r>'
 '<w:r><w:t>2</w:t></w:r>'
 '<w:r><w:fldChar w:fldCharType="end"/></w:r>'
 '<w:r><w:t xml:space="preserve"> of </w:t></w:r>'
 '<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
 '<w:r><w:instrText>NUMPAGES</w:instrText></w:r>'
 '<w:r><w:fldChar w:fldCharType="separate"/></w:r>'
 '<w:r><w:t>4</w:t></w:r>'
 '<w:r><w:fldChar w:fldCharType="end"/></w:r>'
 '</w:p>')

WSP = (
 '<wps:wsp><wps:cNvPr id="2" name="Text Box 2"/><wps:cNvSpPr txBox="1"/>'
 '<wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="2743200" cy="304800"/></a:xfrm>'
 '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/><a:ln><a:noFill/></a:ln></wps:spPr>'
 '<wps:txbx><w:txbxContent>' + FIELD_PARA + '</w:txbxContent></wps:txbx>'
 '<wps:bodyPr rot="0" vert="horz" wrap="square" lIns="0" tIns="0" rIns="0" bIns="0" '
 'anchor="t" anchorCtr="0"><a:noAutofit/></wps:bodyPr></wps:wsp>')

def drawing(grouped):
    if grouped:
        uri = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup"
        inner = ('<wpg:wgp><wpg:cNvGrpSpPr/><wpg:grpSpPr>'
                 '<a:xfrm><a:off x="0" y="0"/><a:ext cx="2743200" cy="304800"/>'
                 '<a:chOff x="0" y="0"/><a:chExt cx="2743200" cy="304800"/></a:xfrm>'
                 '</wpg:grpSpPr>' + WSP + '</wpg:wgp>')
        name = "Group 1"
    else:
        uri = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape"
        inner = WSP
        name = "Text Box 1"
    return ('<w:r><w:drawing><wp:anchor distT="0" distB="0" distL="114300" distR="114300" '
            'simplePos="0" relativeHeight="251659264" behindDoc="0" locked="0" '
            'layoutInCell="1" allowOverlap="1">'
            '<wp:simplePos x="0" y="0"/>'
            '<wp:positionH relativeFrom="column"><wp:posOffset>0</wp:posOffset></wp:positionH>'
            '<wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>'
            '<wp:extent cx="2743200" cy="304800"/>'
            '<wp:effectExtent l="0" t="0" r="0" b="0"/><wp:wrapNone/>'
            '<wp:docPr id="1" name="%s"/><wp:cNvGraphicFramePr/>'
            '<a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
            '<a:graphicData uri="%s">%s</a:graphicData></a:graphic>'
            '</wp:anchor></w:drawing></w:r>' % (name, uri, inner))

TBL_OPEN = ('<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/><w:tblLayout w:type="fixed"/>'
            '<w:tblLook w:val="04A0"/></w:tblPr><w:tblGrid><w:gridCol w:w="9639"/></w:tblGrid>'
            '<w:tr><w:tc><w:tcPr><w:tcW w:w="9639" w:type="dxa"/></w:tcPr>')
TBL_CLOSE = '</w:tc></w:tr></w:tbl><w:p/>'

def footer(grouped, intable):
    para = '<w:p>' + drawing(grouped) + '</w:p>'
    body = (TBL_OPEN + para + TBL_CLOSE) if intable else para
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<w:ftr %s>%s</w:ftr>' % (NS, body))

DOC_BODY = ''.join(
    '<w:p><w:r><w:t>Body page %d, line %d.</w:t></w:r></w:p>' % (p, l)
    for p in range(1, 5) for l in range(1, 4))
# insert page breaks between the four pages
paras = []
for p in range(1, 5):
    for l in range(1, 4):
        paras.append('<w:p><w:r><w:t>Body page %d, line %d.</w:t></w:r></w:p>' % (p, l))
    if p < 4:
        paras.append('<w:p><w:r><w:br w:type="page"/></w:r></w:p>')
DOC_BODY = ''.join(paras)

DOCUMENT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
  '<w:document %s><w:body>%s'
  '<w:sectPr><w:footerReference w:type="default" r:id="rId1"/>'
  '<w:pgSz w:w="11906" w:h="16838"/>'
  '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" '
  'w:header="709" w:footer="709" w:gutter="0"/>'
  '<w:cols w:space="708"/><w:docGrid w:linePitch="360"/></w:sectPr>'
  '</w:body></w:document>' % (NS, DOC_BODY))

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
 '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
 '<Default Extension="xml" ContentType="application/xml"/>'
 '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
 '<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>'
 '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>'
 '<Override PartName="/word/fontTable.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml"/>'
 '<Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>'
 '<Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>'
 '<Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>'
 '</Types>')

RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
 '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>'
 '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>'
 '</Relationships>')

DOCRELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer" Target="footer1.xml"/>'
 '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
 '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>'
 '<Relationship Id="rId4" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable" Target="fontTable.xml"/>'
 '</Relationships>')

SETTINGS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<w:settings %s>'
 '<w:zoom w:percent="100"/><w:defaultTabStop w:val="708"/>'
 '<w:characterSpacingControl w:val="doNotCompress"/>'
 '<w:compat><w:compatSetting w:name="compatibilityMode" '
 'w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>'
 '</w:settings>' % NS)

STYLES = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<w:styles %s>'
 '<w:docDefaults><w:rPrDefault><w:rPr>'
 '<w:rFonts w:ascii="Calibri" w:hAnsi="Calibri" w:cs="Times New Roman"/>'
 '<w:sz w:val="22"/><w:szCs w:val="22"/><w:lang w:val="en-GB"/></w:rPr></w:rPrDefault>'
 '<w:pPrDefault><w:pPr><w:spacing w:after="160" w:line="259" w:lineRule="auto"/></w:pPr></w:pPrDefault>'
 '</w:docDefaults>'
 '<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>'
 '<w:qFormat/></w:style>'
 '<w:style w:type="paragraph" w:styleId="Footer"><w:name w:val="footer"/>'
 '<w:basedOn w:val="Normal"/><w:pPr><w:spacing w:after="0" w:line="240" w:lineRule="auto"/></w:pPr>'
 '</w:style>'
 '</w:styles>' % NS)

FONTTABLE = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<w:fonts %s><w:font w:name="Calibri"><w:charset w:val="00"/>'
 '<w:family w:val="swiss"/><w:pitch w:val="variable"/></w:font></w:fonts>' % NS)

CORE = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" '
 'xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" '
 'xmlns:dcmitype="http://purl.org/dc/dcmitype/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">'
 '<dc:title>pagefield probe</dc:title></cp:coreProperties>')

APP = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" '
 'xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">'
 '<Pages>4</Pages><Words>48</Words></Properties>')

def build(path, grouped, intable):
    z = zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED)
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/document.xml', DOCUMENT)
    z.writestr('word/_rels/document.xml.rels', DOCRELS)
    z.writestr('word/settings.xml', SETTINGS)
    z.writestr('word/styles.xml', STYLES)
    z.writestr('word/fontTable.xml', FONTTABLE)
    z.writestr('word/footer1.xml', footer(grouped, intable))
    z.writestr('docProps/core.xml', CORE)
    z.writestr('docProps/app.xml', APP)
    z.close()
    print('built', path)

out = sys.argv[1]
build(os.path.join(out, 'syn-plain-para.docx'),  False, False)
build(os.path.join(out, 'syn-plain-table.docx'), False, True)
build(os.path.join(out, 'syn-group-para.docx'),  True,  False)
build(os.path.join(out, 'syn-group-table.docx'), True,  True)
