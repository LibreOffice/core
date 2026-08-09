#!/usr/bin/env python3
"""Does the *content* of a header decide whether the next section inherits it?

Round 42 authored six section shapes and LibreOffice inherited the first section's header in all
six, which refutes "our link-to-previous rule is wrong". Round 43's `header-slot-mutations.py`
then mutated the real document — `UG.CAO.00133 … Language.docx` — and found the reference draws no
head on sections 1, 2 and 4 **even when those sections name no header at all**. Both measurements
reproduce, so the variable that separates them is not in the `w:sectPr`.

The remaining difference between the two files is what the *inherited* header holds. Round 42's
probe puts a line of text in it. Every EASA document in this corpus lays its running head out as a
**table**, with a logo in one cell and the title and revision in the next.

So this holds the section markup fixed — section 1 names nothing, the shape round 42 measured
inheriting — and varies only the first section's header content:

    text            one paragraph of text            (round 42's shape, the control)
    table           a 1x2 table of text
    nested-table    a table whose cell holds a table
    image           a paragraph holding an inline PNG
    table-image     a table with the PNG in its first cell  (the corpus document's shape)
    text-then-table a paragraph of text and then a table

    header-inherit-content-shape.py /abs/scratch/dir

Page 2 belongs to the second section. What it prints is the answer: the first section's header
text means inherited, nothing means not.
"""
from __future__ import annotations

import base64
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

NS = (
    'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
    'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
    'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" '
    'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
    'xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture"'
)

# A 4x4 red PNG, so the header can hold a real picture without a dependency on the corpus.
PNG = base64.b64decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAYAAACp8Z5+AAAAFUlEQVR42mP8z8BQz0AEYBxVSF+F"
    "ABJADveWkH6oAAAAAElFTkSuQmCC")

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

# Stated explicitly, per the skill: a package with no `word/styles.xml` lays out in the
# application's fallback face. This probe measures a *presence* per page rather than a length, so
# it is not exposed to that trap, but the file is three lines.
STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}><w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="22"/>
</w:rPr></w:rPrDefault></w:docDefaults></w:styles>"""

DOCUMENT = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:p><w:pPr><w:sectPr>
<w:headerReference w:type="default" r:id="rIdH1"/>
<w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="708" w:footer="708"/>
</w:sectPr></w:pPr><w:r><w:t>FIRSTSECTIONBODY</w:t></w:r></w:p>
<w:p><w:r><w:t>SECONDSECTIONBODY</w:t></w:r></w:p>
<w:sectPr>
<w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="708" w:footer="708"/>
</w:sectPr></w:body></w:document>"""

DRAWING = """<w:r><w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0">
<wp:extent cx="304800" cy="304800"/><wp:docPr id="1" name="Picture 1"/>
<a:graphic><a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">
<pic:pic><pic:nvPicPr><pic:cNvPr id="1" name="p.png"/><pic:cNvPicPr/></pic:nvPicPr>
<pic:blipFill><a:blip r:embed="rIdIMG"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>
<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="304800" cy="304800"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>
</a:graphicData></a:graphic></wp:inline></w:drawing></w:r>"""

TBL_PR = ('<w:tblPr><w:tblW w:w="0" w:type="auto"/><w:tblBorders>'
          '<w:top w:val="single" w:sz="4" w:space="0" w:color="auto"/>'
          '<w:left w:val="single" w:sz="4" w:space="0" w:color="auto"/>'
          '<w:bottom w:val="single" w:sz="4" w:space="0" w:color="auto"/>'
          '<w:right w:val="single" w:sz="4" w:space="0" w:color="auto"/>'
          '</w:tblBorders></w:tblPr><w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="6000"/></w:tblGrid>')


def cell(body: str) -> str:
    return f'<w:tc><w:tcPr><w:tcW w:w="3000" w:type="dxa"/></w:tcPr>{body}</w:tc>'


def para(text: str) -> str:
    return f"<w:p><w:r><w:t>{text}</w:t></w:r></w:p>"


def table(first: str, second: str, trailing: bool = True) -> str:
    # The trailing `<w:p/>` matters, and finding that out is what this probe is for. Word writes a
    # header whose last element is `</w:tbl>` — no paragraph after the table — and
    # `header-inherit-bisect.py` shows that is what stops LibreOffice copying the header into the
    # next section. Every variant here carries the paragraph except the one that says it does not.
    return f"<w:tbl>{TBL_PR}<w:tr>{cell(first)}{cell(second)}</w:tr></w:tbl>" + ("<w:p/>" if trailing else "")


BODIES = {
    "text": para("FIRSTSECTIONHEAD"),
    "table": table(para("FIRSTSECTIONHEAD"), para("REVISION1")),
    "nested-table": table(f"<w:tbl>{TBL_PR}<w:tr>{cell(para('FIRSTSECTIONHEAD'))}"
                          f"{cell(para('REVISION1'))}</w:tr></w:tbl><w:p/>", para("OUTER")),
    "image": f"<w:p>{DRAWING}<w:r><w:t>FIRSTSECTIONHEAD</w:t></w:r></w:p>",
    "table-image": table(f"<w:p>{DRAWING}</w:p>", para("FIRSTSECTIONHEAD")),
    "text-then-table": para("FIRSTSECTIONHEAD") + table(para("CELLA"), para("CELLB")),
    # The corpus shape, exactly: a table with the logo in one cell and the title in the next, and
    # **no paragraph after it**. This is the only variant that does not inherit.
    "table-no-trailing-p": table(f"<w:p>{DRAWING}</w:p>", para("FIRSTSECTIONHEAD"), trailing=False),
    "table-trailing-p": table(f"<w:p>{DRAWING}</w:p>", para("FIRSTSECTIONHEAD"), trailing=True),
    # Whether the copy aborts *at* the table or is skipped whole. A paragraph before the table and
    # no paragraph after it: if page 2 carries FIRSTSECTIONHEAD and not the cells, the copy runs
    # until it reaches the table and then fails, which is what an exception inside
    # `copyHeaderFooterTextProperty` would look like from outside.
    "text-then-table-no-trailing-p":
        para("FIRSTSECTIONHEAD") + table(para("CELLA"), para("CELLB"), trailing=False),
}


def build(path: Path, header_body: str) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("[Content_Types].xml", CONTENT_TYPES)
        zf.writestr("_rels/.rels", ROOT_RELS)
        zf.writestr("word/document.xml", DOCUMENT)
        zf.writestr("word/styles.xml", STYLES)
        zf.writestr("word/media/p.png", PNG)
        zf.writestr("word/header1.xml",
                    f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
                    f"<w:hdr {NS}>{header_body}</w:hdr>")
        zf.writestr("word/_rels/document.xml.rels",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                    '<Relationship Id="rIdS" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                    'relationships/styles" Target="styles.xml"/>'
                    '<Relationship Id="rIdH1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                    'relationships/header" Target="header1.xml"/>'
                    "</Relationships>")
        zf.writestr("word/_rels/header1.xml.rels",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                    '<Relationship Id="rIdIMG" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                    'relationships/image" Target="media/p.png"/>'
                    "</Relationships>")


def page_text(pdf: Path, page: int) -> str:
    out = subprocess.run(["pdftotext", "-f", str(page), "-l", str(page), str(pdf), "-"],
                         capture_output=True, text=True).stdout
    return " ".join(out.split())


def main() -> int:
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/header-content-shape")
    out.mkdir(parents=True, exist_ok=True)
    profile = out / "prof"

    print(f"{'header content':18s} {'page 1':40s} page 2 (second section)")
    for name, body in BODIES.items():
        docx = out / f"{name}.docx"
        build(docx, body)
        pdfdir = out / name
        shutil.rmtree(pdfdir, ignore_errors=True)
        pdfdir.mkdir()
        subprocess.run(["soffice", f"-env:UserInstallation=file://{profile}", "--headless",
                        "--convert-to", "pdf", "--outdir", str(pdfdir), str(docx)],
                       capture_output=True, text=True, timeout=300)
        pdf = pdfdir / f"{name}.pdf"
        if not pdf.exists():
            print(f"{name:18s} — no output")
            continue
        print(f"{name:18s} {page_text(pdf, 1):40s} {page_text(pdf, 2)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
