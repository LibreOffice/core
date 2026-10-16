"""Builds `default.xlsx`: which cell format a cell that states no `s` takes.

`042_Business_monthly_budget` prints `500` where both references print `500.00`, and its
`cellXfs[0]` names built-in id 40 while its cells state no `s` at all. Two candidate rules
answer that — the cell takes `cellXfs[0]`, or it takes the Default cell style, which is the
`cellStyleXfs` entry the `Normal` `cellStyle` names — and in that workbook both are id 40, so
the document cannot separate them. This makes them different and asks the binaries.

It also asks the two questions that sit beside it: whether a `<col style=…>` and a
`<row s=… customFormat="1">` reach a cell that states no `s` of its own, and whether a row's
`s` reaches it without `customFormat`.
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from importlib import import_module

codes = import_module("make-codes".replace("-", "_")) if False else None

import importlib.util

_spec = importlib.util.spec_from_file_location("mkcodes", Path(__file__).parent / "make-codes.py")
mkcodes = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(mkcodes)

MAIN = mkcodes.MAIN

# cellXfs[0] -> "0.000"      (three decimals: unmistakable)
# cellStyleXfs[0] -> "0.0"   (one decimal)
# col style     -> cellXfs[2] -> "0.00000"
# row style     -> cellXfs[3] -> "0.0000000"
STYLES = (
    f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    f'<styleSheet xmlns="{MAIN}">'
    f'<numFmts count="4">'
    f'<numFmt numFmtId="200" formatCode="0.000"/>'
    f'<numFmt numFmtId="201" formatCode="0.0"/>'
    f'<numFmt numFmtId="202" formatCode="0.00000"/>'
    f'<numFmt numFmtId="203" formatCode="0.0000000"/>'
    f'</numFmts>'
    f'<fonts count="1"><font><sz val="11"/><name val="Liberation Sans"/></font></fonts>'
    f'<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
    f'<borders count="1"><border/></borders>'
    f'<cellStyleXfs count="1">'
    f'<xf numFmtId="201" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
    f'<cellXfs count="4">'
    f'<xf numFmtId="200" fontId="0" fillId="0" borderId="0" xfId="0"/>'
    f'<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>'
    f'<xf numFmtId="202" fontId="0" fillId="0" borderId="0" xfId="0" applyNumberFormat="1"/>'
    f'<xf numFmtId="203" fontId="0" fillId="0" borderId="0" xfId="0" applyNumberFormat="1"/>'
    f'</cellXfs>'
    f'<cellStyles count="1">'
    f'<cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles></styleSheet>')

ROWS = [
    # A: label, B: the cell under test
    ('<row r="1"><c r="A1" t="inlineStr"><is><t>no-s</t></is></c>'
     '<c r="B1"><v>1.0</v></c></row>'),
    ('<row r="2"><c r="A2" t="inlineStr"><is><t>s=0</t></is></c>'
     '<c r="B2" s="0"><v>1.0</v></c></row>'),
    ('<row r="3"><c r="A3" t="inlineStr"><is><t>s=1-General</t></is></c>'
     '<c r="B3" s="1"><v>1.0</v></c></row>'),
    # column C carries <col style="2">, and the cell states no s
    ('<row r="4"><c r="A4" t="inlineStr"><is><t>col-style</t></is></c>'
     '<c r="C4"><v>1.0</v></c></row>'),
    # row 5 states s=3 with customFormat
    ('<row r="5" s="3" customFormat="1"><c r="A5" t="inlineStr"><is><t>row-custom</t></is></c>'
     '<c r="B5"><v>1.0</v></c></row>'),
    # row 6 states s=3 without customFormat
    ('<row r="6" s="3"><c r="A6" t="inlineStr"><is><t>row-no-custom</t></is></c>'
     '<c r="B6"><v>1.0</v></c></row>'),
]

SHEET = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
         f'<worksheet xmlns="{MAIN}"><cols>'
         f'<col min="1" max="1" width="18" customWidth="1"/>'
         f'<col min="2" max="2" width="24" customWidth="1"/>'
         f'<col min="3" max="3" width="24" style="2" customWidth="1"/></cols>'
         f'<sheetData>{"".join(ROWS)}</sheetData></worksheet>')

if __name__ == "__main__":
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "default.xlsx")
    mkcodes.write(out, STYLES, SHEET)
    print(f"wrote {out}")
    print("cellXfs[0]=0.000  cellStyleXfs[0]=0.0  col=0.00000  row=0.0000000")
    for binary in ("/opt/libreoffice26.2/program/soffice", "/usr/bin/soffice"):
        pdf = mkcodes.render(binary, out)
        if pdf is None:
            print(f"{binary}: RENDER FAILED")
            continue
        print(binary)
        for line in mkcodes.read(pdf):
            print("   ", line)
