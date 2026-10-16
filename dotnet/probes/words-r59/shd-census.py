#!/usr/bin/env python3
"""How many corpus cells and paragraphs state a `w:shd` we currently read as no fill at all.

    shd-census.py <manifest>

`ShadeColour` reads `w:fill` and ignores `w:val` except for `nil`.  ECMA-376's `w:val` is a
*pattern*, and `writerfilter`'s `CellColorHandler::getProperties` turns it into a per-mille
weight — `clear` 0, `solid` 1000, `pctN` N x 10, every striped and crossed value 333 — and then
blends `w:color` over `w:fill` at that weight.  `w:color="auto"` is black and `w:fill="auto"` is
white (`CellColorHandler::lcl_attribute`, `sw/source/writerfilter/dmapper`), so
`<w:shd w:val="solid" w:color="auto" w:fill="auto"/>` is a **black** cell and we draw nothing.

Counted per *shape*, not summed: `solid`, the percentages and the stripes go through the same
blend but a document can carry only one of them and the three answer different questions about
how much this is worth.  `clear` is counted too, as the control that must not move.

What it cannot see:
  * the 66 `.doc` documents.  WW8 states the same thing as `sprmTSetShd`/`sprmCFtcBi`-adjacent
    shading descriptors in binary, which this does not parse.
  * whether the shaded cell is ever drawn — a `w:shd` in an unreferenced table style moves
    nothing.  So every column here is an upper bound.
  * table-style conditional formatting, which can put a `w:shd` on a cell that states none.
"""
import csv, os, re, sys, zipfile, collections

man = sys.argv[1]
root = os.path.dirname(os.path.abspath(man))
SHD = re.compile(r'<w:shd\b[^>]*/>|<w:shd\b[^>]*>')
ATTR = re.compile(r'w:(val|color|fill)="([^"]*)"')

kinds = collections.Counter()
docs = collections.defaultdict(collections.Counter)
notexamined = 0

with open(man, newline='', encoding='utf-8') as f:
    for r in csv.DictReader(f, delimiter='\t'):
        if r['family'] != 'words':
            continue
        if r['ext'] != 'docx':
            notexamined += 1
            continue
        try:
            z = zipfile.ZipFile(os.path.join(root, r['path']))
        except Exception:
            continue
        for part in ('word/document.xml', 'word/styles.xml'):
            try:
                body = z.read(part).decode('utf-8', 'replace')
            except KeyError:
                continue
            for element in SHD.findall(body):
                a = dict(ATTR.findall(element))
                val = a.get('val', 'clear')
                if val in ('clear', 'nil'):
                    kind = val
                elif val == 'solid':
                    kind = 'solid'
                elif val.startswith('pct'):
                    kind = 'percentage'
                else:
                    kind = 'striped'
                # What we draw today: the fill alone, and `auto` is nothing.
                drawnNow = a.get('fill') not in (None, 'auto') and val != 'nil'
                key = (kind, 'we draw it' if drawnNow else 'WE DRAW NOTHING')
                kinds[key] += 1
                docs[key][r['path']] += 1

print('docx examined; .doc not examined: %d' % notexamined)
print()
for (kind, drawn), n in sorted(kinds.items(), key=lambda t: -t[1]):
    print('%-11s %-16s %5d elements in %3d documents' % (kind, drawn, n, len(docs[(kind, drawn)])))
print()
print('the arm that moves — a stated pattern whose fill we read as nothing:')
for (kind, drawn), by in sorted(docs.items()):
    if drawn == 'we draw it' or kind in ('clear', 'nil'):
        continue
    for path, n in by.most_common():
        print('   %-9s %4d  %s' % (kind, n, path))
