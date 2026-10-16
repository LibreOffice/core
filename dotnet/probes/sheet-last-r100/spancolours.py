#!/usr/bin/env python3
"""Every text span's colour in a PDF, tallied — the instrument for a run-colour claim."""
import sys, pymupdf, collections
for tag, path in (('a', sys.argv[1]), ('b', sys.argv[2])):
    d = pymupdf.open(path)
    c = collections.Counter()
    for p in d:
        for blk in p.get_text('dict')['blocks']:
            for ln in blk.get('lines', []):
                for sp in ln['spans']:
                    if not sp['text'].strip():
                        continue
                    c['#%06x' % sp['color']] += 1
    print('%-8s %s' % (tag, path))
    for k, v in c.most_common(8):
        print('     %-10s %d' % (k, v))
    print('     non-black spans %d' % sum(v for k, v in c.items() if k != '#000000'))
    d.close()
