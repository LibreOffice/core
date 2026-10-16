#!/usr/bin/env python3
"""What exactly promotes a `continuous` section break to a page break in 26.2.4.2?

Group B established that `continuous` with an identical page setup does not break, and
group A that `continuous` across an orientation change does. This narrows *which* property
of the page setup is compared: size only, margins too, or the whole setup.

Every variant fills section 1 with 47 paragraphs — one line past a full page — so a
promoted break shows as P3 and a genuine continuation as P2.
"""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import sweep as S  # noqa: E402
from mkdocx import build  # noqa: E402

P = '<w:pgSz w:w="12240" w:h="15840"/>'
P_WIDE = '<w:pgSz w:w="12960" w:h="15840"/>'          # +0.5in wide, still portrait
P_TALL = '<w:pgSz w:w="12240" w:h="16560"/>'          # +0.5in tall
P_ORIENT_ONLY = '<w:pgSz w:w="12240" w:h="15840" w:orient="portrait"/>'  # same numbers, attribute added
L = '<w:pgSz w:w="15840" w:h="12240" w:orient="landscape"/>'
L_NOATTR = '<w:pgSz w:w="15840" w:h="12240"/>'        # swapped numbers, no w:orient
P_1TWIP = '<w:pgSz w:w="12241" w:h="15840"/>'         # one twip wider

MARG_A = '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>'
MARG_B = '<w:pgMar w:top="2160" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>'

VARIANTS = [
    ("continuous, identical setup",            P, P,             None),
    ("continuous, width +720tw",               P, P_WIDE,        None),
    ("continuous, height +720tw",              P, P_TALL,        None),
    ("continuous, width +1 twip",              P, P_1TWIP,       None),
    ("continuous, w:orient attr added only",   P, P_ORIENT_ONLY, None),
    ("continuous, landscape via w:orient",     P, L,             None),
    ("continuous, landscape by swapped w/h",   P, L_NOATTR,      None),
    ("continuous, top margin +720tw",          P, P,             MARG_B),
]

if __name__ == "__main__":
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    S.OUT = out
    for i, (label, g1, g2, marg) in enumerate(VARIANTS):
        d = out / f"c{i}.docx"
        import mkdocx
        old = mkdocx.MARGINS
        # margin change applies to the SECOND section only
        if marg:
            # build section 1 with MARG_A and section 2 with MARG_B by monkeypatching
            # around the two sectpr emissions is fragile; instead write it directly.
            body = []
            for j in range(47):
                if j == 46:
                    body.append(f'<w:p><w:pPr><w:sectPr>{g1}{MARG_A}</w:sectPr></w:pPr>'
                                f'<w:r><w:t>A{j + 1}</w:t></w:r></w:p>')
                else:
                    body.append(f"<w:p><w:r><w:t>A{j + 1}</w:t></w:r></w:p>")
            for j in range(3):
                body.append(f"<w:p><w:r><w:t>B{j + 1}</w:t></w:r></w:p>")
            doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
                   f'<w:body>{"".join(body)}'
                   f'<w:sectPr><w:type w:val="continuous"/>{g2}{marg}</w:sectPr></w:body></w:document>')
            import zipfile
            with zipfile.ZipFile(d, "w", zipfile.ZIP_DEFLATED) as z:
                z.writestr("[Content_Types].xml", mkdocx.CT)
                z.writestr("_rels/.rels", mkdocx.RELS)
                z.writestr("word/_rels/document.xml.rels", mkdocx.DRELS)
                z.writestr("word/styles.xml", mkdocx.STYLES)
                z.writestr("word/document.xml", doc)
        else:
            build(d, n_first=47, brk="continuous", m_second=3,
                  first_geom=g1, second_geom=g2)
        pdf = S.render(d)
        print(f"{label}\t{S.shapes(pdf) if pdf.exists() else 'RENDER-FAILED'}")
