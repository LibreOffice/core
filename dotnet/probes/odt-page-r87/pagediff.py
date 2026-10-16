#!/usr/bin/env python3
"""Align two PDFs page by page on their text and report where they first drift apart.

Prints, for each page of A, the page of B whose text best matches its opening line, so a
one-page insertion shows as the offset stepping by one at the page that caused it.
"""
import subprocess, sys, re

def pages(path):
    out = subprocess.run(['pdftotext', '-layout', path, '-'], capture_output=True).stdout
    return out.decode('utf-8', 'replace').split('\f')[:-1]

def norm(s):
    return re.sub(r'\s+', ' ', s).strip()

def key(p, n=3):
    ls = [norm(l) for l in p.splitlines() if norm(l)]
    return ' | '.join(ls[:n])

a, b = pages(sys.argv[1]), pages(sys.argv[2])
print(f"A {sys.argv[1]}: {len(a)} pages")
print(f"B {sys.argv[2]}: {len(b)} pages")
bi = 0
prev_off = 0
for i, pa in enumerate(a):
    ka = key(pa)
    # search B forwards from bi for the best match of A's first lines
    best, bestj = 0.0, None
    for j in range(max(0, bi - 3), min(len(b), bi + 12)):
        kb = key(b[j])
        if not ka and not kb:
            s = 1.0
        else:
            sa, sb = set(ka.split()), set(kb.split())
            s = len(sa & sb) / max(1, len(sa | sb))
        if s > best:
            best, bestj = s, j
    if bestj is None:
        continue
    off = bestj - i
    if off != prev_off:
        print(f"  A p{i+1:>4} -> B p{bestj+1:<4} offset {off:+d} (was {prev_off:+d}) sim {best:.2f}")
        print(f"      A: {ka[:120]}")
        print(f"      B: {key(b[bestj])[:120]}")
        prev_off = off
    bi = bestj + 1
print("final offset", prev_off)
