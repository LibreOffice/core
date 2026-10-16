#!/usr/bin/env python3
"""A *direct* `w:spacing` that states only one side — what happens to the other?

    direct-one-sided-spacing.py /abs/scratch/dir

`words-pagination-01` settled the question for a one-sided `w:spacing` on a **style**:
the unstated half is filled from Writer's pool row for the style's own name when the
style is one of Writer's headings, and from the parent's name otherwise. This asks the
same question one level down, where the answer turns out to be completely different.

The case that raises it is `FAA 2025-26 Holdover Tables.docx`. Its `NOTES` headings are
`Heading4`, based on a custom `Notes/Cautions Heading` that states
`w:before="180" w:after="60"`; `Heading4` itself states only `w:before="120"`. **31 of
the 113 NOTES headings additionally carry a direct `<w:spacing w:before="80"/>` on the
paragraph**, and those 31 are exactly the 31 pages where LibreOffice puts the first note
**3.00 pt** — 60 twips, the inherited `w:after` to the twip — closer to the heading than
we do. The space *above* the heading agrees on all 31, so the direct `w:before` is read
correctly and only the unstated `w:after` diverges.

So the corpus says: a direct one-sided `w:spacing` zeroes the side it does not state.
The corpus cannot say whether that is a property of `w:spacing` as a whole, of the
direct-formatting path only, or of `w:after` in particular — and it cannot rule out the
much duller explanation that those 31 paragraphs differ some other way. These variants
can. Every one holds the style chain fixed and varies only the paragraph.

The observable is `fo:margin-top` / `fo:margin-bottom` on the **automatic** style the
paragraph gets in `soffice --convert-to fodt` output — the importer's own answer, read
before any layout, font or rounding beyond the 1/100 mm the format stores. 180 twips is
0.125in, 120 is 0.0835in, 80 is 0.0555in, 60 is 0.0417in and 0 is 0in: no two of them are
confusable.
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

# The document's own chain, reproduced: a custom base stating both sides, a built-in
# `heading 4` on top of it stating only `before`. Everything the variants change is on
# the paragraph.
#
# **The declaration order is the whole experiment and getting it wrong makes the probe
# unable to answer.** `words-pagination-01` established that a one-sided `w:spacing` on a
# style resolves its unstated half from Writer's pool row for the style's own name *only*
# when the parent is declared after the child; declared before, ordinary inheritance
# answers instead. In `FAA 2025-26 Holdover Tables.docx` `Heading4` is style 4 and
# `Notes/Cautions Heading` is style 186 — child first — so the style resolves to 120
# below and the direct-formatted paragraphs resolve to 60. Run with the parent first,
# both readings are 60 and every variant agrees whatever the rule is. That version of
# this probe was written, run, and threw away its own question.
def styles(parent_first: bool) -> str:
    par = ('<w:style w:type="paragraph" w:styleId="NotesCautionsHeading">'
           '<w:name w:val="Notes/Cautions Heading"/><w:basedOn w:val="Normal"/>'
           '<w:pPr><w:spacing w:before="180" w:after="60"/></w:pPr></w:style>')
    kid = ('<w:style w:type="paragraph" w:styleId="Heading4"><w:name w:val="heading 4"/>'
           '<w:basedOn w:val="NotesCautionsHeading"/>'
           '<w:pPr><w:spacing w:before="120"/><w:outlineLvl w:val="3"/></w:pPr></w:style>')
    chain = (par + kid) if parent_first else (kid + par)
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}>
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Liberation Serif"
 w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr/></w:pPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
</w:style>
{chain}
<w:style w:type="paragraph" w:styleId="Plain"><w:name w:val="Plain Base"/>
<w:basedOn w:val="Normal"/>
<w:pPr><w:spacing w:before="180" w:after="60"/></w:pPr></w:style>
</w:styles>"""


def document(style: str, ppr_extra: str) -> str:
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:p><w:r><w:t>before</w:t></w:r></w:p>
<w:p><w:pPr><w:pStyle w:val="{style}"/>{ppr_extra}</w:pPr><w:r><w:t>PROBE</w:t></w:r></w:p>
<w:p><w:r><w:t>after</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"
 w:header="0" w:footer="0" w:gutter="0"/></w:sectPr>
</w:body></w:document>"""


# name -> (style the paragraph names, extra pPr on the paragraph itself)
#
# `style-only` is the control that must reproduce `words-pagination-01`'s answer: with
# nothing on the paragraph, `Heading4`'s own one-sided `w:before` is resolved by the rule
# that round established. Everything else is measured against it.
VARIANTS = [
    ("style-only",            "Heading4", ""),
    ("direct-before-only",    "Heading4", '<w:spacing w:before="80"/>'),
    ("direct-after-only",     "Heading4", '<w:spacing w:after="80"/>'),
    ("direct-both",           "Heading4", '<w:spacing w:before="80" w:after="40"/>'),
    ("direct-neither",        "Heading4", '<w:spacing w:line="240" w:lineRule="auto"/>'),
    ("direct-before-zero",    "Heading4", '<w:spacing w:before="0"/>'),
    # The same five on a style with no built-in name and both sides stated, so that the
    # style-level one-sided rule cannot be what is being measured.
    ("plain-style-only",      "Plain",    ""),
    ("plain-before-only",     "Plain",    '<w:spacing w:before="80"/>'),
    ("plain-after-only",      "Plain",    '<w:spacing w:after="80"/>'),
    ("plain-both",            "Plain",    '<w:spacing w:before="80" w:after="40"/>'),
    ("plain-neither",         "Plain",    '<w:spacing w:line="240" w:lineRule="auto"/>'),
    # And the line-spacing question on its own: does a direct `w:spacing` that states only
    # `w:line` disturb either margin?
    ("plain-line-only",       "Plain",    '<w:spacing w:line="360" w:lineRule="auto"/>'),
    # tdf#118521's condition is three-way — `bTopSet != bBottomSet || bBottomSet != bContextSet`
    # — so a paragraph stating only `w:contextualSpacing` should fill *both* margins from the
    # DOCX chain even though it names no `w:spacing` at all. These decide whether that arm has
    # to be implemented or is a distinction without a difference on this style chain.
    ("ctx-only",              "Heading4", '<w:contextualSpacing/>'),
    ("ctx-and-before",        "Heading4", '<w:contextualSpacing/><w:spacing w:before="80"/>'),
    ("ctx-and-both",          "Heading4",
     '<w:contextualSpacing/><w:spacing w:before="80" w:after="40"/>'),
]


def author(path: Path, style: str, extra: str, parent_first: bool) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/document.xml", document(style, extra))
        z.writestr("word/styles.xml", styles(parent_first))


IN = 1440.0  # twips per inch, for reading the ODF lengths back as twips


def as_twips(v: str) -> str:
    m = re.fullmatch(r'([-\d.]+)(in|cm|mm|pt)', v or '')
    if not m:
        return v or '-'
    n = float(m.group(1))
    unit = m.group(2)
    tw = {'in': n * IN, 'cm': n / 2.54 * IN, 'mm': n / 25.4 * IN, 'pt': n * 20}[unit]
    return f"{tw:.0f}"


def measure(fodt: Path) -> tuple[str, str]:
    """The margins in force on the paragraph that says PROBE.

    Read off the *automatic* style the paragraph names, falling back to the named style
    it is based on for whichever side the automatic style does not restate — which is
    exactly how ODF layers them.
    """
    text = fodt.read_text(encoding="utf8", errors="replace")
    # `<text:h>` and not only `<text:p>`: a style carrying `w:outlineLvl` imports as a
    # heading, and matching only the paragraph form silently returns "?" for exactly the
    # variants this probe exists to measure.
    m = re.search(r'<text:(?:p|h) [^>]*text:style-name="([^"]+)"[^>]*>PROBE</text:(?:p|h)>',
                  text)
    if not m:
        return ('?', '?')
    want = m.group(1)
    styles = {}
    for s in re.finditer(
            r'<style:style style:name="([^"]+)"([^>]*)>(.*?)</style:style>', text, re.S):
        styles[s.group(1)] = (s.group(2), s.group(3))
    top = bot = None
    seen = set()
    while want and want in styles and want not in seen:
        seen.add(want)
        head, body = styles[want]
        if top is None:
            t = re.search(r'fo:margin-top="([^"]*)"', body)
            top = t.group(1) if t else None
        if bot is None:
            b = re.search(r'fo:margin-bottom="([^"]*)"', body)
            bot = b.group(1) if b else None
        p = re.search(r'style:parent-style-name="([^"]+)"', head)
        want = p.group(1) if p else None
    return (as_twips(top), as_twips(bot))


def fixture(path: Path) -> None:
    """Write the test fixture: the chain, child declared first, and nothing else.

    Only `word/styles.xml` is read by `DirectOneSidedSpacingTests`, which builds each variant's
    `w:pPr` itself — so the fixture is the style chain and the declaration order, which are the
    two things a hand-written test would get wrong.
    """
    author(path, "Heading4", "", parent_first=False)
    print('wrote', path)


def main() -> int:
    if len(sys.argv) > 2 and sys.argv[1] == '--fixture':
        fixture(Path(sys.argv[2]).resolve())
        return 0

    out = Path(sys.argv[1] if len(sys.argv) > 1 else '.').resolve()
    out.mkdir(parents=True, exist_ok=True)
    prof = out / 'prof'
    print(f"{'variant':22s} {'child declared first':>22s}   {'parent declared first':>22s}")
    print(f"{'':22s} {'top':>10} {'bottom':>11}   {'top':>10} {'bottom':>11}")
    print('-' * 78)
    for name, style, extra in VARIANTS:
        got = []
        for parent_first in (False, True):
            tag = f'{name}-{"pf" if parent_first else "cf"}'
            docx = out / f'{tag}.docx'
            author(docx, style, extra, parent_first)
            subprocess.run(
                ['soffice', f'-env:UserInstallation=file://{prof}', '--headless',
                 '--convert-to', 'fodt', '--outdir', str(out), str(docx)],
                capture_output=True, timeout=180)
            fodt = out / f'{tag}.fodt'
            got.append(measure(fodt) if fodt.exists() else ('-', '-'))
        (t1, b1), (t2, b2) = got
        print(f"{name:22s} {t1:>10} {b1:>11}   {t2:>10} {b2:>11}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
