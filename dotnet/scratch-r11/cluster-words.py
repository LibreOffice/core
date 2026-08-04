#!/usr/bin/env python3
"""Cluster the words-track failures by shared shape rather than by batch."""
import sys, collections, os, re

rows = []
for line in open(sys.argv[1], encoding='utf8'):
    f = line.rstrip('\n').split('\t')
    if len(f) < 7:
        continue
    doc, ext, pages, words, fonts, unemb, verdict = f[:7]
    def pair(s):
        try:
            a, b = s.split('/'); return int(a), int(b)
        except Exception:
            return None, None
    op, rp = pair(pages); ow, rw = pair(words)
    rows.append(dict(doc=doc, ext=ext, op=op, rp=rp, ow=ow, rw=rw, verdict=verdict,
                     name=os.path.basename(doc),
                     batch=doc.split('/')[1]))

fail = [r for r in rows if r['verdict'] != 'match']

def wpct(r):
    return 100.0 * (r['ow'] - r['rw']) / r['rw'] if r['rw'] else 0.0

print("=== A. pagination-only: page delta non-zero, word delta inside 0.5% ===")
a = [r for r in fail if r['op'] != r['rp'] and abs(wpct(r)) <= 0.5]
for r in sorted(a, key=lambda r: (abs(r['op']-r['rp']), r['name'])):
    print(f"  {r['op']-r['rp']:+d}  pg {r['op']:>4}/{r['rp']:<4} w {r['ow']:>6}/{r['rw']:<6} {wpct(r):+6.2f}%  {r['name'][:60]} [{r['batch']}]")
print(f"  -> {len(a)} documents, total |page error| {sum(abs(r['op']-r['rp']) for r in a)}")

print("\n=== B. page count right, words wrong ===")
b = [r for r in fail if r['op'] == r['rp']]
for r in sorted(b, key=lambda r: -abs(wpct(r))):
    print(f"   0  pg {r['op']:>4}/{r['rp']:<4} w {r['ow']:>6}/{r['rw']:<6} {wpct(r):+6.2f}%  {r['name'][:60]} [{r['batch']}]")
print(f"  -> {len(b)} documents")

print("\n=== C. both wrong: pages off AND words beyond 0.5% ===")
c = [r for r in fail if r['op'] != r['rp'] and abs(wpct(r)) > 0.5]
for r in sorted(c, key=lambda r: -abs(wpct(r))):
    print(f"  {r['op']-r['rp']:+d}  pg {r['op']:>4}/{r['rp']:<4} w {r['ow']:>6}/{r['rw']:<6} {wpct(r):+6.2f}%  {r['name'][:60]} [{r['batch']}]")
print(f"  -> {len(c)} documents")

print("\n=== named families over the whole track (match / total) ===")
fams = {
    'mcar_part': r'mcar_part',
    '150_5300_13': r'150_5300_13',
    '150-5370-10': r'150.5370.10',
    'CAO/FCTOA (EASA forms)': r'(TE\.CAO|UG\.CAO|FO\.FCTOA|EASA Form|registro_de_aprovacao)',
    'ABCD-': r'ABCD-',
    'ESPN-R': r'ESPN-R',
    'tpr/report template': r'(tpr_template|technical-progress-report|report-template|technical-report-template)',
    'bulletin': r'[Bb]ulletin',
}
for label, pat in fams.items():
    sel = [r for r in rows if re.search(pat, r['name'])]
    m = sum(1 for r in sel if r['verdict'] == 'match')
    print(f"  {label:<26} {m}/{len(sel)}")
    for r in sel:
        if r['verdict'] != 'match':
            print(f"       {r['op']-r['rp']:+d} pg {r['op']}/{r['rp']}  w {r['ow']}/{r['rw']} {wpct(r):+.1f}%  {r['name'][:58]}")
