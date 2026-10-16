import zipfile, os

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>'''
RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''
DRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>'''
def styles(after):
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
     '<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
     '<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Liberation Sans" w:hAnsi="Liberation Sans"/>'
     '<w:sz w:val="20"/></w:rPr></w:rPrDefault><w:pPrDefault><w:pPr/></w:pPrDefault></w:docDefaults>'
     f'<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:pPr><w:spacing w:after="{after}"/></w:pPr>'
     '<w:rPr><w:rFonts w:ascii="Liberation Sans" w:hAnsi="Liberation Sans"/><w:sz w:val="20"/></w:rPr></w:style></w:styles>')
SETTINGS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
 '<w:compat><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>'
 '</w:settings>')
SECT = ('<w:pgSz w:w="11907" w:h="16840"/>'
        '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="709" w:footer="709" w:gutter="0"/>')

def pack(path, body, after):
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body>'
           + body + '</w:body></w:document>')
    z = zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED)
    z.writestr('[Content_Types].xml', CT); z.writestr('_rels/.rels', RELS)
    z.writestr('word/_rels/document.xml.rels', DRELS)
    z.writestr('word/styles.xml', styles(after)); z.writestr('word/settings.xml', SETTINGS)
    z.writestr('word/document.xml', doc); z.close()

filler = ''.join(f'<w:p><w:r><w:t>Filler line {i}</w:t></w:r></w:p>' for i in range(3))

# 1. Space-before survives a nextPage section break at compatibility mode 15.
#    Normal states 10 pt of space-after; the heading states 20 pt of space-before.
pack('section-break-top-spacing.docx',
     filler
     + f'<w:p><w:pPr><w:sectPr w:type="nextPage">{SECT}</w:sectPr></w:pPr></w:p>'
     + '<w:p><w:pPr><w:spacing w:before="400"/></w:pPr><w:r><w:t>TARGET</w:t></w:r></w:p>'
     + f'<w:p><w:pPr><w:sectPr>{SECT}</w:sectPr></w:pPr></w:p>',
     after=200)

# 2. The discarded section mark's own space-after is what the next section collapses against.
#    Normal states none; the mark states 12 pt; the heading states 20 pt. 20 - 12 = 8 pt.
pack('section-mark-below-spacing.docx',
     filler
     + f'<w:p><w:pPr><w:spacing w:after="240"/><w:sectPr w:type="nextPage">{SECT}</w:sectPr></w:pPr></w:p>'
     + '<w:p><w:pPr><w:spacing w:before="400"/></w:pPr><w:r><w:t>TARGET</w:t></w:r></w:p>'
     + f'<w:p><w:pPr><w:sectPr>{SECT}</w:sectPr></w:pPr></w:p>',
     after=0)

# 3. Keep-with-next is ignored when the successor opens a section of its own.
#    The successor's line is 720 pt tall and can never share a page with anything.
pack('section-break-keep-with-next.docx',
     '<w:p><w:r><w:t>First line</w:t></w:r></w:p>'
     '<w:p><w:r><w:t>Second line</w:t></w:r></w:p>'
     '<w:p><w:pPr><w:keepNext/></w:pPr><w:r><w:t>KEPT</w:t></w:r></w:p>'
     + f'<w:p><w:pPr><w:keepNext/><w:sectPr w:type="nextPage">{SECT}</w:sectPr></w:pPr></w:p>'
     + '<w:p><w:pPr><w:rPr><w:sz w:val="1440"/></w:rPr></w:pPr><w:r><w:rPr><w:sz w:val="1440"/></w:rPr><w:t>.</w:t></w:r></w:p>'
     + f'<w:p><w:pPr><w:sectPr>{SECT}</w:sectPr></w:pPr></w:p>',
     after=0)
print('built')
