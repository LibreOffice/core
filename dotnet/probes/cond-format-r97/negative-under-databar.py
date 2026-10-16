#!/usr/bin/env python3
"""Every numeric cell a corpus `dataBar` rule covers, and how many of them are negative.

This is the census behind §2 arm (4)'s **nil reach**. The arm itself is a real correction — a bar
whose extension states no `x14:negativeFillColor` paints a negative value in the source's own
`COL_LIGHTRED` and not in the bar's colour, which this tree had backwards — but a correction with
no cell to apply it to moves no rendering, and the ground rules want the census rather than "I
could not find a witness".

Two independent checks, because either alone is weak:

* the values, straight out of each package over each rule's own `sqref`, which says the branch is
  never entered; and
* `#ff0000` filled rectangles in 26.2.4.2's own banked rendering of each of the six documents,
  which says the reference never draws one either.

The ranges are read from the worksheets rather than hard-coded, so the script stays correct if the
corpus gains a rule. Run it from anywhere.

Usage: negative-under-databar.py [corpus-root] [reference-bank]
"""
import csv
import os
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

NS = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"
ROOT = sys.argv[1] if len(sys.argv) > 1 else "/home/user/sample-files"
BANK = sys.argv[2] if len(sys.argv) > 2 else "/home/user/gate-orig-r83/ref"


def col_index(letters):
    n = 0
    for ch in letters:
        n = n * 26 + (ord(ch) - 64)
    return n


def ranges_of(sqref):
    """`A1:B2 C3` -> [(firstCol, firstRow, lastCol, lastRow), ...], 1-based inclusive."""
    out = []
    for part in sqref.split():
        ends = part.split(":")
        cells = []
        for e in ends:
            m = re.match(r"\$?([A-Z]+)\$?(\d+)$", e)
            if not m:
                break
            cells.append((col_index(m.group(1)), int(m.group(2))))
        if len(cells) == 1:
            cells.append(cells[0])
        if len(cells) == 2:
            (c0, r0), (c1, r1) = cells
            out.append((min(c0, c1), min(r0, r1), max(c0, c1), max(r0, r1)))
    return out


def main():
    total = negative = rules = 0
    docs = set()
    for dirpath, _, names in os.walk(ROOT):
        for name in names:
            path = os.path.join(dirpath, name)
            try:
                z = zipfile.ZipFile(path)
            except (zipfile.BadZipFile, OSError, IsADirectoryError):
                continue
            with z:
                sheets = [n for n in z.namelist() if n.startswith("xl/worksheets/")
                          and n.endswith(".xml")]
                if not sheets:
                    continue
                for sheet in sheets:
                    try:
                        root = ET.fromstring(z.read(sheet))
                    except ET.ParseError:
                        continue
                    covered = []
                    for block in root.iter(NS + "conditionalFormatting"):
                        if not any(r.get("type") == "dataBar"
                                   for r in block.findall(NS + "cfRule")):
                            continue
                        rules += sum(1 for r in block.findall(NS + "cfRule")
                                     if r.get("type") == "dataBar")
                        covered += ranges_of(block.get("sqref", ""))
                    if not covered:
                        continue
                    docs.add(name)
                    for c in root.iter(NS + "c"):
                        m = re.match(r"([A-Z]+)(\d+)$", c.get("r", ""))
                        v = c.find(NS + "v")
                        if not m or v is None or c.get("t") in ("s", "str", "inlineStr", "e"):
                            continue
                        col, row = col_index(m.group(1)), int(m.group(2))
                        if not any(c0 <= col <= c1 and r0 <= row <= r1
                                   for c0, r0, c1, r1 in covered):
                            continue
                        try:
                            x = float(v.text)
                        except (TypeError, ValueError):
                            continue
                        total += 1
                        if x < 0:
                            negative += 1
                            print(f"NEGATIVE  {name}  {sheet}  {c.get('r')}  {x}")

    print(f"dataBar rules {rules} in {len(docs)} documents")
    print(f"numeric cells they cover: {total}; NEGATIVE: {negative}")

    # The second leg: does the reference itself ever paint COL_LIGHTRED in one of them?
    try:
        import pymupdf
    except ImportError:
        return
    drawn = 0
    for name in sorted(docs):
        pdf = os.path.join(BANK, f"{os.path.splitext(name)[0]}__xlsx.pdf")
        if not os.path.exists(pdf):
            print(f"  (no banked reference for {name})")
            continue
        n = 0
        for page in pymupdf.open(pdf):
            for d in page.get_drawings():
                if d["type"] in ("f", "fs") and d.get("fill") is not None:
                    if "#%02x%02x%02x" % tuple(int(round(v * 255))
                                               for v in d["fill"]) == "#ff0000":
                        n += 1
        drawn += n
        print(f"  {name[:56]:56s} #ff0000 rectangles in 26.2.4.2's own PDF: {n}")
    print(f"COL_LIGHTRED bars the reference draws anywhere in the family: {drawn}")


if __name__ == "__main__":
    main()
