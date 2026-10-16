#!/usr/bin/env python3
"""Does a DOCX paragraph that states no `w:widowControl` anywhere get widow/orphan control?

`WordParagraphFormats` answers **no**: `IsOn(... "widowControl" ...)` is false when the element is
absent, so `OrphanLines`/`WidowLines` are 0 unless a paragraph or a style in its chain turns the
flag on. The WW8 reader answers **yes** — `Ww8LayoutFormat` has `HasWidowControl ?? true`. One
reader defaults it on and the other defaults it off, which is already a reason to look.

`words/batch-014/docx/gpp-pr-top-7-office-markets-4q-2023.docx` says the reference answers yes. No
`w:widowControl` appears in its `word/styles.xml` and none in `word/settings.xml`; its "In the year
2023 …" paragraph is two lines; the reference puts **both on page 2** while page 1 still has room
for one, and LibreOffice's own flat-ODF export of the file gives the `Standard` paragraph style
`fo:orphans="2" fo:widows="2"`.

That is one document, and the corpus cannot separate "Writer's default paragraph style carries 2"
from "something in this file turns it on". This authors the pair that can.

    widow-orphan-default.py /abs/scratch/dir

**The design, and why the control is in the same run.** Each variant is a short page filled with N
single-line paragraphs followed by a four-line target paragraph whose lines are one unbreakable
28-character token each, so the wrap is not a prediction. The `off` variant — the same file with
`<w:widowControl w:val="0"/>` on the target — *measures the room* at the foot of page 1: with the
control off, the number of target lines on page 1 is exactly how many fit. The `bare` variant
states nothing at all. Comparing the two at the same N is what separates a default from a page
that simply had no room.

This measures which page a token lands on — a count, not a length — so it is not exposed to the
probe-style trap. `word/styles.xml` states Liberation Serif 12 pt regardless.
"""
from __future__ import annotations

import re
import shutil
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
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rIdS" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rIdT" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>"""

# Stated explicitly: a package with no styles part lays out in the application's fallback face.
def styles_part(doc_defaults: str) -> str:
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}>{doc_defaults}
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
<w:pPr><w:spacing w:line="240" w:lineRule="exact"/></w:pPr>
<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr>
</w:style></w:styles>"""


RPR_DEFAULT = ('<w:rPrDefault><w:rPr><w:rFonts w:ascii="Liberation Serif" '
               'w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr></w:rPrDefault>')

# The variable this probe turns out to be about. `w:pPrDefault` is *empty* in the corpus document
# that raised the question, so what is being varied is its presence, not its content.
DD_NO_PPR = f"<w:docDefaults>{RPR_DEFAULT}</w:docDefaults>"
DD_EMPTY_PPR = f"<w:docDefaults>{RPR_DEFAULT}<w:pPrDefault/></w:docDefaults>"
DD_FULL_PPR = (f"<w:docDefaults>{RPR_DEFAULT}<w:pPrDefault><w:pPr>"
               f'<w:spacing w:after="0"/></w:pPr></w:pPrDefault></w:docDefaults>')
DD_PPR_WIDOW_OFF = (f"<w:docDefaults>{RPR_DEFAULT}<w:pPrDefault><w:pPr>"
                    f'<w:widowControl w:val="0"/></w:pPr></w:pPrDefault></w:docDefaults>')
DD_NONE = ""


def settings(doc_level: str) -> str:
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<w:settings {NS}>{doc_level}</w:settings>')


# One unbreakable token per line: 28 capital Ms at 12 pt is about 325 pt and the column is 523 pt,
# so two never share a line and the wrap is arithmetic rather than a guess.
def target_lines(n: int) -> str:
    return " ".join(f"L{i}" + "M" * 27 for i in range(1, n + 1))


def document(fillers: int, target_pPr: str, lines: int) -> str:
    body = "".join(
        f'<w:p><w:r><w:t>F{i:02d}filler</w:t></w:r></w:p>' for i in range(1, fillers + 1))
    body += (f'<w:p><w:pPr>{target_pPr}</w:pPr><w:r><w:t>{target_lines(lines)}</w:t></w:r></w:p>')
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>{body}
<w:sectPr><w:pgSz w:w="11906" w:h="5670"/>
<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" w:header="0" w:footer="0"/>
</w:sectPr></w:body></w:document>"""


def build(path: Path, fillers: int, target_pPr: str, doc_level: str, lines: int,
          doc_defaults: str) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/styles.xml", styles_part(doc_defaults))
        z.writestr("word/settings.xml", settings(doc_level))
        z.writestr("word/document.xml", document(fillers, target_pPr, lines))


PPR = "<w:spacing w:line=\"240\" w:lineRule=\"exact\"/>"

VARIANTS = {
    # name                    paragraph pPr                          settings   docDefaults
    "no-pPrDefault":         (PPR,                                   "", DD_NO_PPR),
    "no-docDefaults":        (PPR,                                   "", DD_NONE),
    "empty-pPrDefault":      (PPR,                                   "", DD_EMPTY_PPR),
    "pPrDefault-with-pPr":   (PPR,                                   "", DD_FULL_PPR),
    "pPrDefault-widow-off":  (PPR,                                   "", DD_PPR_WIDOW_OFF),
    "pPrDefault-para-off":   (PPR + '<w:widowControl w:val="0"/>',   "", DD_EMPTY_PPR),
    "para-off":              (PPR + '<w:widowControl w:val="0"/>',   "", DD_NO_PPR),
    "para-on":               (PPR + '<w:widowControl/>',             "", DD_NO_PPR),
    "settings-on":           (PPR,                                   "<w:widowControl/>", DD_NO_PPR),
}


def pages_of(pdf: Path) -> list[str]:
    out = subprocess.run(["pdftotext", "-layout", str(pdf), "-"],
                         capture_output=True, text=True).stdout
    return out.split("\f")


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    work = Path(sys.argv[1]).resolve()
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    profile = work / "prof"

    lines = int(sys.argv[2]) if len(sys.argv) > 2 else 4
    print(f"target paragraph: {lines} lines, one 28-character token each")
    print(f"{'fillers':>7}  {'variant':<22} {'target lines on page 1':>22}  which")
    rows = {}
    for fillers in range(13, 18):
        for name, (ppr, doc_level, dd) in VARIANTS.items():
            stem = f"n{fillers:02d}-{name}"
            docx = work / f"{stem}.docx"
            build(docx, fillers, ppr, doc_level, lines, dd)
            subprocess.run(
                ["soffice", "--headless", f"-env:UserInstallation=file://{profile}",
                 "--convert-to", "pdf", "--outdir", str(work), str(docx)],
                capture_output=True, timeout=240)
            pdf = work / f"{stem}.pdf"
            if not pdf.exists():
                print(f"{fillers:>7}  {name:<22} {'CONVERT FAILED':>22}")
                continue
            pages = pages_of(pdf)
            on1 = [i for i in range(1, lines + 1)
                   if re.search(rf"\bL{i}M", pages[0])]
            rows[(fillers, name)] = len(on1)
            print(f"{fillers:>7}  {name:<22} {len(on1):>22}  "
                  f"{','.join('L%d' % i for i in on1) or '-'}")
        print()

    print("`para-off` measures the room at the foot of page 1: with the control off, the number")
    print("of target lines there is exactly how many fit. A variant that puts fewer lines on")
    print("page 1 than `para-off` does at the same filler count has widow/orphan control on.")
    fillers = list(range(13, 18))
    for name in VARIANTS:
        on = [f for f in fillers if rows.get((f, name)) is not None
              and rows[(f, name)] < rows[(f, "para-off")]]
        off = [f for f in fillers if rows.get((f, name)) is not None
               and rows[(f, name)] == rows[(f, "para-off")]]
        print(f"  {name:<24} control ON at fillers {on or '-'}, same as off at {off or '-'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
