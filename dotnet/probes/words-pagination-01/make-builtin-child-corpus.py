#!/usr/bin/env python3
"""Author `tests/corpus/features/style-one-sided-spacing-builtin-child.docx`.

The sibling `style-one-sided-spacing.docx` covers a half-stated `w:spacing` on a style whose
own `w:name` Writer does not know, so every one of its four cases takes the fallback through
the *parent*. This one covers the other half of the rule: a child whose own `w:name` IS one of
Writer's, where the pool row that fills the unstated margin is the child's and not the
parent's.

Five paragraph styles, each based on a parent declared *after* it, each stating only
`w:before="480"` — a value deliberately unlike any pool row, so that "mirror the stated
value" would be visible as 24 pt and is not:

    styleId        w:name        parent (w:name)          measured below
    H4Custom       heading 4     Par Custom (custom)       6 pt   own name answers
    H5Heading      heading 5     Par Heading (heading 2)   6 pt   parent answers, same number
    BodyKid        Body Text     Par Custom (custom)       0 pt   own name does NOT answer
    CustomHeading  Custom Kid    Par Heading (heading 2)   6 pt   parent answers
    CustomCustom   Custom Kid 2  Par Custom (custom)       0 pt   neither answers

`BodyKid` is the one that stops this being read as "any built-in name answers from either
position". "Body Text" is 7 pt below in Writer's pool and answers with that 7 pt when it is
the *parent* — and answers with nothing at all when it is the child. Only the heading family
(Heading 1-9, Title, Subtitle) answers from the child, and always with the `Heading` base's
12 pt / 6 pt rather than the per-level rows `DocumentStylePoolManager.cxx` declares.

Nothing here is predicted. Every figure in that table was read back out of
`soffice --convert-to fodt` on the installed 26.2.4.2, and the fifteen-name sweep in
`one-sided-spacing-source.py` is what establishes the membership.

Run with no arguments to write the file; pass a directory to also convert it with the
installed soffice and print the margins LibreOffice resolves, which is how the expectations
above were obtained rather than predicted.
"""
from __future__ import annotations

import re
import subprocess
import sys
import zipfile
from pathlib import Path

NS = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

CASES = [
    ("H4Custom",      "heading 4",    "ParCustom",  "Par Custom"),
    ("H5Heading",     "heading 5",    "ParHeading", "heading 2"),
    ("BodyKid",       "Body Text",    "ParCustom",  "Par Custom"),
    ("CustomHeading", "Custom Kid",   "ParHeading", "heading 2"),
    ("CustomCustom",  "Custom Kid 2", "ParCustom",  "Par Custom"),
]

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


def document() -> str:
    body = "".join(
        f'<w:p><w:pPr><w:pStyle w:val="{sid}"/></w:pPr><w:r><w:t>{sid}</w:t></w:r></w:p>'
        f'<w:p><w:r><w:t>plain</w:t></w:r></w:p>'
        for sid, _, _, _ in CASES)
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<w:document {NS}><w:body>{body}'
            f'<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
            f'<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
            f' w:header="0" w:footer="0" w:gutter="0"/></w:sectPr>'
            f'</w:body></w:document>')


def styles() -> str:
    kids = "".join(
        f'<w:style w:type="paragraph" w:styleId="{sid}"><w:name w:val="{name}"/>'
        f'<w:basedOn w:val="{pid}"/><w:next w:val="Normal"/><w:qFormat/>'
        f'<w:pPr><w:spacing w:before="480"/></w:pPr>'
        f'<w:rPr><w:sz w:val="22"/></w:rPr></w:style>'
        for sid, name, pid, _ in CASES)
    # Both parents are declared *after* every child, and both state a w:spacing of their own
    # so that plain inheritance would be visible as 15 pt below rather than any pool row.
    parents = "".join(
        f'<w:style w:type="paragraph" w:styleId="{pid}"><w:name w:val="{pname}"/>'
        f'<w:basedOn w:val="Normal"/><w:next w:val="Normal"/><w:qFormat/>'
        f'<w:pPr><w:spacing w:before="240" w:after="300"/></w:pPr>'
        f'<w:rPr><w:sz w:val="22"/></w:rPr></w:style>'
        for pid, pname in [("ParCustom", "Par Custom"), ("ParHeading", "heading 2")])
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<w:styles {NS}>'
            f'<w:docDefaults><w:rPrDefault><w:rPr>'
            f'<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
            f'<w:sz w:val="22"/></w:rPr></w:rPrDefault>'
            f'<w:pPrDefault><w:pPr/></w:pPrDefault></w:docDefaults>'
            f'<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
            f'<w:name w:val="Normal"/><w:qFormat/></w:style>'
            f'{kids}{parents}</w:styles>')


def main() -> int:
    target = (Path(__file__).resolve().parents[2]
              / "tests/corpus/features/style-one-sided-spacing-builtin-child.docx")
    with zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/document.xml", document())
        z.writestr("word/styles.xml", styles())
    print(f"wrote {target} ({target.stat().st_size} bytes)")

    if len(sys.argv) < 2:
        return 0

    out = Path(sys.argv[1]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        ["soffice", f"-env:UserInstallation=file://{out / 'prof'}", "--headless",
         "--convert-to", "fodt", "--outdir", str(out), str(target)],
        check=False, capture_output=True, timeout=300)
    text = (out / f"{target.stem}.fodt").read_text(encoding="utf8", errors="replace")
    print(f"{'style':34}{'margin-top':>12}{'margin-bottom':>15}")
    for m in re.finditer(r'<style:style style:name="([^"]+)"([^>]*)>(.*?)</style:style>',
                         text, re.S):
        if 'style:family="paragraph"' not in m.group(2):
            continue
        pp = re.search(r"<style:paragraph-properties([^>]*)", m.group(3))
        if not pp:
            continue
        top = re.search(r'fo:margin-top="([^"]+)"', pp.group(1))
        bot = re.search(r'fo:margin-bottom="([^"]+)"', pp.group(1))
        if top or bot:
            print(f"{m.group(1):34}{top.group(1) if top else '-':>12}"
                  f"{bot.group(1) if bot else '-':>15}")
    print("\n0.3335in=480tw(24pt)  0.2085in=300tw(15pt)  0.0972in=140tw(7pt)"
          "  0.0835in=120tw(6pt)  0in=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
