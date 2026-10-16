#!/usr/bin/env python3
"""A `<w:br/>` inside a shape's text body, and whether its own run states a size.

    txbx-br-census.py <corpus-root> <manifest.tsv> <out.tsv>

`086_Printable_Graph_Paper_Template_Gray_Theme` puts four `<w:br/>` runs at `w:sz="4"` --
two points -- ahead of the only line of a 32.65 pt text box.  26.2.4.2 gives each of those
lines 2.7 pt, which is 2 pt at the paragraph's own 1.35 spacing, and this tree gives each
14.8 pt, which is the *paragraph mark's* 11 pt; the body then exceeds the box and this tree
draws none of it.  So the census counts, per document:

  brs        `<w:br/>` inside a `w:txbxContent`
  sized      those whose own run states a `w:sz`
  smaller    those whose own `w:sz` is SMALLER than the paragraph mark's (or the 22 half-point
             default when the mark states none) -- the population where the two rules differ
"""
import pathlib
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
PARTS = re.compile(r'^word/(document|header\d*|footer\d*|footnotes|endnotes)\.xml$')

root, man, out = sys.argv[1:4]
rows = []
for line in pathlib.Path(man).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    if len(p) < 4 or p[3] not in ('docx', 'docm', 'dotx'):
        continue
    brs = sized = smaller = 0
    try:
        with zipfile.ZipFile(pathlib.Path(root) / p[2]) as z:
            for name in z.namelist():
                if not PARTS.match(name):
                    continue
                try:
                    tree = ET.fromstring(z.read(name))
                except ET.ParseError:
                    continue
                for body in tree.iter(W + 'txbxContent'):
                    for para in body.iter(W + 'p'):
                        mark = para.find(f'{W}pPr/{W}rPr/{W}sz')
                        marksz = int(mark.get(W + 'val')) if mark is not None \
                            and (mark.get(W + 'val') or '').isdigit() else 22
                        for run in para.iter(W + 'r'):
                            if run.find(W + 'br') is None:
                                continue
                            brs += 1
                            sz = run.find(f'{W}rPr/{W}sz')
                            if sz is None or not (sz.get(W + 'val') or '').isdigit():
                                continue
                            sized += 1
                            if int(sz.get(W + 'val')) < marksz:
                                smaller += 1
    except (zipfile.BadZipFile, OSError):
        continue
    if brs:
        rows.append((p[2], brs, sized, smaller))

rows.sort(key=lambda r: -r[3])
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('path\tbrs_in_txbx\tbr_run_states_size\tbr_run_smaller_than_mark\n')
    for r in rows:
        fh.write('\t'.join(str(x) for x in r) + '\n')
print(f'{len(rows)} docx hold a <w:br/> inside a shape text body, {sum(r[1] for r in rows)} breaks; '
      f'{sum(1 for r in rows if r[3])} documents and {sum(r[3] for r in rows)} breaks state a size '
      f'SMALLER than their own paragraph mark, which is where the two rules disagree')
