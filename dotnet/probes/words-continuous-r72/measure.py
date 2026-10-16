#!/usr/bin/env python3
"""Per variant: page count, which pages carry the running head, and the top of
the first body word on pages 1..3."""
import re, subprocess, sys
from pathlib import Path

HEAD = 'HDSS Bulletin Issue 285'

def words(pdf, page):
    xml = subprocess.run(['pdftotext', '-bbox', '-f', str(page), '-l', str(page),
                          str(pdf), '-'], capture_output=True, text=True).stdout
    return [(float(m[0]), float(m[1]), m[4]) for m in
            re.findall(r'<word xMin="([0-9.-]+)" yMin="([0-9.-]+)" '
                       r'xMax="([0-9.-]+)" yMax="([0-9.-]+)">([^<]*)</word>', xml)]

def npages(pdf):
    out = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, text=True).stdout
    return int(re.search(r'Pages:\s+(\d+)', out).group(1))

def headpages(pdf, n):
    hit = []
    for p in range(1, n + 1):
        t = subprocess.run(['pdftotext', '-f', str(p), '-l', str(p), str(pdf), '-'],
                           capture_output=True, text=True).stdout
        if HEAD in ' '.join(t.split()):
            hit.append(p)
    return hit

def main():
    root = Path(sys.argv[1])
    print(f"{'variant':16} {'side':6} {'pages':>5}  {'head on':22} "
          f"{'p1 top':>8} {'p2 top':>8} {'p3 top':>8}")
    for name in sorted(p.stem for p in (root / 'ours').glob('*.pdf')):
        for side in ('ours', 'ref26'):
            pdf = root / side / (name + '.pdf')
            if not pdf.exists():
                print(f"{name:16} {side:6} MISSING"); continue
            n = npages(pdf)
            hp = headpages(pdf, n)
            tops = []
            for p in (1, 2, 3):
                if p > n:
                    tops.append(float('nan')); continue
                w = [x for x in words(pdf, p) if x[2].strip()]
                # skip the running head line if present
                if hp and p in hp:
                    ymin0 = min(x[1] for x in w)
                    w = [x for x in w if x[1] > ymin0 + 5]
                tops.append(min((x[1] for x in w), default=float('nan')))
            rng = (','.join(map(str, hp)) if len(hp) < 6
                   else f"{hp[0]}-{hp[-1]} ({len(hp)})") if hp else 'none'
            print(f"{name:16} {side:6} {n:5}  {rng:22} "
                  + ' '.join(f"{t:8.2f}" for t in tops))
        print()

if __name__ == '__main__':
    main()
