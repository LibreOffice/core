"""Where does a paragraph whose only content is a frame anchor go, when a fly fills the column?

The same shape as `clearance.py`, with the empty paragraphs replaced by one carrying an anchored
text box reading `MARK` at `positionV relativeFrom="paragraph" posOffset="0"`. `MARK`'s y is
therefore its anchor paragraph's own top, which is the thing `clearance.py` cannot see.

The fly's height is swept, because that is the variable `clearance.py` and
`HC-Bulletin-template.docx` disagree about: a short fly leaves room below it on the page, a tall
one does not.

Usage: XRAY_OFFICE_CLI=<cli> python3 anchored.py <outdir>
"""
import os, re, subprocess, sys, zipfile
from pathlib import Path

OUT = Path(sys.argv[1]); OUT.mkdir(parents=True, exist_ok=True)
CLI = os.environ["XRAY_OFFICE_CLI"]

GRID, TBLPY = 10598, 953   # wider than the 9922-twip column, as HC-Bulletin's fly is

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

# 2 inch by half an inch, at the anchor paragraph's own top-left.
ANCHOR = """<w:r><w:drawing>
 <wp:anchor xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
            distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="1"
            behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
  <wp:simplePos x="0" y="0"/>
  <wp:positionH relativeFrom="column"><wp:posOffset>0</wp:posOffset></wp:positionH>
  <wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>
  <wp:extent cx="1828800" cy="457200"/>
  <wp:wrapNone/>
  <wp:docPr id="1" name="Mark"/>
  <a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
   <a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
    <wps:wsp xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
     <wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="1828800" cy="457200"/></a:xfrm>
      <a:prstGeom prst="rect"><a:avLst/></a:prstGeom></wps:spPr>
     <wps:txbx><w:txbxContent><w:p><w:r><w:t>MARK</w:t></w:r></w:p></w:txbxContent></wps:txbx>
     <wps:bodyPr/>
    </wps:wsp>
   </a:graphicData>
  </a:graphic>
 </wp:anchor>
</w:drawing></w:r>"""

def document(row):
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
 <w:body>
  <w:tbl>
   <w:tblPr>
    <w:tblpPr w:vertAnchor="page" w:horzAnchor="margin" w:tblpY="{TBLPY}"/>
    <w:tblW w:w="{GRID}" w:type="dxa"/>
   </w:tblPr>
   <w:tblGrid><w:gridCol w:w="{GRID}"/></w:tblGrid>
   <w:tr>
    <w:trPr><w:trHeight w:val="{row}" w:hRule="exact"/></w:trPr>
    <w:tc><w:tcPr><w:tcW w:w="{GRID}" w:type="dxa"/></w:tcPr>
     <w:p><w:r><w:t>CELL</w:t></w:r></w:p></w:tc>
   </w:tr>
  </w:tbl>
  <w:p>{ANCHOR}</w:p>
  <w:p><w:r><w:t>AFTER</w:t></w:r></w:p>
  <w:sectPr>
   <w:pgSz w:w="11906" w:h="16838"/>
   <w:pgMar w:top="1134" w:right="991" w:bottom="993" w:left="993"
            w:header="993" w:footer="720" w:gutter="0"/>
  </w:sectPr>
 </w:body>
</w:document>"""

def write(path, row):
    with zipfile.ZipFile(path, "w") as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/document.xml", document(row))

def find(pdf, want):
    r = subprocess.run(["pdftotext", "-bbox", str(pdf), "-"], capture_output=True, text=True)
    page = 0
    for m in re.finditer(r'<page |<word xMin="([\d.]+)" yMin="([\d.]+)"[^>]*>([^<]*)</word>', r.stdout):
        if m.group(0).startswith("<page"):
            page += 1
        elif m.group(3) == want:
            return page, round(float(m.group(2)), 1)
    return None

print(f"{'row (pt)':>9}  {'ref MARK':>14}  {'ref AFTER':>14}  {'our MARK':>14}  {'our AFTER':>14}")
for row in (4000, 12000, 14200):
    src = OUT / f"anch{row}.docx"
    write(src, row)
    here = OUT / f"r{row}"; here.mkdir(exist_ok=True)
    subprocess.run(["soffice", f"-env:UserInstallation=file://{here / 'prof'}", "--headless",
                    "--convert-to", "pdf", "--outdir", str(here), str(src)],
                   capture_output=True, timeout=300)
    subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(OUT / f"o{row}")],
                   capture_output=True, timeout=300)
    rp, op = here / f"anch{row}.pdf", OUT / f"o{row}" / f"anch{row}.pdf"
    print(f"{row / 20:>9.0f}  {str(find(rp, 'MARK')):>14}  {str(find(rp, 'AFTER')):>14}  "
          f"{str(find(op, 'MARK')):>14}  {str(find(op, 'AFTER')):>14}", flush=True)
