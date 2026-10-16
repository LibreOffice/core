#!/usr/bin/env python3
"""Does the label raise the base for *its own* line only, or for every line of the item?

    label-wrapped-paragraph.py /abs/scratch/dir

The other two probes here use one-line items, so they cannot tell these apart:

    per-line   the base is a property of each line, and only the first line carries the
               label — so a three-line item at 200% grows by box(L) + 2 x box(12)
    per-para   the label raises the base for the whole paragraph — grows by 3 x box(L)

At L = 28 over 12 pt text those are 59.8 pt and 96.6 pt, so one render decides it. The
assumption being measured is the one our layout already makes (`MeasureLine` takes a
range, and the label enters as an object at offset nought, which touches line one), and
measuring it is cheaper than being wrong about it on 93 documents.

Rows: {1 line, 3 lines} x {100%, 200%}, 28 pt level over 12 pt Liberation Serif, styles
stated because this measures a length.
"""
from __future__ import annotations

import re
import subprocess
import sys
import zipfile
from pathlib import Path

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')

LEVEL_POINTS = 28.0

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
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

# Three lines in a 468 pt column at 12 pt Liberation Serif, checked by the line count the
# probe prints rather than assumed.
LONG = ("Alpha bravo charlie delta echo foxtrot golf hotel india juliett kilo lima mike "
        "november oscar papa quebec romeo sierra tango uniform victor whiskey xray yankee "
        "zulu alpha bravo charlie delta echo foxtrot golf hotel india juliett kilo lima")


def document(percent: int, long_item: bool) -> str:
    spacing = f'<w:spacing w:line="{percent * 240 // 100}" w:lineRule="auto"/>'
    text = LONG if long_item else "Item"
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:p><w:r><w:t>ABOVEMARK</w:t></w:r></w:p>
<w:p><w:pPr><w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>{spacing}</w:pPr>
<w:r><w:t xml:space="preserve">{text}</w:t></w:r></w:p>
<w:p><w:r><w:t>BELOWMARK</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0"/>
</w:sectPr></w:body></w:document>"""


def build(path: Path, percent: int, long_item: bool) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/numbering.xml", NUMBERING)
        z.writestr("word/document.xml", document(percent, long_item))


def read(pdf: Path, ops: Path) -> tuple[dict, list[float], set]:
    out = subprocess.run([str(ops), "dump", str(pdf), "--page", "1"],
                         capture_output=True, text=True).stdout
    found: dict = {}
    faces: set[str] = set()
    ys: set[float] = set()
    for line in out.splitlines():
        m = re.match(r"\s*text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(\S+)\s+(\S+)", line)
        if not m:
            continue
        faces.add(m.group(4).split("+")[-1])
        ys.add(round(float(m.group(2)), 1))
        for mark in ("ABOVEMARK", "BELOWMARK"):
            if mark in line and mark not in found:
                found[mark] = float(m.group(2))
    return found, sorted(ys, reverse=True), faces


def pitches(ys: list[float]) -> str:
    """Baseline-to-baseline distances down the page, which is where the extension lands."""
    return " ".join(f"{a - b:.2f}" for a, b in zip(ys, ys[1:]))


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
    print(f"{'item':>6} {'spacing':>8} {'LibreOffice':>12} "
          f"{'ours':>10} {'ours-LO':>9}  baseline pitches")
    for long_item in (False, True):
        for percent in (100, 150, 200):
            stem = f"{'long' if long_item else 'short'}-{percent}"
            src = work / f"{stem}.docx"
            build(src, percent, long_item)
            subprocess.run(
                ["soffice", "--headless", f"-env:UserInstallation=file://{work}/prof",
                 "--convert-to", "pdf", "--outdir", str(work / "ref"), str(src)],
                capture_output=True, timeout=240)
            subprocess.run([str(cli), "render", "--outdir", str(work / "ours"), str(src)],
                           capture_output=True, timeout=240)
            ref_pdf, our_pdf = work / "ref" / f"{stem}.pdf", work / "ours" / f"{stem}.pdf"
            if not ref_pdf.exists() or not our_pdf.exists():
                print(f"{stem:>6} CONVERT FAILED")
                continue
            r, ry, faces = read(ref_pdf, ops)
            o, oy, _ = read(our_pdf, ops)
            if "ABOVEMARK" not in r or "BELOWMARK" not in r:
                print(f"{stem:>6} MARK NOT FOUND")
                continue
            rg, og = r["ABOVEMARK"] - r["BELOWMARK"], o["ABOVEMARK"] - o["BELOWMARK"]
            rows[(long_item, percent)] = (rg, og)
            name = "long" if long_item else "short"
            print(f"{name:>6} {percent:>7}% {rg:>12.2f} {og:>10.2f} {og - rg:>+9.2f}  "
                  f"LO  {pitches(ry)}   [{','.join(sorted(faces))}]")
            print(f"{'':>6} {'':>8} {'':>12} {'':>10} {'':>9}  ours {pitches(oy)}")

    print()
    print("gap(200%) - gap(100%):")
    for long_item in (False, True):
        a, b = rows.get((long_item, 100)), rows.get((long_item, 200))
        if a and b:
            print(f"  {'3-line' if long_item else '1-line'} item: "
                  f"LibreOffice {b[0] - a[0]:+7.2f}   ours {b[1] - a[1]:+7.2f}")
    print()
    print(f"  per-line predicts {LEVEL_POINTS * 1.15:.2f} + 2 x 13.80 = "
          f"{LEVEL_POINTS * 1.15 + 27.6:.2f} on the 3-line row")
    print(f"  per-paragraph predicts 3 x {LEVEL_POINTS * 1.15:.2f} = "
          f"{3 * LEVEL_POINTS * 1.15:.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
