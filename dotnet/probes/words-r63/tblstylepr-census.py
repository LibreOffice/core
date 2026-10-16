#!/usr/bin/env python3
"""What a `w:tblStylePr` layer's *non-run* halves would reach across the words corpus.

Round 62 established that `012`'s 56 missing fills come from `w:tblStylePr`'s `w:tcPr` half —
`firstCol` and `band1Horz` — which this reader has never read. This sizes that, and it sizes the
risk beside it: adding the band layers to `WordTableStyleConditions.Names` also feeds them to
`TableStyleRunProperties`, so a band layer carrying a `w:rPr` would change how text is *measured*
and could move a page count.

Reported per document and never summed across kinds:

  tcPr-shd        layers with a `w:shd` under `w:tcPr`, by layer name
  tcPr-borders    layers with a `w:tcBorders` under `w:tcPr`, by layer name
  rPr-band        layers named band*Horz / band*Vert that carry a `w:rPr`  ← the regression risk
  used            the style is actually named by a `w:tblStyle` on a table in a body part

What this census CANNOT see, stated before the sweep:

  * whether a cell that resolves to a conditional shading also states its own `w:shd`, which wins;
  * `w:tblLook`'s own bits — a style may declare `firstCol` on a table that never asks for it;
  * inheritance through `w:basedOn`: a style is counted for the layers it declares itself, so a
    style based on one that declares layers reads as declaring none;
  * the `.doc` and `.rtf` readers, which have no `w:tblStylePr` at all;
  * latent styles and `w:tblStyleRowBandSize` defaults — a style stating no band size bands at 1.

    tblstylepr-census.py [corpus-root]
"""
import collections
import os
import re
import sys
import zipfile
from xml.etree import ElementTree as ET

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
MANIFEST = '/c/sandbox/workdir/sample-files/MANIFEST.tsv'
BANDS = ('band1Horz', 'band2Horz', 'band1Vert', 'band2Vert')


def paths(track='words'):
    out = []
    with open(MANIFEST, encoding='utf-8') as f:
        next(f)
        for line in f:
            p = line.rstrip('\n').split('\t')
            if len(p) > 3 and p[0] == track:
                out.append((p[2], p[3]))
    return out


def main(root='/c/sandbox/workdir/sample-files'):
    docs = 0
    with_layers = 0
    used_docs = []
    counts = collections.Counter()
    band_rpr = []
    band_sizes = collections.Counter()
    for rel, ext in paths():
        if ext != 'docx':
            continue
        docs += 1
        full = os.path.join(root, rel)
        try:
            with zipfile.ZipFile(full) as z:
                names = set(z.namelist())
                if 'word/styles.xml' not in names:
                    continue
                styles = ET.fromstring(z.read('word/styles.xml'))
                bodies = b''.join(z.read(n) for n in names
                                  if re.fullmatch(r'word/(document|header\d*|footer\d*)\.xml', n))
        except Exception as exc:                                   # noqa: BLE001
            print('  ! %s: %s' % (rel, exc))
            continue

        named = set(re.findall(rb'<w:tblStyle [^>]*w:val="([^"]+)"', bodies))
        named = {n.decode('utf-8', 'replace') for n in named}

        per = collections.Counter()
        rows = []
        for style in styles.iter(W + 'style'):
            sid = style.get(W + 'styleId') or ''
            layers = style.findall(W + 'tblStylePr')
            if not layers:
                continue
            tblpr = style.find(W + 'tblPr')
            if tblpr is not None:
                for kind in ('tblStyleRowBandSize', 'tblStyleColBandSize'):
                    el = tblpr.find(W + kind)
                    if el is not None:
                        band_sizes[(kind, el.get(W + 'val'))] += 1
            for layer in layers:
                kind = layer.get(W + 'type') or '?'
                tcpr = layer.find(W + 'tcPr')
                if tcpr is not None and tcpr.find(W + 'shd') is not None:
                    per[('tcPr-shd', kind)] += 1
                if tcpr is not None and tcpr.find(W + 'tcBorders') is not None:
                    per[('tcPr-borders', kind)] += 1
                if kind in BANDS and layer.find(W + 'rPr') is not None:
                    rows.append((sid, kind, sid in named))
        if not per and not rows:
            continue
        with_layers += 1
        counts.update(per)
        if rows:
            band_rpr.append((rel, rows))
        hit = [k for k in per if k[0] == 'tcPr-shd']
        if hit:
            used_docs.append((rel, sum(v for k, v in per.items() if k[0] == 'tcPr-shd')))

    print('.docx in the manifest                     : %d' % docs)
    print('declaring a w:tblStylePr with tcPr or band rPr : %d' % with_layers)
    print()
    print('layers carrying a w:tcPr half, by kind:')
    for (half, kind), n in sorted(counts.items()):
        print('   %-14s %-12s %d' % (half, kind, n))
    print()
    print('band-size declarations: %s' % dict(band_sizes))
    print()
    print('documents whose styles declare a tcPr/w:shd layer (%d):' % len(used_docs))
    for rel, n in sorted(used_docs, key=lambda r: -r[1]):
        print('   %-4d %s' % (n, rel))
    print()
    print('band layers carrying a w:rPr — the line-breaking risk (%d documents):' % len(band_rpr))
    for rel, rows in band_rpr:
        used = [r for r in rows if r[2]]
        print('   %s' % rel)
        for sid, kind, is_used in rows:
            print('        %-24s %-10s style named by a table: %s' % (sid, kind, is_used))
        if not used:
            print('        -> no table in the document names any of these styles')


if __name__ == '__main__':
    main(*sys.argv[1:])
