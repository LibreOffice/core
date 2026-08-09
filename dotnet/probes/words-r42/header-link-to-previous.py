#!/usr/bin/env python3
"""What does LibreOffice do with a section that names *some* headers but no default one?

§17.10.1 says a slot a `w:sectPr` does not name is inherited from the section before it, which is
what "link to previous" writes, and it is what `DocxReader.Furniture` implements. On
`words/batch-014/docx/UG.CAO.00133 … Language.docx` the reference does something else: sections 1,
2 and 4 each name an *even* and a *first* header and no default one, and LibreOffice's own flat-ODF
export of the document gives all three an **empty** `<style:header>` while section 0's and section
3's carry the logo. Every page but five therefore has no running head in the reference, and we draw
one on all eighteen — 244 words of the document's 244-word surplus.

The corpus cannot separate the two readings of that, because every section in the document names
at least one header. So this authors the pair that can: two sections, the first with a default
header, the second naming nothing / only a first / only an even / a default of its own.

    header-link-to-previous.py /abs/scratch/dir

Each variant is written, converted with `soffice`, and the second page's text reported. This
measures a *presence*, not a length, so it is not exposed to the probe-style trap.
"""
from __future__ import annotations

import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

NS = (
    'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
    'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
)

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
{headers}
</Types>"""

HEADER_TYPE = ('<Override PartName="/word/header{n}.xml" ContentType='
               '"application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>')

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

# Stated explicitly: a package with no `word/styles.xml` lays out in the application's fallback
# face rather than the one a real document gets. This probe counts words rather than measuring
# them, but the habit is cheap and the file is three lines.
STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}><w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="22"/>
</w:rPr></w:rPrDefault></w:docDefaults></w:styles>"""


def header(text: str) -> str:
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<w:hdr {NS}>{"<w:p><w:r><w:t>" + text + "</w:t></w:r></w:p>" if text else "<w:p/>"}</w:hdr>')


def document(second_section_refs: str) -> str:
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:p><w:pPr><w:sectPr>
<w:headerReference w:type="default" r:id="rIdH1"/>
<w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="708" w:footer="708"/>
</w:sectPr></w:pPr><w:r><w:t>FIRSTSECTIONBODY</w:t></w:r></w:p>
<w:p><w:r><w:t>SECONDSECTIONBODY</w:t></w:r></w:p>
<w:sectPr>{second_section_refs}
<w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="708" w:footer="708"/>
</w:sectPr></w:body></w:document>"""


VARIANTS = {
    # name: (second section's header references, extra header parts beyond header1)
    "names-nothing": ("", {}),
    "names-first-only": ('<w:headerReference w:type="first" r:id="rIdH2"/>', {2: "FIRSTPAGEHEAD"}),
    "names-even-only": ('<w:headerReference w:type="even" r:id="rIdH2"/>', {2: "EVENPAGEHEAD"}),
    "names-first-and-even": (
        '<w:headerReference w:type="first" r:id="rIdH2"/>'
        '<w:headerReference w:type="even" r:id="rIdH3"/>',
        {2: "FIRSTPAGEHEAD", 3: "EVENPAGEHEAD"}),
    "names-its-own-default": (
        '<w:headerReference w:type="default" r:id="rIdH2"/>', {2: "SECONDDEFAULT"}),
    # The shape `UG.CAO.00133` actually has: Word writes the unused first- and even-page parts as
    # a bare paragraph mark, so the second section names two headers and both are *empty*.
    "names-empty-first-and-even": (
        '<w:headerReference w:type="first" r:id="rIdH2"/>'
        '<w:headerReference w:type="even" r:id="rIdH3"/>',
        {2: "", 3: ""}),
}


def build(path: Path, refs: str, extra: dict[int, str]) -> None:
    headers = {1: "FIRSTSECTIONHEAD", **extra}
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("[Content_Types].xml", CONTENT_TYPES.format(
            headers="\n".join(HEADER_TYPE.format(n=n) for n in headers)))
        zf.writestr("_rels/.rels", ROOT_RELS)
        zf.writestr("word/document.xml", document(refs))
        zf.writestr("word/styles.xml", STYLES)
        rels = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
                '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">',
                '<Relationship Id="rIdS" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                'relationships/styles" Target="styles.xml"/>']
        for n, text in headers.items():
            zf.writestr(f"word/header{n}.xml", header(text))
            rels.append(f'<Relationship Id="rIdH{n}" Type="http://schemas.openxmlformats.org/'
                        f'officeDocument/2006/relationships/header" Target="header{n}.xml"/>')
        rels.append("</Relationships>")
        zf.writestr("word/_rels/document.xml.rels", "\n".join(rels))


def page_text(pdf: Path, page: int) -> str:
    out = subprocess.run(["pdftotext", "-f", str(page), "-l", str(page), str(pdf), "-"],
                         capture_output=True, text=True).stdout
    return " ".join(out.split())


def main() -> int:
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/header-link-probe")
    out.mkdir(parents=True, exist_ok=True)
    profile = out / "prof"

    print(f"{'variant':24s} {'page 1':38s} page 2")
    for name, (refs, extra) in VARIANTS.items():
        docx = out / f"{name}.docx"
        build(docx, refs, extra)
        pdfdir = out / name
        shutil.rmtree(pdfdir, ignore_errors=True)
        pdfdir.mkdir()
        subprocess.run(
            ["soffice", f"-env:UserInstallation=file://{profile}", "--headless",
             "--convert-to", "pdf", "--outdir", str(pdfdir), str(docx)],
            capture_output=True, text=True, timeout=240)
        pdf = pdfdir / f"{name}.pdf"
        if not pdf.exists():
            print(f"{name:24s} — no output")
            continue
        print(f"{name:24s} {page_text(pdf, 1):38s} {page_text(pdf, 2)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
