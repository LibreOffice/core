#!/usr/bin/env python3
"""One-attribute variants of the O91 witness, 086_Printable_Graph_Paper_Template_Gray_Theme.

Every mutation is applied to *both* `mc:Choice` (the `wps:wsp` LibreOffice reads) and
`mc:Fallback` (the VML twin) so the two branches never disagree; and every arm changes exactly
one attribute against `asis`.
"""
import pathlib
import re
import shutil
import sys
import zipfile

SRC = pathlib.Path('/home/user/sample-files/words/chartset-006/docx/'
                   '086_Printable_Graph_Paper_Template_Gray_Theme_7300e5d7.docx')
OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'fixtures')

BREAK_RUN = ('<w:r><w:rPr><w:rFonts w:ascii="Century Gothic" w:hAnsi="Century Gothic"/>'
             '<w:color w:val="000000" w:themeColor="text1"/><w:sz w:val="4"/>'
             '<w:szCs w:val="4"/><w:lang w:val="en-US"/></w:rPr><w:br/></w:r>')
PARA_RPR = ('<w:rPr><w:rFonts w:ascii="Century Gothic" w:hAnsi="Century Gothic"/>'
            '<w:color w:val="000000" w:themeColor="text1"/><w:lang w:val="en-US"/></w:rPr>'
            '</w:pPr>')


RUN = re.compile(r'<w:r>(?:(?!<w:r>|</w:r>).)*?<w:br/></w:r>', re.S)


def keep(doc, k):
    """Keep only the first k of each body's four break runs."""
    runs = list(RUN.finditer(doc))
    assert len(runs) == 16, len(runs)               # 4 bodies x 4 break runs
    res, last = [], 0
    for i, m in enumerate(runs):
        res.append(doc[last:m.start()])
        if i % 4 < k:
            res.append(m.group(0))
        last = m.end()
    res.append(doc[last:])
    return ''.join(res)


def break_size(doc, half_points):
    def sub(m):
        return (m.group(0).replace('w:sz w:val="4"', 'w:sz w:val="%d"' % half_points)
                .replace('w:szCs w:val="4"', 'w:szCs w:val="%d"' % half_points))
    return RUN.sub(sub, doc)


def mark_size(doc, half_points):
    return doc.replace(PARA_RPR, PARA_RPR.replace('<w:lang w:val="en-US"/></w:rPr>',
                                                  '<w:sz w:val="%d"/><w:szCs w:val="%d"/>'
                                                  '<w:lang w:val="en-US"/></w:rPr>'
                                                  % (half_points, half_points)))


def anchor(doc, value):
    return doc.replace('anchor="ctr"', 'anchor="%s"' % value)


def cy(doc, value):
    return doc.replace('cy="414655"', 'cy="%d"' % value).replace(
        'height:32.65pt', 'height:%.2fpt' % (value / 12700.0))


def overflow(doc, value):
    if value is None:
        return doc.replace(' vertOverflow="overflow"', '')
    return doc.replace('vertOverflow="overflow"', 'vertOverflow="%s"' % value)


ARMS = {
    'asis':            lambda d: d,
    # A -- k of four breaks kept, authored anchor="ctr"
    'k0':              lambda d: keep(d, 0),
    'k1':              lambda d: keep(d, 1),
    'k2':              lambda d: keep(d, 2),
    'k3':              lambda d: keep(d, 3),
    'k4':              lambda d: keep(d, 4),
    # A' -- the same sweep with the body top-anchored, so the sum of the break heights is the
    # displacement of the text line and no centring halves it
    'topk0':           lambda d: anchor(keep(d, 0), 't'),
    'topk1':           lambda d: anchor(keep(d, 1), 't'),
    'topk2':           lambda d: anchor(keep(d, 2), 't'),
    'topk3':           lambda d: anchor(keep(d, 3), 't'),
    'topk4':           lambda d: anchor(keep(d, 4), 't'),
    # B -- the break RUN's size, top-anchored, one break only (so the body still fits)
    'bsz4':            lambda d: anchor(break_size(keep(d, 1), 4), 't'),
    'bsz8':            lambda d: anchor(break_size(keep(d, 1), 8), 't'),
    'bsz12':           lambda d: anchor(break_size(keep(d, 1), 12), 't'),
    'bsz16':           lambda d: anchor(break_size(keep(d, 1), 16), 't'),
    'bsz22':           lambda d: anchor(break_size(keep(d, 1), 22), 't'),
    'bsz40':           lambda d: anchor(break_size(keep(d, 1), 40), 't'),
    # C -- the PARAGRAPH MARK's size, break run left at 4, one break, top-anchored
    'msz4':            lambda d: anchor(keep(mark_size(d, 4), 1), 't'),
    'msz22':           lambda d: anchor(keep(mark_size(d, 22), 1), 't'),
    'msz40':           lambda d: anchor(keep(mark_size(d, 40), 1), 't'),
    'msz80':           lambda d: anchor(keep(mark_size(d, 80), 1), 't'),
    # D -- anchor, as authored otherwise
    'anchor-t':        lambda d: anchor(d, 't'),
    'anchor-b':        lambda d: anchor(d, 'b'),
    # E -- whether the body fits
    'tall':            lambda d: cy(d, 1414655),
    'tall-k0':         lambda d: cy(keep(d, 0), 1414655),
    # F -- vertOverflow
    'ovf-clip':        lambda d: overflow(d, 'clip'),
    'ovf-absent':      lambda d: overflow(d, None),
    'ovf-ellipsis':    lambda d: overflow(d, 'ellipsis'),
}


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(SRC) as z:
        names = z.namelist()
        blobs = {n: z.read(n) for n in names}
    doc = blobs['word/document.xml'].decode('utf8')
    for name, fn in sorted(ARMS.items()):
        mutated = fn(doc)
        if name not in ('asis', 'k4') and mutated == doc:
            raise SystemExit('arm %s changed nothing' % name)
        target = OUT / ('%s.docx' % name)
        with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as z:
            for n in names:
                z.writestr(n, mutated.encode('utf8') if n == 'word/document.xml' else blobs[n])
        print('%-12s %8d bytes  brs=%d' % (name, target.stat().st_size,
                                           mutated.count('<w:br/>')))


if __name__ == '__main__':
    main()
