"""A group's own `rot`, `flipH` and `flipV`, one probe per case.

Writes `<dir>/O_*.docx`. Each is one anchored drawing holding a nested `wpg:grpSp` that states the
orientation under test, a grey member filling the group -- so that what the members cover is the
group's own rectangle and the fit to `wp:extent` is the identity -- and a gold, text-bearing mark
in the middle, which is what says where the orientation put it and which way up.

Render both ways and read the mark off the raster by colour with `measure.py`; the mark's own
"ABC" says whether the text turned with the group, which is the one place the reference and the
arithmetic part company.
"""

import os
import sys
import zipfile

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
<wp:extent cx="{w}" cy="{h}"/><wp:wrapNone/>
<wp:docPr id="1" name="G"/>
<a:graphic><a:graphicData><wpg:wgp>
<wpg:grpSpPr><a:xfrm{top}><a:off x="0" y="0"/><a:ext cx="{w}" cy="{h}"/>
<a:chOff x="0" y="0"/><a:chExt cx="{w}" cy="{h}"/></a:xfrm></wpg:grpSpPr>
{body}
</wpg:wgp></a:graphicData></a:graphic>
</wp:anchor></w:drawing></w:r></w:p>
<w:sectPr><w:pgSz w:w="16838" w:h="11906" w:orient="landscape"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""

W, H = 5080000, 2540000          # 400 x 200 pt


def shape(x, y, cx, cy, colour, text=None):
    body = (f"<wps:txbx><w:txbxContent><w:p><w:r><w:t>{text}</w:t></w:r></w:p></w:txbxContent></wps:txbx>"
            if text else "")
    return (f'<wps:wsp><wps:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
            f'<a:solidFill><a:srgbClr val="{colour}"/></a:solidFill></wps:spPr>{body}<wps:bodyPr/></wps:wsp>')


def group(attributes, inner):
    return (f'<wpg:grpSp><wpg:grpSpPr><a:xfrm{attributes}><a:off x="0" y="0"/>'
            f'<a:ext cx="{W}" cy="{H}"/><a:chOff x="0" y="0"/><a:chExt cx="{W}" cy="{H}"/>'
            f"</a:xfrm></wpg:grpSpPr>{inner}</wpg:grpSp>")


def write(folder, name, body, top=""):
    os.makedirs(folder, exist_ok=True)
    with zipfile.ZipFile(os.path.join(folder, name + ".docx"), "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/document.xml", DOC.format(w=W, h=H, body=body, top=top))


def main(folder):
    filler = shape(0, 0, W, H, "DDDDDD")
    mark = shape(W // 4, 3 * H // 8, W // 2, H // 4, "FFCC00", "ABC")

    cases = {
        "O_plain": "",
        "O_rot90": ' rot="5400000"',
        "O_rot180": ' rot="10800000"',
        "O_rot270": ' rot="16200000"',
        "O_flipH": ' flipH="1"',
        "O_flipV": ' flipV="1"',
        "O_rot180_flipH": ' rot="10800000" flipH="1"',
    }
    for name, attributes in cases.items():
        write(folder, name, group(attributes, filler + mark))

    # A flip inside a flip, which composes to a half turn of the positions and to nothing at all
    # for the text -- the case `004_Free_Genogram_Diagram_Template_Editable_Format` states.
    write(folder, "O_flipH_in_flipV",
          group(' flipV="1"', group(' flipH="1"', filler + mark)))

    # And the corner case: a `rot` on the *outermost* `wpg:wgp` is dropped by LibreOffice --
    # the anchor's fly frame cannot turn -- so the same orientation has to be stated on a nested
    # group to be seen at all. This one must come back identical to `O_plain`.
    write(folder, "O_rot90_at_top", filler + mark, top=' rot="5400000"')
    print(f"written to {folder}")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else ".")
