#!/usr/bin/env python3
"""Isolate the duplicated `w:object` draw with a one-attribute variant of each document.

    dxaorig-variant.py <corpus-root> <objdup.tsv> <workdir>

`One` reaches for `w:dxaOrig`/`w:dyaOrig` only when the element it was handed states no CSS
size, which inside a `w:object` is every descendant except the `v:shape` itself.  Removing
those two attributes therefore removes exactly the spurious frames and nothing else -- the
`v:shape`'s own `style` still sizes the real one.  Rendering each document as authored and
with the pair removed, and differencing OUR two renderings, measures the defect with the
reference nowhere in the instrument.
"""
import os
import pathlib
import re
import shutil
import subprocess
import sys
import zipfile

import pymupdf

root, census, work = sys.argv[1:4]
CLI = os.environ['XRAY_OFFICE_CLI']
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
PARTS = re.compile(r'^word/(document|header\d*|footer\d*|footnotes|endnotes)\.xml$')
ATTR = re.compile(r'\s+w:(?:dxa|dya)Orig="[^"]*"')

work = pathlib.Path(work)
work.mkdir(parents=True, exist_ok=True)


def measure(pdf):
    d = pymupdf.open(pdf)
    chars = imgs = draws = 0
    text = []
    for p in d:
        t = p.get_text('text')
        text.append(t)
        chars += len(''.join(t.split()))
        imgs += len(p.get_image_info())
        draws += len(p.get_drawings())
    n = d.page_count
    d.close()
    return n, chars, imgs, draws, ''.join(text)


def render(src, out):
    out.mkdir(parents=True, exist_ok=True)
    subprocess.run([CLI, 'render', str(src), '--outdir', str(out)],
                   capture_output=True, check=False,
                   env=dict(os.environ, SOURCE_DATE_EPOCH='1757462400'))
    got = sorted(out.glob('*.pdf'))
    return got[0] if got else None


print('document\tobjects\tpages\td_chars\td_images\td_draws\tdup_text_chars')
tot = [0, 0, 0, 0]
for line in pathlib.Path(census).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    rel, nobj = p[0], int(p[4])
    stem = pathlib.PurePosixPath(rel).stem
    src = pathlib.Path(root) / rel
    d = work / stem
    if d.exists():
        shutil.rmtree(d)
    d.mkdir(parents=True)
    stripped = d / 'stripped.docx'
    n = 0
    with zipfile.ZipFile(src) as zin, zipfile.ZipFile(stripped, 'w', zipfile.ZIP_DEFLATED) as zo:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if PARTS.match(item.filename):
                text = data.decode('utf-8')
                text, k = ATTR.subn('', text)
                n += k
                data = text.encode('utf-8')
            zo.writestr(item, data)
    a = render(src, d / 'as-authored')
    b = render(stripped, d / 'stripped')
    if not a or not b:
        print(f'{stem}\t{nobj}\tRENDER-FAILED')
        continue
    ma, mb = measure(a), measure(b)
    # the text the duplicate adds: longest run of characters present in `a` and not in `b`
    dup = ma[1] - mb[1]
    print(f'{stem}\t{nobj}\t{ma[0]}/{mb[0]}\t{ma[1] - mb[1]}\t{ma[2] - mb[2]}\t'
          f'{ma[3] - mb[3]}\t{dup}', flush=True)
    tot[0] += ma[1] - mb[1]
    tot[1] += ma[2] - mb[2]
    tot[2] += ma[3] - mb[3]
    tot[3] += 1 if ma[0] != mb[0] else 0
print(f'TOTAL\t\t\t{tot[0]}\t{tot[1]}\t{tot[2]}\tpage-count moves {tot[3]}')
