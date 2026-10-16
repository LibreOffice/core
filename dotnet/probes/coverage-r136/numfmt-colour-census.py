#!/usr/bin/env python3
"""Cells whose number format states a colour, which this tree discards.

    numfmt-colour-census.py <corpus-root> <manifest.tsv> <out.tsv>

`NumberFormatSection` drops every bracketed body it does not recognise as a currency symbol,
an elapsed unit or a calendar directive, under the comment *"Anything else -- a colour name,
[ENG] -- changes appearance rather than the text this extracts"*.  That is true of the text
and false of the ink: `[Red]` in a section makes the cells that section formats red.

Counted per workbook: the `cellXfs` entries whose format code holds a colour clause -- from
`xl/styles.xml`'s own `numFmt` codes and from the built-in ids that carry one -- and the cells
on every sheet that use one of them.  A cell is counted only when it holds a number, because
a colour clause sits in one section and only decides the cells that section formats; the
count is therefore an UPPER bound on the cells drawn in the wrong colour and a lower bound on
nothing.
"""
import pathlib
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

NS = '{http://schemas.openxmlformats.org/spreadsheetml/2006/main}'
COLOURS = re.compile(r'\[(black|blue|cyan|green|magenta|red|white|yellow|color\s*\d+)\]', re.I)
# ECMA-376 17.4.7.  The ids whose built-in code carries a colour clause.
BUILTIN = {5, 6, 7, 8, 38, 40}

root, man, out = sys.argv[1:4]
rows = []
for line in pathlib.Path(man).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    if len(p) < 4 or p[3] not in ('xlsx', 'xlsm', 'xltx'):
        continue
    try:
        z = zipfile.ZipFile(pathlib.Path(root) / p[2])
    except (zipfile.BadZipFile, OSError):
        continue
    with z:
        try:
            st = ET.fromstring(z.read('xl/styles.xml'))
        except (KeyError, ET.ParseError):
            continue
        codes = {}
        for nf in st.iter(NS + 'numFmt'):
            try:
                codes[int(nf.get('numFmtId'))] = nf.get('formatCode') or ''
            except (TypeError, ValueError):
                pass
        cellxfs = st.find(NS + 'cellXfs')
        tinted = {}
        for i, xf in enumerate(list(cellxfs) if cellxfs is not None else []):
            try:
                fid = int(xf.get('numFmtId') or 0)
            except ValueError:
                continue
            code = codes.get(fid, '')
            if fid in BUILTIN:
                # every built-in id in the set carries its colour in the NEGATIVE section
                tinted[i] = 'neg'
            elif COLOURS.search(code):
                parts = code.split(';')
                tinted[i] = 'all' if COLOURS.search(parts[0]) else 'neg'
        if not tinted:
            continue
        cells = hit = 0
        for name in z.namelist():
            if not re.match(r'^xl/worksheets/sheet\d+\.xml$', name):
                continue
            try:
                sh = ET.fromstring(z.read(name))
            except ET.ParseError:
                continue
            for c in sh.iter(NS + 'c'):
                if c.get('t') in ('s', 'str', 'inlineStr', 'b', 'e'):
                    continue
                if c.find(NS + 'v') is None:
                    continue
                try:
                    s = int(c.get('s') or 0)
                except ValueError:
                    continue
                if s not in tinted:
                    continue
                cells += 1
                if tinted[s] == 'all':
                    hit += 1
                else:
                    try:
                        if float(c.find(NS + 'v').text or '0') < 0:
                            hit += 1
                    except (TypeError, ValueError):
                        pass
        rows.append((p[2], len(tinted), cells, hit))

rows.sort(key=lambda r: -r[3])
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('path\ttinted_cellXfs\tnumeric_cells_using_one\tcells_whose_value_takes_the_coloured_section\n')
    for r in rows:
        fh.write('\t'.join(str(x) for x in r) + '\n')
print(f'{len(rows)} of the corpus xlsx state a number format with a colour clause; '
      f'{sum(1 for r in rows if r[2])} use one on at least one numeric cell '
      f'({sum(r[2] for r in rows)} cells), and '
      f'{sum(1 for r in rows if r[3])} documents and {sum(r[3] for r in rows)} cells hold a '
      f'value that actually takes the coloured section')
