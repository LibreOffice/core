"""Probe 3: which cell is the base when the sqref holds several ranges, and
what happens when rebasing pushes a relative reference off-sheet."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkxlsx import build

cells = {}
# A8=1, A10=0, A12=1  -> discriminates base B12 (ScAddress order) from B10
# (componentwise minimum) for a rule written as $A10.
for r, v in ((8, 1), (10, 0), (12, 1), (2, 1), (4, 0), (6, 1)):
    cells['A%d' % r] = v

cfs = [
    # (1) two ranges, listed D-first. componentwise min = B10, ScAddress min = B12
    ('D10 B12', ['$A10=1']),
    # (2) same two ranges listed in the other order
    ('B22 D20', ['$A20=1']),
    # (3) three ranges, the smallest column last
    ('F30:G30 D32 B34', ['$A30=1']),
    # (4) rebasing off-sheet: base will be B42; cell D40 asks for row 1-2 = -1
    ('D40 B42', ['$A1=1']),
]
for r, v in ((20, 1), (22, 1), (30, 1), (32, 1), (34, 1), (40, 1), (42, 1)):
    cells.setdefault('A%d' % r, v)
cells['A20'] = 1; cells['A22'] = 0
cells['A28'] = 1; cells['A30'] = 0; cells['A32'] = 1; cells['A34'] = 0
cells['A1'] = 1

for ref in ('D10','B12','B22','D20','F30','G30','D32','B34','D40','B42'):
    cells[ref] = ref

colors = ['FFCC0000', 'FF00AA00', 'FF0000CC', 'FFCC00CC']
out = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'probe3.xlsx')
build(out, cells, (), cfs, colors, dim='A1:J45')
print(out)
for i, (sq, f) in enumerate(cfs):
    print('dxf%d %-20s %-12s %s' % (i, sq, f[0], colors[i]))
print({k: v for k, v in sorted(cells.items()) if k.startswith('A')})
