#!/usr/bin/env python3
"""Which paragraph properties reach the text of a VML text box in Writer.

On `069_Work_Breakdown_Structure_Template_Professional_Format` the reference draws a box's five
paragraphs 14.00 pt apart and this tree draws them 23.10 apart, and that document's
`w:docDefaults/w:pPrDefault` states `w:spacing w:after="160" w:line="259" w:lineRule="auto"` --
1.079 x 13.97 + 8 = 23.08, which is ours exactly, against the bare face metric, which is theirs.
The same box's unaligned paragraphs come out centred in the reference and left in ours.

So these fixtures vary one paragraph property at a time, with a `w:docDefaults` that states the
spacing and paragraphs that mostly do not, and read the pitch and the left edge back out.
"""
import pathlib, sys, zipfile

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOCUMENT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering" Target="numbering.xml"/>
</Relationships>"""

SETTINGS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>"""

# The defaults `069` carries: a 1.079 line multiplier and eight points after every paragraph.
STYLES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/>
</w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr><w:spacing w:after="160" w:line="259" w:lineRule="auto"/></w:pPr></w:pPrDefault>
</w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
</w:styles>"""

NUMBERING = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:numbering xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:abstractNum w:abstractNumId="0"><w:lvl w:ilvl="0"><w:start w:val="1"/>
<w:numFmt w:val="decimal"/><w:lvlText w:val="%1."/><w:lvlJc w:val="left"/>
<w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr></w:lvl></w:abstractNum>
<w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>
</w:numbering>"""

DOCUMENT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:v="urn:schemas-microsoft-com:vml"
 xmlns:o="urn:schemas-microsoft-com:office:office"
 xmlns:w10="urn:schemas-microsoft-com:office:word">
<w:body>
<w:p><w:r><w:t>Anchor</w:t></w:r></w:p>
<w:p><w:r><w:pict>
<v:shapetype id="_x0000_t202" coordsize="21600,21600" o:spt="202" path="m,l,21600r21600,l21600,xe"><v:stroke joinstyle="miter"/><v:path gradientshapeok="t" o:connecttype="rect"/></v:shapetype>
<v:shape id="s1" type="#_x0000_t202" style="position:absolute;margin-left:72pt;margin-top:144pt;width:300pt;height:400pt;z-index:1;mso-position-horizontal-relative:page;mso-position-vertical-relative:page" filled="f" stroked="f">
<v:textbox><w:txbxContent>@@PARAS@@</w:txbxContent></v:textbox>
<w10:wrap type="none"/>
</v:shape>
</w:pict></w:r></w:p>
<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>
<w:pgMar w:top="1440" w:bottom="1440" w:left="1440" w:right="1440"/></w:sectPr>
</w:body></w:document>"""

WORDS = ["Alpha", "Bravo", "Charlie", "Delta"]


def para(text, properties=""):
    return f"<w:p><w:pPr>{properties}</w:pPr><w:r><w:t>{text}</w:t></w:r></w:p>"


def plain():
    """Four paragraphs stating nothing, so the document default decides."""
    return "".join(para(word) for word in WORDS)


def stated():
    """The control: the same spacing stated on each paragraph rather than defaulted."""
    return "".join(
        para(word, '<w:spacing w:after="160" w:line="259" w:lineRule="auto"/>') for word in WORDS)


def tight():
    """The other control: spacing explicitly turned off."""
    return "".join(
        para(word, '<w:spacing w:after="0" w:line="240" w:lineRule="auto"/>') for word in WORDS)


def alignment():
    """Does an alignment reach the paragraph after the one that states it?"""
    return (para("Alpha", '<w:jc w:val="center"/>') + para("Bravo")
            + para("Charlie", '<w:jc w:val="left"/>') + para("Delta"))


def indent():
    """Does an indent reach the box at all?"""
    return (para("Alpha") + para("Bravo", '<w:ind w:left="1440"/>')
            + para("Charlie", '<w:ind w:firstLine="720"/>') + para("Delta"))


def numbered():
    """Does a numbering reach it?"""
    return "".join(
        para(word, '<w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>') for word in WORDS)


# The same four paragraphs inside a `v:group`.  A group's member cannot be a Writer text frame,
# so its text is an EditEngine text on the draw layer instead -- which is a different set of
# defaults, and `069`'s box is exactly this.
GROUPED = DOCUMENT.replace(
    '<v:shape id="s1" type="#_x0000_t202" style="position:absolute;margin-left:72pt;'
    'margin-top:144pt;width:300pt;height:400pt;z-index:1;'
    'mso-position-horizontal-relative:page;mso-position-vertical-relative:page"'
    ' filled="f" stroked="f">',
    '<v:group id="g1" style="position:absolute;margin-left:72pt;margin-top:144pt;'
    'width:300pt;height:400pt;z-index:1;mso-position-horizontal-relative:page;'
    'mso-position-vertical-relative:page" coordorigin="0,0" coordsize="6000,8000">'
    '<v:shape id="s1" type="#_x0000_t202" '
    'style="position:absolute;left:0;top:0;width:6000;height:8000"'
    ' filled="f" stroked="f">'
).replace(
    "</v:shape>\n</w:pict>", "</v:shape></v:group>\n</w:pict>")

def run(text, properties=""):
    return f"<w:p><w:pPr></w:pPr><w:r><w:rPr>{properties}</w:rPr><w:t>{text}</w:t></w:r></w:p>"


def characters():
    """Which of a `w:rPr`'s properties reach a grouped shape's text."""
    return (run("Alpha", '<w:sz w:val="36"/>')
            + run("Bravo", "<w:b/>")
            + run("Charlie", "<w:i/>")
            + run("Delta", '<w:rFonts w:ascii="Liberation Mono" w:hAnsi="Liberation Mono"/>'))


ARMS = {
    "default-spacing": plain,
    "stated-spacing": stated,
    "tight-spacing": tight,
    "alignment": alignment,
    "indent": indent,
    "numbered": numbered,
    "grouped-default-spacing": plain,
    "grouped-alignment": alignment,
    "grouped-indent": indent,
    "grouped-numbered": numbered,
    "characters": characters,
    "grouped-characters": characters,
    "grouped-rect-spacing": plain,
}


def main():
    out = pathlib.Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    for name, build in ARMS.items():
        template = GROUPED if name.startswith("grouped-") else DOCUMENT
        if name.startswith("grouped-rect"):
            template = template.replace(
                '<v:shape id="s1" type="#_x0000_t202" ', '<v:rect id="s1" ').replace(
                "</v:shape></v:group>", "</v:rect></v:group>")
        with zipfile.ZipFile(out / f"{name}.docx", "w", zipfile.ZIP_DEFLATED) as z:
            z.writestr("[Content_Types].xml", CONTENT_TYPES)
            z.writestr("_rels/.rels", RELS)
            z.writestr("word/_rels/document.xml.rels", DOCUMENT_RELS)
            z.writestr("word/settings.xml", SETTINGS)
            z.writestr("word/styles.xml", STYLES)
            z.writestr("word/numbering.xml", NUMBERING)
            z.writestr("word/document.xml", template.replace("@@PARAS@@", build()))
    print(f"{len(ARMS)} fixtures in {out}")


if __name__ == "__main__":
    main()
