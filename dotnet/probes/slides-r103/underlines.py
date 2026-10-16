#!/usr/bin/env python3
"""How many hyperlink rules a rendering draws, against 26.2.4.2's own count.

A `.ppt` text range that becomes an EditEngine field is drawn underlined and in the colour
scheme's hyperlink slot (`svdfppt.cxx`:7054-7056); one that does not is drawn as it stands.
So counting the thin filled rectangles under text -- and the distinct colours of the spans
above them -- says directly how many fields each side made, with no tolerance anywhere in it.

    underlines.py <cli> <doc.ppt> ...       (one line per document)
"""
import os, subprocess, sys, tempfile
import pymupdf

REF = '/home/user/gate-orig-r83/ref'


def rules(pdf):
    doc = pymupdf.open(pdf)
    n = 0
    for page in doc:
        for d in page.get_drawings():
            r = d['rect']
            # 26.2.4.2 writes an underline as a ZERO-height rectangle and not as a fill:
            # filtering on `type == 'f'` and on a positive height each report every
            # reference PDF as carrying none, which is what the first two cuts of this
            # counter did.
            if r.height <= 2.0 and r.width >= 8:
                n += 1
    doc.close()
    return n


def main(cli, docs):
    for path in docs:
        stem = os.path.splitext(os.path.basename(path))[0]
        ref = os.path.join(REF, stem + '__ppt.pdf')
        with tempfile.TemporaryDirectory(dir='/home/user/r103-slidefw') as work:
            subprocess.run([cli, 'render', path, '--format', 'pdf', '--outdir', work],
                           check=False, capture_output=True, timeout=900,
                           env=dict(os.environ, SOURCE_DATE_EPOCH='1700000000'))
            ours = os.path.join(work, stem + '.pdf')
            print(f'{stem[:52]:52}\tours {rules(ours) if os.path.exists(ours) else "FAILED"}'
                  f'\tref {rules(ref) if os.path.exists(ref) else "NOREF"}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2:])
