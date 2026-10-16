#!/usr/bin/env python3
"""Sweep section-break types and first-section fill against the installed 26.2.4.2.

Emits, per variant, the page count and the per-page orientation sequence read back from
the PDF's own media boxes — the renderer's answer, not a parse of the input.
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from mkdocx import build, PORTRAIT, LANDSCAPE  # noqa: E402

OUT = Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/we-sweep")
OUT.mkdir(parents=True, exist_ok=True)
ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")


def render(docx: Path) -> Path:
    prof = OUT / "prof"
    subprocess.run(["soffice", "--headless", "--norestore",
                    f"-env:UserInstallation=file://{prof}",
                    "--convert-to", "pdf", "--outdir", str(OUT), str(docx)],
                   env=ENV, capture_output=True, timeout=180)
    return docx.with_suffix(".pdf")


def shapes(pdf: Path) -> str:
    out = subprocess.run(["pdfinfo", "-l", "100000", str(pdf)],
                         capture_output=True, text=True).stdout
    seq = []
    for line in out.splitlines():
        m = re.match(r"Page\s+\d+ size:\s+([\d.]+) x ([\d.]+)", line)
        if m:
            w, h = float(m.group(1)), float(m.group(2))
            seq.append("L" if w > h else "P")
    return rle("".join(seq))


def rle(s: str) -> str:
    if not s:
        return "(empty)"
    out, cur, n = [], s[0], 1
    for c in s[1:]:
        if c == cur:
            n += 1
        else:
            out.append(f"{cur}{n}")
            cur, n = c, 1
    out.append(f"{cur}{n}")
    return "".join(out)


def run(name: str, **kw) -> str:
    d = OUT / f"{name}.docx"
    build(d, **kw)
    p = render(d)
    if not p.exists():
        return "RENDER-FAILED"
    return shapes(p)


if __name__ == "__main__":
    rows = []
    # A. break type × first-section fill, portrait -> landscape.
    #    46 paragraphs is one full portrait page at 12 pt / single spacing on Letter.
    for fill, label in [(1, "1 para"), (46, "1 full page"), (47, "1 page + 1 line"),
                        (92, "2 full pages"), (93, "2 pages + 1 line")]:
        for brk in ["nextPage", "continuous", "evenPage", "oddPage", None]:
            k = brk or "absent"
            rows.append((f"A/{label}/{k}", run(f"a_{fill}_{k}", n_first=fill, brk=brk, m_second=3)))
    # B. the same break types with NO geometry change at all (portrait -> portrait).
    for fill in [46, 47]:
        for brk in ["nextPage", "continuous", "evenPage", "oddPage"]:
            rows.append((f"B/{fill}/{brk}/no-geom-change",
                         run(f"b_{fill}_{brk}", n_first=fill, brk=brk, m_second=3,
                             second_geom=PORTRAIT)))
    for label, seq in rows:
        print(f"{label}\t{seq}")
