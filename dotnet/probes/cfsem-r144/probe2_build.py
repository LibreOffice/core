"""Probe 2: conditional-format firing semantics, measured by rendering to PDF
and reading the painted fills.  Each probed cell carries its own address as
text so fills can be mapped back to cells."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkxlsx import build

cells = {}
names = [
    ('NameColRel', 'Sheet1!A$1'),     # base A1: col offset 0, row abs 1
    ('NameRowRel', 'Sheet1!$A1'),     # base A1: col abs A, row offset 0
    ('NameCol',    'COLUMN()'),
]

# row 1 pattern: B1=1 C1=0 D1=1 E1=0
for c, v in (('B', 1), ('C', 0), ('D', 1), ('E', 0)):
    cells['%s1' % c] = v
# column A pattern for row-relative test
for r, v in ((13, 1), (14, 0), (15, 1), (16, 0)):
    cells['A%d' % r] = v

probe_rows = {
    2:  '=3',                 # non-zero constant, not TRUE
    3:  '=0',
    4:  '=-1',
    5:  '=0.5',
    6:  '="x"',               # string result
    7:  '=MOD(COLUMN(),2)',
    8:  '=B$1=1',             # relative column, base is sqref top-left B8
    9:  '=NameColRel=1',      # name with relative column
    10: '=NameCol',           # name wrapping COLUMN()
    11: '=1/0',               # error result
    12: '=""',                # empty string result
}
cfs = [('B%d:E%d' % (r, r), [f.lstrip('=')]) for r, f in sorted(probe_rows.items())]
for r in probe_rows:
    for c in 'BCDE':
        cells['%s%d' % (c, r)] = '%s%d' % (c, r)

# vertical: ROW() and a row-relative name, in column G rows 13..16
cfs.append(('G13:G16', ['MOD(ROW(),2)']))
cfs.append(('H13:H16', ['NameRowRel=1']))
for r in (13, 14, 15, 16):
    cells['G%d' % r] = 'G%d' % r
    cells['H%d' % r] = 'H%d' % r

colors = []
for i in range(len(cfs)):
    colors.append('FF%02X%02X%02X' % ((i * 37) % 200 + 40, (i * 91) % 200 + 20, (i * 53) % 200 + 30))

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'probe2.xlsx')
build(out, cells, names, cfs, colors, dim='A1:J20')
print(out)
for i, (sq, f) in enumerate(cfs):
    print('dxf%-2d %-10s %-22s %s' % (i, sq, f[0], colors[i]))
