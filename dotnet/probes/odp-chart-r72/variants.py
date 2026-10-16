#!/usr/bin/env python3
"""Build one-attribute variants of a corpus .odp's chart and render each through 26.2.4.2.

This is the probe that settled why 26.2.4.2 draws
`038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.odp`'s category
labels with no text layer at all. Each variant changes exactly one thing in the embedded
chart's `content.xml`; the reading is `textlines` (spans PyMuPDF finds in the label band) against
`smallfills` (glyph-sized filled paths there).

Measured 2026-09-07 against LibreOffice 26.2.4.2 from the TDF tarball, all four font confounds
aside. `textlines` counts spans in the label band, `smallfills` glyph-sized filled paths there,
and `legend` is the unrotated "Our Brand" key's width and font size -- the anisotropy witness.
The two legend keys are always in `textlines`, so 2 means no label was drawn as text at all.

    variant      what it changes                textlines  fills  legend
    base         (nothing)                          2       66    59.37, 13.94   <- outlined
    nolinebreak  text:line-break="false"            2       66    59.37, 13.94   <- outlined
    noarrange    no chart:label-arrangement         2       66    59.37, 13.94   <- outlined
    overlap      chart:text-overlap="true"         11        2    59.35, 14.00   wrapped, upright
    shortcats    names shortened to A..E            7        3    59.35, 14.00   upright, one line
    onecat       only "Cost Efficiency" short      10        2    59.35, 14.00   wrapped, upright
    oc15         onecat + rotation-angle 15         5        2    59.35, 14.00   TURNED, still text
    oc30         onecat + rotation-angle 30         7        2    59.35, 14.00   TURNED, still text
    oc45         onecat + rotation-angle 45         2       66    59.37, 13.94   <- outlined
    oc60         onecat + rotation-angle 60         2       61    59.38, 13.96   <- outlined
    oc90         onecat + rotation-angle 90         2        2    59.25, 13.87   text, drawn lower
    oc270        onecat + rotation-angle 270        2        2    59.25, 13.87   text, drawn lower
    short45      shortcats + rotation-angle 45      7        3    59.35, 14.00   TURNED, still text

The discriminator is the last row against oc45: same 45 degrees, same file, and the short labels
stay text. It is therefore NOT the angle. What separates them is whether the labels overflow the
chart's own page, because an embedded chart is fitted to its *drawn* extent
(ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithParameters,
svx/source/sdr/contact/viewcontactofsdrole2obj.cxx:88-116) by two factors, one per axis. An
anisotropic scale composed with a rotation that is not a right angle decomposes to a SHEAR, and
VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D
(drawinglayer/source/processor2d/vclprocessor2d.cxx:126-141) accepts a text primitive only when
`abs(fontScaling.getY() * fShearX) < 1` -- "Acceptance is restricted to no shearing and positive
scaling in X and Y". Everything else falls through to the primitive's own decomposition, which
for text is filled polygons.

The witness for the anisotropy is the legend, which is unrotated and stays text in every variant:
its font size reads 14.00 pt in the variants whose labels fit and 13.94 to 13.87 in the ones that
do not, while its width is unchanged at 59.35 to 59.38 pt.

Usage: variants.py <corpus.odp> <outdir>
"""
import os, subprocess, sys, zipfile

import pymupdf

OLD = ('chart:display-label="true" chart:tick-marks-major-inner="false" '
       'chart:tick-marks-major-outer="false" chart:logarithmic="false" '
       'chart:reverse-direction="false"')


def rotated(angle):
    return lambda s: s.replace(
        OLD, OLD.replace('chart:display-label="true"',
                         'chart:display-label="true" style:rotation-angle="%s"' % angle))


def short(s):
    for a, b in (('Product Quality', 'A'), ('Brand Reputation', 'B'), ('Cost Efficiency', 'C'),
                 ('Customer Service', 'D'), ('<text:p>Innovation</text:p>', '<text:p>E</text:p>')):
        s = s.replace(a, b)
    return s


VARIANTS = {
    'base': lambda s: s,
    'nolinebreak': lambda s: s.replace('text:line-break="true"', 'text:line-break="false"'),
    'noarrange': lambda s: s.replace(' chart:label-arrangement="side-by-side"', ''),
    'overlap': lambda s: s.replace(OLD, 'chart:text-overlap="true" ' + OLD),
    'shortcats': short,
    'onecat': lambda s: s.replace('<text:p>Cost Efficiency</text:p>', '<text:p>Cost Eff</text:p>'),
    'oc15': rotated(15), 'oc30': rotated(30), 'oc45': rotated(45),
    'oc60': rotated(60), 'oc90': rotated(90), 'oc270': rotated(270),
    'short45': lambda s: rotated(45)(short(s)),
}


def build(src, dst, fn):
    zin = zipfile.ZipFile(src)
    zo = zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED)
    for item in zin.infolist():
        data = zin.read(item.filename)
        if item.filename.startswith('Object ') and item.filename.endswith('content.xml'):
            data = fn(data.decode('utf-8')).encode('utf-8')
        if item.filename == 'mimetype':
            zo.writestr(zipfile.ZipInfo('mimetype'), data, zipfile.ZIP_STORED)
        else:
            zo.writestr(item, data)
    zo.close()
    zin.close()


def read(pdf):
    page = pymupdf.open(pdf)[0]
    lines = []
    for block in page.get_text('dict')['blocks']:
        if block['type'] != 0:
            continue
        for line in block['lines']:
            text = ''.join(s['text'] for s in line['spans'])
            if line['bbox'][0] > 400 and line['bbox'][1] > 300 and text.strip():
                lines.append((round(line['dir'][0], 3), text))
    fills = sum(1 for d in page.get_drawings()
                if d['type'] == 'f' and d['rect'].x0 > 400 and 300 < d['rect'].y0 < 430
                and d['rect'].width < 20)
    legend = [(round(l['bbox'][2] - l['bbox'][0], 2), round(l['spans'][0]['size'], 2))
              for b in page.get_text('dict')['blocks'] if b['type'] == 0
              for l in b['lines'] if ''.join(s['text'] for s in l['spans']).strip() == 'Our Brand']
    return lines, fills, legend


def main():
    src, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    for name, fn in VARIANTS.items():
        odp = os.path.join(out, name + '.odp')
        build(src, odp, fn)
        here = os.path.join(out, name)
        os.makedirs(here, exist_ok=True)
        subprocess.run(['/opt/libreoffice26.2/program/soffice', '--headless',
                        '-env:UserInstallation=file://%s/prof' % here,
                        '--convert-to', 'pdf', '--outdir', here, odp],
                       capture_output=True, timeout=300)
        pdfs = [f for f in os.listdir(here) if f.endswith('.pdf')]
        if not pdfs:
            print('%-12s NO PDF' % name)
            continue
        lines, fills, legend = read(os.path.join(here, pdfs[0]))
        print('%-12s textlines=%d smallfills=%3d legend=%s  %s'
              % (name, len(lines), fills, legend, lines[:3]))


main()
