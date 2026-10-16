#!/usr/bin/env python3
"""Lines we end with a number-opening hyphen that 26.2.4.2 would not break there.

    hyphcensus.py <ours-dir> <label>

The break the audit found wrong is precisely: a line ending in a hyphen whose *next* line begins
with a digit, where the character before the hyphen is NOT a digit.  `10-` / `19` is the one
arrangement 26.2.4.2 also breaks, so it is excluded.  Read off our own renderings, because it is
our breaks that would move.
"""
import glob, os, re, subprocess, sys, collections
ours, label = sys.argv[1], sys.argv[2]
pat = re.compile(r'(?:^|(?<=[^0-9]))\-$')
docs = collections.Counter()
total = 0
files = sorted(glob.glob(os.path.join(ours, '*.pdf')))
if not files:
    print('REFUSING: no PDFs in', ours); sys.exit(2)
for p in files:
    txt = subprocess.run(['pdftotext', '-layout', p, '-'],
                         capture_output=True, timeout=300).stdout.decode('utf-8', 'replace')
    lines = [l.rstrip() for l in txt.splitlines()]
    n = 0
    for i in range(len(lines) - 1):
        a, b = lines[i].rstrip(), lines[i + 1].lstrip()
        if not a or not b: continue
        if a.endswith('-') and (len(a) < 2 or not a[-2].isdigit()) and b[0].isdigit():
            n += 1
    if n: docs[os.path.basename(p)[:-4]] = n; total += n
print('%s: %d documents, %d such line ends (of %d renderings)' % (label, len(docs), total, len(files)))
for d, n in docs.most_common(20): print('   %5d  %s' % (n, d))
