#!/usr/bin/env python3
"""A right tab stop declared past the text area, at several distances, on paragraphs
with and without a right indent. Answers: where does Writer put the stop?"""
import sys, zipfile

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>'''

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

DRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>'''

SETTINGS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml"
 xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" mc:Ignorable="w14">
<w:compat><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="COMPAT"/></w:compat>
</w:settings>'''

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

def para(pos, right):
    label = f"P{pos}R{right}"
    return (f'<w:p><w:pPr><w:tabs><w:tab w:val="right" w:leader="dot" w:pos="{pos}"/></w:tabs>'
            f'<w:ind w:left="0" w:right="{right}"/><w:spacing w:after="0" w:line="240" w:lineRule="auto"/>'
            f'<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="20"/></w:rPr></w:pPr>'
            f'<w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="20"/></w:rPr>'
            f'<w:t xml:space="preserve">{label}</w:t></w:r>'
            f'<w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="20"/></w:rPr><w:tab/>'
            f'<w:t>9</w:t></w:r></w:p>')

def build(path, compat):
    paras = []
    for right in (0, 360, 1134):
        for pos in (8000, 9000, 9360, 9500, 9800, 10500, 10799, 11000, 12000, 13000):
            paras.append(para(pos, right))
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document {W}><w:body>' + "".join(paras) +
           '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
           '<w:pgMar w:top="1080" w:right="1440" w:bottom="1080" w:left="1440" w:header="432" w:footer="432" w:gutter="0"/>'
           '</w:sectPr></w:body></w:document>')
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DRELS)
        z.writestr("word/settings.xml", SETTINGS.replace("COMPAT", str(compat)))
        z.writestr("word/document.xml", doc)

build(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else "15")
