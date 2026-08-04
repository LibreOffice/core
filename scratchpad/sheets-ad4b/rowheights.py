#!/usr/bin/env python3
"""Pull per-row heights out of a flat ODS, as LibreOffice computed them."""
import sys, re
from xml.etree import ElementTree as ET

NS = {
    'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
    'table': 'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
    'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
    'fo': 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
    'text': 'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
}
def q(n):
    p, l = n.split(':')
    return '{%s}%s' % (NS[p], l)

def to_twips(v):
    m = re.match(r'^([-\d.]+)(in|cm|mm|pt|pc)$', v)
    if not m: return None
    x = float(m.group(1)); u = m.group(2)
    return {'in': 1440.0, 'cm': 1440/2.54, 'mm': 144/2.54, 'pt': 20.0, 'pc': 240.0}[u] * x

path = sys.argv[1]
want_sheet = sys.argv[2] if len(sys.argv) > 2 else None

styles = {}   # name -> (twips, use_optimal)
cur = None
sheet = None
row = 0
out = []
for ev, el in ET.iterparse(path, events=('start', 'end')):
    if ev == 'start':
        if el.tag == q('style:style') and el.get(q('style:family')) == 'table-row':
            cur = el.get(q('style:name'))
        elif el.tag == q('table:table'):
            sheet = el.get(q('table:name')); row = 0
        continue
    # end
    if el.tag == q('style:table-row-properties') and cur:
        h = el.get(q('style:row-height'))
        opt = el.get(q('style:use-optimal-row-height'))
        styles[cur] = (to_twips(h) if h else None, opt)
    elif el.tag == q('style:style') and cur:
        cur = None
    elif el.tag == q('table:table-row'):
        rep = int(el.get(q('table:number-rows-repeated'), '1'))
        sn = el.get(q('table:style-name'))
        h, opt = styles.get(sn, (None, None))
        if want_sheet is None or sheet == want_sheet:
            out.append((sheet, row, rep, h, opt))
        row += rep
        el.clear()
    elif el.tag == q('table:table'):
        el.clear()

for sheet, row, rep, h, opt in out:
    print('%s\t%d\t%d\t%s\t%s' % (sheet, row, rep, ('%.1f' % h) if h is not None else '-', opt))
