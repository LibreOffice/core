#!/usr/bin/env python3
"""Score trunc / round-to-a-twip / round-to-a-tenth over base size x proportion, from ADVANCES.

The digits of a strongly-raised small run land on a PyMuPDF "line" of their own, so the label and
its digits are paired by geometry -- the digit span starts where the label span ends -- rather than
by being on one line.  An arm whose digit span is missing or short is dropped and said so.
"""
import sys, re, math, pymupdf

PDF = sys.argv[1]
SIZES = [float(x) for x in sys.argv[2].split(',')]
PROPS = [int(x) for x in sys.argv[3].split(',')]
N1, N2 = 2, 42
UPEM, HMTX = 2048, 1139

doc = pymupdf.open(PDF)
labels, digits = {}, []
for pno in range(doc.page_count):
    for b in doc[pno].get_text('dict')['blocks']:
        for l in b.get('lines', []):
            for s in l['spans']:
                t = s['text']
                m = re.match(r'ARM S(\d+)P(\d+)N(\d+) $', t)
                if m:
                    labels[(int(m.group(1)), int(m.group(2)), int(m.group(3)))] = (pno, s['bbox'][2], s['bbox'][1], s['bbox'][3])
                elif t and set(t) == {'0'}:
                    digits.append((pno, s['bbox'][0], s['bbox'][1], s['bbox'][3], len(t)))

def start(key, n):
    if key not in labels:
        return None
    pno, x1, y0, y1 = labels[key]
    hit = [x0 for dp, x0, dy0, dy1, ln in digits
           if dp == pno and ln == n and -1.0 <= x0 - x1 <= 3.0 and dy1 > y0 - 1.0 and dy0 < y1 + 1.0]
    return hit[0] if len(hit) == 1 else None

half = lambda x: math.floor(x + 0.5)
names = ['trunc', 'roundtw', 'tenth']
sc = {n: 0 for n in names}
sp = {n: 0 for n in names}
tot = dtot = drop = 0
print('base\tprop\tmeas_tw\t' + '\t'.join(names) + '\tverdict')
for si, size in enumerate(SIZES):
    tw = round(size * 20)
    for p in PROPS:
        a, b = start((si, p, N1), N1), start((si, p, N2), N2)
        if a is None or b is None:
            print(f'{size}\t{p}\tdropped (no digit span)'); drop += 1; continue
        meas = (a - b) / (N2 - N1) * UPEM / HMTX * 20
        cand = {'trunc': int(tw * p / 100), 'roundtw': half(tw * p / 100), 'tenth': 2 * half(tw * p / 200)}
        hits = [n for n in names if abs(cand[n] - meas) < 0.35]
        d = len(set(cand.values())) > 1
        tot += 1; dtot += d
        for n in hits:
            sc[n] += 1
            if d: sp[n] += 1
        print(f'{size}\t{p}\t{meas:.3f}\t' + '\t'.join(str(cand[n]) for n in names)
              + '\t' + (','.join(hits) or 'NONE') + ('\t*' if d else ''))
print()
print(f'scored {tot}, dropped {drop}')
print('all:            ' + '  '.join(f'{n} {sc[n]}/{tot}' for n in names))
print('discriminating: ' + '  '.join(f'{n} {sp[n]}/{dtot}' for n in names))
