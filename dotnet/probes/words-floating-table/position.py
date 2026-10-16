import os, sys, zipfile

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

W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main"

CELL = """<w:tc><w:tcPr><w:tcW w:w="{w}" w:type="dxa"/></w:tcPr>
<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr>
<w:r><w:t>{t}</w:t></w:r></w:p></w:tc>"""

DOC = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="{ns}"><w:body>
<w:tbl><w:tblPr>{pos}<w:tblW w:w="{tw}" w:type="dxa"/>
<w:tblBorders>
<w:top w:val="single" w:sz="8" w:space="0" w:color="000000"/>
<w:left w:val="single" w:sz="8" w:space="0" w:color="000000"/>
<w:bottom w:val="single" w:sz="8" w:space="0" w:color="000000"/>
<w:right w:val="single" w:sz="8" w:space="0" w:color="000000"/>
<w:insideH w:val="single" w:sz="8" w:space="0" w:color="000000"/>
<w:insideV w:val="single" w:sz="8" w:space="0" w:color="000000"/>
</w:tblBorders></w:tblPr>
<w:tblGrid><w:gridCol w:w="{cw}"/><w:gridCol w:w="{cw}"/></w:tblGrid>
<w:tr><w:trPr><w:trHeight w:val="600" w:hRule="exact"/></w:trPr>{c1}{c2}</w:tr>
<w:tr><w:trPr><w:trHeight w:val="600" w:hRule="exact"/></w:trPr>{c3}{c4}</w:tr>
</w:tbl>
<w:p><w:r><w:t>AFTER</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="708" w:footer="708" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""


def write(folder, name, pos, grid=2000):
    os.makedirs(folder, exist_ok=True)
    cells = [CELL.format(w=grid, t=t) for t in ("A", "B", "C", "D")]
    xml = DOC.format(ns=W, pos=pos, tw=grid * 2, cw=grid,
                     c1=cells[0], c2=cells[1], c3=cells[2], c4=cells[3])
    with zipfile.ZipFile(os.path.join(folder, name + ".docx"), "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/document.xml", xml)


def main(d):
    cases = {
        "T_flow": "",
        "T_pageY": '<w:tblpPr w:vertAnchor="page" w:tblpY="1440"/>',
        "T_marginX": '<w:tblpPr w:horzAnchor="margin" w:tblpX="-594" w:tblpY="1"/>',
        "T_pageX": '<w:tblpPr w:horzAnchor="page" w:tblpX="1440" w:tblpY="1"/>',
        "T_center": '<w:tblpPr w:horzAnchor="margin" w:tblpXSpec="center" w:tblpY="1"/>',
        "T_textY": '<w:tblpPr w:vertAnchor="text" w:tblpY="720"/>',
        "T_pageBoth": '<w:tblpPr w:vertAnchor="page" w:horzAnchor="margin" '
                      'w:tblpX="-594" w:tblpY="1025"/>',
    }
    for name, pos in cases.items():
        write(d, name, pos)

    # A fly as wide as the whole text area, where "beside" is impossible.
    write(d, "T_wide", '<w:tblpPr w:vertAnchor="page" w:horzAnchor="margin" w:tblpY="1440"/>',
          grid=4513)
    write(d, "T_wideflow", "", grid=4513)
    print("written", len(cases))


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else ".")
