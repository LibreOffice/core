#!/usr/bin/env python3
"""One-attribute variants of `075_Idea_planner_tasks`' blocked header cell.

`B6` holds " Task Status Indicator" in a column one character wide, wraps, and has
`C6` occupied beside it.  26.2.4.2 draws none of it and this tree draws all of it on
one line; these variants say which of the three properties decides that.
"""
import re, shutil, sys, zipfile
from pathlib import Path

SRC = sys.argv[1] if len(sys.argv) > 1 else 'frozen/075_Idea_planner_tasks_f44ebd73.xlsx'
OUT = Path(sys.argv[2] if len(sys.argv) > 2 else 'idea')
OUT.mkdir(exist_ok=True)

VARIANTS = {
    # column B is one character wide; give it forty
    'v1-wide-b': ('xl/worksheets/sheet1.xml',
                  lambda d: d.replace(b'<col min="2" max="2" width="1" style="1" customWidth="1"/>',
                                      b'<col min="2" max="2" width="40" style="1" customWidth="1"/>')),
    # the neighbour that blocks the spill
    'v2-empty-c6': ('xl/worksheets/sheet1.xml',
                    lambda d: d.replace(b'<c r="C6" s="4" t="s"><v>8</v></c>', b'<c r="C6" s="4"/>')),
    # the cell wraps; style 4 is B6's and C6's alone in row 6, so drop the wrap from it
    'v3-no-wrap': ('xl/styles.xml',
                   lambda d: d.replace(
                       b'<xf numFmtId="0" fontId="8" fillId="4" borderId="0" xfId="15" applyAlignment="1">'
                       b'<alignment horizontal="left" vertical="center" wrapText="1" indent="1"/>',
                       b'<xf numFmtId="0" fontId="8" fillId="4" borderId="0" xfId="15" applyAlignment="1">'
                       b'<alignment horizontal="left" vertical="center" indent="1"/>')),
    # a taller row, so a wrapped block has room for its lines
    'v4-tall-row': ('xl/worksheets/sheet1.xml',
                    lambda d: re.sub(rb'(<row r="6"[^>]*?)ht="[0-9.]+"', rb'\1ht="200"', d)),
}

shutil.copy(SRC, OUT / 'v0-control.xlsx')
for name, (part, fn) in VARIANTS.items():
    with zipfile.ZipFile(SRC) as zf, \
            zipfile.ZipFile(OUT / f'{name}.xlsx', 'w', zipfile.ZIP_DEFLATED) as out:
        changed = False
        for i in zf.infolist():
            d = zf.read(i.filename)
            if i.filename == part:
                nd = fn(d)
                changed = nd != d
                d = nd
            out.writestr(i, d)
    print(name, 'changed' if changed else 'NO CHANGE')
