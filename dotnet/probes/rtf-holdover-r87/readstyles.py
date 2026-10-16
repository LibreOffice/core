#!/usr/bin/env python3
"""Read one probe rendering: every span's text, top and size, and which page each
of HEAD/TAILWORD landed on. Usage: readstyles.py <pdf>"""
import sys, pymupdf

d = pymupdf.open(sys.argv[1])
out = []
for bl in d[0].get_text('dict')['blocks']:
    for line in bl.get('lines', []):
        for s in line['spans']:
            out.append(f"{s['text'].strip()}@{s['bbox'][1]:.2f}/{s['size']:.1f}")
pages = {w: [i for i in range(d.page_count) if w in d[i].get_text()]
         for w in ('HEAD', 'TAILWORD')}
print(' '.join(out), '|', d.page_count, 'pages', pages if d.page_count > 1 else '')
