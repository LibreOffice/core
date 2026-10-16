#!/usr/bin/env python3
"""Which corpus DOCX carry a continuous section, and whether its page descriptor
can land anywhere.

A continuous section gets a page descriptor of its own only when it names a
header or footer reference (`SectionPropertyMap::CloseSectionGroup`'s
`!m_b*LinkToPrevious` guard, `dmapper/PropertyMap.cxx`:1746-1752), and that
descriptor is then hung on the first paragraph inside the section carrying an
explicit page break.  With no such paragraph the descriptor reaches no page and
the section keeps the previous one's — geometry, header and footer together.
"""
import sys, zipfile
import xml.etree.ElementTree as ET
from pathlib import Path

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
CORPUS = Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files/words')

def has_break(el):
    ppr = el.find(W + 'pPr')
    if ppr is not None and ppr.find(W + 'pageBreakBefore') is not None:
        pbb = ppr.find(W + 'pageBreakBefore')
        if pbb.get(W + 'val') not in ('0', 'false'):
            return True
    for br in el.iter(W + 'br'):
        if br.get(W + 'type') == 'page':
            return True
    return False

def report(path):
    try:
        with zipfile.ZipFile(path) as z:
            doc = z.read('word/document.xml')
    except Exception:
        return None
    try:
        body = ET.fromstring(doc).find(W + 'body')
    except Exception:
        return None
    if body is None:
        return None

    children = list(body)
    # section boundaries: a child paragraph carrying a w:sectPr ends a section;
    # the body-level w:sectPr ends the last one.
    bounds, secprs = [], []
    for k, ch in enumerate(children):
        ppr = ch.find(W + 'pPr') if ch.tag == W + 'p' else None
        s = ppr.find(W + 'sectPr') if ppr is not None else None
        if s is not None:
            bounds.append(k); secprs.append(s)
    tail = body.find(W + 'sectPr')
    if tail is not None:
        bounds.append(len(children) - 1); secprs.append(tail)

    rows = []
    start = 0
    for i, (end, sp) in enumerate(zip(bounds, secprs)):
        t = sp.find(W + 'type')
        kind = t.get(W + 'val') if t is not None else 'nextPage'
        own = any(e.tag in (W + 'headerReference', W + 'footerReference') for e in sp)
        brk = any(has_break(children[j]) for j in range(start, min(end + 1, len(children))))
        rows.append((i, kind, own, brk))
        start = end + 1
    return rows

def main():
    docs = sorted(p for p in CORPUS.rglob('*') if p.suffix.lower() == '.docx')
    tot = dropped = deferred = 0
    print('doc\tsection\tkind\townFurniture\thasPageBreak\tverdict')
    for p in docs:
        rows = report(p)
        if not rows:
            continue
        hit = False
        for i, kind, own, brk in rows:
            if i == 0 or kind != 'continuous':
                continue
            verdict = ('lands-at-break' if own and brk
                       else 'dropped')
            print(f'{p.name}\t{i}\t{kind}\t{int(own)}\t{int(brk)}\t{verdict}')
            hit = True
            if verdict == 'dropped':
                dropped += 1
            else:
                deferred += 1
        tot += hit
    print(f'# docx scanned {len(docs)}; documents with a non-first continuous section {tot}; '
          f'sections dropped {dropped}; sections landing at a break {deferred}')

if __name__ == '__main__':
    main()
