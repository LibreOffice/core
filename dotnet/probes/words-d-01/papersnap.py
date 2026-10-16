#!/usr/bin/env python3
"""Where does LibreOffice 26.2.4.2 snap a stated page dimension to a standard one?

Authored, minimal DOCX: one paragraph, one section, `w:pgSz` swept. The PDF's MediaBox is
read back — that *is* the sheet the reference decided on, with no inference in between.

The C++ tree in this checkout is 27.2.0.0.alpha0+ and says
`DomainMapper.cxx`:827/836 pass every `w:pgSz` dimension through
`PaperInfo::sloppyFitPageDimension`, whose window is `MAXSLOPPY = PT2MM100(1.25)`. That is a
description of a *future* binary. This probe measures the installed one.

    papersnap.py <outdir> [coarse|fine|cross]
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

NS = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""


def build(path: Path, w: int, h: int) -> None:
    body = ('<w:p><w:r><w:t>MARKER</w:t></w:r></w:p>'
            f'<w:sectPr><w:pgSz w:w="{w}" w:h="{h}"/>'
            '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720"'
            ' w:header="0" w:footer="0"/></w:sectPr>')
    doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
           f'<w:document {NS}><w:body>{body}</w:body></w:document>')
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/document.xml", doc)


def render(sources, outdir: Path, workers: int = 8):
    outdir.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")

    def one(job):
        i, src = job
        subprocess.run(["soffice", "--headless", "--norestore",
                        f"-env:UserInstallation=file://{outdir}/prof{i % workers}",
                        "--convert-to", "pdf", "--outdir", str(outdir), str(src)],
                       capture_output=True, timeout=600, env=env)
        return outdir / (src.stem + ".pdf")

    with ThreadPoolExecutor(max_workers=workers) as pool:
        return list(pool.map(one, list(enumerate(sources))))


def box(pdf: Path):
    out = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True).stdout
    m = re.search(r"^Page size:\s+([\d.]+) x ([\d.]+)", out, re.M)
    return (float(m.group(1)), float(m.group(2))) if m else None


def sweep(outdir: Path, pairs, label: str):
    srcs = []
    for w, h in pairs:
        p = outdir / f"{label}-{w}x{h}.docx"
        build(p, w, h)
        srcs.append(p)
    pdfs = render(srcs, outdir)
    print(f"# {label}: stated twips -> emitted MediaBox pt (and the pt the twips alone give)")
    print("w_tw\th_tw\tw_pt_stated\th_pt_stated\tw_pt_pdf\th_pt_pdf\tsnapped")
    for (w, h), pdf in zip(pairs, pdfs):
        b = box(pdf)
        if b is None:
            print(f"{w}\t{h}\t-\t-\tFAILED")
            continue
        ws, hs = w / 20, h / 20
        snap = []
        if abs(b[0] - ws) > 0.02:
            snap.append("w")
        if abs(b[1] - hs) > 0.02:
            snap.append("h")
        print(f"{w}\t{h}\t{ws:.2f}\t{hs:.2f}\t{b[0]:.3f}\t{b[1]:.3f}\t{''.join(snap) or '-'}")


def main() -> int:
    outdir = Path(sys.argv[1])
    which = sys.argv[2] if len(sys.argv) > 2 else "coarse"
    outdir.mkdir(parents=True, exist_ok=True)
    if which == "coarse":
        # A4 is 11905.5 x 16837.8 twips. Walk the height past both edges of any window.
        sweep(outdir, [(11906, h) for h in range(16780, 16901, 10)], "h-coarse")
    elif which == "fine":
        sweep(outdir, [(11906, h) for h in range(16805, 16871)], "h-fine")
    elif which == "widefine":
        sweep(outdir, [(w, 16838) for w in range(11875, 11941)], "w-fine")
    elif which == "cross":
        # A width near 297 mm is not any paper's *width*; it is A4's and A3's *height*. If the
        # rule pools every dimension in the table rather than matching whole formats, this snaps.
        sweep(outdir, [(16840, 23820), (16830, 23830), (12240, 15845), (12245, 15840),
                       (11906, 16838), (11900, 16830), (11840, 16780)], "cross")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
