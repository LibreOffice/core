"""Builds `numfmt.xlsx`: sixteen cells over the seven number-format codes that use `?` or `*`.

Hand-written OPC rather than a library, because none is installed and the parts needed for a
one-sheet workbook are five short XML files. The point of it is one question — what does
LibreOffice put where a `?` placeholder has no digit — and the two installed binaries answer it
differently, so the file is rendered through both and the glyphs read out of the PDFs rather than
out of `pdftotext`, which cannot tell U+2007 from U+0020 by eye.

Measured 2026-09-06, in `/home/user/wt-slidechart`, corpus `/home/user/sample-files`:
26.2.4.2 writes U+2007 for every unfilled `?` and 24.2.7.2 writes U+0020 for every one.
"""
import html
import subprocess
import sys
import zipfile
from pathlib import Path

CODES = [
    '_("$"* #,##0.00_);_("$"* \\(#,##0.00\\);_("$"* "-"??_);_(@_)',
    "??0",
    "# ??/??",
    "# ?/?",
    "0.??",
    "0 ?/?",
    "#,##0.00_);(#,##0.00)",
]
VALUES = {
    0: [0.0, 1234.5, -1234.5],
    1: [5.0, 50.0],
    2: [2.7, 1.25, 0.389],
    3: [0.25, 3.0],
    4: [1.5, 1.0],
    5: [2.25, 2.0],
    6: [0.0, 12.0],
}
BASE = 164
MAIN = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
REL = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
PKG = "http://schemas.openxmlformats.org/package/2006/relationships"


def build(target: Path) -> None:
    numfmts = "".join(
        f'<numFmt numFmtId="{BASE + i}" formatCode="{html.escape(c, quote=True)}"/>'
        for i, c in enumerate(CODES))
    xfs = "".join(
        f'<xf numFmtId="{BASE + i}" fontId="0" fillId="0" borderId="0" xfId="0"'
        f' applyNumberFormat="1"/>' for i in range(len(CODES)))
    styles = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f'<styleSheet xmlns="{MAIN}">'
        f'<numFmts count="{len(CODES)}">{numfmts}</numFmts>'
        f'<fonts count="1"><font><sz val="11"/><name val="Liberation Sans"/></font></fonts>'
        f'<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
        f'<borders count="1"><border/></borders>'
        f'<cellStyleXfs count="1">'
        f'<xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
        f'<cellXfs count="{len(CODES) + 1}">'
        f'<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>{xfs}</cellXfs>'
        f'<cellStyles count="1">'
        f'<cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles></styleSheet>')

    rows, at = [], 1
    for i in range(len(CODES)):
        for value in VALUES[i]:
            cells = (f'<c r="A{at}" t="inlineStr"><is><t>fmt{i}</t></is></c>'
                     f'<c r="B{at}" s="{i + 1}"><v>{value!r}</v></c>')
            rows.append(f'<row r="{at}">{cells}</row>')
            at += 1

    sheet = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             f'<worksheet xmlns="{MAIN}"><cols>'
             f'<col min="1" max="1" width="12" customWidth="1"/>'
             f'<col min="2" max="2" width="30" customWidth="1"/></cols>'
             f'<sheetData>{"".join(rows)}</sheetData></worksheet>')

    parts = {
        "[Content_Types].xml":
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
            'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
            '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-'
            'officedocument.spreadsheetml.sheet.main+xml"/>'
            '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.'
            'openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
            '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-'
            'officedocument.spreadsheetml.styles+xml"/></Types>',
        "_rels/.rels":
            f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="{PKG}">'
            f'<Relationship Id="rId1" Type="{REL}/officeDocument" Target="xl/workbook.xml"/>'
            f'</Relationships>',
        "xl/workbook.xml":
            f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<workbook xmlns="{MAIN}" xmlns:r="{REL}"><sheets>'
            f'<sheet name="Sheet1" sheetId="1" r:id="rId1"/></sheets></workbook>',
        "xl/_rels/workbook.xml.rels":
            f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="{PKG}">'
            f'<Relationship Id="rId1" Type="{REL}/worksheet" Target="worksheets/sheet1.xml"/>'
            f'<Relationship Id="rId2" Type="{REL}/styles" Target="styles.xml"/></Relationships>',
        "xl/styles.xml": styles,
        "xl/worksheets/sheet1.xml": sheet,
    }

    with zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as z:
        for name, body in parts.items():
            z.writestr(name, body)


def read(pdf: Path) -> None:
    """Print every drawn string with its non-ASCII characters spelled out."""
    import pymupdf

    page = pymupdf.open(pdf)[0]
    rows: dict[float, list[tuple[float, str]]] = {}
    for block in page.get_text("dict")["blocks"]:
        if block["type"] != 0:
            continue
        for line in block["lines"]:
            for span in line["spans"]:
                rows.setdefault(round(span["bbox"][1], 1), []).append(
                    (span["bbox"][0], span["text"]))
    for y in sorted(rows):
        text = " | ".join(t for _, t in sorted(rows[y]))
        print("   ", "".join(
            c if 32 <= ord(c) < 127 else f"<U+{ord(c):04X}>" for c in text))


if __name__ == "__main__":
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "numfmt.xlsx")
    build(out)
    print(f"wrote {out}")
    for binary in ("/opt/libreoffice26.2/program/soffice", "/usr/bin/soffice"):
        work = out.parent / ("out-" + Path(binary).parts[1])
        work.mkdir(parents=True, exist_ok=True)
        subprocess.run([binary, f"-env:UserInstallation=file://{work.absolute()}/profile",
                        "--headless", "--norestore", "--convert-to", "pdf",
                        "--outdir", str(work), str(out)], capture_output=True, timeout=300)
        rendered = work / (out.stem + ".pdf")
        if not rendered.exists():
            print(f"{binary}: RENDER FAILED")
            continue
        print(binary)
        read(rendered)
