#!/usr/bin/env python3
"""Every corpus DOCX anchor holding an EMBEDDED object, with the three things that decide its base.

    ./census-embedded.py [--rows]

An embedded object is what `DomainMapper_Impl.cxx`:9792 reaches, and only it: a `wp:anchor` whose
`a:graphicData/@uri` names a chart or an OLE object. For each one this prints

  * `layoutInCell`  -- absent, 1 or 0. Absent is TRUE (`GraphicImport.cxx`:340), so the interesting
    column is how many state 0;
  * the `wp:positionV/@relativeFrom`, because `page` and `topMargin` are the two relations whose base
    the follow-text-flow substitution moves and only `page` is implemented; and
  * whether the anchor is inside a `w:tbl`, which is the half of the rule this tree still does not
    model -- there the object is captured in its cell.

Counted on the *element* rather than on a string, because `a:graphicData` also carries pictures,
shapes, groups, diagrams and ink, and only two of its URIs are an embedded object.
"""
import collections
import re
import sys
import pathlib
import zipfile

ROOT = pathlib.Path('/home/user/sample-files')
CHART = 'drawingml/2006/chart'
OLE = 'presentationml/2006/ole'          # PowerPoint's spelling, seen in DOCX in the wild
OLEOBJ = 'officeDocument/2006/oleObject'

ANCHOR = re.compile(rb'<wp:anchor\b.*?</wp:anchor>', re.S)
URI = re.compile(rb'<a:graphicData[^>]*\buri="([^"]*)"')
INCELL = re.compile(rb'\blayoutInCell="([^"]*)"')
POSV = re.compile(rb'<wp:positionV[^>]*\brelativeFrom="(\w+)"')
OFFSET = re.compile(rb'<wp:positionV.*?</wp:positionV>', re.S)

incell = collections.Counter()
relation = collections.Counter()
intable = collections.Counter()
docs = set()
rows = []


def embedded(uri: bytes) -> bool:
    text = uri.decode('utf-8', 'replace')
    return CHART in text or OLE in text or OLEOBJ in text


for path in sorted(ROOT.rglob('*')):
    if not path.is_file() or path.suffix.lower() not in ('.docx', '.docm', '.dotx'):
        continue
    try:
        package = zipfile.ZipFile(path)
    except Exception:
        continue
    for name in package.namelist():
        if not re.match(r'word/(document|header\d*|footer\d*)\.xml$', name):
            continue
        try:
            xml = package.read(name)
        except Exception:
            continue
        # A `w:tbl` the anchor falls inside: every open tag before it and its matching close after.
        for match in ANCHOR.finditer(xml):
            body = match.group(0)
            uri = URI.search(body)
            if uri is None or not embedded(uri.group(1)):
                continue
            before = xml[:match.start()]
            inside = before.count(b'<w:tbl>') > before.count(b'</w:tbl>')
            stated = INCELL.search(body)
            cell = 'absent' if stated is None else stated.group(1).decode()
            rel = POSV.search(body)
            base = 'absent' if rel is None else rel.group(1).decode()
            block = OFFSET.search(body)
            kind = ('offset' if block and b'<wp:posOffset>' in block.group(0)
                    else 'align' if block else 'absent')
            incell[cell] += 1
            relation[f'{base}/{kind}'] += 1
            intable['in a w:tbl' if inside else 'outside'] += 1
            docs.add(str(path.relative_to(ROOT)))
            rows.append((str(path.relative_to(ROOT)), name, cell, base, kind, inside))

print(f'embedded-object anchors {len(rows)} in {len(docs)} documents')
for title, counter in (('layoutInCell', incell), ('positionV', relation), ('anchor', intable)):
    print(f'  {title}: ' + '  '.join(f'{k}={v}' for k, v in sorted(counter.items())))

if '--rows' in sys.argv:
    for row in rows:
        print('\t'.join(str(field) for field in row))
