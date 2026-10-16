#!/usr/bin/env python3
"""Is the empty last paragraph of a section *dropped*, or merely not allowed to break a page?

46 body paragraphs is exactly one full Letter page at 12 pt single-spaced (established in
sweep.py group A: 46 -> P1, 47 -> P2).  So put 46 text paragraphs in section 1 and vary only
what the 47th — the one carrying the `w:sectPr` — contains.  Dropped: 1 portrait page.
Laid out: 2.
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


def build(path: Path, n: int, last: str, trailing: str = "") -> None:
    body = "".join(f"<w:p><w:r><w:t>A{i + 1}</w:t></w:r></w:p>" for i in range(n))
    body += f'<w:p><w:pPr><w:sectPr>{P}{MARG}</w:sectPr></w:pPr>{last}</w:p>'
    body += trailing
    body += "".join(f"<w:p><w:r><w:t>B{i}</w:t></w:r></w:p>" for i in (1, 2, 3))
    body += f'<w:sectPr><w:type w:val="nextPage"/>{L}{MARG}</w:sectPr>'
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
           f"<w:body>{body}</w:body></w:document>")
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", mkdocx.CT)
        z.writestr("_rels/.rels", mkdocx.RELS)
        z.writestr("word/_rels/document.xml.rels", mkdocx.DRELS)
        z.writestr("word/styles.xml", mkdocx.STYLES)
        z.writestr("word/document.xml", doc)


VARIANTS = [
    ("45 text + empty sectPr para",          45, ""),
    ("45 text + text sectPr para",           45, "<w:r><w:t>Z</w:t></w:r>"),
    ("46 text + empty sectPr para",          46, ""),
    ("46 text + text sectPr para",           46, "<w:r><w:t>Z</w:t></w:r>"),
    ("46 text + sectPr para holding an empty run", 46, "<w:r></w:r>"),
    ("46 text + sectPr para holding an empty w:t", 46, "<w:r><w:t></w:t></w:r>"),
    ("46 text + sectPr para holding a space",     46, "<w:r><w:t xml:space='preserve'> </w:t></w:r>"),
    ("46 text + sectPr para holding a bookmark",  46,
     '<w:bookmarkStart w:id="1" w:name="bm"/><w:bookmarkEnd w:id="1"/>'),
]

if __name__ == "__main__":
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    S.OUT = out
    for i, (label, n, last) in enumerate(VARIANTS):
        d = out / f"f{i}.docx"
        build(d, n, last)
        pdf = S.render(d)
        print(f"f{i}\t{label}\t{S.shapes(pdf) if pdf.exists() else 'RENDER-FAILED'}")
