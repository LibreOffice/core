#!/usr/bin/env python3
"""Does 26.2.4.2 clip an overflowing text block, or draw it and let the page clip?

Counts every text show whose baseline lies outside [0, page height] -- above the top or
below the bottom -- over the reference bank's 51 .ppt and over ours.  A renderer that
clipped a block to its shape, or shrank it, or dropped the lines that did not fit, could
not produce one.

    offpage.py <shows-directory>
"""
import collections, glob, os, subprocess, sys

root = sys.argv[1]
tot = {'ref': [0, 0, 0], 'ours': [0, 0, 0]}
docs = {'ref': collections.Counter(), 'ours': collections.Counter()}
for f in sorted(glob.glob(os.path.join(root, '*.ref.tsv'))):
    stem = os.path.basename(f)[:-len('.ref.tsv')]
    bank = f'/home/user/gate-orig-r83/ref/{stem}__ppt.pdf'
    info = subprocess.run(['pdfinfo', bank], capture_output=True, text=True).stdout
    h = float([l for l in info.splitlines() if l.startswith('Page size')][0].split()[-2])
    for tag, path in (('ref', f), ('ours', f.replace('.ref.tsv', '.ours.tsv'))):
        try: lines = open(path).read().splitlines()
        except FileNotFoundError: continue
        if lines and lines[0] == 'FAILED': continue
        for ln in lines:
            c = ln.split('\t')
            if len(c) < 5: continue
            y = float(c[4])
            tot[tag][0] += 1
            if y > h: tot[tag][1] += 1; docs[tag][stem] += 1
            elif y < 0: tot[tag][2] += 1; docs[tag][stem] += 1

for tag in ('ref', 'ours'):
    s, a, b = tot[tag]
    print(f"{tag:5} shows {s:6}   baseline above the page {a:4}   below it {b:4}"
          f"   ({a + b}, {100 * (a + b) / s:.2f}%)   in {len(docs[tag])} of 51 documents")
print()
for stem in sorted(set(docs['ref']) | set(docs['ours'])):
    print(f"   {stem[:60]:60} ref {docs['ref'][stem]:4}   ours {docs['ours'][stem]:4}")
