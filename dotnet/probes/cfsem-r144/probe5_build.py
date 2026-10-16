"""Probe 5: what happens when a relative reference is rebased off the sheet.
Names go through ScCompiler::MoveRelWrap; plain rule references do not."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkxlsx import build

MAXROW = 1048576
cells = {
    'A1': 42,
    'A%d' % MAXROW: 1,       # the row a wrap from row -1 would land on
    'D40': 'D40', 'B42': 'B42',
    'C2': '=WrapName',       # name whose row offset overflows the sheet
    'C3': '=WrapCol',
}
names = [
    ('WrapName', 'Sheet1!$A%d' % MAXROW),   # base A1 -> row offset MAXROW-1
    ('WrapCol',  'Sheet1!XFD$1'),           # base A1 -> col offset 16383
]
cfs = [('D40 B42', ['$A1=1'])]              # base is B42 -> row offset -41
colors = ['FFCC00CC']
out = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'probe5.xlsx')
build(out, cells, names, cfs, colors, dim='A1:Z50')
print(out)
