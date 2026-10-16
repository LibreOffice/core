#!/usr/bin/env python3
"""Where each arm's one block of text sits, from the PDF's own text origin."""
import glob
import os
import sys

import pymupdf

for path in sorted(glob.glob(os.path.join(sys.argv[1] if len(sys.argv) > 1 else 'out', '*.pdf'))):
    page = pymupdf.open(path)[0]
    spans = [s for b in page.get_text('dict')['blocks'] if b.get('lines')
             for l in b['lines'] for s in l['spans']]
    if len(spans) != 1:
        raise SystemExit('%s: expected one span, found %d' % (path, len(spans)))
    box = spans[0]['bbox']
    print('%-12s x %8.3f   y %8.3f   %r'
          % (os.path.basename(path)[:-4], box[0], box[1], spans[0]['text']))
