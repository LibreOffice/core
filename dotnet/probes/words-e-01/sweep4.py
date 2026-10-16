#!/usr/bin/env python3
"""A page break at the tail of a section, immediately before a nextPage section break.

`1_tpr_template__from_fy14_.docx` ends its first section with, in order: two empty
paragraphs, a paragraph whose only run is `<w:br w:type="page"/>`, and an empty paragraph
carrying the `w:sectPr`.  The reference emits no page for any of that; we emit one holding
only the footer.  This sweep varies one thing at a time around that shape.

Section 1 is deliberately short (3 paragraphs), so section 1 occupies page 1 and any page
the tail produces is unambiguous: 2 pages means the tail was absorbed, 3 means it was not.
"""
from __future__ import annotations

import sys
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mkdocx  # noqa: E402
import sweep as S  # noqa: E402

P = '<w:pgSz w:w="12240" w:h="15840"/>'
L = '<w:pgSz w:w="15840" w:h="12240" w:orient="landscape"/>'
MARG = mkdocx.MARGINS
BR = '<w:r><w:br w:type="page"/></w:r>'


def doc(tail: str, sect2_kind: str = "nextPage", geom2: str = L) -> str:
    head = "".join(f"<w:p><w:r><w:t>A{i}</w:t></w:r></w:p>" for i in (1, 2, 3))
    body = (head + tail
            + "".join(f"<w:p><w:r><w:t>B{i}</w:t></w:r></w:p>" for i in (1, 2, 3))
            + f'<w:sectPr><w:type w:val="{sect2_kind}"/>{geom2}{MARG}</w:sectPr>')
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
            f"<w:body>{body}</w:body></w:document>")


def sect1(inner: str = "") -> str:
    return f'<w:p><w:pPr><w:sectPr>{P}{MARG}</w:sectPr></w:pPr>{inner}</w:p>'


VARIANTS = [
    # label,                                          tail of section 1
    ("empty para carries sectPr, no page break",      sect1()),
    ("text para carries sectPr, no page break",       sect1("<w:r><w:t>Z</w:t></w:r>")),
    ("page-break para, then empty sectPr para",       f"<w:p>{BR}</w:p>" + sect1()),
    ("page-break para, then TEXT sectPr para",        f"<w:p>{BR}</w:p>" + sect1("<w:r><w:t>Z</w:t></w:r>")),
    ("page break inside the empty sectPr para",       sect1(BR)),
    ("page-break para, then empty, then sectPr para", f"<w:p>{BR}</w:p><w:p/>" + sect1()),
    ("two page-break paras, then empty sectPr para",  f"<w:p>{BR}</w:p><w:p>{BR}</w:p>" + sect1()),
    ("page-break para, sectPr para, sect2 continuous",
     f"<w:p>{BR}</w:p>" + sect1()),                       # kind overridden below
    ("page-break para, then empty sectPr para, sect2 PORTRAIT",
     f"<w:p>{BR}</w:p>" + sect1()),                       # geom overridden below
]

if __name__ == "__main__":
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    S.OUT = out
    for i, (label, tail) in enumerate(VARIANTS):
        kind = "continuous" if "continuous" in label else "nextPage"
        geom = P if "PORTRAIT" in label else L
        d = out / f"e{i}.docx"
        with zipfile.ZipFile(d, "w", zipfile.ZIP_DEFLATED) as z:
            z.writestr("[Content_Types].xml", mkdocx.CT)
            z.writestr("_rels/.rels", mkdocx.RELS)
            z.writestr("word/_rels/document.xml.rels", mkdocx.DRELS)
            z.writestr("word/styles.xml", mkdocx.STYLES)
            z.writestr("word/document.xml", doc(tail, kind, geom))
        pdf = S.render(d)
        print(f"e{i}\t{label}\t{S.shapes(pdf) if pdf.exists() else 'RENDER-FAILED'}")
