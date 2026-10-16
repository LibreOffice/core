"""The whole law in one geometry: what a fly filling its column does to the flow after it.

One page, one fly (`vertAnchor="page" tblpY="953"`, a single exact row), and after it a run of
paragraphs. Each paragraph is one of

    .   empty
    @   empty but for an anchored text box reading `MARK` at posOffset 0 from the paragraph
    A   the inked paragraph, reading `AFTER`

The fly is 10598 twips wide in a 9922-twip column, so it fills it and overflows -- there is no
strip beside it to wrap into, which is the case `Paginator.FillsTheColumn` answers for.

Observables, both read out of the PDF text layer with `pdftotext -bbox`: `MARK`'s y, which is the
anchored frame's own position, and `AFTER`'s y, which is where the flow reached.

Usage: XRAY_OFFICE_CLI=<cli> python3 law.py <outdir>
"""
import os, re, subprocess, sys, zipfile
from pathlib import Path

OUT = Path(sys.argv[1]); OUT.mkdir(parents=True, exist_ok=True)
CLI = os.environ["XRAY_OFFICE_CLI"]
GRID, TBLPY = 10598, 953

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

def document(row, shape):
    flow = "".join({".": "<w:p/>", "@": f"<w:p>{ANCHOR}</w:p>",
                    "A": "<w:p><w:r><w:t>AFTER</w:t></w:r></w:p>"}[c] for c in shape)
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
  {flow}
  <w:sectPr>
   <w:pgSz w:w="11906" w:h="16838"/>
   <w:pgMar w:top="1134" w:right="991" w:bottom="993" w:left="993"
            w:header="993" w:footer="720" w:gutter="0"/>
  </w:sectPr>
 </w:body>
</w:document>"""

def write(path, row, shape):
    with zipfile.ZipFile(path, "w") as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/document.xml", document(row, shape))

def find(pdf, want):
    r = subprocess.run(["pdftotext", "-bbox", str(pdf), "-"], capture_output=True, text=True)
    page = 0
    for m in re.finditer(r'<page |<word xMin="[\d.]+" yMin="([\d.]+)"[^>]*>([^<]*)</word>', r.stdout):
        if m.group(0).startswith("<page"):
            page += 1
        elif m.group(2) == want:
            return f"p{page} {float(m.group(1)):.1f}"
    return "-"

print(f"{'row':>5} {'shape':>8} | {'ref MARK':>10} {'ref AFTER':>10} | {'our MARK':>10} {'our AFTER':>10}")
for row in (4000, 12000, 14200):
    for shape in ("A", ".A", "...A", "@A", "@..A", ".@A", "..@A"):
        tag = f"{row}_{shape.replace('.', 'e').replace('@', 'm')}"
        src = OUT / f"law{tag}.docx"
        write(src, row, shape)
        here = OUT / f"r{tag}"; here.mkdir(exist_ok=True)
        subprocess.run(["soffice", f"-env:UserInstallation=file://{here / 'prof'}", "--headless",
                        "--convert-to", "pdf", "--outdir", str(here), str(src)],
                       capture_output=True, timeout=300)
        subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(OUT / f"o{tag}")],
                       capture_output=True, timeout=300)
        rp, op = here / f"law{tag}.pdf", OUT / f"o{tag}" / f"law{tag}.pdf"
        print(f"{row / 20:>5.0f} {shape:>8} | {find(rp, 'MARK'):>10} {find(rp, 'AFTER'):>10} | "
              f"{find(op, 'MARK'):>10} {find(op, 'AFTER'):>10}", flush=True)
