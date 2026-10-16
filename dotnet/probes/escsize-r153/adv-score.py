#!/usr/bin/env python3
"""Score the three candidate quantisations of an escaped size against a measured advance.

    adv-score.py <pdf> [N1 N2 upem hmtx]

Reads each ARM H<hp> N<n> line, takes the LAST span on it (the superscript run), and differences
the two arms' span left edges at each size.  The difference is (N2-N1) advances of one glyph at
whatever size the layout used, with the label, the margin and the side bearing all cancelled.
"""
import sys, re, pymupdf, math

pdf = sys.argv[1]
N1, N2 = int(sys.argv[2]), int(sys.argv[3])
UPEM, HMTX = int(sys.argv[4]), int(sys.argv[5])

doc = pymupdf.open(pdf)
left = {}
for pno in range(doc.page_count):
    for b in doc[pno].get_text('dict')['blocks']:
        for l in b.get('lines', []):
            txt = ''.join(s['text'] for s in l['spans'])
            m = re.match(r'ARM H(\d+) N(\d+) ', txt)
            if not m:
                continue
            hp, n = int(m.group(1)), int(m.group(2))
            left[(hp, n)] = l['spans'][-1]['bbox'][0]

half_up = lambda x: math.floor(x + 0.5)
rows = []
for hp in sorted({k[0] for k in left}):
    if (hp, N1) not in left or (hp, N2) not in left:
        continue
    d = left[(hp, N1)] - left[(hp, N2)]           # (N2-N1) advances
    adv = d / (N2 - N1)
    size = adv * UPEM / HMTX                       # pt
    tw = hp * 10                                   # base size in twips (half-points -> twips is x10)
    cand = {
        'trunc':  int(tw * 58 / 100),
        'roundtw': half_up(tw * 58 / 100),
        'tenth':  2 * half_up(tw * 58 / 200),
    }
    rows.append((hp, tw, size * 20, cand))

names = ['trunc', 'roundtw', 'tenth']
score = {n: 0 for n in names}
split = {n: 0 for n in names}
print('base_pt\tbase_tw\tmeasured_tw\t' + '\t'.join(names) + '\tverdict')
for hp, tw, meas, cand in rows:
    hits = [n for n in names if abs(cand[n] - meas) < 0.35]
    for n in hits:
        score[n] += 1
    disc = len({cand[n] for n in names}) > 1
    print(f'{hp/2:.1f}\t{tw}\t{meas:.3f}\t' + '\t'.join(str(cand[n]) for n in names)
          + '\t' + (','.join(hits) if hits else 'NONE') + ('\t*' if disc else ''))
    if disc:
        for n in hits:
            split[n] += 1
print()
print('all sizes:        ' + '  '.join(f'{n} {score[n]}/{len(rows)}' for n in names))
ndisc = sum(1 for _, _, _, c in rows if len({c[n] for n in names}) > 1)
print(f'discriminating:   ' + '  '.join(f'{n} {split[n]}/{ndisc}' for n in names))
