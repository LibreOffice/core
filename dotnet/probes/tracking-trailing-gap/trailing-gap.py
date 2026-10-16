#!/usr/bin/env python3
"""Does a tracked run's LAST character carry its tracking, and does Writer agree with Impress?

Why this exists
---------------

`OM template for non-complex NCC operators_August 2016.docx` renders its header
as `Rev. Xof [date]` where the reference renders `Rev. X of [date]`. The source
has no space there — `header1.xml` is `Rev.`, a space run, `X`, then `of` — but
the `X` run carries `<w:spacing w:val="45"/>`, and measured from the two PDFs'
own word boxes the reference leaves exactly 2.25 pt between them:

    reference   'X' 300.84..307.50   'of' 309.75..319.18     gap 2.25 pt
    ours        'Xof' 298.47..314.58                         gap 0

45 twentieths is 2.25 pt, so the gap is the run's *trailing* tracking, and it is
wide enough that `pdftotext` splits the word where we join it.

We charge tracking for the gap **before** each character, so a run of `n` carries
`n − 1` and the character after a tracked run gets nothing from it.
`FormattedRun.Tracking` justifies that with `SvxFont::QuickGetTextSize`
(`editeng/source/items/svxfont.cxx`:481-500), "which adds one per distinct
advance and then takes the trailing one back off".

**That citation is `editeng`, which is Draw and Impress's text engine.** A Writer
document never goes through it — Writer builds a kern array in
`sw/source/core/txtnode/fntcache.cxx` and adds the kern to each character. So the
two engines may genuinely disagree, and the rule may have to be per-family rather
than one constant.

Changing it blind is not cheap: every tracked run in every family would widen by
one tracking unit, and the corpus's commonest value is −0.2 pt over runs of fifty
characters. This measures the question instead.

What it builds
--------------

The same shape in both formats: a tracked run immediately followed by an
untracked one, with no space between them, so the only thing that can separate
the two words is the trailing tracking.

- `words.docx` — a Writer document, `w:spacing` in twentieths of a point.
- `slides.pptx` — an Impress deck, `a:rPr/@spc` in hundredths of a point.

Both are rendered by the reference alone. Ours is not the question here: what is
being established is whether *LibreOffice* treats the two the same way, because
that decides whether our single rule can stay single.

Reading the output
------------------

`gap` is the distance from the tracked word's right edge to the next word's left
edge. A gap equal to the declared tracking means the last character carried it; a
gap of nought means it did not. If Writer and Impress differ, the charge is a
per-family compatibility question like `TabOverSpacing` and the others, and not a
single constant to be flipped.

Usage
-----

    python3 trailing-gap.py /abs/workdir
"""

import os
import re
import subprocess
import sys
import zipfile

FACE = 'Liberation Serif'

# 45 twentieths of a point, the OM template's own value, and its equivalent in
# the hundredths DrawingML counts in.
TRACK_TWENTIETHS = 45
TRACK_HUNDREDTHS = 225
TRACK_POINTS = 2.25

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
P = 'http://schemas.openxmlformats.org/presentationml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'

# The <Relationships> element itself lives in the PACKAGE namespace, not the one its
# Type attributes point into. Using R for both writes a file LibreOffice refuses with
# "source file could not be loaded" and no further detail.
PKG = 'http://schemas.openxmlformats.org/package/2006/relationships'


def words_docx(path):
    """A tracked run then an untracked one, no space between them."""
    run = f'<w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}"/><w:sz w:val="40"/>'
    body = (
        f'<w:p><w:pPr><w:rPr>{run}</w:rPr></w:pPr>'
        f'<w:r><w:rPr>{run}<w:spacing w:val="{TRACK_TWENTIETHS}"/></w:rPr><w:t>AAA</w:t></w:r>'
        f'<w:r><w:rPr>{run}</w:rPr><w:t>BBB</w:t></w:r></w:p>'
        # The control: the same two runs with no tracking at all, so the pair can be
        # differenced and the face's own side bearings drop out.
        f'<w:p><w:pPr><w:rPr>{run}</w:rPr></w:pPr>'
        f'<w:r><w:rPr>{run}</w:rPr><w:t>CCC</w:t></w:r>'
        f'<w:r><w:rPr>{run}</w:rPr><w:t>DDD</w:t></w:r></w:p>')

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
        '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="567" w:right="567" w:bottom="567" w:left="567"/>'
        '</w:sectPr></w:body></w:document>')

    types = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
             '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
             'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
             '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
             'officedocument.wordprocessingml.document.main+xml"/></Types>')
    rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<Relationships xmlns="{PKG}"><Relationship Id="rId1" Type="{R}/officeDocument"'
            ' Target="word/document.xml"/></Relationships>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', types)
        z.writestr('_rels/.rels', rels)
        z.writestr('word/document.xml', document)


def slides_pptx(path):
    """The same shape as a one-slide deck, with `a:rPr/@spc` instead."""
    def body(prefix, tracked):
        spc = f' spc="{TRACK_HUNDREDTHS}"' if tracked else ''
        return (
            '<a:p>'
            f'<a:r><a:rPr lang="en-GB" sz="2000"{spc}>'
            f'<a:latin typeface="{FACE}"/></a:rPr><a:t>{prefix}AA</a:t></a:r>'
            f'<a:r><a:rPr lang="en-GB" sz="2000"><a:latin typeface="{FACE}"/></a:rPr>'
            f'<a:t>{prefix}BB</a:t></a:r></a:p>')

    slide = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f'<p:sld xmlns:a="{A}" xmlns:p="{P}" xmlns:r="{R}"><p:cSld><p:spTree>'
        '<p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>'
        '<p:grpSpPr/>'
        '<p:sp><p:nvSpPr><p:cNvPr id="2" name="t"/><p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>'
        '<p:spPr><a:xfrm><a:off x="500000" y="500000"/><a:ext cx="7000000" cy="2000000"/></a:xfrm>'
        '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></p:spPr>'
        f'<p:txBody><a:bodyPr/><a:lstStyle/>{body("A", True)}{body("C", False)}</p:txBody>'
        '</p:sp></p:spTree></p:cSld></p:sld>')

    presentation = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f'<p:presentation xmlns:a="{A}" xmlns:p="{P}" xmlns:r="{R}">'
        '<p:sldIdLst><p:sldId id="256" r:id="rId1"/></p:sldIdLst>'
        '<p:sldSz cx="9144000" cy="6858000"/><p:notesSz cx="6858000" cy="9144000"/>'
        '</p:presentation>')

    types = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
             '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
             'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
             '<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.'
             'openxmlformats-officedocument.presentationml.presentation.main+xml"/>'
             '<Override PartName="/ppt/slides/slide1.xml" ContentType="application/vnd.'
             'openxmlformats-officedocument.presentationml.slide+xml"/></Types>')
    rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<Relationships xmlns="{PKG}"><Relationship Id="rId1" Type="{R}/officeDocument"'
            ' Target="ppt/presentation.xml"/></Relationships>')
    prels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             f'<Relationships xmlns="{PKG}"><Relationship Id="rId1" Type="{R}/slide"'
             ' Target="slides/slide1.xml"/></Relationships>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', types)
        z.writestr('_rels/.rels', rels)
        z.writestr('ppt/presentation.xml', presentation)
        z.writestr('ppt/_rels/presentation.xml.rels', prels)
        z.writestr('ppt/slides/slide1.xml', slide)


def boxes(pdf):
    text = subprocess.run(['pdftotext', '-bbox', '-f', '1', '-l', '1', pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    return [(float(a), float(b), float(c), d) for a, b, c, d in re.findall(
        r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="[\d.]+">([^<]*)</word>',
        text)]


def gap(pdf, first, second):
    """The distance between two words' facing edges, or None when they were joined."""
    found = boxes(pdf)

    for x0, _, x1, word in found:
        if word == first + second:
            return 0.0, f'joined as {word!r}'

    left = next(((x0, x1) for x0, _, x1, w in found if w == first), None)
    right = next(((x0, x1) for x0, _, x1, w in found if w == second), None)

    if left is None or right is None:
        return None, f'{first!r}/{second!r} not both found in {[w for *_, w in found]}'

    return right[0] - left[1], f'{first!r} ends {left[1]:.2f}, {second!r} starts {right[0]:.2f}'


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/trailing-gap'
    os.makedirs(out, exist_ok=True)

    docx, pptx = os.path.join(out, 'words.docx'), os.path.join(out, 'slides.pptx')
    words_docx(docx)
    slides_pptx(pptx)

    for path in (docx, pptx):
        subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', out, path],
                       capture_output=True, check=True)

    print(f'declared tracking {TRACK_POINTS:.2f} pt in both formats\n')

    for label, pdf, tracked, control in [
        ('Writer  (w:spacing)', os.path.join(out, 'words.pdf'), ('AAA', 'BBB'), ('CCC', 'DDD')),
        ('Impress (a:rPr/@spc)', os.path.join(out, 'slides.pdf'), ('AAA', 'ABB'), ('CAA', 'CBB')),
    ]:
        if not os.path.isfile(pdf):
            print(f'{label}: {pdf} was not written')
            continue

        measured, how = gap(pdf, *tracked)
        baseline, _ = gap(pdf, *control)

        if measured is None:
            print(f'{label}: {how}')
            continue

        carried = measured - (baseline or 0.0)
        print(f'{label}: gap {measured:.2f} pt, control {baseline:.2f} pt, '
              f'trailing tracking {carried:+.2f} pt  ({how})')

    print('\ntrailing tracking near the declared value means the last character carried it;')
    print('near nought means it did not. Writer and Impress disagreeing makes this per-family.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
