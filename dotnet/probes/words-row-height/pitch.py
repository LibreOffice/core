"""What a `w:trHeight` floor actually costs a row: the border, the margins, or neither.

Six one-cell rows, all stating the same `w:trHeight`, with the grid's border width, the `w:hRule`,
the cell's top and bottom margins and the cell's content swept independently. The observable is the
*pitch* between consecutive horizontal rules on page one, read off a 600 dpi raster, which is the
row height and nothing else -- the first and last rules are dropped, so the table's own outer half
borders cannot reach it.

Usage: XRAY_OFFICE_CLI=<cli> python3 pitch.py <outdir>
"""
import os, statistics, subprocess, sys, tempfile, zipfile
from pathlib import Path
import numpy as np
from PIL import Image

OUT = Path(sys.argv[1]); OUT.mkdir(parents=True, exist_ok=True)
CLI = os.environ["XRAY_OFFICE_CLI"]
DPI = 600
ROWS = 6
HEIGHT = 480          # 24 pt, the figure `row-min-height-border.py` used

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

def document(sz, rule, margin, text):
    borders = "".join(
        f'<w:{side} w:val="single" w:sz="{sz}" w:space="0" w:color="000000"/>'
        for side in ("top", "left", "bottom", "right")) if sz else ""
    cellMar = (f'<w:tcMar><w:top w:w="{margin}" w:type="dxa"/>'
               f'<w:bottom w:w="{margin}" w:type="dxa"/></w:tcMar>')
    hRule = "" if rule is None else f' w:hRule="{rule}"'
    body = f'<w:r><w:t>{text}</w:t></w:r>' if text else ""
    row = (f'<w:tr><w:trPr><w:trHeight w:val="{HEIGHT}"{hRule}/></w:trPr>'
           f'<w:tc><w:tcPr><w:tcW w:w="5000" w:type="dxa"/>'
           f'<w:tcBorders>{borders}</w:tcBorders>{cellMar}</w:tcPr>'
           f'<w:p>{body}</w:p></w:tc></w:tr>')
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
 <w:body>
  <w:tbl>
   <w:tblPr><w:tblW w:w="5000" w:type="dxa"/>
    <w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/></w:tblCellMar>
   </w:tblPr>
   <w:tblGrid><w:gridCol w:w="5000"/></w:tblGrid>
   {row * ROWS}
  </w:tbl>
  <w:sectPr>
   <w:pgSz w:w="11906" w:h="16838"/>
   <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
            w:header="708" w:footer="708" w:gutter="0"/>
  </w:sectPr>
 </w:body>
</w:document>"""

def write(path, sz, rule, margin, text):
    with zipfile.ZipFile(path, "w") as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/document.xml", document(sz, rule, margin, text))

def pitch(pdf):
    """The median gap between the tops of consecutive horizontal rules, in twips."""
    if not Path(pdf).exists(): return None
    with tempfile.TemporaryDirectory() as t:
        subprocess.run(["pdftoppm", "-r", str(DPI), "-f", "1", "-l", "1", "-gray", "-png",
                        str(pdf), os.path.join(t, "p")], capture_output=True)
        made = sorted(os.listdir(t))
        if not made: return None
        a = np.asarray(Image.open(os.path.join(t, made[0])).convert("L")).astype(float)
    dark = (a < 200).sum(axis=1) > (a.shape[1] * 0.25)
    tops, start = [], None
    for y, on in enumerate(dark):
        if on and start is None: start = y
        elif not on and start is not None:
            tops.append(start); start = None
    if len(tops) < 4: return None
    gaps = [(tops[i + 1] - tops[i]) * 1440 / DPI for i in range(len(tops) - 1)]
    return round(statistics.median(gaps[1:-1] or gaps), 1)

def render(src, tag):
    here = OUT / f"r{tag}"; here.mkdir(exist_ok=True)
    subprocess.run(["soffice", f"-env:UserInstallation=file://{here / 'prof'}", "--headless",
                    "--convert-to", "pdf", "--outdir", str(here), str(src)],
                   capture_output=True, timeout=300)
    subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(OUT / f"o{tag}")],
                   capture_output=True, timeout=300)
    stem = src.stem
    return pitch(here / f"{stem}.pdf"), pitch(OUT / f"o{tag}" / f"{stem}.pdf")

CASES = [(sz, rule, margin, text)
         for rule in (None, "atLeast", "exact", "auto")
         for sz in (0, 4, 8, 24)
         for margin in (0, 100)
         for text in ("", "X")]

print(f"floor {HEIGHT} twips; pitch in twips, and what it is over the floor")
print(f"{'hRule':>8} {'w:sz':>5} {'tcMar':>6} {'cell':>5} | {'ref':>7} {'over':>6} | {'ours':>7} {'over':>6}")
for sz, rule, margin, text in CASES:
    tag = f"{rule or 'none'}_{sz}_{margin}_{'X' if text else 'e'}"
    src = OUT / f"h{tag}.docx"
    write(src, sz, rule, margin, text)
    ref, ours = render(src, tag)
    def over(v): return "-" if v is None else f"{v - HEIGHT:+.0f}"
    print(f"{str(rule or '(none)'):>8} {sz:>5} {margin:>6} {('X' if text else 'empty'):>5} | "
          f"{str(ref):>7} {over(ref):>6} | {str(ours):>7} {over(ours):>6}", flush=True)
