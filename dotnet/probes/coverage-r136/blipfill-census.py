#!/usr/bin/env python3
"""Every shape in the corpus whose own fill is a picture, by format.

    blipfill-census.py <corpus-root> <manifest.tsv> <out.tsv>

`a:blipFill` inside a shape's `spPr` is a bitmap used as that shape's interior.  It is not the
same element as the picture wrapper -- `xdr:pic/xdr:blipFill`, `p:pic/p:blipFill`,
`pic:pic/pic:blipFill` -- which every reader here already draws, so the census separates them
and counts only the fill form.  `a:grpFill` is counted beside it because a member saying
`a:grpFill` inside a blip-filled group inherits the same picture.
"""
import pathlib
import sys
import xml.etree.ElementTree as ET
import zipfile

A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'
SHAPE = ('sp', 'cxnSp', 'grpSp', 'graphicFrame')
PARTS = {
    'xlsx': ('xl/drawings/drawing',),
    'pptx': ('ppt/slides/slide', 'ppt/slideLayouts/slideLayout', 'ppt/slideMasters/slideMaster'),
    'docx': ('word/document.xml', 'word/header', 'word/footer'),
}
PARTS['xlsm'] = PARTS['xlsx']

root, man, out = sys.argv[1:4]
rows = []
for line in pathlib.Path(man).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    if len(p) < 4:
        continue
    ext = p[3]
    pref = PARTS.get(ext)
    if pref is None:
        continue
    n_fill = n_grp = 0
    try:
        with zipfile.ZipFile(pathlib.Path(root) / p[2]) as z:
            for name in z.namelist():
                if not any(name.startswith(x) for x in pref) or not name.endswith('.xml'):
                    continue
                try:
                    tree = ET.fromstring(z.read(name))
                except ET.ParseError:
                    continue
                for el in tree.iter():
                    if not el.tag.endswith('}spPr'):
                        continue
                    # the shape's own properties, not a picture wrapper's
                    if el.find(A + 'blipFill') is not None:
                        n_fill += 1
                    if el.find(A + 'grpFill') is not None:
                        n_grp += 1
    except (zipfile.BadZipFile, OSError, KeyError):
        continue
    if n_fill or n_grp:
        rows.append((p[2], ext, p[0], n_fill, n_grp))

rows.sort(key=lambda r: -r[3])
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('path\text\ttrack\tspPr_blipFill\tspPr_grpFill\n')
    for r in rows:
        fh.write('\t'.join(str(x) for x in r) + '\n')
for e in ('xlsx', 'xlsm', 'pptx', 'docx'):
    sel = [r for r in rows if r[1] == e and r[3]]
    print(f'{e}: {len(sel)} documents, {sum(r[3] for r in sel)} shapes whose own fill is a picture')
