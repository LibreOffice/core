#!/usr/bin/env python3
"""How many of the corpus's absolutely positioned objects sit inside a w:tbl.

`bCheckBottom = !DoesObjFollowsTextFlow()` (`tocntntanchoredobjectposition.cxx`:457) skips the
*bottom* half of the capture for an object that follows the text flow.  What decides that is
`IsFollowingTextFlow`, and two facts settle what it answers for a DOCX:

* its pool default is **false** — `{ RES_FOLLOW_TEXT_FLOW, new SwFormatFollowTextFlow(false), ... }`,
  `sw/source/core/bastyp/init.cxx`:437 — and no writerfilter path calls `SwDoc::SetDefault` for it
  (the only two that do are the HTML filter, `swhtml.cxx`:370, and WW8, `ww8graf.cxx`:2430 and
  `ww8par2.cxx`:3454, both per object rather than as a default);
* writerfilter writes `PROP_FOLLOW_TEXT_FLOW` at exactly three seats and every one of them is
  gated on being in a table — `GraphicImport.cxx`:1316-1318 and :1859-1861 on
  `m_rDomainMapper.IsInTable()`, `OOXMLFastContextHandler.cxx`:1879-1883 on `mnTableDepth > 0`.

So for a DOCX the question `bCheckBottom` asks of an object is exactly *is this anchor inside a
table*, and this counts that over the corpus.  Headers and footers are scanned as well as the
body, because two of the ten documents the wide capture rule moved anchor only there.

    anchor-census.py [manifest]
"""
import collections, csv, re, sys, zipfile
from xml.etree import ElementTree as ET

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
WP = '{http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing}'
V = '{urn:schemas-microsoft-com:vml}'
O = '{urn:schemas-microsoft-com:office:office}'
PART = re.compile(r'word/(document|header\d*|footer\d*)\.xml$')

def walk(node, intbl, out, part):
    for child in node:
        t = child.tag
        nt = intbl + (1 if t == W + 'tbl' else 0)
        if t == WP + 'anchor':
            wrap = next((c.tag.split('}')[1] for c in child
                         if c.tag.split('}')[1].startswith('wrap')), None)
            out.append((part, 'wp:anchor', nt, child.get('layoutInCell'), wrap))
        elif t.startswith(V) and t.split('}')[1] in (
                'rect', 'shape', 'group', 'oval', 'line', 'roundrect'):
            if 'position:absolute' in (child.get('style') or '').replace(' ', ''):
                out.append((part, 'vml', nt, child.get(O + 'allowincell'), None))
        walk(child, nt, out, part)

def objects(path):
    out = []
    with zipfile.ZipFile(path) as z:
        for n in z.namelist():
            if PART.match(n):
                try:
                    walk(ET.fromstring(z.read(n)), 0, out, n)
                except ET.ParseError:
                    pass
    return out

def main():
    manifest = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files/MANIFEST.tsv'
    root = manifest.rsplit('/', 1)[0] + '/'
    rows = [r for r in csv.DictReader(open(manifest), delimiter='\t')
            if r['ext'] in ('docx', 'docm', 'dotx', 'dotm')]
    tot = collections.Counter()
    docs = collections.Counter()
    with_table = []
    for r in rows:
        try:
            out = objects(root + r['path'])
        except Exception:
            continue
        docs['docx'] += 1
        if out:
            docs['with-object'] += 1
        nt = sum(1 for o in out if o[2])
        tot['objects'] += len(out)
        tot['in-table'] += nt
        if nt:
            with_table.append((r['path'].split('/')[-1], nt, len(out)))
    print(f"DOCX-family documents scanned: {docs['docx']}, "
          f"carrying an absolutely positioned object: {docs['with-object']}")
    print(f"objects: {tot['objects']}   of them inside a w:tbl: {tot['in-table']}   "
          f"documents with one: {len(with_table)}")
    for n, a, b in sorted(with_table, key=lambda r: -r[1]):
        print(f"   {a:>4}/{b:<4} {n[:80]}")

main()
