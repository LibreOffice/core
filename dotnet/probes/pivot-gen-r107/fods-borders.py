#!/usr/bin/env python3
"""Expand a sheet of a .fods into a cell grid and print each cell's four borders.

Usage: fods-borders.py <file.fods> <sheet-name> [maxrow] [maxcol]

Prints one line per cell that states any border, as
    <A1 address> TAB left TAB right TAB top TAB bottom TAB style TAB parent
so a generated grid can be compared against an implementation cell by cell.
"""
import sys, re
import xml.etree.ElementTree as ET

NS = {
    'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
    'table': 'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
    'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
    'fo': 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
    'text': 'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
}
for k, v in NS.items():
    ET.register_namespace(k, v)
def q(n):
    p, l = n.split(':')
    return '{%s}%s' % (NS[p], l)

def colname(c):
    s = ''
    c += 1
    while c:
        c, r = divmod(c - 1, 26)
        s = chr(65 + r) + s
    return s

def load_styles(root):
    """style name -> (borders dict, parent, family)"""
    out = {}
    for holder in root:
        if holder.tag not in (q('office:automatic-styles'), q('office:styles')):
            continue
        for st in holder.findall(q('style:style')):
            name = st.get(q('style:name'))
            parent = st.get(q('style:parent-style-name'))
            props = st.find(q('style:table-cell-properties'))
            b = {}
            if props is not None:
                for side, attr in (('all', 'fo:border'), ('left', 'fo:border-left'),
                                   ('right', 'fo:border-right'), ('top', 'fo:border-top'),
                                   ('bottom', 'fo:border-bottom')):
                    v = props.get(q(attr))
                    if v is not None:
                        b[side] = v
            out[name] = (b, parent, st.get(q('style:family')))
    return out

def resolve(styles, name):
    """Four sides, following parents."""
    chain = []
    seen = set()
    n = name
    while n and n in styles and n not in seen:
        seen.add(n)
        chain.append(n)
        n = styles[n][1]
    sides = {'left': None, 'right': None, 'top': None, 'bottom': None}
    for n in reversed(chain):
        b = styles[n][0]
        if 'all' in b:
            for s in sides:
                sides[s] = b['all']
        for s in sides:
            if s in b:
                sides[s] = b[s]
    return sides

def parent_of(styles, name):
    return styles.get(name, ({}, None, None))[1]

def main():
    path, sheet = sys.argv[1], sys.argv[2]
    maxrow = int(sys.argv[3]) if len(sys.argv) > 3 else 10 ** 9
    maxcol = int(sys.argv[4]) if len(sys.argv) > 4 else 10 ** 9
    root = ET.parse(path).getroot()
    styles = load_styles(root)
    body = root.find(q('office:body')).find(q('office:spreadsheet'))
    for table in body.findall(q('table:table')):
        if table.get(q('table:name')) != sheet:
            continue
        coldef = []
        for col in table.findall(q('table:table-column')):
            crep = int(col.get(q('table:number-columns-repeated'), 1))
            coldef += [col.get(q('table:default-cell-style-name'))] * min(crep, 4096)
        r = -1
        for row in table.findall(q('table:table-row')):
            rep = int(row.get(q('table:number-rows-repeated'), 1))
            cells = list(row.findall(q('table:table-cell'))) + list(row.findall(q('table:covered-table-cell')))
            cells = [c for c in row if c.tag in (q('table:table-cell'), q('table:covered-table-cell'))]
            for _ in range(min(rep, maxrow)):
                r += 1
                if r > maxrow:
                    return
                c = -1
                for cell in cells:
                    crep = int(cell.get(q('table:number-columns-repeated'), 1))
                    sn0 = cell.get(q('table:style-name'))
                    txt = ''.join(cell.itertext())
                    for _ in range(crep):
                        sn = sn0
                        c += 1
                        if c > maxcol:
                            break
                        rowdef = row.get(q('table:default-cell-style-name'))
                        eff = sn or rowdef or (coldef[c] if c < len(coldef) else None)
                        if eff is None:
                            continue
                        sn = eff
                        s = resolve(styles, sn)
                        if not any(s.values()):
                            continue
                        print('%s%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s' % (
                            colname(c), r + 1,
                            s['left'] or '-', s['right'] or '-', s['top'] or '-', s['bottom'] or '-',
                            sn, parent_of(styles, sn) or '-', txt[:24].replace('\t', ' ')))
        return
    sys.stderr.write('sheet %r not found\n' % sheet)
    sys.exit(2)

main()
