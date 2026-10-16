#!/usr/bin/env python3
"""Every `w:object` in the corpus, and whether this tree draws its picture twice.

    object-dup-census.py <corpus-root> <manifest.tsv> <out.tsv>

`DocxVmlFrames.TopLevel` filters its candidates with `IsShape(child) is not false`, and
`IsShape` returns `true` or `null` and never `false` -- so EVERY descendant of the element is
offered to `One`.  Inside a `w:pict` that is harmless: a `v:imagedata`, a `v:path` or an
`o:lock` carries no CSS `style`, `One` finds no width and returns null.  Inside a `w:object`
it is not, because `One` falls back to the object's own `w:dxaOrig`/`w:dyaOrig`, which gives
every descendant a size; and `DocxPictures.ReadVml` searches `DescendantsAndSelf`, so the
`v:imagedata` element resolves ITSELF and the replacement picture is drawn a second time.

So the census counts, per document and per `w:object`:

  has_imagedata   the object states a `v:imagedata` -- something to draw twice
  has_orig        the object states `w:dxaOrig`/`w:dyaOrig` -- the size the second draw takes
  dup             both: this tree draws the replacement picture twice
  same_box        the `v:shape` states no `style` size either, so the two draws coincide
                  exactly and the doubling is invisible except in the ink and the text
"""
import pathlib
import re
import sys
import zipfile

root, man, out = sys.argv[1:4]
W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
PARTS = re.compile(r'^word/(document|header\d*|footer\d*|footnotes|endnotes)\.xml$')

import xml.etree.ElementTree as ET

rows = []
for line in pathlib.Path(man).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    if len(p) < 4 or p[3] not in ('docx', 'docm', 'dotx'):
        continue
    src = pathlib.Path(root) / p[2]
    objs = dup = same = withimg = withorig = 0
    try:
        with zipfile.ZipFile(src) as z:
            for name in z.namelist():
                if not PARTS.match(name):
                    continue
                try:
                    tree = ET.fromstring(z.read(name))
                except ET.ParseError:
                    continue
                for obj in tree.iter(W + 'object'):
                    objs += 1
                    img = any(e.tag.endswith('}imagedata') for e in obj.iter())
                    orig = obj.get(W + 'dxaOrig') is not None and obj.get(W + 'dyaOrig') is not None
                    withimg += bool(img)
                    withorig += bool(orig)
                    if img and orig:
                        dup += 1
                        styled = False
                        for e in obj.iter():
                            if e.tag.endswith('}shape') and e.get('style') \
                                    and 'width:' in e.get('style'):
                                styled = True
                        if not styled:
                            same += 1
    except (zipfile.BadZipFile, OSError):
        continue
    if objs:
        rows.append((p[2], objs, withimg, withorig, dup, same))

rows.sort(key=lambda r: -r[4])
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('path\tobjects\twith_imagedata\twith_dxaOrig\tdup\tdup_same_box\n')
    for r in rows:
        fh.write('\t'.join(str(x) for x in r) + '\n')
print(f'{len(rows)} docx hold a w:object; '
      f'{sum(1 for r in rows if r[4])} hold one this tree draws twice, '
      f'{sum(r[4] for r in rows)} objects in all, '
      f'{sum(r[5] for r in rows)} of them at the identical box')
