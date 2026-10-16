import os, sys, zipfile

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>"""
RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""
DRELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>"""
STYLES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/>
</w:rPr></w:rPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
</w:styles>"""
W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main"

DOC = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="{ns}"><w:body>
{body}
<w:sectPr><w:pgSz w:w="16838" w:h="11906" w:orient="landscape"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="708" w:footer="708" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""

LINE = """<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/>
<w:tabs><w:tab w:val="left" w:pos="9000"/></w:tabs></w:pPr>
<w:r>{rpr}<w:t xml:space="preserve">{text}</w:t></w:r><w:r><w:tab/><w:t>|</w:t></w:r></w:p>"""


def write(folder, name, body):
    os.makedirs(folder, exist_ok=True)
    with zipfile.ZipFile(os.path.join(folder, name + ".docx"), "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DRELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", DOC.format(ns=W, body=body))


def main(d):
    text = "Hamburgefonstiv 12345"
    body = ""
    for pct in (None, 100, 99, 95, 90, 50, 150, 200):
        rpr = "" if pct is None else f'<w:rPr><w:w w:val="{pct}"/></w:rPr>'
        body += LINE.format(rpr=rpr, text=text)
    write(d, "S_scale", body)
    # and one with tracking as well, to see whether the two compose
    body2 = ""
    for pct, spc in ((100, 0), (50, 0), (100, 40), (50, 40)):
        rpr = f'<w:rPr><w:w w:val="{pct}"/><w:spacing w:val="{spc}"/></w:rPr>'
        body2 += LINE.format(rpr=rpr, text=text)
    write(d, "S_both", body2)
    print("written")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else ".")
