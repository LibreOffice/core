#!/usr/bin/env python3
"""The dominant drawn text size of every page of a rendering, and the OpenSymbol bullet beside it.

An autofit is not stated anywhere a reader can see, so the only way to compare two renderings'
answers is the sizes they actually draw: LibreOffice rounds an autofitted size to a whole point
(`SdrTextObj::setupAutoFitText`, `svx/source/svdraw/svdotext.cxx`:1231, "We need to round the
font size nearest integer pt size"), so the ratio of two dominant sizes is the ratio of two
fits.

"Dominant" is the size carrying the most alphanumeric characters on the page, which is the body
placeholder on a slide that has one -- the same statistic the census in
`XRay.Office.Presentations/TODO.md` used.
"""
import sys, collections

try:
    import pymupdf as fitz
except ImportError:
    import fitz

def dominant(path):
    doc = fitz.open(path)
    out = []
    for page in doc:
        weight = collections.Counter()
        for b in page.get_text('dict')['blocks']:
            if b['type'] != 0: continue
            for line in b['lines']:
                for s in line['spans']:
                    n = sum(1 for ch in s['text'] if ch.isalnum())
                    if n: weight[round(s['size'], 2)] += n
        out.append(weight.most_common(1)[0] if weight else (0.0, 0))
    doc.close()
    return out

if __name__ == '__main__':
    for path in sys.argv[1:]:
        for i, (size, n) in enumerate(dominant(path)):
            print(f'{path}\t{i + 1}\t{size}\t{n}')
