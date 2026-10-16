#!/usr/bin/env python3
"""Does a row's `w:trHeight` floor include the row's borders, or sit under them?

    row-min-height-border.py /abs/scratch/dir [/abs/path/to/XRay.Office.Cli]

`TableLayouter` adds the border to the measured content and then raises the total to
`w:trHeight`, so a row whose content is shorter than its floor comes out exactly
`w:trHeight` tall. Two independent measurements say LibreOffice comes out *taller* than
that by what looks like one border:

  * `ESPN-R - MCF - Manual`, single-line rows with `w:trHeight w:val="488"` (24.41 pt):
    pitch 24.40 for us against 24.90 for the reference, and one pair at 25.25 against
    25.50. Rows whose content exceeds the floor match exactly.
  * `FAA 2025-26 Holdover Tables` page 20, whose content is identical on both sides:
    four row boundaries read 15.01 / 11.39 / 11.26 / 16.50 for us against
    15.56 / 11.85 / 11.81 / 16.75.

Both documents draw a `w:sz="4"` grid, which is half a point, and 0.50 pt is what the gap
measures — so "the floor sits under the borders" and "LibreOffice adds a constant half
point" both fit, and the two documents cannot separate them because they use the same
border.

This sweeps the border width instead. Five variants, identical but for `w:sz`, each a
six-row table of one short line per row under a fixed `w:trHeight`:

    w:sz    border      floor-under-borders predicts     constant predicts
      0     0 pt        24.00 pt                          24.50
      4     0.5 pt      24.50                             24.50
      8     1 pt        25.00                             24.50
     16     2 pt        26.00                             24.50
     24     3 pt        27.00                             24.50

and an `w:hRule="exact"` variant at `w:sz="16"`, because clipping is a separate branch in
both implementations and nothing has measured whether it carries the border too.

The observable is the mean gap between consecutive rows' baselines, read from the PDF
text layer rather than from ink, and the first and last rows are excluded so that the
table's own outer half-borders cannot contaminate it.
"""
from __future__ import annotations

import re
import subprocess
import sys
import zipfile
from pathlib import Path

NS = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
ROWS = 6
TR_HEIGHT = 480          # 24 pt, comfortably above one 10 pt line
CASES = [(0, "atLeast"), (4, "atLeast"), (8, "atLeast"), (16, "atLeast"),
         (24, "atLeast"), (16, "exact")]

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


def document(size: int, rule: str) -> str:
    if size:
        edge = "".join(
            f'<w:{s} w:val="single" w:sz="{size}" w:space="0" w:color="000000"/>'
            for s in ("top", "left", "bottom", "right", "insideH", "insideV"))
        borders = f"<w:tblBorders>{edge}</w:tblBorders>"
    else:
        borders = ('<w:tblBorders>'
                   + "".join(f'<w:{s} w:val="none" w:sz="0" w:space="0" w:color="auto"/>'
                             for s in ("top", "left", "bottom", "right",
                                       "insideH", "insideV"))
                   + '</w:tblBorders>')
    rows = "".join(
        f'<w:tr><w:trPr><w:trHeight w:val="{TR_HEIGHT}" w:hRule="{rule}"/></w:trPr>'
        f'<w:tc><w:tcPr><w:tcW w:w="4000" w:type="dxa"/>'
        f'<w:tcMar><w:top w:w="0" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/>'
        f'</w:tcMar></w:tcPr>'
        f'<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" '
        f'w:lineRule="auto"/></w:pPr>'
        f'<w:r><w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
        f'<w:sz w:val="20"/></w:rPr><w:t>R{i}</w:t></w:r></w:p></w:tc></w:tr>'
        for i in range(ROWS))
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<w:document {NS}><w:body>'
            f'<w:tbl><w:tblPr><w:tblW w:w="4000" w:type="dxa"/>{borders}'
            f'<w:tblCellMar><w:top w:w="0" w:type="dxa"/>'
            f'<w:bottom w:w="0" w:type="dxa"/></w:tblCellMar></w:tblPr>'
            f'<w:tblGrid><w:gridCol w:w="4000"/></w:tblGrid>{rows}</w:tbl>'
            f'<w:p/><w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
            f'<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
            f' w:header="0" w:footer="0" w:gutter="0"/></w:sectPr>'
            f'</w:body></w:document>')


def pitch(pdf: Path) -> float | None:
    """Mean baseline gap between the interior rows, in points."""
    if not pdf.exists():
        return None
    text = subprocess.run(["pdftotext", "-bbox", str(pdf), "-"],
                          capture_output=True, text=True).stdout
    ys = []
    for m in re.finditer(
            r'<word xMin="[\d.eE+-]+" yMin="([\d.eE+-]+)"[^>]*>R\d</word>', text):
        ys.append(float(m.group(1)))
    ys.sort()
    if len(ys) < 4:
        return None
    gaps = [ys[i + 1] - ys[i] for i in range(1, len(ys) - 2)]
    return sum(gaps) / len(gaps)


def main() -> int:
    out = Path(sys.argv[1]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    cli = sys.argv[2] if len(sys.argv) > 2 else None

    print(f"{'w:sz':>6}{'border pt':>11}{'rule':>10}"
          f"{'LibreOffice':>13}{'ours':>9}{'diff':>8}")
    for size, rule in CASES:
        tag = f"rowmin-sz{size}-{rule}"
        docx = out / f"{tag}.docx"
        with zipfile.ZipFile(docx, "w", zipfile.ZIP_DEFLATED) as z:
            z.writestr("[Content_Types].xml", CONTENT_TYPES)
            z.writestr("_rels/.rels", ROOT_RELS)
            z.writestr("word/document.xml", document(size, rule))

        ref_dir = out / f"ref-{tag}"
        ref_dir.mkdir(exist_ok=True)
        subprocess.run(
            ["soffice", f"-env:UserInstallation=file://{out / 'prof'}", "--headless",
             "--convert-to", "pdf", "--outdir", str(ref_dir), str(docx)],
            check=False, capture_output=True, timeout=300)
        ref = pitch(ref_dir / f"{tag}.pdf")

        ours = None
        if cli:
            our_dir = out / f"our-{tag}"
            our_dir.mkdir(exist_ok=True)
            subprocess.run([cli, "render", str(docx), "--format", "pdf",
                            "--outdir", str(our_dir)],
                           check=False, capture_output=True, timeout=300)
            ours = pitch(our_dir / f"{tag}.pdf")

        fmt = lambda v: "-" if v is None else f"{v:.2f}"          # noqa: E731
        diff = "-" if (ref is None or ours is None) else f"{ref - ours:+.2f}"
        print(f"{size:>6}{size / 8:>11.2f}{rule:>10}"
              f"{fmt(ref):>13}{fmt(ours):>9}{diff:>8}")

    print()
    print(f"w:trHeight = {TR_HEIGHT} twips = {TR_HEIGHT / 20:.2f} pt, and the single 10 pt")
    print("line in each row is far shorter, so every row sits on its floor.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
