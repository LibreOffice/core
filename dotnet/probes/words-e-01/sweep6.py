#!/usr/bin/env python3
"""The remaining guards in LibreOffice's own removal test, measured rather than read.

`DomainMapper.cxx`:4852's `bRemove` protects a section-mark paragraph that carries a
*column* break, a field, a framed predecessor or an anchored object — and conspicuously
does **not** protect one that carries a page break.  Two of those are reachable from a
minimal DOCX and are measured here; the page-break arm is measured by two further routes
(`w:pageBreakBefore` on the mark itself, and a mark on an already-full page).
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
M = mkdocx.MARGINS
BRP = '<w:r><w:br w:type="page"/></w:r>'
BRC = '<w:r><w:br w:type="column"/></w:r>'
FIELD = ('<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
         '<w:r><w:instrText xml:space="preserve"> PAGE </w:instrText></w:r>'
         '<w:r><w:fldChar w:fldCharType="end"/></w:r>')


def build(path: Path, n: int, tail: str) -> None:
    body = "".join(f"<w:p><w:r><w:t>A{i + 1}</w:t></w:r></w:p>" for i in range(n))
    body += tail
    body += "".join(f"<w:p><w:r><w:t>B{i}</w:t></w:r></w:p>" for i in (1, 2, 3))
    body += f'<w:sectPr><w:type w:val="nextPage"/>{L}{M}</w:sectPr>'
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
           f"<w:body>{body}</w:body></w:document>")
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", mkdocx.CT)
        z.writestr("_rels/.rels", mkdocx.RELS)
        z.writestr("word/_rels/document.xml.rels", mkdocx.DRELS)
        z.writestr("word/styles.xml", mkdocx.STYLES)
        z.writestr("word/document.xml", doc)


def mark(inner: str = "", ppr_extra: str = "") -> str:
    return f'<w:p><w:pPr>{ppr_extra}<w:sectPr>{P}{M}</w:sectPr></w:pPr>{inner}</w:p>'


VARIANTS = [
    ("3 paras, mark with w:pageBreakBefore",     3, mark(ppr_extra="<w:pageBreakBefore/>")),
    ("3 paras, page-break para + empty mark",    3, f"<w:p>{BRP}</w:p>" + mark()),
    ("3 paras, column-break para + empty mark",  3, f"<w:p>{BRC}</w:p>" + mark()),
    ("3 paras, mark holding a column break",     3, mark(BRC)),
    ("3 paras, mark holding a PAGE field",       3, mark(FIELD)),
    ("46 paras (page full), empty mark",        46, mark()),
    ("46 paras (page full), page-break para + empty mark", 46, f"<w:p>{BRP}</w:p>" + mark()),
    ("46 paras (page full), mark with w:pageBreakBefore", 46,
     mark(ppr_extra="<w:pageBreakBefore/>")),
]

if __name__ == "__main__":
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    S.OUT = out
    for i, (label, n, tail) in enumerate(VARIANTS):
        d = out / f"g{i}.docx"
        build(d, n, tail)
        pdf = S.render(d)
        print(f"g{i}\t{label}\t{S.shapes(pdf) if pdf.exists() else 'RENDER-FAILED'}")
