#!/usr/bin/env python3
"""Join the reference's resolved row heights against ours, row by row, and report
the per-row line-count error using the document's own quantum.

usage: rowdiff.py <ref-rows.txt> <our-rows.txt> [quantum] [base] [--csv out.tsv]
"""
import sys, re, collections

CAP = 5000   # a run longer than this is a default-height tail; take its head only

def read(path, ref):
    out, sheet = {}, None
    for line in open(path):
        if line.startswith('== sheet'):
            sheet = line[len('== sheet '):].rstrip('\n')
            out[sheet] = {}
            continue
        p = line.split()
        if len(p) < 3 or p[0] != 'rows': continue
        a, b = (int(x) for x in p[1].split('-'))
        if ref:
            h, opt = p[3], ('opt=true' in line)
        else:
            h, opt = p[2], ('opt=True' in line)
        if h == '-': continue
        h = float(h)
        if b - a > CAP: b = a + CAP
        d = out[sheet]
        for r in range(a, b+1): d[r] = (h, opt)
    return out

args = [a for a in sys.argv[1:] if not a.startswith('--')]
csv = None
for i, a in enumerate(sys.argv):
    if a == '--csv': csv = sys.argv[i+1]; args = [x for x in args if x != csv]

ref = read(args[0], True)
ours = read(args[1], False)
Q = float(args[2]) if len(args) > 2 else 268.3
B = float(args[3]) if len(args) > 3 else 299.952

def lines(h): return int(round((h - B)/Q)) + 1

per = collections.Counter()
examples = collections.defaultdict(list)
allrows = agree = 0
refline = ourline = 0
rows_out = []
for s in sorted(set(ref) & set(ours)):
    for r in sorted(set(ref[s]) & set(ours[s])):
        rh, ropt = ref[s][r]
        oh, oopt = ours[s][r]
        allrows += 1
        refline += lines(rh); ourline += lines(oh)
        if abs(rh - oh) <= 1.0:
            agree += 1; continue
        d = lines(rh) - lines(oh)
        per[d] += 1
        rows_out.append((s, r, rh, oh, d))
        if len(examples[d]) < 8: examples[d].append((s, r, rh, oh))

print('rows compared          %d' % allrows)
print('rows agreeing (<=1tw)  %d  (%.1f%%)' % (agree, 100.0*agree/max(1,allrows)))
print('rows differing         %d' % sum(per.values()))
print('total lines  ref %d   ours %d   deficit %d' % (refline, ourline, refline-ourline))
print('line delta histogram (ref-ours):')
for d in sorted(per): print('   %+3d  %6d' % (d, per[d]))
print()
for d in sorted(examples):
    print('  delta %+d:' % d)
    for s, r, rh, oh in examples[d]:
        print('     %-30s row %-6d ref %9.2f ours %9.2f' % (s, r, rh, oh))
if csv:
    with open(csv, 'w') as f:
        f.write('sheet\trow\tref\tours\tdelta\n')
        for s, r, rh, oh, d in rows_out:
            f.write('%s\t%d\t%.3f\t%.3f\t%d\n' % (s, r, rh, oh, d))
