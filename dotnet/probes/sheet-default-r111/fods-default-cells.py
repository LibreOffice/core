#!/usr/bin/env python3
"""Which cells did 26.2.4.2 resolve to the document's `Default` cell style?

A cell's style is its own `table:style-name`, else its row's `table:default-cell-style-name`,
else its column's.  Prints, per sheet, how many *non-empty* cells land on `Default` and where
the first few of them are — the cells whose drawn format the sheet default decides.
"""
import sys
import xml.etree.ElementTree as ET

T = 'urn:oasis:names:tc:opendocument:xmlns:table:1.0'
O = 'urn:oasis:names:tc:opendocument:xmlns:office:1.0'
X = 'urn:oasis:names:tc:opendocument:xmlns:text:1.0'
def t(n): return '{%s}%s' % (T, n)
def o(n): return '{%s}%s' % (O, n)


def colname(i):
    s = ''
    i += 1
    while i:
        i, r = divmod(i - 1, 26)
        s = chr(65 + r) + s
    return s


def main(path):
    root = ET.parse(path).getroot()
    body = root.find(o('body')).find(o('spreadsheet'))
    for table in body.findall(t('table')):
        name = table.get(t('name'))
        colstyle = []
        for c in table.findall(t('table-column')):
            rep = int(c.get(t('number-columns-repeated'), '1'))
            colstyle += [c.get(t('default-cell-style-name'))] * min(rep, 20000)
        hits = []
        empty_hits = 0
        r = 0
        for row in table.iter(t('table-row')):
            rrep = int(row.get(t('number-rows-repeated'), '1'))
            rowdef = row.get(t('default-cell-style-name'))
            ci = 0
            for cell in row:
                if cell.tag not in (t('table-cell'), t('covered-table-cell')):
                    continue
                crep = int(cell.get(t('number-columns-repeated'), '1'))
                st = cell.get(t('style-name')) or rowdef or (
                    colstyle[ci] if ci < len(colstyle) else None)
                has = (cell.get(o('value-type')) is not None
                       or cell.find('{%s}p' % X) is not None)
                if st == 'Default' or st is None:
                    if has:
                        for k in range(min(crep, 50)):
                            hits.append('%s%d' % (colname(ci + k), r + 1))
                    else:
                        empty_hits += min(crep, 50) * min(rrep, 50)
                ci += crep
            r += rrep
        if hits or empty_hits:
            print('  %-40s non-empty on Default: %4d   %s'
                  % (name, len(hits), ' '.join(hits[:14]) + (' …' if len(hits) > 14 else '')))


if __name__ == '__main__':
    for p in sys.argv[1:]:
        print('=====', p.split('/')[-1])
        main(p)
