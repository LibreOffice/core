#!/usr/bin/env python3
"""Score the predicted pivot grid against 26.2.4.2's own resolved view, both ways.

    check-grid.py <workbook.xlsx> <workbook.fods>

For every pivot the predictor accepts, compares every edge of every cell of the pivot's own
rectangle — page-field rows included — against the `.fods` the reference writes for the same
workbook. Both directions are counted: an edge the reference states and the prediction does not
is a MISS, one the prediction states and the reference does not is EXTRA, and a stated width
that differs is WRONG. Scoring only the predicted edges, as this script's first draft did, is a
precision that a prediction of nothing scores 100 % on.

Cell styles are compared too, by the parent name the reference gives the cell's automatic style:
`Pivot Table Category`, `Title`, `Result` carry formatting, the rest do not.

This is r107's script with one correction, and the correction is what this round is about: rows
are not always direct children of `<table:table>`, and reading only the direct children shifts
every row after a `<table:table-header-rows>` up by one.  Eleven of the fifty-eight sheets in the
eleven pivot-bearing workbooks hold such a row, and three of them carry the three pivots r107
declined — which is why the hidden-header layout looked unmodellable and is not.
"""
import sys, os, re, zipfile
import importlib.util
import xml.etree.ElementTree as ET

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('pg', os.path.join(HERE, 'pivot-grid.py'))
pg = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pg)

FODNS = {'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
         'table': 'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
         'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
         'fo': 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'}
def fq(n):
    p, l = n.split(':')
    return '{%s}%s' % (FODNS[p], l)

SIDES = (('all', 'fo:border'), ('left', 'fo:border-left'), ('right', 'fo:border-right'),
         ('top', 'fo:border-top'), ('bottom', 'fo:border-bottom'))


def twips(v):
    if not v or v.startswith('none'):
        return 0
    mm = re.match(r'([0-9.]+)(pt|in|cm|mm)', v)
    if not mm:
        return -1
    n, u = float(mm.group(1)), mm.group(2)
    pt = n if u == 'pt' else n * 72 if u == 'in' else n * 72 / 2.54 if u == 'cm' else n * 72 / 25.4
    return int(round(pt * 20))


def inches(v):
    if not v:
        return 0
    mm = re.match(r'([0-9.]+)(pt|in|cm|mm)', v)
    if not mm:
        return 0
    n, u = float(mm.group(1)), mm.group(2)
    pt = n if u == 'pt' else n * 72 if u == 'in' else n * 72 / 2.54 if u == 'cm' else n * 72 / 25.4
    return int(round(pt * 20))


def oracle(path, sheet):
    """(row, col) -> (four edge widths in twips, generated style name or '-', indent twips)."""
    root = ET.parse(path).getroot()
    styles = {}
    for holder in root:
        if holder.tag not in (fq('office:automatic-styles'), fq('office:styles')):
            continue
        for st in holder.findall(fq('style:style')):
            props = st.find(fq('style:table-cell-properties'))
            para = st.find(fq('style:paragraph-properties'))
            b = {}
            if props is not None:
                for side, attr in SIDES:
                    v = props.get(fq(attr))
                    if v is not None:
                        b[side] = v
            indent = para.get(fq('fo:margin-left')) if para is not None else None
            styles[st.get(fq('style:name'))] = (b, st.get(fq('style:parent-style-name')), indent)

    cache = {}
    def resolve(name):
        if name in cache:
            return cache[name]
        chain, seen, n = [], set(), name
        while n and n in styles and n not in seen:
            seen.add(n)
            chain.append(n)
            n = styles[n][1]
        sides = {'left': None, 'right': None, 'top': None, 'bottom': None}
        indent = None
        for n in reversed(chain):
            b = styles[n][0]
            if 'all' in b:
                for s in sides:
                    sides[s] = b['all']
            for s in sides:
                if s in b:
                    sides[s] = b[s]
            if styles[n][2] is not None:
                indent = styles[n][2]
        gen = '-'
        for n in chain:
            if n.startswith('Pivot_20_Table_20_'):
                gen = n[len('Pivot_20_Table_20_'):]
                break
        if gen not in ('Category', 'Title', 'Result'):
            gen = '-'
        v = ([twips(sides['left']), twips(sides['right']), twips(sides['top']), twips(sides['bottom'])],
             gen, inches(indent))
        cache[name] = v
        return v

    # Rows and columns are not always direct children of <table:table>: print titles put
    # them inside <table:table-header-rows> and outlines inside <table:table-row-group>,
    # in document order.  Walking only the direct children silently drops those rows and
    # shifts every row after them up — 033_Event_planning_tracker's own pivot sheet has one
    # such header row, and reading it as a direct-child-only sequence is what made
    # firstHeaderRow="0" look unmodellable in r107.
    ROWHOLDERS = (fq('table:table-header-rows'), fq('table:table-row-group'),
                  fq('table:table-rows'))
    COLHOLDERS = (fq('table:table-header-columns'), fq('table:table-column-group'),
                  fq('table:table-columns'))

    def walk(node, want, holders):
        for child in node:
            if child.tag == want:
                yield child
            elif child.tag in holders:
                yield from walk(child, want, holders)

    body = root.find(fq('office:body')).find(fq('office:spreadsheet'))
    for table in body.findall(fq('table:table')):
        if table.get(fq('table:name')) != sheet:
            continue
        coldef = []
        for col in walk(table, fq('table:table-column'), COLHOLDERS):
            crep = min(int(col.get(fq('table:number-columns-repeated'), 1)), 1024)
            coldef += [col.get(fq('table:default-cell-style-name'))] * crep
        grid = {}
        r = -1
        for row in walk(table, fq('table:table-row'), ROWHOLDERS):
            rep = min(int(row.get(fq('table:number-rows-repeated'), 1)), 4096)
            rowdef = row.get(fq('table:default-cell-style-name'))
            cells = [c for c in row
                     if c.tag in (fq('table:table-cell'), fq('table:covered-table-cell'))]
            for _ in range(rep):
                r += 1
                c = -1
                for cell in cells:
                    crep = min(int(cell.get(fq('table:number-columns-repeated'), 1)), 1024)
                    sn0 = cell.get(fq('table:style-name'))
                    for _ in range(crep):
                        c += 1
                        eff = sn0 or rowdef or (coldef[c] if c < len(coldef) else None)
                        if eff is None:
                            continue
                        grid[(r, c)] = resolve(eff)
        return grid
    return None


CARRIES = {'Category': ('left', False), 'Title': ('left', True), 'Result': (None, True)}


def main():
    xlsx, fods = sys.argv[1], sys.argv[2]
    z = zipfile.ZipFile(xlsx)
    name = os.path.basename(xlsx)
    grand = [0, 0, 0, 0]
    for sheet, pivots in pg.pivots_by_sheet(z):
        if not pivots:
            continue
        cells = None
        for part, root, cached in pivots:
            o, why = pg.generate(root, cached)
            if o is None:
                print('%s\t%s\t%s\tDECLINED\t%s' % (name, sheet, os.path.basename(part), why))
                continue
            if cells is None:
                cells = oracle(fods, sheet)
                if cells is None:
                    print('%s\t%s\tSHEET NOT IN FODS' % (name, sheet))
                    break
            miss = extra = wrong = agree = 0
            smiss = sextra = swrong = sagree = 0
            imiss = iextra = 0
            bad = []
            r0 = o.page_start if o.npage else o.tsr
            # A workbook can give every cell of a sheet a left margin of its own, and Calc's
            # ScIndentItem replaces rather than adds: 049_Expenses_calculator states 210 twips on
            # all fifty-four cells of its pivot's rectangle. Compare against the rectangle's own
            # floor, so the quantity scored is the indent the pivot adds.
            floor = min((cells.get((r, c), ([0, 0, 0, 0], '-', 0))[2]
                         for r in range(r0, o.ter + 1) for c in range(o.tsc, o.tec + 1)),
                        default=0)
            for r in range(r0, o.ter + 1):
                for c in range(o.tsc, o.tec + 1):
                    got = cells.get((r, c), ([0, 0, 0, 0], '-', 0))
                    pred = o.g.cells.get((r, c), [0, 0, 0, 0])
                    for k in range(4):
                        p, g = pred[k], got[0][k]
                        if p == g or (g > 0 and p > 0 and abs(g - p) <= 1):
                            if p:
                                agree += 1
                            continue
                        tag = 'MISS' if p == 0 else ('EXTRA' if g == 0 else 'WRONG')
                        if tag == 'MISS':
                            miss += 1
                        elif tag == 'EXTRA':
                            extra += 1
                        else:
                            wrong += 1
                        if len(bad) < 8:
                            bad.append('%s%d.%s %s p=%d r=%d' % (
                                pg.colname(c), r + 1, 'lrtb'[k], tag, p, g))
                    ps = o.g.styles.get((r, c), '-')
                    gs = got[1]
                    if ps == gs:
                        if ps != '-':
                            sagree += 1
                    elif ps == '-':
                        smiss += 1
                    elif gs == '-':
                        sextra += 1
                    else:
                        swrong += 1
                    pi, gi = o.g.indents.get((r, c), 0), max(got[2] - floor, 0)
                    if pi != gi:
                        if pi == 0:
                            imiss += 1
                        else:
                            iextra += 1
            print('%s\t%s\t%s\tedges agree=%d MISS=%d EXTRA=%d WRONG=%d\t'
                  'styles agree=%d miss=%d extra=%d wrong=%d\tindent miss=%d extra=%d\t%s'
                  % (name, sheet, os.path.basename(part), agree, miss, extra, wrong,
                     sagree, smiss, sextra, swrong, imiss, iextra, '; '.join(bad)))
            grand[0] += agree
            grand[1] += miss
            grand[2] += extra
            grand[3] += wrong
    print('TOTAL\t%s\tagree=%d MISS=%d EXTRA=%d WRONG=%d' % (name, *grand))


main()
