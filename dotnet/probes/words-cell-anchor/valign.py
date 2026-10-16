"""Does a cell's vertical alignment survive an anchored object sitting in it?

One table, one row 200 pt tall, one cell whose `w:vAlign` is swept. The cell holds a paragraph
carrying a text run reading `TEXT` and — in most cases — an anchored text box reading `MARK` at
`positionV relativeFrom="paragraph" posOffset="0"`. `TEXT`'s y says where the alignment put the
paragraph; `MARK`'s says where the frame went.

Three dimensions are swept because Writer's rule turns on all three (`#i43913#`,
`sw/source/core/layout/tabfrm.cxx`:6270-6330): the alignment, the object's wrap, and whether the
object overlaps the cell at all. Both reference binaries are measured, because the guard's Word
compatibility half — `FORCE_TOP_ALIGNMENT_IN_CELL_WITH_FLOATING_ANCHOR`, tdf#166710 — exists only
in 26.2.

Usage: XRAY_OFFICE_CLI=<cli> python3 valign.py <outdir>
"""
import os, re, subprocess, sys, zipfile
from pathlib import Path

OUT = Path(sys.argv[1]); OUT.mkdir(parents=True, exist_ok=True)
CLI = os.environ["XRAY_OFFICE_CLI"]
OLD = "soffice"                                        # the distro's 24.2.7.2
NEW = "/opt/libreoffice26.2/program/soffice"           # the TDF tarball's 26.2.4.2
ROW = 4000                                             # twips, 200 pt

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
 <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
 <Default Extension="xml" ContentType="application/xml"/>
 <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
 <Relationship Id="rId1" Target="word/document.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
</Relationships>"""

WRAPS = {
    "none": "<wp:wrapNone/>",
    "square": '<wp:wrapSquare wrapText="bothSides"/>',
    "topAndBottom": "<wp:wrapTopAndBottom/>",
    "through": '<wp:wrapThrough wrapText="bothSides"><wp:wrapPolygon edited="0">'
               '<wp:start x="0" y="0"/><wp:lineTo x="0" y="21600"/>'
               '<wp:lineTo x="21600" y="21600"/><wp:lineTo x="21600" y="0"/>'
               '<wp:lineTo x="0" y="0"/></wp:wrapPolygon></wp:wrapThrough>',
}

def anchor(wrap, away):
    """The text box. `away` puts it below the table instead of inside the cell."""
    where = ('<wp:positionV relativeFrom="page"><wp:posOffset>8000000</wp:posOffset></wp:positionV>'
             if away else
             '<wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>')
    return f"""<w:r><w:drawing>
 <wp:anchor xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
            distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="1"
            behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
  <wp:simplePos x="0" y="0"/>
  <wp:positionH relativeFrom="column"><wp:posOffset>1600200</wp:posOffset></wp:positionH>
  {where}
  <wp:extent cx="1143000" cy="228600"/>
  {WRAPS[wrap]}
  <wp:docPr id="1" name="Mark"/>
  <a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
   <a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
    <wps:wsp xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
     <wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="1143000" cy="228600"/></a:xfrm>
      <a:prstGeom prst="rect"><a:avLst/></a:prstGeom></wps:spPr>
     <wps:txbx><w:txbxContent><w:p><w:r><w:t>MARK</w:t></w:r></w:p></w:txbxContent></wps:txbx>
     <wps:bodyPr/>
    </wps:wsp>
   </a:graphicData>
  </a:graphic>
 </wp:anchor>
</w:drawing></w:r>"""

def document(valign, wrap, away):
    drawing = "" if wrap is None else anchor(wrap, away)
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
 <w:body>
  <w:tbl>
   <w:tblPr><w:tblW w:w="8000" w:type="dxa"/>
    <w:tblBorders><w:top w:val="single" w:sz="4"/><w:bottom w:val="single" w:sz="4"/>
     <w:left w:val="single" w:sz="4"/><w:right w:val="single" w:sz="4"/></w:tblBorders>
    <w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/></w:tblCellMar>
   </w:tblPr>
   <w:tblGrid><w:gridCol w:w="8000"/></w:tblGrid>
   <w:tr><w:trPr><w:trHeight w:val="{ROW}" w:hRule="exact"/></w:trPr>
    <w:tc><w:tcPr><w:tcW w:w="8000" w:type="dxa"/><w:vAlign w:val="{valign}"/></w:tcPr>
     <w:p>{drawing}<w:r><w:t>TEXT</w:t></w:r></w:p></w:tc>
   </w:tr>
  </w:tbl>
  <w:sectPr>
   <w:pgSz w:w="11906" w:h="16838"/>
   <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
            w:header="708" w:footer="708" w:gutter="0"/>
  </w:sectPr>
 </w:body>
</w:document>"""

def write(path, valign, wrap, away):
    with zipfile.ZipFile(path, "w") as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/document.xml", document(valign, wrap, away))

def word_y(pdf, want):
    if not Path(pdf).exists(): return None
    r = subprocess.run(["pdftotext", "-bbox", str(pdf), "-"], capture_output=True, text=True)
    for m in re.finditer(r'<word xMin="[\d.]+" yMin="([\d.]+)"[^>]*>([^<]*)</word>', r.stdout):
        if m.group(2) == want: return round(float(m.group(1)), 2)
    return None

def render(binary, src, folder):
    folder.mkdir(exist_ok=True)
    out = folder / (src.stem + ".pdf")
    if not out.exists():
        subprocess.run([binary, f"-env:UserInstallation=file://{folder / 'prof'}", "--headless",
                        "--convert-to", "pdf", "--outdir", str(folder), str(src)],
                       capture_output=True, timeout=300)
    return out

CASES = [(v, w, a)
         for v in ("top", "center", "bottom")
         for w, a in [(None, False), ("none", False), ("square", False),
                      ("topAndBottom", False), ("through", False), ("none", True)]]

print(f"{'vAlign':>7} {'wrap':>13} {'where':>7} | {'24.2 TEXT':>9} | {'26.2 TEXT':>9} "
      f"| {'our TEXT':>9} {'our MARK':>9}")
for valign, wrap, away in CASES:
    tag = f"{valign}_{wrap or 'nodraw'}_{'away' if away else 'in'}"
    src = OUT / f"v{tag}.docx"
    write(src, valign, wrap, away)
    old = render(OLD, src, OUT / f"a{tag}")
    new = render(NEW, src, OUT / f"b{tag}")
    subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(OUT / f"o{tag}")],
                   capture_output=True, timeout=300)
    ours = OUT / f"o{tag}" / f"v{tag}.pdf"
    print(f"{valign:>7} {str(wrap or '(none)'):>13} {('away' if away else 'in'):>7} | "
          f"{str(word_y(old, 'TEXT')):>9} | {str(word_y(new, 'TEXT')):>9} | "
          f"{str(word_y(ours, 'TEXT')):>9} {str(word_y(ours, 'MARK')):>9}", flush=True)
