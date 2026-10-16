#!/usr/bin/env python3
"""A right-aligned dot-leader tab stop beyond the paragraph's right indent.

The shape of every `TOC2` entry in `02_mcar_part-2_and_IS_v2.10.docx`: a text column
9360 twips wide, a right tab stop declared at 9360 -- exactly the column's right edge --
and `w:ind w:right=R` pulling the paragraph's own wrap boundary R twips inside it. Each
paragraph varies the title's length, so the sweep finds the width at which each engine
stops fitting the page number on the entry's first line.

`word/settings.xml` is written even though it is empty: without it the importer takes a
different set of OOXML compatibility defaults and the fixture answers a different
question (`office-corpus/SKILL.md`).
"""
import os, sys, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
RIGHTS = (0, 180, 360, 540, 720, 850, 994, 1130, 1440, 1800)
TITLES = tuple(range(40, 62, 2))

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
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>'''
DRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/></Relationships>'''
SETTINGS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>'''
STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="DejaVu Sans" w:hAnsi="DejaVu Sans"/><w:sz w:val="22"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
</w:styles>'''


def para(right, title, tag):
    return ('<w:p><w:pPr>'
            '<w:tabs><w:tab w:val="left" w:pos="990"/>'
            '<w:tab w:val="right" w:leader="dot" w:pos="9360"/></w:tabs>'
            '<w:spacing w:after="0"/>'
            f'<w:ind w:left="720" w:right="{right}" w:hanging="720"/>'
            '</w:pPr>'
            f'<w:r><w:t>{tag}</w:t></w:r>'
            '<w:r><w:tab/></w:r>'
            f'<w:r><w:t>{title}</w:t></w:r>'
            '<w:r><w:tab/></w:r>'
            '<w:r><w:t>2-123</w:t></w:r>'
            '</w:p>')


def build(path):
    body = []
    for right in RIGHTS:
        for n in TITLES:
            body.append(para(right, "A" * n, f"r{right}n{n}"))
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
           '<w:body>' + "".join(body) +
           '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
           '<w:pgMar w:top="1080" w:right="1440" w:bottom="1080" w:left="1440"'
           ' w:header="432" w:footer="432"/></w:sectPr></w:body></w:document>')
    if os.path.exists(path):
        os.remove(path)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DRELS)
        z.writestr("word/settings.xml", SETTINGS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", doc)


if __name__ == "__main__":
    out = os.path.join(HERE, "toc-righttab.docx")
    build(out)
    print("built", out, len(RIGHTS) * len(TITLES), "arms")
