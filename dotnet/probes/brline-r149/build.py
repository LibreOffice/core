#!/usr/bin/env python3
"""Hand-built minimal DOCX fixtures for the O91 break-line rule.

Each fixture is ONE page holding ONE `wps:wsp` at a known position and size, top-anchored with
zero insets unless the arm says otherwise, whose body is N runs carrying nothing but `<w:br/>`
at `w:sz=BSZ` followed by one run of `Hxy` at 11 pt.  A ruler run is drawn in the page body at a
fixed place so an absolute origin is never needed.

**Every fixture carries `word/settings.xml`.**  `dotnet/CLAUDE.md` records an agent getting five
clean, mutually corroborating and entirely wrong answers from hand-built DOCX that omitted it:
without that part LibreOffice does not apply its OOXML compatibility defaults.
"""
import pathlib
import sys
import zipfile

SHAPE_X, SHAPE_Y = 0, 0                 # EMU, relative to the page's text origin
SHAPE_CX = 4114800                      # 324 pt
SHAPE_CY = 3657600                      # 288 pt -- big enough that nothing overflows

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

DOCRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>'''

SETTINGS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:compat><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>
</w:settings>'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="DejaVu Sans" w:hAnsi="DejaVu Sans" w:cs="DejaVu Sans"/><w:sz w:val="22"/><w:szCs w:val="22"/></w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr><w:spacing w:after="0" w:line="%(LINE)s" w:lineRule="%(RULE)s"/></w:pPr></w:pPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:qFormat/></w:style>
</w:styles>'''

DOC = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:wpc="http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas" xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math" xmlns:wp14="http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing" xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape" mc:Ignorable="w14 wp14">
<w:body>
<w:p><w:r><w:t>RULER</w:t></w:r>
<w:r><mc:AlternateContent><mc:Choice Requires="wps">
<w:drawing><wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="251659264" behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
<wp:simplePos x="0" y="0"/>
<wp:positionH relativeFrom="column"><wp:posOffset>%(X)d</wp:posOffset></wp:positionH>
<wp:positionV relativeFrom="paragraph"><wp:posOffset>%(Y)d</wp:posOffset></wp:positionV>
<wp:extent cx="%(CX)d" cy="%(CY)d"/><wp:effectExtent l="0" t="0" r="0" b="0"/><wp:wrapNone/>
<wp:docPr id="1" name="Rectangle 1"/><wp:cNvGraphicFramePr/>
<a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
<wps:wsp><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="%(CX)d" cy="%(CY)d"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/><a:ln><a:solidFill><a:srgbClr val="000000"/></a:solidFill></a:ln></wps:spPr>
<wps:txbx><w:txbxContent>%(BODY)s</w:txbxContent></wps:txbx>
<wps:bodyPr rot="0" spcFirstLastPara="0" %(VOF)s horzOverflow="overflow" vert="horz" wrap="square" lIns="0" tIns="%(TINS)d" rIns="0" bIns="%(TINS)d" numCol="1" spcCol="0" rtlCol="0" fromWordArt="0" anchor="%(ANCHOR)s" anchorCtr="0" forceAA="0" compatLnSpc="1"><a:noAutofit/></wps:bodyPr>
</wps:wsp></a:graphicData></a:graphic></wp:anchor></w:drawing>
</mc:Choice></mc:AlternateContent></w:r></w:p>
%(AFTER)s
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/><w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" w:header="0" w:footer="0" w:gutter="0"/></w:sectPr>
</w:body></w:document>'''


def rpr(sz=None, font=None):
    bits = ''
    if font:
        bits += '<w:rFonts w:ascii="%s" w:hAnsi="%s" w:cs="%s"/>' % (font, font, font)
    if sz is not None:
        bits += '<w:sz w:val="%d"/><w:szCs w:val="%d"/>' % (sz, sz)
    return '<w:rPr>%s</w:rPr>' % bits if bits else ''


def body(n_breaks, bsz, marksz=None, tail='Hxy', tailsz=None, bfont=None,
         line=None, rule=None, leading_char=''):
    ppr = '<w:pPr>'
    if line:
        ppr += '<w:spacing w:after="0" w:line="%s" w:lineRule="%s"/>' % (line, rule)
    if marksz is not None:
        ppr += '<w:rPr><w:sz w:val="%d"/><w:szCs w:val="%d"/></w:rPr>' % (marksz, marksz)
    ppr += '</w:pPr>'
    runs = ''
    for i in range(n_breaks):
        text = ('<w:t xml:space="preserve">%s</w:t>' % leading_char) if (i == 0 and leading_char) else ''
        runs += '<w:r>%s%s<w:br/></w:r>' % (rpr(bsz, bfont), text)
    runs += '<w:r>%s<w:t>%s</w:t></w:r>' % (rpr(tailsz), tail)
    return '<w:p>%s%s</w:p>' % (ppr, runs)


def write(path, doc_body, after='', anchor='t', tins=0, cy=SHAPE_CY, vof='vertOverflow="overflow"',
          line='240', rule='auto'):
    doc = DOC % dict(X=SHAPE_X, Y=SHAPE_Y, CX=SHAPE_CX, CY=cy, BODY=doc_body, ANCHOR=anchor,
                     TINS=tins, AFTER=after, VOF=vof)
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/_rels/document.xml.rels', DOCRELS)
        z.writestr('word/settings.xml', SETTINGS)
        z.writestr('word/styles.xml', STYLES % dict(LINE=line, RULE=rule))
        z.writestr('word/document.xml', doc)


def main():
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'fixtures3')
    out.mkdir(parents=True, exist_ok=True)
    made = []
    # A -- how many 2 pt break lines
    for n in range(0, 7):
        write(out / ('n%d.docx' % n), body(n, 4)); made.append('n%d' % n)
    # B -- one break, the break RUN's size swept
    for sz in (2, 4, 8, 12, 16, 20, 22, 24, 32, 40, 48, 60, 80):
        write(out / ('b%02d.docx' % sz), body(1, sz)); made.append('b%02d' % sz)
    # B2 -- TWO breaks, same sweep: isolates a first-line effect from a per-line one
    for sz in (2, 4, 8, 16, 22, 40, 80):
        write(out / ('d%02d.docx' % sz), body(2, sz)); made.append('d%02d' % sz)
    # C -- the paragraph MARK's size, one 2 pt break
    for sz in (2, 4, 22, 40, 80, 120):
        write(out / ('m%03d.docx' % sz), body(1, 4, marksz=sz)); made.append('m%03d' % sz)
    # D -- the break run carries a leading character, so the line has a real portion too
    for sz in (4, 40, 80):
        write(out / ('c%02d.docx' % sz), body(1, sz, leading_char='A')); made.append('c%02d' % sz)
    # E -- the run AFTER the break, to check the break line does not take the next run's size
    for sz in (4, 40, 80):
        write(out / ('t%02d.docx' % sz), body(1, 4, tailsz=sz)); made.append('t%02d' % sz)
    # F -- line-spacing rule, one 2 pt break
    for tag, (ln, rl) in {'auto240': ('240', 'auto'), 'auto259': ('259', 'auto'),
                          'auto480': ('480', 'auto'), 'exact240': ('240', 'exact'),
                          'atleast480': ('480', 'atLeast')}.items():
        write(out / ('l-%s.docx' % tag), body(1, 4, line=ln, rule=rl), line=ln, rule=rl)
        made.append('l-%s' % tag)
    # G -- the break run's FACE
    for tag, face in (('dejavu', 'DejaVu Sans'), ('libsans', 'Liberation Sans'),
                      ('libmono', 'Liberation Mono'), ('freeserif', 'FreeSerif')):
        write(out / ('f-%s.docx' % tag), body(1, 40, bfont=face)); made.append('f-%s' % tag)
    # H -- the same paragraph in the PAGE BODY rather than in a shape
    for n in (0, 1, 2):
        after = body(n, 4).replace('Hxy', 'Pgy')
        write(out / ('p%d.docx' % n), body(0, 4), after=after); made.append('p%d' % n)
    print('\n'.join(made))
    print('%d fixtures' % len(made))


if __name__ == '__main__':
    main()
