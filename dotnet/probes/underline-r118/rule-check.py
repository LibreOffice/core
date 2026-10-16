#!/usr/bin/env python3
"""Measure the link-coloured rules on both sides of every document the change moved.

Two statistics, and the second is the one to read. A *count* of rules is a count of drawing
objects, and the two sides do not object-ify a rule the same way: 26.2.4.2 draws a hyperlink
cell as one field and strokes one rule across it, while this tree keeps the cell's rich
segments and fills one rule under each. On `fm-provider-service-measures.xlsx` page 33 the
reference strokes x 113.3-373.9 once and this tree fills 113.3-138.9 and 141.9-374.3 -- the
same rule, the same ink, two objects. So the count says 2 against 1 and the covered length
says 261.0 against 260.6, and only the second is about the page. `render-comparison`'s own
rule -- rank on ink, never on object counts -- in miniature.

An underline is ink and no gate sees it, so the only honest "did it get closer" figure is a
count of the rules themselves.  The reference *strokes* them and this tree *fills* them --
`PDFWriterImpl::drawStraightTextLine` (`vcl/source/pdf/pdfwriter_impl.cxx:6658-6760`) emits a
stroke of the metric's own thickness, `SheetTextLayout.Rule` emits a rectangle of it -- so a
census that looks only at strokes scores every one of ours as absent.  Both shapes are
counted here, keyed on the colour and on being a thin horizontal band.

`#000080` is the application's LINKS colour and is what a hyperlink cell's rule is painted
in on both sides, so it separates the rules this seat is about from every other thin rule on
the page.
"""
import glob, os, shutil, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor
import pymupdf

SOFFICE = '/opt/libreoffice26.2/program/soffice'
CLI = sys.argv[1]
DOCS = [l.strip() for l in open(sys.argv[2]) if l.strip()]
NAVY = (0.0, 0.0, 0.502)


def near(c):
    return c is not None and all(abs(a - b) < 0.01 for a, b in zip(c, NAVY))


def rules(pdf):
    doc = pymupdf.open(pdf)
    n = 0
    length = 0.0
    for pi in range(doc.page_count):
        for d in doc[pi].get_drawings():
            r = d['rect']
            if r.width < 3 or r.height > 3:
                continue
            if near(d.get('color')) or near(d.get('fill')):
                n += 1
                length += r.width
    doc.close()
    return n, length


def one(path):
    tmp = tempfile.mkdtemp(prefix='rule-')
    stem = os.path.splitext(os.path.basename(path))[0]
    try:
        a = b = (-1, -1.0)
        subprocess.run(['timeout', '-k', '30', '900', SOFFICE,
                        '-env:UserInstallation=file://' + tmp + '/p', '--headless', '--norestore',
                        '--convert-to', 'pdf', '--outdir', tmp + '/r', path], capture_output=True)
        got = glob.glob(tmp + '/r/*.pdf')
        if got:
            b = rules(got[0])
        subprocess.run(['timeout', '-k', '30', '900', CLI, 'render', path,
                        '--format', 'pdf', '--outdir', tmp + '/o'],
                       capture_output=True, env=dict(os.environ, SOURCE_DATE_EPOCH='1700000000'))
        got = glob.glob(tmp + '/o/*.pdf')
        if got:
            a = rules(got[0])
        return (os.path.basename(path), a[0], a[1], b[0], b[1])
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


print('doc\tours_rules\tours_length\tref_rules\tref_length')
with ThreadPoolExecutor(3) as pool:
    for name, an, al, bn, bl in pool.map(one, DOCS):
        print('%s\t%d\t%.1f\t%d\t%.1f' % (name, an, al, bn, bl))
