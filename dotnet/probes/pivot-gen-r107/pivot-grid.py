#!/usr/bin/env python3
"""Predict the border grid ScDPOutput generates for an OOXML pivot table.

A line-by-line mirror of `XlsxPivotGrid` in `dotnet/src/XRay.Office.Spreadsheets/Ooxml`, which is
itself a transcription of `sc/source/core/data/dpoutput.cxx`'s `CalcSizes`, `outputPageFields`,
`outputColumnHeaders`, `outputRowHeader`, `HeaderCell` and `ScDPOutputImpl::OutputDataArea`, fed
from the pivot part's own laid-out axis (`rowItems`/`colItems`) instead of from a DP source.

This file is the probe's copy, not the shipped code: it exists so the prediction can be scored
against the reference's `.fods` without a C# harness. Where the two disagree the C# is the
subject and this is the bug.

Usage: pivot-grid.py <workbook.xlsx>
"""
import sys, zipfile, re, os
import xml.etree.ElementTree as ET

MAIN = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
REL = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
def m(t): return '{%s}%s' % (MAIN, t)

INNER, OUTER = 20, 40
INDENT_TWIPS = 195

HASMEMBER, SUBTOTAL, CONTINUE = 1, 2, 4
CATEGORY, TITLE, RESULT = 'Category', 'Title', 'Result'


def colnum(s):
    n = 0
    for ch in s:
        n = n * 26 + ord(ch) - 64
    return n - 1


def colname(c):
    s = ''
    c += 1
    while c:
        c, r = divmod(c - 1, 26)
        s = chr(65 + r) + s
    return s


def parse_ref(ref):
    a, _, b = ref.partition(':')
    b = b or a
    ma = re.match(r'\$?([A-Z]+)\$?(\d+)', a)
    mb = re.match(r'\$?([A-Z]+)\$?(\d+)', b)
    return colnum(ma.group(1)), int(ma.group(2)) - 1, colnum(mb.group(1)), int(mb.group(2)) - 1


def flag(el, name, default=True):
    v = el.get(name)
    if v is None:
        return default
    return v not in ('0', 'false')


def children(parent, name):
    return list(parent.findall(m(name))) if parent is not None else []


class Grid:
    def __init__(self):
        self.cells = {}
        self.styles = {}
        self.indents = {}

    def edge(self, r, c, side, w):
        if r < 0 or c < 0:
            return
        self.cells.setdefault((r, c), [0, 0, 0, 0])['lrtb'.index(side)] = w


class Out:
    def __init__(self, tsc, tsr, dsc, dsr, tec, ter):
        self.tsc, self.tsr, self.dsc, self.dsr = tsc, tsr, dsc, dsr
        self.tec, self.ter = tec, ter
        self.cols, self.rows = [], []
        self.colseen, self.rowseen = set(), set()
        self.g = Grid()

    def add_row(self, r):
        if r not in self.rowseen:
            self.rowseen.add(r)
            self.rows.append(r)

    def add_col(self, c):
        if c not in self.colseen:
            self.colseen.add(c)
            self.cols.append(c)

    def block(self, c0, r0, c1, r1, hori=False):
        """OutputBlockFrame, dpoutput.cxx:217."""
        if c1 < c0 or r1 < r0:
            return
        L = OUTER if c0 == self.tsc else INNER
        T = OUTER if r0 == self.tsr else INNER
        R = OUTER if c1 == self.tec else INNER
        B = OUTER if r1 == self.ter else INNER
        for r in range(r0, r1 + 1):
            self.g.edge(r, c0, 'l', L)
            self.g.edge(r, c1, 'r', R)
        for c in range(c0, c1 + 1):
            self.g.edge(r0, c, 't', T)
            self.g.edge(r1, c, 'b', B)
        if not hori:
            return
        for r in range(r0, r1 + 1):
            for c in range(c0, c1 + 1):
                if r != r0:
                    self.g.edge(r, c, 't', INNER)
                if r != r1:
                    self.g.edge(r, c, 'b', INNER)

    def box(self, c, r, w):
        """lcl_SetFrame, dpoutput.cxx:297."""
        for s in 'lrtb':
            self.g.edge(r, c, s, w)

    def style(self, c0, r0, c1, r1, name):
        """lcl_SetStyleById, dpoutput.cxx:264 — last writer wins."""
        if c1 < c0 or r1 < r0:
            return
        for r in range(r0, r1 + 1):
            for c in range(c0, c1 + 1):
                self.g.styles[(r, c)] = name

    def data_area(self):
        """ScDPOutputImpl::OutputDataArea, dpoutput.cxx:127."""
        self.add_row(self.dsr)
        self.add_col(self.dsc)
        cols = sorted(self.cols + [self.tec + 1])
        rows = sorted(self.rows + [self.ter + 1])
        allrows = (self.ter - self.dsr + 2) == len(rows)
        for i in range(len(cols) - 1):
            if allrows:
                self.block(cols[i], rows[0], cols[i + 1] - 1, rows[-1] - 1, True)
                continue
            if i < len(cols) - 2:
                for k in range(i % 2, len(rows) - 2, 2):
                    self.block(cols[i], rows[k], cols[i + 1] - 1, rows[k + 1] - 1)
                if len(rows) >= 2:
                    self.block(cols[i], rows[-2], cols[i + 1] - 1, rows[-1] - 1)
            else:
                for k in range(len(rows) - 1):
                    self.block(cols[i], rows[k], cols[i + 1] - 1, rows[k + 1] - 1)
        if self.tsc != self.dsc:
            if self.tsr != self.dsr:
                self.block(self.tsc, self.tsr, self.dsc - 1, self.dsr - 1)
            self.block(self.tsc, self.dsr, self.dsc - 1, self.ter)
        self.block(self.dsc, self.tsr, self.tec, self.dsr - 1)


def read_axis(items, fields, count):
    """MemberResult flags per (field, position); ScDPResultMember::FillMemberResults."""
    flags = [[0] * count for _ in range(max(fields, 0))]
    for pos in range(min(count, len(items))):
        it = items[pos]
        repeated = max(0, min(int(it.get('r', 0)), fields))
        stated = max(len(it.findall(m('x'))), 1)
        depth = min(repeated + stated, fields)
        subtotal = (it.get('t', 'data') != 'data')
        inherited = min(repeated, max(depth - 1, 0)) if subtotal else repeated
        for f in range(inherited):
            flags[f][pos] |= CONTINUE
        if subtotal:
            if depth > 0:
                flags[depth - 1][pos] |= HASMEMBER | SUBTOTAL
        else:
            for f in range(repeated, depth):
                flags[f][pos] |= HASMEMBER
    return flags


def generatable(root, worksheet_cache, fhr, fdr, fdc, rowfields, colcount, rowitems, colitems):
    if not worksheet_cache:
        return 'cache is not a worksheet range'
    if not rowitems or not colitems:
        return 'no rowItems or no colItems'
    if fhr != 1:
        return 'firstHeaderRow=%d' % fhr
    if fdr != 1 + colcount:
        return 'firstDataRow=%d, Calc computes %d' % (fdr, 1 + colcount)
    if fdc != len(rowfields) or fdc == 0:
        return 'firstDataCol=%d, %d row fields' % (fdc, len(rowfields))
    datafields = len(children(root.find(m('dataFields')), 'dataField'))
    if datafields <= 1 and any(int(f.get('x', '-1')) < 0 for f in rowfields):
        return 'data-layout dimension on the row axis'
    if len(rowfields) == 1:
        return None
    pivotfields = children(root.find(m('pivotFields')), 'pivotField')
    for f in rowfields:
        x = int(f.get('x', '-1'))
        if x < 0 or x >= len(pivotfields):
            continue
        pf = pivotfields[x]
        if flag(pf, 'subtotalTop') and flag(pf, 'outline') and flag(pf, 'compact'):
            return 'compact row field'
    return None


def generate(root, worksheet_cache=True):
    """The grid, or (None, reason) when the pivot is one the reference would not lay out here."""
    loc = root.find(m('location'))
    if loc is None:
        return None, 'no location'
    tsc, tsr0, _, _ = parse_ref(loc.get('ref'))
    fhr = int(loc.get('firstHeaderRow', 1))
    fdr = int(loc.get('firstDataRow', 1))
    fdc = int(loc.get('firstDataCol', 0))

    rowfields = children(root.find(m('rowFields')), 'field')
    colfields = children(root.find(m('colFields')), 'field')
    rowitems = children(root.find(m('rowItems')), 'i')
    colitems = children(root.find(m('colItems')), 'i')
    datafields = children(root.find(m('dataFields')), 'dataField')

    C = len(colfields)
    if C == 1 and int(colfields[0].get('x', '0')) < 0 and len(datafields) <= 1:
        C = 0

    why = generatable(root, worksheet_cache, fhr, fdr, fdc, rowfields, C, rowitems, colitems)
    if why:
        return None, why

    show_drill = flag(root, 'showDrill', True)
    npage = len(children(root.find(m('pageFields')), 'pageField'))
    page_start = max(tsr0 - npage - 1, 0) if npage else tsr0
    tsr = page_start + npage + 1 if npage else tsr0
    msr = tsr + fhr
    dsr = tsr + fdr
    dsc = tsc + fdc
    nrow, ncol = len(rowitems), len(colitems)
    ter = dsr + nrow - 1 if nrow else dsr
    tec = dsc + ncol - 1 if ncol else dsc

    o = Out(tsc, tsr, dsc, dsr, tec, ter)
    o.page_start, o.npage = page_start, npage

    for f in range(npage):
        o.box(tsc + 1, page_start + f, INNER)

    cflags = read_axis(colitems, C, ncol)
    for f in range(C):
        if msr > tsr:
            o.box(dsc + f, tsr, INNER)
        rowpos = msr + f
        for n in range(ncol):
            colpos = dsc + n
            fl = cflags[f][n]
            if fl & SUBTOTAL:
                o.block(colpos, msr + f, colpos, dsr - 1)
                o.style(colpos, msr + f, colpos, dsr - 1, TITLE)
                o.style(colpos, dsr, colpos, ter, RESULT)
                o.add_col(colpos)
                continue
            if not (fl & HASMEMBER):
                continue
            end = n
            while end + 1 < ncol and (cflags[f][end + 1] & CONTINUE):
                end += 1
            endpos = dsc + end
            if f + 1 >= C:
                o.style(colpos, rowpos, colpos, dsr - 1, CATEGORY)
                continue
            if f + 2 == C:
                o.add_col(colpos)
                if colpos + 1 == endpos:
                    o.block(colpos, rowpos, endpos, rowpos + 1, True)
            else:
                o.block(colpos, rowpos, endpos, rowpos)
            o.style(colpos, rowpos, endpos, dsr - 1, CATEGORY)
        if f == 0 and C == 1 and msr > tsr:
            o.block(dsc, tsr, tec, msr - 1)

    R = len(rowfields)
    rflags = read_axis(rowitems, R, nrow)
    framed = [False] * nrow
    for f in range(R):
        o.box(tsc + f, dsr - 1, INNER)
        colpos = tsc + f
        for n in range(nrow):
            rowpos = dsr + n
            fl = rflags[f][n]
            if fl & SUBTOTAL:
                o.block(colpos, rowpos, dsc - 1, rowpos)
                o.style(colpos, rowpos, dsc - 1, rowpos, TITLE)
                o.style(dsc, rowpos, tec, rowpos, RESULT)
                o.add_row(rowpos)
                continue
            if not (fl & HASMEMBER):
                continue
            if f + 1 >= R:
                o.style(colpos, rowpos, dsc - 1, rowpos, CATEGORY)
                continue
            end = n
            while end + 1 < nrow and (rflags[f][end + 1] & CONTINUE):
                end += 1
            endrow = dsr + end
            o.add_row(rowpos)
            if not framed[n]:
                o.block(colpos, rowpos, tec, endrow)
                framed[n] = True
            o.block(colpos, rowpos, colpos, endrow)
            if f == R - 2:
                o.block(colpos + 1, rowpos, colpos + 1, endrow)
            o.style(colpos, rowpos, dsc - 1, endrow, CATEGORY)
            if show_drill:
                o.g.indents[(rowpos, colpos)] = INDENT_TWIPS

    o.data_area()
    return o, None


def worksheet_cached(z, part):
    rp = os.path.join(os.path.dirname(part), '_rels', os.path.basename(part) + '.rels')
    if rp not in z.namelist():
        return False
    for r in ET.fromstring(z.read(rp)):
        if not r.get('Type', '').endswith('/pivotCacheDefinition'):
            continue
        if r.get('TargetMode') == 'External':
            continue
        raw = r.get('Target')
        t = (raw.lstrip('/') if raw.startswith('/')
             else os.path.normpath(os.path.join(os.path.dirname(part), raw))).replace('\\', '/')
        if t not in z.namelist():
            continue
        src = ET.fromstring(z.read(t)).find(m('cacheSource'))
        if src is not None and src.get('type') == 'worksheet':
            return True
    return False


def pivots_by_sheet(z):
    wb = ET.fromstring(z.read('xl/workbook.xml'))
    rels = ET.fromstring(z.read('xl/_rels/workbook.xml.rels'))
    target = {r.get('Id'): r.get('Target') for r in rels}
    out = []
    for sh in wb.find(m('sheets')):
        t = target[sh.get('{%s}id' % REL)]
        part = t.lstrip('/') if t.startswith('/') else ('xl/' + t if not t.startswith('xl/') else t)
        rp = os.path.join(os.path.dirname(part), '_rels', os.path.basename(part) + '.rels')
        pivots = []
        if rp in z.namelist():
            for r in ET.fromstring(z.read(rp)):
                if not r.get('Type', '').endswith('/pivotTable'):
                    continue
                raw = r.get('Target')
                pt = (raw.lstrip('/') if raw.startswith('/')
                      else os.path.normpath(os.path.join(os.path.dirname(part), raw))).replace('\\', '/')
                pivots.append((pt, ET.fromstring(z.read(pt)), worksheet_cached(z, pt)))
        out.append((sh.get('name'), pivots))
    return out


def main():
    z = zipfile.ZipFile(sys.argv[1])
    for sheet, pivots in pivots_by_sheet(z):
        for part, root, cached in pivots:
            o, why = generate(root, cached)
            if o is None:
                print('## %s / %s\tDECLINED: %s' % (sheet, part, why))
                continue
            print('## %s / %s' % (sheet, part))
            for (r, c), e in sorted(o.g.cells.items()):
                if any(e):
                    print('%s%d\t%d\t%d\t%d\t%d\t%s\t%s' % (
                        colname(c), r + 1, e[0], e[1], e[2], e[3],
                        o.g.styles.get((r, c), '-'), o.g.indents.get((r, c), 0)))


if __name__ == '__main__':
    main()
