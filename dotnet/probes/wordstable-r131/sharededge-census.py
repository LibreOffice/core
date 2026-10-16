#!/usr/bin/env python3
"""How many word-processing documents state a horizontal cell edge the two sides disagree about?

    sharededge-census.py <out.tsv>

The base rate O82 needs (C9). A `nil` top under a stated bottom is invisible on a page where both
rows are drawn, so the census counts three nested populations:

  * documents holding a table at all,
  * documents where some cell states `nil`/nothing on a horizontal edge whose facing cell states a
    border -- the class the resolution changes,
  * of those, the disagreeing edges themselves.

DOCX only, read out of `word/document.xml` plus the headers and footers. The other three formats
state the same thing in their own spellings and are counted only as "holds a table", which is what
keeps the denominator honest rather than pretending the numerator covers them.
"""
import csv
import pathlib
import re
import sys
import zipfile

CORPUS = pathlib.Path('/home/user/sample-files')
TR = re.compile(r'<w:tr\b.*?</w:tr>', re.S)
TC = re.compile(r'<w:tc>.*?(?=<w:tc>|</w:tr>)', re.S)
TBL = re.compile(r'<w:tbl>.*?</w:tbl>', re.S)
BORDERS = re.compile(r'<w:tcBorders>(.*?)</w:tcBorders>', re.S)
GRID = re.compile(r'<w:gridSpan w:val="(\d+)"')


def edge(tc, side):
    """The stated width in eighths of a point, or None when the cell states nothing."""
    m = BORDERS.search(tc)
    if not m:
        return None
    e = re.search(r'<w:%s ([^/>]*)/>' % side, m.group(1))
    if not e:
        return None
    if 'w:val="nil"' in e.group(1) or 'w:val="none"' in e.group(1):
        return 0
    sz = re.search(r'w:sz="(\d+)"', e.group(1))
    return int(sz.group(1)) if sz else 4


def columns(tc):
    m = GRID.search(tc)
    return int(m.group(1)) if m else 1


rows = []
with (CORPUS / 'MANIFEST.tsv').open() as fh:
    rows = [r for r in csv.DictReader(fh, delimiter='\t') if r['family'] == 'words']

with open(sys.argv[1], 'w', encoding='utf-8') as out:
    out.write('path\text\ttables\trows\tdisagreeing_edges\tnil_under_border\n')
    for r in rows:
        src = CORPUS / r['path']
        ext = r['ext'].lower()
        if ext not in ('docx', 'docm', 'dotx', 'dotm'):
            out.write(f'{r["path"]}\t{ext}\t-1\t-1\t-1\t-1\n')
            continue
        try:
            with zipfile.ZipFile(src) as z:
                parts = [n for n in z.namelist()
                         if re.fullmatch(r'word/(document|header\d*|footer\d*)\.xml', n)]
                text = ''.join(z.read(n).decode('utf-8', 'replace') for n in parts)
        except Exception:                              # noqa: BLE001
            out.write(f'{r["path"]}\t{ext}\t-1\t-1\t-1\t-1\n')
            continue

        tables = rowcount = disagree = nil_under = 0
        for tbl in TBL.findall(text):
            tables += 1
            grid = []
            for tr in TR.findall(tbl):
                rowcount += 1
                cells, col = [], 0
                for tc in TC.findall(tr):
                    span = columns(tc)
                    cells.append((col, col + span, edge(tc, 'top'), edge(tc, 'bottom')))
                    col += span
                grid.append(cells)
            for above, below in zip(grid, grid[1:]):
                for a0, a1, _, ab in above:
                    for b0, b1, bt, _ in below:
                        if b1 <= a0 or b0 >= a1:
                            continue
                        if ab is None and bt is None:
                            continue
                        if (ab or 0) == (bt or 0):
                            continue
                        disagree += 1
                        if (bt or 0) == 0 < (ab or 0) or (ab or 0) == 0 < (bt or 0):
                            nil_under += 1
        out.write(f'{r["path"]}\t{ext}\t{tables}\t{rowcount}\t{disagree}\t{nil_under}\n')
print('done')
