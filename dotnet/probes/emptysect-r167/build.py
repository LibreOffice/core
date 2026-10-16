#!/usr/bin/env python3
"""A one-paragraph section between two section breaks, the second changing the page setup.

The shape of `150_5300_13_chg10.doc`'s appendix boundary, where 26.2.4.2 spends a whole page
on a section whose only content is an empty paragraph. Authored as DOCX and converted to DOC
and RTF by the reference itself, so the same structure is measured through all three readers.

`word/settings.xml` is written even though it is empty: without it the importer takes a
different set of OOXML compatibility defaults (`office-corpus/SKILL.md`).
"""
import os, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "fixtures")
os.makedirs(OUT, exist_ok=True)

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
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="DejaVu Serif" w:hAnsi="DejaVu Serif"/><w:sz w:val="24"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
</w:styles>'''

# Two page setups that differ only in their left margin, so a page-style change is unmistakable
# and nothing else about the sheet moves.
PAGE_A = ('<w:pgSz w:w="12240" w:h="15840"/>'
          '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
          ' w:header="720" w:footer="720"/>')
PAGE_B = ('<w:pgSz w:w="12240" w:h="15840"/>'
          '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="2880"'
          ' w:header="720" w:footer="720"/>')


def p(text="", sect=None):
    pr = f'<w:pPr>{sect}</w:pPr>' if sect else ''
    run = f'<w:r><w:t xml:space="preserve">{text}</w:t></w:r>' if text else ''
    return f'<w:p>{pr}{run}</w:p>'


def sectpr(kind, page, columns=1, balance=True):
    cols = ('' if columns == 1
            else f'<w:cols w:num="{columns}" w:space="708" w:equalWidth="1"/>')
    # `w:bidi`/`w:rtlGutter` omitted; `dont-balance` is the section's own flag in ODF and has
    # no direct OOXML spelling -- Word balances the last page of a multi-column section only
    # when the section ends with a continuous break, which is what these arms vary.
    return f'<w:sectPr><w:type w:val="{kind}"/>{cols}{page}</w:sectPr>'


def doc(body):
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
            f'<w:body>{body}</w:body></w:document>')


def arms():
    a = p("SECTION A first") + p("SECTION A second")
    c = p("SECTION C first") + p("SECTION C second")

    # The witness: A, then a section holding ONE EMPTY paragraph, then C on a new page setup.
    yield "empty-middle", (
        a + p("", sectpr("continuous", PAGE_A))
        + p("", sectpr("nextPage", PAGE_A))
        + c + sectpr("nextPage", PAGE_B))

    # The same, with the middle section's own break page-changing rather than the one after it.
    yield "empty-middle-pagechange", (
        a + p("", sectpr("continuous", PAGE_A))
        + p("", sectpr("nextPage", PAGE_B))
        + c + sectpr("nextPage", PAGE_B))

    # Control: no middle section at all.
    yield "no-middle", (
        a + p("", sectpr("continuous", PAGE_A))
        + c + sectpr("nextPage", PAGE_B))

    # Control: the middle section holds a word, so nothing about it is empty.
    yield "worded-middle", (
        a + p("", sectpr("continuous", PAGE_A))
        + p("MIDDLE", sectpr("nextPage", PAGE_B))
        + c + sectpr("nextPage", PAGE_B))

    # Control: the middle section holds two empty paragraphs.
    yield "two-empty-middle", (
        a + p("", sectpr("continuous", PAGE_A))
        + p() + p("", sectpr("nextPage", PAGE_B))
        + c + sectpr("nextPage", PAGE_B))

    # The middle break is nextPage rather than continuous.
    yield "nextpage-middle", (
        a + p("", sectpr("nextPage", PAGE_A))
        + p("", sectpr("nextPage", PAGE_B))
        + c + sectpr("nextPage", PAGE_B))

    # No page-setup change anywhere: does the empty section still cost a page?
    yield "empty-middle-samepage", (
        a + p("", sectpr("continuous", PAGE_A))
        + p("", sectpr("nextPage", PAGE_A))
        + c + sectpr("nextPage", PAGE_A))

    # A plain page break instead of the middle section, as the shape to compare against.
    yield "pagebreak-middle", (
        a + p("", sectpr("continuous", PAGE_A))
        + '<w:p><w:r><w:br w:type="page"/></w:r></w:p>'
        + c + sectpr("nextPage", PAGE_B))

    # --- the real document's shape: the section BEFORE the empty one is two-column.
    # `150_5300_13_chg10.doc`'s appendix body is `Sect1`, `fo:column-count="2"`, and the
    # one-paragraph `Sect5` that follows it is where the reference spends a page.
    filler = "".join(p(f"TWO COLUMN BODY line {i} of the appendix text.") for i in range(1, 25))

    yield "twocol-empty-middle", (
        filler + p("", sectpr("continuous", PAGE_A, columns=2))
        + p("", sectpr("nextPage", PAGE_A))
        + c + sectpr("nextPage", PAGE_B))

    yield "twocol-empty-middle-pagechange", (
        filler + p("", sectpr("continuous", PAGE_A, columns=2))
        + p("", sectpr("nextPage", PAGE_B))
        + c + sectpr("nextPage", PAGE_B))

    # Control: the same two-column body with no empty section between it and the page change.
    yield "twocol-no-middle", (
        filler + p("", sectpr("continuous", PAGE_A, columns=2))
        + c + sectpr("nextPage", PAGE_B))

    # Control: the two-column body followed directly by the table paragraph, one column.
    yield "twocol-worded-middle", (
        filler + p("", sectpr("continuous", PAGE_A, columns=2))
        + p("MIDDLE", sectpr("nextPage", PAGE_B))
        + c + sectpr("nextPage", PAGE_B))


def build(name, body):
    path = os.path.join(OUT, f"{name}.docx")
    if os.path.exists(path):
        os.remove(path)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DRELS)
        z.writestr("word/settings.xml", SETTINGS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", doc(body))


if __name__ == "__main__":
    n = 0
    for name, body in arms():
        build(name, body)
        n += 1
    print("built", n, "fixtures in", OUT)
