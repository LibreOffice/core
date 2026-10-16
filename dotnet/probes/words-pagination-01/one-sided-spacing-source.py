#!/usr/bin/env python3
"""When a paragraph style states only one of `w:spacing/@w:before` and `@w:after`,
whose default fills the other half?

    one-sided-spacing-source.py /abs/scratch/dir

`WordStyles.CompleteOneSidedSpacing` implements one answer — Writer's pool spacing for the
**parent** style's `w:name` — and it was fitted to a probe that cannot distinguish it from
three other answers, because the corpus case it was aimed at
(`final-technical-report-template.docx`: a `heading 1` based on a `heading 2`) has the child
and the parent both mapping onto the same 12 pt / 6 pt pool row.

The Holdover Tables separate them. `Heading4` there has `w:name="heading 4"`, is based on a
custom `Notes/Cautions Heading` declared *after* it, and states only `w:before="120"`.
LibreOffice 26.2.4.2 resolves that style to `fo:margin-top="0.0835in"
fo:margin-bottom="0.0835in"` — 120 twips on *both* sides, where:

    parent-pool  (what we do)   after = 0     (a custom name has no pool row)
    inheritance                 after = 60    (the parent states w:after="60")
    child-pool                  after = 120   (Writer's "Heading 4" pool row is 12 pt / 6 pt)
    mirror the stated value     after = 120

so the real document narrows four answers to two, and cannot split those two.

These sixteen authored variants split them, and a second phase then measures *which names*
answer from the child position, which turns out to be far fewer than answer from the parent
position. The child states `w:before="480"` — deliberately
not 120 — so `mirror` predicts 480 and `child-pool` predicts 120. The four cells vary the
child's name (built-in / custom) against the parent's name (built-in / custom), and each is
run with the parent declared after the child and before it, which is the condition the whole
mechanism is supposed to turn on.

The observable is `fo:margin-bottom` on the resolved style in `soffice --convert-to fodt`
output. That is the importer's own answer read directly, with no layout, no font and no
rounding but the 1/100 mm the format stores — 120 twips is 0.0835in and 60 is 0.0417in, which
are two apart in the fourth decimal and cannot be confused.
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

DOCUMENT = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:p><w:pPr><w:pStyle w:val="Kid"/></w:pPr><w:r><w:t>child</w:t></w:r></w:p>
<w:p><w:r><w:t>after</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"
 w:header="0" w:footer="0" w:gutter="0"/></w:sectPr>
</w:body></w:document>"""

# The child states only `before`. 480 is chosen so that "mirror the stated value" and
# "Writer's Heading pool below (120)" cannot give the same answer.
CHILD_BEFORE = 480
PARENT_BEFORE = 300
PARENT_AFTER = 360


def styles(child_name: str, parent_name: str, parent_first: bool) -> str:
    kid = (f'<w:style w:type="paragraph" w:styleId="Kid"><w:name w:val="{child_name}"/>'
           f'<w:basedOn w:val="Par"/>'
           f'<w:pPr><w:spacing w:before="{CHILD_BEFORE}"/></w:pPr></w:style>')
    par = (f'<w:style w:type="paragraph" w:styleId="Par"><w:name w:val="{parent_name}"/>'
           f'<w:basedOn w:val="Normal"/>'
           f'<w:pPr><w:spacing w:before="{PARENT_BEFORE}" w:after="{PARENT_AFTER}"/></w:pPr>'
           f'</w:style>')
    body = (par + kid) if parent_first else (kid + par)
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}>
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Liberation Serif"
 w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr/></w:pPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
</w:style>
{body}
</w:styles>"""


def author(path: Path, child_name: str, parent_name: str, parent_first: bool) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/document.xml", DOCUMENT)
        z.writestr("word/styles.xml", styles(child_name, parent_name, parent_first))


def margins(fodt: Path) -> tuple[str, str]:
    """`fo:margin-top` and `fo:margin-bottom` of the style the child paragraph names."""
    text = fodt.read_text(encoding="utf8", errors="replace")
    # The child style is whatever `Kid` became; find it by its unique 480 twip top margin
    # only as a fallback — normally by display name.
    for m in re.finditer(r'<style:style style:name="([^"]+)"([^>]*)>(.*?)</style:style>',
                         text, re.S):
        head, body = m.group(2), m.group(3)
        if 'style:family="paragraph"' not in head:
            continue
        pp = re.search(r"<style:paragraph-properties([^>]*)", body)
        if not pp:
            continue
        top = re.search(r'fo:margin-top="([^"]+)"', pp.group(1))
        bot = re.search(r'fo:margin-bottom="([^"]+)"', pp.group(1))
        if top and top.group(1) in ("0.3335in", "0.3336in"):   # 480 twips
            return top.group(1), (bot.group(1) if bot else "-")
    return "-", "-"


# Phase two. Fifteen children, each named after a built-in style, over ONE custom parent
# declared last -- so nothing but the child's own name can supply the unstated margin.
BUILT_IN = ["heading 1", "heading 2", "heading 3", "heading 4", "heading 5", "heading 6",
            "heading 7", "heading 8", "heading 9", "Title", "Subtitle", "Caption",
            "Body Text", "List", "Quote"]


def sweep_styles(side: str) -> str:
    kids = "".join(
        f'<w:style w:type="paragraph" w:styleId="K{i}"><w:name w:val="{n}"/>'
        f'<w:basedOn w:val="ZPar"/>'
        f'<w:pPr><w:spacing w:{side}="{CHILD_BEFORE}"/></w:pPr></w:style>'
        for i, n in enumerate(BUILT_IN))
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}>
<w:docDefaults><w:rPrDefault><w:rPr><w:sz w:val="22"/></w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr/></w:pPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
{kids}
<w:style w:type="paragraph" w:styleId="ZPar"><w:name w:val="Zed Custom Par"/>
<w:basedOn w:val="Normal"/>
<w:pPr><w:spacing w:before="{PARENT_BEFORE}" w:after="{PARENT_AFTER}"/></w:pPr></w:style>
</w:styles>"""


def sweep_document() -> str:
    body = "".join(
        f'<w:p><w:pPr><w:pStyle w:val="K{i}"/></w:pPr><w:r><w:t>k{i}</w:t></w:r></w:p>'
        for i in range(len(BUILT_IN)))
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<w:document {NS}><w:body>{body}'
            f'<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
            f'<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
            f' w:header="0" w:footer="0" w:gutter="0"/></w:sectPr>'
            f'</w:body></w:document>')


def all_paragraph_margins(fodt: Path) -> dict[str, tuple[str | None, str | None]]:
    """Every paragraph style's `fo:margin-top` and `fo:margin-bottom`, by ODF style name."""
    text = fodt.read_text(encoding="utf8", errors="replace")
    found: dict[str, tuple[str | None, str | None]] = {}
    for m in re.finditer(r'<style:style style:name="([^"]+)"([^>]*)>(.*?)</style:style>',
                         text, re.S):
        if 'style:family="paragraph"' not in m.group(2):
            continue
        pp = re.search(r"<style:paragraph-properties([^>]*)", m.group(3))
        if not pp:
            continue
        top = re.search(r'fo:margin-top="([^"]+)"', pp.group(1))
        bot = re.search(r'fo:margin-bottom="([^"]+)"', pp.group(1))
        found[m.group(1)] = (top.group(1) if top else None, bot.group(1) if bot else None)
    return found


def twips(value: str | None) -> str:
    return "absent" if value is None else str(round(float(value.rstrip("in")) * 1440))


def sweep(out: Path) -> None:
    read: dict[str, dict[str, tuple[str | None, str | None]]] = {}
    for side in ("before", "after"):
        tag = f"sweep-only-{side}"
        docx = out / f"{tag}.docx"
        with zipfile.ZipFile(docx, "w", zipfile.ZIP_DEFLATED) as z:
            z.writestr("[Content_Types].xml", CONTENT_TYPES)
            z.writestr("_rels/.rels", ROOT_RELS)
            z.writestr("word/_rels/document.xml.rels", DOC_RELS)
            z.writestr("word/document.xml", sweep_document())
            z.writestr("word/styles.xml", sweep_styles(side))
        subprocess.run(
            ["soffice", f"-env:UserInstallation=file://{out / 'prof'}", "--headless",
             "--convert-to", "fodt", "--outdir", str(out), str(docx)],
            check=False, capture_output=True, timeout=600)
        read[side] = all_paragraph_margins(out / f"{tag}.fodt")

    print()
    print("child's own w:name, over one custom parent declared last")
    print(f"{'w:name':16}{'above':>10}{'below':>10}")
    for name in BUILT_IN:
        odf = name.replace(" ", "_20_")
        odf = odf[0].upper() + odf[1:]
        above = read["after"].get(odf, (None, None))[0]
        below = read["before"].get(odf, (None, None))[1]
        print(f"{name:16}{twips(above):>10}{twips(below):>10}")
    print()
    print("`absent` is a style LibreOffice renamed away from the built-in slot, which happens")
    print("only where it kept no margin at all; 480 is the control and must never appear.")


def main() -> int:
    out = Path(sys.argv[1]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    prof = out / "prof"

    rows = []
    for child_name in ("heading 4", "Custom Kid"):
        for parent_name in ("heading 2", "Custom Par"):
            for parent_first in (False, True):
                tag = (f"{child_name.replace(' ', '')}-{parent_name.replace(' ', '')}"
                       f"-{'parentfirst' if parent_first else 'childfirst'}")
                docx = out / f"{tag}.docx"
                author(docx, child_name, parent_name, parent_first)
                subprocess.run(
                    ["soffice", f"-env:UserInstallation=file://{prof}", "--headless",
                     "--convert-to", "fodt", "--outdir", str(out), str(docx)],
                    check=False, capture_output=True, timeout=300)
                top, bot = margins(out / f"{tag}.fodt")
                rows.append((tag, top, bot))

    print(f"{'variant':46} {'margin-top':>12} {'margin-bottom':>14}")
    for tag, top, bot in rows:
        print(f"{tag:46} {top:>12} {bot:>14}")
    print()
    print("0.3335in = 480tw   0.2085in = 300tw   0.25in = 360tw")
    print("0.0835in = 120tw   0.0417in =  60tw   0in = 0tw")

    sweep(out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
