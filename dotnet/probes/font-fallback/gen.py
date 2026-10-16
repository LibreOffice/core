"""What does each stack resolve an absent family to? One family per page, so `pdffonts -f N -l N`
attributes the answer without ambiguity."""
import zipfile, pathlib
FAMS = ["Arial","Times New Roman","Calibri","Cambria","Courier New","Verdana","Tahoma",
        "Adobe Hebrew","Gabriola","Ebrima","Nyala","Estrangelo Edessa","Adobe Garamond Pro",
        "Vrinda","Shruti","Plantagenet Cherokee","Microsoft Yi Baiti","Segoe UI",
        "Zzzz Nonexistent Family","Helvetica","Georgia","Garamond","Book Antiqua","Century Gothic"]
DOC = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
 '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body>']
for i,f in enumerate(FAMS):
    brk = '<w:r><w:br w:type="page"/></w:r>' if i else ''
    DOC.append(f'<w:p>{brk}<w:r><w:rPr><w:rFonts w:ascii="{f}" w:hAnsi="{f}" w:cs="{f}"/>'
               f'<w:sz w:val="28"/></w:rPr><w:t xml:space="preserve">{i:02d} {f} Handgloves 12345</w:t></w:r></w:p>')
DOC.append('<w:sectPr><w:pgSz w:w="11906" w:h="16838"/><w:pgMar w:top="1134" w:right="1134" '
           '<w:bottom w:val="1134"/><w:left w:val="1134"/></w:sectPr></w:body></w:document>')
doc = ''.join(DOC).replace('<w:bottom w:val="1134"/><w:left w:val="1134"/>','w:bottom="1134" w:left="1134"/>').replace('<w:sectPr><w:pgSz w:w="11906" w:h="16838"/><w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"/>','<w:sectPr><w:pgSz w:w="11906" w:h="16838"/><w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"/>')

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
 '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
 '<Default Extension="xml" ContentType="application/xml"/>'
 '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
 '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>')
DRELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/></Relationships>')
# An empty settings part, without which LibreOffice takes different OOXML compatibility
# defaults -- `.claude/skills/office-corpus` records a round lost to exactly that.
SET = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>')
out = pathlib.Path('font-fallback.docx')
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml',CT); z.writestr('_rels/.rels',RELS)
    z.writestr('word/_rels/document.xml.rels',DRELS)
    z.writestr('word/document.xml',doc); z.writestr('word/settings.xml',SET)
print("wrote", out, out.stat().st_size, "bytes,", len(FAMS), "families")
open('families.txt','w').write('\n'.join(FAMS))
