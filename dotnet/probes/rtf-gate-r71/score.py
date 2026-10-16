#!/usr/bin/env python3
"""Join a sweep-ours.sh `ours.tsv` onto the banked reference columns and apply the gate rule.

    score.py <ours.tsv> [bank.tsv] > rows.tsv

The gate rule is batch-check.sh's of 2026-09-05: page count, then alphanumeric characters
within max(2%, 15), then unembedded fonts.
"""
import sys, collections

OURS = sys.argv[1]
BANK = sys.argv[2] if len(sys.argv) > 2 else '/home/user/gate-odf-rows.tsv'

ref = {}
for line in open(BANK):
    if line.startswith('#'):
        continue
    p = line.rstrip('\n').split('\t')
    if len(p) < 9 or p[1] != 'rtf':
        continue
    ref[p[0]] = dict(pages=p[2].split('/')[1], words=p[3].split('/')[1],
                     fonts=p[4].split('/')[1], rawwords=p[7].split('/')[1],
                     glyphs=p[8].split('/')[1])

print('# ours = fresh render; ref columns = banked from gate-odf-rows.tsv (odf-gate-01)')
print('path\text\tpages\twords\tfonts\tunemb\tverdict\trawwords\tglyphs')
tally = collections.Counter()
for line in open(OURS):
    path, op, ow, oraw, og, of, un = line.rstrip('\n').split('\t')
    r = ref.get(path)
    if r is None:
        v = 'no-bank'
        r = dict(pages='-', words='-', fonts='-', rawwords='-', glyphs='-')
    elif op == '-':
        v = 'ours-failed'
    elif r['pages'] == '-':
        v = 'ref-failed'
    else:
        v = ''
        if op != r['pages']:
            v = 'pages'
        rg = int(r['glyphs']); ogi = int(og)
        if rg > 0:
            if abs(ogi - rg) > rg * 0.02 and abs(ogi - rg) > 15:
                v = (v + ',' if v else '') + 'words'
        elif ogi > 15:
            v = (v + ',' if v else '') + 'words'
        if un != '0':
            v = (v + ',' if v else '') + 'unembedded'
        v = v or 'match'
    tally[v] += 1
    print('\t'.join([path, 'rtf', f"{op}/{r['pages']}", f"{ow}/{r['words']}",
                     f"{of}/{r['fonts']}", un, v,
                     f"{oraw}/{r['rawwords']}", f"{og}/{r['glyphs']}"]))
n = 0
for k, c in sorted(tally.items(), key=lambda x: -x[1]):
    print(f'{k} {c}', file=sys.stderr); n += c
print(f'TOTAL {n}', file=sys.stderr)
