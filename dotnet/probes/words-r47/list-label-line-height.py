#!/usr/bin/env python3
"""Does a list label take part in the line's *text* height, the way a run does?

    list-label-line-height.py /abs/scratch/dir

Round 45 established the law for an as-character picture:

    a line at p% is extended by (p - 100)% of the line's **text** height, added rather
    than scaled, and an as-character object raises the line without taking a share.

Round 46 said in one line that a list label taller than its item "is the same rule",
because `SwNumberPortion` carries `PortionType::Number` and
`SwLinePortion::IsUsedToCalcLineSpacingHeight` returns true only for
`PortionType::Text` (`sw/source/core/text/porlin.cxx`:324). That is a citation, so it is
the hypothesis. This is the measurement.

Nine authored DOCX, varying **one thing at a time**, read against the installed 24.2.7.2:

    level size L in {12, 14, 20, 28} pt over 12 pt item text, at p in {100, 150, 200}%
    plus an unlabelled control at each p, so the label's own contribution is a difference
    rather than an absolute

and the two readings are far apart, which is the point of taking L up to 28:

    label counts as text   gap(p, L) - gap(100, L) == (p-100)% x box(L)
    label counts as a fly  gap(p, L) - gap(100, L) == (p-100)% x box(12)

At L = 28 and p = 200 those differ by about 18 pt, a fifth of a page band.

This probe measures a **length**, so `word/styles.xml` states the face and size outright
(Liberation Serif 12 pt) and the run prints the faces each reference PDF reports, so a
substitution cannot pass unnoticed. The label's own `w:rPr` names the same family, so the
only thing varying between rows is the size.

The gap measured is between the baseline of the paragraph *above* the labelled one and
the baseline of the paragraph *below* it. Both marks are unlabelled 12 pt paragraphs at
100%, so everything they contribute is constant across rows and the differences are the
middle line's alone.
"""
from __future__ import annotations

import re
import subprocess
import sys
import zipfile
from pathlib import Path

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')

LEVEL_SIZES = [12.0, 14.0, 20.0, 28.0]
PERCENTS = [100, 150, 200]

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


def numbering(level_points: float) -> str:
    half = int(round(level_points * 2))
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:numbering {NS}>
<w:abstractNum w:abstractNumId="0"><w:multiLevelType w:val="singleLevel"/>
<w:lvl w:ilvl="0"><w:start w:val="1"/><w:numFmt w:val="decimal"/>
<w:lvlText w:val="%1."/><w:lvlJc w:val="left"/>
<w:pPr><w:ind w:left="720" w:hanging="720"/></w:pPr>
<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>
<w:sz w:val="{half}"/></w:rPr>
</w:lvl></w:abstractNum>
<w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>
</w:numbering>"""


def document(percent: int, labelled: bool) -> str:
    spacing = f'<w:spacing w:line="{percent * 240 // 100}" w:lineRule="auto"/>'
    num = '<w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>' if labelled else ""
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:p><w:r><w:t>ABOVEMARK</w:t></w:r></w:p>
<w:p><w:pPr>{num}{spacing}</w:pPr><w:r><w:t>Item</w:t></w:r></w:p>
<w:p><w:r><w:t>BELOWMARK</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0"/>
</w:sectPr></w:body></w:document>"""


def build(path: Path, level_points: float, percent: int, labelled: bool) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/numbering.xml", numbering(level_points))
        z.writestr("word/document.xml", document(percent, labelled))


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
    print(f"{'label':>7} {'spacing':>8} {'LibreOffice':>12} {'ours':>10} {'ours-LO':>9}  faces")
    cases = [("none", 0.0, p) for p in PERCENTS] + \
            [("label", L, p) for L in LEVEL_SIZES for p in PERCENTS]
    for kind, L, p in cases:
        stem = f"{kind}-{L:g}-{p}"
        src = work / f"{stem}.docx"
        build(src, L if L else 12.0, p, labelled=(kind == "label"))
        subprocess.run(
            ["soffice", "--headless", f"-env:UserInstallation=file://{work}/prof",
             "--convert-to", "pdf", "--outdir", str(work / "ref"), str(src)],
            capture_output=True, timeout=240)
        subprocess.run([str(cli), "render", "--outdir", str(work / "ours"), str(src)],
                       capture_output=True, timeout=240)
        ref_pdf, our_pdf = work / "ref" / f"{stem}.pdf", work / "ours" / f"{stem}.pdf"
        if not ref_pdf.exists() or not our_pdf.exists():
            print(f"{kind:>7} {p:>7}% CONVERT FAILED")
            continue
        r, o = baselines(ref_pdf, ops), baselines(our_pdf, ops)
        if "ABOVEMARK" not in r or "BELOWMARK" not in r:
            print(f"{kind:>7} {p:>7}% MARK NOT FOUND")
            continue
        rg = r["ABOVEMARK"] - r["BELOWMARK"]
        og = o["ABOVEMARK"] - o["BELOWMARK"]
        rows[(kind, L, p)] = (rg, og)
        name = "none" if kind == "none" else f"{L:g}pt"
        print(f"{name:>7} {p:>7}% {rg:>12.2f} {og:>10.2f} {og - rg:>+9.2f}  "
              f"{','.join(sorted(r['_faces']))}")

    print()
    print("the label's own contribution at 100% — box(L) - box(12), if it is the level's line box:")
    base = rows.get(("none", 0.0, 100))
    for L in LEVEL_SIZES:
        row = rows.get(("label", L, 100))
        if base and row:
            print(f"  L={L:g}pt: LibreOffice {row[0] - base[0]:+7.2f}   ours {row[1] - base[1]:+7.2f}")

    print()
    print("what the percentage is taken of — gap(p) - gap(100), per level size:")
    for L in LEVEL_SIZES:
        hundred = rows.get(("label", L, 100))
        if not hundred:
            continue
        for p in PERCENTS:
            row = rows.get(("label", L, p))
            if p == 100 or not row:
                continue
            print(f"  L={L:g}pt p={p}%: LibreOffice {row[0] - hundred[0]:+7.2f}   "
                  f"ours {row[1] - hundred[1]:+7.2f}")
    print()
    print("  compare against the unlabelled control's own extension, which is (p-100)% x box(12):")
    hundred = rows.get(("none", 0.0, 100))
    for p in PERCENTS:
        row = rows.get(("none", 0.0, p))
        if p == 100 or not (row and hundred):
            continue
        print(f"    p={p}%: LibreOffice {row[0] - hundred[0]:+7.2f}   ours {row[1] - hundred[1]:+7.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
