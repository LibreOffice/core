#!/usr/bin/env python3
"""How many corpus VML anchors state an offset larger than the cell it is measured from?

`audit_vmlanchor.py` verified the site's 96-dpi claim on 26.2.4.2 and, in the same rendering,
found a rule the site does not state: **26.2.4.2 clamps a VML anchor's offset to the anchored
cell's own extent.**  A row offset of 200 or 400 px into a 60 pt (80 px) row both land on exactly
the next row's top -- 179.885 pt against a row-3 top of 180.0.  `XlsxVml.ParseAnchor` converts the
offset and does not clamp it.

So the question is whether any corpus anchor is large enough for the difference to show.

**The first cut of this census answered "zero" and was measuring its own shortcut.** It compared
every anchor against the tallest row and the widest column stated *anywhere in the workbook*,
which is an upper bound generous enough that nothing can exceed it -- one 200 pt row exempts the
whole file.  The largest row offset in the corpus is 111 px (83.25 pt) against a 20 px default
row, so "zero" was a property of the bound.  This version resolves each VML part to its own
worksheet through the relationships and compares each anchor against **that row's own height**.

WHAT THIS STILL CANNOT SEE: `.xls` carries the same objects in an `OBJ`/`TXO` pair rather than in
VML and is not read here; a column's width in characters is turned into pixels through the
workbook's own digit width, approximated at 7 px per character; and a row whose height the
optimal-height pass recomputes is followed only as far as its stated `ht`.
"""
import csv, os, re, zipfile
from collections import defaultdict

ROOT = "/c/sandbox/workdir/sample-files/"
PX_PER_POINT = 96.0 / 72.0
PX_PER_CHAR = 7.0

paths = []
with open(ROOT + "MANIFEST.tsv") as f:
    r = csv.reader(f, delimiter="\t")
    next(r)
    for row in r:
        if row[0] == "sheets":
            paths.append(row[2])


def grid(sheet_xml):
    """(rowHeightPx by index, colWidthPx by index, defaults) for one worksheet part."""
    default_row = 15.0
    m = re.search(r'<sheetFormatPr[^>]*\bdefaultRowHeight="([\d.]+)"', sheet_xml)
    if m:
        default_row = float(m.group(1))
    default_col = 8.43
    m = re.search(r'<sheetFormatPr[^>]*\bdefaultColWidth="([\d.]+)"', sheet_xml)
    if m:
        default_col = float(m.group(1))

    rows = {}
    for m in re.finditer(r'<row[^>]*\br="(\d+)"[^>]*>', sheet_xml):
        tag = m.group(0)
        h = re.search(r'\bht="([\d.]+)"', tag)
        if h:
            rows[int(m.group(1)) - 1] = float(h.group(1))

    cols = {}
    for m in re.finditer(r"<col\b[^>]*/?>", sheet_xml):
        tag = m.group(0)
        a = re.search(r'\bmin="(\d+)"', tag)
        b = re.search(r'\bmax="(\d+)"', tag)
        w = re.search(r'\bwidth="([\d.]+)"', tag)
        if a and b and w:
            for c in range(int(a.group(1)) - 1, min(int(b.group(1)), 1000)):
                cols[c] = float(w.group(1))

    return rows, cols, default_row, default_col


docs = anchors = over = 0
overdocs = defaultdict(int)
worst = 0.0
for p in paths:
    if not p.lower().endswith((".xlsx", ".xlsm", ".xltx", ".xltm")):
        continue
    try:
        z = zipfile.ZipFile(ROOT + p)
    except Exception:
        continue
    names = set(z.namelist())
    seen_any = False

    for sheet in sorted(n for n in names if re.match(r"xl/worksheets/sheet\d+\.xml$", n)):
        rel = "xl/worksheets/_rels/" + os.path.basename(sheet) + ".rels"
        if rel not in names:
            continue
        rels = z.read(rel).decode("utf8", "replace")
        targets = [m.group(1) for m in re.finditer(
            r'<Relationship[^>]*Type="[^"]*/vmlDrawing"[^>]*Target="([^"]+)"', rels)]
        targets += [m.group(1) for m in re.finditer(
            r'<Relationship[^>]*Target="([^"]+)"[^>]*Type="[^"]*/vmlDrawing"', rels)]
        if not targets:
            continue

        sheet_xml = z.read(sheet).decode("utf8", "replace")
        rows, cols, drow, dcol = grid(sheet_xml)

        for t in targets:
            part = os.path.normpath(os.path.join("xl/worksheets", t)).replace("\\", "/")
            if part not in names:
                continue
            seen_any = True
            d = z.read(part).decode("utf8", "replace")
            for m in re.finditer(r"<x:Anchor>([^<]*)</x:Anchor>", d):
                parts = [q.strip() for q in m.group(1).split(",")]
                if len(parts) < 8:
                    continue
                try:
                    v = [int(q) for q in parts]
                except ValueError:
                    continue
                anchors += 1
                bad = False
                for col, off, row, roff in ((v[0], v[1], v[2], v[3]), (v[4], v[5], v[6], v[7])):
                    rh = rows.get(row, drow) * PX_PER_POINT
                    cw = cols.get(col, dcol) * PX_PER_CHAR
                    if rh > 0:
                        worst = max(worst, roff / rh)
                    if roff > rh or off > cw:
                        bad = True
                if bad:
                    over += 1
                    overdocs[p] += 1
    if seen_any:
        docs += 1

print("xlsx-family sheets documents carrying a VML part: %d" % docs)
print("x:Anchor elements resolved to their own worksheet: %d" % anchors)
print("  anchors whose offset exceeds its own cell:       %d" % over)
print("  documents affected:                              %d" % len(overdocs))
print("  largest row offset seen, as a multiple of its own row: %.2f" % worst)
for p, n in sorted(overdocs.items()):
    print("    %3d  %s" % (n, p))
