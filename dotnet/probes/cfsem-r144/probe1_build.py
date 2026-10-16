"""Probe 1: evaluate names / arithmetic / functions / empty+text in ORDINARY
formula cells, then read the reference's own computed values out of .fods."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkxlsx import build, colname

cells = {}
# row 4 across A..Z: value = 1-based column index
for c in range(1, 27):
    cells['%s4' % colname(c)] = c
# C1..C12 and D1..D12
for r in range(1, 13):
    cells['C%d' % r] = 300 + r
    cells['D%d' % r] = 400 + r
cells['G2'] = 'hello'          # text cell
# H2 deliberately left EMPTY

names = [
    ('RowRel', 'Sheet1!$C1'),      # absolute col, relative row (base A1)
    ('ColRel', 'Sheet1!A$4'),      # relative col, absolute row (base A1)
    ('Free',   'Sheet1!A1'),       # both relative (base A1)
    ('RowRel5','Sheet1!$C5'),      # relative row written as 5 -> offset +4 from base A1
    ('PlanLike','Sheet1!A$4=MEDIAN(Sheet1!A$4,Sheet1!$C1,Sheet1!$C1+Sheet1!$D1-1)'),
    ('ColNo',  'COLUMN()'),
    ('RowNo',  'ROW()'),
]

# name-resolution probes, rows 5..8, cols K,L,M,N,O,S,T
for r in range(5, 9):
    cells['K%d' % r] = '=RowRel'
    cells['L%d' % r] = '=ColRel'
    cells['M%d' % r] = '=Free'
    cells['N%d' % r] = '=ROW()'
    cells['O%d' % r] = '=COLUMN()'
    cells['S%d' % r] = '=RowRel5'
    cells['T%d' % r] = '=ColNo'
    cells['U%d' % r] = '=RowNo'

arith = {
  'P5':  '=(1=1)',
  'P6':  '=(1=1)*(2=2)',
  'P7':  '=(1=1)+(1=1)',
  'P8':  '=(1=2)+(1=1)',
  'P9':  '=(1=1)*5',
  'P10': '=MEDIAN(5,1,10)',
  'P11': '=MEDIAN(5,10,1)',
  'P12': '=INT(-2.5)',
  'P13': '=INT(2.9)',
  'P14': '=MOD(-3,2)',
  'P15': '=MOD(7,2)',
  'P16': '=MOD(6,2)',
  'P17': '=(1=2)*(1=1)',
  'P18': '=TYPE((1=1))',
  'P19': '=TYPE((1=1)*1)',
  'P20': '=TYPE((1=1)+(1=1))',
  'P21': '=MEDIAN(3,1,2)',
  'P22': '=MEDIAN(1,2)',
  'P23': '=INT(2.5)',
  'P24': '=MOD(-1,2)',
  'Q5':  '=(H2=0)',
  'Q6':  '=H2',
  'Q7':  '=H2*1',
  'Q8':  '=H2>0',
  'Q9':  '=G2>0',
  'Q10': '=G2=0',
  'Q11': '=MEDIAN(1,G2,3)',
  'Q12': '=G2*1',
  'Q13': '=(G2="hello")',
  'Q14': '=ISBLANK(H2)',
  'Q15': '=MEDIAN(H2,1,2)',
  'Q16': '=H2+1',
  'Q17': '=(H2="")',
  'Q18': '=COUNT(H2)',
  'Q19': '=G2<0',
  'Q20': '=(G2>0)*1',
  'Q21': '=ISERROR(G2*1)',
  'Q22': '=H2=""',
  'Q23': '=MEDIAN(H2,0,5)',
  'Q24': '=TYPE(H2)',
}
cells.update(arith)

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'probe1.xlsx')
build(out, cells, names, dim='A1:Z30')
print(out)
