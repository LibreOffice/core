#!/usr/bin/env python3
"""Does not clearing the pivot's range of the workbook's own alignment and indent show?

    clearing-census.py <workbook.xlsx> <workbook.fods>

`PivotTable::finalizeImport` clears the stated range of `HARDATTR | STYLES` before the data
pilot is created (`sc/source/filter/oox/pivottablebuffer.cxx`:1331-1336) and `ScDPOutput::Output`
then deletes the computed range outright (`dpoutput.cxx`:1226), so what the reference draws on a
pivot cell is only what `ScDPOutput` puts back. This tree lays the generated styles over the
workbook's own formatting instead of replacing it, which differs wherever the workbook states
something the generated style does not.

For every cell of every pivot rectangle the predictor accepts this prints three quantities and
counts the cells where the last two disagree:

  stated    the horizontal alignment and indent the workbook's own `cellXfs` give the cell,
            through the cell's own `s`, its row's when the row is `customFormat`, or its
            column's — the resolution `XlsxCellFormats` performs.
  merged    what this tree ends up with: the generated style where there is one (Category and
            Title justify left; none of them state an indent, which the drill step supplies
            separately), and the stated value everywhere else.  With `--cleared` the base is the
            sheet's own default `cellXf` instead of the stated value, which is what the shipped
            code does after this round: `clearContents` takes a cell back to the Default cell
            style rather than to nothing, so what the workbook's default states survives.
  reference the alignment and indent 26.2.4.2's own `.fods` of the same workbook gives the
            cell, which is the cleared-then-regenerated answer.

A cell counts as a difference only when `merged` and `reference` disagree; where the workbook
states nothing the two are the same and the clearing is invisible.
"""
import sys, os, re, zipfile
import importlib.util
import xml.etree.ElementTree as ET

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location('pg', os.path.join(HERE, 'pivot-grid.py'))
pg = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(pg)

_src = open(os.path.join(HERE, 'check-grid.py'), encoding='utf-8').read().replace('\nmain()\n', '\n')
_cg = {'__name__': 'cg', '__file__': os.path.join(HERE, 'check-grid.py')}
exec(compile(_src, 'check-grid.py', 'exec'), _cg)

M = pg.MAIN
REL = pg.REL
m = pg.m

FODNS = _cg['FODNS']
fq = _cg['fq']

#  The generated styles that state a horizontal alignment: Category and Title justify left,
#  Result states only a weight (lcl_SetStyleById, dpoutput.cxx:264-296).
GENERATED_LEFT = ('Category', 'Title')


def cell_formats(z):
    """cellXfs index -> (horizontal alignment or None, indent in units of three characters)."""
    st = ET.fromstring(z.read('xl/styles.xml'))
    out = []
    xfs = st.find(m('cellXfs'))
    for xf in (xfs if xfs is not None else []):
        al = xf.find(m('alignment'))
        h = al.get('horizontal') if al is not None else None
        ind = int(al.get('indent', '0')) if al is not None else 0
        applied = xf.get('applyAlignment') not in ('0', 'false')
        out.append(((h if h not in (None, 'general') else None) if applied else None,
                    ind if applied else 0))
    return out


def sheet_states(z, part, xfs):
    """(row, col) -> (alignment, indent) from the worksheet's own cells, rows and columns."""
    ws = ET.fromstring(z.read(part))
    coldef = {}
    cols = ws.find(m('cols'))
    for col in (cols if cols is not None else []):
        s = int(col.get('style', '0'))
        if s == 0 or col.get('customFormat') in ('0', 'false'):
            pass
        for c in range(int(col.get('min', '1')) - 1, min(int(col.get('max', '1')), 16384)):
            coldef[c] = s
    states = {}
    rowdef = {}
    data = ws.find(m('sheetData'))
    for row in (data if data is not None else []):
        r = int(row.get('r', '0')) - 1
        if row.get('customFormat') not in (None, '0', 'false'):
            rowdef[r] = int(row.get('s', '0'))
        for c in row:
            a = c.get('r')
            if not a:
                continue
            mm = re.match(r'([A-Z]+)(\d+)', a)
            states[(int(mm.group(2)) - 1, pg.colnum(mm.group(1)))] = int(c.get('s', '0'))
    return states, rowdef, coldef


def resolve(states, rowdef, coldef, xfs, r, c):
    s = states.get((r, c))
    if s is None:
        s = rowdef.get(r)
    if s is None:
        s = coldef.get(c)
    if s is None or s >= len(xfs):
        return None, 0
    return xfs[s]


def fods_alignment(path, sheet):
    """(row, col) -> (fo:text-align or None, fo:margin-left in twips)."""
    root = ET.parse(path).getroot()
    styles = {}
    for holder in root:
        if holder.tag not in (fq('office:automatic-styles'), fq('office:styles')):
            continue
        for stl in holder.findall(fq('style:style')):
            para = stl.find(fq('style:paragraph-properties'))
            cellp = stl.find(fq('style:table-cell-properties'))
            align = None
            if para is not None:
                align = para.get(fq('fo:text-align'))
            indent = para.get(fq('fo:margin-left')) if para is not None else None
            styles[stl.get(fq('style:name'))] = (align, indent,
                                                 stl.get(fq('style:parent-style-name')),
                                                 cellp)

    cache = {}

    def chain(name):
        if name in cache:
            return cache[name]
        seen, order, n = set(), [], name
        while n and n in styles and n not in seen:
            seen.add(n)
            order.append(n)
            n = styles[n][2]
        align, indent = None, None
        for n in reversed(order):
            if styles[n][0] is not None:
                align = styles[n][0]
            if styles[n][1] is not None:
                indent = styles[n][1]
        v = (align, _cg['inches'](indent))
        cache[name] = v
        return v

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
            rd = row.get(fq('table:default-cell-style-name'))
            cells = [c for c in row
                     if c.tag in (fq('table:table-cell'), fq('table:covered-table-cell'))]
            for _ in range(rep):
                r += 1
                c = -1
                for cell in cells:
                    crep = min(int(cell.get(fq('table:number-columns-repeated'), 1)), 1024)
                    sn = cell.get(fq('table:style-name'))
                    for _ in range(crep):
                        c += 1
                        eff = sn or rd or (coldef[c] if c < len(coldef) else None)
                        if eff is not None:
                            grid[(r, c)] = chain(eff)
        return grid
    return None


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    clearing = '--cleared' in sys.argv[1:]
    xlsx, fods = args[0], args[1]
    z = zipfile.ZipFile(xlsx)
    name = os.path.basename(xlsx)
    xfs = cell_formats(z)

    wb = ET.fromstring(z.read('xl/workbook.xml'))
    rels = ET.fromstring(z.read('xl/_rels/workbook.xml.rels'))
    target = {r.get('Id'): r.get('Target') for r in rels}
    parts = {}
    for sh in wb.find(m('sheets')):
        t = target[sh.get('{%s}id' % REL)]
        parts[sh.get('name')] = (t.lstrip('/') if t.startswith('/')
                                 else ('xl/' + t if not t.startswith('xl/') else t))

    grand = [0, 0, 0, 0]
    for sheet, pivots in pg.pivots_by_sheet(z):
        if not pivots:
            continue
        states = rowdef = coldef = None
        cells = None
        for part, root, cached in pivots:
            o, why = pg.generate(root, cached)
            if o is None:
                continue
            if states is None:
                states, rowdef, coldef = sheet_states(z, parts[sheet], xfs)
                cells = fods_alignment(fods, sheet)
                if cells is None:
                    print('%s\t%s\tSHEET NOT IN FODS' % (name, sheet))
                    break
            r0 = o.page_start if o.npage else o.tsr
            #  What a cleared cell falls back to: the workbook's default cellXf, which is what
            #  every cell stating no `s` of its own already resolves through.
            default = xfs[0] if xfs else (None, 0)
            n = [0, 0, 0, 0]       # cells, stated something, merged != reference, of those indent
            for r in range(r0, o.ter + 1):
                for c in range(o.tsc, o.tec + 1):
                    n[0] += 1
                    align, ind = resolve(states, rowdef, coldef, xfs, r, c)
                    if align or ind:
                        n[1] += 1
                    gen = o.g.styles.get((r, c), '-')
                    base_align, base_indent = (default if clearing else (align, ind))
                    merged_align = 'left' if gen in GENERATED_LEFT else base_align
                    merged_indent = (o.g.indents.get((r, c), 0)
                                     or (1 if base_indent else 0))
                    ra, ri = cells.get((r, c), (None, 0))
                    #  ODF spells the two justifications `start` and `end`.
                    ra = {'start': 'left', 'end': 'right', None: None}.get(ra, ra)
                    same_align = (merged_align or 'general') == (ra or 'general')
                    if not same_align:
                        n[2] += 1
                    if (merged_indent > 0) != (ri > 0):
                        n[3] += 1
            print('%s\t%s\t%s\tcells=%d stated=%d align-differs=%d indent-differs=%d'
                  % (name, sheet, os.path.basename(part), *n))
            for k in range(4):
                grand[k] += n[k]
    print('TOTAL\t%s\tcells=%d stated=%d align-differs=%d indent-differs=%d' % (name, *grand))


main()
