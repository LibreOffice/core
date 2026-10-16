#!/usr/bin/env python3
"""Of two renderings that differ byte for byte, do they differ on the page?

A byte difference is not a visible one. A deck template commonly parks an instruction box
*outside* the slide, and text there is in the content stream, is re-broken by a layout change,
and is clipped away by every reader — `pdftotext` and PyMuPDF both report only what falls
inside the page box. So the byte reach of a change overstates its reach on paper, and the
difference between the two is worth stating rather than eliding.
"""
import sys

try:                       # the `fitz` alias prints a deprecation line on stdout
    import pymupdf as fitz
except ImportError:
    import fitz

def spans(path):
    out = []
    doc = fitz.open(path)
    for pno, page in enumerate(doc):
        for b in page.get_text('dict')['blocks']:
            if b['type'] != 0: continue
            for line in b['lines']:
                for s in line['spans']:
                    t = s['text'].rstrip()
                    if t:
                        out.append((pno, round(s['origin'][1], 2), round(s['origin'][0], 2), t))
    doc.close()
    return sorted(out)

a, b = spans(sys.argv[1]), spans(sys.argv[2])
print('VISIBLE-SAME' if a == b else 'VISIBLE-DIFFER')
