#!/usr/bin/env python3
"""Score a batch-check rows.tsv against MANIFEST.tsv's own path list.

    score.py <manifest> <rows.tsv> [family]

`batch-check.sh` reports more rows than the manifest has documents -- 355 for the words track's
337 -- because this mount is case-insensitive and carries an upper-case alias directory entry
for some documents.  Which aliases a shell glob enumerates is **not stable between runs**: the
round-58 slides sweep enumerated 311 rows before the change and 315 after, and the four extra
were all alias entries of documents already counted.  Reading those two totals as a gain of four
would have been a whole false result, so the score is taken against the manifest's own path list
and nothing else.

Refuses to print unless every manifest path found exactly one row.
"""
import csv, sys, collections
man, rows = sys.argv[1], sys.argv[2]
family = sys.argv[3] if len(sys.argv) > 3 else 'words' 
paths = []
status = {}
with open(man, newline='', encoding='utf-8') as f:
    for r in csv.DictReader(f, delimiter='\t'):
        if r['family'] == family:
            paths.append(r['path']); status[r['path']] = r['status']
byp = collections.defaultdict(list)
with open(rows, newline='', encoding='utf-8') as f:
    for r in csv.reader(f, delimiter='\t'):
        if r: byp[r[0]].append(r)
missing = [p for p in paths if len(byp.get(p, [])) != 1]
if missing:
    print('REFUSING: %d manifest paths without exactly one row' % len(missing))
    for p in missing[:20]: print('   ', p, len(byp.get(p, [])))
    sys.exit(2)
m = [p for p in paths if byp[p][0][6] == 'match']
x = [p for p in paths if byp[p][0][6] != 'match']
print('manifest %s paths:' % family, end=' ')
print(' %d   match %d   mismatch %d' % (len(paths), len(m), len(x)))
dis = [(p, status[p], byp[p][0][6]) for p in paths
       if (status[p] == 'done') != (byp[p][0][6] == 'match')]
print('disagreements with manifest status column: %d' % len(dis))
for p, s, v in dis: print('   ', p, 'manifest=%s measured=%s' % (s, v))
print('\nopen (mismatch) documents:')
for p in x: print('   ', byp[p][0][6], p, byp[p][0][2], byp[p][0][3], byp[p][0][4])
