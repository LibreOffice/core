#!/usr/bin/env python3
"""Compare a `sweep-ours.sh` run against the banked gate, and check the instrument first.

    score-sweep.py <rows.tsv> <banked parity.tsv> [track-prefix]

Two controls before any verdict is quoted:

  * every row of the track is present, counted against the banked file rather than against
    the sweep's own total; and
  * the *reference* half agrees with the bank document by document. The sweep re-derives it
    from the banked PDFs rather than trusting the stored numbers, so a disagreement here
    means the reading changed and no verdict movement below is attributable to the tree.
"""
import sys, collections

rows_path, bank_path = sys.argv[1], sys.argv[2]
prefix = sys.argv[3] if len(sys.argv) > 3 else 'sheets'


def read(path):
    out = {}
    for line in open(path, encoding='utf-8'):
        if line.startswith('#') or line.startswith('path\t'):
            continue
        f = line.rstrip('\n').split('\t')
        if len(f) < 9 or not f[0].startswith(prefix):
            continue
        out[f[0]] = {'pages': f[2], 'words': f[3], 'unemb': f[5],
                     'verdict': f[6], 'raw': f[7], 'glyphs': f[8]}
    return out


now, bank = read(rows_path), read(bank_path)
print(f'{len(now)} rows swept, {len(bank)} in the bank for "{prefix}"')

missing = sorted(set(bank) - set(now))
extra = sorted(set(now) - set(bank))
if missing:
    print(f'!! {len(missing)} banked rows not in the sweep:', *missing[:5], sep='\n   ')
if extra:
    print(f'!! {len(extra)} swept rows not in the bank:', *extra[:5], sep='\n   ')

drift = [p for p in now if p in bank
         and (now[p]['glyphs'].split('/')[1] != bank[p]['glyphs'].split('/')[1]
              or now[p]['pages'].split('/')[1] != bank[p]['pages'].split('/')[1])]
print(f'reference half re-derived and equal on {len(now) - len(drift)} of {len(now)}')
for p in drift[:10]:
    print(f'   !! ref moved: {p}  {bank[p]["pages"]}|{bank[p]["glyphs"]}'
          f' -> {now[p]["pages"]}|{now[p]["glyphs"]}')

tally = collections.Counter(v['verdict'] for v in now.values())
was = collections.Counter(bank[p]['verdict'] for p in now if p in bank)
print('\nverdicts now :', dict(sorted(tally.items())))
print('verdicts bank:', dict(sorted(was.items())))

moved = [p for p in sorted(now) if p in bank and now[p]['verdict'] != bank[p]['verdict']]
print(f'\n{len(moved)} verdicts moved')
for p in moved:
    print(f'  {bank[p]["verdict"]:>12s} -> {now[p]["verdict"]:<12s} '
          f'pages {bank[p]["pages"]}->{now[p]["pages"]} '
          f'glyphs {bank[p]["glyphs"]}->{now[p]["glyphs"]}  {p}')

changed = [p for p in sorted(now) if p in bank
           and now[p]['glyphs'].split('/')[0] != bank[p]['glyphs'].split('/')[0]]
print(f'\n{len(changed)} documents where our own glyph count moved at all')
for p in changed:
    a = int(bank[p]['glyphs'].split('/')[0]); b = int(now[p]['glyphs'].split('/')[0])
    print(f'  {b - a:+6d}  {bank[p]["glyphs"]} -> {now[p]["glyphs"]}  '
          f'[{bank[p]["verdict"]} -> {now[p]["verdict"]}]  {p}')
