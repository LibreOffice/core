#!/usr/bin/env python3
"""Is a `w:style` with no `w:name` used at all?

    unnamed-style.py /abs/scratch/dir

`template---tpr-technical-progress-report-with-guidance.docx` reads 7/8. Its page 2 sets
cell text on a 15.45 pt pitch in the reference and 13.45 pt in ours — 115% against 100% of
Carlito's 11 pt line — and its table style states `<w:spacing w:line="240"/>` while
`w:docDefaults` states 276. §17.7.2 puts a table style above the document defaults, so the
table style should win, and it does in ours.

Two causal mutations of the real file say the reference is not resolving that table style
at all:

    table style 240 -> 480     cell pitch unchanged at 15.45
    docDefaults 276 -> 480     cell pitch moves to 26.90
    add <w:name w:val="Table1"/>   cell text becomes 10 pt, the style's own w:sz

The style is one of **sixteen in that file with a `w:styleId` and no `w:name`** — a Google
Docs export — and `StyleSheetTable.cxx`:774 is explicit: on an OOXML import an entry whose
`m_sStyleName` is empty is **not appended to the style table or its id map**, so nothing can
ever reference it.

That citation is the hypothesis; these six authored variants are the evidence, and they
also settle the **scope**, which the corpus document cannot: the rule is about `w:style`
in general or only about table styles.

    table-named / table-unnamed     w:tblStyle, style sets w:sz 20 (10 pt)
    para-named  / para-unnamed      w:pStyle,   style sets w:sz 20
    char-named  / char-unnamed      w:rStyle,   style sets w:sz 20

Each pair varies exactly one thing: whether `<w:name>` is present. The observable is the
**drawn size** reported by `pdf-ops.py`, which is discrete — 10.00 against 12.00 — so this
is a count rather than a length and cannot be spoiled by a font substitution; the faces are
printed anyway.
"""
from __future__ import annotations

import re
import subprocess
import sys
import zipfile
from pathlib import Path

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')

FONT = '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/>'
SMALL = '<w:sz w:val="20"/><w:szCs w:val="20"/>'

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

CASES = ["table", "para", "char"]


def styles(kind: str, named: bool) -> str:
    name = '<w:name w:val="Probe"/>' if named else ""
    if kind == "table":
        target = (f'<w:style w:type="table" w:styleId="Probe">{name}'
                  f'<w:basedOn w:val="TableNormal"/><w:rPr>{SMALL}</w:rPr></w:style>')
    elif kind == "para":
        target = (f'<w:style w:type="paragraph" w:styleId="Probe">{name}'
                  f'<w:basedOn w:val="Normal"/><w:rPr>{SMALL}</w:rPr></w:style>')
    else:
        target = (f'<w:style w:type="character" w:styleId="Probe">{name}'
                  f'<w:rPr>{SMALL}</w:rPr></w:style>')
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}>
<w:docDefaults><w:rPrDefault><w:rPr>{FONT}</w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr/></w:pPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
<w:rPr>{FONT}</w:rPr></w:style>
<w:style w:type="table" w:default="1" w:styleId="TableNormal"><w:name w:val="Normal Table"/></w:style>
{target}
</w:styles>"""


def document(kind: str) -> str:
    if kind == "table":
        content = """<w:tbl><w:tblPr><w:tblStyle w:val="Probe"/><w:tblW w:w="6000" w:type="dxa"/>
<w:tblLayout w:type="fixed"/></w:tblPr><w:tblGrid><w:gridCol w:w="6000"/></w:tblGrid>
<w:tr><w:tc><w:tcPr><w:tcW w:w="6000" w:type="dxa"/></w:tcPr>
<w:p><w:r><w:t>PROBEMARK</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"""
    elif kind == "para":
        content = ('<w:p><w:pPr><w:pStyle w:val="Probe"/></w:pPr>'
                   '<w:r><w:t>PROBEMARK</w:t></w:r></w:p>')
    else:
        content = ('<w:p><w:r><w:rPr><w:rStyle w:val="Probe"/></w:rPr>'
                   '<w:t>PROBEMARK</w:t></w:r></w:p>')
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
{content}
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0"/>
</w:sectPr></w:body></w:document>"""


def build(path: Path, kind: str, named: bool) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/styles.xml", styles(kind, named))
        z.writestr("word/document.xml", document(kind))


def drawn(pdf: Path, ops: Path) -> tuple[str, str]:
    out = subprocess.run([str(ops), "dump", str(pdf), "--page", "1"],
                         capture_output=True, text=True).stdout
    for line in out.splitlines():
        m = re.match(r"\s*text\s+p\d+\s+\(\s*[-\d.]+,\s*[-\d.]+\)\s+(\S+)\s+(\S+)", line)
        if m and "PROBEMARK" in line:
            return m.group(1), m.group(2).split("+")[-1]
    return "?", "?"


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

    print(f"{'style':>7} {'w:name':>7} {'LibreOffice':>12} {'ours':>10}  agree  faces")
    for kind in CASES:
        for named in (True, False):
            stem = f"{kind}-{'named' if named else 'unnamed'}"
            src = work / f"{stem}.docx"
            build(src, kind, named)
            subprocess.run(
                ["soffice", "--headless", f"-env:UserInstallation=file://{work}/prof",
                 "--convert-to", "pdf", "--outdir", str(work / "ref"), str(src)],
                capture_output=True, timeout=240)
            subprocess.run([str(cli), "render", "--outdir", str(work / "ours"), str(src)],
                           capture_output=True, timeout=240)
            ref_pdf, our_pdf = work / "ref" / f"{stem}.pdf", work / "ours" / f"{stem}.pdf"
            if not ref_pdf.exists() or not our_pdf.exists():
                print(f"{kind:>7} {str(named):>7}  CONVERT FAILED")
                continue
            rs, rf = drawn(ref_pdf, ops)
            os_, _ = drawn(our_pdf, ops)
            print(f"{kind:>7} {('yes' if named else 'no'):>7} {rs:>12} {os_:>10}  "
                  f"{'ok ' if rs == os_ else 'NO ':>5}  {rf}")

    print()
    print("10.00pt means the style was applied; 12.00pt means it was not.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
