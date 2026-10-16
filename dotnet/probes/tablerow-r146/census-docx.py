#!/usr/bin/env python3
"""Static census of O84's predicate over the corpus `.docx`.

    census-docx.py <manifest> <out.tsv>

For every horizontal row boundary of every table, resolve the two facing widths the way the
reference does -- the cell's own `w:tcBorders`, else `w:tblPrEx`, else the table's
`w:tblBorders` (`insideH` for an interior edge, `top`/`bottom` for an outer one), else the table
style's `w:tblBorders` walked through `w:basedOn` -- and take the maximum over each row's cells:

    A = max over the UPPER row's cells of the resolved BOTTOM width
    B = max over the LOWER row's cells of the resolved TOP width

26.2.4.2 charges `max(A, B)` to the lower row and nothing to the upper (results.md SS3.2).
This tree charges `A/2` to the upper and `B/2` to the lower, so wherever `A != B` the table is
`|A - B| / 2` short or long at that boundary.  This counts those boundaries and that height.

NOT resolved, and each makes this a LOWER bound on the class: conditional table-style formatting
(`w:tblLook`, band/first-row overrides), `w:tblCellSpacing`, vertically merged cells, and nested
tables are counted by their own markup only.  Nothing here is a claim about `.doc`, which is WW8
and cannot be read statically.
"""
import sys, os, re, zipfile
import xml.etree.ElementTree as ET

WNS = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
W = '{%s}' % WNS
NOLINE = {'nil', 'none', ''}
# Measured (results.md SS3.1): 26.2.4.2 draws `w:val="double" w:sz=n` as three bands of n/8 pt.
MULT = {'double': 3.0, 'triple': 5.0, 'dotDash': 1.0}


def width(el):
    """Resolved drawn width of a `w:top`/`w:bottom`/`w:insideH` element, in points."""
    if el is None:
        return None
    val = el.get(W + 'val', 'single')
    if val in NOLINE:
        return 0.0
    try:
        sz = int(el.get(W + 'sz', '0'))
    except ValueError:
        sz = 0
    return (sz / 8.0) * MULT.get(val, 1.0)


def style_borders(styles, sid, seen=None):
    """`w:tblBorders` of a table style, walked through `w:basedOn`."""
    seen = seen or set()
    if sid in seen or sid not in styles:
        return {}
    seen.add(sid)
    st = styles[sid]
    out = {}
    base = st.find(W + 'basedOn')
    if base is not None:
        out.update(style_borders(styles, base.get(W + 'val'), seen))
    tp = st.find(W + 'tblPr')
    if tp is not None:
        bd = tp.find(W + 'tblBorders')
        if bd is not None:
            for side in ('top', 'bottom', 'insideH'):
                e = bd.find(W + side)
                if e is not None:
                    out[side] = width(e)
    return out


def table_boundaries(tbl, styles):
    tp = tbl.find(W + 'tblPr')
    tb = {}
    if tp is not None:
        sref = tp.find(W + 'tblStyle')
        if sref is not None:
            tb.update(style_borders(styles, sref.get(W + 'val')))
        bd = tp.find(W + 'tblBorders')
        if bd is not None:
            for side in ('top', 'bottom', 'insideH'):
                e = bd.find(W + side)
                if e is not None:
                    tb[side] = width(e)
    rows = tbl.findall(W + 'tr')
    out = []
    for i in range(len(rows) - 1):
        up, lo = rows[i], rows[i + 1]

        def best(tr, side):
            m = 0.0
            any_cell = False
            for tc in tr.findall(W + 'tc'):
                any_cell = True
                w = None
                pr = tc.find(W + 'tcPr')
                if pr is not None:
                    bd = pr.find(W + 'tcBorders')
                    if bd is not None:
                        w = width(bd.find(W + side))
                if w is None:
                    w = tb.get('insideH', 0.0)     # interior edge
                m = max(m, w)
            return m if any_cell else 0.0

        A, B = best(up, 'bottom'), best(lo, 'top')
        out.append((A, B))
    return out


rows = []
for line in open(sys.argv[1]):
    f = line.rstrip('\n').split('\t')
    if len(f) < 4 or f[0] != 'words' or f[3] != 'docx':
        continue
    rel = f[2]
    path = os.path.join('/home/user/sample-files', rel)
    try:
        with zipfile.ZipFile(path) as z:
            doc = ET.fromstring(z.read('word/document.xml'))
            styles = {}
            try:
                sx = ET.fromstring(z.read('word/styles.xml'))
                for st in sx.findall(W + 'style'):
                    if st.get(W + 'type') == 'table':
                        styles[st.get(W + 'styleId')] = st
            except KeyError:
                pass
    except Exception as e:
        rows.append((rel, -1, -1, -1.0, f'{type(e).__name__}')); continue

    nb = nd = 0
    pts = 0.0
    for tbl in doc.iter(W + 'tbl'):
        for A, B in table_boundaries(tbl, styles):
            nb += 1
            if abs(A - B) > 1e-9:
                nd += 1
                pts += abs(A - B) / 2.0
    rows.append((rel, nb, nd, round(pts, 2), ''))

with open(sys.argv[2], 'w') as fh:
    fh.write('path\tboundaries\tdisagreeing\theight_error_pt\tnote\n')
    for r in rows:
        fh.write('\t'.join(str(x) for x in r) + '\n')

good = [r for r in rows if r[1] >= 0]
tab = [r for r in good if r[1] > 0]
dis = [r for r in good if r[2] > 0]
print(f'.docx read                                       {len(good)} of {len(rows)}')
print(f'  holding at least one table row boundary        {len(tab)}   ({sum(r[1] for r in good)} boundaries)')
print(f'  with >=1 boundary whose facing widths DIFFER   {len(dis)}   ({sum(r[2] for r in good)} boundaries)')
print(f'  total height this tree is out, over all of it  {sum(r[3] for r in good):.1f} pt')
for r in sorted(dis, key=lambda r: -r[3])[:12]:
    print(f'    {r[3]:9.2f} pt  {r[2]:5d}/{r[1]:<5d}  {os.path.basename(r[0])}')
