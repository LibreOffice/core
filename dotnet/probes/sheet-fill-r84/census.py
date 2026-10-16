#!/usr/bin/env python3
"""What a worksheet shape declares for its fill and outline, and what the theme answers.

    census.py <corpus-root> [style|kinds]

`style` — the `xdr:style` half: which `a:fillRef`/`a:lnRef` indices the 583 styled shapes
          name, and what kind of element the workbook's own theme holds at that index. That
          is the question a reader has to answer before it can draw one, because
          `a:fillStyleLst` entry 2 and 3 are a gradient in every theme Office ships.
`kinds`  — every `xdr:` child of an anchor by name, so "644 shapes" can be split into the
          `sp`, `cxnSp` and `grpSp` that make it up.

xlsx family only; the corpus holds no ODF spreadsheet.
"""
import collections
import os
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'
XDR = '{http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing}'
R = '{http://schemas.openxmlformats.org/officeDocument/2006/relationships}'
EXTS = ('.xlsx', '.xlsm', '.xltx')


def workbooks(root):
    for dirpath, _, files in os.walk(root):
        for name in sorted(files):
            if name.lower().endswith(EXTS):
                path = os.path.join(dirpath, name)
                try:
                    yield os.path.relpath(path, root), zipfile.ZipFile(path)
                except Exception:
                    continue


def theme_of(z):
    """The workbook's first theme part, as (fillStyleLst kinds, lnStyleLst widths)."""
    for name in z.namelist():
        if re.match(r'xl/theme/theme[^/]*\.xml$', name):
            try:
                root = ET.fromstring(z.read(name))
            except Exception:
                return None
            fmt = root.find(f'{A}themeElements/{A}fmtScheme')
            if fmt is None:
                return None
            fills = [c.tag.replace(A, '') for c in fmt.find(f'{A}fillStyleLst') or []]
            lines = [c.get('w') for c in fmt.find(f'{A}lnStyleLst') or []]
            return fills, lines
    return None


def style(root):
    fillrefs = collections.Counter()
    lnrefs = collections.Counter()
    resolved = collections.Counter()
    lineresolved = collections.Counter()
    both = 0
    styled = 0
    for rel, z in workbooks(root):
        theme = theme_of(z)
        for name in z.namelist():
            if not re.match(r'xl/drawings/drawing[^/]*\.xml$', name):
                continue
            try:
                tree = ET.fromstring(z.read(name))
            except Exception:
                continue
            for shape in tree.iter(XDR + 'sp'):
                st = shape.find(XDR + 'style')
                if st is None:
                    continue
                styled += 1
                f = st.find(A + 'fillRef')
                ln = st.find(A + 'lnRef')
                fi = f.get('idx') if f is not None else None
                li = ln.get('idx') if ln is not None else None
                fillrefs[fi] += 1
                lnrefs[li] += 1
                if theme and fi and fi.isdigit():
                    i = int(fi)
                    kinds = theme[0]
                    resolved[kinds[min(i, len(kinds)) - 1] if 1 <= i and kinds else 'none'] += 1
                if theme and li and li.isdigit():
                    i = int(li)
                    ws = theme[1]
                    lineresolved[ws[min(i, len(ws)) - 1] if 1 <= i and ws else 'none'] += 1
                # does the shape ALSO state its own fill?
                pr = shape.find(XDR + 'spPr')
                if pr is not None and any(pr.find(A + k) is not None for k in
                                          ('solidFill', 'noFill', 'gradFill', 'blipFill',
                                           'pattFill', 'grpFill')):
                    both += 1
    print(f'{styled} styled shapes; {both} of them also state a fill of their own')
    print('a:fillRef idx :', fillrefs.most_common())
    print('  resolves to :', resolved.most_common())
    print('a:lnRef   idx :', lnrefs.most_common())
    print('  theme line w:', lineresolved.most_common())


def kinds(root):
    counts = collections.Counter()
    docs = collections.defaultdict(set)
    for rel, z in workbooks(root):
        for name in z.namelist():
            if not re.match(r'xl/drawings/drawing[^/]*\.xml$', name):
                continue
            try:
                tree = ET.fromstring(z.read(name))
            except Exception:
                continue
            for anchor in tree:
                for child in anchor:
                    if not child.tag.startswith(XDR):
                        continue
                    tag = child.tag.replace(XDR, '')
                    if tag in ('from', 'to', 'ext', 'pos', 'clientData'):
                        continue
                    counts[tag] += 1
                    docs[tag].add(rel)
            for grp in tree.iter(XDR + 'grpSp'):
                for child in grp:
                    tag = child.tag.replace(XDR, '')
                    if tag in ('nvGrpSpPr', 'grpSpPr'):
                        continue
                    counts['in-group:' + tag] += 1
                    docs['in-group:' + tag].add(rel)
    for key, value in counts.most_common():
        print(f'{key:22s} {value:5d} in {len(docs[key]):3d} documents')


if __name__ == '__main__':
    root = sys.argv[1]
    for what in sys.argv[2:] or ['style']:
        print(f'== {what}')
        globals()[what](root)
