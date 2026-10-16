#!/usr/bin/env python3
"""Print every text span's colour, keyed on the string it draws."""
import sys
import pymupdf
d = pymupdf.open(sys.argv[1])
for pi in range(d.page_count):
    for b in d[pi].get_text('dict')['blocks']:
        for l in b.get('lines', []):
            for s in l['spans']:
                t = s['text'].strip()
                if t:
                    print('%-14s #%06X' % (t, s['color']))
d.close()
