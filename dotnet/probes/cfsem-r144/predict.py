"""Predict the witness's conditional fills from the semantics derived in this
probe, then diff against what 26.2.4.2 actually painted."""
import sys, math
sys.path.insert(0, '/home/user/libreoffice-core/dotnet/probes/cfsem-r144')
from readfods import cellvals
from witness_read import read as read_pdf

FODS = '/home/user/libreoffice-core/dotnet/probes/cfsem-r144/out/072_Gantt_project_planner_dde00e33.fods'
PDF  = '/home/user/libreoffice-core/dotnet/probes/cfsem-r144/out/072_Gantt_project_planner_dde00e33.pdf'

V = cellvals(FODS)
def cell(ref):
    """Reference's own value for a cell: float, str, or None for empty."""
    d = V.get('Project Planner!' + ref)
    if d is None:
        return None
    if d['vt'] in ('float', 'percentage', 'currency'):
        return float(d['v'])
    if d['vt'] == 'boolean':
        return 1.0 if d['bv'] == 'true' else 0.0
    if d['vt'] is None and not d['text']:
        return None
    return d['text']            # string cell

class Err(Exception):
    pass

def num(v):
    """Numeric coercion of a cell value in an arithmetic context."""
    if v is None:
        return 0.0              # empty cell counts as 0
    if isinstance(v, str):
        raise Err('text in arithmetic')
    return v

def cmp_eq(a, b):
    return _cmp(a, b) == 0
def cmp_lt(a, b):
    return _cmp(a, b) < 0
def cmp_gt(a, b):
    return _cmp(a, b) > 0

def _cmp(a, b):
    """sc::CompareFunc: empty == 0 and == "", any number < any text."""
    ae, be = a is None, b is None
    if ae and be: return 0
    if ae:
        if isinstance(b, str): return 0 if b == '' else -1
        return 0 if b == 0 else (1 if b < 0 else -1)
    if be:
        if isinstance(a, str): return 0 if a == '' else 1
        return 0 if a == 0 else (-1 if a < 0 else 1)
    if isinstance(a, str) and isinstance(b, str):
        a, b = a.lower(), b.lower()
        return 0 if a == b else (-1 if a < b else 1)
    if isinstance(a, str): return 1           # string > number
    if isinstance(b, str): return -1          # number < string
    if abs(a - b) < 1e-12 * max(1.0, abs(a), abs(b)): return 0
    return -1 if a < b else 1

def median(args):
    """MEDIAN: a direct cell reference that is not numeric is skipped;
    a computed scalar is always taken."""
    vals = []
    for kind, v in args:
        if kind == 'ref':
            if v is None or isinstance(v, str):
                continue        # GetNumberSequenceArray: only hasNumeric() pushed
            vals.append(v)
        else:
            vals.append(num(v))
    if not vals:
        raise Err('MEDIAN of nothing')
    vals.sort()
    n = len(vals)
    return vals[n // 2] if n % 2 else (vals[n // 2 - 1] + vals[n // 2]) / 2

def INT(x):  return math.floor(x)
def MOD(a, b):
    if b == 0: raise Err('div0')
    return a - math.floor(a / b) * b

def colname(n):
    s = ''
    while n > 0:
        n, r = divmod(n - 1, 26); s = chr(65 + r) + s
    return s

def evaluate(c, r):
    """Values of the ten rules at worksheet cell (column c 1-based, row r).
    A name written '$C1' has base A1, so its row offset is 0 -> row r.
    A name written 'A$4' has base A1, so its column offset is 0 -> column c."""
    A4 = cell('%s4' % colname(c))     # 'Project Planner'!A$4
    C  = cell('C%d' % r); D = cell('D%d' % r); E = cell('E%d' % r)
    F  = cell('F%d' % r); G = cell('G%d' % r)
    out = {}
    def rule(name, fn):
        try:
            out[name] = fn()
        except Err:
            out[name] = 'ERR'

    def PeriodInPlan():
        return cmp_eq(A4, median([('ref', A4), ('ref', C), ('calc', num(C) + num(D) - 1)]))
    def PeriodInActual():
        return cmp_eq(A4, median([('ref', A4), ('ref', E), ('calc', num(E) + num(F) - 1)]))
    def PercentCompleteBeyond():
        lhs = median([('ref', A4), ('ref', E), ('calc', num(E) + num(F))]) * (1 if cmp_gt(E, 0) else 0)
        t1 = 1 if cmp_eq(A4, lhs) else 0
        t2 = (1 if cmp_lt(A4, INT(num(E) + num(F) * num(G))) else 0) + (1 if cmp_eq(A4, E) else 0)
        return t1 * t2 * (1 if cmp_gt(G, 0) else 0)

    rule('PercentComplete',       lambda: PercentCompleteBeyond() * (1 if PeriodInPlan() else 0))
    rule('PercentCompleteBeyond', PercentCompleteBeyond)
    rule('Actual',                lambda: ((1 if PeriodInActual() else 0) * (1 if cmp_gt(E, 0) else 0))
                                          * (1 if PeriodInPlan() else 0))
    rule('ActualBeyond',          lambda: (1 if PeriodInActual() else 0) * (1 if cmp_gt(E, 0) else 0))
    rule('Plan',                  lambda: (1 if PeriodInPlan() else 0) * (1 if cmp_gt(C, 0) else 0))
    rule('PeriodHighlight',       lambda: cmp_eq(cell('%s4' % colname(c)), cell('H2')))
    rule('OddColumn',             lambda: MOD(c, 2))
    rule('EvenColumn',            lambda: cmp_eq(MOD(c, 2), 0))
    return out

ORDER = [('PercentComplete',       '#735773'),
         ('PercentCompleteBeyond', '#E9AB51'),
         ('Actual',                '#B5A1B5'),
         ('ActualBeyond',          '#D6BCA8'),
         ('Plan',                  '#DCD5DC'),
         ('PeriodHighlight',       '#F6DDB9'),
         ('OddColumn',             '#F2F2F2'),
         ('EvenColumn',            '#FFFFFF')]

def fires(v):
    """ScConditionEntry::IsCellValid, Direct mode: nVal1 != 0.0.
    A string or error result leaves nVal1 at 0 and never fires."""
    if v == 'ERR' or isinstance(v, str):
        return False
    if isinstance(v, bool):
        return v
    return v != 0.0

def predict(c, r):
    vals = evaluate(c, r)
    for name, colr in ORDER:
        if fires(vals[name]):
            return colr, name
    return None, None

if __name__ == '__main__':
    cols, rows, obs = read_pdf(PDF)
    bad = 0; tot = 0
    for p in sorted(cols):            # p = period number, column H+p-1 -> col index 7+p
        for r in sorted(rows):
            c = 7 + p
            want, name = predict(c, r)
            got = obs[(p, r)]
            tot += 1
            if (want or '').upper() != (got or '').upper():
                bad += 1
                if bad <= 25:
                    print('MISMATCH %s%d  predicted %s (%s)  observed %s'
                          % (colname(c), r, want, name, got))
    print('cells compared: %d   mismatches: %d' % (tot, bad))
