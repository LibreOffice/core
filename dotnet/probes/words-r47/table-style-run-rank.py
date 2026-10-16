#!/usr/bin/env python3
"""Where does a table style's own `w:rPr` rank against the other run-property layers?

    table-style-run-rank.py /abs/scratch/dir

`unnamed-style.py` shows LibreOffice applying a **named** table style's `w:rPr` to the
runs in its cells — 10 pt where the document defaults and the `Normal` style both say 12 —
and shows us not applying it at all. Before wiring it in, the rank has to be measured
rather than read off §17.7.2, which puts a table style *below* every paragraph and
character style and is already contradicted by that first row: `Normal` states `w:sz 24`
explicitly there and the table style's 20 still won.

Writer's reason, cited as the hypothesis: `DomainMapperTableHandler` pushes a table
style's properties into the cell's own property map, so they arrive as **direct**
formatting rather than as a style layer, which beats a style and loses to the run.

Five variants, each adding exactly one competing layer to the same base (table style
`w:sz 20`, `Normal` and `w:docDefaults` both `w:sz 24`):

    base            nothing else                       -> 10 pt if the table style applies
    vs-run          the run states w:sz 28             -> 14 pt if a run beats it
    vs-charstyle    the run names a character style 28 -> 14 pt if a character style beats it
    vs-parastyle    the paragraph names a style 28     -> the decisive rank question
    vs-cellpara     the paragraph's own pPr/rPr is 28  -> the paragraph mark, which should
                                                          not reach the run at all

The observable is the drawn size, which is discrete, so this is a count rather than a
length; the faces are printed anyway.
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
<w:styles {NS}>
<w:docDefaults><w:rPrDefault><w:rPr>{FONT}</w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr/></w:pPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
<w:rPr>{FONT}</w:rPr></w:style>
<w:style w:type="table" w:default="1" w:styleId="TableNormal"><w:name w:val="Normal Table"/></w:style>
<w:style w:type="table" w:styleId="Probe"><w:name w:val="Probe"/>
<w:basedOn w:val="TableNormal"/><w:rPr><w:sz w:val="20"/><w:szCs w:val="20"/></w:rPr></w:style>
<w:style w:type="paragraph" w:styleId="Bigger"><w:name w:val="Bigger"/>
<w:basedOn w:val="Normal"/><w:rPr><w:sz w:val="28"/><w:szCs w:val="28"/></w:rPr></w:style>
<w:style w:type="character" w:styleId="BigChar"><w:name w:val="Big Char"/>
<w:rPr><w:sz w:val="28"/><w:szCs w:val="28"/></w:rPr></w:style>
</w:styles>"""

BIG = '<w:sz w:val="28"/><w:szCs w:val="28"/>'

VARIANTS = {
    "base": ("", ""),
    "vs-run": ("", f"<w:rPr>{BIG}</w:rPr>"),
    "vs-charstyle": ("", '<w:rPr><w:rStyle w:val="BigChar"/></w:rPr>'),
    "vs-parastyle": ('<w:pStyle w:val="Bigger"/>', ""),
    "vs-cellpara": (f"<w:rPr>{BIG}</w:rPr>", ""),
}


def document(ppr: str, rpr: str) -> str:
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:tbl><w:tblPr><w:tblStyle w:val="Probe"/><w:tblW w:w="6000" w:type="dxa"/>
<w:tblLayout w:type="fixed"/></w:tblPr><w:tblGrid><w:gridCol w:w="6000"/></w:tblGrid>
<w:tr><w:tc><w:tcPr><w:tcW w:w="6000" w:type="dxa"/></w:tcPr>
<w:p><w:pPr>{ppr}</w:pPr><w:r>{rpr}<w:t>PROBEMARK</w:t></w:r></w:p></w:tc></w:tr></w:tbl>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0"/>
</w:sectPr></w:body></w:document>"""


def build(path: Path, ppr: str, rpr: str) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", document(ppr, rpr))


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

    print(f"{'variant':>13} {'LibreOffice':>12} {'ours':>10}  agree  faces")
    for name, (ppr, rpr) in VARIANTS.items():
        src = work / f"{name}.docx"
        build(src, ppr, rpr)
        subprocess.run(
            ["soffice", "--headless", f"-env:UserInstallation=file://{work}/prof",
             "--convert-to", "pdf", "--outdir", str(work / "ref"), str(src)],
            capture_output=True, timeout=240)
        subprocess.run([str(cli), "render", "--outdir", str(work / "ours"), str(src)],
                       capture_output=True, timeout=240)
        ref_pdf, our_pdf = work / "ref" / f"{name}.pdf", work / "ours" / f"{name}.pdf"
        if not ref_pdf.exists() or not our_pdf.exists():
            print(f"{name:>13}  CONVERT FAILED")
            continue
        rs, rf = drawn(ref_pdf, ops)
        os_, _ = drawn(our_pdf, ops)
        print(f"{name:>13} {rs:>12} {os_:>10}  {'ok ' if rs == os_ else 'NO ':>5}  {rf}")

    print()
    print("10.00pt = the table style won, 12.00pt = the defaults won, 14.00pt = the other layer won")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
