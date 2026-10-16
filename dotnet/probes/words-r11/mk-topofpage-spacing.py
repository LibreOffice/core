#!/usr/bin/env python3
"""Probe DOCXs: does LibreOffice keep a paragraph's space-before at the top of a page
when the break is explicit?  Three shapes, all with 20 pt (400 twip) space-before."""
import sys, os, zipfile

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DRELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>"""

STYLES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:sz w:val="22"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
</w:styles>"""

SETTINGS_HEAD = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" mc:Ignorable="w14">"""


def settings(mode):
    body = ""
    if mode is not None:
        body = ('<w:compat><w:compatSetting w:name="compatibilityMode" '
                f'w:uri="http://schemas.microsoft.com/office/word" w:val="{mode}"/></w:compat>')
    return SETTINGS_HEAD + body + "</w:settings>"


SETTINGS = settings(15)

SECT = ('<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" '
        'w:header="709" w:footer="709" w:gutter="0"/></w:sectPr>')


def para(text, before=None, brk=None, extra=""):
    pr = "<w:pPr>"
    if brk == "pageBreakBefore":
        pr += '<w:pageBreakBefore/>'
    if before is not None:
        pr += f'<w:spacing w:before="{before}" w:after="0"/>'
    pr += extra + "</w:pPr>"
    runs = ""
    if brk == "br":
        runs += '<w:r><w:br w:type="page"/></w:r>'
    runs += f"<w:r><w:t xml:space=\"preserve\">{text}</w:t></w:r>"
    return f"<w:p>{pr}{runs}</w:p>"


def doc(body):
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
            f"<w:body>{body}{SECT}</w:body></w:document>")


def write(path, body, mode=15):
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DRELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/settings.xml", settings(mode))
        z.writestr("word/document.xml", doc(body))


out = sys.argv[1]
os.makedirs(out, exist_ok=True)

# A: explicit break as w:pageBreakBefore on the second paragraph
write(os.path.join(out, "probe-pagebreakbefore.docx"),
      para("First page.", before=0) + para("Second page.", before=400, brk="pageBreakBefore"))

# B: explicit break as a leading w:br w:type="page" run in the second paragraph
write(os.path.join(out, "probe-leading-br.docx"),
      para("First page.", before=0) + para("Second page.", before=400, brk="br"))

# C: automatic break — 60 filler paragraphs push the marked one onto page 2
filler = "".join(para(f"Filler line {i}.", before=0) for i in range(58))
write(os.path.join(out, "probe-automatic.docx"),
      filler + para("Second page.", before=400))

# D: the document's very first paragraph carries the space-before, and a second page
#    follows so the same report shape works.
write(os.path.join(out, "probe-firstpage.docx"),
      para("First page.", before=400) + para("Second page.", before=400, brk="pageBreakBefore"))

# E: like B but the paragraph above states a space-after equal to the space-before,
#    which is what lcl_PartiallyCollapseUpper subtracts.
write(os.path.join(out, "probe-leading-br-prevafter.docx"),
      '<w:p><w:pPr><w:spacing w:before="0" w:after="400"/></w:pPr>'
      '<w:r><w:t>First page.</w:t></w:r></w:p>'
      + para("Second page.", before=400, brk="br"))

# F/G/H: the same explicit break under the other two compatibility settings.
for tag, mode in (("compat14", 14), ("compat12", 12), ("nocompat", None)):
    write(os.path.join(out, f"probe-pagebreakbefore-{tag}.docx"),
          para("First page.", before=0) + para("Second page.", before=400, brk="pageBreakBefore"),
          mode=mode)
    write(os.path.join(out, f"probe-automatic-{tag}.docx"),
          filler + para("Second page.", before=400), mode=mode)

# I: a section break rather than a page break — LibreOffice's IsCollapseUpper declines to
#    ignore the upper margin when a new page *style* is applied (RES_PAGEDESC).
SECT_INNER = ('<w:sectPr><w:type w:val="nextPage"/><w:pgSz w:w="11906" w:h="16838"/>'
              '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" '
              'w:header="709" w:footer="709" w:gutter="0"/></w:sectPr>')
write(os.path.join(out, "probe-sectionbreak.docx"),
      f'<w:p><w:pPr><w:spacing w:before="0" w:after="0"/>{SECT_INNER}</w:pPr>'
      '<w:r><w:t>First page.</w:t></w:r></w:p>'
      + para("Second page.", before=400))
print("written to", out)
