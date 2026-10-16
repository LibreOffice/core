#!/usr/bin/env python3
"""Separate "we drop a picture the file states" from "the reference rasterises".

    image-gap-census.py <corpus-root> <manifest.tsv> <ranking.tsv> <out.tsv>

A reference that draws image placements where we draw none has two quite different causes,
and the count alone cannot tell them apart:

  * the file holds a picture and we do not draw it -- ours;
  * the file holds NO picture at all and 26.2.4.2 rasterised something it could not emit as
    vectors, which is confound C7, the raster ceiling.

The discriminator is the package itself: how many parts live under a `media/` folder.  It is
crude -- a file may hold a picture on a slide we do draw and lose a different one -- so the
column it produces is a filter for a hand check and not a verdict.
"""
import pathlib
import sys
import zipfile

root, man, rank, out = sys.argv[1:5]

ident = {}
for line in pathlib.Path(man).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    if len(p) < 4:
        continue
    ident[p[2].rsplit('/', 1)[-1].rsplit('.', 1)[0] + '__' + p[3]] = p[2]

rows = []
for line in pathlib.Path(rank).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    io, ir = int(p[15]), int(p[16])
    if ir <= io:
        continue
    rel = ident.get(p[0])
    if rel is None:
        continue
    try:
        with zipfile.ZipFile(pathlib.Path(root) / rel) as z:
            media = sum(1 for n in z.namelist() if '/media/' in n)
        kind = 'zip'
    except (zipfile.BadZipFile, OSError):
        media, kind = -1, 'ole2'
    rows.append((p[0], p[18], p[17], io, ir, ir - io, media, kind, p[9], p[10]))

rows.sort(key=lambda r: -r[5])
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\ttrack\tprior\timgs_ours\timgs_ref\tmissing\tmedia_parts\tcontainer\t'
             'draws_ours\tdraws_ref\n')
    for r in rows:
        fh.write('\t'.join(str(x) for x in r) + '\n')
z0 = [r for r in rows if r[3] == 0]
print(f'{len(rows)} documents where the reference places more images than we do; '
      f'{len(z0)} of them draw NO image at all, of which '
      f'{sum(1 for r in z0 if r[6] == 0)} hold no media part (C7) and '
      f'{sum(1 for r in z0 if r[6] > 0)} do hold one')
