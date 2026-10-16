#!/usr/bin/env python3
"""Does a line holding nothing but an as-character picture keep the paragraph font's descent?

Round 45 left this open in those words. Its eight probes matched LibreOffice to 0.05 pt wherever
there was text beside the picture and were **2.6 pt tall** — exactly Liberation Serif's 12 pt
descent — on the three where the picture was alone on its line. It did not act, because
`MeasuredParagraph` cites an ODF fixture (`dotnet/tests/corpus/features/picture-anchor.fodt`)
where LibreOffice *does* add that descent, and the round's probes could not separate a **format**
difference from a **text-on-the-line** one.

    picture-alone-descent.py /abs/scratch/dir

This is the pair that separates them: the same two shapes — picture alone, and picture with text
beside it — authored in **both** DOCX and flat ODF, at two picture heights so the slope is fixed
rather than assumed, and measured as the distance between the baseline above the picture
paragraph and the baseline below it.

This probe measures a **length**, so both packages state their styles explicitly: 12 pt Liberation
Serif, which is also what the fodt fixture resolves to. The run prints the face each reference PDF
reports so a substitution cannot pass unnoticed.

The reading to expect if the difference is *text on the line* rather than *format*:

    alone(h)          == h + <a constant that does not include the 12 pt descent>
    with-text(h)      == alone(h) + descent
    and the same in both formats.

The reading that would make it a format difference is the two formats disagreeing on `alone`.
"""
from __future__ import annotations

import base64
import re
import struct
import subprocess
import sys
import zipfile
import zlib
from pathlib import Path

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
      'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture"')

EMU_PER_POINT = 12700

# Two points fix the slope; the small ones separate "drop the descent" from "drop the font".
HEIGHTS = [5.0, 20.0, 50.0, 150.0]


def png(width: int = 8, height: int = 8) -> bytes:
    """A solid mid-grey PNG, built here so the probe carries no binary fixture."""
    raw = b"".join(b"\x00" + bytes([128, 128, 128] * width) for _ in range(height))

    def chunk(tag: bytes, data: bytes) -> bytes:
        return (struct.pack(">I", len(data)) + tag + data
                + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    return (b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(raw))
            + chunk(b"IEND", b""))


CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rIdS" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rIdI" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/p.png"/>
</Relationships>"""

STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}><w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/>
</w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr>
</w:style></w:styles>"""


def drawing(points: float) -> str:
    emu = int(points * EMU_PER_POINT)
    return (f'<w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">'
            f'<wp:extent cx="{emu}" cy="{emu}"/><wp:docPr id="1" name="P"/>'
            f'<a:graphic><a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">'
            f'<pic:pic><pic:nvPicPr><pic:cNvPr id="1" name="P"/><pic:cNvPicPr/></pic:nvPicPr>'
            f'<pic:blipFill><a:blip r:embed="rIdI"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>'
            f'<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{emu}" cy="{emu}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>'
            f'</a:graphicData></a:graphic></wp:inline></w:drawing>')


def docx_document(points: float, with_text: bool) -> str:
    text = '<w:r><w:t xml:space="preserve">Xy </w:t></w:r>' if with_text else ""
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:p><w:r><w:t>ABOVEMARK</w:t></w:r></w:p>
<w:p>{text}<w:r>{drawing(points)}</w:r></w:p>
<w:p><w:r><w:t>BELOWMARK</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0"/>
</w:sectPr></w:body></w:document>"""


def build_docx(path: Path, points: float, with_text: bool) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/media/p.png", png())
        z.writestr("word/document.xml", docx_document(points, with_text))


FODT = """<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
 <office:automatic-styles>
  <style:style style:name="pageheader" style:family="paragraph"/>
  <style:page-layout style:name="pm1"><style:page-layout-properties
    fo:page-width="21.001cm" fo:page-height="29.7cm" fo:margin-top="1.27cm"
    fo:margin-bottom="1.27cm" fo:margin-left="1.27cm" fo:margin-right="1.27cm"/>
  </style:page-layout>
 </office:automatic-styles>
 <office:styles>
  <style:style style:name="Standard" style:family="paragraph">
   <style:text-properties style:font-name="Liberation Serif" fo:font-size="12pt"/>
  </style:style>
 </office:styles>
 <office:master-styles>
  <style:master-page style:name="Standard" style:page-layout-name="pm1"/>
 </office:master-styles>
 <office:body><office:text>
  <text:p>ABOVEMARK</text:p>
  <text:p>{text}<draw:frame draw:name="Pic1" text:anchor-type="as-char"
     svg:width="{pt}pt" svg:height="{pt}pt"><draw:image><office:binary-data
     xmlns:office2="x">{data}</office:binary-data></draw:image></draw:frame></text:p>
  <text:p>BELOWMARK</text:p>
 </office:text></office:body>
</office:document>"""


def build_fodt(path: Path, points: float, with_text: bool) -> None:
    path.write_text(
        FODT.replace("{text}", "Xy " if with_text else "")
            .replace("{pt}", f"{points:g}")
            .replace("{data}", base64.b64encode(png()).decode("ascii")),
        encoding="utf8")


def baselines(pdf: Path, ops: Path) -> dict[str, float]:
    out = subprocess.run([str(ops), "dump", str(pdf), "--page", "1"],
                         capture_output=True, text=True).stdout
    found: dict[str, float] = {}
    faces: set[str] = set()
    for line in out.splitlines():
        m = re.match(r"\s*text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(\S+)\s+(\S+)", line)
        if not m:
            continue
        faces.add(m.group(4).split("+")[-1])
        for mark in ("ABOVEMARK", "BELOWMARK"):
            if mark in line and mark not in found:
                found[mark] = float(m.group(2))
    found["_faces"] = faces  # type: ignore[assignment]
    return found


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    work = Path(sys.argv[1]).resolve()
    work.mkdir(parents=True, exist_ok=True)
    global HEIGHTS
    if len(sys.argv) > 2:
        HEIGHTS = [float(x) for x in sys.argv[2].split(",")]
    here = Path(__file__).resolve()
    ops = here.parents[3] / ".claude/skills/render-comparison/scripts/pdf-ops.py"
    cli = (here.parents[3]
           / "dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli")

    print(f"{'format':<6} {'shape':<10} {'picture':>8} "
          f"{'LibreOffice':>12} {'ours':>10} {'ours-LO':>9}  faces")
    rows = {}
    for points in HEIGHTS:
        for fmt in ("docx", "fodt"):
            for shape, with_text in (("alone", False), ("with-text", True)):
                stem = f"{fmt}-{shape}-{points:g}"
                src = work / f"{stem}.{fmt}"
                (build_docx if fmt == "docx" else build_fodt)(src, points, with_text)

                subprocess.run(
                    ["soffice", "--headless", f"-env:UserInstallation=file://{work}/prof",
                     "--convert-to", "pdf", "--outdir", str(work / "ref"), str(src)],
                    capture_output=True, timeout=240)
                subprocess.run([str(cli), "render", "--outdir", str(work / "ours"), str(src)],
                               capture_output=True, timeout=240)

                ref_pdf = work / "ref" / f"{stem}.pdf"
                our_pdf = work / "ours" / f"{stem}.pdf"
                if not ref_pdf.exists() or not our_pdf.exists():
                    print(f"{fmt:<6} {shape:<10} {points:>8g}  CONVERT FAILED")
                    continue
                r, o = baselines(ref_pdf, ops), baselines(our_pdf, ops)
                if "ABOVEMARK" not in r or "BELOWMARK" not in r:
                    print(f"{fmt:<6} {shape:<10} {points:>8g}  MARK NOT FOUND (reference)")
                    continue
                rg = r["ABOVEMARK"] - r["BELOWMARK"]
                og = o["ABOVEMARK"] - o["BELOWMARK"]
                rows[(fmt, shape, points)] = (rg, og)
                print(f"{fmt:<6} {shape:<10} {points:>8g} {rg:>12.2f} {og:>10.2f} "
                      f"{og - rg:>+9.2f}  {','.join(sorted(r['_faces']))}")

    print()
    print("what separates the two readings:")
    for points in HEIGHTS:
        for fmt in ("docx", "fodt"):
            a = rows.get((fmt, "alone", points))
            t = rows.get((fmt, "with-text", points))
            if a and t:
                print(f"  {fmt} {points:g}pt: LibreOffice with-text − alone = {t[0] - a[0]:+.2f}, "
                      f"ours {t[1] - a[1]:+.2f}")
    for shape in ("alone", "with-text"):
        for points in HEIGHTS:
            d = rows.get(("docx", shape, points))
            f = rows.get(("fodt", shape, points))
            if d and f:
                print(f"  {shape} {points:g}pt: LibreOffice fodt − docx = {f[0] - d[0]:+.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
