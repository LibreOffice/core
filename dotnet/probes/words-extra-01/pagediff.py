#!/usr/bin/env python3
"""Per-page token diff between two PDFs. Tokens = whitespace-split, letter/digit-bearing."""
import subprocess, sys, collections, unicodedata

def words(pdf, page):
    out = subprocess.run(['pdftotext', '-f', str(page), '-l', str(page), pdf, '-'],
                         capture_output=True, text=True).stdout
    return [t for t in out.split() if any(c.isalnum() for c in t)]

def npages(pdf):
    out = subprocess.run(['pdfinfo', pdf], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith('Pages:'):
            return int(line.split()[1])
    return 0

a, b = sys.argv[1], sys.argv[2]
pa, pb = npages(a), npages(b)
print(f"pages: ours={pa} ref={pb}")
ta = tb = 0
for i in range(1, max(pa, pb) + 1):
    wa = words(a, i) if i <= pa else []
    wb = words(b, i) if i <= pb else []
    ta += len(wa); tb += len(wb)
    ca, cb = collections.Counter(wa), collections.Counter(wb)
    extra = ca - cb
    missing = cb - ca
    if extra or missing:
        print(f"--- page {i}: ours={len(wa)} ref={len(wb)} delta={len(wa)-len(wb):+d}")
        if extra:
            print("    only-ours:", ' '.join(f"{k}x{v}" if v > 1 else k
                                             for k, v in sorted(extra.items()))[:400])
        if missing:
            print("    only-ref :", ' '.join(f"{k}x{v}" if v > 1 else k
                                             for k, v in sorted(missing.items()))[:400])
print(f"TOTAL ours={ta} ref={tb} delta={ta-tb:+d}")
