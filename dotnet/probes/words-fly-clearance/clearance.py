"""Does a fly filling its column push the empty paragraphs after it down, or only the inked ones?

Writes one document per empty-paragraph count, renders it with the reference and with ours, and
reads back where the single inked paragraph -- `AFTER` -- landed.

    AFTER at the fly's bottom, whatever N            -> only inked blocks are displaced
    AFTER at the fly's bottom + N line heights       -> the whole flow is displaced

Usage: XRAY_OFFICE_CLI=<cli> python3 clearance.py <outdir>
"""
import os, re, subprocess, sys, zipfile
from pathlib import Path

OUT = Path(sys.argv[1]); OUT.mkdir(parents=True, exist_ok=True)
CLI = os.environ["XRAY_OFFICE_CLI"]

# A4 at 72 pt margins: 11906 - 1440 - 1440 twips of column, and a 4000-twip row is 200 pt tall.
GRID, ROW, TBLPY = 9026, 4000, 1440

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

def document(empties):
    blanks = "<w:p/>" * empties
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
    <w:trPr><w:trHeight w:val="{ROW}" w:hRule="exact"/></w:trPr>
    <w:tc><w:tcPr><w:tcW w:w="{GRID}" w:type="dxa"/></w:tcPr>
     <w:p><w:r><w:t>CELL</w:t></w:r></w:p></w:tc>
   </w:tr>
  </w:tbl>
  {blanks}
  <w:p><w:r><w:t>AFTER</w:t></w:r></w:p>
  <w:sectPr>
   <w:pgSz w:w="11906" w:h="16838"/>
   <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
            w:header="708" w:footer="708" w:gutter="0"/>
  </w:sectPr>
 </w:body>
</w:document>"""

def write(path, empties):
    with zipfile.ZipFile(path, "w") as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/document.xml", document(empties))

def word_at(pdf, want):
    r = subprocess.run(["pdftotext", "-bbox", str(pdf), "-"], capture_output=True, text=True)
    for m in re.finditer(
            r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="[\d.]+" yMax="[\d.]+">([^<]*)</word>',
            r.stdout):
        if m.group(3) == want:
            return round(float(m.group(1)), 2), round(float(m.group(2)), 2)
    return None

print(f"{'empties':>7}  {'reference AFTER':>16}  {'ours AFTER':>16}")
for n in (0, 1, 3, 6):
    src = OUT / f"clear{n}.docx"
    write(src, n)

    here = OUT / f"r{n}"; here.mkdir(exist_ok=True)
    subprocess.run(["soffice", f"-env:UserInstallation=file://{here / 'prof'}", "--headless",
                    "--convert-to", "pdf", "--outdir", str(here), str(src)],
                   capture_output=True, timeout=300)
    subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(OUT / f"o{n}")],
                   capture_output=True, timeout=300)

    ref = word_at(here / f"clear{n}.pdf", "AFTER")
    ours = word_at(OUT / f"o{n}" / f"clear{n}.pdf", "AFTER")
    print(f"{n:>7}  {str(ref):>16}  {str(ours):>16}", flush=True)
