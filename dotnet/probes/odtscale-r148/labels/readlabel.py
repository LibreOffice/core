#!/usr/bin/env python3
"""Does 26.2.4.2 scale a LIST LABEL from the level's own character style?

The level states `style:num-suffix=""` and `text:label-followed-by="nothing"`, so the
paragraph's own `X` begins exactly at the label's right edge and the two are one span --
the label's drawn width is therefore readable without a separate origin.

`lab-ctl24` is the control and is not optional: an arm that does not move is evidence only
once a DIFFERENT attribute on the SAME style is shown to move. Without it, "the reference
ignores the scale" and "the probe never referenced the style" are the same picture.
"""
import glob
import os

import pymupdf

for path in sorted(glob.glob(os.path.join(os.path.dirname(__file__) or '.', 'out', '*.pdf'))):
    page = pymupdf.open(path)[0]
    spans = [s for b in page.get_text('dict')['blocks'] if b.get('lines')
             for l in b['lines'] for s in l['spans']]
    if not spans:
        raise SystemExit('%s: nothing drawn' % path)
    for span in spans:
        print('%-14s %-6r size %5.2f  width %7.3f  x %8.3f .. %8.3f'
              % (os.path.basename(path)[:-4], span['text'], span['size'],
                 span['bbox'][2] - span['bbox'][0], span['bbox'][0], span['bbox'][2]))
