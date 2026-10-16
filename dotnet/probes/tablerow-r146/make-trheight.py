#!/usr/bin/env python3
"""trheight.docx -- does a `w:trHeight` floor charge the STATED top border or the RESOLVED one?

`lcl_CalcMinRowHeight` (tabfrm.cxx:5093) adds `lcl_GetTopSpace(row)` -- the row's OWN stated top
line -- to the floor, while `SwRowFrame::Format` (:5399-5404) sets the cell print margin from
`max(own top, the row above's bottom line)`.  If those really are two different quantities, then
a 3 pt edge STATED BY THE ROW ABOVE raises the floor by nothing and a 3 pt edge STATED BY THE ROW
ITSELF raises it by 3 pt, although the drawn line is identical in the two arms.
"""
import sys, os, importlib.util
here = os.path.dirname(os.path.abspath(__file__))
sys.argv = [sys.argv[0], os.path.join(here, 'fixtures')]
spec = importlib.util.spec_from_file_location('mp', os.path.join(here, 'make-probes.py'))
mp = importlib.util.module_from_spec(spec); spec.loader.exec_module(mp)

TRH = '<w:trPr><w:cantSplit/><w:trHeight w:val="400" w:hRule="atLeast"/></w:trPr>'
PLAIN = '<w:trPr><w:cantSplit/></w:trPr>'


def arm(name, bottom_of_0, top_of_1):
    """Row 0 plain; row 1 carries `w:trHeight atLeast 400` (20 pt); row 2 plain."""
    grid = ''.join(f'<w:gridCol w:w="{w}"/>' for w in mp.COL2)
    specs = [
        (PLAIN, [(mp.OUT, bottom_of_0, mp.OUT, mp.OUT)] * 2),
        (TRH,   [(top_of_1, mp.NIL, mp.OUT, mp.OUT)] * 2),
        (PLAIN, [(mp.NIL, mp.OUT, mp.OUT, mp.OUT)] * 2),
    ]
    body = ''
    for i, (trpr, row) in enumerate(specs):
        cs = ''.join(mp.cell(mp.COL2[c], f'{name}{i}{c}', *row[c]) for c in range(2))
        body += f'<w:tr>{trpr}{cs}</w:tr>'
    t = (f'<w:tbl><w:tblPr><w:tblW w:w="{sum(mp.COL2)}" w:type="dxa"/>'
         f'<w:tblLayout w:type="fixed"/></w:tblPr><w:tblGrid>{grid}</w:tblGrid>{body}</w:tbl>')
    return mp.page(name, t)


arms = [
    arm('R0', mp.NIL, mp.NIL),   # no edge at boundary 1 at all
    arm('RA', mp.X,   mp.NIL),   # 3 pt stated by the row ABOVE
    arm('RB', mp.NIL, mp.X),     # 3 pt stated by the trHeight row ITSELF
    arm('RC', mp.X,   mp.X),     # both
    arm('RD', mp.X,   mp.S),     # row above 3 pt, trHeight row 0.5 pt
]
mp.write(os.path.join(sys.argv[1], 'trheight.docx'), ''.join(arms))
