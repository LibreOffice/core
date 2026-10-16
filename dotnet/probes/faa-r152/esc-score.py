#!/usr/bin/env python3
"""Score the superscript-size rule over every base size in the E family, both renderers.

Candidate rules, all applied to the base size the renderer actually drew:
  tw-trunc   trunc(size_twips * 58 / 100)
  tw-round   round(size_twips * 58 / 100)
  tenth      round(size_tenths_of_a_pt * 58 / 100)      -- i.e. to the nearest 1/10 pt
  mm100      round-trip through 1/100 mm
"""
import sys, re, pymupdf


def read(path):
    doc = pymupdf.open(path)
    lines = []
    for pno in range(doc.page_count):
        for b in doc[pno].get_text('dict')['blocks']:
            for l in b.get('lines', []):
                lines.append((pno, round(l['bbox'][1], 3), l))
    lines.sort(key=lambda t: (t[0], t[1]))
    out, cur = {}, None
    for pno, y, l in lines:
        txt = ''.join(s['text'] for s in l['spans']).strip()
        m = re.match(r'ARM E(\d+)$', txt)
        if m:
            cur = int(m.group(1)); continue
        if cur is not None and txt.startswith('Mx') and len(l['spans']) >= 2:
            out[cur] = (l['spans'][0]['size'], l['spans'][-1]['size'])
            cur = None
    return out


def half_up(x):
    import math
    return math.floor(x + 0.5)


def rules(base_pt):
    tw = half_up(base_pt * 20)
    return {
        'tw-trunc': int(tw * 58 / 100) / 20.0,
        'tw-round': half_up(tw * 58 / 100) / 20.0,
        'tenth': half_up(half_up(base_pt * 10) * 58 / 100) / 10.0,
        'mm100': half_up(half_up(half_up(base_pt * 2540 / 72) * 58 / 100) * 72 / 2540 * 20) / 20.0,
    }


ref, our = read(sys.argv[1]), read(sys.argv[2])
names = list(rules(8.0))
score = {n: [0, 0] for n in names}
print('hp\tbase\tref_sup\tours_sup\t' + '\t'.join(names))
for hp in sorted(ref):
    if hp not in our: continue
    base, rs = ref[hp]
    _, os_ = our[hp]
    r = rules(base)
    for n in names:
        if abs(r[n] - rs) < 1e-6: score[n][0] += 1
        if abs(r[n] - os_) < 1e-6: score[n][1] += 1
    print(f'{hp}\t{base:.2f}\t{rs:.3f}\t{os_:.3f}\t' + '\t'.join(f'{r[n]:.3f}' for n in names))
n_tot = len([h for h in ref if h in our])
print()
print('rule\tmatches ref\tmatches ours\t(of %d)' % n_tot)
for n in names:
    print(f'{n}\t{score[n][0]}\t{score[n][1]}')
