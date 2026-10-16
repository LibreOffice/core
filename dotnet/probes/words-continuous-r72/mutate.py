#!/usr/bin/env python3
"""Build one-attribute mutations of hdss-bulletin-issue-285 to settle what a
continuous section break does with its header and its w:pgMar.

Usage: mutate.py <outdir>
"""
import re, shutil, sys, zipfile
from pathlib import Path

SRC = Path('/home/user/sample-files/words/done-013/docx/'
           'hdss-bulletin-issue-285-25-june-2025.docx')

def sectprs(doc):
    return [m.span() for m in re.finditer(r'<w:sectPr[ >].*?</w:sectPr>', doc, re.S)]

def edit_sect(doc, n, fn):
    spans = sectprs(doc)
    a, b = spans[n]
    return doc[:a] + fn(doc[a:b]) + doc[b:]

def write(out, name, doc):
    dst = out / (name + '.docx')
    shutil.copy(SRC, dst)
    zin = zipfile.ZipFile(SRC)
    zout = zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED)
    for it in zin.infolist():
        data = zin.read(it.filename)
        if it.filename == 'word/document.xml':
            data = doc.encode('utf-8')
        zout.writestr(it, data)
    zout.close()
    print(name)

def main():
    out = Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
    base = zipfile.ZipFile(SRC).read('word/document.xml').decode('utf-8')

    write(out, 'base', base)

    # section 2 (index 1) declares no header at all -> every link-to-previous
    write(out, 's2-nohdr',
          edit_sect(base, 1, lambda s: re.sub(r'<w:headerReference[^/]*/>', '', s)))

    # section 2's own top margin made absurd; if it is inert the page cannot move
    write(out, 's2-top3000',
          edit_sect(base, 1, lambda s: s.replace('w:top="1418"', 'w:top="3000"')))

    # section 1 (index 0) loses its header: measures the empty header's height
    write(out, 's1-nohdr',
          edit_sect(base, 0, lambda s: re.sub(r'<w:headerReference[^/]*/>', '', s)))

    # section 1's own top margin moved by a known amount
    write(out, 's1-top1000',
          edit_sect(base, 0, lambda s: s.replace('w:top="454"', 'w:top="1000"')))

    # section 3 (index 2, the body-level sectPr) top margin moved
    write(out, 's3-top3000',
          edit_sect(base, 2, lambda s: s.replace('w:top="1418"', 'w:top="3000"')))

    # a page break inside section 2 gives its page style somewhere to land
    m = re.search(r'<w:p [^>]*w:rsidR="00B519CD"[^>]*>', base)
    doc = base
    # first Body paragraph after the section-1 sectPr paragraph's closing table
    i = doc.index('</w:tbl>')
    j = doc.index('<w:p ', i)
    k = doc.index('>', j) + 1
    ins = '<w:pPr><w:pageBreakBefore/></w:pPr>'
    # the paragraph may already have a pPr
    tail = doc[k:k + 200]
    if tail.startswith('<w:pPr>'):
        doc = doc[:k + len('<w:pPr>')] + '<w:pageBreakBefore/>' + doc[k + len('<w:pPr>'):]
    else:
        doc = doc[:k] + ins + doc[k:]
    write(out, 's2-pagebreak', doc)

if __name__ == '__main__':
    main()
