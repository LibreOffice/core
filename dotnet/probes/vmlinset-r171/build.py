#!/usr/bin/env python3
"""One-attribute DOCX variants of a VML text box, to read the reference's inset and autofit rule.

Each fixture is the same 200 x 60 pt `v:shape` holding a `v:textbox`, differing in one attribute.
The reference's answer is read out of its PDF: the first span's left edge gives the left inset,
its top the upper one, and how many of the four lines survive gives the fixed-height rule.

A `word/settings.xml` part is present on purpose -- without one the importer takes a different
set of compatibility defaults and the fixture answers a question nobody asked.
"""
import pathlib, sys, zipfile

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOCUMENT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>"""

SETTINGS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>"""

LINES = ["Alpha", "Bravo", "Charlie", "Delta"]

DOCUMENT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:v="urn:schemas-microsoft-com:vml"
 xmlns:w10="urn:schemas-microsoft-com:office:word"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<w:body>
<w:p><w:r><w:t>Anchor</w:t></w:r></w:p>
<w:p><w:r><w:pict>
<v:shapetype id="_x0000_t202" coordsize="21600,21600" o:spt="202" path="m,l,21600r21600,l21600,xe" xmlns:o="urn:schemas-microsoft-com:office:office"><v:stroke joinstyle="miter"/><v:path gradientshapeok="t" o:connecttype="rect"/></v:shapetype>
<v:shape id="s1" type="#_x0000_t202" style="position:absolute;margin-left:72pt;margin-top:144pt;width:200pt;height:@@HEIGHT@@pt;z-index:1;mso-position-horizontal-relative:page;mso-position-vertical-relative:page@@SHAPESTYLE@@" filled="f" stroked="f">
<v:textbox@@BOXATTR@@>
<w:txbxContent>@@PARAS@@</w:txbxContent>
</v:textbox>
<w10:wrap type="none"/>
</v:shape>
</w:pict></w:r></w:p>
<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>
<w:pgMar w:top="1440" w:bottom="1440" w:left="1440" w:right="1440"/></w:sectPr>
</w:body></w:document>"""

PARA = ('<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr>'
        '<w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
        '<w:sz w:val="20"/></w:rPr><w:t>{}</w:t></w:r></w:p>')

ARMS = {
    "plain":            ("", "", 60),
    "inset-zero":       (' inset="0,0,0,0"', "", 60),
    "inset-partial":    (' inset=",.6mm,,.6mm"', "", 60),
    "inset-one":        (' inset="4mm"', "", 60),
    "inset-four":       (' inset="7.45pt,3.85pt,7.45pt,3.85pt"', "", 60),
    "insetmode-auto":   (' insetmode="auto"', "", 60),
    "insetmode-custom": (' insetmode="custom"', "", 60),
    "short":            ("", "", 24),
    "short-boxfit":     (' style="mso-fit-shape-to-text:t"', "", 24),
    "short-shapefit":   ("", ";mso-fit-shape-to-text:t", 24),
}


def main():
    out = pathlib.Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    paras = "".join(PARA.format(line) for line in LINES)
    for name, (boxattr, shapestyle, height) in ARMS.items():
        body = (DOCUMENT.replace("@@BOXATTR@@", boxattr)
                        .replace("@@SHAPESTYLE@@", shapestyle)
                        .replace("@@HEIGHT@@", str(height))
                        .replace("@@PARAS@@", paras))
        with zipfile.ZipFile(out / f"{name}.docx", "w", zipfile.ZIP_DEFLATED) as z:
            z.writestr("[Content_Types].xml", CONTENT_TYPES)
            z.writestr("_rels/.rels", RELS)
            z.writestr("word/_rels/document.xml.rels", DOCUMENT_RELS)
            z.writestr("word/settings.xml", SETTINGS)
            z.writestr("word/document.xml", body)
    print(f"{len(ARMS)} fixtures in {out}")


if __name__ == "__main__":
    main()
