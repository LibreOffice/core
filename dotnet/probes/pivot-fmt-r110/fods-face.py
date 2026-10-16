#!/usr/bin/env python3
"""What did 26.2.4.2 itself resolve the pivot rectangle's cells to?

The census in `statedface.py` reads the workbook. This reads the reference binary's own
resolved view of the same cells, out of `soffice --convert-to fods`, and prints the resolved
font face, font size, font colour and cell background of every cell of a rectangle beside the
`Default` cell style's own values.

    fods-face.py <file.fods> <sheet> <A1:B2> [<A1:B2> ...]

A cell printed as `= Default` on all four resolved to exactly what the Default cell style
gives, which is what a cleared cell resolves to. Anything else is formatting the reference
kept.
"""
import sys, re
import xml.etree.ElementTree as ET

NS = {'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
      'table': 'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
      'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
      'text': 'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
      'fo': 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'}


def fq(n):
    p, l = n.split(':')
    return '{%s}%s' % (NS[p], l)


PROPS = (('face', fq('style:text-properties'), fq('style:font-name')),
         ('size', fq('style:text-properties'), fq('fo:font-size')),
         ('colour', fq('style:text-properties'), fq('fo:color')),
         ('weight', fq('style:text-properties'), fq('fo:font-weight')),
         ('fill', fq('style:table-cell-properties'), fq('fo:background-color')))


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


def colname(c):
    s = ''
    c += 1
    while c:
        c, r = divmod(c - 1, 26)
        s = chr(65 + r) + s
    return s


ROWHOLDERS = (fq('table:table-header-rows'), fq('table:table-row-group'), fq('table:table-rows'))
COLHOLDERS = (fq('table:table-header-columns'), fq('table:table-column-group'),
              fq('table:table-columns'))


def walk(node, want, holders):
    for child in node:
        if child.tag == want:
            yield child
        elif child.tag in holders:
            yield from walk(child, want, holders)


def main():
    path, sheet = sys.argv[1], sys.argv[2]
    root = ET.parse(path).getroot()

    raw = {}
    for holder in root:
        if holder.tag not in (fq('office:automatic-styles'), fq('office:styles')):
            continue
        for st in holder.findall(fq('style:style')):
            vals = {}
            for key, tag, attr in PROPS:
                e = st.find(tag)
                if e is not None and e.get(attr) is not None:
                    vals[key] = e.get(attr)
            raw[st.get(fq('style:name'))] = (vals, st.get(fq('style:parent-style-name')))

    cache = {}

    def resolve(name):
        if name in cache:
            return cache[name]
        chain, seen, n = [], set(), name
        while n and n in raw and n not in seen:
            seen.add(n)
            chain.append(n)
            n = raw[n][1]
        out = {}
        for n in reversed(chain):
            out.update(raw[n][0])
        cache[name] = out
        return out

    default = resolve('Default')
    print('Default: ' + '  '.join('%s=%s' % (k, default.get(k)) for k, _, _ in PROPS))

    body = root.find(fq('office:body')).find(fq('office:spreadsheet'))
    table = next(t for t in body.findall(fq('table:table'))
                 if t.get(fq('table:name')) == sheet)

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
                    grid[(r, c)] = resolve(eff) if eff else default

    keys = [k for k, _, _ in PROPS]
    agree = {k: 0 for k in keys}
    total = 0
    for ref in sys.argv[3:]:
        c0, r0, c1, r1 = parse_ref(ref)
        print('\n%s  %s' % (sheet, ref))
        for r in range(r0, r1 + 1):
            line = []
            for c in range(c0, c1 + 1):
                v = grid.get((r, c), default)
                total += 1
                diff = []
                for k in keys:
                    if v.get(k) == default.get(k):
                        agree[k] += 1
                    else:
                        diff.append('%s=%s' % (k, v.get(k)))
                line.append('%s%d:%s' % (colname(c), r + 1, ','.join(diff) if diff else '='))
            print('  ' + '  '.join(line))
    print('\ncells=%d' % total)
    for k in keys:
        print('  %-7s resolved to the Default value in %d of %d' % (k, agree[k], total))


main()
