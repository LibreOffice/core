#!/usr/bin/env python3
"""Dump the text layer of one side, one line per text line, with position, for diffing."""
import sys, pymupdf, census as C
d = pymupdf.open(C.only_pdf(sys.argv[1]))
for pno, p in enumerate(d):
    for b in p.get_text('dict')['blocks']:
        if b['type'] != 0: continue
        for ln in b['lines']:
            t = ''.join(s['text'] for s in ln['spans'])
            if t.strip():
                print('p%d %7.1f %7.1f  %s' % (pno+1, ln['bbox'][0], ln['bbox'][1], t))
