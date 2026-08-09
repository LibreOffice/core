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

def sectpr(kind=None, start=None):
    s = '<w:sectPr>'
    if kind: s += f'<w:type w:val="{kind}"/>'
    s += '<w:pgSz w:w="11906" w:h="16838"/><w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" w:header="0" w:footer="0" w:gutter="0"/>'
    if start is not None: s += f'<w:pgNumType w:start="{start}"/>'
    s += '</w:sectPr>'
    return s

def para(text, sect=None):
    p = '<w:p>'
    if sect: p += '<w:pPr>' + sect + '</w:pPr>'
    p += f'<w:r><w:t>{text}</w:t></w:r></w:p>'
    return p

def build(path, body):
    doc = f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}</w:body></w:document>'
    z = zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED)
    z.writestr('[Content_Types].xml', CT)
    z.writestr('_rels/.rels', RELS)
    z.writestr('word/document.xml', doc)
    z.close()

# A: section 1 one page, section 2 oddPage break. Natural next physical page is 2 (even).
build('A-odd-nostart.docx',
      para('SECTION ONE', sectpr()) + para('SECTION TWO') + sectpr(kind='oddPage'))

# B: same, but section 1 numbers its first page 2 -> next virtual number is 3 (odd).
build('B-odd-start2.docx',
      para('SECTION ONE', sectpr(start=2)) + para('SECTION TWO') + sectpr(kind='oddPage'))

# C: same, but section 2 restarts numbering at 19 (odd) as well as an oddPage break.
build('C-odd-start2-restart19.docx',
      para('SECTION ONE', sectpr(start=2)) + para('SECTION TWO') + sectpr(kind='oddPage', start=19))

# D: no odd break at all, but section 2 restarts at 19 (odd) landing on physical page 2.
build('D-nextpage-restart19.docx',
      para('SECTION ONE', sectpr()) + para('SECTION TWO') + sectpr(start=19))

# E: no odd break, section 2 restarts at 20 (even) landing on physical page 2.
build('E-nextpage-restart20.docx',
      para('SECTION ONE', sectpr()) + para('SECTION TWO') + sectpr(start=20))

# F: two pages in section 1 (next natural physical page 3, odd), then oddPage break.
build('F-odd-twopages.docx',
      para('SECTION ONE', sectpr()) + para('PAGE TWO', sectpr()) + para('SECTION THREE') + sectpr(kind='oddPage'))
