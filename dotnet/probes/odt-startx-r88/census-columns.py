#!/usr/bin/env python3
"""Every style:columns in a corpus of ODF files, by where it sits and how it describes its columns.

Two arms decide how a reader must lay one out, and they are `XMLTextColumnsContext::endFastElement`'s
own (`xmloff/source/text/XMLTextColumnsContext.cxx`:268-315):

  * a stated `fo:column-gap` sets `bAutomatic`, and the per-column `style:column` descriptions are then
    ignored outright — even columns, that gap;
  * with no gap and one description per column, the descriptions are taken and the columns are
    apportioned by `style:rel-width`.

So a file stating a gap *and* unequal widths is drawn with even columns, and reading its widths is
wrong.  The counts below are what separate the two arms on the converted corpus.
"""
import collections, pathlib, re, sys, zipfile

COLUMNS = re.compile(r'<style:columns\b[^>]*(?:/>|>.*?</style:columns>)', re.S)
COLUMN = re.compile(r'<style:column\b[^>]*/>')
COUNT = re.compile(r'fo:column-count="(\d+)"')
GAP = re.compile(r'fo:column-gap=')
REL = re.compile(r'style:rel-width="(\d+)\*"')
# The properties element a style:columns sits in, which says whether it is a page's or a section's.
OWNER = re.compile(r'<style:(section|page-layout)-properties\b')

def owners(xml):
    """(offset, owner) for every properties element, so a columns element can be attributed."""
    return [(m.start(), m.group(1)) for m in OWNER.finditer(xml)]

def owner_of(marks, at):
    kind = '?'
    for start, name in marks:
        if start > at:
            break
        kind = name
    return kind

counts = collections.Counter()
docs = collections.defaultdict(set)

for path in sorted(pathlib.Path(sys.argv[1]).rglob('*')):
    if path.suffix.lower() not in ('.odt', '.ott', '.fodt'):
        continue
    try:
        with zipfile.ZipFile(path) as z:
            parts = [(n, z.read(n).decode('utf-8', 'replace'))
                     for n in ('content.xml', 'styles.xml') if n in z.namelist()]
    except Exception:
        continue

    for _, xml in parts:
        marks = owners(xml)
        for m in COLUMNS.finditer(xml):
            block = m.group(0)
            head = block.split('>', 1)[0]
            count = int(COUNT.search(head).group(1)) if COUNT.search(head) else 0
            stated = COLUMN.findall(block)
            widths = {r for r in REL.findall(block)}
            arm = ('gap' if GAP.search(head)
                   else 'explicit' if count > 1 and len(stated) == count
                   else 'even')
            key = (owner_of(marks, m.start()), arm,
                   'unequal' if len(widths) > 1 else 'equal', count)
            counts[key] += 1
            docs[key[:3]].add(path.name)

for key in sorted(counts):
    print(f"{key[0]:>12} {key[1]:>9} {key[2]:>8} count={key[3]}   {counts[key]}")
print()
for key in sorted(docs):
    print(f"{key[0]}/{key[1]}/{key[2]}: {len(docs[key])} documents")
    if key[1] == 'explicit':
        for name in sorted(docs[key]):
            print(f"    {name}")
