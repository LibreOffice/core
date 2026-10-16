"""Builds `codes.xlsx`: one cell per number-format question this round has to settle.

Hand-written OPC, following `probes/chart-cat-reverse/make-numfmt.py`, because no library is
installed and a one-sheet workbook is five short XML files.

Three questions, all of them read off the rendered page rather than guessed from a table:

* what a *built-in* number-format id with no `<numFmt>` of its own prints — ids 14, 18-22 and
  37-44, which no file spells out and every file uses;
* what the hour placeholder does at one letter and at two, inside and outside `[ ]`;
* what `aaa` / `aaaa` print, which is the East Asian day-name code, against `ddd` / `dddd`.

Rendered through both installed binaries so a difference between them is visible rather than
inherited.
"""
import html
import subprocess
import sys
import zipfile
from pathlib import Path

MAIN = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
REL = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
PKG = "http://schemas.openxmlformats.org/package/2006/relationships"

# A serial that is a Sunday at 02:20:00 — 2022-08-21 is a Sunday, so the day-name codes and the
# hour codes can be read off one value.
SUNDAY = 44794.09722222222
ZERO = 0.0
FLIGHT = 0.0972222222222222   # 2:20 as a duration

# (label, numFmtId, formatCode or None for a built-in, value)
CASES = [
    ("builtin-20", 20, None, FLIGHT),
    ("builtin-20-zero", 20, None, ZERO),
    ("builtin-18", 18, None, FLIGHT),
    ("builtin-19", 19, None, FLIGHT),
    ("builtin-21", 21, None, FLIGHT),
    ("builtin-22", 22, None, SUNDAY),
    ("builtin-14", 14, None, SUNDAY),
    ("builtin-37", 37, None, -100.0),
    ("builtin-38", 38, None, -100.0),
    ("builtin-39", 39, None, -100.0),
    ("builtin-40", 40, None, -100.0),
    ("builtin-40-pos", 40, None, 1000.0),
    ("builtin-41", 41, None, -100.0),
    ("builtin-43", 43, None, -100.0),
    ("builtin-44", 44, None, -100.0),
    ("builtin-45", 45, None, FLIGHT),
    ("builtin-46", 46, None, FLIGHT),
    ("h:mm", None, "h:mm", FLIGHT),
    ("hh:mm", None, "hh:mm", FLIGHT),
    ("[h]:mm", None, "[h]:mm", FLIGHT),
    ("[hh]:mm", None, "[hh]:mm", FLIGHT),
    ("[h]:mm-25h", None, "[h]:mm", 1.0972222222222222),
    ("[hh]:mm-25h", None, "[hh]:mm", 1.0972222222222222),
    ("aaa", None, "aaa", SUNDAY),
    ("aaaa", None, "aaaa", SUNDAY),
    ("mmddyy-aaaa", None, "mm/dd/yy\\ aaaa", SUNDAY),
    ("ddd", None, "ddd", SUNDAY),
    ("dddd", None, "dddd", SUNDAY),
    ("nn", None, "nn", SUNDAY),
    ("nnnn", None, "nnnn", SUNDAY),
]

BASE = 200


def build(target: Path) -> None:
    numfmts, xfs = [], []
    for i, (_, builtin, code, _value) in enumerate(CASES):
        if code is None:
            xfs.append(f'<xf numFmtId="{builtin}" fontId="0" fillId="0" borderId="0"'
                       f' xfId="0" applyNumberFormat="1"/>')
        else:
            fmt = BASE + i
            numfmts.append(f'<numFmt numFmtId="{fmt}"'
                           f' formatCode="{html.escape(code, quote=True)}"/>')
            xfs.append(f'<xf numFmtId="{fmt}" fontId="0" fillId="0" borderId="0"'
                       f' xfId="0" applyNumberFormat="1"/>')

    styles = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f'<styleSheet xmlns="{MAIN}">'
        f'<numFmts count="{len(numfmts)}">{"".join(numfmts)}</numFmts>'
        f'<fonts count="1"><font><sz val="11"/><name val="Liberation Sans"/></font></fonts>'
        f'<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
        f'<borders count="1"><border/></borders>'
        f'<cellStyleXfs count="1">'
        f'<xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
        f'<cellXfs count="{len(xfs) + 1}">'
        f'<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>{"".join(xfs)}</cellXfs>'
        f'<cellStyles count="1">'
        f'<cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles></styleSheet>')

    rows = []
    for i, (label, _b, _c, value) in enumerate(CASES):
        at = i + 1
        rows.append(
            f'<row r="{at}">'
            f'<c r="A{at}" t="inlineStr"><is><t>{html.escape(label)}</t></is></c>'
            f'<c r="B{at}" s="{i + 1}"><v>{value!r}</v></c></row>')

    sheet = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             f'<worksheet xmlns="{MAIN}"><cols>'
             f'<col min="1" max="1" width="18" customWidth="1"/>'
             f'<col min="2" max="2" width="30" customWidth="1"/></cols>'
             f'<sheetData>{"".join(rows)}</sheetData></worksheet>')

    write(target, styles, sheet)


def write(target: Path, styles: str, sheet: str) -> None:
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


def read(pdf: Path) -> list[str]:
    """Every drawn string, grouped by baseline, with non-ASCII spelled out."""
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
    out = []
    for y in sorted(rows):
        text = " | ".join(t for _, t in sorted(rows[y]))
        out.append("".join(
            c if 32 <= ord(c) < 127 else f"<U+{ord(c):04X}>" for c in text))
    return out


def render(binary: str, doc: Path) -> Path | None:
    work = doc.parent / ("out-" + binary.strip("/").split("/")[1])
    work.mkdir(parents=True, exist_ok=True)
    subprocess.run([binary, f"-env:UserInstallation=file://{work.absolute()}/profile",
                    "--headless", "--norestore", "--convert-to", "pdf",
                    "--outdir", str(work), str(doc)], capture_output=True, timeout=600)
    rendered = work / (doc.stem + ".pdf")
    # Assert the instrument produced output before comparing anything to it.
    return rendered if rendered.exists() else None


if __name__ == "__main__":
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "codes.xlsx")
    build(out)
    print(f"wrote {out}")
    for binary in ("/opt/libreoffice26.2/program/soffice", "/usr/bin/soffice"):
        pdf = render(binary, out)
        if pdf is None:
            print(f"{binary}: RENDER FAILED")
            continue
        print(binary)
        for line in read(pdf):
            print("   ", line)
