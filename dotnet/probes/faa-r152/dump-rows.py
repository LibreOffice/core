#!/usr/bin/env python3
"""Per-row markup of one body-level table: trHeight, and each cell's stated top/bottom border."""
import sys, xml.etree.ElementTree as ET
W='{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
def q(t): return W+t
root = ET.parse(sys.argv[1]).getroot()
tbl = list(root.find(q('body')))[int(sys.argv[2])]
print('row\ttrHeight\thRule\ttops\tbottoms\ttext')
for ri, tr in enumerate(tbl.findall(q('tr'))):
    pr = tr.find(q('trPr'))
    h = hr = ''
    if pr is not None:
        th = pr.find(q('trHeight'))
        if th is not None:
            h = th.get(q('val')) or ''
            hr = th.get(q('hRule')) or ''
    tops, bots = [], []
    for tc in tr.findall(q('tc')):
        tcpr = tc.find(q('tcPr'))
        b = tcpr.find(q('tcBorders')) if tcpr is not None else None
        def side(name):
            if b is None: return '-'
            e = b.find(q(name))
            if e is None: return '-'
            return f"{e.get(q('val'))}/{e.get(q('sz'))}"
        tops.append(side('top')); bots.append(side('bottom'))
    txt = ''.join(tr.itertext()).replace('\n', ' ')[:52]
    print(f'{ri}\t{h}\t{hr}\t{",".join(tops)}\t{",".join(bots)}\t{txt}')
