#!/usr/bin/env python3
"""Is a table of contents wrapped beside a floating obstacle, or does it descend past it?

`absrc-pac-01-info-note-en.doc` page 1 holds a left-anchored one-column table with 319 pt of
room to its right. 26.2.4.2 wraps an ordinary paragraph beside it — both engines agree on that
— and puts the `text:table-of-content` that follows **below** it at the left margin, where this
tree wraps the index's paragraphs beside it too, 179.9 pt right and 340.9 pt high.
`probes/pages-r164` §3 proposed that a Writer section frame is not wrapped beside a fly and
left it unsettled.

These arms settle it in DOCX, where the question can be asked cheaply: the hypothesis is about
Writer's layout rather than about the WW8 reader, so if it reproduces here the seat is
`FrameLayout` and if it does not the seat is the reader.

`word/settings.xml` is written even though it is empty (`office-corpus/SKILL.md`).
"""
import os, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "fixtures")
os.makedirs(OUT, exist_ok=True)

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')

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
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
<w:style w:type="paragraph" w:styleId="TOC1"><w:name w:val="toc 1"/></w:style>
<w:style w:type="paragraph" w:styleId="Heading1"><w:name w:val="heading 1"/>
  <w:pPr><w:outlineLvl w:val="0"/></w:pPr></w:style>
</w:styles>'''

# An anchored text box 154 pt wide and 265 pt tall at the left margin, square-wrapped. A
# `w:tblpPr` table is NOT an obstacle in this tree by an explicit decision in
# `Paginator.PlaceFloatedTable`, so a fixture built on one never reaches the question --
# see this probe's `results.md` §1.
SHAPE = '''<w:p><w:r><w:drawing>
  <wp:anchor xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
             distT="0" distB="0" distL="114300" distR="114300" simplePos="0" relativeHeight="1"
             behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
    <wp:simplePos x="0" y="0"/>
    <wp:positionH relativeFrom="margin"><wp:posOffset>0</wp:posOffset></wp:positionH>
    <wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>
    <wp:extent cx="1956435" cy="3365500"/>
    <wp:effectExtent l="0" t="0" r="0" b="0"/>
    <wp:wrapSquare wrapText="bothSides"/>
    <wp:docPr id="1" name="QuickLinks"/>
    <a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
      <a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
        <wps:wsp xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
          <wps:cNvSpPr txBox="1"/>
          <wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="1956435" cy="3365500"/></a:xfrm>
            <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
            <a:ln w="6350"><a:solidFill><a:srgbClr val="000000"/></a:solidFill></a:ln></wps:spPr>
          <wps:txbx><w:txbxContent>%s</w:txbxContent></wps:txbx>
          <wps:bodyPr rot="0" anchor="t"/>
        </wps:wsp>
      </a:graphicData>
    </a:graphic>
  </wp:anchor>
</w:drawing></w:r></w:p>'''
BOXLINE = '<w:p><w:r><w:t>QUICK LINK %d</w:t></w:r></w:p>'

# A floating table: one column 2.1417 in wide, anchored left, which is `absrc`'s `Table2`.
FLOATER = '''<w:tbl>
  <w:tblPr>
    <w:tblpPr w:leftFromText="180" w:rightFromText="180" w:vertAnchor="text"
              w:horzAnchor="margin" w:tblpX="1" w:tblpY="1"/>
    <w:tblW w:w="3084" w:type="dxa"/>
    <w:tblBorders><w:top w:val="single" w:sz="4"/><w:left w:val="single" w:sz="4"/>
      <w:bottom w:val="single" w:sz="4"/><w:right w:val="single" w:sz="4"/></w:tblBorders>
  </w:tblPr>
  <w:tblGrid><w:gridCol w:w="3084"/></w:tblGrid>
  %s
</w:tbl>'''
ROW = ('<w:tr><w:tc><w:tcPr><w:tcW w:w="3084" w:type="dxa"/></w:tcPr>'
       '<w:p><w:r><w:t>QUICK LINK %d</w:t></w:r></w:p></w:tc></w:tr>')


def prose(n, tag="PROSE"):
    return "".join(
        f'<w:p><w:r><w:t>{tag} line {i} beside or below the quick links table.</w:t></w:r></w:p>'
        for i in range(1, n + 1))


def toc(entries, tabbed=False):
    """A `TOC` field whose cached result is a run of `toc 1` paragraphs, as Word writes one.

    `tabbed` gives each entry the dot-leader right stop at the full measure that a real Word
    contents entry carries, so that the entry is as wide as the text area whatever it says.
    """
    stop = ('<w:tabs><w:tab w:val="right" w:leader="dot" w:pos="9360"/></w:tabs>'
            if tabbed else '')
    tail = '<w:r><w:tab/></w:r><w:r><w:t>2</w:t></w:r>' if tabbed else ''
    body = "".join(
        f'<w:p><w:pPr><w:pStyle w:val="TOC1"/>{stop}</w:pPr>'
        f'<w:r><w:t>INDEX ENTRY {i}</w:t></w:r>{tail}</w:p>'
        for i in range(1, entries + 1))
    return (
        f'<w:p><w:pPr><w:pStyle w:val="TOC1"/>{stop}</w:pPr>'
        '<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
        '<w:r><w:instrText xml:space="preserve"> TOC \\o "1-3" \\h \\z \\u </w:instrText></w:r>'
        '<w:r><w:fldChar w:fldCharType="separate"/></w:r>'
        f'<w:r><w:t>INDEX ENTRY 0</w:t></w:r>{tail}</w:p>'
        + body
        + f'<w:p><w:pPr><w:pStyle w:val="TOC1"/>{stop}</w:pPr>'
          '<w:r><w:fldChar w:fldCharType="end"/></w:r></w:p>')


def arms():
    rows = "".join(ROW % i for i in range(1, 8))
    table = FLOATER % rows
    shape = SHAPE % "".join(BOXLINE % i for i in range(1, 8))

    # The control both engines already agree on: ordinary paragraphs beside the obstacle.
    yield "plain-after", table + prose(10)

    # The witness: a table of contents where those paragraphs were.
    yield "toc-after", table + toc(9)

    # A heading before the index, as `absrc` has (`INFORMATION HIGHLIGHTS`), so the arm shows
    # whether the heading wraps while the index does not.
    yield "heading-then-toc", (
        table
        + '<w:p><w:pPr><w:pStyle w:val="Heading1"/></w:pPr>'
          '<w:r><w:t>INFORMATION HIGHLIGHTS</w:t></w:r></w:p>'
        + toc(9))

    # And the same heading before plain prose, as the control for that.
    yield "heading-then-plain", (
        table
        + '<w:p><w:pPr><w:pStyle w:val="Heading1"/></w:pPr>'
          '<w:r><w:t>INFORMATION HIGHLIGHTS</w:t></w:r></w:p>'
        + prose(10))


def shaped():
    """The same four arms with an anchored text box, which the obstacle path does see."""
    yield "box-plain-after", SHAPE % "".join(BOXLINE % i for i in range(1, 8)) + prose(10)
    yield "box-toc-after", SHAPE % "".join(BOXLINE % i for i in range(1, 8)) + toc(9)
    yield "box-heading-then-toc", (
        SHAPE % "".join(BOXLINE % i for i in range(1, 8))
        + '<w:p><w:pPr><w:pStyle w:val="Heading1"/><w:jc w:val="center"/></w:pPr>'
          '<w:r><w:t>INFORMATION HIGHLIGHTS</w:t></w:r></w:p>'
        + toc(9))
    # The entries carry the full-measure right stop a real Word contents entry has, so each is
    # as wide as the text area and cannot fit beside the box on its own terms.
    yield "box-toc-tabbed", SHAPE % "".join(BOXLINE % i for i in range(1, 8)) + toc(9, tabbed=True)
    yield "box-plain-tabbed", (
        SHAPE % "".join(BOXLINE % i for i in range(1, 8))
        + "".join('<w:p><w:pPr><w:tabs><w:tab w:val="right" w:leader="dot" w:pos="9360"/></w:tabs>'
                  '</w:pPr><w:r><w:t>PROSE line %d</w:t></w:r><w:r><w:tab/></w:r>'
                  '<w:r><w:t>2</w:t></w:r></w:p>' % i for i in range(1, 11)))

    # Longer than the room beside the box, so the section cannot fit in the narrowed measure —
    # which is the conditional form of the hypothesis the six short arms refute.
    yield "box-toc-long", SHAPE % "".join(BOXLINE % i for i in range(1, 8)) + toc(30)
    yield "box-plain-long", SHAPE % "".join(BOXLINE % i for i in range(1, 8)) + prose(30)

    yield "box-heading-then-plain", (
        SHAPE % "".join(BOXLINE % i for i in range(1, 8))
        + '<w:p><w:pPr><w:pStyle w:val="Heading1"/><w:jc w:val="center"/></w:pPr>'
          '<w:r><w:t>INFORMATION HIGHLIGHTS</w:t></w:r></w:p>'
        + prose(10))


def build(name, body):
    doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {NS}><w:body>'
           + body +
           '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
           ' w:header="720" w:footer="720"/></w:sectPr></w:body></w:document>')
    path = os.path.join(OUT, f"{name}.docx")
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
    n = 0
    for name, body in list(arms()) + list(shaped()):
        build(name, body)
        n += 1
    print("built", n, "fixtures in", OUT)
