"""Where a page-anchored positioned table goes, and what happens when it is taller than the page.

`Case-Study-Heathrow-Airport.docx` states `w:tblpPr w:horzAnchor="page" w:tblpX="705"
w:vertAnchor="text" w:tblpY="662"` on a table that runs to three pages. X-Ray.Office honours neither
offset: `DocxLayoutSource.PositionedLeftEdge` excludes `horzAnchor="page"`, and
`Paginator.PlaceFloatedTable` drops the whole `w:tblpPr` for a table taller than the body.

This measures what the reference actually does, in three dimensions: the horizontal anchor, the
vertical anchor, and whether the table fits on one page. The observables are the x and y of the
first cell's text on each page, read from the PDF text layer.

Usage: python3 place.py <outdir>   (reference only; add XRAY_OFFICE_CLI to score ours too)
"""
import os, re, subprocess, sys, zipfile
from pathlib import Path

OUT = Path(sys.argv[1]); OUT.mkdir(parents=True, exist_ok=True)
CLI = os.environ.get("XRAY_OFFICE_CLI")
OLD = "soffice"
NEW = "/opt/libreoffice26.2/program/soffice"

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

def document(horz, vert, tblpX, tblpY, rows):
    # Every row carries its index so the y of any one of them can be found by name.
    body = "".join(
        f'<w:tr><w:trPr><w:trHeight w:val="400" w:hRule="exact"/></w:trPr>'
        f'<w:tc><w:tcPr><w:tcW w:w="6000" w:type="dxa"/></w:tcPr>'
        f'<w:p><w:r><w:t>R{i:03d}</w:t></w:r></w:p></w:tc></w:tr>'
        for i in range(rows))
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
 <w:body>
  <w:tbl>
   <w:tblPr>
    <w:tblpPr w:leftFromText="180" w:rightFromText="180"
              w:vertAnchor="{vert}" w:horzAnchor="{horz}"
              w:tblpX="{tblpX}" w:tblpY="{tblpY}"/>
    <w:tblW w:w="6000" w:type="dxa"/>
    <w:tblBorders><w:top w:val="single" w:sz="4"/><w:bottom w:val="single" w:sz="4"/>
     <w:left w:val="single" w:sz="4"/><w:right w:val="single" w:sz="4"/>
     <w:insideH w:val="single" w:sz="4"/></w:tblBorders>
   </w:tblPr>
   <w:tblGrid><w:gridCol w:w="6000"/></w:tblGrid>
   {body}
  </w:tbl>
  <w:p/>
  <w:sectPr>
   <w:pgSz w:w="11906" w:h="16838"/>
   <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
            w:header="708" w:footer="708" w:gutter="0"/>
  </w:sectPr>
 </w:body>
</w:document>"""

def write(path, horz, vert, tblpX, tblpY, rows):
    with zipfile.ZipFile(path, "w") as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/document.xml", document(horz, vert, tblpX, tblpY, rows))

def place(pdf, want):
    """(page, x, y) of a word, counting pages as they appear."""
    if not Path(pdf).exists(): return None
    r = subprocess.run(["pdftotext", "-bbox", str(pdf), "-"], capture_output=True, text=True)
    page = 0
    for m in re.finditer(r'<page |<word xMin="([\d.]+)" yMin="([\d.]+)"[^>]*>([^<]*)</word>', r.stdout):
        if m.group(0).startswith("<page"): page += 1
        elif m.group(3) == want:
            return f"p{page} {float(m.group(1)):.1f},{float(m.group(2)):.1f}"
    return "-"

def render(binary, src, folder):
    folder.mkdir(parents=True, exist_ok=True)
    out = folder / (src.stem + ".pdf")
    if not out.exists():
        subprocess.run([binary, f"-env:UserInstallation=file://{folder / 'prof'}", "--headless",
                        "--convert-to", "pdf", "--outdir", str(folder), str(src)],
                       capture_output=True, timeout=600)
    return out

# 400-twip rows: 33 fill a page, so 20 fits and 90 runs to three.
CASES = [(h, v, x, y, n)
         for h, v, x, y in (("page", "text", 705, 662), ("page", "text", 2880, 0),
                            ("margin", "text", 705, 662), ("page", "page", 705, 1440),
                            ("page", "margin", 705, 662))
         for n in (20, 90)]

print(f"{'horz':>7} {'vert':>7} {'tblpX':>6} {'tblpY':>6} {'rows':>5} | "
      f"{'24.2 first':>14} {'24.2 R033':>14} | {'26.2 first':>14} {'26.2 R033':>14}"
      + (f" | {'ours first':>14} {'ours R033':>14}" if CLI else ""))
for horz, vert, tblpX, tblpY, rows in CASES:
    tag = f"{horz}_{vert}_{tblpX}_{tblpY}_{rows}"
    src = OUT / f"t{tag}.docx"
    write(src, horz, vert, tblpX, tblpY, rows)
    old = render(OLD, src, OUT / f"a{tag}")
    new = render(NEW, src, OUT / f"b{tag}")
    line = (f"{horz:>7} {vert:>7} {tblpX:>6} {tblpY:>6} {rows:>5} | "
            f"{str(place(old, 'R000')):>14} {str(place(old, 'R033')):>14} | "
            f"{str(place(new, 'R000')):>14} {str(place(new, 'R033')):>14}")
    if CLI:
        subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(OUT / f"o{tag}")],
                       capture_output=True, timeout=600)
        ours = OUT / f"o{tag}" / f"t{tag}.pdf"
        line += f" | {str(place(ours, 'R000')):>14} {str(place(ours, 'R033')):>14}"
    print(line, flush=True)
