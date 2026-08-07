#!/usr/bin/env python3
"""Build a probe deck that asks LibreOffice what autofit scale it picks, one shape per slide.

The question this exists to answer: on a shape whose paragraphs state a proportional line
spacing *below* 100 per cent, which (font scale, spacing scale) pair does the reference
choose? Both numbers are readable out of the PDF without inferring anything — the font
scale is the `/Tf` size over the stated size, and the spacing scale is the baseline pitch
over `natural x proportion x fontsize`.

One shape per slide so a page's `/Tf` and `Td` belong to exactly one box, and the shape
under test is never the first on its page — the first text object a page lays out is
formatted before `SetFixedCellHeight` takes hold and measures its lines at the face's own
ascent + descent rather than at 1.2 em (see `SlideAutofit`'s remarks). A spacer box goes
first on every slide for that reason.

    make-autofit-probe.py <template.pptx> <out.pptx> [--spacing 80] [--size 2000]
"""
import argparse
import shutil
import zipfile

EMU_PT = 12700

HEAD = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"'
        ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
        ' xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">'
        '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/>'
        '<p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>')
TAIL = '</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>'

# Deliberately plain ASCII words of mixed length: the measurement is a pitch and a size,
# and anything with an unusual advance only makes the wrap harder to predict.
WORDS = ("Proficient in more than one language and able to convey meaning "
         "accurately between two parties without adding or omitting anything")


def shape(idx, name, x, y, cx, cy, body, autofit=True):
    fit = '<a:normAutofit/>' if autofit else '<a:noAutofit/>'
    return (f'<p:sp><p:nvSpPr><p:cNvPr id="{idx}" name="{name}"/>'
            f'<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>'
            f'<p:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
            f'<p:txBody><a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0"'
            f' anchor="t">{fit}</a:bodyPr><a:lstStyle/>{body}</p:txBody></p:sp>')


def paragraph(text, size, spacing):
    lnspc = ('' if spacing == 100
             else f'<a:lnSpc><a:spcPct val="{spacing * 1000}"/></a:lnSpc>')
    return (f'<a:p><a:pPr>{lnspc}</a:pPr>'
            f'<a:r><a:rPr lang="en-GB" sz="{size}"/><a:t>{text}</a:t></a:r></a:p>')


def slide(heights_pt, width_pt, size, spacing, paras, autofit=True):
    """One measured box per slide, with a one-word spacer above it."""
    body = ''.join(paragraph(WORDS, size, spacing) for _ in range(paras))
    parts = [shape(2, 'Spacer', 200000, 100000, 2000000, 400000,
                   paragraph('spacer', 1200, 100), autofit=False),
             shape(3, 'Fit', 200000, 700000,
                   int(width_pt * EMU_PT), int(heights_pt * EMU_PT), body, autofit)]
    return HEAD + ''.join(parts) + TAIL


def build(template, out, size, spacing, heights, width, paras,
          sizes=None, autofit=True):
    shutil.copyfile(template, out)
    z = zipfile.ZipFile(template)
    names = z.namelist()
    keep = {n: z.read(n) for n in names if not n.startswith('ppt/slides/')}

    ct = keep['[Content_Types].xml'].decode()
    rels = keep['ppt/_rels/presentation.xml.rels'].decode()
    pres = keep['ppt/presentation.xml'].decode()
    layout_rel = z.read('ppt/slides/_rels/slide1.xml.rels').decode()

    ct_over, ids, pres_rels = [], [], []
    for i in range(len(heights)):
        n = i + 1
        ct_over.append(f'<Override PartName="/ppt/slides/slide{n}.xml" ContentType='
                       f'"application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>')
        ids.append(f'<p:sldId id="{255 + n}" r:id="rId{100 + n}"/>')
        pres_rels.append(f'<Relationship Id="rId{100 + n}" Type="http://schemas.openxmlformats.org'
                         f'/officeDocument/2006/relationships/slide" Target="slides/slide{n}.xml"/>')

    ct = ct.replace('<Override PartName="/ppt/slides/slide1.xml" ContentType='
                    '"application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>',
                    ''.join(ct_over))
    rels = rels.replace('<Relationship Id="rId2" Type="http://schemas.openxmlformats.org'
                        '/officeDocument/2006/relationships/slide" Target="slides/slide1.xml"/>',
                        ''.join(pres_rels))
    pres = pres.replace('<p:sldId id="256" r:id="rId2"/>', ''.join(ids))

    keep['[Content_Types].xml'] = ct.encode()
    keep['ppt/_rels/presentation.xml.rels'] = rels.encode()
    keep['ppt/presentation.xml'] = pres.encode()

    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as w:
        for n, data in keep.items():
            w.writestr(n, data)
        for i, h in enumerate(heights):
            sz = sizes[i] if sizes else size
            w.writestr(f'ppt/slides/slide{i + 1}.xml',
                       slide(h, width, sz, spacing, paras, autofit))
            w.writestr(f'ppt/slides/_rels/slide{i + 1}.xml.rels', layout_rel)


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('template')
    ap.add_argument('out')
    ap.add_argument('--spacing', type=int, default=80, help='line spacing per cent')
    ap.add_argument('--size', type=int, default=2000, help='font size in hundredths of a point')
    ap.add_argument('--width', type=float, default=360.0, help='box width in points')
    ap.add_argument('--paras', type=int, default=3)
    ap.add_argument('--from-height', type=float, default=40.0)
    ap.add_argument('--to-height', type=float, default=200.0)
    ap.add_argument('--step', type=float, default=5.0)
    ap.add_argument('--reverse', action='store_true', help='emit the heights largest first')
    ap.add_argument('--sizes', help='comma-separated pt sizes; one unshrunk slide each')
    a = ap.parse_args()

    if a.sizes:
        # The height-model probe: one slide per font size, a box far too big to shrink,
        # autofit off. What comes back is the line count and the pitch the reference
        # gives that size — the two inputs the fit search runs on, measured rather than
        # modelled.
        sizes = [int(round(float(s) * 100)) for s in a.sizes.split(',')]
        hs = [500.0] * len(sizes)
        build(a.template, a.out, a.size, a.spacing, hs, a.width, a.paras,
              sizes=sizes, autofit=False)
        print(f'{a.out}: {len(sizes)} slides, sizes {a.sizes}, spacing {a.spacing}%, '
              f'no autofit, {a.paras} paragraphs')
    else:
        hs = []
        h = a.from_height
        while h <= a.to_height + 1e-9:
            hs.append(round(h, 2))
            h += a.step
        if a.reverse:
            # The same boxes in the opposite order. If a box's answer depends on where it
            # sits in the deck, the fit is being seeded by the previous shape's answer
            # through the shared draw outliner rather than solved from scratch.
            hs.reverse()
        build(a.template, a.out, a.size, a.spacing, hs, a.width, a.paras)
        print(f'{a.out}: {len(hs)} slides, heights {hs[0]}..{hs[-1]} pt, '
              f'spacing {a.spacing}%, size {a.size / 100}pt, {a.paras} paragraphs')
