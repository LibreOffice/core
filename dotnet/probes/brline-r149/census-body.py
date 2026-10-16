#!/usr/bin/env python3
"""The same population OUTSIDE a shape -- ordinary `<w:br/>` in page-body paragraphs.

Section 2.5 measures the rule as identical in a Writer paragraph and in a shape body, so the
population a fix touches is not only `w:txbxContent`.  Counted separately because whether this
tree is also wrong there could not be measured this round.

And the converted ODF twin: `text:line-break` in `/home/user/corpus-odf/odt`.  There is no `odp`
column at all -- a census over it returns 0 occurrences AND 0 base-rate tokens, which is the
tell -- and the `rtf` column, contrary to the brief, does exist with 337 files.
"""
import collections
import pathlib
import re
import xml.etree.ElementTree as ET
import zipfile

import census as C

W = C.W
ODF = pathlib.Path('/home/user/corpus-odf')
TEXT = '{urn:oasis:names:tc:opendocument:xmlns:text:1.0}'
DRAW = '{urn:oasis:names:tc:opendocument:xmlns:drawing:1.0}'


def body_breaks(path):
    stats = collections.Counter()
    with zipfile.ZipFile(path) as pkg:
        styles, default = C.read_styles(pkg)
        for name in pkg.namelist():
            if not C.PARTS.match(name):
                continue
            try:
                root = ET.fromstring(pkg.read(name))
            except ET.ParseError:
                continue
            inside = {p for b in root.iter(W + 'txbxContent') for p in b.iter(W + 'p')}
            for para in root.iter(W + 'p'):
                if para in inside:
                    continue
                stats['paras'] += 1
                ppr = para.find(W + 'pPr')
                mark = C.sz_of(ppr.find(W + 'rPr') if ppr is not None else None, styles, default)
                if ppr is not None:
                    ps = ppr.find(W + 'pStyle')
                    if ps is not None and ppr.find(W + 'rPr/' + W + 'sz') is None:
                        mark = styles.get(ps.get(W + 'val'), mark)
                for run in para.findall(W + 'r'):
                    brs = [b for b in run.findall(W + 'br')
                           if b.get(W + 'type') in (None, 'textWrapping')]
                    if not brs:
                        continue
                    stats['breaks'] += len(brs)
                    if C.sz_of(run.find(W + 'rPr'), styles, default) != mark:
                        stats['differ'] += len(brs)
    return stats


def odf_column(kind, tag):
    d = ODF / kind
    total = collections.Counter()
    docs = collections.Counter()
    files = sorted(d.glob('*.' + kind)) if d.is_dir() else []
    for path in files:
        try:
            with zipfile.ZipFile(path) as pkg:
                names = [n for n in ('content.xml', 'styles.xml') if n in pkg.namelist()]
                blob = b''.join(pkg.read(n) for n in names)
                content = pkg.read('content.xml') if 'content.xml' in names else b''
        except zipfile.BadZipFile:
            continue
        try:
            root = ET.fromstring(content) if content else None
        except ET.ParseError:
            root = None
        total['docs'] += 1
        n = blob.count(b'<text:line-break')
        total['spans'] += blob.count(b'<text:span')
        total['breaks'] += n
        if n:
            docs['with'] += 1
        if root is not None:
            # any drawing object's text body, not just draw:frame -- LibreOffice's own
            # converter writes these shapes as draw:custom-shape
            inframe = sum(1 for f in root.iter()
                          if f.tag.startswith(DRAW)
                          for _ in f.iter(TEXT + 'line-break'))
            total['in_draw'] += inframe
            if inframe:
                docs['with_draw'] += 1
    return total, docs, len(files)


def main():
    total = collections.Counter()
    docs = 0
    with_diff = 0
    files = sorted(p for p in C.CORPUS.rglob('*')
                   if p.suffix.lower() in ('.docx', '.docm', '.dotx', '.dotm'))
    for path in files:
        try:
            s = body_breaks(path)
        except zipfile.BadZipFile:
            continue
        docs += 1
        total += s
        if s['differ']:
            with_diff += 1
    print('# the same rule OUTSIDE a shape, over the same %d documents' % docs)
    print('page-body paragraphs                %6d' % total['paras'])
    print('<w:br/> in them                     %6d' % total['breaks'])
    print('... run size differs from the mark  %6d   in %d documents'
          % (total['differ'], with_diff))
    print()
    for kind in ('odt', 'ods', 'odp'):
        t, d, n = odf_column(kind, TEXT + 'line-break')
        print('# /home/user/corpus-odf/%-4s  %d files on disk' % (kind, n))
        if not n:
            print('    ABSENT -- 0 occurrences and 0 base-rate tokens, which is the tell')
            continue
        print('    documents read %d, <text:span> %d (base rate), <text:line-break> %d '
              'in %d documents, of which %d inside a <draw:frame> in %d documents'
              % (t['docs'], t['spans'], t['breaks'], d['with'], t['in_draw'], d['with_draw']))


if __name__ == '__main__':
    main()


# rtf: 337 files are on disk but an RTF is not an ODF package, so the reader above reads none of
# them.  Its analogue of this rule is `\line` inside a `\shptxt`, on a different importer, and
# it is out of this round's scope.
