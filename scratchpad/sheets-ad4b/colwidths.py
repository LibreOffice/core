#!/usr/bin/env python3
"""Pull per-column widths out of a flat ODS."""
import sys, re
from xml.etree import ElementTree as ET

NS = {
    'table': 'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
    'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
}
def q(n):
    p, l = n.split(':')
    return '{%s}%s' % (NS[p], l)

def to_twips(v):
    m = re.match(r'^([-\d.]+)(in|cm|mm|pt|pc)$', v)
    if not m: return None
    x = float(m.group(1)); u = m.group(2)
    return {'in': 1440.0, 'cm': 1440/2.54, 'mm': 144/2.54, 'pt': 20.0, 'pc': 240.0}[u] * x

styles = {}
cur = None
sheet = None
col = 0
for ev, el in ET.iterparse(sys.argv[1], events=('start', 'end')):
    if ev == 'start':
        if el.tag == q('style:style') and el.get(q('style:family')) == 'table-column':
            cur = el.get(q('style:name'))
        elif el.tag == q('table:table'):
            sheet = el.get(q('table:name')); col = 0
        continue
    if el.tag == q('style:table-column-properties') and cur:
        w = el.get(q('style:column-width'))
        styles[cur] = to_twips(w) if w else None
    elif el.tag == q('style:style') and cur:
        cur = None
    elif el.tag == q('table:table-column'):
        rep = int(el.get(q('table:number-columns-repeated'), '1'))
        w = styles.get(el.get(q('table:style-name')))
        if rep < 500:
            for i in range(rep):
                print('%s\t%d\t%s' % (sheet, col + i, ('%.1f' % w) if w else '-'))
        col += rep
        el.clear()
    elif el.tag == q('table:table-row'):
        el.clear()
