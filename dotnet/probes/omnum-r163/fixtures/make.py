#!/usr/bin/env python3
"""Build minimal DOCX fixtures for the heading-numbering indent question."""
import os, zipfile, sys

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

DOCRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering" Target="numbering.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>'''

SETTINGS = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings {W}><w:compat><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat></w:settings>'''

def numbering(levels):
    lv = []
    for i, (left, hang, pstyle, text) in enumerate(levels):
        lv.append(
            f'<w:lvl w:ilvl="{i}"><w:start w:val="1"/><w:numFmt w:val="decimal"/>'
            f'<w:pStyle w:val="{pstyle}"/><w:isLgl/><w:lvlText w:val="{text}"/>'
            f'<w:lvlJc w:val="left"/><w:pPr><w:ind w:left="{left}" w:hanging="{hang}"/></w:pPr></w:lvl>')
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:numbering {W}>'
            f'<w:abstractNum w:abstractNumId="0"><w:multiLevelType w:val="multilevel"/>'
            + ''.join(lv) + '</w:abstractNum><w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num></w:numbering>')

def styles(style_inds):
    """style_inds: list of (styleId, name, basedOn, ilvl_or_None, numId_or_None, ind_or_None)"""
    out = [f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:styles {W}>',
           '<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="22"/></w:rPr></w:rPrDefault>'
           '<w:pPrDefault><w:pPr><w:spacing w:after="0" w:line="240" w:lineRule="auto"/></w:pPr></w:pPrDefault></w:docDefaults>',
           '<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:qFormat/></w:style>']
    for sid, name, based, ilvl, numid, ind, outline in style_inds:
        p = []
        np = ''
        if numid is not None or ilvl is not None:
            np = '<w:numPr>' + (f'<w:ilvl w:val="{ilvl}"/>' if ilvl is not None else '') + \
                 (f'<w:numId w:val="{numid}"/>' if numid is not None else '') + '</w:numPr>'
        p.append(np)
        if ind: p.append(ind)
        if outline is not None: p.append(f'<w:outlineLvl w:val="{outline}"/>')
        ppr = '<w:pPr>' + ''.join(p) + '</w:pPr>' if any(p) else ''
        out.append(f'<w:style w:type="paragraph" w:styleId="{sid}"><w:name w:val="{name}"/>'
                   + (f'<w:basedOn w:val="{based}"/>' if based else '')
                   + '<w:qFormat/>' + ppr + '</w:style>')
    out.append('</w:styles>')
    return ''.join(out)

def document(paras):
    body = []
    for sid, ind, text in paras:
        ppr = f'<w:pPr><w:pStyle w:val="{sid}"/>' + (ind or '') + '</w:pPr>'
        body.append(f'<w:p>{ppr}<w:r><w:t>{text}</w:t></w:r></w:p>')
    sect = ('<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
            '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" '
            'w:header="709" w:footer="709" w:gutter="0"/></w:sectPr>')
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:document {W}><w:body>'
            + ''.join(body) + sect + '</w:body></w:document>')

def build(path, num_xml, styles_xml, doc_xml, with_settings=True):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/_rels/document.xml.rels', DOCRELS)
        z.writestr('word/document.xml', doc_xml)
        z.writestr('word/styles.xml', styles_xml)
        z.writestr('word/numbering.xml', num_xml)
        if with_settings:
            z.writestr('word/settings.xml', SETTINGS)
    print('wrote', path)
