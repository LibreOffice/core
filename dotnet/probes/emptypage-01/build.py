import zipfile, sys, os

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
# A4, 1in margins, no header/footer distance oddities
SECT = '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/><w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/></w:sectPr>'

def build(path, body):
    doc = f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}{SECT}</w:body></w:document>'
    z = zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED)
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/document.xml', doc)
    z.close()

def p(text='', brk=False):
    pr = '<w:pPr>' + ('<w:pageBreakBefore/>' if brk else '') + '</w:pPr>'
    r = f'<w:r><w:t>{text}</w:t></w:r>' if text else ''
    return '<w:p>' + pr + r + '</w:p>'

outdir = sys.argv[1] if len(sys.argv) > 1 else '.'
os.makedirs(outdir, exist_ok=True)
# N filler lines, then E empty paragraphs, then AFTER with a hard page break
for n in range(38, 50):
    for e in (1, 2, 3):
        body = ''.join(p(f'L{i:02d} filler line') for i in range(n))
        body += ''.join(p() for _ in range(e))
        body += p('AFTER', brk=True)
        build(os.path.join(outdir, f'n{n}-e{e}.docx'), body)
print('built')
