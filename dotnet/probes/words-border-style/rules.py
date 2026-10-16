"""What a non-single `w:pBdr` border draws, and how much room it takes.

One paragraph reading `ABOVE`, one carrying the border under test and reading `TEXT`, one reading
`BELOW`. The observables come from a 300 dpi greyscale raster of page one:

  * every horizontal run of dark pixels across the border's own band, as (top, thickness) in points,
    which is the rule the style actually draws;
  * `TEXT`'s own y from the PDF text layer, which is the room the border took.

Usage: XRAY_OFFICE_CLI=<cli> python3 rules.py <outdir> [style ...]
"""
import os, re, subprocess, sys, zipfile
from pathlib import Path
import numpy as np
from PIL import Image

OUT = Path(sys.argv[1]); OUT.mkdir(parents=True, exist_ok=True)
CLI = os.environ["XRAY_OFFICE_CLI"]
STYLES = sys.argv[2:] or ["single", "double", "dotted", "dashed", "thick",
                          "thinThickSmallGap", "thickThinSmallGap", "triple"]
SIZES = [4, 8, 12, 24]
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
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
 <w:body>
  <w:p><w:r><w:t>ABOVE</w:t></w:r></w:p>
  <w:p>
   <w:pPr><w:pBdr>
    <w:top w:val="{style}" w:sz="{sz}" w:space="0" w:color="000000"/>
   </w:pBdr></w:pPr>
   <w:r><w:t>TEXT</w:t></w:r>
  </w:p>
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
    """Horizontal dark runs on page one, as a list of (top pt, thickness pt)."""
    out = OUT / "px"; out.mkdir(exist_ok=True)
    stem = str(out / "p")
    for f in out.glob("p-*.png"): f.unlink()
    subprocess.run(["pdftoppm", "-r", str(DPI), "-f", "1", "-l", "1", "-gray", "-png",
                    str(pdf), stem], capture_output=True)
    made = sorted(out.glob("p-*.png"))
    if not made: return []
    a = np.asarray(Image.open(made[0]).convert("L")).astype(float)
    # A rule spans most of the measure; a line of text does not.
    dark = (a < 128).sum(axis=1) > (a.shape[1] * 0.5)
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

print(f"{'style':>18} {'sz':>3} | {'reference strokes':>34} {'TEXT':>7} | {'our strokes':>26} {'TEXT':>7}")
for style in STYLES:
    for sz in SIZES:
        tag = f"{style}{sz}"
        src = OUT / f"b{tag}.docx"
        write(src, style, sz)
        here = OUT / f"r{tag}"; here.mkdir(exist_ok=True)
        subprocess.run(["soffice", f"-env:UserInstallation=file://{here / 'prof'}", "--headless",
                        "--convert-to", "pdf", "--outdir", str(here), str(src)],
                       capture_output=True, timeout=300)
        subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(OUT / f"o{tag}")],
                       capture_output=True, timeout=300)
        rp, op = here / f"b{tag}.pdf", OUT / f"o{tag}" / f"b{tag}.pdf"
        rs = str(strokes(rp)) if rp.exists() else "-"
        os_ = str(strokes(op)) if op.exists() else "-"
        print(f"{style:>18} {sz:>3} | {rs:>34} {str(word_y(rp, 'TEXT')):>7} | "
              f"{os_:>26} {str(word_y(op, 'TEXT')):>7}", flush=True)
