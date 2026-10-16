#!/usr/bin/env python3
"""Does a table style's `w:pPr` outrank `w:docDefaults`, as §17.7.2 says it does?

    table-style-vs-docdefaults.py /abs/scratch/dir

`template---tpr-technical-progress-report-with-guidance.docx` reads 7/8 and its page 2
holds 37 lines against the reference's 33 while ending *lower* on the page. Its cell text
is set on a 15.45 pt pitch by LibreOffice and a 13.45 pt pitch by us — exactly 115% and
100% of Carlito's 11 pt line. Its `word/styles.xml` states both:

    <w:docDefaults><w:pPrDefault><w:pPr><w:spacing w:line="276" w:lineRule="auto"/>
    <w:style w:type="table" w:styleId="Table1"><w:pPr><w:spacing w:line="240" …/>

ECMA-376 §17.7.2 orders the two document defaults **below** table styles, so the table
style's single spacing should win, and that is what we implement. LibreOffice takes the
document default. The mechanism that would explain it is the one round 46 established for
widow control: `StyleSheetTable::applyDefaults` does not keep `w:docDefaults` as a layer
of its own — it writes the defaults onto the **built-in paragraph style every other
paragraph style inherits from**, and a paragraph style outranks a table style.

Six authored variants decide between three readings, each varying one thing:

    control              nothing states spacing anywhere        -> single
    docdefaults-only     w:pPrDefault 276                       -> does it reach a cell at all
    tablestyle-only      table style 480, no defaults            -> does a table style apply at all
    both-small-table     defaults 276, table style 240           -> the corpus case
    both-large-table     defaults 276, table style 480           -> which layer wins, not which value
    parastyle-vs-table   Normal 276, table style 480             -> the uncontested half of §17.7.2

`both-large-table` is the one that separates "the document default wins" from "the larger
of the two wins", which the corpus document alone cannot do.

Measured as the pitch between the two baselines of a wrapped cell paragraph, so it is a
line height rather than a row height and no cell margin or border enters it. Styles are
stated — Liberation Serif 12 pt — because this measures a length, and the run prints the
faces each reference PDF reports.
"""
from __future__ import annotations

import re
import subprocess
import sys
import zipfile
from pathlib import Path

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')

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

FONT = '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/>'

# variant -> (docDefaults w:line or None, Normal style w:line or None, table style w:line or None)
VARIANTS = {
    "control": (None, None, None),
    "docdefaults-only": (276, None, None),
    "tablestyle-only": (None, None, 480),
    "both-small-table": (276, None, 240),
    "both-large-table": (276, None, 480),
    "parastyle-vs-table": (None, 276, 480),
}


def spacing(line: int | None) -> str:
    return f'<w:spacing w:line="{line}" w:lineRule="auto"/>' if line else ""


def styles(defaults: int | None, normal: int | None, table: int | None) -> str:
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}>
<w:docDefaults><w:rPrDefault><w:rPr>{FONT}</w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr>{spacing(defaults)}</w:pPr></w:pPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
<w:pPr>{spacing(normal)}</w:pPr><w:rPr>{FONT}</w:rPr></w:style>
<w:style w:type="table" w:default="1" w:styleId="TableNormal"><w:name w:val="Normal Table"/></w:style>
<w:style w:type="table" w:styleId="Table1"><w:name w:val="Table1"/>
<w:basedOn w:val="TableNormal"/><w:pPr>{spacing(table)}</w:pPr></w:style>
</w:styles>"""


# Long enough to wrap to two lines in a 200 pt cell at 12 pt Liberation Serif; the run prints
# how many baselines it actually found, so a variant that failed to wrap cannot pass unnoticed.
CELL_TEXT = ("alpha bravo charlie delta echo foxtrot golf hotel india juliett kilo lima "
             "mike november oscar")

DOCUMENT = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:tbl><w:tblPr><w:tblStyle w:val="Table1"/><w:tblW w:w="4000" w:type="dxa"/>
<w:tblLayout w:type="fixed"/></w:tblPr>
<w:tblGrid><w:gridCol w:w="4000"/></w:tblGrid>
<w:tr><w:tc><w:tcPr><w:tcW w:w="4000" w:type="dxa"/></w:tcPr>
<w:p><w:r><w:t xml:space="preserve">{CELL_TEXT}</w:t></w:r></w:p>
</w:tc></w:tr></w:tbl>
<w:p><w:r><w:t xml:space="preserve">{CELL_TEXT}</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0"/>
</w:sectPr></w:body></w:document>"""


def build(path: Path, defaults: int | None, normal: int | None, table: int | None) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/styles.xml", styles(defaults, normal, table))
        z.writestr("word/document.xml", DOCUMENT)


def baselines(pdf: Path, ops: Path) -> tuple[list[float], set]:
    out = subprocess.run([str(ops), "dump", str(pdf), "--page", "1"],
                         capture_output=True, text=True).stdout
    faces: set[str] = set()
    ys: set[float] = set()
    for line in out.splitlines():
        m = re.match(r"\s*text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(\S+)\s+(\S+)", line)
        if not m:
            continue
        faces.add(m.group(4).split("+")[-1])
        ys.add(round(float(m.group(2)), 2))
    return sorted(ys, reverse=True), faces


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

    print(f"{'variant':>19} {'defaults':>9} {'Normal':>7} {'table':>6} "
          f"{'cell pitch LO':>14} {'ours':>7} {'body pitch LO':>14} {'ours':>7}  faces")
    for name, (defaults, normal, table) in VARIANTS.items():
        src = work / f"{name}.docx"
        build(src, defaults, normal, table)
        subprocess.run(
            ["soffice", "--headless", f"-env:UserInstallation=file://{work}/prof",
             "--convert-to", "pdf", "--outdir", str(work / "ref"), str(src)],
            capture_output=True, timeout=240)
        subprocess.run([str(cli), "render", "--outdir", str(work / "ours"), str(src)],
                       capture_output=True, timeout=240)
        ref_pdf, our_pdf = work / "ref" / f"{name}.pdf", work / "ours" / f"{name}.pdf"
        if not ref_pdf.exists() or not our_pdf.exists():
            print(f"{name:>19}  CONVERT FAILED")
            continue
        ry, faces = baselines(ref_pdf, ops)
        oy, _ = baselines(our_pdf, ops)
        if len(ry) < 4 or len(oy) < 4:
            print(f"{name:>19}  EXPECTED FOUR BASELINES, GOT {len(ry)} and {len(oy)}")
            continue
        print(f"{name:>19} {str(defaults):>9} {str(normal):>7} {str(table):>6} "
              f"{ry[0] - ry[1]:>14.2f} {oy[0] - oy[1]:>7.2f} "
              f"{ry[2] - ry[3]:>14.2f} {oy[2] - oy[3]:>7.2f}  {','.join(sorted(faces))}")

    print()
    print("readings: 13.80 is single, 15.87 is 276/240, 27.60 is 480/240 for 12 pt Liberation Serif")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
