#!/usr/bin/env python3
"""Separates "the label counts as text" from "the base is simply the line height".

    label-and-picture.py /abs/scratch/dir

`list-label-line-height.py` shows that at 150% and 200% LibreOffice extends a labelled
line by (p-100)% of the **label's** box, not of the item text's. Two rules fit that on
its own:

    H1  the base is the tallest portion that is not a fly-in-content — so a label counts
        and an as-character picture does not (round 45's law, with the label inside it)
    H2  the base is simply the whole line's height, and round 45's picture measurements
        were something else

One document separates them: a 28 pt level over 12 pt text with a **100 pt picture** on
the same line. The line is 100 pt tall either way, so only the extension differs:

    H1   gap(200%) - gap(100%) == box(28) == 32.2 pt
    H2   gap(200%) - gap(100%) == the whole line, about 114 pt

Four rows, so each leg is a difference rather than an absolute: {no picture, picture} x
{100%, 200%}, all with the same 28 pt label. Styles are stated — Liberation Serif 12 pt —
because this measures a length.
"""
from __future__ import annotations

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
LEVEL_POINTS = 28.0
PICTURE_POINTS = 100.0

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rIdS" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rIdN" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering" Target="numbering.xml"/>
<Relationship Id="rIdI" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/p.png"/>
</Relationships>"""

STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}><w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/>
</w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr>
</w:style></w:styles>"""

NUMBERING = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:numbering {NS}>
<w:abstractNum w:abstractNumId="0"><w:multiLevelType w:val="singleLevel"/>
<w:lvl w:ilvl="0"><w:start w:val="1"/><w:numFmt w:val="decimal"/>
<w:lvlText w:val="%1."/><w:lvlJc w:val="left"/>
<w:pPr><w:ind w:left="720" w:hanging="720"/></w:pPr>
<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>
<w:sz w:val="{int(LEVEL_POINTS * 2)}"/></w:rPr>
</w:lvl></w:abstractNum>
<w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>
</w:numbering>"""


def png(width: int = 8, height: int = 8) -> bytes:
    raw = b"".join(b"\x00" + bytes([128, 128, 128] * width) for _ in range(height))

    def chunk(tag: bytes, data: bytes) -> bytes:
        return (struct.pack(">I", len(data)) + tag + data
                + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    return (b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(raw))
            + chunk(b"IEND", b""))


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


def document(percent: int, with_picture: bool) -> str:
    spacing = f'<w:spacing w:line="{percent * 240 // 100}" w:lineRule="auto"/>'
    picture = f"<w:r>{drawing(PICTURE_POINTS)}</w:r>" if with_picture else ""
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:p><w:r><w:t>ABOVEMARK</w:t></w:r></w:p>
<w:p><w:pPr><w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>{spacing}</w:pPr>
<w:r><w:t xml:space="preserve">Item </w:t></w:r>{picture}</w:p>
<w:p><w:r><w:t>BELOWMARK</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0"/>
</w:sectPr></w:body></w:document>"""


def build(path: Path, percent: int, with_picture: bool) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/numbering.xml", NUMBERING)
        z.writestr("word/media/p.png", png())
        z.writestr("word/document.xml", document(percent, with_picture))


def baselines(pdf: Path, ops: Path) -> dict:
    out = subprocess.run([str(ops), "dump", str(pdf), "--page", "1"],
                         capture_output=True, text=True).stdout
    found: dict = {}
    faces: set[str] = set()
    for line in out.splitlines():
        m = re.match(r"\s*text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(\S+)\s+(\S+)", line)
        if not m:
            continue
        faces.add(m.group(4).split("+")[-1])
        for mark in ("ABOVEMARK", "BELOWMARK"):
            if mark in line and mark not in found:
                found[mark] = float(m.group(2))
    found["_faces"] = faces
    return found


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    work = Path(sys.argv[1]).resolve()
    (work / "ref").mkdir(parents=True, exist_ok=True)
    (work / "ours").mkdir(parents=True, exist_ok=True)
    here = Path(__file__).resolve()
    ops = here.parents[3] / ".claude/skills/render-comparison/scripts/pdf-ops.py"
    cli = (here.parents[3]
           / "dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli")

    rows: dict = {}
    print(f"{'picture':>8} {'spacing':>8} {'LibreOffice':>12} {'ours':>10} {'ours-LO':>9}  faces")
    for with_picture in (False, True):
        for percent in (100, 200):
            stem = f"{'pic' if with_picture else 'nopic'}-{percent}"
            src = work / f"{stem}.docx"
            build(src, percent, with_picture)
            subprocess.run(
                ["soffice", "--headless", f"-env:UserInstallation=file://{work}/prof",
                 "--convert-to", "pdf", "--outdir", str(work / "ref"), str(src)],
                capture_output=True, timeout=240)
            subprocess.run([str(cli), "render", "--outdir", str(work / "ours"), str(src)],
                           capture_output=True, timeout=240)
            ref_pdf, our_pdf = work / "ref" / f"{stem}.pdf", work / "ours" / f"{stem}.pdf"
            if not ref_pdf.exists() or not our_pdf.exists():
                print(f"{str(with_picture):>8} {percent:>7}% CONVERT FAILED")
                continue
            r, o = baselines(ref_pdf, ops), baselines(our_pdf, ops)
            if "ABOVEMARK" not in r or "BELOWMARK" not in r:
                print(f"{str(with_picture):>8} {percent:>7}% MARK NOT FOUND")
                continue
            rg, og = r["ABOVEMARK"] - r["BELOWMARK"], o["ABOVEMARK"] - o["BELOWMARK"]
            rows[(with_picture, percent)] = (rg, og)
            print(f"{('yes' if with_picture else 'no'):>8} {percent:>7}% {rg:>12.2f} "
                  f"{og:>10.2f} {og - rg:>+9.2f}  {','.join(sorted(r['_faces']))}")

    print()
    print("gap(200%) - gap(100%):")
    for with_picture in (False, True):
        a, b = rows.get((with_picture, 100)), rows.get((with_picture, 200))
        if a and b:
            print(f"  picture={'yes' if with_picture else 'no ':>3}: "
                  f"LibreOffice {b[0] - a[0]:+7.2f}   ours {b[1] - a[1]:+7.2f}")
    print()
    print(f"  H1 (base = tallest non-fly portion) predicts {LEVEL_POINTS * 1.15:+.2f} in both rows")
    print(f"  H2 (base = the whole line) predicts about {PICTURE_POINTS + 13.8:+.2f} in the second")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
