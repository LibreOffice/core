import zipfile, sys, os

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
STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Liberation Sans" w:hAnsi="Liberation Sans"/><w:sz w:val="20"/></w:rPr></w:rPrDefault><w:pPrDefault><w:pPr/></w:pPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:pPr><w:spacing w:after="{AFTER}"/></w:pPr><w:rPr><w:rFonts w:ascii="Liberation Sans" w:hAnsi="Liberation Sans"/><w:sz w:val="20"/></w:rPr></w:style>
</w:styles>'''
SETTINGS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:compat><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="{MODE}"/></w:compat>
</w:settings>'''

SECT = '<w:pgSz w:w="11907" w:h="16840"/><w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="709" w:footer="709" w:gutter="0"/>{TITLEPG}'

def build(path, breakkind, mode=15, before=400, after=200, titlePg=False, geomchange=False):
    tp = '<w:titlePg/>' if titlePg else ''
    sect1 = SECT.format(TITLEPG=tp)
    if geomchange:
        sect2 = '<w:pgSz w:w="16840" w:h="11907" w:orient="landscape"/><w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="709" w:footer="709" w:gutter="0"/>'+tp
    else:
        sect2 = sect1
    filler = ''.join(f'<w:p><w:r><w:t>Filler line {i}</w:t></w:r></w:p>' for i in range(3))
    if breakkind == 'section':
        first = f'<w:p><w:pPr><w:sectPr w:type="nextPage">{sect1}</w:sectPr></w:pPr></w:p>'
        body = filler + first
        tail_sect = sect2
    elif breakkind == 'pagebreakbefore':
        body = filler
        tail_sect = sect1
    elif breakkind == 'brpage':
        body = filler
        tail_sect = sect1
    else:  # auto: not used
        body = filler
        tail_sect = sect1
    ppr = '<w:pPr><w:spacing w:before="%d"/>%s</w:pPr>' % (before, '<w:pageBreakBefore/>' if breakkind=='pagebreakbefore' else '')
    run = ('<w:r><w:br w:type="page"/></w:r>' if breakkind=='brpage' else '') + '<w:r><w:t>TARGET</w:t></w:r>'
    target = f'<w:p>{ppr}{run}</w:p>'
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body>'
           + body + target + '<w:p><w:pPr><w:sectPr>' + tail_sect + '</w:sectPr></w:pPr></w:p></w:body></w:document>')
    z = zipfile.ZipFile(path,'w',zipfile.ZIP_DEFLATED)
    z.writestr('[Content_Types].xml',CT); z.writestr('_rels/.rels',RELS)
    z.writestr('word/_rels/document.xml.rels',DRELS)
    z.writestr('word/styles.xml',STYLES.replace('{AFTER}',str(after)))
    z.writestr('word/settings.xml',SETTINGS.replace('{MODE}',str(mode)))
    z.writestr('word/document.xml',doc); z.close()

if __name__=='__main__':
    for mode in (15,12):
        for kind in ('section','pagebreakbefore','brpage'):
            for tp in (False,True):
                for gc in (False,True):
                    if kind!='section' and gc: continue
                    n=f"s_{kind}_m{mode}_tp{int(tp)}_gc{int(gc)}.docx"
                    build(n,kind,mode=mode,titlePg=tp,geomchange=gc)
    print('built')
