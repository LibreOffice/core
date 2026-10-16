#!/usr/bin/env python3
"""Which line's height does a paragraph's proportional extension come from?

    first-line-or-last-line.py /abs/scratch/dir

`label-wrapped-paragraph.py` produced a result the label rule alone cannot explain:

    1-line labelled item, 200%   LibreOffice grows by box(28) = 32.20
    3-line labelled item, 200%   LibreOffice grows by 3 x box(12) = 41.40, not
                                 box(28) + 2 x box(12) = 59.80

and its per-baseline pitches say where the space goes. Within the item the gap above
line n is (p-100)% of line n's own base; the gap above line **one** never moves with the
item's own percentage, and the gap between the item's last line and the paragraph after
it moves with it. That is Writer's own arrangement:

    SwTextFrame::GetLineSpace   = (prop - 100)% x GetHeightOfLastLine()
    CalcHeightOfLastLine        = MaxAscentDescent(..., bNoFlyCnt = true) of the LAST line
    SwTextFormatter::CalcRealHeight is guarded by `if (!IsParaLine())`

— the paragraph's share is charged to the space *after* it, taken from its **last** line,
and its first line gets nothing of its own.  Summed over a paragraph the two models differ
by exactly one term: ours spends `ext(base(first line))` where Writer spends
`ext(base(last line))`.

On a paragraph whose lines are all the same height those are equal, which is why the
per-line model has held for eleven rounds. This probe makes them differ **without a list
label anywhere**, so the two halves can be tested apart:

    tall-first   a 28 pt word on line 1 of a three-line 12 pt paragraph
    tall-last    the same word on line 3
    flat         no tall word, the control

    Writer:  ext = sum over n>=2 of base(n)  +  base(last)
    ours:    ext = sum over n>=1 of base(n)

    tall-first at 200%   Writer 41.40   ours 59.80
    tall-last  at 200%   Writer 78.20   ours 59.80

Styles are stated (Liberation Serif 12 pt) because this measures a length, and the run
prints each reference PDF's faces and the per-baseline pitches rather than only the total.
"""
from __future__ import annotations

import re
import subprocess
import sys
import zipfile
from pathlib import Path

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')

TALL_POINTS = 28.0

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
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
</Relationships>"""

STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}><w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/>
</w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr>
</w:style></w:styles>"""

WORDS = ("alpha bravo charlie delta echo foxtrot golf hotel india juliett kilo lima mike "
         "november oscar papa quebec romeo sierra tango uniform victor whiskey xray "
         "yankee zulu alpha bravo charlie delta echo foxtrot golf hotel india").split()

TALL = (f'<w:r><w:rPr><w:sz w:val="{int(TALL_POINTS * 2)}"/></w:rPr>'
        f'<w:t xml:space="preserve">Big </w:t></w:r>')


def runs(where: str) -> str:
    """Three lines of 12 pt text with one 28 pt word placed first, last, or nowhere."""
    body = f'<w:r><w:t xml:space="preserve">{" ".join(WORDS)}</w:t></w:r>'
    if where == "first":
        return TALL + body
    if where == "last":
        return body + " " + TALL
    return body


def document(percent: int, where: str) -> str:
    spacing = f'<w:spacing w:line="{percent * 240 // 100}" w:lineRule="auto"/>'
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:p><w:r><w:t>ABOVEMARK</w:t></w:r></w:p>
<w:p><w:pPr>{spacing}</w:pPr>{runs(where)}</w:p>
<w:p><w:r><w:t>BELOWMARK</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0"/>
</w:sectPr></w:body></w:document>"""


def build(path: Path, percent: int, where: str) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", document(percent, where))


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
    print(f"{'28pt on':>8} {'spacing':>8} {'LibreOffice':>12} {'ours':>10} {'ours-LO':>9}  "
          f"baseline pitches")
    for where in ("none", "first", "last"):
        for percent in (100, 200):
            stem = f"{where}-{percent}"
            src = work / f"{stem}.docx"
            build(src, percent, where)
            subprocess.run(
                ["soffice", "--headless", f"-env:UserInstallation=file://{work}/prof",
                 "--convert-to", "pdf", "--outdir", str(work / "ref"), str(src)],
                capture_output=True, timeout=240)
            subprocess.run([str(cli), "render", "--outdir", str(work / "ours"), str(src)],
                           capture_output=True, timeout=240)
            ref_pdf, our_pdf = work / "ref" / f"{stem}.pdf", work / "ours" / f"{stem}.pdf"
            if not ref_pdf.exists() or not our_pdf.exists():
                print(f"{stem:>8} CONVERT FAILED")
                continue
            r, ry, faces = read(ref_pdf, ops)
            o, oy, _ = read(our_pdf, ops)
            if "ABOVEMARK" not in r or "BELOWMARK" not in r:
                print(f"{stem:>8} MARK NOT FOUND")
                continue
            rg, og = r["ABOVEMARK"] - r["BELOWMARK"], o["ABOVEMARK"] - o["BELOWMARK"]
            rows[(where, percent)] = (rg, og)
            print(f"{where:>8} {percent:>7}% {rg:>12.2f} {og:>10.2f} {og - rg:>+9.2f}  "
                  f"LO   {pitches(ry)}   [{','.join(sorted(faces))}]")
            print(f"{'':>8} {'':>8} {'':>12} {'':>10} {'':>9}  ours {pitches(oy)}")

    print()
    print("gap(200%) - gap(100%), which is the paragraph's whole proportional extension:")
    for where in ("none", "first", "last"):
        a, b = rows.get((where, 100)), rows.get((where, 200))
        if a and b:
            print(f"  28pt word on {where:>5}: LibreOffice {b[0] - a[0]:+7.2f}   "
                  f"ours {b[1] - a[1]:+7.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
