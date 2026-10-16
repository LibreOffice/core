"""The same question for a table's own borders: what the style draws, and what it costs a row.

One two-row, one-column table whose `w:tblBorders` state the style under test on every side, with
`ABOVE` before it and `BELOW` after. The observables are the horizontal strokes on page one and
`BELOW`'s own y, which is the table's whole height plus what the border took.

Usage: XRAY_OFFICE_CLI=<cli> python3 tables.py <outdir> [style ...]
"""
import os, re, subprocess, sys, zipfile
from pathlib import Path
import numpy as np
from PIL import Image

OUT = Path(sys.argv[1]); OUT.mkdir(parents=True, exist_ok=True)
CLI = os.environ["XRAY_OFFICE_CLI"]
STYLES = sys.argv[2:] or ["single", "double", "thick", "dotted", "dashed",
                          "thinThickSmallGap", "outset"]
SIZES = [int(x) for x in os.environ.get("SIZES", "8 24").split()]
DPI = 300

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

def document(style, sz):
    side = f'w:val="{style}" w:sz="{sz}" w:space="0" w:color="000000"'
    row = ('<w:tr><w:tc><w:tcPr><w:tcW w:w="5000" w:type="dxa"/></w:tcPr>'
           '<w:p><w:r><w:t>CELL</w:t></w:r></w:p></w:tc></w:tr>')
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
 <w:body>
  <w:p><w:r><w:t>ABOVE</w:t></w:r></w:p>
  <w:tbl>
   <w:tblPr><w:tblW w:w="5000" w:type="dxa"/>
    <w:tblBorders>
     <w:top {side}/><w:bottom {side}/><w:left {side}/><w:right {side}/>
     <w:insideH {side}/><w:insideV {side}/>
    </w:tblBorders>
   </w:tblPr>
   <w:tblGrid><w:gridCol w:w="5000"/></w:tblGrid>
   {row}{row}
  </w:tbl>
  <w:p><w:r><w:t>BELOW</w:t></w:r></w:p>
  <w:sectPr>
   <w:pgSz w:w="11906" w:h="16838"/>
   <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
            w:header="708" w:footer="708" w:gutter="0"/>
  </w:sectPr>
 </w:body>
</w:document>"""

def write(path, style, sz):
    with zipfile.ZipFile(path, "w") as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/document.xml", document(style, sz))

def strokes(pdf):
    out = OUT / "px"; out.mkdir(exist_ok=True)
    for f in out.glob("p-*.png"): f.unlink()
    subprocess.run(["pdftoppm", "-r", str(DPI), "-f", "1", "-l", "1", "-gray", "-png",
                    str(pdf), str(out / "p")], capture_output=True)
    made = sorted(out.glob("p-*.png"))
    if not made: return []
    a = np.asarray(Image.open(made[0]).convert("L")).astype(float)
    # The table is 250 pt of a 451 pt measure, so a rule covers a quarter of the page width.
    dark = (a < 200).sum(axis=1) > (a.shape[1] * 0.25)
    runs, start = [], None
    for y, on in enumerate(dark):
        if on and start is None: start = y
        elif not on and start is not None:
            runs.append((round(start * 72 / DPI, 2), round((y - start) * 72 / DPI, 2)))
            start = None
    return runs

def word_y(pdf, want):
    r = subprocess.run(["pdftotext", "-bbox", str(pdf), "-"], capture_output=True, text=True)
    for m in re.finditer(r'<word xMin="[\d.]+" yMin="([\d.]+)"[^>]*>([^<]*)</word>', r.stdout):
        if m.group(2) == want: return round(float(m.group(1)), 2)
    return None

print(f"{'style':>18} {'sz':>3} | {'reference rules':>44} {'BELOW':>7} "
      f"| {'our rules':>44} {'BELOW':>7}")
for style in STYLES:
    for sz in SIZES:
        tag = f"{style}{sz}"
        src = OUT / f"t{tag}.docx"
        write(src, style, sz)
        here = OUT / f"r{tag}"; here.mkdir(exist_ok=True)
        subprocess.run(["soffice", f"-env:UserInstallation=file://{here / 'prof'}", "--headless",
                        "--convert-to", "pdf", "--outdir", str(here), str(src)],
                       capture_output=True, timeout=300)
        subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(OUT / f"o{tag}")],
                       capture_output=True, timeout=300)
        rp, op = here / f"t{tag}.pdf", OUT / f"o{tag}" / f"t{tag}.pdf"
        print(f"{style:>18} {sz:>3} | {str(strokes(rp)):>44} {str(word_y(rp, 'BELOW')):>7} "
              f"| {str(strokes(op)):>44} {str(word_y(op, 'BELOW')):>7}", flush=True)
