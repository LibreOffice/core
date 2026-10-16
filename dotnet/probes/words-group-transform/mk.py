import zipfile, os, sys

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOC = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape"
 xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup">
<w:body><w:p><w:r><w:drawing>
<wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="1"
 behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
<wp:simplePos x="0" y="0"/>
<wp:positionH relativeFrom="page"><wp:posOffset>1270000</wp:posOffset></wp:positionH>
<wp:positionV relativeFrom="page"><wp:posOffset>635000</wp:posOffset></wp:positionV>
<wp:extent cx="{ax}" cy="{ay}"/><wp:wrapNone/>
<wp:docPr id="1" name="G"/>
<a:graphic><a:graphicData><wpg:wgp>
<wpg:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{gx}" cy="{gy}"/>
<a:chOff x="{cox}" y="{coy}"/><a:chExt cx="{cx}" cy="{cy}"/></a:xfrm></wpg:grpSpPr>
{body}
</wpg:wgp></a:graphicData></a:graphic>
</wp:anchor></w:drawing></w:r></w:p>
<w:sectPr><w:pgSz w:w="16838" w:h="11906" w:orient="landscape"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""

def sq(colour, x, y, cx, cy):
    return (f'<wps:wsp><wps:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
            f'<a:solidFill><a:srgbClr val="{colour}"/></a:solidFill></wps:spPr></wps:wsp>')

def grp(x, y, cx, cy, chx, chy, inner, chox=0, choy=0):
    return (f'<wpg:grpSp><wpg:grpSpPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/>'
            f'<a:chOff x="{chox}" y="{choy}"/><a:chExt cx="{chx}" cy="{chy}"/></a:xfrm></wpg:grpSpPr>'
            f'{inner}</wpg:grpSp>')

def write(name, body, ax=5080000, ay=2540000, gx=5080000, gy=2540000,
          cx=10160000, cy=5080000, cox=0, coy=0):
    xml = DOC.format(ax=ax, ay=ay, gx=gx, gy=gy, cx=cx, cy=cy, cox=cox, coy=coy, body=body)
    out = os.path.join(sys.argv[1], name + ".docx")
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/document.xml", xml)
    return out

R, B = "FF0000", "0000FF"
U = 1270000            # 100 pt
H = 635000             # 50 pt

# A: one small child at the child-space origin; children union is a quarter of chExt.
write("A_small", sq(R, 0, 0, U, H))
# B: the child exactly fills chExt.
write("B_exact", sq(R, 0, 0, 10160000, 5080000))
# C: one small child away from the child-space origin.
write("C_offset", sq(R, 2540000, 1270000, U, H))
# D: two children, union half of chExt on both axes.
write("D_two", sq(R, 0, 0, U, H) + sq(B, 5080000, 2540000, U, H))
# E: two children, union half across and a quarter down.
write("E_aniso", sq(R, 0, 0, U, H) + sq(B, 5080000, 635000, U, H))
# F: the nested case, restated.
write("F_nested", sq(R, 0, 0, U, H) + grp(5080000, 2540000, 5080000, 2540000, 5080000, 2540000,
                                          sq(B, U, H, U, H)))
print("written")

# G: the group's own a:ext is half the anchor extent; the child fills chExt.
write("G_extdiff", sq(R, 0, 0, 10160000, 5080000), gx=2540000, gy=1270000)
# H: the child overflows chExt.
write("H_big", sq(R, 0, 0, 20320000, 10160000))
# I: no chExt at all.
write("I_nochext", sq(R, 0, 0, U, H), cx=0, cy=0)
