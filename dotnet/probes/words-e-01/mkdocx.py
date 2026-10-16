#!/usr/bin/env python3
"""Author a minimal two-section DOCX: N portrait paragraphs, a section break, M landscape ones.

Nothing else varies. Every part is written literally so the only difference between two
documents in a sweep is the one attribute the sweep names.
"""
from __future__ import annotations

import zipfile
from pathlib import Path

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DRELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>"""

# One 12 pt Liberation Serif default, no spacing, so the lines-per-page arithmetic is clean.
STYLES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif" w:cs="Liberation Serif"/>
<w:sz w:val="24"/><w:szCs w:val="24"/></w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr></w:pPrDefault>
</w:docDefaults></w:styles>"""

PORTRAIT = '<w:pgSz w:w="12240" w:h="15840"/>'
LANDSCAPE = '<w:pgSz w:w="15840" w:h="12240" w:orient="landscape"/>'
MARGINS = '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>'


def para(text: str) -> str:
    return f"<w:p><w:r><w:t>{text}</w:t></w:r></w:p>"


def sectpr(kind: str, geom: str, extra: str = "") -> str:
    t = "" if kind is None else f'<w:type w:val="{kind}"/>'
    return f"<w:sectPr>{t}{geom}{MARGINS}{extra}</w:sectPr>"


def build(path: Path, n_first: int, brk: str | None, m_second: int,
          first_geom: str = PORTRAIT, second_geom: str = LANDSCAPE,
          first_kind: str | None = None) -> None:
    """Section 1: n_first paragraphs, ended by a sectPr with first_geom.
    Section 2 (the last, body-level sectPr) starts with break type `brk` and second_geom."""
    body = []
    for i in range(n_first):
        if i == n_first - 1:
            body.append(f"<w:p><w:pPr>{sectpr(first_kind, first_geom)}</w:pPr>"
                        f"<w:r><w:t>A{i + 1}</w:t></w:r></w:p>")
        else:
            body.append(para(f"A{i + 1}"))
    for i in range(m_second):
        body.append(para(f"B{i + 1}"))
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
           f"<w:body>{''.join(body)}{sectpr(brk, second_geom)}</w:body></w:document>")
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DRELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", doc)
