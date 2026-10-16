#!/usr/bin/env python3
"""Two censuses over the converted `.odt` column.

`split` — every `draw:frame` whose text box states `fo:min-height` (which is what makes the frame
grow to its text), with the `loext:may-break-between-pages` its *element* carries, what its content
is made of, and how much text it holds.  Cross-referenced against a sweep's rows so the reach is
counted in failing rows rather than in frames.

`wm` — every `writing-mode` attribute on a `style:table-cell-properties`, by spelling and value.
"""
import collections
import csv
import pathlib
import re
import sys
import zipfile
from xml.etree import ElementTree as ET

NS = {
    'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
    'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
    'draw': 'urn:oasis:names:tc:opendocument:xmlns:drawing:1.0',
    'text': 'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
    'table': 'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
    'fo': 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
    'svg': 'urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0',
    'loext': 'urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0',
}
Q = lambda p: '{%s}%s' % (NS[p.split(':')[0]], p.split(':')[1])

CORPUS = pathlib.Path('/home/user/corpus-odf/words')


def rows(path):
    """A sweep's rows.tsv keyed by document basename."""
    out = {}
    with open(path, encoding='utf-8') as fh:
        for row in csv.reader(fh, delimiter='\t'):
            if len(row) < 8:
                continue
            out[pathlib.Path(row[0]).name] = row
    return out


def frames(doc):
    z = zipfile.ZipFile(doc)
    root = ET.fromstring(z.read('content.xml'))
    for frame in root.iter(Q('draw:frame')):
        box = frame.find(Q('draw:text-box'))
        if box is None or box.get(Q('fo:min-height')) is None:
            continue
        tables = [k for k in box if k.tag == Q('table:table')]
        paras = [k for k in box
                 if k.tag in (Q('text:p'), Q('text:h'), Q('text:list'))]
        yield {
            'split': frame.get(Q('loext:may-break-between-pages'))
                     or frame.get(Q('draw:may-break-between-pages')),
            'anchor': frame.get(Q('text:anchor-type')),
            'height': frame.get(Q('svg:height')),
            'minheight': box.get(Q('fo:min-height')),
            'tables': len(tables),
            'rows': sum(1 for _ in box.iter(Q('table:table-row'))),
            'paras': len(paras),
            'chars': len(''.join(box.itertext())),
        }


def split_census(rowfile):
    scored = rows(rowfile) if rowfile else {}
    docs = sorted(CORPUS.glob('*/odt/*.odt'))
    total = held = 0
    bydoc = []
    for doc in docs:
        try:
            found = list(frames(doc))
        except Exception as exc:                      # a corpus file is taken as found
            print(f'! {doc.name}: {exc}', file=sys.stderr)
            continue
        if not found:
            continue
        total += len(found)
        splittable = [f for f in found if f['split'] == 'true']
        held += len(splittable)
        row = scored.get(doc.name)
        bydoc.append((doc.name, found, splittable, row))

    print(f'{total} growing frames in {len(bydoc)} documents; '
          f'{held} state may-break-between-pages')

    shape = collections.Counter()
    for _, _, splittable, _ in bydoc:
        for f in splittable:
            shape['table only' if f['tables'] and not f['paras'] else
                  'paragraphs only' if f['paras'] and not f['tables'] else
                  'mixed' if f['paras'] and f['tables'] else 'empty'] += 1
    print('  splittable frames by content:', dict(shape))

    verdicts = collections.Counter()
    print()
    print('document\tverdict\tpages\tglyphs\tsplittable\tcontent\trows\tchars')
    for name, found, splittable, row in sorted(bydoc):
        verdict = row[6] if row else '-'
        verdicts[verdict] += 1
        content = ','.join(
            ('table' if f['tables'] and not f['paras'] else
             'mixed' if f['tables'] else 'paras') for f in splittable) or '-'
        print(f'{name}\t{verdict}\t{row[2] if row else "-"}\t{row[8] if row else "-"}\t'
              f'{len(splittable)}/{len(found)}\t{content}\t'
              f'{sum(f["rows"] for f in splittable)}\t{sum(f["chars"] for f in splittable)}')
    print()
    print('  verdicts of the documents holding a growing frame:', dict(verdicts))
    failing = [n for n, _, s, r in bydoc if s and r and r[6] != 'match']
    print(f'  {len(failing)} of them hold a splittable frame AND fail the gate')


if __name__ == '__main__':
    split_census(sys.argv[1] if len(sys.argv) > 1 else None)
