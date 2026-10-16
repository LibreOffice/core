#!/usr/bin/env python3
"""Which header contents LibreOffice passes down to a section that names none.

Round `words-ug-01`, 2026-08-15, against the installed LibreOffice **26.2.4.2** with
`fonts-dejavu-core` present.

Each probe is a two-section DOCX: section 1 names `header1.xml` as its default header, section 2
names no header at all and so must inherit. Only the header part's own children vary. Convert them
all and read page 2 — the head is there or it is not.

    python3 header-inheritance-shape.py /abs/outdir
    for f in /abs/outdir/*.docx; do soffice --headless --convert-to pdf --outdir /abs/outdir/ref "$f"; done
    for f in /abs/outdir/ref/*.pdf; do echo "$f"; pdftotext -f 2 -l 2 "$f" -; done

Measured answer:

    K_tbl_only      table alone, no paragraph          page 2 BARE
    N_tbl_tbl       two tables, no paragraph           page 2 BARE
    L_tbl_trailp    table then an empty <w:p/>         page 2 headed
    M_leadp_tbl     an empty <w:p/> then the table     page 2 headed
    O_para_only     one ordinary paragraph             page 2 headed

so the rule is the presence of a top-level `w:p`, all-or-nothing: where one is present the tables
beside it travel down with it. Round 43 established the same rule against 24.2.7.2 from the other
direction (`probes/words-r43/`); this is the re-check CLAUDE.md asks for after the move to 26.2.4.2.
"""
import os, sys, zipfile
NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')
CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
%s</Types>'''
RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>'''
TBL = ('<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/></w:tblPr>'
       '<w:tblGrid><w:gridCol w:w="4000"/></w:tblGrid>'
       '<w:tr><w:tc><w:tcPr><w:tcW w:w="4000" w:type="dxa"/></w:tcPr>'
       '<w:p><w:r><w:t>HDRTABLETEXT</w:t></w:r></w:p></w:tc></w:tr></w:tbl>')
def build(path, hdrbody):
    pg=('<w:pgSz w:w="11906" w:h="16838"/><w:pgMar w:top="2200" w:right="1417" w:bottom="1417" '
        'w:left="1417" w:header="708" w:footer="708" w:gutter="0"/>')
    s1='<w:sectPr><w:headerReference w:type="default" r:id="rIdA"/>'+pg+'</w:sectPr>'
    s2='<w:sectPr>'+pg+'</w:sectPr>'
    doc=('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document %s><w:body>'
         '<w:p><w:pPr>%s</w:pPr><w:r><w:t>SECTIONONEBODY</w:t></w:r></w:p>'
         '<w:p><w:r><w:t>SECTIONTWOBODY</w:t></w:r></w:p>%s</w:body></w:document>')%(NS,s1,s2)
    parts={'word/document.xml':doc,
           'word/header1.xml':'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:hdr %s>%s</w:hdr>'%(NS,hdrbody),
           '[Content_Types].xml':CT%'<Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>',
           '_rels/.rels':RELS,
           'word/_rels/document.xml.rels':'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Id="rIdA" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header" Target="header1.xml"/></Relationships>'}
    with zipfile.ZipFile(path,'w',zipfile.ZIP_DEFLATED) as z:
        for n,c in parts.items(): z.writestr(n,c)
o=sys.argv[1]; os.makedirs(o,exist_ok=True)
build(o+'/K_tbl_only.docx', TBL)
build(o+'/L_tbl_trailp.docx', TBL+'<w:p/>')
build(o+'/M_leadp_tbl.docx', '<w:p/>'+TBL)
build(o+'/N_tbl_tbl.docx', TBL+TBL)
build(o+'/O_para_only.docx', '<w:p><w:r><w:t>HDRTABLETEXT</w:t></w:r></w:p>')
print('ok')
