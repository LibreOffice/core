#!/usr/bin/env python3
"""What do the cells inside each pivot's stated range resolve to, for the four unmeasured
properties — the font face, the font size, the font colour and the fill?

`statedborders.py` in `probes/pivot-res-r108` censused a border, a bold weight, a horizontal
alignment and an indent, and it counted only cells that carry an `s` of their own. That
undercounts: a cell with no `s` still resolves through its row's format when the row is
`customFormat`, then through its column's, then through the sheet default — which is the
resolution `XlsxCellFormats` performs and the one `clearContents` undoes.

So this counts, over every cell of every pivot's stated rectangle:

  cells         the rectangle's own cell count, which is the base rate every count below is
                against
  differs=…     the cells whose *resolved* value for that property is not the value the
                clearing falls back to — the sheet's own default `cellXf`, `cellXfs[0]`,
                which is what `XlsxSheetFormats` calls `SheetDefault`

`class` is the font's `<family val="N"/>` — the declared generic class, which travels with the
face and is a separate property from its name: `035_Project_plan_for_law_firms` states `Cambria`
`family="1"` on part of its pivot where its default states `Cambria` `family="2"`, and a census
that compared only the names would report that document as stating nothing at all.

A property whose `differs` is zero over every range cannot be seen on this corpus whatever the
code does with it.

    statedface.py <workbook> ...
"""
import sys, zipfile, os, re
import xml.etree.ElementTree as ET

M = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'


def m(t):
    return '{%s}%s' % (M, t)


def colnum(s):
    n = 0
    for ch in s:
        n = n * 26 + ord(ch) - 64
    return n - 1


def parse_ref(ref):
    a, _, b = ref.partition(':')
    b = b or a
    ma = re.match(r'\$?([A-Z]+)\$?(\d+)', a)
    mb = re.match(r'\$?([A-Z]+)\$?(\d+)', b)
    return colnum(ma.group(1)), int(ma.group(2)) - 1, colnum(mb.group(1)), int(mb.group(2)) - 1


def colour_of(e):
    """A `<color>`'s identity as written: rgb, theme+tint, indexed, or auto."""
    if e is None:
        return None
    if e.get('rgb'):
        return ('rgb', e.get('rgb').upper())
    if e.get('theme') is not None:
        return ('theme', e.get('theme'), e.get('tint') or '0')
    if e.get('indexed') is not None:
        return ('indexed', e.get('indexed'))
    if e.get('auto') in ('1', 'true'):
        return None
    return None


def styles(z):
    """cellXfs index -> (face, size, colour, fill); plus the same for the `Normal` cellStyleXf."""
    st = ET.fromstring(z.read('xl/styles.xml'))

    fonts = []
    for f in st.find(m('fonts')) or []:
        name = f.find(m('name'))
        sz = f.find(m('sz'))
        fam = f.find(m('family'))
        fonts.append((
            name.get('val') if name is not None else None,
            fam.get('val') if fam is not None else None,
            sz.get('val') if sz is not None else None,
            colour_of(f.find(m('color'))),
        ))

    fills = []
    for fl in st.find(m('fills')) or []:
        p = fl.find(m('patternFill'))
        g = fl.find(m('gradientFill'))
        if g is not None:
            fills.append(('gradient',))
        elif p is None or p.get('patternType') in (None, 'none'):
            fills.append(None)
        else:
            fills.append((p.get('patternType'),
                          colour_of(p.find(m('fgColor'))),
                          colour_of(p.find(m('bgColor')))))
        del fl

    def unpack(xf):
        fid = int(xf.get('fontId', '0'))
        flid = int(xf.get('fillId', '0'))
        face, cls, size, col = (fonts[fid] if fid < len(fonts) else (None, None, None, None))
        fill = fills[flid] if flid < len(fills) else None
        return (face, cls, size, col, fill)

    cellxfs = [unpack(xf) for xf in (st.find(m('cellXfs')) or [])]
    stylexfs = [unpack(xf) for xf in (st.find(m('cellStyleXfs')) or [])]

    normal = None
    cs = st.find(m('cellStyles'))
    if cs is not None:
        for s in cs:
            if (s.get('name') or '').lower() == 'normal' or s.get('builtinId') == '0':
                i = int(s.get('xfId', '0'))
                if i < len(stylexfs):
                    normal = stylexfs[i]
                break
    if normal is None and stylexfs:
        normal = stylexfs[0]
    return cellxfs, normal


PROPS = ('face', 'class', 'size', 'colour', 'fill')
TOTAL = [0, 0, 0, 0, 0]
CELLS = 0
RANGES = 0
WS_TOTAL = [0, 0, 0, 0, 0]
WS_CELLS = [0]
WS_RANGES = [0]
NORMAL_DIFFERS = []

for path in sys.argv[1:]:
    z = zipfile.ZipFile(path)
    name = os.path.basename(path)
    xfs, normal = styles(z)
    base = xfs[0] if xfs else (None, None, None, None, None)
    if normal is not None and normal != base:
        NORMAL_DIFFERS.append((name, base, normal))

    wb = ET.fromstring(z.read('xl/workbook.xml'))
    rels = ET.fromstring(z.read('xl/_rels/workbook.xml.rels'))
    tgt = {r.get('Id'): r.get('Target') for r in rels}
    for sh in wb.find(m('sheets')):
        t = tgt[sh.get('{%s}id' % R)]
        part = t.lstrip('/') if t.startswith('/') else ('xl/' + t if not t.startswith('xl/') else t)
        rp = os.path.join(os.path.dirname(part), '_rels', os.path.basename(part) + '.rels')
        if rp not in z.namelist():
            continue
        pts = []
        for r in ET.fromstring(z.read(rp)):
            if r.get('Type', '').endswith('/pivotTable'):
                raw = r.get('Target')
                pts.append((raw.lstrip('/') if raw.startswith('/')
                            else os.path.normpath(os.path.join(os.path.dirname(part), raw))).replace('\\', '/'))
        if not pts:
            continue

        ws = ET.fromstring(z.read(part))

        # column formats, then row formats, then the cell's own — the order XlsxCellFormats
        # resolves in, reversed.
        cols = {}
        cs = ws.find(m('cols'))
        if cs is not None:
            for c in cs:
                if c.get('style') is None:
                    continue
                lo, hi = int(c.get('min', '1')) - 1, int(c.get('max', '1')) - 1
                for cc in range(lo, min(hi, 16383) + 1):
                    cols[cc] = int(c.get('style'))

        rows = {}
        cells = {}
        for row in ws.find(m('sheetData')) or []:
            ri = int(row.get('r')) - 1 if row.get('r') else None
            if ri is None:
                continue
            if row.get('customFormat') in ('1', 'true') and row.get('s') is not None:
                rows[ri] = int(row.get('s'))
            for c in row:
                a = c.get('r')
                if not a:
                    continue
                mm = re.match(r'([A-Z]+)(\d+)', a)
                cells[(int(mm.group(2)) - 1, colnum(mm.group(1)))] = int(c.get('s', '0'))

        def resolve(r, c):
            if (r, c) in cells:
                return cells[(r, c)]
            if r in rows:
                return rows[r]
            if c in cols:
                return cols[c]
            return 0

        for pt in pts:
            root = ET.fromstring(z.read(pt))
            # the cache's source kind: an external cache is not imported as a data pilot at
            # all, so nothing is generated for it and nothing of it is cleared.
            prp = os.path.join(os.path.dirname(pt), '_rels', os.path.basename(pt) + '.rels')
            kind = '?'
            if prp in z.namelist():
                for r in ET.fromstring(z.read(prp)):
                    if r.get('Type', '').endswith('/pivotCacheDefinition'):
                        raw = r.get('Target')
                        cp = (raw.lstrip('/') if raw.startswith('/')
                              else os.path.normpath(os.path.join(os.path.dirname(pt), raw))).replace('\\', '/')
                        if cp in z.namelist():
                            src = ET.fromstring(z.read(cp)).find(m('cacheSource'))
                            kind = src.get('type') if src is not None else '?'
                        break
            loc = root.find(m('location'))
            c0, r0, c1, r1 = parse_ref(loc.get('ref'))
            n = [0, 0, 0, 0, 0]
            count = 0
            for r in range(r0, r1 + 1):
                for c in range(c0, c1 + 1):
                    count += 1
                    s = resolve(r, c)
                    v = xfs[s] if s < len(xfs) else base
                    for k in range(len(PROPS)):
                        if v[k] != base[k]:
                            n[k] += 1
            RANGES += 1
            CELLS += count
            for k in range(len(PROPS)):
                TOTAL[k] += n[k]
            if kind == 'worksheet':
                WS_RANGES[0] += 1
                WS_CELLS[0] += count
                for k in range(len(PROPS)):
                    WS_TOTAL[k] += n[k]
            print('%s\t%s\t%s\t%s\tref=%s\tcells=%d\tface=%d class=%d size=%d colour=%d fill=%d' % (
                name, sh.get('name'), os.path.basename(pt), kind, loc.get('ref'), count, *n))

print()
print('ranges=%d cells=%d' % (RANGES, CELLS))
for k, p in enumerate(PROPS):
    print('  %-7s differs in %5d of %5d cells  (%.2f %%)' % (
        p, TOTAL[k], CELLS, 100.0 * TOTAL[k] / CELLS if CELLS else 0.0))
print('worksheet-cached ranges only (the ones a grid is generated for): ranges=%d cells=%d'
      % (WS_RANGES[0], WS_CELLS[0]))
for k, p in enumerate(PROPS):
    print('  %-7s differs in %5d of %5d cells  (%.2f %%)' % (
        p, WS_TOTAL[k], WS_CELLS[0], 100.0 * WS_TOTAL[k] / WS_CELLS[0] if WS_CELLS[0] else 0.0))
print()
if NORMAL_DIFFERS:
    print('workbooks whose `Normal` cellStyleXf differs from cellXfs[0] on these four:')
    for n, b, o in NORMAL_DIFFERS:
        print('  %s\n    cellXfs[0]  = %s\n    Normal      = %s' % (n, b, o))
else:
    print('no workbook gives cellXfs[0] and the `Normal` cellStyleXf different values here')
