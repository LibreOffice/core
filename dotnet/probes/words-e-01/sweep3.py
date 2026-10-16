#!/usr/bin/env python3
"""Is the `continuous` promotion driven by the w:orient *flag* or by computed orientation?

Section 1 is landscape in every variant here, so the previous round's "landscape wins" and
"the flag differs" hypotheses predict opposite answers.  47 paragraphs again: a promoted
break shows as an extra page with a new shape, a continuation as the same shape throughout.
"""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import sweep as S  # noqa: E402
from mkdocx import build  # noqa: E402

P_PLAIN = '<w:pgSz w:w="12240" w:h="15840"/>'
P_ATTR = '<w:pgSz w:w="12240" w:h="15840" w:orient="portrait"/>'
L_ATTR = '<w:pgSz w:w="15840" w:h="12240" w:orient="landscape"/>'
L_PLAIN = '<w:pgSz w:w="15840" w:h="12240"/>'

VARIANTS = [
    ("L(attr) -> continuous P(plain)",  L_ATTR,  P_PLAIN),
    ("L(attr) -> continuous P(attr)",   L_ATTR,  P_ATTR),
    ("L(attr) -> continuous L(attr)",   L_ATTR,  L_ATTR),
    ("L(plain)-> continuous P(plain)",  L_PLAIN, P_PLAIN),
    ("L(plain)-> continuous L(attr)",   L_PLAIN, L_ATTR),
    ("P(attr) -> continuous P(attr)",   P_ATTR,  P_ATTR),
]

if __name__ == "__main__":
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    S.OUT = out
    for i, (label, g1, g2) in enumerate(VARIANTS):
        d = out / f"d{i}.docx"
        build(d, n_first=47, brk="continuous", m_second=3, first_geom=g1, second_geom=g2)
        pdf = S.render(d)
        print(f"{label}\t{S.shapes(pdf) if pdf.exists() else 'RENDER-FAILED'}")
