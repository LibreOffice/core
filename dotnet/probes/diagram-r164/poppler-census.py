#!/usr/bin/env python3
"""The census that decides the whole class: the same pages read by an extractor that
honours PDF clip paths (MuPDF) and one that does not (poppler).

If the reference's alnum count rises to ours under poppler, the reference LAID OUT AND
EMITTED the same characters we did and merely wrapped the overflow in a clip path; the
difference is representation, not content, and the defect is the missing clip, not
invented text.
"""
import pathlib, subprocess
import pymupdf
import census as C


def mupdf(pdf):
    d = pymupdf.open(pdf)
    n = sum(sum(1 for c in p.get_text() if c.isalnum()) for p in d)
    d.close()
    return n


def poppler(pdf):
    out = subprocess.run(['pdftotext', '-q', pdf, '-'], capture_output=True)
    return sum(1 for c in out.stdout.decode('utf8', 'replace') if c.isalnum())


here = pathlib.Path(__file__).parent
rows = [l.split('\t') for l in (here / 'docs.tsv').read_text().split('\n') if l.strip()]
print('%-54s %6s %6s %6s | %8s %8s %8s' % ('stem', 'ourMu', 'refMu', 'delta',
                                           'ourPop', 'refPop', 'delta'))
for stem, _ in rows:
    o = C.only_pdf(str(here / 'ours' / stem)); r = C.only_pdf(str(here / 'ref' / stem))
    om, rm, op, rp = mupdf(o), mupdf(r), poppler(o), poppler(r)
    print('%-54s %6d %6d %+6d | %8d %8d %+8d' % (stem[:54], om, rm, om - rm, op, rp, op - rp))
